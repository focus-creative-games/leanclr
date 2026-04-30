

#include <cstdlib>
#include <csignal>
#include <cstdio>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "alloc/general_allocation.h"
#include "metadata/pe_image_reader.h"
#include "alloc/mem_pool.h"
#include "metadata/module_def.h"
#include "vm/assembly.h"
#include "vm/settings.h"
#include "vm/runtime.h"
#include "vm/class.h"
#include "vm/method.h"
#include "vm/rt_string.h"
#include "vm/object.h"
#include "vm/customattribute.h"
#include "interp/interpreter.h"

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#include <unistd.h>
#endif

using namespace leanclr;

// Global library search directories
static std::vector<std::string> g_lib_dirs;

static std::string get_current_working_directory()
{
    char buffer[4096];
#ifdef _WIN32
    if (_getcwd(buffer, sizeof(buffer)) == nullptr)
#else
    if (getcwd(buffer, sizeof(buffer)) == nullptr)
#endif
    {
        return ".";
    }
    return std::string(buffer);
}

static RtResult<vm::FileData> assembly_file_loader(const char* assembly_name, const char* ext)
{
    for (const auto& dir : g_lib_dirs)
    {
        std::string file_path = dir + "/" + assembly_name + "." + ext;
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

static void setup_default_lib_dirs()
{
    g_lib_dirs.push_back("."); // Current directory
    std::string cur_dir = get_current_working_directory();

    size_t pos = cur_dir.find("src");
    if (pos != std::string::npos)
    {
        std::string library_dir = cur_dir.substr(0, pos) + "src/libraries";
        g_lib_dirs.push_back(library_dir + "/dotnetframework4.x"); // Example additional directory
        std::string test_dir = cur_dir.substr(0, pos) + "src/tests/managed";
        // g_lib_dirs.push_back(test_dir + "/ILTests/bin/Debug");
        g_lib_dirs.push_back(test_dir + "/CoreTests/bin/Debug");
        g_lib_dirs.push_back(test_dir + "/CorlibTests/bin/Debug");
        g_lib_dirs.push_back(test_dir + "/MiscDlls");
    }
    for (const auto& dir : g_lib_dirs)
    {
        std::cout << "Library search directory: " << dir << std::endl;
    }
}

static RtResultVoid initialize_all_classes(metadata::RtAssembly* ass)
{
    metadata::RtModuleDef* mod = ass->mod;
    for (uint32_t rid = 1; rid <= mod->get_class_count(); rid++)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, mod->get_class_by_type_def_rid(rid));
        RET_ERR_ON_FAIL(vm::Class::initialize_all(klass));
        // std::cout << "Initializing class RID: " << rid << " full_name: " << klass->namespaze << "." << klass->name << " fields: " << klass->field_count << "
        // methods: " << klass->method_count
        //           << std::endl;
    }
    RET_VOID_OK();
}

static size_t g_count = 0;
static size_t g_skip = 0; // 12028;

static RtResultVoid transform_all_classes(metadata::RtAssembly* ass)
{
    metadata::RtModuleDef* mod = ass->mod;
    for (uint32_t rid = 1; rid <= mod->get_class_count(); rid++)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, mod->get_class_by_type_def_rid(rid));
        RET_ERR_ON_FAIL(vm::Class::initialize_all(klass));
        for (uint32_t i = 0; i < klass->method_count; i++)
        {
            const metadata::RtMethodInfo* method = klass->methods[i];
            if (!method->arg_descs || method->invoker_type != metadata::RtInvokerType::Interpreter || !vm::Method::has_method_body(method))
            {
                continue;
            }
            if (g_count++ <= g_skip)
            {
                continue;
            }
            RET_ERR_ON_FAIL(interp::Interpreter::execute(method, nullptr));

            std::cout << "[" << (g_count - 1) << "] exec method  full_name: " << klass->namespaze << "." << klass->name << "::" << method->name << std::endl;
        }
    }
    RET_VOID_OK();
}

