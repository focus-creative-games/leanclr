#pragma once

#include <stdint.h>
#include <stddef.h>

typedef float float32_t;
typedef double float64_t;

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

#if !NDEBUG
#ifndef LEANCLR_ENABLE_FRAME_TRACE
#define LEANCLR_ENABLE_FRAME_TRACE 0
#endif
#endif

#define LEANCLR_NO_EXCEPTION noexcept

#include "il2cpp/il2cpp_build_config.h"
