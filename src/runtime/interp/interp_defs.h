#pragma once

#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace interp
{
// Stack object union for interpreter execution
union RtStackObject
{
    uint64_t value;
    void* ptr;
    const void* cptr;
    bool b;
    int8_t i8;
    uint8_t u8;
    int16_t i16;
    uint16_t u16;
    int32_t i32;
    uint32_t u32;
    int64_t i64;
    uint64_t u64;
    float f32;
    double f64;
    size_t size;
    vm::RtObject* obj;
    vm::RtString* str;
};

static_assert(sizeof(RtStackObject) == 8, "RtStackObject must be 8 bytes");

// Evaluation stack data type
enum class RtEvalStackDataType : uint8_t
{
    I4,
    I8,
    R4,
    R8,
    RefOrPtr,
    Other,
};

// Interpreter exception clause
struct RtInterpExceptionClause
{
    metadata::RtILExceptionClauseType flags;
    uint32_t try_begin_offset;
    uint32_t try_end_offset;
    uint32_t handler_begin_offset;
    uint32_t handler_end_offset;
    uint32_t filter_begin_offset;
    metadata::RtClass* ex_klass;

    bool is_in_try_block(uint32_t il_offset) const
    {
        return try_begin_offset <= il_offset && il_offset < try_end_offset;
    }

    bool is_in_handler_block(uint32_t il_offset) const
    {
        return handler_begin_offset <= il_offset && il_offset < handler_end_offset;
    }

    bool is_in_filter_block(uint32_t il_offset) const
    {
        return flags == metadata::RtILExceptionClauseType::Filter && filter_begin_offset <= il_offset && il_offset < handler_end_offset;
    }
};

// Interpreter method info
struct RtInterpMethodInfo
{
    uint8_t* codes;
    const RtInterpExceptionClause* exception_clauses;
    const void** resolved_datas;
    uint16_t total_arg_and_local_stack_object_size;
    uint16_t max_stack_object_size;
    uint8_t exception_clause_count;
    bool init_locals;
    uint32_t code_size;
};

// Constants
const size_t INVALID_EVAL_STACK_OFFSET = static_cast<size_t>(UINT16_MAX);

struct ReduceTypeAndSize
{
    metadata::RtArgOrLocOrFieldReduceType reduce_type;
    size_t byte_size;
};

class InterpDefs
{
  public:
    // Helper function: Get evaluation stack data type from reduce type
    static RtEvalStackDataType get_eval_stack_data_type_by_reduce_type(metadata::RtArgOrLocOrFieldReduceType reduce_type);
    static RtResult<ReduceTypeAndSize> get_reduce_type_and_size_by_typesig(const metadata::RtTypeSig* typeSig);
    static size_t get_stack_object_size_by_byte_size(size_t byte_size);
};

} // namespace interp
} // namespace leanclr
