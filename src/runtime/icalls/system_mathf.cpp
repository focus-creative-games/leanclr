#include "system_mathf.h"

#include <cmath>

namespace leanclr
{
namespace icalls
{

RtResult<float> SystemMathF::round(float value) noexcept
{
    RET_OK(static_cast<float>(std::round(static_cast<double>(value))));
}
/// @icall: System.MathF::Round(System.Single)
static RtResultVoid round_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                  interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<float>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(float, result, SystemMathF::round(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<float> SystemMathF::acos(float value) noexcept
{
    RET_OK(std::acos(value));
}
/// @icall: System.MathF::Acos(System.Single)
static RtResultVoid acos_invoker_mathf(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                       interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<float>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(float, result, SystemMathF::acos(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<float> SystemMathF::acosh(float value) noexcept
{
    RET_OK(static_cast<float>(std::acosh(static_cast<double>(value))));
}
/// @icall: System.MathF::Acosh(System.Single)
static RtResultVoid acosh_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                  interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<float>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(float, result, SystemMathF::acosh(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<float> SystemMathF::asin(float value) noexcept
{
    RET_OK(std::asin(value));
}
/// @icall: System.MathF::Asin(System.Single)
static RtResultVoid asin_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                 interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<float>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(float, result, SystemMathF::asin(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<float> SystemMathF::asinh(float value) noexcept
{
    RET_OK(static_cast<float>(std::asinh(static_cast<double>(value))));
}
/// @icall: System.MathF::Asinh(System.Single)
static RtResultVoid asinh_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                  interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<float>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(float, result, SystemMathF::asinh(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<float> SystemMathF::atan(float value) noexcept
{
    RET_OK(std::atan(value));
}
/// @icall: System.MathF::Atan(System.Single)
static RtResultVoid atan_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                 interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<float>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(float, result, SystemMathF::atan(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<float> SystemMathF::atan2(float y, float x) noexcept
{
    RET_OK(std::atan2(y, x));
}
/// @icall: System.MathF::Atan2(System.Single,System.Single)
static RtResultVoid atan2_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                  interp::RtStackObject* ret) noexcept
{
    auto y = EvalStackOp::get_param<float>(params, 0);
    auto x = EvalStackOp::get_param<float>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(float, result, SystemMathF::atan2(y, x));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<float> SystemMathF::atanh(float value) noexcept
{
    RET_OK(static_cast<float>(std::atanh(static_cast<double>(value))));
}
/// @icall: System.MathF::Atanh(System.Single)
static RtResultVoid atanh_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                  interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<float>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(float, result, SystemMathF::atanh(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<float> SystemMathF::cbrt(float value) noexcept
{
    RET_OK(static_cast<float>(std::cbrt(static_cast<double>(value))));
}
/// @icall: System.MathF::Cbrt(System.Single)
static RtResultVoid cbrt_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                 interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<float>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(float, result, SystemMathF::cbrt(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<float> SystemMathF::ceiling(float value) noexcept
{
    RET_OK(static_cast<float>(std::ceil(static_cast<double>(value))));
}
/// @icall: System.MathF::Ceiling(System.Single)
static RtResultVoid ceiling_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                    interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<float>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(float, result, SystemMathF::ceiling(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<float> SystemMathF::cos(float value) noexcept
{
    RET_OK(std::cos(value));
}
/// @icall: System.MathF::Cos(System.Single)
static RtResultVoid cos_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<float>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(float, result, SystemMathF::cos(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<float> SystemMathF::cosh(float value) noexcept
{
    RET_OK(static_cast<float>(std::cosh(static_cast<double>(value))));
}
/// @icall: System.MathF::Cosh(System.Single)
static RtResultVoid cosh_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                 interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<float>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(float, result, SystemMathF::cosh(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<float> SystemMathF::exp(float value) noexcept
{
    RET_OK(std::exp(value));
}
/// @icall: System.MathF::Exp(System.Single)
static RtResultVoid exp_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<float>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(float, result, SystemMathF::exp(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<float> SystemMathF::floor(float value) noexcept
{
    RET_OK(std::floor(value));
}
/// @icall: System.MathF::Floor(System.Single)
static RtResultVoid floor_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                  interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<float>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(float, result, SystemMathF::floor(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<float> SystemMathF::log(float value) noexcept
{
    RET_OK(std::log(value));
}
/// @icall: System.MathF::Log(System.Single)
static RtResultVoid log_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<float>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(float, result, SystemMathF::log(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<float> SystemMathF::log10(float value) noexcept
{
    RET_OK(std::log10(value));
}
/// @icall: System.MathF::Log10(System.Single)
static RtResultVoid log10_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                  interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<float>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(float, result, SystemMathF::log10(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<float> SystemMathF::pow(float x, float y) noexcept
{
    RET_OK(static_cast<float>(std::pow(static_cast<double>(x), static_cast<double>(y))));
}
/// @icall: System.MathF::Pow(System.Single,System.Single)
static RtResultVoid pow_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                interp::RtStackObject* ret) noexcept
{
    auto x = EvalStackOp::get_param<float>(params, 0);
    auto y = EvalStackOp::get_param<float>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(float, result, SystemMathF::pow(x, y));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<float> SystemMathF::sin(float value) noexcept
{
    RET_OK(std::sin(value));
}
/// @icall: System.MathF::Sin(System.Single)
static RtResultVoid sin_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<float>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(float, result, SystemMathF::sin(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<float> SystemMathF::sinh(float value) noexcept
{
    RET_OK(static_cast<float>(std::sinh(static_cast<double>(value))));
}
/// @icall: System.MathF::Sinh(System.Single)
static RtResultVoid sinh_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                 interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<float>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(float, result, SystemMathF::sinh(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<float> SystemMathF::sqrt(float value) noexcept
{
    RET_OK(static_cast<float>(std::sqrt(static_cast<double>(value))));
}
/// @icall: System.MathF::Sqrt(System.Single)
static RtResultVoid sqrt_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                 interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<float>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(float, result, SystemMathF::sqrt(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<float> SystemMathF::tan(float value) noexcept
{
    RET_OK(std::tan(value));
}
/// @icall: System.MathF::Tan(System.Single)
static RtResultVoid tan_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<float>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(float, result, SystemMathF::tan(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<float> SystemMathF::tanh(float value) noexcept
{
    RET_OK(static_cast<float>(std::tanh(static_cast<double>(value))));
}
/// @icall: System.MathF::Tanh(System.Single)
static RtResultVoid tanh_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                 interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<float>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(float, result, SystemMathF::tanh(value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<float> SystemMathF::fmod(float x, float y) noexcept
{
    RET_OK(static_cast<float>(std::fmod(static_cast<double>(x), static_cast<double>(y))));
}
/// @icall: System.MathF::FMod(System.Single,System.Single)
static RtResultVoid fmod_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                 interp::RtStackObject* ret) noexcept
{
    auto x = EvalStackOp::get_param<float>(params, 0);
    auto y = EvalStackOp::get_param<float>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(float, result, SystemMathF::fmod(x, y));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<float> SystemMathF::modf(float value, float* intpart) noexcept
{
    float int_component = 0.0f;
    float frac = std::modf(value, &int_component);
    if (intpart != nullptr)
        *intpart = int_component;
    RET_OK(frac);
}

/// @icall: System.MathF::ModF(System.Single,System.Single*)
static RtResultVoid modf_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                 interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<float>(params, 0);
    auto intpart_ptr = EvalStackOp::get_param<float*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(float, frac, SystemMathF::modf(value, intpart_ptr));
    EvalStackOp::set_return(ret, frac);
    RET_VOID_OK();
}

static vm::InternalCallEntry s_internal_call_entries_system_mathf[] = {
    {"System.MathF::Round(System.Single)", (vm::InternalCallFunction)&SystemMathF::round, round_invoker},
    {"System.MathF::Acos(System.Single)", (vm::InternalCallFunction)&SystemMathF::acos, acos_invoker_mathf},
    {"System.MathF::Acosh(System.Single)", (vm::InternalCallFunction)&SystemMathF::acosh, acosh_invoker},
    {"System.MathF::Asin(System.Single)", (vm::InternalCallFunction)&SystemMathF::asin, asin_invoker},
    {"System.MathF::Asinh(System.Single)", (vm::InternalCallFunction)&SystemMathF::asinh, asinh_invoker},
    {"System.MathF::Atan(System.Single)", (vm::InternalCallFunction)&SystemMathF::atan, atan_invoker},
    {"System.MathF::Atan2(System.Single,System.Single)", (vm::InternalCallFunction)&SystemMathF::atan2, atan2_invoker},
    {"System.MathF::Atanh(System.Single)", (vm::InternalCallFunction)&SystemMathF::atanh, atanh_invoker},
    {"System.MathF::Cbrt(System.Single)", (vm::InternalCallFunction)&SystemMathF::cbrt, cbrt_invoker},
    {"System.MathF::Ceiling(System.Single)", (vm::InternalCallFunction)&SystemMathF::ceiling, ceiling_invoker},
    {"System.MathF::Cos(System.Single)", (vm::InternalCallFunction)&SystemMathF::cos, cos_invoker},
    {"System.MathF::Cosh(System.Single)", (vm::InternalCallFunction)&SystemMathF::cosh, cosh_invoker},
    {"System.MathF::Exp(System.Single)", (vm::InternalCallFunction)&SystemMathF::exp, exp_invoker},
    {"System.MathF::Floor(System.Single)", (vm::InternalCallFunction)&SystemMathF::floor, floor_invoker},
    {"System.MathF::Log(System.Single)", (vm::InternalCallFunction)&SystemMathF::log, log_invoker},
    {"System.MathF::Log10(System.Single)", (vm::InternalCallFunction)&SystemMathF::log10, log10_invoker},
    {"System.MathF::Pow(System.Single,System.Single)", (vm::InternalCallFunction)&SystemMathF::pow, pow_invoker},
    {"System.MathF::Sin(System.Single)", (vm::InternalCallFunction)&SystemMathF::sin, sin_invoker},
    {"System.MathF::Sinh(System.Single)", (vm::InternalCallFunction)&SystemMathF::sinh, sinh_invoker},
    {"System.MathF::Sqrt(System.Single)", (vm::InternalCallFunction)&SystemMathF::sqrt, sqrt_invoker},
    {"System.MathF::Tan(System.Single)", (vm::InternalCallFunction)&SystemMathF::tan, tan_invoker},
    {"System.MathF::Tanh(System.Single)", (vm::InternalCallFunction)&SystemMathF::tanh, tanh_invoker},
    {"System.MathF::FMod(System.Single,System.Single)", (vm::InternalCallFunction)&SystemMathF::fmod, fmod_invoker},
    {"System.MathF::ModF(System.Single,System.Single*)", (vm::InternalCallFunction)&SystemMathF::modf, modf_invoker},
};

utils::Span<vm::InternalCallEntry> SystemMathF::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_mathf,
                                              sizeof(s_internal_call_entries_system_mathf) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr
