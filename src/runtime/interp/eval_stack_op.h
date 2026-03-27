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
        *reinterpret_cast<T*>(ret) = value;
    }
};

} // namespace interp
} // namespace leanclr
