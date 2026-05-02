#include "system_threading_volatile.h"

namespace leanclr
{
namespace icalls
{

RtResult<int64_t> SystemThreadingVolatile::read_i64(const int64_t* location) noexcept
{
    RET_OK(*location);
}

RtResult<uint64_t> SystemThreadingVolatile::read_u64(const uint64_t* location) noexcept
{
    RET_OK(*location);
}

RtResult<double> SystemThreadingVolatile::read_f64(const double* location) noexcept
{
    RET_OK(*location);
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

RtResultVoid SystemThreadingVolatile::write_f64(double* location, double value) noexcept
{
    *location = value;
    RET_VOID_OK();
}

/// @icall: System.Threading.Volatile::Read(System.Int64&)
static RtResultVoid read_i64_icall_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                           const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto location = EvalStackOp::get_param<const int64_t*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int64_t, value, SystemThreadingVolatile::read_i64(location));
    EvalStackOp::set_return(ret, value);
    RET_VOID_OK();
}

/// @icall: System.Threading.Volatile::Read(System.UInt64&)
static RtResultVoid read_u64_icall_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                           const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto location = EvalStackOp::get_param<const uint64_t*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint64_t, value, SystemThreadingVolatile::read_u64(location));
    EvalStackOp::set_return(ret, value);
    RET_VOID_OK();
}

/// @icall: System.Threading.Volatile::Read(System.Double&)
static RtResultVoid read_f64_icall_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                           const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto location = EvalStackOp::get_param<const double*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(double, value, SystemThreadingVolatile::read_f64(location));
    EvalStackOp::set_return(ret, value);
    RET_VOID_OK();
}

/// @icall: System.Threading.Volatile::Write(System.Int64&,System.Int64)
static RtResultVoid write_i64_icall_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                            const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    auto location = EvalStackOp::get_param<int64_t*>(params, 0);
    auto value = EvalStackOp::get_param<int64_t>(params, 1);
    RET_ERR_ON_FAIL(SystemThreadingVolatile::write_i64(location, value));
    RET_VOID_OK();
}

/// @icall: System.Threading.Volatile::Write(System.UInt64&,System.UInt64)
static RtResultVoid write_u64_icall_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                            const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    auto location = EvalStackOp::get_param<uint64_t*>(params, 0);
    auto value = EvalStackOp::get_param<uint64_t>(params, 1);
    RET_ERR_ON_FAIL(SystemThreadingVolatile::write_u64(location, value));
    RET_VOID_OK();
}

/// @icall: System.Threading.Volatile::Write(System.Double&,System.Double)
static RtResultVoid write_f64_icall_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                            const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    auto location = EvalStackOp::get_param<double*>(params, 0);
    auto value = EvalStackOp::get_param<double>(params, 1);
    RET_ERR_ON_FAIL(SystemThreadingVolatile::write_f64(location, value));
    RET_VOID_OK();
}

utils::Span<vm::InternalCallEntry> SystemThreadingVolatile::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.Threading.Volatile::Read(System.Int64&)", (vm::InternalCallFunction)&SystemThreadingVolatile::read_i64, read_i64_icall_invoker},
        {"System.Threading.Volatile::Read(System.UInt64&)", (vm::InternalCallFunction)&SystemThreadingVolatile::read_u64, read_u64_icall_invoker},
        {"System.Threading.Volatile::Read(System.Double&)", (vm::InternalCallFunction)&SystemThreadingVolatile::read_f64, read_f64_icall_invoker},
        {"System.Threading.Volatile::Write(System.Int64&,System.Int64)", (vm::InternalCallFunction)&SystemThreadingVolatile::write_i64,
         write_i64_icall_invoker},
        {"System.Threading.Volatile::Write(System.UInt64&,System.UInt64)", (vm::InternalCallFunction)&SystemThreadingVolatile::write_u64,
         write_u64_icall_invoker},
        {"System.Threading.Volatile::Write(System.Double&,System.Double)", (vm::InternalCallFunction)&SystemThreadingVolatile::write_f64,
         write_f64_icall_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr
