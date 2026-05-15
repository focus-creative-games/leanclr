#pragma once

#include "rt_metadata.h"

namespace leanclr
{
namespace metadata
{

enum class RtMarshalNativeType
{
    Boolean = 0x2,
    I1 = 0x3,
    U1 = 0x4,
    I2 = 0x5,
    U2 = 0x6,
    I4 = 0x7,
    U4 = 0x8,
    I8 = 0x9,
    U8 = 0xA,
    R4 = 0xB,
    R8 = 0xC,
    LPSTR = 0x14,
    LPWSTR = 0x15,
    Int = 0X1f,
    Uint = 0X20,
    Func = 0x26,
    Array = 0x2a,
    Max = 0x50, // NOT DEFINED IN ecma-335, the implementation of coreclr is 0x50
};

constexpr uint32_t RT_INVALID_PARAM_INDEX_OR_ELEMENT_COUNT = 0xFFFFFFFF;

struct RtMarshalSpec
{
    RtMarshalNativeType native_type;
    RtMarshalNativeType array_element_type;
    uint32_t param_index;
    uint32_t element_count;
};

struct RtSafeArrayBound
{
    uint32_t element_count;
    int32_t lower_bound;
};

struct RtSafeArray
{
    uint16_t dimension_count;
    uint16_t features;
    uint32_t element_size;
    uint32_t lock_count;
    void* data;
    RtSafeArrayBound bounds[1];
};

union RtMarshalCurrency
{
    struct
    {
        uint32_t low;
        uint32_t high;
    };

    int64_t i64;
};

typedef int32_t RtMarshalBoolean;
typedef int16_t RtMarshalVariantBool;
typedef void* RtMarshalCustomMarshaler;
typedef int32_t RtMarshalError;
typedef void* RtMarshalIInspectable;
typedef void* RtMarshalIUnknown;
typedef void* RtMarshalIDispatch;
typedef void* RtMarshalInterface;
typedef void* RtMarshalHString;
typedef char* RtMarshalUTF8Str;
typedef Utf16Char* RtMarshalUTF16Str;
typedef NativeChar* RtMarshalAnsiStr;
typedef intptr_t RtMarshalHandle;

} // namespace metadata
} // namespace leanclr