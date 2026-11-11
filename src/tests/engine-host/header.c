#include "public/engine_host.h"

/* Compile this as C, independently of C++ VM/BCL headers. */
int engine_header_contract(void)
{
    LeanclrEngineError error = {0};
    return leanclr_engine_tick(NULL, 0, &error) == LEANCLR_ENGINE_INVALID_ARGUMENT && error.status == LEANCLR_ENGINE_INVALID_ARGUMENT;
}
