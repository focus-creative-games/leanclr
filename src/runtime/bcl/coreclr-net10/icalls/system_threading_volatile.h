#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemThreadingVolatile
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    static RtResult<int64_t> read_i64(const int64_t* location) noexcept;
    static RtResult<uint64_t> read_u64(const uint64_t* location) noexcept;
    static RtResult<double> read_f64(const double* location) noexcept;

    static RtResultVoid write_i64(int64_t* location, int64_t value) noexcept;
    static RtResultVoid write_u64(uint64_t* location, uint64_t value) noexcept;
    static RtResultVoid write_f64(double* location, double value) noexcept;
};

} // namespace icalls
} // namespace leanclr
