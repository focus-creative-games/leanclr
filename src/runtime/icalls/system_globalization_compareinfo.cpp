#include "system_globalization_compareinfo.h"
#include "icall_base.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <wctype.h>

namespace leanclr
{
namespace icalls
{

// CompareOptions enum values
enum class CompareOptions : int32_t
{
    None = 0,
    IgnoreCase = 1,
    IgnoreNonSpace = 2,
    IgnoreSymbols = 4,
    IgnoreKanaType = 8,
    IgnoreWidth = 0x10,
    OrdinalIgnoreCase = 0x10000000,
    StringSort = 0x20000000,
    Ordinal = 0x40000000,
};

static int32_t compare_char(char16_t c1, char16_t c2, int32_t options)
{
    int32_t result;
    if ((options & (static_cast<int32_t>(CompareOptions::IgnoreCase) | static_cast<int32_t>(CompareOptions::OrdinalIgnoreCase))) != 0)
    {
        result = static_cast<int32_t>(towlower(c1)) - static_cast<int32_t>(towlower(c2));
    }
    else
    {
        result = static_cast<int32_t>(c1) - static_cast<int32_t>(c2);
    }

    if (result < 0)
    {
        return -1;
    }
    else if (result > 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

RtResult<int32_t> SystemGlobalizationCompareInfo::internal_compare_icall(const char16_t* str1, int32_t length1, const char16_t* str2, int32_t length2,
                                                                         int32_t options) noexcept
{
    assert(length1 >= 0 && length2 >= 0);
    int32_t min_len = std::min(length1, length2);
    for (int32_t i = 0; i < min_len; i++)
    {
        char16_t c1 = str1[i];
        char16_t c2 = str2[i];
        int32_t ord = compare_char(c1, c2, options);
        if (ord != 0)
        {
            RET_OK(ord);
        }
    }

    if (length1 < length2)
    {
        RET_OK(-1);
    }
    else if (length1 > length2)
    {
        RET_OK(1);
    }
    else
    {
        RET_OK(0);
    }
}

/// @icall: System.Globalization.CompareInfo::internal_compare_icall
static RtResultVoid internal_compare_icall_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                   interp::RtStackObject* ret) noexcept
{
    auto str1 = EvalStackOp::get_param<const char16_t*>(params, 0);
    auto length1 = EvalStackOp::get_param<int32_t>(params, 1);
    auto str2 = EvalStackOp::get_param<const char16_t*>(params, 2);
    auto length2 = EvalStackOp::get_param<int32_t>(params, 3);
    auto options = EvalStackOp::get_param<int32_t>(params, 4);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, SystemGlobalizationCompareInfo::internal_compare_icall(str1, length1, str2, length2, options));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<int32_t> SystemGlobalizationCompareInfo::internal_index_icall(const char16_t* source, int32_t source_start_index, int32_t source_count,
                                                                       const char16_t* value, int32_t value_length, bool first) noexcept
{
    if (source_count < value_length)
    {
        RET_OK(-1);
    }

    if (first)
    {
        for (int32_t i = 0; i <= source_count - value_length; i++)
        {
            if (std::memcmp(source + source_start_index + i, value, static_cast<size_t>(value_length) * sizeof(char16_t)) == 0)
            {
                RET_OK(source_start_index + i);
            }
        }
    }
    else
    {
        for (int32_t i = source_count - value_length; i >= 0; i--)
        {
            if (std::memcmp(source + source_start_index + i, value, static_cast<size_t>(value_length) * sizeof(char16_t)) == 0)
            {
                RET_OK(source_start_index + i);
            }
        }
    }
    RET_OK(-1);
}

/// @icall: System.Globalization.CompareInfo::internal_index_icall(System.Char*,System.Int32,System.Int32,System.Char*,System.Int32,System.Boolean)
static RtResultVoid internal_index_icall_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                 interp::RtStackObject* ret) noexcept
{
    auto source = EvalStackOp::get_param<const char16_t*>(params, 0);
    auto source_start_index = EvalStackOp::get_param<int32_t>(params, 1);
    auto source_count = EvalStackOp::get_param<int32_t>(params, 2);
    auto value = EvalStackOp::get_param<const char16_t*>(params, 3);
    auto value_length = EvalStackOp::get_param<int32_t>(params, 4);
    auto first = EvalStackOp::get_param<bool>(params, 5);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(
        int32_t, result, SystemGlobalizationCompareInfo::internal_index_icall(source, source_start_index, source_count, value, value_length, first));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

static vm::InternalCallEntry s_internal_call_entries_system_globalization_compareinfo[] = {
    {"System.Globalization.CompareInfo::internal_compare_icall", (vm::InternalCallFunction)&SystemGlobalizationCompareInfo::internal_compare_icall,
     internal_compare_icall_invoker},
    {"System.Globalization.CompareInfo::internal_index_icall(System.Char*,System.Int32,System.Int32,System.Char*,System.Int32,System.Boolean)",
     (vm::InternalCallFunction)&SystemGlobalizationCompareInfo::internal_index_icall, internal_index_icall_invoker},
};

utils::Span<vm::InternalCallEntry> SystemGlobalizationCompareInfo::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_globalization_compareinfo,
                                              sizeof(s_internal_call_entries_system_globalization_compareinfo) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr
