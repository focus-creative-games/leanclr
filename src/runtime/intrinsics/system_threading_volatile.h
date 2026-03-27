#pragma once

#include "vm/intrinsics.h"

namespace leanclr
{
namespace intrinsics
{
class SystemThreadingVolatile
{
  public:
    static utils::Span<vm::IntrinsicEntry> get_intrinsic_entries();

    // Read operations
    static RtResult<bool> read_bool(const bool* location);
    static RtResult<uint8_t> read_byte(const uint8_t* location);
    static RtResult<int8_t> read_sbyte(const int8_t* location);
    static RtResult<int16_t> read_i16(const int16_t* location);
    static RtResult<uint16_t> read_u16(const uint16_t* location);
    static RtResult<int32_t> read_i32(const int32_t* location);
    static RtResult<uint32_t> read_u32(const uint32_t* location);
    static RtResult<int64_t> read_i64(const int64_t* location);
    static RtResult<uint64_t> read_u64(const uint64_t* location);
    static RtResult<intptr_t> read_intptr(const intptr_t* location);
    static RtResult<uintptr_t> read_uintptr(const uintptr_t* location);
    static RtResult<float> read_f32(const float* location);
    static RtResult<double> read_f64(const double* location);
    static RtResult<vm::RtObject*> read_ref(vm::RtObject* const* location);

    // Write operations
    static RtResultVoid write_bool(bool* location, bool value);
    static RtResultVoid write_byte(uint8_t* location, uint8_t value);
    static RtResultVoid write_sbyte(int8_t* location, int8_t value);
    static RtResultVoid write_i16(int16_t* location, int16_t value);
    static RtResultVoid write_u16(uint16_t* location, uint16_t value);
    static RtResultVoid write_i32(int32_t* location, int32_t value);
    static RtResultVoid write_u32(uint32_t* location, uint32_t value);
    static RtResultVoid write_i64(int64_t* location, int64_t value);
    static RtResultVoid write_u64(uint64_t* location, uint64_t value);
    static RtResultVoid write_intptr(intptr_t* location, intptr_t value);
    static RtResultVoid write_uintptr(uintptr_t* location, uintptr_t value);
    static RtResultVoid write_f32(float* location, float value);
    static RtResultVoid write_f64(double* location, double value);
    static RtResultVoid write_ref(vm::RtObject** location, vm::RtObject* value);
};
} // namespace intrinsics
} // namespace leanclr
