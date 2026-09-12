#include "rt_base.h"

namespace leanclr
{
RtErr fatal_on_not_implemented_error()
{
    return fatal_on_not_implemented_error(nullptr, 0);
}

RtErr fatal_on_not_implemented_error(const char* file, int line)
{
    if (file != nullptr)
    {
        printf("Not implemented error at %s:%d\n", file, line);
        fflush(stdout);
    }
    assert(false);
    // crash the program
    int* p = (int*)-1;
    *p = 0;
    return RtErr::NotImplemented;
}

void panic(const char* errMsg)
{
    printf("Panic: %s\n", errMsg);
    int* p = (int*)-1;
    *p = 0;
    // crash the program
}

void print_not_implemented_error(const char* errMsg)
{
    printf("Not implemented error: %s\n", errMsg);
}
} // namespace leanclr
