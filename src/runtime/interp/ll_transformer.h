#pragma once

#include "transform_defs.h"
#include "ll_opcodes.h"
#include "utils/not_free_list.h"
#include "utils/hashmap.h"

namespace leanclr
{
namespace interp
{
namespace hl
{
struct GeneralInst;
struct BasicBlock;
class Transformer;
} // namespace hl

namespace ll
{

struct GeneralInst;

struct BasicBlock
{
    const hl::BasicBlock* hl_bb;
    int32_t ir_offset;
    utils::NotFreeList<const GeneralInst*> insts;
    BasicBlock* next_bb;

    BasicBlock(alloc::MemPool* pool, const hl::BasicBlock* hl_bb) : hl_bb(hl_bb), ir_offset(0), insts(pool), next_bb(nullptr)
    {
    }
};

// Low-level general instruction container
struct GeneralInst
{
    OpCodeEnum code;
    InstArgData arg1_or_src;
    InstArgData arg2;
    InstArgData arg3;
    InstArgData dst_or_ret;
    interp::IRExtraValue extra_data;
    interp::IRExtraValue extra_data2;
    size_t resolved_data_idx;
    int32_t il_offset;
    int32_t ir_offset;

    GeneralInst(const hl::GeneralInst& hl_inst);

    OpCodeEnum get_opcode() const
    {
        return code;
    }

    void set_opcode(OpCodeEnum opcode)
    {
        code = opcode;
    }

    int32_t get_il_offset() const
    {
        return il_offset;
    }

    void set_il_offset(int32_t offset)
    {
        il_offset = offset;
    }

    int32_t get_ir_offset() const
    {
        return ir_offset;
    }

    void set_ir_offset(int32_t offset)
    {
        ir_offset = offset;
    }

    const interp::Variable* get_var_arg1() const
    {
        return arg1_or_src.var;
    }

    const interp::Variable* get_var_arg2() const
    {
        return arg2.var;
    }

    const interp::Variable* get_var_arg3() const
    {
        return arg3.var;
    }

    const interp::Variable* get_var_src() const
    {
        return arg1_or_src.var;
    }

    void update_var_src(const interp::Variable* src)
    {
        arg1_or_src.var = src;
    }

    void update_var_arg1(const interp::Variable* arg)
    {
        arg1_or_src.var = arg;
    }

    void update_var_arg2(const interp::Variable* arg)
    {
        arg2.var = arg;
    }

    void set_var_src_invalid()
    {
        arg1_or_src.var = nullptr;
    }

    void update_var_dst(const interp::Variable* dst)
    {
        dst_or_ret.var = dst;
    }

    void update_var_ret(const interp::Variable* ret)
    {
        dst_or_ret.var = ret;
    }

    const interp::Variable* get_var_dst() const
    {
        return dst_or_ret.var;
    }

    void set_var_dst(const interp::Variable* dst)
    {
        dst_or_ret.var = dst;
    }

    const interp::Variable* get_var_ret() const
    {
        return dst_or_ret.var;
    }

    void set_var_ret(const interp::Variable* arg)
    {
        dst_or_ret.var = arg;
    }

    size_t get_var_arg1_eval_stack_idx() const
    {
        return get_var_arg1()->eval_stack_offset;
    }

    size_t get_var_arg2_eval_stack_idx() const
    {
        return get_var_arg2()->eval_stack_offset;
    }

    size_t get_var_arg3_eval_stack_idx() const
    {
        return get_var_arg3()->eval_stack_offset;
    }

    size_t get_var_src_eval_stack_idx() const
    {
        return get_var_src()->eval_stack_offset;
    }

    size_t get_var_dst_eval_stack_idx() const
    {
        return get_var_dst()->eval_stack_offset;
    }

    size_t get_var_ret_eval_stack_idx() const
    {
        return get_var_ret()->eval_stack_offset;
    }

    size_t get_locals_offset() const
    {
        return arg1_or_src.value;
    }

    void set_locals_offset(size_t offset)
    {
        arg1_or_src.value = offset;
    }

    size_t get_size() const
    {
        return extra_data.u;
    }

    void set_size(size_t new_size)
    {
        assert(extra_data.u == 0);
        extra_data.u = new_size;
    }

    size_t get_stack_object_size() const
    {
        return get_size();
    }

    void set_stack_object_size(size_t new_size)
    {
        set_size(new_size);
    }

    void set_element_size(size_t new_size)
    {
        assert(extra_data2.u == 0);
        extra_data2.u = new_size;
    }

    size_t get_element_size() const
    {
        return extra_data2.u;
    }

    int8_t get_i1() const
    {
        return static_cast<int8_t>(extra_data.i4);
    }

