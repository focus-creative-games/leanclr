#pragma once

// this header must be included before any other runtime headers

#include "build_config.h"
#include "core/stl_compat.h"
#include "core/rt_err.h"
#include "core/rt_result.h"

typedef uint8_t byte;
typedef int8_t sbyte;

namespace leanclr
{

using core::RtErr;
using core::Unit;

template <typename T>
using RtResult = core::Result<T, RtErr>;

using RtResultVoid = RtResult<Unit>;

typedef uint16_t Utf16Char;

constexpr size_t PTR_SIZE = sizeof(void*);
constexpr size_t PTR_ALIGN = PTR_SIZE;

} // namespace leanclr
