#include "environment.h"
#include "rt_string.h"
#include "class.h"
#include "rt_array.h"
#include "rt_array.h"
#include "utils/hashmap.h"
#include "utils/string_util.h"
#include "utils/string_builder.h"

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
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, args_array, Array::new_szarray_from_ele_klass(string_class, argc));

    for (int32_t i = 0; i < argc; ++i)
    {
        RtString* arg_str = String::create_string_from_utf8cstr(argv[i]);
        Array::set_array_data_at<RtString*>(args_array, i, arg_str);
    }

    g_cmdline_args = args_array;
    RET_VOID_OK();
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
    utils::StringBuilder sb;
    utils::StringUtil::utf16_to_utf8(variable_name, static_cast<size_t>(variable_name_length), sb);
    const char* key = sb.as_cstr();
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
            utils::StringBuilder val_sb;
            utils::StringUtil::utf16_to_utf8(value, static_cast<size_t>(value_length), val_sb);
            RtString* val_str = String::create_string_from_utf8chars(val_sb.as_cstr(), static_cast<int32_t>(val_sb.length()));
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
        utils::StringBuilder val_sb;
        utils::StringUtil::utf16_to_utf8(value, static_cast<size_t>(value_length), val_sb);
        const char* new_key = utils::StringUtil::strdup(key);
        RtString* val_str = String::create_string_from_utf8chars(val_sb.as_cstr(), static_cast<int32_t>(val_sb.length()));
        s_environment_variables_map.insert({new_key, val_str});
        RET_VOID_OK();
    }
}

RtResult<RtArray*> Environment::get_environment_variable_names()
{
    metadata::RtClass* string_class = vm::Class::get_corlib_types().cls_string;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, names_array,
                                            Array::new_szarray_from_ele_klass(string_class, static_cast<int32_t>(s_environment_variables_map.size())));
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
