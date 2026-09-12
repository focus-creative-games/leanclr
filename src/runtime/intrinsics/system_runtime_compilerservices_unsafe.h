#pragma once

#include "vm/intrinsics.h"

namespace leanclr
{
namespace intrinsics
{

class SystemRuntimeCompilerServicesUnsafe
{
  public:
    static RtResult<void*> as_pointer(void* location) noexcept;
    static RtResult<void*> as(void* source) noexcept;
    static RtResult<intptr_t> byte_offset(void* origin, void* target) noexcept;
    static RtResult<bool> are_same(void* left, void* right) noexcept;
    static RtResult<bool> is_address_less_than(void* left, void* right) noexcept;
    static RtResult<bool> is_address_greater_than(void* left, void* right) noexcept;
    static RtResult<int32_t> size_of(const metadata::RtMethodInfo* method) noexcept;
    static RtResultVoid copy_block(const interp::RtStackObject* params) noexcept;
    static RtResultVoid read_unaligned(const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                       interp::RtStackObject* ret) noexcept;
    static RtResultVoid write_unaligned(const metadata::RtMethodInfo* method, const interp::RtStackObject* params) noexcept;
    static RtResultVoid add(const metadata::RtMethodInfo* method, const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept;
    static RtResultVoid add_byte_offset(const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                        interp::RtStackObject* ret) noexcept;
    static RtResultVoid subtract_byte_offset(const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                             interp::RtStackObject* ret) noexcept;
    static RtResultVoid skip_init(const interp::RtStackObject* params) noexcept;
    static RtResultVoid bit_cast(const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                 interp::RtStackObject* ret) noexcept;

    static utils::Span<vm::IntrinsicEntry> get_intrinsic_entries() noexcept;
};

} // namespace intrinsics
} // namespace leanclr
