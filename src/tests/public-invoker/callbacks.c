#include "callbacks.h"
#include <stdint.h>
const LeanclrMethodInfo* abi_expected_method;
LeanclrException* abi_exception;
int abi_raise, abi_calls, abi_failures;

void abi_method_marker(void)
{
}

void abi_callback(LeanclrMethodPointer pointer, const LeanclrMethodInfo* method, const LeanclrStackObject* args, LeanclrStackObject* result,
                  LeanclrException** exception)
{
    size_t offset = 0;
    int32_t value = 0;
    ++abi_calls;
    if (pointer != abi_method_marker || method != abi_expected_method || !args || !result || !exception)
    {
        ++abi_failures;
        return;
    }
    leanclr_get_argument(args, &offset, &value, sizeof(value));
    if (value != -12345 || offset != 1)
        ++abi_failures;
    if (abi_raise)
    {
        *exception = abi_exception;
        return;
    }
    value += 17;
    leanclr_set_return_value(result, &value, sizeof(value));
    *exception = 0;
}
