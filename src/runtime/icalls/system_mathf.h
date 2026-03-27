#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemMathF
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries();

    static RtResult<float> acos(float value);
    static RtResult<float> acosh(float value);
    static RtResult<float> asin(float value);
    static RtResult<float> asinh(float value);
    static RtResult<float> atan(float value);
    static RtResult<float> atan2(float y, float x);
    static RtResult<float> atanh(float value);
    static RtResult<float> cbrt(float value);
    static RtResult<float> ceiling(float value);
    static RtResult<float> cos(float value);
    static RtResult<float> cosh(float value);
    static RtResult<float> exp(float value);
    static RtResult<float> floor(float value);
    static RtResult<float> log(float value);
    static RtResult<float> log10(float value);
    static RtResult<float> pow(float x, float y);
    static RtResult<float> sin(float value);
    static RtResult<float> sinh(float value);
    static RtResult<float> sqrt(float value);
    static RtResult<float> tan(float value);
    static RtResult<float> tanh(float value);
    static RtResult<float> fmod(float x, float y);
    static RtResult<float> modf(float value, float* intpart);
};

} // namespace icalls
} // namespace leanclr
