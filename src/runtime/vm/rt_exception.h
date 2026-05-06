#pragma once

#include "rt_managed_types.h"
#include "utils/string_builder.h"

namespace leanclr
{
namespace interp
{
struct InterpFrame;
}

namespace vm
{
struct AotExceptionWrapper
{
    RtException* exception;
};

class Exception
{
  public:
    static RtResultVoid initialize();
    static void set_current_exception(RtException* ex);
    static RtException* get_and_clear_current_exception();
    static RtException* raise_error_as_exception(RtErr err, interp::InterpFrame* frame, const void* ip);
    static RtException* raise_aot_error_as_exception(RtErr err, const metadata::RtMethodInfo* methodInfo, int32_t ip);
    static RtException* raise_aot_exception(RtException* ex, const metadata::RtMethodInfo* methodInfo, int32_t ip);
    static RtErr raise_internal_runtime_error_as_exception(RtErr err, const char* message);
    static RtException* raise_exception(RtException* ex, interp::InterpFrame* frame, const void* ip);
    static RtException* raise_internal_runtime_exception(metadata::RtClass* ex_klass, const char* message);
    [[noreturn]] static void raise_as_cpp_exception(RtException* ex);

    static RtResultVoid report_unhandled_exception(RtException* exception);

    static void format_exception(RtException* ex, utils::StringBuilder& sb);

    // Additional exception handling functions
    // static RtResult<RtException*> raise_native_error_exception_with_message(RtErr err, const uint8_t* msg, size_t msg_len);
    // static metadata::RtClass* get_exception_klass_of_runtime_error(RtErr err);
    // static RtException* raise_runtime_error_as_exception(RtErr err, interp::InterpFrame* frame, const void* ip);
    // static RtException* raise_runtime_exception(metadata::RtClass* ex_class, const uint8_t* message, size_t message_len);
    // static RtException* not_raise_runtime_error_as_exception(RtErr err);
    // static metadata::RtClass* get_class_invalid_cast_exception();
    // static RtException* new_exception(metadata::RtClass* ex_class, RtException* inner_exception, RtString* message);
    // static RtResultVoid raise_null_reference_exception(interp::InterpFrame* frame, const void* ip);
};

#define RET_ERR_WITH_MSG(err, msg)                                                   \
    do                                                                               \
    {                                                                                \
        RET_ERR(vm::Exception::raise_internal_runtime_error_as_exception(err, msg)); \
    } while (0)

} // namespace vm
} // namespace leanclr
