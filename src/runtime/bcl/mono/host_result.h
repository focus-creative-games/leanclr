#pragma once
#include "core/rt_base.h"
#include "host/services.h"

namespace leanclr
{
namespace bcl
{
inline RtErr host_error(int32_t error)
{
    return error == host::NotSupported ? RtErr::NotSupported : error == 87 ? RtErr::Argument : error == 2 ? RtErr::FileNotFound : RtErr::InvalidOperation;
}
} // namespace bcl
} // namespace leanclr
