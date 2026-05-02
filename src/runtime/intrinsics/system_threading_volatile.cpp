#include "system_threading_volatile.h"
#include "interp/eval_stack_op.h"

namespace leanclr
{
namespace intrinsics
{

// ========== Read Implementation Functions ==========

RtResult<bool> SystemThreadingVolatile::read_bool(const bool* location) noexcept
{
    RET_OK(*location);
}

RtResult<uint8_t> SystemThreadingVolatile::read_byte(const uint8_t* location) noexcept
{
    RET_OK(*location);
}

RtResult<int8_t> SystemThreadingVolatile::read_sbyte(const int8_t* location) noexcept
{
    RET_OK(*location);
}

RtResult<int16_t> SystemThreadingVolatile::read_i16(const int16_t* location) noexcept
{
    RET_OK(*location);
}

RtResult<uint16_t> SystemThreadingVolatile::read_u16(const uint16_t* location) noexcept
{
    RET_OK(*location);
}

RtResult<int32_t> SystemThreadingVolatile::read_i32(const int32_t* location) noexcept
{
    RET_OK(*location);
}

RtResult<uint32_t> SystemThreadingVolatile::read_u32(const uint32_t* location) noexcept
{
    RET_OK(*location);
}

RtResult<int64_t> SystemThreadingVolatile::read_i64(const int64_t* location) noexcept
{
    RET_OK(*location);
}

RtResult<uint64_t> SystemThreadingVolatile::read_u64(const uint64_t* location) noexcept
{
    RET_OK(*location);
}

RtResult<intptr_t> SystemThreadingVolatile::read_intptr(const intptr_t* location) noexcept
{
    RET_OK(*location);
}

RtResult<uintptr_t> SystemThreadingVolatile::read_uintptr(const uintptr_t* location) noexcept
{
    RET_OK(*location);
}

RtResult<float> SystemThreadingVolatile::read_f32(const float* location) noexcept
{
    RET_OK(*location);
}

RtResult<double> SystemThreadingVolatile::read_f64(const double* location) noexcept
{
    RET_OK(*location);
}

RtResult<vm::RtObject*> SystemThreadingVolatile::read_ref(vm::RtObject* const* location) noexcept
{
    RET_OK(*location);
}

// ========== Write Implementation Functions ==========

RtResultVoid SystemThreadingVolatile::write_bool(bool* location, bool value) noexcept
{
    *location = value;
    RET_VOID_OK();
}

RtResultVoid SystemThreadingVolatile::write_byte(uint8_t* location, uint8_t value) noexcept
{
    *location = value;
    RET_VOID_OK();
}

RtResultVoid SystemThreadingVolatile::write_sbyte(int8_t* location, int8_t value) noexcept
{
    *location = value;
    RET_VOID_OK();
}

RtResultVoid SystemThreadingVolatile::write_i16(int16_t* location, int16_t value) noexcept
{
    *location = value;
    RET_VOID_OK();
}

RtResultVoid SystemThreadingVolatile::write_u16(uint16_t* location, uint16_t value) noexcept
{
    *location = value;
    RET_VOID_OK();
}

RtResultVoid SystemThreadingVolatile::write_i32(int32_t* location, int32_t value) noexcept
{
    *location = value;
    RET_VOID_OK();
}

RtResultVoid SystemThreadingVolatile::write_u32(uint32_t* location, uint32_t value) noexcept
{
    *location = value;
    RET_VOID_OK();
}

RtResultVoid SystemThreadingVolatile::write_i64(int64_t* location, int64_t value) noexcept
{
    *location = value;
    RET_VOID_OK();
}

RtResultVoid SystemThreadingVolatile::write_u64(uint64_t* location, uint64_t value) noexcept
{
    *location = value;
    RET_VOID_OK();
}

RtResultVoid SystemThreadingVolatile::write_intptr(intptr_t* location, intptr_t value) noexcept
{
    *location = value;
    RET_VOID_OK();
}

RtResultVoid SystemThreadingVolatile::write_uintptr(uintptr_t* location, uintptr_t value) noexcept
{
    *location = value;
    RET_VOID_OK();
}

RtResultVoid SystemThreadingVolatile::write_f32(float* location, float value) noexcept
{
    *location = value;
    RET_VOID_OK();
}

RtResultVoid SystemThreadingVolatile::write_f64(double* location, double value) noexcept
{
    *location = value;
    RET_VOID_OK();
}

RtResultVoid SystemThreadingVolatile::write_ref(vm::RtObject** location, vm::RtObject* value) noexcept
{
    *location = value;
    RET_VOID_OK();
}

// ========== Read Invoker Functions ==========

/// @intrinsic: System.Threading.Volatile::Read(System.Boolean&)
static RtResultVoid read_bool_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    const bool* location = interp::EvalStackOp::get_param<const bool*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, value, SystemThreadingVolatile::read_bool(location));
    interp::EvalStackOp::set_return(ret, static_cast<int32_t>(value));
    RET_VOID_OK();
}

