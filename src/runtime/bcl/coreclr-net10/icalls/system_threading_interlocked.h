#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemThreadingInterlocked
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    // Add operations
    static RtResult<int32_t> add_i32(int32_t* location, int32_t value) noexcept;
    static RtResult<int64_t> add_i64(int64_t* location, int64_t value) noexcept;

    // Increment operations
    static RtResult<int32_t> increment_i32(int32_t* location) noexcept;
    static RtResult<int64_t> increment_i64(int64_t* location) noexcept;

    // Decrement operations
    static RtResult<int32_t> decrement_i32(int32_t* location) noexcept;
    static RtResult<int64_t> decrement_i64(int64_t* location) noexcept;

    // CompareExchange operations
    static RtResult<int32_t> compare_exchange_i32(int32_t* location, int32_t value, int32_t comparand) noexcept;
    static RtResult<int32_t> compare_exchange2_i32(int32_t* location, int32_t value, int32_t comparand, bool* succ) noexcept;
    static RtResult<int64_t> compare_exchange_i64(int64_t* location, int64_t value, int64_t comparand) noexcept;
    static RtResult<intptr_t> compare_exchange_intptr(intptr_t* location, intptr_t value, intptr_t comparand) noexcept;
    static RtResultVoid compare_exchange_object2(vm::RtObject** location, vm::RtObject** value, vm::RtObject** comparand, vm::RtObject** result) noexcept;
    static RtResult<vm::RtObject*> compare_exchange_object(vm::RtObject** location, vm::RtObject* value, vm::RtObject* comparand) noexcept;
    static RtResult<float> compare_exchange_f32(float* location, float value, float comparand) noexcept;
    static RtResult<double> compare_exchange_f64(double* location, double value, double comparand) noexcept;

    // Exchange operations
    static RtResult<int32_t> exchange_i32(int32_t* location, int32_t value) noexcept;
    static RtResult<int64_t> exchange_i64(int64_t* location, int64_t value) noexcept;
    static RtResult<intptr_t> exchange_intptr(intptr_t* location, intptr_t value) noexcept;
    static RtResult<float> exchange_f32(float* location, float value) noexcept;
    static RtResult<double> exchange_f64(double* location, double value) noexcept;
    static RtResultVoid exchange_object(vm::RtObject** location, vm::RtObject** value, vm::RtObject** result) noexcept;

    // Memory barrier
    static RtResultVoid memory_barrier_process_wide() noexcept;

    // Read
    static RtResult<int64_t> read(int64_t* location) noexcept;
};

} // namespace icalls
} // namespace leanclr
