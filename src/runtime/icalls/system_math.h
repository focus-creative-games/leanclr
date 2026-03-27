#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemMath
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries();

    static RtResult<double> round(double value);
    static RtResult<float> abs_f32(float value);
    static RtResult<double> abs_f64(double value);
    static RtResult<double> acos(double value);
    static RtResult<double> acosh(double value);
    static RtResult<double> asin(double value);
    static RtResult<double> asinh(double value);
    static RtResult<double> atan(double value);
    static RtResult<double> atan2(double y, double x);
    static RtResult<double> atanh(double value);
    static RtResult<double> cbrt(double value);
    static RtResult<double> ceil(double value);
    static RtResult<double> cos(double value);
    static RtResult<double> cosh(double value);
    static RtResult<double> exp(double value);
    static RtResult<double> floor(double value);
    static RtResult<double> log(double value);
    static RtResult<double> log10(double value);
    static RtResult<double> pow(double x, double y);
    static RtResult<double> sin(double value);
    static RtResult<double> sinh(double value);
    static RtResult<double> sqrt(double value);
    static RtResult<double> tan(double value);
    static RtResult<double> tanh(double value);
    static RtResult<double> fmod(double x, double y);
    static RtResult<double> modf(double value, double* intpart);
};

} // namespace icalls
} // namespace leanclr