/// @intrinsic: System.Threading.Volatile::Read(System.Byte&)
static RtResultVoid read_byte_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    const uint8_t* location = interp::EvalStackOp::get_param<const uint8_t*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint8_t, value, SystemThreadingVolatile::read_byte(location));
    interp::EvalStackOp::set_return(ret, static_cast<int32_t>(value));
    RET_VOID_OK();
}

/// @intrinsic: System.Threading.Volatile::Read(System.SByte&)
static RtResultVoid read_sbyte_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                       interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    const int8_t* location = interp::EvalStackOp::get_param<const int8_t*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int8_t, value, SystemThreadingVolatile::read_sbyte(location));
    interp::EvalStackOp::set_return(ret, static_cast<int32_t>(value));
    RET_VOID_OK();
}

/// @intrinsic: System.Threading.Volatile::Read(System.Int16&)
static RtResultVoid read_i16_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                     interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    const int16_t* location = interp::EvalStackOp::get_param<const int16_t*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int16_t, value, SystemThreadingVolatile::read_i16(location));
    interp::EvalStackOp::set_return(ret, static_cast<int32_t>(value));
    RET_VOID_OK();
}

/// @intrinsic: System.Threading.Volatile::Read(System.UInt16&)
static RtResultVoid read_u16_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                     interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    const uint16_t* location = interp::EvalStackOp::get_param<const uint16_t*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint16_t, value, SystemThreadingVolatile::read_u16(location));
    interp::EvalStackOp::set_return(ret, static_cast<int32_t>(value));
    RET_VOID_OK();
}

/// @intrinsic: System.Threading.Volatile::Read(System.Int32&)
static RtResultVoid read_i32_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                     interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    const int32_t* location = interp::EvalStackOp::get_param<const int32_t*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, value, SystemThreadingVolatile::read_i32(location));
    interp::EvalStackOp::set_return(ret, value);
    RET_VOID_OK();
}

/// @intrinsic: System.Threading.Volatile::Read(System.UInt32&)
static RtResultVoid read_u32_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                     interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    const uint32_t* location = interp::EvalStackOp::get_param<const uint32_t*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint32_t, value, SystemThreadingVolatile::read_u32(location));
    interp::EvalStackOp::set_return(ret, value);
    RET_VOID_OK();
}

/// @intrinsic: System.Threading.Volatile::Read(System.Int64&)
static RtResultVoid read_i64_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                     interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    const int64_t* location = interp::EvalStackOp::get_param<const int64_t*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int64_t, value, SystemThreadingVolatile::read_i64(location));
    interp::EvalStackOp::set_return(ret, value);
    RET_VOID_OK();
}

/// @intrinsic: System.Threading.Volatile::Read(System.UInt64&)
static RtResultVoid read_u64_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                     interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    const uint64_t* location = interp::EvalStackOp::get_param<const uint64_t*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint64_t, value, SystemThreadingVolatile::read_u64(location));
    interp::EvalStackOp::set_return(ret, value);
    RET_VOID_OK();
}

/// @intrinsic: System.Threading.Volatile::Read(System.IntPtr&)
static RtResultVoid read_intptr_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                        interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    const intptr_t* location = interp::EvalStackOp::get_param<const intptr_t*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(intptr_t, value, SystemThreadingVolatile::read_intptr(location));
    interp::EvalStackOp::set_return(ret, value);
    RET_VOID_OK();
}

