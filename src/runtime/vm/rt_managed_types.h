#pragma once

#include "metadata/rt_metadata.h"
#include "metadata/rt_marshal_types.h"

namespace leanclr
{
namespace vm
{
// BCL-managed objects are opaque to the execution core. Definitions belong to
// the selected adapter, never to this header.
struct ArrayBounds
{
    int32_t length;
    int32_t lower_bound;
};
struct RtMarshalByRefObject;
struct RtReflectionType;
struct RtReflectionRuntimeType;
struct RtReflectionField;
struct RtReflectionMethod;
struct RtMonoMethodInfo;
struct RtMonoPropertyInfo;
struct RtReflectionProperty;
struct RtReflectionEvent;
struct RtReflectionEventInfo;
struct RtReflectionParameter;
struct RtReflectionModule;
struct RtReflectionAssembly;
struct RtReflectionAssemblyName;
struct RtCustomAttribute;
struct RtDelegateData;
struct RtDelegate;
struct RtMulticastDelegate;
struct RtAppDomain;
struct RtAppContext;
struct RtMonoAppDomain;
struct RtException;
struct RtCultureData;
struct RtCalendarData;
struct RtDateTimeFormatInfo;
struct RtNumberFormatInfo;
struct RtCultureInfo;
struct RtRegionInfo;
struct RtStackFrame;
struct RtReflectionExceptionHandlingClause;
struct RtReflectionLocalVariableInfo;
struct RtReflectionMethodBody;
struct RtReflectionMonoEventInfo;
struct RtNativeThread;
struct RtInternalThread;
struct RtThread;

using RtReflectionMonoType = RtReflectionRuntimeType;
using RtReflectionGenericMethod = RtReflectionMethod;
#if LEANCLR_BCL_CORECLR_NET10
struct RtReflectionConstructor;
struct RtSignature;
#else
using RtReflectionConstructor = RtReflectionMethod;
#endif

struct RtObject
{
    const metadata::RtClass* klass;
    void* __sync_block;
};

struct RtArray : public RtObject
{
    const ArrayBounds* bounds;
    int32_t length;
    uint64_t first_data;
};

struct RtString : RtObject
{
    int32_t length;
    Utf16Char first_char;
};

struct RtTypedReference
{
#if LEANCLR_BCL_CORECLR_NET10
    const void* type_handle;
#else
    const metadata::RtTypeSig* type_handle;
#endif
    const void* value;
    const metadata::RtClass* klass;
};

template <typename T>
struct RtReadOnlySpan
{
    const T* pointer;
    int32_t length;
};

enum class RtThreadState : int32_t
{
    Running = 0x0,
    StopRequested = 0x1,
    SuspendRequested = 0x2,
    Background = 0x4,
    Unstarted = 0x8,
    Stopped = 0x10,
    WaitSleepJoin = 0x20,
    Suspended = 0x40,
    AbortRequested = 0x80,
    Aborted = 0x100,
};

enum class RtStackCrawlMark : int32_t
{
    LookForMe,
    LookForMyCaller,
    LookForMyCallersCaller,
    LookForThread,
};

// Constants for managed types
constexpr uint32_t RT_OBJECT_HEADER_SIZE = sizeof(RtObject);

// NOTE: RtArray is not a standard-layout type (both RtObject and RtArray have
// non-static data members), so offsetof on it is "conditionally-supported" per
// C++17. In practice the layout is stable on every toolchain we ship
// (MSVC/GCC/Clang/Emscripten), so we suppress the resulting warning here.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4840)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
constexpr uint32_t RT_STRING_FIRST_CHAR_OFFSET = offsetof(RtString, first_char);
constexpr uint32_t RT_ARRAY_HEADER_SIZE = offsetof(RtArray, first_data);
constexpr uint32_t RT_ARRAY_FIRST_DATA_OFFSET = offsetof(RtArray, first_data);
constexpr uint32_t RT_ARRAY_LENGTH_OFFSET = offsetof(RtArray, length);
constexpr uint32_t RT_ARRAY_BOUNDS_OFFSET = offsetof(RtArray, bounds);
#ifdef _MSC_VER
#pragma warning(pop)
#else
#pragma GCC diagnostic pop
#endif

const int32_t RT_MAX_ARRAY_INDEX = INT32_MAX;
const int32_t RT_MAX_ARRAY_RANK = 32;

constexpr uint32_t RT_TYPED_REFERENCE_SIZE = sizeof(RtTypedReference);
constexpr size_t RT_PUBLIC_KEY_BYTES_LEN = 8;
constexpr size_t RT_PUBLIC_KEY_TOKEN_HEX_STRING_WITH_NULL_TERMINATOR_LENGTH = 17;

} // namespace vm
} // namespace leanclr

namespace leanclr
{
namespace core
{
// Only the LSB tagging guarantee is part of the core contract. Exact alignment
// and fields are adapter details; its layout checks validate this guarantee.
#define LEANCLR_OPAQUE_RESULT_ALIGNMENT(Type, Alignment) \
    template <>                                          \
    struct pointer_alignment<vm::Type>                   \
    {                                                    \
        static const size_t value = Alignment;           \
    };                                                   \
    template <>                                          \
    struct pointer_alignment<const vm::Type>             \
    {                                                    \
        static const size_t value = Alignment;           \
    };
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtMarshalByRefObject, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtReflectionType, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtReflectionRuntimeType, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtReflectionField, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtReflectionMethod, 2)
#if LEANCLR_BCL_CORECLR_NET10
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtReflectionConstructor, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtSignature, 2)
#endif
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtMonoMethodInfo, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtMonoPropertyInfo, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtReflectionProperty, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtReflectionEvent, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtReflectionEventInfo, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtReflectionParameter, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtReflectionModule, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtReflectionAssembly, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtReflectionAssemblyName, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtCustomAttribute, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtDelegateData, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtDelegate, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtMulticastDelegate, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtAppDomain, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtAppContext, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtMonoAppDomain, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtException, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtCultureData, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtCalendarData, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtDateTimeFormatInfo, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtNumberFormatInfo, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtCultureInfo, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtRegionInfo, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtStackFrame, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtReflectionExceptionHandlingClause, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtReflectionLocalVariableInfo, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtReflectionMethodBody, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtReflectionMonoEventInfo, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtNativeThread, 1)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtInternalThread, 2)
LEANCLR_OPAQUE_RESULT_ALIGNMENT(RtThread, 2)
#undef LEANCLR_OPAQUE_RESULT_ALIGNMENT
} // namespace core
} // namespace leanclr
