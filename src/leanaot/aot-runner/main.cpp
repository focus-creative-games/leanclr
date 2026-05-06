#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "alloc/general_allocation.h"
#include "metadata/module_def.h"
#include "metadata/pe_image_reader.h"
#include "alloc/mem_pool.h"
#include "vm/assembly.h"
#include "vm/class.h"
#include "vm/method.h"
#include "vm/runtime.h"
#include "vm/settings.h"
#include "vm/type.h"
#include "vm/rt_exception.h"
#include "vm/property.h"
#include "vm/field.h"
#include "metadata/metadata_name.h"

#ifdef _WIN32
#include <windows.h>
#endif

using namespace leanclr;

// Global library search directories
static std::vector<std::string> g_lib_dirs;

static RtResult<vm::FileData> assembly_file_loader(const char* assembly_name, const char* extension)
{
    for (const auto& dir : g_lib_dirs)
    {
        std::string file_path = dir + "/" + assembly_name + "." + extension;
        std::ifstream dll_file(file_path, std::ios::binary | std::ios::ate);
        if (!dll_file.is_open())
        {
            continue; // Try next directory
        }

        std::streamsize file_size = dll_file.tellg();
        dll_file.seekg(0, std::ios::beg);

        auto* dll_data = static_cast<uint8_t*>(alloc::GeneralAllocation::malloc(file_size));
        if (!dll_data)
        {
            return RtErr::OutOfMemory;
        }

        if (!dll_file.read(reinterpret_cast<char*>(dll_data), file_size))
        {
            alloc::GeneralAllocation::free(dll_data);
            continue;
        }
        dll_file.close();

        return vm::FileData{dll_data, static_cast<size_t>(file_size)};
    }

    return RtErr::FileNotFound;
}

static void print_usage(const char* program_name)
{
    std::cerr << "Usage: " << program_name << " [options] <dll_name> [-- <dll_args>...]\n"
              << "Options:\n"
              << "  -l, --lib-dir <dir>    Add library search directory\n"
              << "  -e, --entry <entry>    Specify entry point (format: FullClassName::MethodName)\n"
              << "  --                     Arguments after this are passed to the target dll\n"
              << "\nExample:\n"
              << "  " << program_name << " -l . -l bin/Release MyApp -- arg1 arg2\n"
              << "  " << program_name << " -e MyNamespace.MyClass::Main MyApp\n";
}

static void print_error_and_exit(const std::string& err_message, RtErr err)
{
    std::cerr << "Error: " << err_message << " (Error code: " << static_cast<int>(err) << ")" << std::endl;

    vm::RtException* ex = vm::Exception::raise_error_as_exception(err, nullptr, nullptr);
    if (!ex)
    {
        std::cerr << "Failed to raise exception for invocation error" << std::endl;
        std::exit(-1);
    }

    const metadata::RtPropertyInfo* prop = vm::Class::get_property_for_name(ex->klass, "StackTrace", true);
    assert(prop);
    auto ret = vm::Runtime::invoke_with_run_cctor(prop->get_method, ex, nullptr);
    if (ret.is_err())
    {
        std::cerr << "Failed to get exception stack trace" << std::endl;
        std::exit(-1);
    }

    std::cerr << std::endl;
    std::cerr << std::endl;

    utils::StringBuilder sb;

    metadata::MetadataName::append_klass_full_name(sb, ex->klass).is_ok();
    sb.append_cstr(": ");
    sb.sure_null_terminator_but_not_append();
    std::cerr << sb.as_cstr();

    sb.clear();
    vm::RtString* message = ex->message;
    if (message)
    {
        sb.append_utf16_str(&message->first_char, message->length);
    }
    else
    {
        sb.sure_null_terminator_but_not_append();
    }
    std::cerr << sb.as_cstr() << std::endl << std::endl;

    sb.clear();
    vm::RtString* stack_trace_str = reinterpret_cast<vm::RtString*>(ret.unwrap());
    sb.append_utf16_str(&stack_trace_str->first_char, stack_trace_str->length);
    std::cerr << sb.as_cstr() << std::endl << std::endl;

    std::exit(-1);
}

static void print_error_and_exit(const std::string& message)
{
    std::cerr << "Error: " << message << std::endl;
    std::exit(-1);
}

static RtResult<const metadata::RtMethodInfo*> find_entry_method(metadata::RtModuleDef* mod, const std::string& entry_spec)
{
    // Parse entry spec: "FullClassName::MethodName"
    size_t separator = entry_spec.find("::");
    if (separator == std::string::npos)
    {
        std::cerr << "Invalid entry format. Expected: <FullClassName>::<MethodName>" << std::endl;
        RET_ERR(RtErr::EntryPointNotFound);
    }

    std::string class_name = entry_spec.substr(0, separator);
    std::string method_name = entry_spec.substr(separator + 2);

    // Find class
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, mod->get_class_by_nested_full_name(class_name.c_str(), false, true));

    if (klass == nullptr)
    {
        std::cerr << "Class not found: " << class_name << std::endl;
        RET_ERR(RtErr::EntryPointNotFound);
    }

    // Initialize class
    RET_ERR_ON_FAIL(vm::Class::initialize_all(klass));

    const metadata::RtMethodInfo* method = vm::Method::find_matched_method_in_class_by_name(klass, method_name.c_str());
    if (!method)
    {
        std::cerr << "Method not found: " << method_name << " in class " << class_name << std::endl;
        RET_ERR(RtErr::EntryPointNotFound);
    }
    RET_OK(method);
}

