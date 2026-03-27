#include "architecture.h"

#include "vm/rt_string.h"

namespace leanclr
{
namespace os
{
vm::RtString* Architecture::get_architecture_name()
{
#if defined(_M_X64) || defined(__x86_64__)
    return vm::String::create_string_from_utf8cstr("X64");
#elif defined(_M_ARM64) || defined(__aarch64__)
    return vm::String::create_string_from_utf8cstr("Arm64");
#else
    return vm::String::create_string_from_utf8cstr("Unknown");
#endif
}

vm::RtString* Architecture::get_os_name()
{
    return vm::String::get_empty_string();
}
} // namespace os
} // namespace leanclr
