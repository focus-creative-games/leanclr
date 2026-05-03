#pragma once

// this header must be included before any other runtime headers

#include "build_config.h"
#include "stl_compat.h"
#include "rt_err.h"
#include "rt_result.h"

typedef uint8_t byte;
typedef int8_t sbyte;

namespace leanclr
{

using core::RtErr;
using core::Unit;

template <typename T>
using RtResult = core::Result<T>;

using RtResultVoid = core::ResultVoid;

typedef uint16_t Utf16Char;

constexpr size_t PTR_SIZE = sizeof(void*);
constexpr size_t PTR_ALIGN = PTR_SIZE;

#if LEANCLR_FATAL_ON_RAISE_NOT_IMPLEMENTED_ERROR
RtErr fatal_on_not_implemented_error();

#define RETURN_NOT_IMPLEMENTED_ERROR() RET_ERR(fatal_on_not_implemented_error())
#else
#define RETURN_NOT_IMPLEMENTED_ERROR() RET_ERR(RtErr::NotImplemented)
#endif

#define RET_ASSERT_ERR(err)    \
    do                         \
    {                          \
        assert(false && #err); \
        return err;            \
    } while (0)

} // namespace leanclr
