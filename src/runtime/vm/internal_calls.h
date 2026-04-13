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

using InternalCallFunction = metadata::RtManagedMethodPointer;
using InternalCallInvoker = metadata::RtInvokeMethodPointer;
using Il2CppMethodPointer = metadata::RtManagedMethodPointer;

// Registry struct for internal call functions
struct InternalCallRegistry
{
    InternalCallFunction func;
    InternalCallInvoker invoker;
};

struct InternalCallEntry
{
    const char* name;
    InternalCallFunction func;
    InternalCallInvoker invoker;
};

struct NewobjInternalCallEntry
{
    const char* name;
    InternalCallInvoker invoker;
};

class InternalCalls
{
  public:
    // Initialize internal calls
    static void initialize();

    static void register_lite_internal_call(const char* name, Il2CppMethodPointer func);
    static Il2CppMethodPointer get_lite_internal_call(const char* name);

    // Register/get internal call functions
    static void register_internal_call(const char* name, InternalCallFunction func, InternalCallInvoker invoker);
    static const InternalCallRegistry* get_internal_call(const char* name);
    static RtResult<const InternalCallRegistry*> get_internal_call_by_method(const metadata::RtMethodInfo* method);

    // Register/get newobj internal calls
    static void register_newobj_internal_call(const char* name, InternalCallInvoker invoker);
    static InternalCallInvoker get_newobj_internal_call(const char* name);
    static RtResult<InternalCallInvoker> get_newobj_internal_call_by_method(const metadata::RtMethodInfo* method);

    // Internal call invoker ID management
    static uint16_t register_internal_call_invoker_id(InternalCallInvoker invoker);
    static InternalCallInvoker get_internal_call_invoker_by_id_unchecked(uint16_t id);
};

} // namespace vm
} // namespace leanclr
