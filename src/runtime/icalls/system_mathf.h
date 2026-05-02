#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemMathF
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    static RtResult<float> round(float value) noexcept;
    static RtResult<float> acos(float value) noexcept;
    static RtResult<float> acosh(float value) noexcept;
    static RtResult<float> asin(float value) noexcept;
    static RtResult<float> asinh(float value) noexcept;
    static RtResult<float> atan(float value) noexcept;
    static RtResult<float> atan2(float y, float x) noexcept;
    static RtResult<float> atanh(float value) noexcept;
    static RtResult<float> cbrt(float value) noexcept;
    static RtResult<float> ceiling(float value) noexcept;
    static RtResult<float> cos(float value) noexcept;
    static RtResult<float> cosh(float value) noexcept;
    static RtResult<float> exp(float value) noexcept;
    static RtResult<float> floor(float value) noexcept;
    static RtResult<float> log(float value) noexcept;
    static RtResult<float> log10(float value) noexcept;
    static RtResult<float> pow(float x, float y) noexcept;
    static RtResult<float> sin(float value) noexcept;
    static RtResult<float> sinh(float value) noexcept;
    static RtResult<float> sqrt(float value) noexcept;
    static RtResult<float> tan(float value) noexcept;
    static RtResult<float> tanh(float value) noexcept;
    static RtResult<float> fmod(float x, float y) noexcept;
    static RtResult<float> modf(float value, float* intpart) noexcept;
};

} // namespace icalls
} // namespace leanclr
