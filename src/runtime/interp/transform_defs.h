#pragma once

#include "interp_defs.h"

namespace leanclr
{
namespace interp
{
// Variable information structure
struct Variable
{
    size_t id;
    const metadata::RtTypeSig* type_;
    metadata::RtArgOrLocOrFieldReduceType reduce_type;
    RtEvalStackDataType data_type;
    size_t byte_size;
    size_t stack_object_size;
    size_t eval_stack_offset;

    bool is_i32_or_native_int() const
    {
        return data_type == RtEvalStackDataType::I4 || data_type == RtEvalStackDataType::RefOrPtr;
    }

    bool is_i32_or_i64_or_native_int() const
    {
        return data_type == RtEvalStackDataType::I4 || data_type == RtEvalStackDataType::I8 || data_type == RtEvalStackDataType::RefOrPtr;
    }

    bool is_not_i32_and_native_int() const
    {
        return data_type != RtEvalStackDataType::I4 && data_type != RtEvalStackDataType::RefOrPtr;
    }

    bool is_not_i32_and_i64_and_native_int() const
    {
        return data_type != RtEvalStackDataType::I4 && data_type != RtEvalStackDataType::I8 && data_type != RtEvalStackDataType::RefOrPtr;
    }

    bool is_f32_or_f64() const
    {
        return data_type == RtEvalStackDataType::R4 || data_type == RtEvalStackDataType::R8;
    }

    bool is_primitive() const
    {
        return data_type == RtEvalStackDataType::I4 || data_type == RtEvalStackDataType::I8 || data_type == RtEvalStackDataType::R4 ||
               data_type == RtEvalStackDataType::R8 || data_type == RtEvalStackDataType::RefOrPtr;
    }

    bool is_reference() const
    {
        return data_type == RtEvalStackDataType::RefOrPtr;
    }

    bool is_not_reference() const
    {
        return data_type != RtEvalStackDataType::RefOrPtr;
    }

    bool is_typedbyref() const
    {
        return data_type == RtEvalStackDataType::Other && type_ && type_->ele_type == metadata::RtElementType::TypedByRef;
    }
};

} // namespace interp
} // namespace leanclr
