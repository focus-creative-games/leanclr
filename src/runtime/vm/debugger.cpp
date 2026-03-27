#include "debugger.h"
#include "rt_string.h"
#include "settings.h"

namespace leanclr
{
namespace vm
{

void Debugger::log(int32_t level, RtString* category, RtString* message)
{
    auto log_func = Settings::get_debugger_log_function();
    if (log_func != nullptr)
    {
        const Utf16Char* category_str = category ? String::get_chars_ptr(category) : nullptr;
        size_t category_len = category ? static_cast<size_t>(String::get_length(category)) : 0;
        const Utf16Char* message_str = message ? String::get_chars_ptr(message) : nullptr;
        size_t message_len = message ? static_cast<size_t>(String::get_length(message)) : 0;
        log_func(level, category_str, category_len, message_str, message_len);
    }
}

} // namespace vm
} // namespace leanclr
