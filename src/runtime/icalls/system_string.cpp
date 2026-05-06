#include <limits>
#include "icalls/system_string.h"

#include "icall_base.h"
#include "vm/rt_string.h"
#include "vm/rt_array.h"
#include "utils/string_util.h"
#include "utils/string_builder.h"

namespace leanclr
{
namespace icalls
{

RtResult<vm::RtString*> SystemString::newobj_char_array(vm::RtArray* charArray) noexcept
{
    if (charArray == nullptr)
        RET_ERR(RtErr::NullReference);

    auto eleClass = vm::Array::get_array_element_class(charArray);
    assert(eleClass->by_val->ele_type == metadata::RtElementType::Char);

    int32_t length = vm::Array::get_array_length(charArray);
    const uint16_t* chars = vm::Array::get_array_data_start_as<uint16_t>(charArray);
    vm::RtString* utf16_string = vm::String::create_string_from_utf16chars(chars, length);
    RET_OK(utf16_string);
}

/// @newobj: System.String::.ctor(System.Char[])
static RtResultVoid newobj_char_array_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                              const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto charArray = EvalStackOp::get_param<vm::RtArray*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, str, SystemString::newobj_char_array(charArray));
    EvalStackOp::set_return(ret, str);
    RET_VOID_OK();
}

RtResult<vm::RtString*> SystemString::newobj_char_array_range(vm::RtArray* charArray, int32_t startIndex, int32_t length) noexcept
{
    if (charArray == nullptr)
        RET_ERR(RtErr::NullReference);

    auto eleClass = vm::Array::get_array_element_class(charArray);
    assert(eleClass->by_val->ele_type == metadata::RtElementType::Char);

    uint32_t arr_length = static_cast<uint32_t>(vm::Array::get_array_length(charArray));
    uint32_t start_index_u32 = static_cast<uint32_t>(startIndex);
    if (start_index_u32 > arr_length || static_cast<uint32_t>(length) > arr_length - start_index_u32)
        RET_ERR(RtErr::ArgumentOutOfRange);

    const uint16_t* chars_start = vm::Array::get_array_data_start_as<uint16_t>(charArray) + startIndex;
    vm::RtString* utf16_string = vm::String::create_string_from_utf16chars(chars_start, length);
    RET_OK(utf16_string);
}

/// @newobj: System.String::.ctor(System.Char[],System.Int32,System.Int32)
static RtResultVoid newobj_char_array_range_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                    const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto charArray = EvalStackOp::get_param<vm::RtArray*>(params, 0);
    auto startIndex = EvalStackOp::get_param<int32_t>(params, 1);
    auto length = EvalStackOp::get_param<int32_t>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, str, SystemString::newobj_char_array_range(charArray, startIndex, length));
    EvalStackOp::set_return(ret, str);
    RET_VOID_OK();
}

RtResult<vm::RtString*> SystemString::newobj_utf16chars(const Utf16Char* chars) noexcept
{
    // Determine length of null-terminated UTF-16 string
    int32_t length = static_cast<int32_t>(utils::StringUtil::get_utf16chars_length(chars));
    vm::RtString* utf16_string = vm::String::create_string_from_utf16chars(reinterpret_cast<const uint16_t*>(chars), length);
    RET_OK(utf16_string);
}

/// @newobj: System.String::.ctor(System.Char*)
static RtResultVoid newobj_utf16chars_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                              const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto chars = EvalStackOp::get_param<const Utf16Char*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, str, SystemString::newobj_utf16chars(chars));
    EvalStackOp::set_return(ret, str);
    RET_VOID_OK();
}

RtResult<vm::RtString*> SystemString::newobj_utf16chars_range(const Utf16Char* chars, int32_t startIndex, int32_t length) noexcept
{
    // Compute total length of null-terminated UTF-16 buffer
    if (startIndex < 0 || length < 0 || startIndex > vm::RT_MAX_ARRAY_INDEX - length)
        RET_ERR(RtErr::ArgumentOutOfRange);
    const Utf16Char* chars_start = chars + static_cast<size_t>(startIndex);
    vm::RtString* utf16_string = vm::String::create_string_from_utf16chars(reinterpret_cast<const uint16_t*>(chars_start), length);
    RET_OK(utf16_string);
}

