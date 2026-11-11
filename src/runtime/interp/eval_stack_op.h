#pragma once

#include "interp_defs.h"

namespace leanclr
{
namespace interp
{
class EvalStackOp
{
  public:
    static vm::RtObject* get_this(const RtStackObject* params)
    {
        return *(vm::RtObject**)params;
    }

    // Helper function to get parameter from stack object array
    template <typename T>
    static T get_param(const RtStackObject* params, size_t index)
    {
        // This is a simplified implementation - actual implementation depends on RtStackObject layout
        return *reinterpret_cast<const T*>(params + index);
    }

    template <typename T>
    static void set_param(RtStackObject* params, size_t index, const T& value)
    {
        *reinterpret_cast<T*>(params + index) = value;
    }

    // Helper function to set return value
    template <typename T>
    static void set_return(RtStackObject* ret, const T& value)
    {
        // CLI evaluation-stack scalars narrower than Int32 are represented as
        // Int32. A byte-only write leaves stale argument bits in the return
        // slot and can turn false into true in the caller's branch instruction.
        if constexpr ((std::is_integral_v<T> || std::is_enum_v<T>) && sizeof(T) < sizeof(int32_t))
            ret->i32 = static_cast<int32_t>(value);
        else
            *reinterpret_cast<T*>(ret) = value;
    }
};

} // namespace interp
} // namespace leanclr
