#include "settings.h"
#include "utils/string_builder.h"

namespace leanclr
{
namespace vm
{
static void default_debugger_log_function(int32_t level, const uint16_t* category, size_t category_len, const uint16_t* message, size_t message_len);

static FileLoader g_file_loader = nullptr;
static InternalFunctionInitializer g_internal_functions_initializer = nullptr;
static int32_t g_cmd_argc = 0;
static const char** g_cmd_argv = nullptr;

static size_t g_default_eval_stack_object_count = 1024 * 128;
static size_t g_default_frame_stack_size = 1024 * 2;

static DebuggerLogFunc g_debugger_log_function = default_debugger_log_function;

static ReportUnhandledExceptionFunc g_report_unhandled_exception_function = nullptr;

static const metadata::RtAotModulesData* g_aot_modules_data = nullptr;

static utils::StringBuilder g_debugger_log_buffer;

static void default_debugger_log_function(int32_t level, const uint16_t* category, size_t category_len, const uint16_t* message, size_t message_len)
{
    g_debugger_log_buffer.clear();
    g_debugger_log_buffer.append_char('[');
    g_debugger_log_buffer.append_u32(level);
    g_debugger_log_buffer.append_cstr("] ");
    if (category && category_len > 0)
    {
        g_debugger_log_buffer.append_utf16_str(category, category_len);
        g_debugger_log_buffer.append_cstr(" - ");
    }
    if (message && message_len > 0)
    {
        g_debugger_log_buffer.append_utf16_str(message, message_len);
    }
    // g_debugger_log_buffer.sure_null_terminator_but_not_append();

    // Output to standard output (could be replaced with other logging mechanisms)
    printf("%s\n", g_debugger_log_buffer.as_cstr());
}

FileLoader Settings::get_file_loader()
{
    return g_file_loader;
}

void Settings::set_file_loader(FileLoader loader)
{
    g_file_loader = loader;
}

void Settings::set_internal_functions_initializer(InternalFunctionInitializer initializer)
{
    g_internal_functions_initializer = initializer;
}

InternalFunctionInitializer Settings::get_internal_functions_initializer()
{
    return g_internal_functions_initializer;
}

void Settings::set_debugger_log_function(DebuggerLogFunc log_func)
{
    g_debugger_log_function = log_func;
}

DebuggerLogFunc Settings::get_debugger_log_function()
{
    return g_debugger_log_function;
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

void Settings::set_command_line_arguments(int32_t argc, const char** argv)
{
    g_cmd_argc = argc;
    g_cmd_argv = argv;
}

void Settings::get_command_line_arguments(int32_t& argc, const char**& argv)
{
    argc = g_cmd_argc;
    argv = g_cmd_argv;
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

} // namespace vm
} // namespace leanclr
