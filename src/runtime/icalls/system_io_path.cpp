#include "system_io_path.h"

#include "platform/rt_path.h"

namespace leanclr
{
namespace icalls
{

/// @icall: System.IO.Path::get_temp_path
RtResult<vm::RtString*> SystemIOPath::get_temp_path() noexcept
{
    RET_OK(os::Path::get_temp_path());
}

static RtResultVoid get_temp_path_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                          interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, s, SystemIOPath::get_temp_path());
    EvalStackOp::set_return(ret, s);
    RET_VOID_OK();
}

static vm::InternalCallEntry s_internal_call_entries_system_io_path[] = {
    {"System.IO.Path::get_temp_path", (vm::InternalCallFunction)&SystemIOPath::get_temp_path, get_temp_path_invoker},
};

utils::Span<vm::InternalCallEntry> SystemIOPath::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_io_path,
                                              sizeof(s_internal_call_entries_system_io_path) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr
