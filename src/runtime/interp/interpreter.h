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
};
} // namespace interp
} // namespace leanclr
