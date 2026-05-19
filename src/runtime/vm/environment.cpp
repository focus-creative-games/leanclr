#include "environment.h"
#include "rt_string.h"
#include "class.h"
#include "rt_array.h"
#include "rt_array.h"
#include "utils/hashmap.h"
#include "utils/string_util.h"
#include "utils/string_builder.h"
#include "gc/roots/gc_roots.h"

namespace leanclr
{
namespace vm
{

static int32_t s_exit_code = 0;
static bool s_shutdown = false;

static RtArray* g_cmdline_args = nullptr;

static utils::HashMap<const char*, RtString*, utils::CStrHasher, utils::CStrCompare> s_environment_variables_map;

void Environment::exit(int32_t code)
{
    s_exit_code = code;
    assert(false && "Environment::exit not implemented for this platform");
}

int32_t Environment::get_exit_code()
{
    return s_exit_code;
}

void Environment::set_exit_code(int32_t code)
{
    s_exit_code = code;
}

void Environment::shutdown()
{
    s_shutdown = true;
}

bool Environment::has_shutdown_started()
{
    return s_shutdown;
}

RtResult<RtString*> Environment::get_machine_name()
{
    RET_OK(String::create_string_from_utf8cstr("leanclr-machine"));
}

RtResult<RtString*> Environment::get_new_line()
{
#ifdef LEANCLR_PLATFORM_WIN
    RET_OK(String::create_string_from_utf8cstr("\r\n"));
#else
    RET_OK(String::create_string_from_utf8cstr("\n"));
#endif
}

RtResult<vm::RtString*> Environment::get_os_version_string()
{
    RET_OK(vm::String::create_string_from_utf8cstr("leanclr-os"));
}

RtResult<vm::RtString*> Environment::get_user_name()
{
    RET_OK(vm::String::create_string_from_utf8cstr("leanclr-user"));
}

Platform Environment::get_platform()
{
#ifdef LEANCLR_PLATFORM_WIN
    return Platform::Win32NT;
#elif defined(LEANCLR_PLATFORM_MAC)
    return Platform::MacOSX;
#elif defined(LEANCLR_PLATFORM_LINUX) || defined(LEANCLR_PLATFORM_ANDROID) || defined(LEANCLR_PLATFORM_IOS) || defined(LEANCLR_PLATFORM_WASM) || \
    defined(LEANCLR_PLATFORM_POSIX)
    return Platform::Unix;
#else
    return Platform::Unix;
#endif
}

RtArray* Environment::get_command_line_args()
{
    return g_cmdline_args;
}

RtResultVoid Environment::init_cmdline_args(const char** argv, int32_t argc)
{
    assert(!g_cmdline_args && "Command line arguments have already been initialized");
    // Create string array for command line arguments
    metadata::RtClass* string_class = vm::Class::get_corlib_types().cls_string;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, args_array, LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERNAL(string_class, argc, "Environment::init_cmdline_args"));

    for (int32_t i = 0; i < argc; ++i)
    {
        RtString* arg_str = String::create_string_from_utf8cstr(argv[i]);
        Array::set_array_data_at<RtString*>(args_array, i, arg_str);
    }

    g_cmdline_args = args_array;
    RET_VOID_OK();
}

static void visit_environment_roots(gc::GcVisitObjectRoot visit, void* userdata)
{
    if (g_cmdline_args != nullptr)
    {
        visit(reinterpret_cast<vm::RtObject*>(g_cmdline_args), userdata);
    }
    for (utils::HashMap<const char*, RtString*, utils::CStrHasher, utils::CStrCompare>::const_iterator it = s_environment_variables_map.begin();
         it != s_environment_variables_map.end(); ++it)
    {
        if (it->second != nullptr)
        {
            visit(reinterpret_cast<vm::RtObject*>(it->second), userdata);
        }
    }
}

void register_environment_gc_roots()
{
    gc::GcRoots::register_slot(reinterpret_cast<vm::RtObject**>(&g_cmdline_args));
    gc::GcRoots::register_visit_object_roots(visit_environment_roots);
}

RtResult<RtString*> Environment::get_environment_variable(const char* variable_name)
{
    auto it = s_environment_variables_map.find(variable_name);
    if (it != s_environment_variables_map.end())
    {
        RET_OK(it->second);
    }
    RET_OK(nullptr);
}

RtResultVoid Environment::set_environment_variable(const Utf16Char* variable_name, int32_t variable_name_length, const Utf16Char* value, int32_t value_length)
{
    utils::Utf8StringBuilder sb;
    sb.append_utf16_str(variable_name, static_cast<size_t>(variable_name_length));
    const char* key = sb.get_const_chars();
    auto it = s_environment_variables_map.find(key);
    if (it != s_environment_variables_map.end())
    {
        if (value == nullptr)
        {
            s_environment_variables_map.erase(it);
            RET_VOID_OK();
        }
        else
        {
            utils::Utf8StringBuilder val_sb;
            val_sb.append_utf16_str(value, static_cast<size_t>(value_length));
            RtString* val_str = String::create_string_from_utf8chars(val_sb.get_const_chars(), static_cast<int32_t>(val_sb.length()));
            it->second = val_str;
            RET_VOID_OK();
        }
    }
    else
    {
        if (value == nullptr)
        {
            RET_VOID_OK();
        }
        utils::Utf8StringBuilder val_sb;
        val_sb.append_utf16_str(value, static_cast<size_t>(value_length));
        const char* new_key = sb.dup_zero_terminated_chars();
        RtString* val_str = String::create_string_from_utf8chars(val_sb.get_const_chars(), static_cast<int32_t>(val_sb.length()));
        s_environment_variables_map.insert({new_key, val_str});
        RET_VOID_OK();
    }
}

RtResult<RtArray*> Environment::get_environment_variable_names()
{
    metadata::RtClass* string_class = vm::Class::get_corlib_types().cls_string;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, names_array,
                                            LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERNAL(string_class, static_cast<int32_t>(s_environment_variables_map.size()), "Environment::get_environment_variable_names"));
    size_t index = 0;
    for (utils::HashMap<const char*, RtString*, utils::CStrHasher, utils::CStrCompare>::const_iterator it = s_environment_variables_map.begin();
         it != s_environment_variables_map.end(); ++it)
    {
        const char* key = it->first;
        RtString* name_str = String::create_string_from_utf8cstr(key);
        Array::set_array_data_at<RtString*>(names_array, static_cast<int32_t>(index), name_str);
        ++index;
    }
    RET_OK(names_array);
}

RtResult<vm::RtString*> Environment::get_windows_folder_path(int32_t)
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}

RtResult<vm::RtArray*> Environment::get_logical_drives()
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}

RtResult<vm::RtString*> Environment::get_machine_config_path()
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}

RtResult<vm::RtString*> Environment::get_home()
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}

RtResult<vm::RtString*> Environment::get_bundled_machine_config()
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}

void Environment::fail_fast()
{
    int32_t* p = nullptr;
    *p = 0;
}

int32_t Environment::get_processor_count()
{
    return 1;
}

int32_t Environment::get_page_size()
{
    return 4096;
}

} // namespace vm
} // namespace leanclr
