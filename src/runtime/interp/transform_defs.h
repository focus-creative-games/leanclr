#pragma once

#include "interp_defs.h"

namespace leanclr
{
namespace interp
{
// Extra value union for instruction data

namespace hl
{
struct BasicBlock;
}

namespace ll
{
struct BasicBlock;
}

struct Variable;

union InstArgData
{
    const Variable* var;
    const Variable** vars;
    size_t value;
};

union IRExtraValue
{
    size_t value;
    int32_t i4;
    int64_t i8;
    size_t u;
    float r4;
    double r8;
    const void* ptr;
    const Variable* var;
    const hl::BasicBlock* hl_bb;
    const hl::BasicBlock** hl_bbs;
    const ll::BasicBlock* ll_bb;
    const ll::BasicBlock** ll_bbs;
    metadata::RtClass* klass;
    const metadata::RtMethodInfo* method;
    const metadata::RtFieldInfo* field;
    const metadata::RtMethodSig* method_sig;
    const vm::RtString* user_string;
};

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
