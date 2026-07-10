#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemTextNormalization
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    /// @icall: System.Text.Normalization::load_normalization_resource(System.IntPtr&,System.IntPtr&,System.IntPtr&,System.IntPtr&,System.IntPtr&,System.IntPtr&)
    static RtResultVoid load_normalization_resource(intptr_t* props, intptr_t* mapped_chars, intptr_t* char_map_index, intptr_t* helper_index,
                                                    intptr_t* map_idx_to_composite, intptr_t* combining_class) noexcept;
};

} // namespace icalls
} // namespace leanclr
