#include "stacktrace.h"
#include "rt_string.h"
#include "rt_array.h"
#include "class.h"
#include "field.h"
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
namespace
{
struct StackFrameFields
{
    metadata::RtClass* klass;
    const metadata::RtFieldInfo* method;
    const metadata::RtFieldInfo* native_offset;
    const metadata::RtFieldInfo* il_offset;
    const metadata::RtFieldInfo* file_name;
    const metadata::RtFieldInfo* line_number;
    const metadata::RtFieldInfo* column_number;
    const metadata::RtFieldInfo* is_last_frame_from_foreign_exception_stack_trace;
};

RtResult<const metadata::RtFieldInfo*> get_required_stack_frame_field(metadata::RtClass* cls_stackframe, const char* name)
{
    const metadata::RtFieldInfo* field = Class::get_field_for_name(cls_stackframe, name, false);
    if (field == nullptr)
    {
        RET_ERR(RtErr::MissingField);
    }
    RET_OK(field);
}

RtResult<StackFrameFields*> get_stack_frame_fields()
{
    metadata::RtClass* cls_stackframe = Class::get_corlib_types().cls_stackframe;
    RET_ERR_ON_FAIL(Class::initialize_fields(cls_stackframe));

    static StackFrameFields fields{};
    if (fields.klass != cls_stackframe)
    {
        StackFrameFields refreshed{};
        refreshed.klass = cls_stackframe;
        UNWRAP_OR_RET_ERR_ON_FAIL(refreshed.method, get_required_stack_frame_field(cls_stackframe, "_method"));
        UNWRAP_OR_RET_ERR_ON_FAIL(refreshed.native_offset, get_required_stack_frame_field(cls_stackframe, "_nativeOffset"));
        UNWRAP_OR_RET_ERR_ON_FAIL(refreshed.il_offset, get_required_stack_frame_field(cls_stackframe, "_ilOffset"));
        UNWRAP_OR_RET_ERR_ON_FAIL(refreshed.file_name, get_required_stack_frame_field(cls_stackframe, "_fileName"));
        UNWRAP_OR_RET_ERR_ON_FAIL(refreshed.line_number, get_required_stack_frame_field(cls_stackframe, "_lineNumber"));
        UNWRAP_OR_RET_ERR_ON_FAIL(refreshed.column_number, get_required_stack_frame_field(cls_stackframe, "_columnNumber"));
        UNWRAP_OR_RET_ERR_ON_FAIL(refreshed.is_last_frame_from_foreign_exception_stack_trace,
                                  get_required_stack_frame_field(cls_stackframe, "_isLastFrameFromForeignExceptionStackTrace"));
        fields = refreshed;
    }

    RET_OK(&fields);
}

RtResultVoid get_stack_frame_source_info(const interp::InterpFrame* frame, int32_t* il_offset, RtString** file_name, int32_t* line_number,
                                         int32_t* column_number)
{
    *il_offset = -1;
    *file_name = nullptr;
    *line_number = 0;
    *column_number = 0;

    if (frame->method == nullptr || frame->method->interp_data == nullptr || frame->ip == nullptr)
    {
        RET_VOID_OK();
    }

    *il_offset = static_cast<int32_t>(frame->ip - frame->method->interp_data->codes);
    metadata::PdbImage* pdb_image = frame->method->parent->image->get_pdb_image();
    if (pdb_image != nullptr)
    {
        const char* pdb_file_name = nullptr;
        pdb_image->get_debug_info_for_method(frame->method, *il_offset, il_offset, &pdb_file_name, line_number, column_number);
        *file_name = pdb_file_name != nullptr ? String::create_string_from_utf8cstr(pdb_file_name) : nullptr;
    }

    RET_VOID_OK();
}
} // namespace

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