/// @newobj: System.String::.ctor(System.Char*,System.Int32,System.Int32)
static RtResultVoid newobj_utf16chars_range_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                    const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto chars = EvalStackOp::get_param<const Utf16Char*>(params, 0);
    auto startIndex = EvalStackOp::get_param<int32_t>(params, 1);
    auto length = EvalStackOp::get_param<int32_t>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, str, SystemString::newobj_utf16chars_range(chars, startIndex, length));
    EvalStackOp::set_return(ret, str);
    RET_VOID_OK();
}

RtResult<vm::RtString*> SystemString::newobj_utf8chars(const int8_t* chars) noexcept
{
    int32_t utf8_length = static_cast<int32_t>(std::strlen(reinterpret_cast<const char*>(chars)));
    vm::RtString* utf16_string = vm::String::create_string_from_utf8chars(reinterpret_cast<const char*>(chars), utf8_length);
    RET_OK(utf16_string);
}

/// @newobj: System.String::.ctor(System.SByte*)
static RtResultVoid newobj_utf8chars_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                             const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto chars = EvalStackOp::get_param<const int8_t*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, str, SystemString::newobj_utf8chars(chars));
    EvalStackOp::set_return(ret, str);
    RET_VOID_OK();
}

RtResult<vm::RtString*> SystemString::newobj_utf8chars_range(const int8_t* chars, int32_t start_index, int32_t length) noexcept
{
    if (start_index < 0 || length < 0 || start_index > vm::RT_MAX_ARRAY_INDEX - length)
        RET_ERR(RtErr::ArgumentOutOfRange);
    const char* chars_start = reinterpret_cast<const char*>(chars) + static_cast<size_t>(start_index);
    vm::RtString* utf16_string = vm::String::create_string_from_utf8chars(chars_start, length);
    RET_OK(utf16_string);
}

/// @newobj: System.String::.ctor(System.SByte*,System.Int32,System.Int32)
static RtResultVoid newobj_utf8chars_range_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                   const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto chars = EvalStackOp::get_param<const int8_t*>(params, 0);
    auto startIndex = EvalStackOp::get_param<int32_t>(params, 1);
    auto length = EvalStackOp::get_param<int32_t>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, str, SystemString::newobj_utf8chars_range(chars, startIndex, length));
    EvalStackOp::set_return(ret, str);
    RET_VOID_OK();
}

RtResult<vm::RtString*> SystemString::newobj_char_count(Utf16Char c, int32_t charCount) noexcept
{
    if (charCount < 0)
        RET_ERR(RtErr::ArgumentOutOfRange);
    utils::Vector<Utf16Char> buf;
    buf.resize(static_cast<size_t>(charCount), c);
    vm::RtString* string_obj = vm::String::create_string_from_utf16chars(buf.data(), charCount);
    RET_OK(string_obj);
}

/// @newobj: System.String::.ctor(System.Char,System.Int32)
static RtResultVoid newobj_char_count_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                              const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto c = EvalStackOp::get_param<Utf16Char>(params, 0);
    auto charCount = EvalStackOp::get_param<int32_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, str, SystemString::newobj_char_count(c, charCount));
    EvalStackOp::set_return(ret, str);
    RET_VOID_OK();
}

RtResult<vm::RtString*> SystemString::newobj_readonlyspan(const vm::RtReadOnlySpan<Utf16Char> span) noexcept
{
    vm::RtString* utf16_string = vm::String::create_string_from_utf16chars(reinterpret_cast<const uint16_t*>(span.pointer), span.length);
    RET_OK(utf16_string);
}

/// @newobj: System.String::.ctor(System.ReadOnlySpan`1<System.Char>)
static RtResultVoid newobj_readonlyspan_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto span = EvalStackOp::get_param<vm::RtReadOnlySpan<Utf16Char>>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, str, SystemString::newobj_readonlyspan(span));
    EvalStackOp::set_return(ret, str);
    RET_VOID_OK();
}

