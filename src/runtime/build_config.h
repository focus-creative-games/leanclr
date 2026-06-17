#pragma once

#include <stdint.h>
#include <stddef.h>

typedef float float32_t;
typedef double float64_t;

// ---------------------------------------------------------------------------
// Platform detection
//
// Derived from compiler-predefined macros so both CMake- and non-CMake-based
// consumers of this header see a consistent view. The order matters:
//   * __ANDROID__ is checked before __linux__ because Android also defines
//     __linux__.
//   * __EMSCRIPTEN__ is checked before __linux__ for the same reason on some
//     toolchains.
// POSIX platforms additionally define LEANCLR_PLATFORM_POSIX.
// ---------------------------------------------------------------------------
#if defined(_WIN32)
#define LEANCLR_PLATFORM_WIN 1
#elif defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#define LEANCLR_PLATFORM_IOS 1
#else
#define LEANCLR_PLATFORM_MAC 1
#endif
#define LEANCLR_PLATFORM_POSIX 1
#elif defined(__ANDROID__)
#define LEANCLR_PLATFORM_ANDROID 1
#define LEANCLR_PLATFORM_POSIX 1
#elif defined(__OHOS__) || defined(__HARMONYOS__)
#define LEANCLR_PLATFORM_OHOS 1
#define LEANCLR_PLATFORM_POSIX 1
#elif defined(__EMSCRIPTEN__)
#define LEANCLR_PLATFORM_WASM 1
#define LEANCLR_PLATFORM_POSIX 1
#elif defined(__linux__)
#define LEANCLR_PLATFORM_LINUX 1
#define LEANCLR_PLATFORM_POSIX 1
#else
#define LEANCLR_PLATFORM_UNKNOWN 1
#endif

// Native aligned heap: Windows CRT (_aligned_*), or POSIX posix_memalign where
// we can query usable size for aligned_realloc (Apple / glibc / bionic). Other
// targets use a portable header-based aligned allocator in general_allocation.cpp.
#if LEANCLR_PLATFORM_WIN
#define LEANCLR_USE_POSIX_ALIGNED_HEAP 0
#elif LEANCLR_PLATFORM_POSIX && !LEANCLR_PLATFORM_WASM && (defined(__APPLE__) || defined(__ANDROID__) || (defined(__linux__) && !defined(__MUSL__)))
#define LEANCLR_USE_POSIX_ALIGNED_HEAP 1
#else
#define LEANCLR_USE_POSIX_ALIGNED_HEAP 0
#endif

#if defined(DEBUG) || defined(_DEBUG)
#define LEANCLR_DEBUG 1
#else
#define LEANCLR_DEBUG 0
#endif

#define LEANCLR_SUPPORT_UNALIGNED_ACCESS 1

#if UINTPTR_MAX == 0xFFFFFFFF
#define LEANCLR_ARCH_32BIT 1
#else
#define LEANCLR_ARCH_64BIT 1
#endif

#if (defined(__GNUC__) || defined(__clang__)) && !defined(__EMSCRIPTEN__)
#define LEANCLR_USE_COMPUTED_GOTO_DISPATCHER 1
#else
#define LEANCLR_USE_COMPUTED_GOTO_DISPATCHER 0
#endif

#if LEANCLR_DEBUG
#define LEANCLR_ENABLE_TEST_PINVOKES 1
#define LEANCLR_ENABLE_TEST_INTRINSICS 1
#define LEANCLR_ENABLE_TEST_INTERNAL_CALLS 1
#endif

#define LEANCLR_ENABLE_FRAME_TRACE 0

#if LEANCLR_DEBUG
#ifndef LEANCLR_ENABLE_FRAME_TRACE
#define LEANCLR_ENABLE_FRAME_TRACE 0
#endif
#endif

#define LEANCLR_NO_EXCEPTION noexcept

#define LEANCLR_FATAL_ON_RAISE_NOT_IMPLEMENTED_ERROR 1

// ---------------------------------------------------------------------------
// Branch prediction and optimization assumptions (C++11)
//
// LEANCLR_LIKELY / LEANCLR_UNLIKELY
//   Wrap boolean conditions. On GCC/Clang/ICC (GNU compat), expands to
//   __builtin_expect; elsewhere expands to a no-op cast.
//
// LEANCLR_ASSUME(expr)
//   Tells the optimizer that (expr) is true at this program point. If (expr)
//   is false at runtime, the program has undefined behavior (MSVC __assume;
//   GCC/Clang via __builtin_unreachable). Use only when (expr) is already
//   guaranteed by prior logic (e.g. after a successful allocation check).
//
// LEANCLR_ASSUME_NON_NULL(ptr) / LEANCLR_ASSUME_NOT_ZERO(value)
//   Convenience wrappers around LEANCLR_ASSUME for pointers and scalar non-zero.
// ---------------------------------------------------------------------------
#if defined(__GNUC__) || defined(__clang__)
#define LEANCLR_LIKELY(x) (__builtin_expect(!!(x), 1))
#define LEANCLR_UNLIKELY(x) (__builtin_expect(!!(x), 0))
#else
#define LEANCLR_LIKELY(x) (x)
#define LEANCLR_UNLIKELY(x) (x)
#endif

