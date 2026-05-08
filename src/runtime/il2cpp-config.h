#pragma once

#if !defined(IL2CPP_EXPORT)
#ifdef _MSC_VER
#include <malloc.h>
#define IL2CPP_EXPORT __declspec(dllexport)
#define IL2CPP_IMPORT __declspec(dllimport)
#else
#define IL2CPP_EXPORT __attribute__((visibility("default")))
#define IL2CPP_IMPORT
#endif
#endif


#if _MSC_VER
#ifndef STDCALL
#define STDCALL __stdcall
#endif
#ifndef CDECL
#define CDECL __cdecl
#endif
#ifndef FASTCALL
#define FASTCALL __fastcall
#endif
#ifndef THISCALL
#define THISCALL __thiscall
#endif
#else
#define STDCALL
#define CDECL
#define FASTCALL
#define THISCALL
#endif
