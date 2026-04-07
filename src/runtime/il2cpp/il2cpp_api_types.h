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
typedef metadata::RtFieldInfo FieldInfo;
typedef metadata::RtMethodInfo MethodInfo;
typedef metadata::RtPropertyInfo PropertyInfo;
typedef metadata::RtEventInfo EventInfo;

// Opaque types that have no leanclr equivalent (forward declarations).
struct Il2CppCustomAttrInfo;
struct Il2CppDebuggerTransport;
struct Il2CppManagedMemorySnapshot;
struct Il2CppMemoryCallbacks;
struct Il2CppMethodDebugInfo;
struct Il2CppProfiler;
struct Il2CppRuntimeStats;
typedef interp::InterpFrame Il2CppStackFrameInfo;

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