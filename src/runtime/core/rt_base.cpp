#include "rt_base.h"

namespace leanclr
{
RtErr fatal_on_not_implemented_error()
{
    assert(false);
    // crash the program
    int* p = (int*)-1;
    *p = 0;
    return RtErr::NotImplemented;
}

void print_not_implemented_error(const char* errMsg)
{
    printf("Not implemented error: %s\n", errMsg);
}
} // namespace leanclr