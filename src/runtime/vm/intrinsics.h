#pragma once

#include "core/stl_compat.h"

#include "rt_managed_types.h"
#include "utils/hashmap.h"
#include "utils/rt_vector.h"
#include "utils/string_util.h"
#include "utils/rt_span.h"
#include "interp/eval_stack_op.h"

namespace leanclr
{
namespace vm
{

using IntrinsicFunction = metadata::RtManagedMethodPointer;
using IntrinsicInvoker = metadata::RtInvokeMethodPointer;

// Registry struct for intrinsic functions
struct IntrinsicRegistry
{
    IntrinsicFunction func;
    IntrinsicInvoker invoker;
};

struct IntrinsicEntry
{
    const char* name;
    IntrinsicFunction func;
    IntrinsicInvoker invoker;
};

struct NewobjIntrinsicEntry
{
    const char* name;
    IntrinsicInvoker invoker;
};

// Register/get intrinsic functions

class Intrinsics
{
  public:
    // Initialize intrinsics
    static void initialize();

    static void register_intrinsic(const char* name, IntrinsicFunction func, IntrinsicInvoker invoker);
    static const IntrinsicRegistry* get_intrinsic(const char* name);
    static RtResult<const IntrinsicRegistry*> get_intrinsic_by_method(const metadata::RtMethodInfo* method);

    // Register/get newobj intrinsics
    static void register_newobj_intrinsic(const char* name, IntrinsicInvoker invoker);
    static IntrinsicInvoker get_newobj_intrinsic(const char* name);
    static RtResult<IntrinsicInvoker> get_newobj_intrinsic_by_method(const metadata::RtMethodInfo* method);

    // Intrinsic invoker ID management
    static uint16_t register_intrinsic_invoker_id(IntrinsicInvoker invoker);
    static IntrinsicInvoker get_intrinsic_invoker_by_id_unchecked(uint16_t id);
};
} // namespace vm
} // namespace leanclr
