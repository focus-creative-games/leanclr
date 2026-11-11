#pragma once

#ifndef LEANCLR_EXPORT
#ifdef _MSC_VER
#define LEANCLR_EXPORT __declspec(dllexport)
#define LEANCLR_IMPORT __declspec(dllimport)
#else
#define LEANCLR_EXPORT __attribute__((visibility("default")))
#define LEANCLR_IMPORT
#endif
#endif

#if defined(LEANCLR_STATIC)
#define LEANCLR_API
#elif defined(LEANCLR_DLL_EXPORTS)
#define LEANCLR_API LEANCLR_EXPORT
#else
#define LEANCLR_API LEANCLR_IMPORT
#endif
