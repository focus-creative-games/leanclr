#include "settings.h"
#include "utils/string_builder.h"

namespace leanclr
{
namespace vm
{

static const char* g_domain_name = nullptr;
static const char* g_config_dir = nullptr;
static const char* g_data_dir = nullptr;
static const char* g_temp_dir = nullptr;
static const char* g_config = nullptr;

static const char* g_commandline_arguments[1024] = {nullptr};
static int32_t g_commandline_arguments_count = 0;

static void default_debugger_log_function(int32_t level, const uint16_t* category, size_t category_len, const uint16_t* message, size_t message_len);

static FileLoader g_file_loader = nullptr;
static InternalFunctionInitializer g_internal_functions_initializer = nullptr;

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
    g_debugger_log_buffer.append_u32(static_cast<uint32_t>(level));
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

const char* Settings::get_config_dir()
{
    return g_config_dir;
}

void Settings::set_config_dir(const char* config_dir)
{
    assert(config_dir != nullptr);
    assert(g_config_dir == nullptr);
    g_config_dir = utils::StringUtil::strdup(config_dir);
}

const char* Settings::get_data_dir()
{
    return g_data_dir;
}

void Settings::set_data_dir(const char* data_dir)
{
    assert(data_dir != nullptr);
    assert(g_data_dir == nullptr);
    g_data_dir = utils::StringUtil::strdup(data_dir);
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

void Settings::set_config(const char* executablePath)
{
    assert(executablePath != nullptr);
    assert(g_config == nullptr);
    g_config = utils::StringUtil::strdup(executablePath);
}

const char* Settings::get_config()
{
    return g_config;
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
    assert(argv != nullptr);
    g_commandline_arguments_count = argc;
    for (int32_t i = 0; i < argc; ++i)
    {
        g_commandline_arguments[i] = utils::StringUtil::strdup(argv[i]);
    }
}

void Settings::set_command_line_arguments_utf16(int32_t argc, const Utf16Char** argv)
{
    assert(argv != nullptr);
    g_commandline_arguments_count = argc;
    utils::StringBuilder sb;
    for (int32_t i = 0; i < argc; ++i)
    {
        sb.clear();
        sb.append_utf16_str(argv[i], static_cast<size_t>(utils::StringUtil::get_utf16chars_length(argv[i])));
        g_commandline_arguments[i] = sb.dup_to_zero_end_cstr();
    }
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

} // namespace vm
} // namespace leanclr
