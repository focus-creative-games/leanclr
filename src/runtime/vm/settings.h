#pragma once

#include "rt_base.h"
#include "utils/rt_span.h"
#include "rt_managed_types.h"

namespace leanclr
{
namespace vm
{

struct FileData
{
    const uint8_t* data;
    size_t length;
};

typedef RtResult<FileData> (*FileLoader)(const char* assembly_name, const char* extension);

typedef void (*InternalFunctionInitializer)();
typedef void (*DebuggerLogFunc)(int32_t level, const uint16_t* category, size_t category_len, const uint16_t* message, size_t message_len);
typedef void (*ReportUnhandledExceptionFunc)(RtException* exception);

class Settings
{
  public:
    static FileLoader get_file_loader();
    static void set_file_loader(FileLoader loader);

    static void get_command_line_arguments(int32_t& argc, const char**& argv);
    static void set_command_line_arguments(int32_t argc, const char** argv);

    static size_t get_default_eval_stack_object_count();
    static void set_default_eval_stack_object_count(size_t count);

    static size_t get_default_frame_stack_size();
    static void set_default_frame_stack_size(size_t size);

    static InternalFunctionInitializer get_internal_functions_initializer();
    static void set_internal_functions_initializer(InternalFunctionInitializer initializer);

    static DebuggerLogFunc get_debugger_log_function();
    static void set_debugger_log_function(DebuggerLogFunc log_func);

    static ReportUnhandledExceptionFunc get_report_unhandled_exception_function();
    static void set_report_unhandled_exception_function(ReportUnhandledExceptionFunc func);

    static const metadata::RtAotModulesData* get_aot_modules_data();
    static void set_aot_modules_data(const metadata::RtAotModulesData* modules_data);
};
} // namespace vm
} // namespace leanclr
