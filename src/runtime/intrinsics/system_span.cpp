#include <cstring>

#include "system_span.h"
#include "system_readonlyspan.h"
#include "interp/interp_defs.h"
#include "interp/eval_stack_op.h"
#include "vm/class.h"

namespace leanclr
{
namespace intrinsics
{

// ========== Implementation Functions ==========

RtResult<const uint8_t*> SystemSpan::get_item(const vm::RtReadOnlySpan<uint8_t>& span, int32_t index, size_t ele_size) noexcept
{
    if ((uint32_t)index >= (uint32_t)span.length)
    {
        RET_ERR(RtErr::IndexOutOfRange);
    }
    RET_OK(span.pointer + (static_cast<size_t>(index) * ele_size));
}

RtResult<vm::RtReadOnlySpan<uint8_t>> SystemSpan::newobj_pointer_length(void* pointer, int32_t length) noexcept
{
    if (length < 0)
    {
        RET_ERR(RtErr::ArgumentOutOfRange);
    }

    vm::RtReadOnlySpan<uint8_t> span{reinterpret_cast<const uint8_t*>(pointer), length};
    RET_OK(span);
}

RtResult<int32_t> SystemSpan::index_of_null_byte(const uint8_t* pointer) noexcept
{
    if (pointer == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    RET_OK(static_cast<int32_t>(std::strlen(reinterpret_cast<const char*>(pointer))));
}

RtResult<int32_t> SystemSpan::sequence_equal(const uint8_t* first, const uint8_t* second, size_t length) noexcept
{
    if (length == 0)
    {
        RET_OK(1);
    }

    if (first == nullptr || second == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    RET_OK(std::memcmp(first, second, length) == 0 ? 1 : 0);
}

// ========== Invoker Functions ==========

static RtResult<const metadata::RtTypeSig*> get_span_element_type(const metadata::RtMethodInfo* method) noexcept
{
    if (method == nullptr || method->parent == nullptr)
    {
        RET_ERR(RtErr::ExecutionEngine);
    }

    const metadata::RtClass* klass = method->parent;
    if (vm::Class::is_generic_inst(klass))
    {
        const metadata::RtGenericClass* generic_class = klass->by_val->data.generic_class;
        if (generic_class != nullptr && generic_class->class_inst != nullptr && generic_class->class_inst->generic_arg_count == 1)
        {
            RET_OK(generic_class->class_inst->generic_args[0]);
        }
    }

    if (method->generic_method != nullptr && method->generic_method->generic_context.class_inst != nullptr &&
        method->generic_method->generic_context.class_inst->generic_arg_count == 1)
    {
        RET_OK(method->generic_method->generic_context.class_inst->generic_args[0]);
    }

    RET_ERR(RtErr::BadImageFormat);
}

/// @intrinsic: System.Span`1::get_Item
static RtResultVoid get_item_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                     interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    const vm::RtReadOnlySpan<uint8_t>& span = *interp::EvalStackOp::get_param<const vm::RtReadOnlySpan<uint8_t>*>(params, 0);
    int32_t index = interp::EvalStackOp::get_param<int32_t>(params, 1);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, ele_type, get_span_element_type(method));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(interp::ReduceTypeAndSize, type_and_size, interp::InterpDefs::get_reduce_type_and_size_by_typesig(ele_type));
    size_t ele_size = type_and_size.byte_size;

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const uint8_t*, item_ptr, SystemSpan::get_item(span, index, ele_size));
    interp::EvalStackOp::set_return(ret, item_ptr);
    RET_VOID_OK();
}

/// @newobj: System.Span`1::.ctor(System.Void*,System.Int32)
/// @newobj: System.ReadOnlySpan`1::.ctor(System.Void*,System.Int32)
static RtResultVoid newobj_pointer_length_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                  const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    void* pointer = interp::EvalStackOp::get_param<void*>(params, 0);
    int32_t length = interp::EvalStackOp::get_param<int32_t>(params, 1);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReadOnlySpan<uint8_t>, span, SystemSpan::newobj_pointer_length(pointer, length));
    interp::EvalStackOp::set_return(ret, span);
    RET_VOID_OK();
}

