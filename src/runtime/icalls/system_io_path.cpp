#include "system_io_path.h"

#include "vm/rt_string.h"

#include <cstdlib>
#include <cstring>

#ifdef LEANCLR_PLATFORM_WIN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace leanclr
{
namespace icalls
{

/// @icall: System.IO.Path::get_temp_path
RtResult<vm::RtString*> SystemIOPath::get_temp_path()
{
#ifdef LEANCLR_PLATFORM_WIN
    wchar_t buffer[MAX_PATH + 1];
    DWORD len = ::GetTempPathW(MAX_PATH + 1, buffer);
    if (len > 0 && len <= MAX_PATH)
    {
        static_assert(sizeof(wchar_t) == sizeof(Utf16Char), "wchar_t must be 16-bit on Windows");
        RET_OK(vm::String::create_string_from_utf16chars(reinterpret_cast<const uint16_t*>(buffer), static_cast<int32_t>(len)));
    }
    RET_OK(vm::String::create_string_from_utf8cstr("C:\\Temp\\"));
#else
    const char* tmp = std::getenv("TMPDIR");
    if (!tmp || !tmp[0])
        tmp = std::getenv("TMP");
    if (!tmp || !tmp[0])
        tmp = std::getenv("TEMP");
    if (!tmp || !tmp[0])
        tmp = "/tmp/";

    size_t tmp_len = std::strlen(tmp);
    if (tmp_len > 0 && tmp[tmp_len - 1] != '/')
    {
        // Ensure trailing path separator
        char buffer[1024];
        if (tmp_len + 2 > sizeof(buffer))
            tmp_len = sizeof(buffer) - 2;
        std::memcpy(buffer, tmp, tmp_len);
        buffer[tmp_len] = '/';
        buffer[tmp_len + 1] = 0;
        RET_OK(vm::String::create_string_from_utf8chars(buffer, static_cast<int32_t>(tmp_len + 1)));
    }
    RET_OK(vm::String::create_string_from_utf8cstr(tmp));
#endif
}

static RtResultVoid get_temp_path_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                          interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, s, SystemIOPath::get_temp_path());
    EvalStackOp::set_return(ret, s);
    RET_VOID_OK();
}

static vm::InternalCallEntry s_internal_call_entries_system_io_path[] = {
    {"System.IO.Path::get_temp_path", (vm::InternalCallFunction)&SystemIOPath::get_temp_path, get_temp_path_invoker},
};

utils::Span<vm::InternalCallEntry> SystemIOPath::get_internal_call_entries()
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_io_path,
                                              sizeof(s_internal_call_entries_system_io_path) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr
