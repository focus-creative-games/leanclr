#pragma once

#include "rt_metadata.h"

namespace leanclr
{
namespace metadata
{

enum class RtMarshalNativeType : uint32_t
{
    // Deprecated
    End = 0x00,
    // void
    Void = 0x01,
    // bool
    Boolean = 0x02,
    // int8
    I1 = 0x03,
    // unsigned int8
    U1 = 0x04,
    // int16
    I2 = 0x05,
    // unsigned int16
    U2 = 0x06,
    // int32
    I4 = 0x07,
    // unsigned int32
    U4 = 0x08,
    // int64
    I8 = 0x09,
    // unsigned int64
    U8 = 0x0A,
    // float32
    R4 = 0x0B,
    // float64
    R8 = 0x0C,
    // syschar
    SysChar = 0x0D,
    // variant
    Variant = 0x0E,
    // currency
    Currency = 0x0F,
    // ptr
    Ptr = 0x10,
    // decimal
    Decimal = 0x11,
    // date
    Date = 0x12,
    // bstr
    BStr = 0x13,
    // lpstr
    LPStr = 0x14,
    // lpwstr
    LPWStr = 0x15,
    // lptstr
    LPTStr = 0x16,
    // fixed sysstring
    FixedSysString = 0x17,
    // objectref
    ObjectRef = 0x18,
    // iunknown
    IUnknown = 0x19,
    // idispatch
    IDispatch = 0x1A,
    // struct
    Struct = 0x1B,
    // interface
    IntF = 0x1C,
    // safearray
    SafeArray = 0x1D,
    // fixed array
    FixedArray = 0x1E,
    // int
    Int = 0x1F,
    // uint
    UInt = 0x20,
    // nested struct
    NestedStruct = 0x21,
    // byvalstr
    ByValStr = 0x22,
    // ansi bstr
    ANSIBStr = 0x23,
    // tbstr
    TBStr = 0x24,
    // variant bool
    VariantBool = 0x25,
    // func
    Func = 0x26,
    // as any
    ASAny = 0x28,
    // array
    Array = 0x2A,
    // lpstruct
    LPStruct = 0x2B,
    // custom marshaler
    CustomMarshaler = 0x2C,
    // error
    Error = 0x2D,
    // iinspectable
    IInspectable = 0x2E,
    // hstring
    HString = 0x2F,
    // UTF-8 encoded string
    LPUTF8Str = 0x30,
    // first invalid element type
    Max = 0x50,
    NotInitialized = 0xFFFFFFFE,
};

constexpr uint32_t RT_INVALID_PARAM_INDEX_OR_ELEMENT_COUNT = 0xFFFFFFFF;

enum class RtMarshalVariantType : uint32_t
{
    /// <summary/>
    Empty = 0,
    /// <summary/>
    None = 0,
    /// <summary/>
    Null = 1,
    /// <summary/>
    I2 = 2,
    /// <summary/>
    I4 = 3,
    /// <summary/>
    R4 = 4,
    /// <summary/>
    R8 = 5,
    /// <summary/>
    CY = 6,
    /// <summary/>
    Date = 7,
    /// <summary/>
    BStr = 8,
    /// <summary/>
    Dispatch = 9,
    /// <summary/>
    Error = 10,
    /// <summary/>
    Bool = 11,
    /// <summary/>
    Variant = 12,
    /// <summary/>
    Unknown = 13,
    /// <summary/>
    Decimal = 14,
    /// <summary/>
    I1 = 16,
    /// <summary/>
    UI1 = 17,
    /// <summary/>
    UI2 = 18,
    /// <summary/>
    UI4 = 19,
    /// <summary/>
    I8 = 20,
    /// <summary/>
    UI8 = 21,
    /// <summary/>
    Int = 22,
    /// <summary/>
    UInt = 23,
    /// <summary/>
    Void = 24,
    /// <summary/>
    HResult = 25,
    /// <summary/>
    Ptr = 26,
    /// <summary/>
    SafeArray = 27,
    /// <summary/>
    CArray = 28,
    /// <summary/>
    UserDefined = 29,
    /// <summary/>
    LPStr = 30,
    /// <summary/>
    LPWStr = 31,
    /// <summary/>
    Record = 36,
    /// <summary/>
    IntPtr = 37,
    /// <summary/>
    UIntPtr = 38,
    /// <summary/>
    FileTime = 64,
    /// <summary/>
    Blob = 65,
    /// <summary/>
    Stream = 66,
    /// <summary/>
    Storage = 67,
    /// <summary/>
    StreamedObject = 68,
    /// <summary/>
    StoredObject = 69,
    /// <summary/>
    BlobObject = 70,
    /// <summary/>
    CF = 71,
    /// <summary/>
    CLSID = 72,
    /// <summary/>
    VersionedStream = 73,
    /// <summary/>
    BStrBlob = 0x0FFF,
    /// <summary/>
    Vector = 0x1000,
    /// <summary/>
    Array = 0x2000,
    /// <summary/>
    ByRef = 0x4000,
    /// <summary/>
    Reserved = 0x8000,
    /// <summary/>
    Illegal = 0xFFFF,
    /// <summary/>
    IllegalMasked = 0x0FFF,
    /// <summary/>
    TypeMask = 0x0FFF,
    /// <summary>This wasn't present in the blob</summary>
    NotInitialized = 0xFFFFFFFF,
};

struct RtMarshalUtf8Span
{
    const char* data;
    size_t length;

    bool is_valid() const
    {
        return length != (size_t)-1;
    }

    bool is_invalid() const
    {
        return length == (size_t)-1;
    }

    void set_invalid()
    {
        length = (size_t)-1;
    }
};

struct RtMarshalSpec
{
    RtMarshalNativeType native_type;

    union
    {
        uint32_t size;
    } fixed_sys_string;

    union
    {
        RtMarshalVariantType variant_type;
        const RtTypeSig* udt;
    } safe_array;

    union
    {
        uint32_t size;
        RtMarshalNativeType array_element_type;
    } fixed_array;

    union
    {
        RtMarshalNativeType array_element_type;
        uint32_t param_index;
        uint32_t element_count;
    } array;

    union
    {
        RtMarshalUtf8Span guid;
        RtMarshalUtf8Span type_name;
        const RtTypeSig* custom_marshaler_type;
        RtMarshalUtf8Span cookie;
    } custom_marshaler;

    union
    {
        uint32_t iid_param_index;
    } interface;
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