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
typedef char Utf8Char;
// LPStr / CharSet.Ansi: null-terminated byte string in the system ACP (Windows) or UTF-8 bytes (POSIX).
typedef char AnsiChar;

constexpr size_t PTR_SIZE = sizeof(void*);
constexpr size_t PTR_ALIGN = PTR_SIZE;

RtErr fatal_on_not_implemented_error();

#if LEANCLR_FATAL_ON_RAISE_NOT_IMPLEMENTED_ERROR
#define RETURN_NOT_IMPLEMENTED_ERROR() RET_ERR(fatal_on_not_implemented_error())
#else
#define RETURN_NOT_IMPLEMENTED_ERROR() RET_ERR(RtErr::NotImplemented)
#endif

void print_not_implemented_error(const char* errMsg);
void panic(const char* errMsg);

#define WARN_NOT_IMPLEMENTED_ERROR_THEN_RETURN_OK(value, errMsg) \
    do                                                           \
    {                                                            \
        print_not_implemented_error(errMsg);                     \
        RET_OK(value);                                           \
    } while (0)

#define WARN_NOT_IMPLEMENTED_ERROR_THEN_RETURN_VOID(errMsg) \
    do                                                      \
    {                                                       \
        print_not_implemented_error(errMsg);                \
        RET_VOID_OK();                                      \
    } while (0)

#define RET_ASSERT_ERR(err)    \
    do                         \
    {                          \
        assert(false && #err); \
        return err;            \
    } while (0)

} // namespace leanclr