static RtResultVoid run_bootstrap_tests(metadata::RtModuleDef* testMod)
{
    std::cout << "Running BootstrapTests..." << std::endl;
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, testClass, testMod->get_class_by_name("BootstrapTests.TestReturnValue", false, true));
        RET_ERR_ON_FAIL(vm::Class::initialize_all(testClass));
        {
            const metadata::RtMethodInfo* testMethod = vm::Method::find_matched_method_in_class_by_name(testClass, "ReturnInt");
            if (!testMethod)
            {
                RET_ERR(RtErr::MissingMethod);
            }
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, result, vm::Runtime::invoke_with_run_cctor(testMethod, nullptr, nullptr));
            assert(result->klass->by_val->ele_type == metadata::RtElementType::I4);
            int32_t int_result = *(int32_t*)(result + 1);
            assert(int_result == 42);
        }
        {
            const metadata::RtMethodInfo* testMethod = vm::Method::find_matched_method_in_class_by_name(testClass, "ReturnString");
            if (!testMethod)
            {
                RET_ERR(RtErr::MissingMethod);
            }
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, result, vm::Runtime::invoke_with_run_cctor(testMethod, nullptr, nullptr));
            assert(result->klass->by_val->ele_type == metadata::RtElementType::String);
            vm::RtString* s = reinterpret_cast<vm::RtString*>(result);
            assert(s->length == 3);
            Utf16Char* chars = &s->first_char;
            assert(chars[0] == 'a' && chars[1] == 'b' && chars[2] == 'c');
        }
        {
            const metadata::RtMethodInfo* testMethod = vm::Method::find_matched_method_in_class_by_name(testClass, "ReturnStruct");
            if (!testMethod)
            {
                RET_ERR(RtErr::MissingMethod);
            }
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, result, vm::Runtime::invoke_with_run_cctor(testMethod, nullptr, nullptr));
            assert(strcmp(result->klass->name, "ReturnStruct"));
            int32_t* fields = reinterpret_cast<int32_t*>(result + 1);
            assert(fields[0] == 1);
            assert(fields[1] == 2);
            assert(fields[2] == 3);
        }
    }
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, testClass, testMod->get_class_by_name("BootstrapTests.TestPassArgument", false, true));
        RET_ERR_ON_FAIL(vm::Class::initialize_all(testClass));
        {
            const metadata::RtMethodInfo* testMethod = vm::Method::find_matched_method_in_class_by_name(testClass, "PassInt");
            if (!testMethod)
            {
                RET_ERR(RtErr::MissingMethod);
            }
            const void* args[1];
            int32_t value = 12345;
            args[0] = &value;
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, result, vm::Runtime::invoke_with_run_cctor(testMethod, nullptr, args));
            assert(result->klass->by_val->ele_type == metadata::RtElementType::I4);
            int32_t int_result = *(int32_t*)(result + 1);
            assert(int_result == value);
        }
        {
            const metadata::RtMethodInfo* testMethod = vm::Method::find_matched_method_in_class_by_name(testClass, "PassString");
            if (!testMethod)
            {
                RET_ERR(RtErr::MissingMethod);
            }
            const void* args[1];
            vm::RtString* value = vm::String::create_string_from_utf8chars("hello", 5);
            args[0] = value;
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, result, vm::Runtime::invoke_with_run_cctor(testMethod, nullptr, args));
            assert(result->klass->by_val->ele_type == metadata::RtElementType::String);
            vm::RtString* s = reinterpret_cast<vm::RtString*>(result);
            assert(s->length == 5);
            Utf16Char* chars = &s->first_char;
            assert(chars[0] == 'h' && chars[1] == 'e' && chars[2] == 'l' && chars[3] == 'l' && chars[4] == 'o');
        }
    }

    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, testClass, testMod->get_class_by_name("BootstrapTests.TestCallMethod", false, true));
        RET_ERR_ON_FAIL(vm::Class::initialize_all(testClass));
        {
            const metadata::RtMethodInfo* testMethod = vm::Method::find_matched_method_in_class_by_name(testClass, "CallSub");
            if (!testMethod)
            {
                RET_ERR(RtErr::MissingMethod);
            }
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, result, vm::Runtime::invoke_with_run_cctor(testMethod, nullptr, nullptr));
            assert(result->klass->by_val->ele_type == metadata::RtElementType::I4);
            int32_t int_result = *(int32_t*)(result + 1);
            assert(int_result == 42);
        }
    }
    RET_VOID_OK();
}

