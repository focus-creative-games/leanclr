#include "bcl/coreclr-net10/platform/rt_sys.h"
#include <cstdlib>
#include <cstdio>
using leanclr::platform::RtSys;

static const leanclr::Utf16Char* u(const char16_t* text)
{
    return reinterpret_cast<const leanclr::Utf16Char*>(text);
}

#define CHECK(x)                                      \
    do                                                \
    {                                                 \
        if (!(x))                                     \
        {                                             \
            std::fprintf(stderr, "failed: %s\n", #x); \
            return 1;                                 \
        }                                             \
    } while (0)

int main()
{
    auto name = u(u"LEANCLR_ENV_BUFFER_CONTRACT");
    CHECK(RtSys::set_environment_variable(name, u(u"A\u4e2d\U0001f600")) != 0);
    leanclr::Utf16Char buffer[8] = {0x7777};
    CHECK(RtSys::get_environment_variable(name, nullptr, 0) == 5);
    CHECK(RtSys::get_environment_variable(name, buffer, 4) == 5 && buffer[0] == 0x7777);
    CHECK(RtSys::get_environment_variable(name, buffer, 5) == 4);
    CHECK(buffer[0] == 'A' && buffer[1] == 0x4e2d && buffer[2] == 0xd83d && buffer[3] == 0xde00 && buffer[4] == 0);
    CHECK(RtSys::set_environment_variable(name, nullptr) != 0);
    CHECK(RtSys::get_environment_variable(name, buffer, 8) == 0 && RtSys::get_last_win32_error() == 203);
    CHECK(RtSys::get_environment_variable(nullptr, buffer, 8) == 0 && RtSys::get_last_win32_error() == 87);
    return 0;
}
