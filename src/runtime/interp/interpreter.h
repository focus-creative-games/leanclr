#pragma once

#include "interp_defs.h"
#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace interp
{
class Interpreter
{
  public:
    // Execute method by method info and parameters
    static RtResult<const RtInterpMethodInfo*> init_interpreter_method(const metadata::RtMethodInfo* method);
    static RtResult<const interp::RtStackObject*> execute(const metadata::RtMethodInfo* method, const interp::RtStackObject* params);
    // Snapshot contains arguments then locals in interpreter stack-slot order.
    // Resume exception dispatch at the faulting IL instruction; never replay it.
    static RtResult<const RtStackObject*> resume_exception(const metadata::RtMethodInfo* method, const RtStackObject* snapshot, size_t slots,
                                                           uint32_t il_offset, core::RtErr error, bool explicit_throw = false);

  private:
    static RtResult<const RtStackObject*> execute_impl(const metadata::RtMethodInfo* method, const RtStackObject* params, size_t snapshot_slots,
                                                       uint32_t il_offset, core::RtErr error, bool explicit_throw);
};
} // namespace interp
} // namespace leanclr
