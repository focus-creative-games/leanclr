#include "intrinsics/system_string.h"

#include "interp/eval_stack_op.h"
#include "vm/rt_string.h"
#include <cstring>

namespace leanclr
{
namespace intrinsics
{

RtResult<uint16_t> SystemString::get_chars(vm::RtString* s, int32_t index) noexcept
{
    return vm::String::get_chars(s, index);
}

RtResult<int32_t> SystemString::get_length(vm::RtString* s) noexcept
{
    if (s == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }
    RET_OK(vm::String::get_length(s));
}

RtResult<int32_t> SystemString::get_hash_code(vm::RtString* str) noexcept
{
    int32_t hash = str ? vm::String::get_hash_code(str) : 0;
    RET_OK(hash);
}

RtResult<bool> SystemString::equals(vm::RtString* left, vm::RtString* right) noexcept
{
    if (left == right)
    {
        RET_OK(true);
    }
    if (left == nullptr || right == nullptr)
    {
        RET_OK(false);
    }
    if (left->length != right->length)
    {
        RET_OK(false);
    }

    RET_OK(std::memcmp(vm::String::get_chars_ptr(left), vm::String::get_chars_ptr(right), static_cast<size_t>(left->length) * sizeof(Utf16Char)) == 0);
}

static Utf16Char to_ascii_lower(Utf16Char ch) noexcept
{
    if (ch >= 'A' && ch <= 'Z')
    {
        return static_cast<Utf16Char>(ch + ('a' - 'A'));
    }
    return ch;
}

static bool chars_equal_for_comparison(Utf16Char left, Utf16Char right, int32_t comparison_type) noexcept
{
    constexpr int32_t current_culture_ignore_case = 1;
    constexpr int32_t invariant_culture_ignore_case = 3;
    constexpr int32_t ordinal_ignore_case = 5;
    if (comparison_type == current_culture_ignore_case || comparison_type == invariant_culture_ignore_case || comparison_type == ordinal_ignore_case)
    {
        left = to_ascii_lower(left);
        right = to_ascii_lower(right);
    }
    return left == right;
}

RtResult<bool> SystemString::contains(vm::RtString* str, vm::RtString* value, int32_t comparison_type) noexcept
{
    if (str == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }
    if (value == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }
    if (value->length == 0)
    {
        RET_OK(true);
    }
    if (value->length > str->length)
    {
        RET_OK(false);
    }

    const Utf16Char* str_chars = vm::String::get_chars_ptr(str);
    const Utf16Char* value_chars = vm::String::get_chars_ptr(value);
    const int32_t last_start = str->length - value->length;
    for (int32_t i = 0; i <= last_start; ++i)
    {
        bool matched = true;
        for (int32_t j = 0; j < value->length; ++j)
        {
            if (!chars_equal_for_comparison(str_chars[i + j], value_chars[j], comparison_type))
            {
                matched = false;
                break;
            }
        }
        if (matched)
        {
            RET_OK(true);
        }
    }
    RET_OK(false);
}

/// @intrinsic: System.String::get_Chars
RtResultVoid get_chars_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                               interp::RtStackObject* ret) noexcept
{
    vm::RtString* s = interp::EvalStackOp::get_param<vm::RtString*>(params, 0);
    int32_t index = interp::EvalStackOp::get_param<int32_t>(params, 1);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint16_t, ch, SystemString::get_chars(s, index));
    interp::EvalStackOp::set_return(ret, static_cast<int32_t>(ch));
    RET_VOID_OK();
}

/// @intrinsic: System.String::get_Length
RtResultVoid get_length_invoker_intrinsics_system_string(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                         const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    vm::RtString* s = interp::EvalStackOp::get_param<vm::RtString*>(params, 0);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, length, SystemString::get_length(s));
    interp::EvalStackOp::set_return(ret, length);
    RET_VOID_OK();
}

/// @intrinsic: System.String::GetHashCode()
static RtResultVoid get_hash_code_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                          interp::RtStackObject* ret) noexcept
{
    auto str = interp::EvalStackOp::get_param<vm::RtString*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, hash, SystemString::get_hash_code(str));
    interp::EvalStackOp::set_return(ret, hash);
    RET_VOID_OK();
}