RtResultVoid StackTrace::set_stack_frame_data(RtObject* stack_frame, const metadata::RtMethodInfo* method, int32_t native_offset,
                                              int32_t il_offset, RtString* file_name, int32_t line_number, int32_t column_number,
                                              bool is_last_frame_from_foreign_exception_stack_trace)
{
    if (stack_frame == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(StackFrameFields*, fields, get_stack_frame_fields());
    RtReflectionMethod* reflection_method = nullptr;
    if (method != nullptr)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(reflection_method, Reflection::get_method_reflection_object(method, method->parent));
    }

    RET_ERR_ON_FAIL(Field::set_instance_value(fields->method, stack_frame, &reflection_method));
    RET_ERR_ON_FAIL(Field::set_instance_value(fields->native_offset, stack_frame, &native_offset));
    RET_ERR_ON_FAIL(Field::set_instance_value(fields->il_offset, stack_frame, &il_offset));
    RET_ERR_ON_FAIL(Field::set_instance_value(fields->file_name, stack_frame, &file_name));
    RET_ERR_ON_FAIL(Field::set_instance_value(fields->line_number, stack_frame, &line_number));
    RET_ERR_ON_FAIL(Field::set_instance_value(fields->column_number, stack_frame, &column_number));
    RET_ERR_ON_FAIL(Field::set_instance_value(fields->is_last_frame_from_foreign_exception_stack_trace, stack_frame,
                                              &is_last_frame_from_foreign_exception_stack_trace));

    RET_VOID_OK();
}

RtResultVoid StackTrace::get_stack_frame_data(RtObject* stack_frame, RtReflectionMethod** method, int32_t* native_offset, int32_t* il_offset,
                                              RtString** file_name, int32_t* line_number, int32_t* column_number,
                                              bool* is_last_frame_from_foreign_exception_stack_trace)
{
    if (stack_frame == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(StackFrameFields*, fields, get_stack_frame_fields());
    RET_ERR_ON_FAIL(Field::get_instance_value(fields->method, stack_frame, method));
    RET_ERR_ON_FAIL(Field::get_instance_value(fields->native_offset, stack_frame, native_offset));
    RET_ERR_ON_FAIL(Field::get_instance_value(fields->il_offset, stack_frame, il_offset));
    RET_ERR_ON_FAIL(Field::get_instance_value(fields->file_name, stack_frame, file_name));
    RET_ERR_ON_FAIL(Field::get_instance_value(fields->line_number, stack_frame, line_number));
    RET_ERR_ON_FAIL(Field::get_instance_value(fields->column_number, stack_frame, column_number));
    RET_ERR_ON_FAIL(Field::get_instance_value(fields->is_last_frame_from_foreign_exception_stack_trace, stack_frame,
                                              is_last_frame_from_foreign_exception_stack_trace));

    RET_VOID_OK();
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
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, trace_ips, LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERNAL(cls_stackframe, static_cast<int32_t>(trace_frames.size()), "StackTrace::setup_trace_ips"));
    for (size_t i = 0, frame_count = trace_frames.size(); i < frame_count; ++i)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, stackframe_obj, LEANCLR_NEWOBJ_INTERNAL(cls_stackframe, "StackTrace::setup_trace_ips"));
        const interp::InterpFrame* frame = trace_frames[i];
        int32_t il_offset = -1;
        RtString* file_name = nullptr;
        int32_t line_number = 0;
        int32_t column_number = 0;
        RET_ERR_ON_FAIL(get_stack_frame_source_info(frame, &il_offset, &file_name, &line_number, &column_number));
        RET_ERR_ON_FAIL(set_stack_frame_data(stackframe_obj, frame->method, -1, il_offset, file_name, line_number, column_number, false));
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
        return LEANCLR_NEW_EMPTY_SZARRAY_BY_ELE_KLASS_INTERNAL(cls_stackframe, "StackTrace::get_stack_trace");
    }

    int32_t stack_count = Array::get_array_length(ex->trace_ips);
    assert(skip_frames >= 0);
    if (skip_frames >= stack_count)
    {
        return LEANCLR_NEW_EMPTY_SZARRAY_BY_ELE_KLASS_INTERNAL(cls_stackframe, "StackTrace::get_stack_trace");
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, result_array, LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERNAL(cls_stackframe, stack_count - skip_frames, "StackTrace::get_stack_trace"));
    for (int32_t i = skip_frames; i < stack_count; ++i)
    {
        RtObject* frame_obj = Array::get_array_data_at<RtObject*>(ex->trace_ips, i);
        Array::set_array_data_at<RtObject*>(result_array, i - skip_frames, frame_obj);
    }

    RET_OK(result_array);
}
} // namespace vm
} // namespace leanclr