RtResultVoid init_customattributes(metadata::RtModuleDef* mod)
{
    auto& cli_image = mod->get_cli_image();
    for (uint32_t rid = 1, count = cli_image.get_table_row_num(metadata::TableType::CustomAttribute); rid <= count; rid++)
    {
        // std::cout << "Initializing CustomAttribute RID: " << rid << std::endl;
        auto opt_ca = mod->get_custom_attribute_raw_data(rid);
        assert(opt_ca.is_ok());
        auto& data = opt_ca.unwrap();
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, ca_obj, vm::CustomAttribute::read_custom_attribute(mod, &data));
    }
    RET_VOID_OK();
}

static metadata::RtClass* cls_unittest = nullptr;

RtResultVoid init_unittest_class(metadata::RtModuleDef* mod)
{
    std::cout << "Initializing UnitTestAttribute class..." << std::endl;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, mod->get_class_by_name("UnitTestAttribute", false, true));
    RET_ERR_ON_FAIL(vm::Class::initialize_all(klass));
    cls_unittest = klass;
    RET_VOID_OK();
}

static size_t g_failed_test_methods = 0;
static size_t g_passed_test_methods = 0;
static size_t g_skipped_test_methods = 0;
static std::string g_current_phase = "startup";
static std::string g_current_test = "(none)";

static void set_current_test_context(const char* phase, const metadata::RtClass* klass, const metadata::RtMethodInfo* method)
{
    g_current_phase = phase ? phase : "unknown";
    if (klass && method)
    {
        g_current_test = std::string(klass->namespaze) + "." + klass->name + "::" + method->name;
    }
    else
    {
        g_current_test = "(none)";
    }
}

static void print_crash_context(const char* reason)
{
    std::cerr << "\n[FATAL] Process terminated unexpectedly." << std::endl;
    if (reason)
    {
        std::cerr << "[FATAL] Reason: " << reason << std::endl;
    }
    std::cerr << "[FATAL] Phase: " << g_current_phase << std::endl;
    std::cerr << "[FATAL] Current test: " << g_current_test << std::endl;
    std::cerr.flush();
}

static void on_terminate()
{
    print_crash_context("std::terminate");
    std::_Exit(134);
}

static void on_signal(int sig)
{
    print_crash_context("signal");
    std::_Exit(128 + sig);
}