/// @intrinsic: System.Threading.Volatile::Read(System.UIntPtr&)
static RtResultVoid read_uintptr_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                         interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    const uintptr_t* location = interp::EvalStackOp::get_param<const uintptr_t*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uintptr_t, value, SystemThreadingVolatile::read_uintptr(location));
    interp::EvalStackOp::set_return(ret, value);
    RET_VOID_OK();
}

/// @intrinsic: System.Threading.Volatile::Read(System.Single&)
static RtResultVoid read_f32_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                     interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    const float* location = interp::EvalStackOp::get_param<const float*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(float, value, SystemThreadingVolatile::read_f32(location));
    interp::EvalStackOp::set_return(ret, value);
    RET_VOID_OK();
}

/// @intrinsic: System.Threading.Volatile::Read(System.Double&)
static RtResultVoid read_f64_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                     interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    const double* location = interp::EvalStackOp::get_param<const double*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(double, value, SystemThreadingVolatile::read_f64(location));
    interp::EvalStackOp::set_return(ret, value);
    RET_VOID_OK();
}

/// @intrinsic: System.Threading.Volatile::Read<>
static RtResultVoid read_ref_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                     interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    vm::RtObject* const* location = interp::EvalStackOp::get_param<vm::RtObject* const*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, value, SystemThreadingVolatile::read_ref(location));
    interp::EvalStackOp::set_return(ret, value);
    RET_VOID_OK();
}

// ========== Write Invoker Functions ==========

/// @intrinsic: System.Threading.Volatile::Write(System.Boolean&,System.Boolean)
static RtResultVoid write_bool_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                       interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    bool* location = interp::EvalStackOp::get_param<bool*>(params, 0);
    bool value = interp::EvalStackOp::get_param<bool>(params, 1);
    return SystemThreadingVolatile::write_bool(location, value);
}

/// @intrinsic: System.Threading.Volatile::Write(System.Byte&,System.Byte)
static RtResultVoid write_byte_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                       interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    uint8_t* location = interp::EvalStackOp::get_param<uint8_t*>(params, 0);
    uint8_t value = interp::EvalStackOp::get_param<uint8_t>(params, 1);
    return SystemThreadingVolatile::write_byte(location, value);
}

/// @intrinsic: System.Threading.Volatile::Write(System.SByte&,System.SByte)
static RtResultVoid write_sbyte_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                        interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    int8_t* location = interp::EvalStackOp::get_param<int8_t*>(params, 0);
    int8_t value = interp::EvalStackOp::get_param<int8_t>(params, 1);
    return SystemThreadingVolatile::write_sbyte(location, value);
}

/// @intrinsic: System.Threading.Volatile::Write(System.Int16&,System.Int16)
static RtResultVoid write_i16_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    int16_t* location = interp::EvalStackOp::get_param<int16_t*>(params, 0);
    int16_t value = interp::EvalStackOp::get_param<int16_t>(params, 1);
    return SystemThreadingVolatile::write_i16(location, value);
}

/// @intrinsic: System.Threading.Volatile::Write(System.UInt16&,System.UInt16)
static RtResultVoid write_u16_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    uint16_t* location = interp::EvalStackOp::get_param<uint16_t*>(params, 0);
    uint16_t value = interp::EvalStackOp::get_param<uint16_t>(params, 1);
    return SystemThreadingVolatile::write_u16(location, value);
}

/// @intrinsic: System.Threading.Volatile::Write(System.Int32&,System.Int32)
static RtResultVoid write_i32_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    int32_t* location = interp::EvalStackOp::get_param<int32_t*>(params, 0);
    int32_t value = interp::EvalStackOp::get_param<int32_t>(params, 1);
    return SystemThreadingVolatile::write_i32(location, value);
}

/// @intrinsic: System.Threading.Volatile::Write(System.UInt32&,System.UInt32)
static RtResultVoid write_u32_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    uint32_t* location = interp::EvalStackOp::get_param<uint32_t*>(params, 0);
    uint32_t value = interp::EvalStackOp::get_param<uint32_t>(params, 1);
    return SystemThreadingVolatile::write_u32(location, value);
}