static int run(const std::string& dll_name, const std::vector<std::string>& dll_args, const std::string* entry_spec)
{
    // Initialize runtime
    std::vector<const char*> args_ptrs;
    args_ptrs.push_back(dll_name.c_str());
    for (const auto& arg : dll_args)
    {
        args_ptrs.push_back(arg.c_str());
    }

    vm::Settings::set_command_line_arguments(static_cast<int>(args_ptrs.size()), args_ptrs.data());

    auto init_result = vm::Runtime::initialize();
    if (init_result.is_err())
    {
        std::cerr << "Failed to initialize runtime, error: " << static_cast<int>(init_result.unwrap_err()) << std::endl;
        return -1;
    }

    // Load assembly
    auto ass_result = vm::Assembly::load_by_name(dll_name.c_str());
    if (ass_result.is_err())
    {
        print_error_and_exit("Failed to load assembly", ass_result.unwrap_err());
    }
    metadata::RtModuleDef* mod = ass_result.unwrap()->mod;

    // Find entry method
    const metadata::RtMethodInfo* entry_method = nullptr;
    if (entry_spec != nullptr && !entry_spec->empty())
    {
        // Use specified entry point
        auto method_result = find_entry_method(mod, *entry_spec);
        if (method_result.is_err())
        {
            print_error_and_exit("Failed to find entry method", method_result.unwrap_err());
        }
        entry_method = method_result.unwrap();
    }
    else
    {
        // Use assembly entry point
        uint32_t entry_token = mod->get_entrypoint_token();
        if (entry_token == 0)
        {
            print_error_and_exit("No entry point found in assembly");
        }

        uint32_t entry_rid = metadata::RtToken::decode_rid(entry_token);
        auto method_result = mod->get_method_by_rid(entry_rid);
        if (method_result.is_err())
        {
            print_error_and_exit("Failed to get entry method", method_result.unwrap_err());
        }
        entry_method = method_result.unwrap();
    }

    // Verify entry method is static
    if (!vm::Method::is_static(entry_method))
    {
        print_error_and_exit("Entry method must be static");
    }

    if (vm::Method::get_param_count_include_this(entry_method) != 0)
    {
        print_error_and_exit("Entry method must have no parameters");
    }

    if (vm::Method::contains_not_instantiated_generic_param(entry_method))
    {
        print_error_and_exit("Entry method must not be generic");
    }

    // Invoke entry method
    auto invoke_result = vm::Runtime::invoke_array_arguments_with_run_cctor(entry_method, nullptr, nullptr);
    if (invoke_result.is_err())
    {
        std::cerr << "Failed to invoke entry method, error: " << static_cast<int>(invoke_result.unwrap_err()) << std::endl;
        print_error_and_exit("Invocation failed", invoke_result.unwrap_err());
    }

    return 0;
}

extern leanclr::metadata::RtAotModulesData g_aot_modules_data;

int main(int argc, char* argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    std::vector<std::string> lib_dirs;
    lib_dirs.push_back("."); // Default current directory
    std::string entry_spec;
    std::string dll_name;
    std::vector<std::string> dll_args;

    // Parse command line arguments
    bool parsing_dll_args = false;
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (parsing_dll_args)
        {
            dll_args.push_back(arg);
            continue;
        }

        if (arg == "--")
        {
            parsing_dll_args = true;
            continue;
        }

        if (arg == "-l" || arg == "--lib-dir")
        {
            if (i + 1 >= argc)
            {
                std::cerr << "Missing value for " << arg << std::endl;
                print_usage(argv[0]);
                return 2;
            }
            lib_dirs.push_back(argv[++i]);
        }
        else if (arg == "-e" || arg == "--entry")
        {
            if (i + 1 >= argc)
            {
                std::cerr << "Missing value for " << arg << std::endl;
                print_usage(argv[0]);
                return 2;
            }
            entry_spec = argv[++i];
        }
        else if (arg == "-h" || arg == "--help")
        {
            print_usage(argv[0]);
            return 0;
        }
        else if (arg[0] == '-')
        {
            std::cerr << "Unknown option: " << arg << std::endl;
            print_usage(argv[0]);
            return 2;
        }
        else
        {
            if (dll_name.empty())
            {
                dll_name = arg;
            }
            else
            {
                std::cerr << "Unexpected positional argument: " << arg << std::endl;
                print_usage(argv[0]);
                return 2;
            }
        }
    }

    if (dll_name.empty())
    {
        std::cerr << "Missing target dll name" << std::endl;
        print_usage(argv[0]);
        return 2;
    }

    // Initialize global library directories
    g_lib_dirs = std::move(lib_dirs);

    // Set assembly loader
    vm::Settings::set_file_loader(assembly_file_loader);
    vm::Settings::set_aot_modules_data(&g_aot_modules_data);

    // Run
    int result = run(dll_name, dll_args, entry_spec.empty() ? nullptr : &entry_spec);
    if (result == 0)
    {
        std::cout << "ok!" << std::endl;
    }
    else
    {
        std::cerr << "run failed." << std::endl;
    }

    return result;
}
