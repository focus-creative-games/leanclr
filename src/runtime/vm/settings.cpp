#include "settings.h"
#include "alloc/scoped_buffer.h"
#include "utils/scope_guard.h"
#include "utils/encode_conv.h"
#include "utils/string_util.h"

namespace leanclr
{
namespace vm
{

static const char* g_domain_name = nullptr;
static const char* g_temp_dir = nullptr;

static const char* g_commandline_arguments[1024] = {nullptr};
static int32_t g_commandline_arguments_count = 0;

static InternalFunctionInitializer g_internal_functions_initializer = nullptr;

static size_t g_default_eval_stack_object_count = 1024 * 128;
static size_t g_default_frame_stack_size = 1024 * 2;

static ReportUnhandledExceptionFunc g_report_unhandled_exception_function = nullptr;

static const metadata::RtAotModulesData* g_aot_modules_data = nullptr;

static gc::GCMode g_gc_mode = gc::GCMode::ENABLED;

const char* Settings::get_domain_name()
{
    return g_domain_name;
}

void Settings::set_domain_name(const char* domain_name)
{
    assert(domain_name != nullptr);
    assert(g_domain_name == nullptr);
    g_domain_name = utils::StringUtil::strdup(domain_name);
}

const char* Settings::get_temp_dir()
{
    return g_temp_dir;
}

void Settings::set_temp_dir(const char* temp_dir)
{
    assert(temp_dir != nullptr);
    assert(g_temp_dir == nullptr);
    g_temp_dir = utils::StringUtil::strdup(temp_dir);
}

void Settings::set_internal_functions_initializer(InternalFunctionInitializer initializer)
{
    g_internal_functions_initializer = initializer;
}

InternalFunctionInitializer Settings::get_internal_functions_initializer()
{
    return g_internal_functions_initializer;
}

void Settings::set_report_unhandled_exception_function(ReportUnhandledExceptionFunc func)
{
    g_report_unhandled_exception_function = func;
}

ReportUnhandledExceptionFunc Settings::get_report_unhandled_exception_function()
{
    return g_report_unhandled_exception_function;
}

const metadata::RtAotModulesData* Settings::get_aot_modules_data()
{
    return g_aot_modules_data;
}

void Settings::set_aot_modules_data(const metadata::RtAotModulesData* data)
{
    g_aot_modules_data = data;
}

namespace
{
RtResult<const char*> copy_argument(const char* value)
{
    return utils::StringUtil::try_strdup(value);
}

RtResult<const char*> copy_argument(const Utf16Char* value)
{
    size_t length = 0;
    while (value[length])
    {
        const Utf16Char ch = value[length++];
        if (ch >= 0xd800 && ch <= 0xdbff)
        {
            const Utf16Char low = value[length];
            if (low < 0xdc00 || low > 0xdfff)
                return RtErr::Argument;
            ++length;
        }
        else if (ch >= 0xdc00 && ch <= 0xdfff)
            return RtErr::Argument;
    }
    size_t bytes;
    if (!utils::CheckedSize::multiply(length, 3, bytes) || !utils::CheckedSize::add(bytes, 1, bytes))
        return RtErr::OutOfMemory;
    auto* copy = static_cast<char*>(alloc::GeneralAllocation::malloc(bytes));
    if (!copy)
        return RtErr::OutOfMemory;
    size_t written;
    utils::EncodeConv::utf16_to_utf8(value, length, copy, written);
    return static_cast<const char*>(copy);
}

template <typename Char>
RtResultVoid replace_arguments(int32_t argc, const Char** argv)
{
    if (argc < 0 || argc > 1024)
        return RtErr::ArgumentOutOfRange;
    if (argc != 0 && !argv)
        return RtErr::ArgumentNull;
    for (int32_t i = 0; i < argc; ++i)
        if (!argv[i])
            return RtErr::ArgumentNull;
    alloc::ScopedBuffer<const char*, 16> staged;
    if (!staged.try_allocate_zeroed(static_cast<size_t>(argc)))
        return RtErr::OutOfMemory;
    auto cleanup = utils::make_scope_exit(
        [&]()
        {
            for (int32_t i = 0; i < argc; ++i)
                alloc::GeneralAllocation::free(const_cast<char*>(staged.data()[i]));
        });
    for (int32_t i = 0; i < argc; ++i)
        UNWRAP_OR_RET_ERR_ON_FAIL(staged.data()[i], copy_argument(argv[i]));
    for (int32_t i = 0; i < g_commandline_arguments_count; ++i)
    {
        alloc::GeneralAllocation::free(const_cast<char*>(g_commandline_arguments[i]));
        g_commandline_arguments[i] = nullptr;
    }
    for (int32_t i = 0; i < argc; ++i)
        g_commandline_arguments[i] = staged.data()[i];
    g_commandline_arguments_count = argc;
    cleanup.dismiss();
    RET_VOID_OK();
}
} // namespace

RtResultVoid Settings::try_set_command_line_arguments(int32_t argc, const char** argv)
{
    return replace_arguments(argc, argv);
}

RtResultVoid Settings::try_set_command_line_arguments_utf16(int32_t argc, const Utf16Char** argv)
{
    return replace_arguments(argc, argv);
}

void Settings::set_command_line_arguments(int32_t argc, const char** argv)
{
    if (try_set_command_line_arguments(argc, argv).is_err())
        leanclr::panic("Command line argument configuration failed");
}

void Settings::set_command_line_arguments_utf16(int32_t argc, const Utf16Char** argv)
{
    if (try_set_command_line_arguments_utf16(argc, argv).is_err())
        leanclr::panic("Command line argument configuration failed");
}

void Settings::get_command_line_arguments(int32_t& argc, const char**& argv)
{
    argc = g_commandline_arguments_count;
    argv = g_commandline_arguments;
}

size_t Settings::get_default_eval_stack_object_count()
{
    return g_default_eval_stack_object_count;
}

void Settings::set_default_eval_stack_object_count(size_t count)
{
    g_default_eval_stack_object_count = count;
}

size_t Settings::get_default_frame_stack_size()
{
    return g_default_frame_stack_size;
}

void Settings::set_default_frame_stack_size(size_t size)
{
    g_default_frame_stack_size = size;
}

void Settings::set_gc_mode(gc::GCMode mode)
{
    g_gc_mode = mode;
}

gc::GCMode Settings::get_gc_mode()
{
    return g_gc_mode;
}
} // namespace vm
} // namespace leanclr
