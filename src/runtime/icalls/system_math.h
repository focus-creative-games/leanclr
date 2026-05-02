#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemMath
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    static RtResult<double> round(double value) noexcept;
    static RtResult<float> abs_f32(float value) noexcept;
    static RtResult<double> abs_f64(double value) noexcept;
    static RtResult<double> acos(double value) noexcept;
    static RtResult<double> acosh(double value) noexcept;
    static RtResult<double> asin(double value) noexcept;
    static RtResult<double> asinh(double value) noexcept;
    static RtResult<double> atan(double value) noexcept;
    static RtResult<double> atan2(double y, double x) noexcept;
    static RtResult<double> atanh(double value) noexcept;
    static RtResult<double> cbrt(double value) noexcept;
    static RtResult<double> ceiling(double value) noexcept;
    static RtResult<double> cos(double value) noexcept;
    static RtResult<double> cosh(double value) noexcept;
    static RtResult<double> exp(double value) noexcept;
    static RtResult<double> floor(double value) noexcept;
    static RtResult<double> log(double value) noexcept;
    static RtResult<double> log10(double value) noexcept;
    static RtResult<double> pow(double x, double y) noexcept;
    static RtResult<double> sin(double value) noexcept;
    static RtResult<double> sinh(double value) noexcept;
    static RtResult<double> sqrt(double value) noexcept;
    static RtResult<double> tan(double value) noexcept;
    static RtResult<double> tanh(double value) noexcept;
    static RtResult<double> fmod(double x, double y) noexcept;
    static RtResult<double> modf(double value, double* intpart) noexcept;
};

} // namespace icalls
} // namespace leanclr