/// @intrinsic: System.Threading.Volatile::Write(System.Int64&,System.Int64)
static RtResultVoid write_i64_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    int64_t* location = interp::EvalStackOp::get_param<int64_t*>(params, 0);
    int64_t value = interp::EvalStackOp::get_param<int64_t>(params, 1);
    return SystemThreadingVolatile::write_i64(location, value);
}

/// @intrinsic: System.Threading.Volatile::Write(System.UInt64&,System.UInt64)
static RtResultVoid write_u64_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    uint64_t* location = interp::EvalStackOp::get_param<uint64_t*>(params, 0);
    uint64_t value = interp::EvalStackOp::get_param<uint64_t>(params, 1);
    return SystemThreadingVolatile::write_u64(location, value);
}

/// @intrinsic: System.Threading.Volatile::Write(System.IntPtr&,System.IntPtr)
static RtResultVoid write_intptr_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                         interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    intptr_t* location = interp::EvalStackOp::get_param<intptr_t*>(params, 0);
    intptr_t value = interp::EvalStackOp::get_param<intptr_t>(params, 1);
    return SystemThreadingVolatile::write_intptr(location, value);
}

/// @intrinsic: System.Threading.Volatile::Write(System.UIntPtr&,System.UIntPtr)
static RtResultVoid write_uintptr_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                          interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    uintptr_t* location = interp::EvalStackOp::get_param<uintptr_t*>(params, 0);
    uintptr_t value = interp::EvalStackOp::get_param<uintptr_t>(params, 1);
    return SystemThreadingVolatile::write_uintptr(location, value);
}

/// @intrinsic: System.Threading.Volatile::Write(System.Single&,System.Single)
static RtResultVoid write_f32_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    float* location = interp::EvalStackOp::get_param<float*>(params, 0);
    float value = interp::EvalStackOp::get_param<float>(params, 1);
    return SystemThreadingVolatile::write_f32(location, value);
}

/// @intrinsic: System.Threading.Volatile::Write(System.Double&,System.Double)
static RtResultVoid write_f64_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    double* location = interp::EvalStackOp::get_param<double*>(params, 0);
    double value = interp::EvalStackOp::get_param<double>(params, 1);
    return SystemThreadingVolatile::write_f64(location, value);
}

/// @intrinsic: System.Threading.Volatile::Write<>
static RtResultVoid write_ref_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    vm::RtObject** location = interp::EvalStackOp::get_param<vm::RtObject**>(params, 0);
    vm::RtObject* value = interp::EvalStackOp::get_param<vm::RtObject*>(params, 1);
    return SystemThreadingVolatile::write_ref(location, value);
}

// ========== Intrinsic Entries ==========