#ifdef _WIN32
static LONG WINAPI windows_unhandled_exception_filter(EXCEPTION_POINTERS* exception_info)
{
    char reason[128] = {0};
    unsigned long code = exception_info && exception_info->ExceptionRecord ? exception_info->ExceptionRecord->ExceptionCode : 0;
    std::snprintf(reason, sizeof(reason), "Windows exception 0x%08lX", code);
    print_crash_context(reason);
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

static void install_crash_handlers()
{
    std::set_terminate(on_terminate);
    std::signal(SIGABRT, on_signal);
    std::signal(SIGSEGV, on_signal);
    std::signal(SIGILL, on_signal);
    std::signal(SIGFPE, on_signal);
#ifdef _WIN32
    SetUnhandledExceptionFilter(windows_unhandled_exception_filter);
#endif
}

RtResultVoid run_tests(metadata::RtModuleDef* mod)
{
    std::cout << "Running UnitTests of: " << mod->get_name_no_ext() << std::endl;
    auto& cli_image = mod->get_cli_image();
    int method_index = 0;
    int skip_count = 0;
    // 1719;
    //  241;
    for (uint32_t rid = 1; rid <= mod->get_class_count(); rid++)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, mod->get_class_by_type_def_rid(rid));
        RET_ERR_ON_FAIL(vm::Class::initialize_all(klass));
        for (uint32_t i = 0; i < klass->method_count; i++)
        {
            const metadata::RtMethodInfo* method = klass->methods[i];
            // if (!method->arg_descs || method->invoker_type != metadata::RtInvokerType::Interpreter || !vm::Method::has_method_body(method))
            // {
            //     continue;
            // }

            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, has_attr, vm::CustomAttribute::has_customattribute_on_method(method, cls_unittest));
            if (!has_attr)
            {
                continue;
            }
            method_index++;
            if (method_index < skip_count)
            {
                ++g_skipped_test_methods;
                // std::cout << "  Skipping..." << std::endl;
                continue;
            }
            set_current_test_context("run_tests", klass, method);
            auto ret1 = vm::Object::new_object(klass);
            if (ret1.is_err())
            {
                std::cout << "  Failed to create test object, error: " << method_index << " run unittest : " << klass->namespaze << "." << klass->name
                          << "::" << method->name << " token: " << method->token << std::endl;
                RET_ERR(ret1.unwrap_err());
            }
            vm::RtObject* test_obj = ret1.unwrap();
            auto ret2 = vm::Runtime::invoke_with_run_cctor(method, test_obj, nullptr);
            if (ret2.is_err())
            {
                std::cout << "  Failed to run unittest, error: " << static_cast<int>(ret2.unwrap_err()) << " " << method_index
                          << " run unittest : " << klass->namespaze << "." << klass->name << "::" << method->name << " token: " << method->token << std::endl;
                ++g_failed_test_methods;
            }
            else
            {
                // std::cout << "  Passed unittest: " << method_index << " run unittest : " << klass->namespaze << "." << klass->name << "::" << method->name
                //           << " token: " << method->token << std::endl;
                ++g_passed_test_methods;
            }
        }
    }
    RET_VOID_OK();
}