static RtResultVoid index_of_null_byte_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                               interp::RtStackObject* ret) noexcept
{
    auto pointer = interp::EvalStackOp::get_param<const uint8_t*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, length, SystemSpan::index_of_null_byte(pointer));
    interp::EvalStackOp::set_return(ret, length);
    RET_VOID_OK();
}

static size_t get_size_param(const metadata::RtMethodInfo* method, const interp::RtStackObject* params, size_t index) noexcept
{
    const metadata::RtTypeSig* param_type = method->parameters[index];
    switch (param_type->ele_type)
    {
    case metadata::RtElementType::U4:
        return static_cast<size_t>(interp::EvalStackOp::get_param<uint32_t>(params, index));
    case metadata::RtElementType::I4:
        return static_cast<size_t>(interp::EvalStackOp::get_param<int32_t>(params, index));
    case metadata::RtElementType::U:
        return static_cast<size_t>(interp::EvalStackOp::get_param<uintptr_t>(params, index));
    case metadata::RtElementType::I:
    default:
        return static_cast<size_t>(interp::EvalStackOp::get_param<intptr_t>(params, index));
    }
}

static RtResultVoid sequence_equal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                           interp::RtStackObject* ret) noexcept
{
    auto first = interp::EvalStackOp::get_param<const uint8_t*>(params, 0);
    auto second = interp::EvalStackOp::get_param<const uint8_t*>(params, 1);
    size_t length = get_size_param(method, params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, equal, SystemSpan::sequence_equal(first, second, length));
    interp::EvalStackOp::set_return(ret, equal);
    RET_VOID_OK();
}

// ========== Intrinsic Entries ==========

static vm::IntrinsicEntry s_intrinsic_entries_system_span[] = {
    {"System.Span`1::get_Item", (vm::IntrinsicFunction)&SystemSpan::get_item, get_item_invoker},
    {"System.ReadOnlySpan`1::get_Item", (vm::IntrinsicFunction)&SystemSpan::get_item, get_item_invoker},
    {"System.SpanHelpers::IndexOfNullByte(System.Byte*)", (vm::IntrinsicFunction)&SystemSpan::index_of_null_byte,
     index_of_null_byte_invoker},
    {"System.SpanHelpers::IndexOfNullByte", (vm::IntrinsicFunction)&SystemSpan::index_of_null_byte, index_of_null_byte_invoker},
    {"System.SpanHelpers::SequenceEqual(System.Byte&,System.Byte&,System.UIntPtr)", (vm::IntrinsicFunction)&SystemSpan::sequence_equal,
     sequence_equal_invoker},
};

static vm::NewobjIntrinsicEntry s_newobj_intrinsic_entries_system_span[] = {
    {"System.Span`1::.ctor(System.Void*,System.Int32)", newobj_pointer_length_invoker},
    {"System.ReadOnlySpan`1::.ctor(System.Void*,System.Int32)", newobj_pointer_length_invoker},
};

utils::Span<vm::IntrinsicEntry> SystemSpan::get_intrinsic_entries() noexcept
{
    constexpr size_t entry_count = sizeof(s_intrinsic_entries_system_span) / sizeof(s_intrinsic_entries_system_span[0]);
    return utils::Span<vm::IntrinsicEntry>(s_intrinsic_entries_system_span, entry_count);
}

utils::Span<vm::NewobjIntrinsicEntry> SystemSpan::get_newobj_intrinsic_entries() noexcept
{
    constexpr size_t entry_count = sizeof(s_newobj_intrinsic_entries_system_span) / sizeof(s_newobj_intrinsic_entries_system_span[0]);
    return utils::Span<vm::NewobjIntrinsicEntry>(s_newobj_intrinsic_entries_system_span, entry_count);
}

} // namespace intrinsics
} // namespace leanclr