static vm::IntrinsicEntry s_intrinsic_entries_system_threading_volatile[] = {
    // Read operations
    {"System.Threading.Volatile::Read(System.Boolean&)", (vm::IntrinsicFunction)&SystemThreadingVolatile::read_bool, read_bool_invoker},
    {"System.Threading.Volatile::Read(System.Byte&)", (vm::IntrinsicFunction)&SystemThreadingVolatile::read_byte, read_byte_invoker},
    {"System.Threading.Volatile::Read(System.SByte&)", (vm::IntrinsicFunction)&SystemThreadingVolatile::read_sbyte, read_sbyte_invoker},
    {"System.Threading.Volatile::Read(System.Int16&)", (vm::IntrinsicFunction)&SystemThreadingVolatile::read_i16, read_i16_invoker},
    {"System.Threading.Volatile::Read(System.UInt16&)", (vm::IntrinsicFunction)&SystemThreadingVolatile::read_u16, read_u16_invoker},
    {"System.Threading.Volatile::Read(System.Int32&)", (vm::IntrinsicFunction)&SystemThreadingVolatile::read_i32, read_i32_invoker},
    {"System.Threading.Volatile::Read(System.UInt32&)", (vm::IntrinsicFunction)&SystemThreadingVolatile::read_u32, read_u32_invoker},
    {"System.Threading.Volatile::Read(System.Int64&)", (vm::IntrinsicFunction)&SystemThreadingVolatile::read_i64, read_i64_invoker},
    {"System.Threading.Volatile::Read(System.UInt64&)", (vm::IntrinsicFunction)&SystemThreadingVolatile::read_u64, read_u64_invoker},
    {"System.Threading.Volatile::Read(System.IntPtr&)", (vm::IntrinsicFunction)&SystemThreadingVolatile::read_intptr, read_intptr_invoker},
    {"System.Threading.Volatile::Read(System.UIntPtr&)", (vm::IntrinsicFunction)&SystemThreadingVolatile::read_uintptr, read_uintptr_invoker},
    {"System.Threading.Volatile::Read(System.Single&)", (vm::IntrinsicFunction)&SystemThreadingVolatile::read_f32, read_f32_invoker},
    {"System.Threading.Volatile::Read(System.Double&)", (vm::IntrinsicFunction)&SystemThreadingVolatile::read_f64, read_f64_invoker},
    {"System.Threading.Volatile::Read<>", (vm::IntrinsicFunction)&SystemThreadingVolatile::read_ref, read_ref_invoker},
    // Write operations
    {"System.Threading.Volatile::Write(System.Boolean&,System.Boolean)", (vm::IntrinsicFunction)&SystemThreadingVolatile::write_bool, write_bool_invoker},
    {"System.Threading.Volatile::Write(System.Byte&,System.Byte)", (vm::IntrinsicFunction)&SystemThreadingVolatile::write_byte, write_byte_invoker},
    {"System.Threading.Volatile::Write(System.SByte&,System.SByte)", (vm::IntrinsicFunction)&SystemThreadingVolatile::write_sbyte, write_sbyte_invoker},
    {"System.Threading.Volatile::Write(System.Int16&,System.Int16)", (vm::IntrinsicFunction)&SystemThreadingVolatile::write_i16, write_i16_invoker},
    {"System.Threading.Volatile::Write(System.UInt16&,System.UInt16)", (vm::IntrinsicFunction)&SystemThreadingVolatile::write_u16, write_u16_invoker},
    {"System.Threading.Volatile::Write(System.Int32&,System.Int32)", (vm::IntrinsicFunction)&SystemThreadingVolatile::write_i32, write_i32_invoker},
    {"System.Threading.Volatile::Write(System.UInt32&,System.UInt32)", (vm::IntrinsicFunction)&SystemThreadingVolatile::write_u32, write_u32_invoker},
    {"System.Threading.Volatile::Write(System.Int64&,System.Int64)", (vm::IntrinsicFunction)&SystemThreadingVolatile::write_i64, write_i64_invoker},
    {"System.Threading.Volatile::Write(System.UInt64&,System.UInt64)", (vm::IntrinsicFunction)&SystemThreadingVolatile::write_u64, write_u64_invoker},
    {"System.Threading.Volatile::Write(System.IntPtr&,System.IntPtr)", (vm::IntrinsicFunction)&SystemThreadingVolatile::write_intptr, write_intptr_invoker},
    {"System.Threading.Volatile::Write(System.UIntPtr&,System.UIntPtr)", (vm::IntrinsicFunction)&SystemThreadingVolatile::write_uintptr, write_uintptr_invoker},
    {"System.Threading.Volatile::Write(System.Single&,System.Single)", (vm::IntrinsicFunction)&SystemThreadingVolatile::write_f32, write_f32_invoker},
    {"System.Threading.Volatile::Write(System.Double&,System.Double)", (vm::IntrinsicFunction)&SystemThreadingVolatile::write_f64, write_f64_invoker},
    {"System.Threading.Volatile::Write<>", (vm::IntrinsicFunction)&SystemThreadingVolatile::write_ref, write_ref_invoker},
};

utils::Span<vm::IntrinsicEntry> SystemThreadingVolatile::get_intrinsic_entries() noexcept
{
    constexpr size_t entry_count = sizeof(s_intrinsic_entries_system_threading_volatile) / sizeof(s_intrinsic_entries_system_threading_volatile[0]);
    return utils::Span<vm::IntrinsicEntry>(s_intrinsic_entries_system_threading_volatile, entry_count);
}

} // namespace intrinsics
} // namespace leanclr
