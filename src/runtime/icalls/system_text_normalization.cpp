#include "system_text_normalization.h"

#include "normalization_tables.h"

namespace leanclr
{
namespace icalls
{

RtResultVoid SystemTextNormalization::load_normalization_resource(intptr_t* props, intptr_t* mapped_chars, intptr_t* char_map_index,
                                                                  intptr_t* helper_index, intptr_t* map_idx_to_composite,
                                                                  intptr_t* combining_class) noexcept
{
    *props = reinterpret_cast<intptr_t>(normalization_tables::props);
    *mapped_chars = reinterpret_cast<intptr_t>(normalization_tables::mapped_chars);
    *char_map_index = reinterpret_cast<intptr_t>(normalization_tables::char_map_index);
    *helper_index = reinterpret_cast<intptr_t>(normalization_tables::helper_index);
    *map_idx_to_composite = reinterpret_cast<intptr_t>(normalization_tables::map_idx_to_composite);
    *combining_class = reinterpret_cast<intptr_t>(normalization_tables::combining_class);
    RET_VOID_OK();
}

static RtResultVoid load_normalization_resource_invoker(metadata::RtManagedMethodPointer /*method_ptr*/, const metadata::RtMethodInfo* /*method*/,
                                                        const interp::RtStackObject* params, interp::RtStackObject* /*ret*/) noexcept
{
    intptr_t* props = EvalStackOp::get_param<intptr_t*>(params, 0);
    intptr_t* mapped_chars = EvalStackOp::get_param<intptr_t*>(params, 1);
    intptr_t* char_map_index = EvalStackOp::get_param<intptr_t*>(params, 2);
    intptr_t* helper_index = EvalStackOp::get_param<intptr_t*>(params, 3);
    intptr_t* map_idx_to_composite = EvalStackOp::get_param<intptr_t*>(params, 4);
    intptr_t* combining_class = EvalStackOp::get_param<intptr_t*>(params, 5);
    RET_ERR_ON_FAIL(SystemTextNormalization::load_normalization_resource(props, mapped_chars, char_map_index, helper_index, map_idx_to_composite,
                                                                         combining_class));
    RET_VOID_OK();
}

utils::Span<vm::InternalCallEntry> SystemTextNormalization::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.Text.Normalization::load_normalization_resource(System.IntPtr&,System.IntPtr&,System.IntPtr&,System.IntPtr&,System.IntPtr&,"
         "System.IntPtr&)",
         (vm::InternalCallFunction)&SystemTextNormalization::load_normalization_resource, load_normalization_resource_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr
