#pragma once

#include <stdint.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <vector>

#if !defined(IL2CPP_EXPORT)
#ifdef _MSC_VER
# include <malloc.h>
# define IL2CPP_EXPORT __declspec(dllexport)
# define IL2CPP_IMPORT __declspec(dllimport)
#else
# define IL2CPP_EXPORT __attribute__ ((visibility ("default")))
# define IL2CPP_IMPORT
#endif
#endif

#define IL2CPP_API_DYNAMIC_NO_DLSYM 1
#if IL2CPP_API_DYNAMIC_NO_DLSYM
#if defined(__cplusplus)
extern "C"
{
#endif // __cplusplus
IL2CPP_EXPORT void il2cpp_api_register_symbols(void);
IL2CPP_EXPORT void* il2cpp_api_lookup_symbol(const char* name);
#if defined(__cplusplus)
}
#endif // __cplusplus
#endif

#ifdef _MSC_VER
#define NORETURN __declspec(noreturn)
#elif defined(__GNUC__) || defined(__clang__)
#define NORETURN __attribute__((noreturn))
#else
#define NORETURN
#endif

#include "il2cpp_api_types.h"

#include "vm/runtime.h"
#include "vm/settings.h"
#include "vm/class.h"
#include "vm/object.h"
#include "vm/rt_array.h"
#include "vm/array_class.h"
#include "vm/field.h"
#include "vm/method.h"
#include "vm/rt_string.h"
#include "vm/reflection.h"
#include "vm/assembly.h"
#include "vm/rt_exception.h"
#include "vm/monitor.h"
#include "vm/property.h"
#include "vm/gc.h"
#include "vm/gchandle.h"
#include "vm/type.h"
#include "vm/appdomain.h"
#include "vm/rt_thread.h"
#include "vm/customattribute.h"
#include "vm/stacktrace.h"
#include "vm/internal_calls.h"
#include "utils/string_builder.h"
#include "metadata/module_def.h"
#include "metadata/metadata_compare.h"
#include "metadata/metadata_cache.h"
#include "gc/garbage_collector.h"
#include "interp/machine_state.h"
#include "fileloader.h"
#include "statistic.h"
#include "liveness.h"
#include "profiler.h"
#include "runtime.h"
#include "stacktrace.h"
#include "debugger.h"
#include "unityengine.h"

using namespace leanclr::il2cpp;

typedef vm::GCMode Il2CppGCMode;
typedef vm::AotExceptionWrapper Il2CppExceptionWrapper;
typedef il2cpp::Il2CppStat Il2CppStat;
typedef alloc::MemoryCallbacks Il2CppMemoryCallbacks;

extern leanclr::metadata::RtAotModulesData g_aot_modules_data;

#if defined(__cplusplus)
extern "C"
{
#endif // __cplusplus
#define DO_API(r, n, p)             IL2CPP_EXPORT r n p;
#define DO_API_NO_RETURN(r, n, p)   IL2CPP_EXPORT NORETURN r n p;
#include "il2cpp-api-functions.h"
#undef DO_API
#undef DO_API_NO_RETURN
#if defined(__cplusplus)
}
#endif // __cplusplus
