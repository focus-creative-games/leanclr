#include "bcl/coreclr-net10/host_result.h"
#include "utils/scope_guard.h"
#include "vm/environment.h"
#include "vm/rt_string.h"
#include "vm/class.h"
#include "vm/rt_array.h"
#include "vm/rt_array.h"
#include "utils/hashmap.h"
#include "utils/string_util.h"
#include "utils/string_builder.h"
#include "gc/gc_roots.h"

namespace leanclr
{
namespace vm
{

static int32_t s_exit_code = 0;
static bool s_shutdown = false;

static RtArray* g_cmdline_args = nullptr;

RtResultVoid Environment::exit(int32_t code)
{
    auto callback = host::Services::extended().exit_process;
    if (!callback)
        return RtErr::NotSupported;
    s_exit_code = code;
    shutdown();
    callback(code);
    // A misbehaving host must not let managed code resume after Exit.
    return RtErr::InvalidOperation;
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

static RtResult<RtString*> query_host_string(host::StringQuery query, const char* name = nullptr)
{
    const char* value = nullptr;
    int32_t error = host::Services::query_string(query, name, &value);
    if (error)
        return bcl::host_error(error);
    if (!value)
        return static_cast<RtString*>(nullptr);
    return String::create_string_from_utf8cstr(value);
}

RtResult<RtString*> Environment::get_machine_name()
{
    return query_host_string(host::StringQuery::MachineName);
}

RtResult<RtString*> Environment::get_new_line()
{
    return query_host_string(host::StringQuery::NewLine);
}

RtResult<RtString*> Environment::get_os_version_string()
{
    return query_host_string(host::StringQuery::OsVersion);
}

RtResult<RtString*> Environment::get_user_name()
{
    return query_host_string(host::StringQuery::UserName);
}

RtResult<Platform> Environment::get_platform()
{
    int64_t value;
    int32_t error = host::Services::query_integer(host::IntegerQuery::PlatformId, &value);
    if (error)
        return bcl::host_error(error);
    if (value < 0 || value > 6)
        return RtErr::InvalidOperation;
    return static_cast<Platform>(value);
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
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, args_array,
                                            LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERNAL(string_class, argc, "Environment::init_cmdline_args"));

    for (int32_t i = 0; i < argc; ++i)
    {
        RtString* arg_str = String::create_string_from_utf8cstr(argv[i]);
        Array::set_array_data_at<RtString*>(args_array, i, arg_str);
    }

    g_cmdline_args = args_array;
    RET_VOID_OK();
}

void register_environment_gc_roots()
{
    gc::GcRoots::register_slot(reinterpret_cast<vm::RtObject**>(&g_cmdline_args));
}

RtResult<RtString*> Environment::get_environment_variable(const char* name)
{
    if (!name)
        return RtErr::ArgumentNull;
    return query_host_string(host::StringQuery::EnvironmentVariable, name);
}

RtResultVoid Environment::set_environment_variable(const Utf16Char* name, int32_t name_length, const Utf16Char* value, int32_t value_length)
{
    if (!name)
        return RtErr::ArgumentNull;
    if (name_length <= 0 || value_length < 0)
        return RtErr::Argument;
    auto cb = host::Services::extended().set_environment;
    if (!cb)
        return RtErr::NotSupported;
    utils::Utf8StringBuilder n, v;
    n.append_utf16_str(name, static_cast<size_t>(name_length));
    if (value)
        v.append_utf16_str(value, static_cast<size_t>(value_length));
    int32_t error = cb(n.get_const_chars(), value ? v.get_const_chars() : nullptr);
    if (error)
        return bcl::host_error(error);
    RET_VOID_OK();
}

RtResult<RtArray*> Environment::get_environment_variable_names()
{
    auto cb = host::Services::extended().environment_names;
    if (!cb)
        return RtErr::NotSupported;

    struct Names
    {
        utils::Vector<const char*> values;
        bool failed = false;
    } names;

    auto cleanup = utils::make_scope_exit(
        [&]()
        {
            for (auto p : names.values)
                alloc::GeneralAllocation::free(const_cast<char*>(p));
        });
    int32_t error = cb(
        [](const char* name, void* context)
        {
            auto& names = *static_cast<Names*>(context);
            if (names.failed || !name)
                return;
            auto copy = utils::StringUtil::try_strdup(name);
            if (copy.is_err())
            {
                names.failed = true;
                return;
            }
            const char* p = copy.unwrap();
            if (!names.values.try_push_back(p))
            {
                alloc::GeneralAllocation::free(const_cast<char*>(p));
                names.failed = true;
            }
        },
        &names);
    if (error)
        return bcl::host_error(error);
    if (names.failed || names.values.size() > INT32_MAX)
        return RtErr::OutOfMemory;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(
        RtArray*, result,
        LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERNAL(Class::get_corlib_types().cls_string, static_cast<int32_t>(names.values.size()), "Environment::names"));
    for (size_t i = 0; i < names.values.size(); ++i)
        Array::set_array_data_at<RtString*>(result, i, String::create_string_from_utf8cstr(names.values[i]));
    return result;
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

RtResult<RtString*> Environment::get_home()
{
    return query_host_string(host::StringQuery::Home);
}

RtResult<vm::RtString*> Environment::get_bundled_machine_config()
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}

void Environment::fail_fast()
{
    leanclr::panic("Managed Environment.FailFast");
}

RtResult<int32_t> Environment::get_processor_count()
{
    int64_t value;
    int32_t error = host::Services::query_integer(host::IntegerQuery::ProcessorCount, &value);
    if (error)
        return bcl::host_error(error);
    if (value <= 0 || value > INT32_MAX)
        return RtErr::InvalidOperation;
    return static_cast<int32_t>(value);
}

RtResult<int32_t> Environment::get_page_size()
{
    int64_t value;
    int32_t error = host::Services::query_integer(host::IntegerQuery::PageSize, &value);
    if (error)
        return bcl::host_error(error);
    if (value <= 0 || value > INT32_MAX)
        return RtErr::InvalidOperation;
    return static_cast<int32_t>(value);
}

} // namespace vm
} // namespace leanclr
