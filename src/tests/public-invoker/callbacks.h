#pragma once
#include "public/leanclr.h"
#ifdef __cplusplus
extern "C"
{
#endif
    extern const LeanclrMethodInfo* abi_expected_method;
    extern LeanclrException* abi_exception;
    extern int abi_raise, abi_calls, abi_failures;
    void abi_method_marker(void);
    void abi_callback(LeanclrMethodPointer pointer, const LeanclrMethodInfo* method, const LeanclrStackObject* args, LeanclrStackObject* result,
                      LeanclrException** exception);
#ifdef __cplusplus
}
#endif