    int16_t get_i2() const
    {
        return static_cast<int16_t>(extra_data.i4);
    }

    int32_t get_i4() const
    {
        return extra_data.i4;
    }

    void set_i4(int32_t val)
    {
        extra_data.i4 = val;
    }

    int64_t get_i8() const
    {
        return extra_data.i8;
    }

    void set_i8(int64_t val)
    {
        extra_data.i8 = val;
    }

    uint32_t get_i8_low() const
    {
        return static_cast<uint32_t>(extra_data.i8 & 0xFFFFFFFF);
    }

    uint32_t get_i8_high() const
    {
        return static_cast<uint32_t>((extra_data.i8 >> 32) & 0xFFFFFFFF);
    }

    void set_resolved_data_index(size_t index)
    {
        assert(resolved_data_idx == 0);
        resolved_data_idx = index;
    }

    size_t get_resolved_data_index() const
    {
        return resolved_data_idx;
    }

    void set_extra_r4(float val)
    {
        extra_data.r4 = val;
    }

    float get_extra_r4() const
    {
        return extra_data.r4;
    }

    void set_extra_r8(double val)
    {
        assert(extra_data.r8 == 0.0);
        extra_data.r8 = val;
    }

    double get_extra_r8() const
    {
        return extra_data.r8;
    }

    void set_extra_ptr(const void* val)
    {
        assert(extra_data.ptr == nullptr);
        extra_data.ptr = val;
    }

    const void* get_extra_ptr() const
    {
        return extra_data.ptr;
    }

    const BasicBlock* get_branch_target() const
    {
        return extra_data.ll_bb;
    }

    void update_branch_target(const BasicBlock* target)
    {
        extra_data.ll_bb = target;
    }

    intptr_t get_branch_target_offset() const
    {
        auto target = extra_data.ll_bb;
        return static_cast<intptr_t>(target->ir_offset) - static_cast<intptr_t>(ir_offset);
    }

    struct SwitchTargets
    {
        const BasicBlock** targets;
        size_t count;
    };

    SwitchTargets get_switch_targets() const
    {
        return {extra_data.ll_bbs, arg2.value};
    }

    void update_switch_targets(const BasicBlock** targets, size_t count)
    {
        extra_data.ll_bbs = targets;
        arg2.value = count;
    }

    size_t get_num_targets() const
    {
        return arg2.value;
    }

    const BasicBlock* get_leave_target() const
    {
        return extra_data.ll_bb;
    }

    void update_leave_target(const BasicBlock* target)
    {
        extra_data.ll_bb = target;
    }

    size_t get_exception_clause_index() const
    {
        return extra_data.u;
    }

    void set_exception_clause_index(size_t index)
    {
        assert(extra_data.u == 0);
        extra_data.u = index;
    }

    void set_field(const metadata::RtFieldInfo* field)
    {
        assert(extra_data.field == nullptr);
        extra_data.field = field;
    }

    size_t get_field_offset() const
    {
        return vm::Field::get_field_offset_includes_object_header_for_reference_type(extra_data.field);
    }

    size_t get_field_size() const
    {
        auto field = extra_data.field;
        auto res = vm::Field::get_field_size(extra_data.field);
        assert(res.is_ok());
        return res.unwrap();
    }

    struct MethodAndParams
    {
        const metadata::RtMethodInfo* method;
        size_t frame_base_idx;
        const interp::Variable** params;
    };

    MethodAndParams get_method_and_params() const
    {
        auto method = extra_data.method;
        auto frame_base_idx = arg2.value;
        auto params = arg1_or_src.vars;
        return {method, frame_base_idx, params};
    }

    const interp::Variable* const* get_params() const
    {
        return arg1_or_src.vars;
    }

    size_t get_frame_base() const
    {
        return arg2.value;
    }

    void set_frame_base(size_t frame_base_idx)
    {
        arg2.value = frame_base_idx;
    }

    size_t get_invoker_idx() const
    {
        return arg3.value;
    }

    size_t get_aot_invoker_idx() const
    {
        return arg3.value;
    }

    void update_invoker_idx(size_t idx)
    {
        arg3.value = idx;
    }

    size_t get_total_params_stack_object_size() const
    {
        return arg3.value;
    }

    void set_total_params_stack_object_size(size_t new_size)
    {
        assert(arg3.value == 0);
        arg3.value = new_size;
    }

    size_t get_first_finally_clause_index() const
    {
        return arg1_or_src.value;
    }

    void set_first_finally_clause_index(size_t index)
    {
        assert(arg1_or_src.value == 0);
        arg1_or_src.value = index;
    }

