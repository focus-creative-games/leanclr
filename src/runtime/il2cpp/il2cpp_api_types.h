#pragma once

#include "vm/rt_managed_types.h"
#include "interp/machine_state.h"

namespace leanclr
{
namespace il2cpp
{

typedef leanclr::Utf16Char Il2CppChar;
typedef metadata::RtClass Il2CppClass;
typedef vm::RtArray Il2CppArray;
typedef vm::RtObject Il2CppObject;
typedef vm::RtString Il2CppString;
typedef vm::RtException Il2CppException;
typedef metadata::RtModuleDef Il2CppImage;
typedef metadata::RtAssembly Il2CppAssembly;
typedef vm::RtAppDomain Il2CppDomain;
typedef vm::RtThread Il2CppThread;
typedef metadata::RtTypeSig Il2CppType;
typedef vm::RtReflectionType Il2CppReflectionType;
typedef vm::RtReflectionMethod Il2CppReflectionMethod;
typedef vm::RtReflectionField Il2CppReflectionField;
typedef metadata::RtFieldInfo FieldInfo;
typedef metadata::RtMethodInfo MethodInfo;
typedef metadata::RtPropertyInfo PropertyInfo;
typedef metadata::RtEventInfo EventInfo;

// Opaque types that have no leanclr equivalent (forward declarations).
struct Il2CppCustomAttrInfo;
struct Il2CppDebuggerTransport;
struct Il2CppManagedMemorySnapshot;
struct Il2CppMethodDebugInfo;
struct Il2CppProfiler;
struct Il2CppRuntimeStats;
typedef interp::InterpFrame Il2CppStackFrameInfo;

// <= Unity 2022.3.x
#if UNITY_VERSION < 60000000
typedef uint32_t Il2CppGCHandle;
#define LEANCLR_USE_VOID_PTR_GCHANDLE 0
#else
// >= Unity 6000.0.0
typedef void* Il2CppGCHandle;
#define LEANCLR_USE_VOID_PTR_GCHANDLE 1
#endif

#if _MSC_VER
typedef wchar_t Il2CppNativeChar;
#else
typedef char Il2CppNativeChar;
#endif

typedef enum
{
    IL2CPP_PROFILE_NONE = 0,
    IL2CPP_PROFILE_ENTER_LEAVE = 1 << 12,
    IL2CPP_PROFILE_ALLOCATIONS = 1 << 7,
    IL2CPP_PROFILE_GC = 1 << 8,
    IL2CPP_PROFILE_FILEIO = 1 << 20,
    IL2CPP_PROFILE_THREADS = 1 << 9,
} Il2CppProfileFlags;

typedef enum
{
    IL2CPP_PROFILE_FILEIO_WRITE = 0,
    IL2CPP_PROFILE_FILEIO_READ
} Il2CppProfileFileIOKind;

typedef enum
{
    IL2CPP_GC_EVENT_START,
    IL2CPP_GC_EVENT_MARK_START,
    IL2CPP_GC_EVENT_MARK_END,
    IL2CPP_GC_EVENT_RECLAIM_START,
    IL2CPP_GC_EVENT_RECLAIM_END,
    IL2CPP_GC_EVENT_END,
    IL2CPP_GC_EVENT_PRE_STOP_WORLD,
    IL2CPP_GC_EVENT_POST_STOP_WORLD,
    IL2CPP_GC_EVENT_PRE_START_WORLD,
    IL2CPP_GC_EVENT_POST_START_WORLD
} Il2CppGCEvent;

typedef enum
{
    IL2CPP_UNHANDLED_POLICY_LEGACY,
    IL2CPP_UNHANDLED_POLICY_CURRENT
} Il2CppRuntimeUnhandledExceptionPolicy;

struct Il2CppMetadataField
{
    uint32_t offset;
    uint32_t typeIndex;
    const char* name;
    bool isStatic;
};

enum Il2CppMetadataTypeFlags
{
    kNone = 0,
    kValueType = 1 << 0,
    kArray = 1 << 1,
    kArrayRankMask = 0xFFFF0000
};

struct Il2CppMetadataType
{
    Il2CppMetadataTypeFlags flags; // If it's an array, rank is encoded in the upper 2 bytes
    Il2CppMetadataField* fields;
    uint32_t fieldCount;
    uint32_t staticsSize;
    uint8_t* statics;
    uint32_t baseOrElementTypeIndex;
    char* name;
    const char* assemblyName;
    uint64_t typeInfoAddress;
    uint32_t size;
};

struct Il2CppMetadataSnapshot
{
    uint32_t typeCount;
    Il2CppMetadataType* types;
};

struct Il2CppManagedMemorySection
{
    uint64_t sectionStartAddress;
    uint32_t sectionSize;
    uint8_t* sectionBytes;
};

struct Il2CppManagedHeap
{
    uint32_t sectionCount;
    Il2CppManagedMemorySection* sections;
};

struct Il2CppStacks
{
    uint32_t stackCount;
    Il2CppManagedMemorySection* stacks;
};

struct NativeObject
{
    uint32_t gcHandleIndex;
    uint32_t size;
    uint32_t instanceId;
    uint32_t classId;
    uint32_t referencedNativeObjectIndicesCount;
    uint32_t* referencedNativeObjectIndices;
};

struct Il2CppGCHandles
{
    uint32_t trackedObjectCount;
    uint64_t* pointersToObjects;
};

struct Il2CppRuntimeInformation
{
    uint32_t pointerSize;
    uint32_t objectHeaderSize;
    uint32_t arrayHeaderSize;
    uint32_t arrayBoundsOffsetInHeader;
    uint32_t arraySizeOffsetInHeader;
    uint32_t allocationGranularity;
};

struct Il2CppManagedMemorySnapshot
{
    Il2CppManagedHeap heap;
    Il2CppStacks stacks;
    Il2CppMetadataSnapshot metadata;
    Il2CppGCHandles gcHandles;
    Il2CppRuntimeInformation runtimeInformation;
    void* additionalUserInformation;
};

typedef void (*il2cpp_register_object_callback)(Il2CppObject** arr, int size, void* userdata);
typedef void* (*il2cpp_liveness_reallocate_callback)(void* ptr, size_t size, void* userdata);
typedef void (*Il2CppFrameWalkFunc)(const Il2CppStackFrameInfo* info, void* user_data);
typedef void (*Il2CppProfileFunc)(Il2CppProfiler* prof);
typedef void (*Il2CppProfileMethodFunc)(Il2CppProfiler* prof, const MethodInfo* method);
typedef void (*Il2CppProfileAllocFunc)(Il2CppProfiler* prof, Il2CppObject* obj, Il2CppClass* klass);
typedef void (*Il2CppProfileGCFunc)(Il2CppProfiler* prof, Il2CppGCEvent event, int generation);
typedef void (*Il2CppProfileGCResizeFunc)(Il2CppProfiler* prof, int64_t new_size);
typedef void (*Il2CppProfileFileIOFunc)(Il2CppProfiler* prof, Il2CppProfileFileIOKind kind, int count);
typedef void (*Il2CppProfileThreadFunc)(Il2CppProfiler* prof, unsigned long tid);

typedef const Il2CppNativeChar* (*Il2CppSetFindPlugInCallback)(const Il2CppNativeChar*);
typedef void (*Il2CppLogCallback)(const char*);

typedef void (*Il2CppMethodPointer)();
typedef size_t (*Il2CppBacktraceFunc)(Il2CppMethodPointer* buffer, size_t maxSize);

typedef uintptr_t il2cpp_array_size_t;
#define ARRAY_LENGTH_AS_INT32(a) ((int32_t)(a))

typedef uint8_t (*Il2CppAndroidUpStateFunc)(const char* ifName, uint8_t* is_up);
#define IL2CPP_ENABLE_PROFILER 1
} // namespace il2cpp
} // namespace leanclr