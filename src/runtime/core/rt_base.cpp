#include "rt_base.h"

namespace leanclr
{
#if LEANCLR_FATAL_ON_RAISE_NOT_IMPLEMENTED_ERROR
RtErr fatal_on_not_implemented_error()
{
    assert(false);
    // crash the program
    int* p = (int*)-1;
    *p = 0;
    return RtErr::NotImplemented;
}
#endif

void print_not_implemented_error(const char* errMsg)
{
    printf("Not implemented error: %s\n", errMsg);
}
} // namespace leanclr