int main()
{
    install_crash_handlers();

#ifdef _WIN32
    // 设置 Windows 控制台为 UTF-8 编码
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    std::cout << "Startup test successful!" << std::endl;

    setup_default_lib_dirs();
    vm::Settings::set_file_loader(assembly_file_loader);
    const char* argv[] = {
        "leanclr",
    };
    vm::Settings::set_command_line_arguments(sizeof(argv) / sizeof(const char*), argv);
    auto result = vm::Runtime::initialize();
    if (result.is_err())
    {
        std::cout << "Failed to initialize runtime, error: " << static_cast<int>(result.unwrap_err()) << std::endl;
        return -1;
    }

    bool is_run_all = true;
    bool is_run_bootstrap_tests = is_run_all || false;
    bool is_run_core_tests = is_run_all || false;
    bool is_load_corlib_customattributes = is_run_all || false;
    bool is_run_corlib_tests = is_run_all || false;

    auto corlib = vm::Assembly::get_corlib();
    std::cout << "Corlib assembly loaded successfully." << corlib << std::endl;

    {
        auto ret = initialize_all_classes(corlib);
        if (ret.is_err())
        {
            std::cout << "Failed to initialize all classes, error: " << static_cast<int>(ret.unwrap_err()) << std::endl;
            return -1;
        }
    }
    {
        // auto ret = transform_all_classes(corlib);
        // if (ret.is_err())
        //{
        //     std::cout << "Failed to transform all classes, error: " << static_cast<int>(ret.unwrap_err()) << std::endl;
        //     return -1;
        // }
    }
    metadata::RtAssembly* coreTests = nullptr;
    {
        auto ret = vm::Assembly::load_by_name("CoreTests");
        if (ret.is_err())
        {
            std::cout << "Failed to load CoreTests assembly, error: " << static_cast<int>(ret.unwrap_err()) << std::endl;
            return -1;
        }
        coreTests = ret.unwrap();
        std::cout << "CoreTests assembly loaded successfully." << coreTests << std::endl;
    }

    if (is_run_bootstrap_tests)
    {
        auto ret2 = run_bootstrap_tests(coreTests->mod);
        if (ret2.is_err())
        {
            std::cout << "Failed to run bootstrap tests, error: " << static_cast<int>(ret2.unwrap_err()) << std::endl;
            return -1;
        }
    }
    {
        auto ret3 = init_unittest_class(coreTests->mod);
        if (ret3.is_err())
        {
            std::cout << "Failed to init unittest class, error: " << static_cast<int>(ret3.unwrap_err()) << std::endl;
            return -1;
        }
    }

    if (is_run_core_tests)
    {
        auto ret4 = run_tests(coreTests->mod);
        if (ret4.is_err())
        {
            std::cout << "Failed to run tests, error: " << static_cast<int>(ret4.unwrap_err()) << std::endl;
            return -1;
        }
    }

    if (is_load_corlib_customattributes)
    {
        auto ret3 = init_customattributes(corlib->mod);
        if (ret3.is_err())
        {
            std::cout << "Failed to init custom attributes, error: " << static_cast<int>(ret3.unwrap_err()) << std::endl;
            return -1;
        }
    }

    {
        metadata::RtAssembly* refNestandard = nullptr;
        auto ret = vm::Assembly::load_by_name("RefNetstandard");
        if (ret.is_err())
        {
            std::cout << "Failed to load RefNestandard assembly, error: " << static_cast<int>(ret.unwrap_err()) << std::endl;
            return -1;
        }
        refNestandard = ret.unwrap();
        std::cout << "RefNestandard assembly loaded successfully." << refNestandard << std::endl;

        auto ret2 = refNestandard->mod->get_class_by_name("Test2", false, true);
        if (ret2.is_err())
        {
            std::cout << "Failed to get Test2 class, error: " << static_cast<int>(ret2.unwrap_err()) << std::endl;
            return -1;
        }
        metadata::RtClass* testClass = ret2.unwrap();
        auto ret3 = vm::Class::initialize_all(testClass);
        if (ret3.is_err())
        {
            std::cout << "Failed to initialize Test2 class, error: " << static_cast<int>(ret3.unwrap_err()) << std::endl;
            return -1;
        }
    }

    metadata::RtAssembly* corelibTests = nullptr;
    {
        auto ret1 = vm::Assembly::load_by_name("CorlibTests");
        if (ret1.is_err())
        {
            std::cout << "Failed to load CorelibTests assembly, error: " << static_cast<int>(ret1.unwrap_err()) << std::endl;
            return -1;
        }
        corelibTests = ret1.unwrap();
        std::cout << "CorlibTests assembly loaded successfully." << corelibTests << std::endl;
    }
    if (is_run_corlib_tests)
    {
        auto ret2 = run_tests(corelibTests->mod);
        if (ret2.is_err())
        {
            std::cout << "Failed to run tests, error: " << static_cast<int>(ret2.unwrap_err()) << std::endl;
            return -1;
        }
    }

    std::cout << std::endl;
    std::cout << "Total test methods: " << (g_passed_test_methods + g_failed_test_methods + g_skipped_test_methods) << std::endl;
    std::cout << "  Passed: " << g_passed_test_methods << std::endl;
    std::cout << "  Failed: " << g_failed_test_methods << std::endl;
    std::cout << "  Skipped: " << g_skipped_test_methods << std::endl;
    std::cout << "" << std::endl;

    return 0;
}