    size_t get_finally_clauses_count() const
    {
        return arg2.value;
    }

    void set_finally_clauses_count(size_t count)
    {
        assert(arg2.value == 0);
        arg2.value = count;
    }
};

class Transformer
{
  public:
    Transformer(hl::Transformer& hl_trans, alloc::MemPool& mem_pool) : _hl_transformer(hl_trans), _mem_pool(mem_pool), _resolved_datas(&mem_pool)
    {
    }

    RtResultVoid transform();

    RtResult<const RtInterpMethodInfo*> build_interp_method_info();

  private:
    enum class LeaveSurroundingBlockType
    {
        None,
        Try,
        CatchOrFilter,
    };

    hl::Transformer& _hl_transformer;
    alloc::MemPool& _mem_pool;
    utils::HashMap<const hl::BasicBlock*, BasicBlock*> _hl_2_ll_bb_map;
    BasicBlock* _bb_head = nullptr;
    utils::NotFreeList<const void*> _resolved_datas;
    utils::HashMap<const void*, size_t> _resolved_data_2_index_map;

    // Helper functions
    RtResult<BasicBlock*> translate_hl_basic_to_ll_basic(const hl::BasicBlock* hl_bb);
    RtResultVoid transform_basic_blocks();
    size_t get_resolved_data_index(const void* data);
    void setup_inst_resolved_data(GeneralInst* ll_inst, const void* data);
    void setup_inst_klass(GeneralInst* ll_inst, const hl::GeneralInst* hl_inst);
    void setup_inst_method(GeneralInst* ll_inst, const hl::GeneralInst* hl_inst);
    utils::NotFreeList<size_t> find_finally_clause_idx_of_leave_target(const BasicBlock* leave_src, const BasicBlock* leave_target);
    LeaveSurroundingBlockType get_leave_surrounding_block_type(const BasicBlock* leave_src, const BasicBlock* leave_target);
    metadata::RtILExceptionClauseType get_cur_endfinally_or_fault_clause_type(const BasicBlock* cur_bb);

    RtResultVoid transform_condition_branch(GeneralInst* ll_inst, const hl::GeneralInst* hl_inst, OpCodeEnum opcode_i4, OpCodeEnum opcode_i8,
                                            OpCodeEnum opcode_r4, OpCodeEnum opcode_r8);

    RtResultVoid transform_brtruefalse(GeneralInst* ll_inst, const hl::GeneralInst* hl_inst, OpCodeEnum opcode_i4, OpCodeEnum opcode_i8);

    RtResultVoid transform_bin_arith_op(GeneralInst* ll_inst, const hl::GeneralInst* hl_inst, OpCodeEnum opcode_i4, OpCodeEnum opcode_i8, OpCodeEnum opcode_r4,
                                        OpCodeEnum opcode_r8);

    RtResultVoid transform_bin_arith_ovf_op(GeneralInst* ll_inst, const hl::GeneralInst* hl_inst, OpCodeEnum opcode_i4, OpCodeEnum opcode_i8);

    RtResultVoid transform_bin_bit_op(GeneralInst* ll_inst, const hl::GeneralInst* hl_inst, OpCodeEnum opcode_i4, OpCodeEnum opcode_i8);

    RtResultVoid transform_bin_bit_shift_op(GeneralInst* ll_inst, const hl::GeneralInst* hl_inst, OpCodeEnum opcode_i4, OpCodeEnum opcode_i8);

    RtResultVoid transform_conv(GeneralInst* ll_inst, const hl::GeneralInst* hl_inst, OpCodeEnum opcode_i4, OpCodeEnum opcode_i8, OpCodeEnum opcode_r4,
                                OpCodeEnum opcode_r8);

    RtResultVoid transform_cmp_op(GeneralInst* ll_inst, const hl::GeneralInst* hl_inst, OpCodeEnum opcode_i4, OpCodeEnum opcode_i8, OpCodeEnum opcode_r4,
                                  OpCodeEnum opcode_r8);

    RtResult<bool> transform_special_call_methods(GeneralInst* ll_inst, const hl::GeneralInst* hl_inst);
    RtResult<bool> transform_special_newobj_methods(GeneralInst* ll_inst, const hl::GeneralInst* hl_inst);
    RtResultVoid transform_instructions();
    RtResultVoid optimize_short_instructions();
    RtResultVoid build_exception_clauses(RtInterpMethodInfo* interp_method);
    RtResult<uint32_t> translate_il_offset_to_ir_offset(uint32_t il_offset);
    RtResultVoid build_codes(RtInterpMethodInfo* interp_method);
};
} // namespace ll
} // namespace interp
} // namespace leanclr
