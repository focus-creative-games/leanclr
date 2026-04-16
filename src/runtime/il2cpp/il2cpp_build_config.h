#pragma once

// Platform detection macros

// Default all to 0
#define IL2CPP_TARGET_WINDOWS 0
#define IL2CPP_TARGET_ANDROID 0
#define IL2CPP_TARGET_IOS 0
#define IL2CPP_TARGET_OSX 0
#define IL2CPP_TARGET_JAVASCRIPT 0
#define IL2CPP_TARGET_UNKNOWN 0

// Set the appropriate platform to 1
#if defined(_WIN32) || defined(_WIN64)
#undef IL2CPP_TARGET_WINDOWS
#define IL2CPP_TARGET_WINDOWS 1
#elif defined(__ANDROID__)
#undef IL2CPP_TARGET_ANDROID
#define IL2CPP_TARGET_ANDROID 1
#elif defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>
#if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#undef IL2CPP_TARGET_IOS
#define IL2CPP_TARGET_IOS 1
#elif defined(TARGET_OS_MAC) && TARGET_OS_MAC
#undef IL2CPP_TARGET_OSX
#define IL2CPP_TARGET_OSX 1
#endif
#elif defined(__EMSCRIPTEN__)
#undef IL2CPP_TARGET_JAVASCRIPT
#define IL2CPP_TARGET_JAVASCRIPT 1
#else
#undef IL2CPP_TARGET_UNKNOWN
#define IL2CPP_TARGET_UNKNOWN 1
#endif