#ifndef LIKELY
#define LIKELY(x) LEANCLR_LIKELY(x)
#endif
#ifndef UNLIKELY
#define UNLIKELY(x) LEANCLR_UNLIKELY(x)
#endif

#if defined(_MSC_VER) && !defined(__clang__)
#define LEANCLR_ASSUME(expr) __assume(!!(expr))
#elif defined(__GNUC__) || defined(__clang__)
#define LEANCLR_ASSUME(expr)         \
    do                               \
    {                                \
        if (!(expr))                 \
            __builtin_unreachable(); \
    } while (0)
#else
#define LEANCLR_ASSUME(expr) ((void)0)
#endif

#define LEANCLR_ASSUME_NOT_NULL(ptr) LEANCLR_ASSUME(ptr)

// ---------------------------------------------------------------------------
// P/Invoke native calling conventions
//
// LeanAOT emits typedefs / extern declarations using these macros so the
// generated signature matches DllImport CallingConvention across MSVC,
// Clang, and GCC. On 32-bit x86 Windows, cdecl/stdcall/thiscall/fastcall
// differ; on x64, ARM, Wasm, and Unix 32/64-bit targets they usually
// collapse to a single C ABI (macros expand empty on those targets).
// ---------------------------------------------------------------------------
#if LEANCLR_PLATFORM_WIN && (defined(_M_IX86) || defined(__i386__))
#define LEANCLR_PINVOKE_ABI_WIN_X86 1
#else
#define LEANCLR_PINVOKE_ABI_WIN_X86 0
#endif

#if defined(_MSC_VER)
#define LEANCLR_PINVOKE_CALL_CDECL __cdecl
#define LEANCLR_PINVOKE_CALL_STDCALL __stdcall
#define LEANCLR_PINVOKE_CALL_FASTCALL __fastcall
#define LEANCLR_PINVOKE_CALL_THISCALL __thiscall
#elif defined(__clang__) || defined(__GNUC__)
#if LEANCLR_PINVOKE_ABI_WIN_X86
#define LEANCLR_PINVOKE_CALL_CDECL __attribute__((cdecl))
#define LEANCLR_PINVOKE_CALL_STDCALL __attribute__((stdcall))
#define LEANCLR_PINVOKE_CALL_FASTCALL __attribute__((fastcall))
#define LEANCLR_PINVOKE_CALL_THISCALL __attribute__((thiscall))
#else
#define LEANCLR_PINVOKE_CALL_CDECL
#define LEANCLR_PINVOKE_CALL_STDCALL
#define LEANCLR_PINVOKE_CALL_FASTCALL
#define LEANCLR_PINVOKE_CALL_THISCALL
#endif
#else
#define LEANCLR_PINVOKE_CALL_CDECL
#define LEANCLR_PINVOKE_CALL_STDCALL
#define LEANCLR_PINVOKE_CALL_FASTCALL
#define LEANCLR_PINVOKE_CALL_THISCALL
#endif

#if LEANCLR_PLATFORM_WIN
#define LEANCLR_PINVOKE_CALL_WINAPI LEANCLR_PINVOKE_CALL_STDCALL
#else
#define LEANCLR_PINVOKE_CALL_WINAPI LEANCLR_PINVOKE_CALL_CDECL
#endif

#ifndef LEANCLR_USE_SHIPPING_ZLIB
#define LEANCLR_USE_SHIPPING_ZLIB 0
#endif

#ifndef LEANCLR_GC_DEBUG
#define LEANCLR_GC_DEBUG 0
#endif

#ifndef LEANCLR_PGO_PROFILE
#define LEANCLR_PGO_PROFILE 0
#endif

#if LEANCLR_GC_DEBUG
#define LEANCLR_GC_DECLARE_CALL_SITE_PARAM , const ::leanclr::gc::GcAllocSite& site
#define LEANCLR_GC_CALL_SITE_PARAM , site
#else
#define LEANCLR_GC_DECLARE_CALL_SITE_PARAM
#define LEANCLR_GC_CALL_SITE_PARAM
#endif

#if !defined(LEANCLR_GC_MARK_SWEEP) && !defined(LEANCLR_GC_ZERO_GC)
#define LEANCLR_GC_MARK_SWEEP 1
#endif

#define LEANCLR_STRINGIFY(x) #x
#define LEANCLR_TOSTRING(x) LEANCLR_STRINGIFY(x)

// UNITY_VERSION (and optionally UNITY_TUANJIE_ENGINE) must be supplied as
// C preprocessor defines for the libil2cpp compile, via IL2CPP additional
// arguments (--compiler-flags). LeanCLR's editor preprocess applies this
// automatically; see UpdateUnityVersionMacrosInCpp.
#ifndef UNITY_VERSION
#error "UNITY_VERSION must be defined for LeanCLR (IL2CPP --compiler-flags / PlayerSettings additional IL2CPP args)."
#endif
