#include "system_math.h"

#include <cmath>

namespace leanclr
{
namespace icalls
{

RtResult<double> SystemMath::round(double value) noexcept
{
    RET_OK(std::round(value));
}

/// @icall: System.Math::Round(System.Double)
static RtResultVoid round_invoker_math(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                       interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto value = EvalStackOp::get_param<double>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(double, result, SystemMath::round(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<float> SystemMath::abs_f32(float value) noexcept
{
    RET_OK(static_cast<float>(std::fabs(value)));
}

/// @icall: System.Math::Abs(System.Single)
static RtResultVoid abs_f32_invoker_math(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                         interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto value = EvalStackOp::get_param<float>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(float, result, SystemMath::abs_f32(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<double> SystemMath::abs_f64(double value) noexcept
{
    RET_OK(std::fabs(value));
}

/// @icall: System.Math::Abs(System.Double)
static RtResultVoid abs_f64_invoker_math(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                         interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto value = EvalStackOp::get_param<double>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(double, result, SystemMath::abs_f64(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<double> SystemMath::acos(double value) noexcept
{
    RET_OK(std::acos(value));
}
/// @icall: System.Math::Acos(System.Double)
static RtResultVoid acos_invoker_math(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<double>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(double, result, SystemMath::acos(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<double> SystemMath::acosh(double value) noexcept
{
    RET_OK(std::acosh(value));
}
/// @icall: System.Math::Acosh(System.Double)
static RtResultVoid acosh_invoker_math(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                       interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<double>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(double, result, SystemMath::acosh(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<double> SystemMath::asin(double value) noexcept
{
    RET_OK(std::asin(value));
}
/// @icall: System.Math::Asin(System.Double)
static RtResultVoid asin_invoker_math(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<double>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(double, result, SystemMath::asin(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<double> SystemMath::asinh(double value) noexcept
{
    RET_OK(std::asinh(value));
}
/// @icall: System.Math::Asinh(System.Double)
static RtResultVoid asinh_invoker_math(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                       interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<double>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(double, result, SystemMath::asinh(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<double> SystemMath::atan(double value) noexcept
{
    RET_OK(std::atan(value));
}
/// @icall: System.Math::Atan(System.Double)
static RtResultVoid atan_invoker_math(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<double>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(double, result, SystemMath::atan(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<double> SystemMath::atan2(double y, double x) noexcept
{
    RET_OK(std::atan2(y, x));
}
/// @icall: System.Math::Atan2(System.Double,System.Double)
static RtResultVoid atan2_invoker_math(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                       interp::RtStackObject* ret) noexcept
{
    auto y = EvalStackOp::get_param<double>(params, 0);
    auto x = EvalStackOp::get_param<double>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(double, result, SystemMath::atan2(y, x));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<double> SystemMath::atanh(double value) noexcept
{
    RET_OK(std::atanh(value));
}
/// @icall: System.Math::Atanh(System.Double)
static RtResultVoid atanh_invoker_math(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                       interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<double>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(double, result, SystemMath::atanh(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<double> SystemMath::cbrt(double value) noexcept
{
    RET_OK(std::cbrt(value));
}
/// @icall: System.Math::Cbrt(System.Double)
static RtResultVoid cbrt_invoker_math(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<double>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(double, result, SystemMath::cbrt(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<double> SystemMath::ceiling(double value) noexcept
{
    RET_OK(std::ceil(value));
}
/// @icall: System.Math::Ceiling(System.Double)
static RtResultVoid ceiling_invoker_math(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                         interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<double>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(double, result, SystemMath::ceiling(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<double> SystemMath::cos(double value) noexcept
{
    RET_OK(std::cos(value));
}
/// @icall: System.Math::Cos(System.Double)
static RtResultVoid cos_invoker_math(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                     interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<double>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(double, result, SystemMath::cos(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<double> SystemMath::cosh(double value) noexcept
{
    RET_OK(std::cosh(value));
}
/// @icall: System.Math::Cosh(System.Double)
static RtResultVoid cosh_invoker_math(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<double>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(double, result, SystemMath::cosh(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<double> SystemMath::exp(double value) noexcept
{
    RET_OK(std::exp(value));
}
/// @icall: System.Math::Exp(System.Double)
static RtResultVoid exp_invoker_math(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                     interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<double>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(double, result, SystemMath::exp(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<double> SystemMath::floor(double value) noexcept
{
    RET_OK(std::floor(value));
}
/// @icall: System.Math::Floor(System.Double)
static RtResultVoid floor_invoker_math(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                       interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<double>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(double, result, SystemMath::floor(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<double> SystemMath::log(double value) noexcept
{
    RET_OK(std::log(value));
}
/// @icall: System.Math::Log(System.Double)
static RtResultVoid log_invoker_math(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                     interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<double>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(double, result, SystemMath::log(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<double> SystemMath::log10(double value) noexcept
{
    RET_OK(std::log10(value));
}
/// @icall: System.Math::Log10(System.Double)
static RtResultVoid log10_invoker_math(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                       interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<double>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(double, result, SystemMath::log10(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<double> SystemMath::pow(double x, double y) noexcept
{
    RET_OK(std::pow(x, y));
}
/// @icall: System.Math::Pow(System.Double,System.Double)
static RtResultVoid pow_invoker_math(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                     interp::RtStackObject* ret) noexcept
{
    auto x = EvalStackOp::get_param<double>(params, 0);
    auto y = EvalStackOp::get_param<double>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(double, result, SystemMath::pow(x, y));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<double> SystemMath::sin(double value) noexcept
{
    RET_OK(std::sin(value));
}
/// @icall: System.Math::Sin(System.Double)
static RtResultVoid sin_invoker_math(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                     interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<double>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(double, result, SystemMath::sin(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<double> SystemMath::sinh(double value) noexcept
{
    RET_OK(std::sinh(value));
}
/// @icall: System.Math::Sinh(System.Double)
static RtResultVoid sinh_invoker_math(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<double>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(double, result, SystemMath::sinh(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<double> SystemMath::sqrt(double value) noexcept
{
    RET_OK(std::sqrt(value));
}
/// @icall: System.Math::Sqrt(System.Double)
static RtResultVoid sqrt_invoker_math(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<double>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(double, result, SystemMath::sqrt(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<double> SystemMath::tan(double value) noexcept
{
    RET_OK(std::tan(value));
}
/// @icall: System.Math::Tan(System.Double)
static RtResultVoid tan_invoker_math(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                     interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<double>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(double, result, SystemMath::tan(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<double> SystemMath::tanh(double value) noexcept
{
    RET_OK(std::tanh(value));
}
/// @icall: System.Math::Tanh(System.Double)
static RtResultVoid tanh_invoker_math(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<double>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(double, result, SystemMath::tanh(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<double> SystemMath::fmod(double x, double y) noexcept
{
    RET_OK(std::fmod(x, y));
}
/// @icall: System.Math::FMod(System.Double,System.Double)
static RtResultVoid fmod_invoker_math(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    auto x = EvalStackOp::get_param<double>(params, 0);
    auto y = EvalStackOp::get_param<double>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(double, result, SystemMath::fmod(x, y));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<double> SystemMath::modf(double value, double* intpart) noexcept
{
    double int_component = 0.0;
    double frac = std::modf(value, &int_component);
    if (intpart != nullptr)
        *intpart = int_component;
    RET_OK(frac);
}

/// @icall: System.Math::ModF(System.Double,System.Double*)
static RtResultVoid modf_invoker_math(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<double>(params, 0);
    auto intpart_ptr = EvalStackOp::get_param<double*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(double, frac, SystemMath::modf(value, intpart_ptr));
    EvalStackOp::set_return(ret, frac);
    RET_VOID_OK();
}

static vm::InternalCallEntry s_entries_system_math[] = {
    {"System.Math::Round(System.Double)", (vm::InternalCallFunction)&SystemMath::round, round_invoker_math},
    {"System.Math::Abs(System.Single)", (vm::InternalCallFunction)&SystemMath::abs_f32, abs_f32_invoker_math},
    {"System.Math::Abs(System.Double)", (vm::InternalCallFunction)&SystemMath::abs_f64, abs_f64_invoker_math},
    {"System.Math::Acos(System.Double)", (vm::InternalCallFunction)&SystemMath::acos, acos_invoker_math},
    {"System.Math::Acosh(System.Double)", (vm::InternalCallFunction)&SystemMath::acosh, acosh_invoker_math},
    {"System.Math::Asin(System.Double)", (vm::InternalCallFunction)&SystemMath::asin, asin_invoker_math},
    {"System.Math::Asinh(System.Double)", (vm::InternalCallFunction)&SystemMath::asinh, asinh_invoker_math},
    {"System.Math::Atan(System.Double)", (vm::InternalCallFunction)&SystemMath::atan, atan_invoker_math},
    {"System.Math::Atan2(System.Double,System.Double)", (vm::InternalCallFunction)&SystemMath::atan2, atan2_invoker_math},
    {"System.Math::Atanh(System.Double)", (vm::InternalCallFunction)&SystemMath::atanh, atanh_invoker_math},
    {"System.Math::Cbrt(System.Double)", (vm::InternalCallFunction)&SystemMath::cbrt, cbrt_invoker_math},
    {"System.Math::Ceiling(System.Double)", (vm::InternalCallFunction)&SystemMath::ceiling, ceiling_invoker_math},
    {"System.Math::Cos(System.Double)", (vm::InternalCallFunction)&SystemMath::cos, cos_invoker_math},
    {"System.Math::Cosh(System.Double)", (vm::InternalCallFunction)&SystemMath::cosh, cosh_invoker_math},
    {"System.Math::Exp(System.Double)", (vm::InternalCallFunction)&SystemMath::exp, exp_invoker_math},
    {"System.Math::Floor(System.Double)", (vm::InternalCallFunction)&SystemMath::floor, floor_invoker_math},
    {"System.Math::Log(System.Double)", (vm::InternalCallFunction)&SystemMath::log, log_invoker_math},
    {"System.Math::Log10(System.Double)", (vm::InternalCallFunction)&SystemMath::log10, log10_invoker_math},
    {"System.Math::Pow(System.Double,System.Double)", (vm::InternalCallFunction)&SystemMath::pow, pow_invoker_math},
    {"System.Math::Sin(System.Double)", (vm::InternalCallFunction)&SystemMath::sin, sin_invoker_math},
    {"System.Math::Sinh(System.Double)", (vm::InternalCallFunction)&SystemMath::sinh, sinh_invoker_math},
    {"System.Math::Sqrt(System.Double)", (vm::InternalCallFunction)&SystemMath::sqrt, sqrt_invoker_math},
    {"System.Math::Tan(System.Double)", (vm::InternalCallFunction)&SystemMath::tan, tan_invoker_math},
    {"System.Math::Tanh(System.Double)", (vm::InternalCallFunction)&SystemMath::tanh, tanh_invoker_math},
    {"System.Math::FMod(System.Double,System.Double)", (vm::InternalCallFunction)&SystemMath::fmod, fmod_invoker_math},
    {"System.Math::ModF(System.Double,System.Double*)", (vm::InternalCallFunction)&SystemMath::modf, modf_invoker_math},
};

utils::Span<vm::InternalCallEntry> SystemMath::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_entries_system_math, sizeof(s_entries_system_math) / sizeof(s_entries_system_math[0]));
}

} // namespace icalls
} // namespace leanclr
