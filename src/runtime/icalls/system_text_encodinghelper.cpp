#include "system_text_encodinghelper.h"

#include "vm/rt_string.h"

namespace leanclr
{
namespace icalls
{

using namespace metadata;

// @icall: System.Text.EncodingHelper::InternalCodePage(System.Int32&)
RtResult<vm::RtString*> SystemTextEncodingHelper::internal_code_page(int32_t* code_page) noexcept
{
    // Match platform default: UTF-8 code page id 3 and name "utf_8"
    *code_page = 3;
    RET_OK(vm::String::create_string_from_utf8cstr("utf_8"));
}

static RtResultVoid internal_code_page_invoker(RtManagedMethodPointer /*method_pointer*/, const RtMethodInfo* /*method*/, const interp::RtStackObject* params,
                                               interp::RtStackObject* ret) noexcept
{
    int32_t* code_page_ptr = EvalStackOp::get_param<int32_t*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, name, SystemTextEncodingHelper::internal_code_page(code_page_ptr));
    EvalStackOp::set_return(ret, name);
    RET_VOID_OK();
}

static vm::InternalCallEntry s_internal_call_entries_system_text_encodinghelper[] = {
    {"System.Text.EncodingHelper::InternalCodePage(System.Int32&)", (vm::InternalCallFunction)&SystemTextEncodingHelper::internal_code_page,
     internal_code_page_invoker},
};

utils::Span<vm::InternalCallEntry> SystemTextEncodingHelper::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_text_encodinghelper,
                                              sizeof(s_internal_call_entries_system_text_encodinghelper) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr
