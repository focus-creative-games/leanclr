#pragma once

#include "core/rt_base.h"
#include "host/services.h"
#include "utils/rt_span.h"
#include "rt_managed_types.h"
#include "gc/gc_common.h"

namespace leanclr
{
namespace vm
{

using FileData = host::FileData;
using FileLoader = host::FileLoader;
using InternalFunctionInitializer = void (*)();
using DebuggerLogFunc = host::DebuggerLog;

typedef void (*ReportUnhandledExceptionFunc)(RtException* exception);

class Settings
{
  public:
    static const char* get_domain_name();
    static void set_domain_name(const char* domain_name);

    static void set_config_dir(const char* config_dir);
    static const char* get_config_dir();
    static void set_data_dir(const char* data_dir);
    static const char* get_data_dir();
    static void set_temp_dir(const char* temp_dir);
    static const char* get_temp_dir();
    static void set_config(const char* executablePath);
    static const char* get_config();

    // Startup configuration on the runtime thread. At most 1024 non-null arguments.
    // Failure preserves the previous owned argument snapshot; argc == 0 clears it.
    static RtResultVoid try_set_command_line_arguments(int32_t argc, const char** argv);
    static RtResultVoid try_set_command_line_arguments_utf16(int32_t argc, const Utf16Char** argv);

    // Legacy setters use fatal policy on invalid input or allocation failure.
    static void set_command_line_arguments_utf16(int32_t argc, const Utf16Char** argv);

    static FileLoader get_file_loader();
    static void set_file_loader(FileLoader loader);

    static void get_command_line_arguments(int32_t& argc, const char**& argv);
    static void set_command_line_arguments(int32_t argc, const char** argv);

    static size_t get_default_eval_stack_object_count();
    static void set_default_eval_stack_object_count(size_t count);

    static size_t get_default_frame_stack_size();
    static void set_default_frame_stack_size(size_t size);
#if LEANCLR_BCL_CORECLR_NET10
    // Configure on the runtime thread. Existing canonical identities remain valid
    // when the limit is reached; new identities return OutOfMemory.
    static size_t get_max_runtime_type_handles();
    static void set_max_runtime_type_handles(size_t count);
#endif

    static InternalFunctionInitializer get_internal_functions_initializer();
    static void set_internal_functions_initializer(InternalFunctionInitializer initializer);

    static DebuggerLogFunc get_debugger_log_function();
    static void set_debugger_log_function(DebuggerLogFunc log_func);

    static ReportUnhandledExceptionFunc get_report_unhandled_exception_function();
    static void set_report_unhandled_exception_function(ReportUnhandledExceptionFunc func);

    static const metadata::RtAotModulesData* get_aot_modules_data();
    static void set_aot_modules_data(const metadata::RtAotModulesData* modules_data);

    static void set_gc_mode(gc::GCMode mode);
    static gc::GCMode get_gc_mode();
};
} // namespace vm
} // namespace leanclr
