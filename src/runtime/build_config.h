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
#elif defined(__EMSCRIPTEN__)
#define LEANCLR_PLATFORM_WASM 1
#elif defined(__linux__)
#define LEANCLR_PLATFORM_LINUX 1
#define LEANCLR_PLATFORM_POSIX 1
#else
#define LEANCLR_PLATFORM_UNKNOWN 1
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

#if !NDEBUG
#define LEANCLR_ENABLE_TEST_PINVOKES 1
#define LEANCLR_ENABLE_TEST_INTRINSICS 1
#define LEANCLR_ENABLE_TEST_INTERNAL_CALLS 1
#endif

#define LEANCLR_ENABLE_FRAME_TRACE 0

#if !NDEBUG
#ifndef LEANCLR_ENABLE_FRAME_TRACE
#define LEANCLR_ENABLE_FRAME_TRACE 0
#endif
#endif

#define LEANCLR_NO_EXCEPTION noexcept

#include "il2cpp/unityversion.h"