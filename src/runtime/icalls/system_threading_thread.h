#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{
class SystemThreadingThread
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries();

    // Thread management
    static RtResultVoid get_current_thread_icall(vm::RtThread** thread);
    static RtResultVoid reset_abort_native(vm::RtThread* this_thread);
    static RtResultVoid suspend_internal(vm::RtThread* this_thread);
    static RtResultVoid resume_internal(vm::RtThread* this_thread);
    static RtResultVoid interrupt_internal(vm::RtThread* this_thread);
    static RtResult<int32_t> get_priority_native(vm::RtThread* this_thread);
    static RtResultVoid set_priority_native(vm::RtThread* this_thread, int32_t priority);
    static RtResult<bool> join_internal(vm::RtThread* this_thread, int32_t milliseconds);
    static RtResultVoid sleep_internal(int32_t milliseconds);
    static RtResult<bool> yield_internal();
    static RtResultVoid memory_barrier();
    static RtResultVoid construct_internal_thread(vm::RtThread* this_thread);

    // Array domain conversions
    static RtResult<vm::RtArray*> byte_array_to_root_domain(vm::RtArray* arr);
    static RtResult<vm::RtArray*> byte_array_to_current_domain(vm::RtArray* arr);

    // AppDomain access
    static RtResult<int32_t> get_domain_id();

    // Thread initialization
    static RtResult<bool> thread_internal(vm::RtThread* this_thread, vm::RtObject* start);

    // Thread name operations
    static RtResult<vm::RtString*> get_name_internal(vm::RtInternalThread* internal_thread);
    static RtResultVoid set_name_icall(vm::RtInternalThread* internal_thread, const Utf16Char* name, int32_t len);

    // Thread abort
    static RtResultVoid abort_internal(vm::RtObject* internal_thread, vm::RtObject* state);
    static RtResult<vm::RtObject*> get_abort_exception_state();

    // Spin wait
    static RtResultVoid spin_wait_nop();

    // Thread state management
    static RtResultVoid set_state(vm::RtInternalThread* internal_thread, int32_t state);
    static RtResultVoid clr_state(vm::RtInternalThread* internal_thread, int32_t state);
    static RtResult<int32_t> get_state(vm::RtInternalThread* internal_thread);

    // Volatile read operations
    static RtResult<uint8_t> volatile_read_u8(uint8_t* loc);
    static RtResult<int8_t> volatile_read_i8(int8_t* loc);
    static RtResult<uint16_t> volatile_read_u16(uint16_t* loc);
    static RtResult<int16_t> volatile_read_i16(int16_t* loc);
    static RtResult<uint32_t> volatile_read_u32(uint32_t* loc);
    static RtResult<int32_t> volatile_read_i32(int32_t* loc);
    static RtResult<uint64_t> volatile_read_u64(uint64_t* loc);
    static RtResult<int64_t> volatile_read_i64(int64_t* loc);
    static RtResult<float> volatile_read_f32(float* loc);
    static RtResult<double> volatile_read_f64(double* loc);
    static RtResult<intptr_t> volatile_read_intptr(intptr_t* loc);
    static RtResult<uintptr_t> volatile_read_uintptr(uintptr_t* loc);
    static RtResult<vm::RtObject*> volatile_read_object(vm::RtObject** loc);

    // Volatile write operations
    static RtResultVoid volatile_write_u8(uint8_t* loc, uint8_t val);
    static RtResultVoid volatile_write_i8(int8_t* loc, int8_t val);
    static RtResultVoid volatile_write_u16(uint16_t* loc, uint16_t val);
    static RtResultVoid volatile_write_i16(int16_t* loc, int16_t val);
    static RtResultVoid volatile_write_u32(uint32_t* loc, uint32_t val);
    static RtResultVoid volatile_write_i32(int32_t* loc, int32_t val);
    static RtResultVoid volatile_write_u64(uint64_t* loc, uint64_t val);
    static RtResultVoid volatile_write_i64(int64_t* loc, int64_t val);
    static RtResultVoid volatile_write_f32(float* loc, float val);
    static RtResultVoid volatile_write_f64(double* loc, double val);
    static RtResultVoid volatile_write_intptr(intptr_t* loc, intptr_t val);
    static RtResultVoid volatile_write_uintptr(uintptr_t* loc, uintptr_t val);
    static RtResultVoid volatile_write_object(vm::RtObject** loc, vm::RtObject* val);

    // Stack size
    static RtResult<int32_t> system_max_stack_stize();

    // Stack traces
    static RtResultVoid get_stack_traces(vm::RtArray** threads, vm::RtArray** stack_frames);
};
} // namespace icalls
} // namespace leanclr
