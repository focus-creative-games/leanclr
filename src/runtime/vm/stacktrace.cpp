#include "stacktrace.h"
#include "rt_string.h"
#include "rt_array.h"
#include "class.h"
#include "reflection.h"
#include "object.h"
#include "method.h"
#include "interp/interp_defs.h"
#include "interp/machine_state.h"
#include "metadata/module_def.h"

namespace leanclr
{
namespace vm
{

static bool is_frame_should_be_counted_to_stacktrace(const interp::InterpFrame* frame)
{
    const metadata::RtMethodInfo* method = frame->method;
    if (!method->parent->image->is_corlib())
    {
        return true;
    }
    const char* klass_name = method->parent->name;
    return !(strcmp(klass_name, "StackFrame") == 0 || strcmp(klass_name, "StackTrace") == 0);
}

RtResultVoid StackTrace::setup_trace_ips(RtException* ex)
{
    if (ex->trace_ips)
    {
        RET_VOID_OK();
    }
    auto& ms = interp::MachineState::get_global_machine_state();
    auto frames = ms.get_active_frames();

    utils::Vector<const interp::InterpFrame*> trace_frames;
    for (size_t i = 0; i < frames.size(); ++i)
    {
        const interp::InterpFrame* frame = &frames[i];
        if (is_frame_should_be_counted_to_stacktrace(frame))
        {
            trace_frames.push_back(frame);
        }
    }
    metadata::RtClass* cls_stackframe = Class::get_corlib_types().cls_stackframe;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, trace_ips, Array::new_szarray_from_ele_klass(cls_stackframe, static_cast<int32_t>(trace_frames.size())));
    for (size_t i = 0, frame_count = trace_frames.size(); i < frame_count; ++i)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, stackframe_obj, Object::new_object(cls_stackframe));
        RtStackFrame* stackframe = static_cast<RtStackFrame*>(stackframe_obj);

        const interp::InterpFrame* frame = trace_frames[i];
        UNWRAP_OR_RET_ERR_ON_FAIL(stackframe->method, Reflection::get_method_reflection_object(frame->method, frame->method->parent));
        stackframe->method_index = static_cast<uint32_t>(Method::get_method_index_in_class(frame->method));
        metadata::PdbImage* pdb_image = frame->method->parent->image->get_pdb_image();
        if (pdb_image)
        {
            int32_t ir_offset = static_cast<int32_t>(frame->ip - frame->method->interp_data->codes);
            const char* pdb_file_name = nullptr;
            pdb_image->get_debug_info_for_method(frame->method, ir_offset, &stackframe->il_offset, &pdb_file_name, &stackframe->line, &stackframe->column);
            stackframe->filename = pdb_file_name ? String::create_string_from_utf8cstr(pdb_file_name) : nullptr;
        }
        else
        {
            stackframe->il_offset = -1;
            stackframe->filename = nullptr;
            stackframe->line = 0;
            stackframe->column = 0;
        }
        stackframe->native_offset = -1;
        Array::set_array_data_at<RtObject*>(trace_ips, static_cast<int32_t>(frame_count - 1 - i), stackframe_obj);
    }
    ex->trace_ips = trace_ips;

    RET_VOID_OK();
}

RtResult<bool> StackTrace::get_frame_info(int32_t skip, bool need_file_info, RtReflectionMethod** method, int32_t* il_offset, int32_t* native_offset,
                                          RtString** file_name, int32_t* line_number, int32_t* column_number)
{
    auto& ms = interp::MachineState::get_global_machine_state();
    auto frames = ms.get_active_frames();
    size_t frame_count = frames.size();
    skip -= 1; // Skip method from StackFrame
    if (skip < 0 || skip >= static_cast<int32_t>(frame_count))
    {
        RET_OK(false);
    }

    const size_t frame_index = frame_count - 1U - static_cast<size_t>(skip);
    const interp::InterpFrame* frame = &frames[frame_index];
    UNWRAP_OR_RET_ERR_ON_FAIL(*method, Reflection::get_method_reflection_object(frame->method, frame->method->parent));

    int32_t ir_offset = static_cast<int32_t>(frame->ip - frame->method->interp_data->codes);

    metadata::PdbImage* pdb_image = frame->method->parent->image->get_pdb_image();
    if (pdb_image)
    {
        const char* pdb_file_name = nullptr;
        int32_t pdb_line_number = 0;
        int32_t pdb_column_number = 0;
        pdb_image->get_debug_info_for_method(frame->method, ir_offset, il_offset, &pdb_file_name, &pdb_line_number, &pdb_column_number);
        if (need_file_info)
        {
            *file_name = pdb_file_name ? String::create_string_from_utf8cstr(pdb_file_name) : nullptr;
            *line_number = pdb_line_number;
            *column_number = pdb_column_number;
        }
    }
    else
    {
        *il_offset = -1;
        if (need_file_info)
        {
            *file_name = nullptr;
            *line_number = 0;
            *column_number = 0;
        }
    }
    *native_offset = -1;
    RET_OK(true);
}

RtResult<RtArray*> StackTrace::get_stack_trace(RtException* ex, int32_t skip_frames, bool need_file_info)
{
    metadata::RtClass* cls_stackframe = Class::get_corlib_types().cls_stackframe;
    if (ex->trace_ips == nullptr)
    {
        return Array::new_empty_szarray_by_ele_klass(cls_stackframe);
    }

    int32_t stack_count = Array::get_array_length(ex->trace_ips);
    assert(skip_frames >= 0);
    if (skip_frames >= stack_count)
    {
        return Array::new_empty_szarray_by_ele_klass(cls_stackframe);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, result_array, Array::new_szarray_from_ele_klass(cls_stackframe, stack_count - skip_frames));
    for (int32_t i = skip_frames; i < stack_count; ++i)
    {
        RtObject* frame_obj = Array::get_array_data_at<RtObject*>(ex->trace_ips, i);
        Array::set_array_data_at<RtObject*>(result_array, i - skip_frames, frame_obj);
    }

    RET_OK(result_array);
}
} // namespace vm
} // namespace leanclr