/// @intrinsic: System.String::Equals(System.String,System.String)
static RtResultVoid equals_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                   interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto left = interp::EvalStackOp::get_param<vm::RtString*>(params, 0);
    auto right = interp::EvalStackOp::get_param<vm::RtString*>(params, 1);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemString::equals(left, right));
    interp::EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

/// @intrinsic: System.String::op_Inequality(System.String,System.String)
static RtResultVoid not_equals_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                       const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto left = interp::EvalStackOp::get_param<vm::RtString*>(params, 0);
    auto right = interp::EvalStackOp::get_param<vm::RtString*>(params, 1);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemString::equals(left, right));
    interp::EvalStackOp::set_return(ret, static_cast<int32_t>(!result));
    RET_VOID_OK();
}

/// @intrinsic: System.String::Contains(System.String)
static RtResultVoid contains_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                     interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto str = interp::EvalStackOp::get_param<vm::RtString*>(params, 0);
    auto value = interp::EvalStackOp::get_param<vm::RtString*>(params, 1);

    constexpr int32_t ordinal = 4;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemString::contains(str, value, ordinal));
    interp::EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

/// @intrinsic: System.String::Contains(System.String,System.StringComparison)
static RtResultVoid contains_comparison_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto str = interp::EvalStackOp::get_param<vm::RtString*>(params, 0);
    auto value = interp::EvalStackOp::get_param<vm::RtString*>(params, 1);
    int32_t comparison_type = interp::EvalStackOp::get_param<int32_t>(params, 2);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemString::contains(str, value, comparison_type));
    interp::EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

// Intrinsic registry
static vm::IntrinsicEntry s_intrinsic_entries_system_string[] = {
    {"System.String::get_Chars(System.Int32)", (vm::IntrinsicFunction)&SystemString::get_chars, get_chars_invoker},
    {"System.String::get_Chars", (vm::IntrinsicFunction)&SystemString::get_chars, get_chars_invoker},
    {"System.String::get_Length()", (vm::IntrinsicFunction)&SystemString::get_length, get_length_invoker_intrinsics_system_string},
    {"System.String::get_Length", (vm::IntrinsicFunction)&SystemString::get_length, get_length_invoker_intrinsics_system_string},
    {"System.String::GetHashCode()", (vm::IntrinsicFunction)&SystemString::get_hash_code, get_hash_code_invoker},
    {"System.String::GetHashCode", (vm::IntrinsicFunction)&SystemString::get_hash_code, get_hash_code_invoker},
    {"System.String::Equals(System.String,System.String)", (vm::IntrinsicFunction)&SystemString::equals, equals_invoker},
    {"System.String::op_Equality(System.String,System.String)", (vm::IntrinsicFunction)&SystemString::equals, equals_invoker},
    {"System.String::op_Inequality(System.String,System.String)", (vm::IntrinsicFunction)&SystemString::equals, not_equals_invoker},
    {"System.String::Contains(System.String)", (vm::IntrinsicFunction)&SystemString::contains, contains_invoker},
    {"System.String::Contains(System.String,System.StringComparison)", (vm::IntrinsicFunction)&SystemString::contains, contains_comparison_invoker},
    // redirected to intrinsic
    {"System.String::GetLegacyNonRandomizedHashCode()", (vm::IntrinsicFunction)&SystemString::get_hash_code, get_hash_code_invoker},
    {"System.String::GetLegacyNonRandomizedHashCode", (vm::IntrinsicFunction)&SystemString::get_hash_code, get_hash_code_invoker},
};

utils::Span<vm::IntrinsicEntry> SystemString::get_intrinsic_entries() noexcept
{
    return utils::Span<vm::IntrinsicEntry>(s_intrinsic_entries_system_string, sizeof(s_intrinsic_entries_system_string) / sizeof(vm::IntrinsicEntry));
}

} // namespace intrinsics
} // namespace leanclr