RtResult<vm::RtString*> SystemString::fast_allocate_string(int32_t length) noexcept
{
    // Prevent overflow: UTF-16 uses 2 bytes per char; ensure length*2 fits in u32
    if (static_cast<uint32_t>(length) > (std::numeric_limits<uint32_t>::max() / 2))
        RET_ERR(RtErr::ArgumentOutOfRange);
    vm::RtString* string_obj = vm::String::fast_allocate_string(length);
    RET_OK(string_obj);
}

/// @icall: System.String::FastAllocateString
static RtResultVoid fast_allocate_string_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                 const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto length = EvalStackOp::get_param<int32_t>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, str, SystemString::fast_allocate_string(length));
    EvalStackOp::set_return(ret, str);
    RET_VOID_OK();
}

/// @icall: System.String::InternalIntern
RtResult<vm::RtString*> SystemString::internal_intern(vm::RtString* s) noexcept
{
    if (s == nullptr)
        RET_OK(s);
    vm::RtString* interned = vm::String::intern_string(s);
    RET_OK(interned);
}

/// @icall: System.String::InternalIsInterned
RtResult<vm::RtString*> SystemString::internal_is_interned(vm::RtString* s) noexcept
{
    if (s == nullptr)
        RET_OK(static_cast<vm::RtString*>(nullptr));
    if (vm::String::is_interned_string(s))
        RET_OK(s);
    RET_OK(static_cast<vm::RtString*>(nullptr));
}

/// @icall: System.String::InternalIntern
static RtResultVoid internal_intern_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                            const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto s = EvalStackOp::get_param<vm::RtString*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, res, SystemString::internal_intern(s));
    EvalStackOp::set_return(ret, res);
    RET_VOID_OK();
}

/// @icall: System.String::InternalIsInterned
static RtResultVoid internal_is_interned_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                 const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto s = EvalStackOp::get_param<vm::RtString*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, res, SystemString::internal_is_interned(s));
    EvalStackOp::set_return(ret, res);
    RET_VOID_OK();
}

// Internal call registry
static vm::InternalCallEntry s_internal_call_entries_system_string[] = {
    {"System.String::FastAllocateString", (vm::InternalCallFunction)&SystemString::fast_allocate_string, fast_allocate_string_invoker},
    {"System.String::InternalIntern", (vm::InternalCallFunction)&SystemString::internal_intern, internal_intern_invoker},
    {"System.String::InternalIsInterned", (vm::InternalCallFunction)&SystemString::internal_is_interned, internal_is_interned_invoker},
};

// Newobj internal call registry
static vm::NewobjInternalCallEntry s_newobj_internal_call_entries[] = {
    {"System.String::.ctor(System.Char[])", newobj_char_array_invoker},
    {"System.String::.ctor(System.Char[],System.Int32,System.Int32)", newobj_char_array_range_invoker},
    {"System.String::.ctor(System.Char*)", newobj_utf16chars_invoker},
    {"System.String::.ctor(System.Char*,System.Int32,System.Int32)", newobj_utf16chars_range_invoker},
    {"System.String::.ctor(System.SByte*)", newobj_utf8chars_invoker},
    {"System.String::.ctor(System.SByte*,System.Int32,System.Int32)", newobj_utf8chars_range_invoker},
    {"System.String::.ctor(System.Char,System.Int32)", newobj_char_count_invoker},
    {"System.String::.ctor(System.ReadOnlySpan`1<System.Char>)", newobj_readonlyspan_invoker},
};

utils::Span<vm::InternalCallEntry> SystemString::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_string,
                                              sizeof(s_internal_call_entries_system_string) / sizeof(vm::InternalCallEntry));
}

utils::Span<vm::NewobjInternalCallEntry> SystemString::get_newobj_internal_call_entries()
{
    return utils::Span<vm::NewobjInternalCallEntry>(s_newobj_internal_call_entries,
                                                    sizeof(s_newobj_internal_call_entries) / sizeof(vm::NewobjInternalCallEntry));
}

} // namespace icalls
} // namespace leanclr
