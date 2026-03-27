#pragma once

#include "core/stl_compat.h"

#include "rt_managed_types.h"
#include "utils/hashmap.h"
#include "utils/rt_vector.h"
#include "utils/string_util.h"
#include "utils/rt_span.h"
#include "rt_exception.h"
#include "interp/eval_stack_op.h"

namespace leanclr
{
namespace vm
{

using PInvokeFunction = metadata::RtManagedMethodPointer;
using PInvokeInvoker = metadata::RtInvokeMethodPointer;

// Registry struct for internal call functions
struct PInvokeRegistry
{
    PInvokeFunction func;
    PInvokeInvoker invoker;
};

struct PInvokeEntry
{
    const char* name;
    PInvokeFunction func;
    PInvokeInvoker invoker;
};

class PInvokes
{
  public:
    // Initialize internal calls
    static void initialize();

    // Register/get internal call functions
    static void register_pinvoke(const char* name, PInvokeFunction func, PInvokeInvoker invoker);
    static const PInvokeRegistry* get_pinvoke(const char* name);
    static RtResult<const PInvokeRegistry*> get_pinvoke_by_method(const metadata::RtMethodInfo* method);
};
} // namespace vm
} // namespace leanclr
