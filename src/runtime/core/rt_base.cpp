#include "host/services.h"
#include "rt_base.h"
#include <cstdlib>

namespace leanclr
{
RtErr fatal_on_not_implemented_error()
{
    panic("Not implemented runtime operation");
}

[[noreturn]] void panic(const char* errMsg)
{
    host::Services::log(host::LogLevel::Fatal, errMsg);
    std::abort();
}

void print_not_implemented_error(const char* errMsg)
{
    host::Services::log(host::LogLevel::Error, errMsg);
}
} // namespace leanclr
