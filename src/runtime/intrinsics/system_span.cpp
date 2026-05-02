#include "system_span.h"
#include "system_readonlyspan.h"
#include "interp/interp_defs.h"
#include "interp/eval_stack_op.h"

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

// ========== Invoker Functions ==========

/// @intrinsic: System.Span`1::get_Item
static RtResultVoid get_item_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                     interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    const vm::RtReadOnlySpan<uint8_t>& span = *interp::EvalStackOp::get_param<const vm::RtReadOnlySpan<uint8_t>*>(params, 0);
    int32_t index = interp::EvalStackOp::get_param<int32_t>(params, 1);

    const metadata::RtClass* klass = method->parent;
    const metadata::RtGenericClass* generic_class = klass->by_val->data.generic_class;
    const metadata::RtTypeSig* ele_type = *generic_class->class_inst->generic_args;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(interp::ReduceTypeAndSize, type_and_size, interp::InterpDefs::get_reduce_type_and_size_by_typesig(ele_type));
    size_t ele_size = type_and_size.byte_size;

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const uint8_t*, item_ptr, SystemSpan::get_item(span, index, ele_size));
    interp::EvalStackOp::set_return(ret, item_ptr);
    RET_VOID_OK();
}

// ========== Intrinsic Entries ==========

static vm::IntrinsicEntry s_intrinsic_entries_system_span[] = {
    {"System.Span`1::get_Item", (vm::IntrinsicFunction)&SystemSpan::get_item, get_item_invoker},
    {"System.ReadOnlySpan`1::get_Item", (vm::IntrinsicFunction)&SystemSpan::get_item, get_item_invoker},
};

utils::Span<vm::IntrinsicEntry> SystemSpan::get_intrinsic_entries() noexcept
{
    constexpr size_t entry_count = sizeof(s_intrinsic_entries_system_span) / sizeof(s_intrinsic_entries_system_span[0]);
    return utils::Span<vm::IntrinsicEntry>(s_intrinsic_entries_system_span, entry_count);
}

} // namespace intrinsics
} // namespace leanclr
