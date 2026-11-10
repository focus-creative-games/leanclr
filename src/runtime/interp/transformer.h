#pragma once

#include "transform_defs.h"
#include "il_opcodes.h"
#include "ll_opcodes.h"
#include "utils/not_free_list.h"
#include "utils/hashmap.h"

namespace leanclr
{
namespace interp
{
struct GeneralInst;

struct BasicBlock
{
    int32_t ir_offset = 0;
    bool visited;
    bool inited_eval_stack;
    size_t il_begin_offset;
    size_t il_end_offset;
    utils::NotFreeList<const GeneralInst*> insts;
    utils::NotFreeList<const Variable*> in_eval_stack;
    size_t in_eval_stack_top;
    utils::NotFreeList<const Variable*> eval_stack;
    size_t eval_stack_top;
    BasicBlock* next_bb;
#if LEANCLR_PGO_PROFILE
    uint32_t profile_cost;
#endif

    BasicBlock(alloc::MemPool* pool)
        : visited(false), inited_eval_stack(false), il_begin_offset(0), il_end_offset(0), insts(pool), in_eval_stack(pool), in_eval_stack_top(0),
          eval_stack(pool), eval_stack_top(0), next_bb(nullptr)
#if LEANCLR_PGO_PROFILE
          ,
          profile_cost(0)
#endif
    {
    }

    void push(const Variable* var)
    {
        assert(eval_stack_top == var->eval_stack_offset);
        eval_stack.push_back(var);
        eval_stack_top += var->stack_object_size;
    }

    void push_in_val_stack(const Variable* var)
    {
        assert(in_eval_stack_top == var->eval_stack_offset);
        in_eval_stack.push_back(var);
        in_eval_stack_top += var->stack_object_size;
    }
};

// Mutable construction form of the final instruction; only one instance is allocated.
struct GeneralInst
{
    ll::OpCodeEnum code;
    InstArgData arg1_or_src;
    InstArgData arg2;
    InstArgData arg3;
    InstArgData dst_or_ret;
    IRExtraValue extra_data;
    IRExtraValue extra_data2;
    size_t resolved_data_idx;
    int32_t il_offset;
    int32_t ir_offset;
    il::OpCodePrefix prefix;

    ll::OpCodeEnum get_opcode() const
    {
        return code;
    }

    void set_opcode(ll::OpCodeEnum opcode)
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

    void set_size(size_t size)
    {
        extra_data.u = size;
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

    BasicBlock* get_branch_target() const
    {
        return (BasicBlock*)(extra_data.ptr);
    }

    void update_branch_target(const BasicBlock* target)
    {
        extra_data.bb = target;
    }

    intptr_t get_branch_target_offset() const
    {
        auto target = extra_data.bb;
        return static_cast<intptr_t>(target->ir_offset) - static_cast<intptr_t>(ir_offset);
    }

    struct SwitchTargets
    {
        const BasicBlock** targets;
        size_t count;
    };

    SwitchTargets get_switch_targets() const
    {
        return {extra_data.bbs, arg2.value};
    }

    void update_switch_targets(const BasicBlock** targets, size_t count)
    {
        extra_data.bbs = targets;
        arg2.value = count;
    }

    size_t get_num_targets() const
    {
        return arg2.value;
    }

    const BasicBlock* get_leave_target() const
    {
        return extra_data.bb;
    }

    void update_leave_target(const BasicBlock* target)
    {
        extra_data.bb = target;
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

    void set_total_params_stack_object_size(size_t size)
    {
        arg3.value = size;
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

    void set_var_arg1(const Variable* arg)
    {
        arg1_or_src.var = arg;
    }

    void set_var_arg2(const Variable* arg)
    {
        arg2.var = arg;
    }

    void set_var_arg3(const Variable* arg)
    {
        arg3.var = arg;
    }

    void set_var_src(const Variable* src)
    {
        arg1_or_src.var = src;
    }

    void set_var_size(const Variable* size)
    {
        extra_data.var = size;
    }

    const Variable* get_var_size() const
    {
        return extra_data.var;
    }

    size_t get_var_arg_eval_stack_idx1() const
    {
        return get_var_arg1()->eval_stack_offset;
    }

    size_t get_var_arg_eval_stack_idx2() const
    {
        return get_var_arg2()->eval_stack_offset;
    }

    size_t get_var_arg_eval_stack_idx3() const
    {
        return get_var_arg3()->eval_stack_offset;
    }

    size_t get_var_size_eval_stack_idx() const
    {
        return get_var_size()->eval_stack_offset;
    }

    size_t unsafe_get_arg1_value() const
    {
        return arg1_or_src.value;
    }

    size_t unsafe_get_arg2_value() const
    {
        return arg2.value;
    }

    size_t unsafe_get_arg3_value() const
    {
        return arg3.value;
    }

    size_t unsafe_get_dst_value() const
    {
        return dst_or_ret.value;
    }

    void set_prefix(il::OpCodePrefix pfx)
    {
        prefix = pfx;
    }

    il::OpCodePrefix get_prefix() const
    {
        return prefix;
    }

    bool contains_prefix_readonly() const
    {
        return (static_cast<int32_t>(prefix) & static_cast<int32_t>(il::OpCodePrefix::ReadOnly)) != 0;
    }

    bool contains_prefix_unaligned() const
    {
        return (static_cast<int32_t>(prefix) & static_cast<int32_t>(il::OpCodePrefix::Unaligned)) != 0;
    }

    IRExtraValue get_extra_data() const
    {
        return extra_data;
    }

    void set_r4(float val)
    {
        extra_data.r4 = val;
    }

    float get_r4() const
    {
        return extra_data.r4;
    }

    void set_r8(double val)
    {
        extra_data.r8 = val;
    }

    double get_r8() const
    {
        return extra_data.r8;
    }

    void set_branch_target(BasicBlock* target)
    {
        extra_data.bb = target;
    }

    void set_switch_targets(const BasicBlock** targets, size_t count)
    {
        extra_data.bbs = targets;
        arg2.value = count;
    }

    void set_leave_target(BasicBlock* target)
    {
        extra_data.bb = target;
    }

    metadata::RtClass* get_class() const
    {
        return extra_data.klass;
    }

    void set_class(metadata::RtClass* klass)
    {
        extra_data.klass = klass;
    }

    const metadata::RtFieldInfo* get_field() const
    {
        return extra_data.field;
    }

    void set_method(const metadata::RtMethodInfo* method)
    {
        extra_data.method = method;
    }

    const metadata::RtMethodInfo* get_method() const
    {
        return extra_data.method;
    }

    void set_invoker_idx(size_t idx)
    {
        arg3.value = idx;
    }

    void set_method_and_params(const metadata::RtMethodInfo* method, size_t frame_base_idx, const Variable** params)
    {
        arg1_or_src.vars = params;
        arg2.value = frame_base_idx;
        extra_data.method = method;
    }

    std::pair<const metadata::RtMethodInfo*, size_t> get_method_and_frame_base() const
    {
        return {extra_data.method, arg2.value};
    }

    void set_newobj_method_and_params(const metadata::RtMethodInfo* method, size_t frame_base_idx, size_t total_param_stack_object_size,
                                      const Variable** params)
    {
        arg1_or_src.vars = params;
        arg2.value = frame_base_idx;
        arg3.value = total_param_stack_object_size;
        extra_data.method = method;
    }

    void set_method_sig_and_params(const metadata::RtMethodSig* method, size_t frame_base_idx, const Variable* method_idx, const Variable** params)
    {
        arg1_or_src.vars = params;
        arg2.value = frame_base_idx;
        arg3.var = method_idx;
        extra_data.method_sig = method;
    }

    std::pair<const metadata::RtMethodSig*, size_t> get_methodsig_and_frame_base() const
    {
        auto method = extra_data.method_sig;
        return {method, arg2.value};
    }

    void set_method_sig(const metadata::RtMethodSig* method_sig)
    {
        extra_data.method_sig = method_sig;
    }

    const metadata::RtMethodSig* get_method_sig() const
    {
        return extra_data.method_sig;
    }

    void set_user_string(const vm::RtString* user_string)
    {
        extra_data.user_string = user_string;
    }

    const vm::RtString* get_user_string() const
    {
        return extra_data.user_string;
    }

    void set_runtime_handle(metadata::RtEncodedRuntimeHandle handle)
    {
        extra_data.u = handle.get_encoded_value();
    }

    metadata::RtEncodedRuntimeHandle get_runtime_handle() const
    {
        return metadata::RtEncodedRuntimeHandle{extra_data.u};
    }
};

class Transformer
{
  public:
    Transformer(metadata::RtModuleDef* mod, const metadata::RtMethodInfo* method_info, const metadata::RtMethodBody& method_body, alloc::MemPool& mem_pool);

    RtResultVoid transform();
    RtResult<const RtInterpMethodInfo*> build_interp_method_info();

    const metadata::RtMethodInfo* get_method_info() const;
    const metadata::RtMethodBody* get_method_body() const;
    metadata::RtModuleDef* get_module() const;
    const metadata::RtGenericContainerContext& get_generic_container_context() const;
    const metadata::RtGenericContext* get_generic_context() const;
    size_t get_total_arg_and_local_stack_object_size() const;
    size_t get_max_stack_size() const;
    bool need_init_locals() const;
    const Variable* get_ret_var() const;
    const BasicBlock* get_basic_blocks() const;

    size_t get_basic_block_count() const
    {
        return _basic_block_count;
    }

    RtResult<BasicBlock*> get_branch_target_bb(size_t global_target_offset);

  private:
    using Emitter = RtResultVoid (Transformer::*)(GeneralInst*, const GeneralInst*);

    // Scratch construction state, drained after each IL instruction. Basic blocks
    // contain only final typed instructions, with no intermediate opcode stream.
    struct PendingInst
    {
        GeneralInst* inst;
        Emitter emit;
    };

    utils::NotFreeList<PendingInst> _pending_insts;
    utils::NotFreeList<const void*> _resolved_datas;
    utils::HashMap<const void*, size_t> _resolved_data_2_index_map;
    RtResultVoid flush_pending_instructions();

    size_t get_cur_eval_stack_top() const;
    size_t alloc_var_id();

    RtResult<const Variable*> alloc_variable_by_typesig(const metadata::RtTypeSig* type_sig, size_t eval_stack_offset);
    Variable* create_eval_stack_variable(RtEvalStackDataType data_type, metadata::RtArgOrLocOrFieldReduceType reduce_type, size_t data_size,
                                         size_t eval_stack_offset);
    RtResult<Variable*> create_eval_stack_variable_from_type_sig(const metadata::RtTypeSig* type_sig, size_t eval_stack_offset);
    Variable* create_eval_stack_variable_from_var(const Variable* var, size_t eval_stack_offset);

    RtResultVoid setup_args();
    RtResultVoid setup_locals();
    const Variable* alloc_exception_variable(size_t eval_stack_offset);
    RtResultVoid setup_exception_clauses();
    RtResultVoid setup_basic_blocks();

    RtResult<metadata::RtRuntimeHandle> get_cached_runtime_handle(uint32_t token);
    RtResult<const metadata::RtMethodInfo*> get_method_from_token(uint32_t raw_token);
    RtResult<metadata::RtMethodSig> get_standalone_method_sig_from_token(uint32_t token);
    RtResult<const metadata::RtTypeSig*> get_type_from_token(uint32_t raw_token);
    RtResult<metadata::RtClass*> get_class_from_token(uint32_t raw_token);
    RtResult<const metadata::RtFieldInfo*> get_field_from_token(uint32_t raw_token);
    RtResult<metadata::RtRuntimeHandle> get_raw_runtime_handle_from_token(uint32_t raw_token);

    // Each IL operation can insert conversions before its main instruction.
    // Finish these constructions before decoding the next IL instruction.
    GeneralInst* create_inst(Emitter emitter);
    void internal_push_eval_stack_var_and_update_max(const Variable* var);
    void update_max_eval_stack_size(size_t new_size);
    const Variable* push_var_to_eval_stack(const Variable* var);
    void clear_eval_stack();
    Variable* push_primitive_to_eval_stack(RtEvalStackDataType data_type);
    Variable* push_i4_to_eval_stack();
    Variable* push_ref_or_ptr_to_eval_stack();
    RtResult<Variable*> push_typesig_to_eval_stack(const metadata::RtTypeSig* type_sig);
    RtResult<Variable*> push_class_to_eval_stack(const metadata::RtClass* klass);
    RtResult<const Variable*> get_top_var() const;
    RtResult<const Variable*> pop_eval_stack();

    void add_prefix(il::OpCodePrefix prefix);
    void set_prefix(il::OpCodePrefix prefix);
    void set_constrainted_prefix(metadata::RtClass* klass);
    void clear_prefix();
    RtResultVoid add_init_locals();
    RtResultVoid add_ldarg(size_t arg_idx);
    RtResultVoid add_starg(size_t arg_idx);
    RtResultVoid add_ldarga(size_t arg_idx);
    RtResultVoid add_ldloc(size_t local_idx);
    RtResultVoid add_stloc(size_t local_idx);
    RtResultVoid add_ldloca(size_t local_idx);
    RtResultVoid add_ldnull();
    RtResultVoid add_ldci4(int32_t value);
    RtResultVoid add_ldci8(int64_t value);
    RtResultVoid add_ldcr4(float value);
    RtResultVoid add_ldcr8(double value);
    RtResultVoid add_dup();
    RtResultVoid add_ret();

    RtResultVoid setup_target_branch_eval_stack(BasicBlock* target);
    RtResultVoid setup_next_and_target_branch_eval_stack(BasicBlock* target);
    RtResultVoid add_br(uint32_t next_offset, int32_t target_offset);
    RtResultVoid add_brtrue_or_false(uint32_t next_offset, int32_t target_offset, bool is_true);
    RtResultVoid add_condition_branch(uint32_t next_offset, int32_t target_offset, Emitter opcode);
    RtResultVoid add_switch(uint32_t next_offset, const void* case_targets, size_t count);

    RtResultVoid add_ldind(Emitter opcode, metadata::RtArgOrLocOrFieldReduceType reduce_type);
    RtResultVoid add_stind(Emitter opcode);
    RtResult<RtEvalStackDataType> calc_bin_arith_result_type(RtEvalStackDataType left, RtEvalStackDataType right);
    RtResult<size_t> calc_arith_op_eval_data_size_by_data_type(RtEvalStackDataType data_type);
    RtResultVoid insert_conv_from_to(const Variable* from_var, const Variable* to_var);
    RtResultVoid insert_conv_for_bin_arith_op(const Variable* left, const Variable* right);
    RtResultVoid add_bin_arith_op(Emitter opcode);
    RtResult<RtEvalStackDataType> calc_bitop_result_type(RtEvalStackDataType left, RtEvalStackDataType right);
    RtResultVoid add_bin_bit_op(Emitter opcode);
    RtResultVoid add_bit_shift_op(Emitter opcode);
    RtResultVoid add_neg();
    RtResultVoid add_not();
    RtResultVoid add_bin_compare_op(Emitter opcode);
    RtResultVoid add_conv(Emitter opcode, RtEvalStackDataType data_type);
    RtResultVoid add_ckfinite();

    RtResult<const metadata::RtMethodInfo*> try_redirect_newobj_method(const metadata::RtMethodInfo* method);
    RtResultVoid add_call_common(const metadata::RtMethodInfo* method, metadata::RtInvokerType invoker_type, metadata::RtInvokeMethodPointer invoker,
                                 bool is_new_obj, bool is_call_vir);
    RtResultVoid add_call(const metadata::RtMethodInfo* method);
    RtResult<bool> try_handle_newobj_intrinsic(const metadata::RtMethodInfo* method);
    RtResultVoid add_newobj(const metadata::RtMethodInfo* method);
    RtResultVoid add_enum_hash_code_call(metadata::RtClass* enum_klass);
    RtResultVoid add_box_ref_inplace(metadata::RtClass* klass, size_t eval_stack_idx, const Variable* obj);
    RtResultVoid add_ldind_ref_inplace(size_t eval_stack_idx, const Variable* obj);

    RtResultVoid add_callvirt(const metadata::RtMethodInfo* method);
    RtResultVoid add_calli(const metadata::RtMethodSig& method_sig);
    RtResultVoid add_ldftn(const metadata::RtMethodInfo* method);
    RtResultVoid add_ldvirtftn(const metadata::RtMethodInfo* method);
    RtResultVoid add_initobj(metadata::RtClass* klass);
    RtResultVoid add_cpobj(metadata::RtClass* klass);
    RtResultVoid add_ldobj(metadata::RtClass* klass);
    RtResultVoid add_stobj(metadata::RtClass* klass);
    RtResultVoid add_ldstr(uint32_t user_string_index);
    RtResultVoid add_castclass(metadata::RtClass* klass);
    RtResultVoid add_isinst(metadata::RtClass* klass);
    RtResultVoid add_box(metadata::RtClass* klass);
    RtResultVoid add_unbox(metadata::RtClass* klass);
    RtResultVoid add_unbox_any(metadata::RtClass* klass);
    RtResultVoid add_ldfld(const metadata::RtFieldInfo* field);
    RtResultVoid add_ldflda(const metadata::RtFieldInfo* field);
    RtResultVoid add_stfld(const metadata::RtFieldInfo* field);
    RtResultVoid add_ldsfld(const metadata::RtFieldInfo* field);
    RtResultVoid add_ldsflda(const metadata::RtFieldInfo* field);
    RtResultVoid add_stsfld(const metadata::RtFieldInfo* field);
    RtResultVoid add_newarr(metadata::RtClass* ele_klass);
    RtResultVoid add_ldlen();
    RtResultVoid add_ldelema(metadata::RtClass* ele_klass);
    RtResultVoid add_ldelem(Emitter opcode, RtEvalStackDataType data_type);
    RtResultVoid add_ldelem_any(metadata::RtClass* ele_klass);
    RtResultVoid add_stelem(Emitter opcode, RtEvalStackDataType data_type);
    RtResultVoid add_stelem_any(metadata::RtClass* ele_klass);
    RtResultVoid add_mkrefany(metadata::RtClass* klass);
    RtResultVoid add_refanytype();
    RtResultVoid add_refanyval(metadata::RtClass* klass);
    RtResultVoid add_ldtoken(uint32_t token);
    RtResultVoid add_throw();
    RtResultVoid add_rethrow();
    RtResultVoid add_endfinally();
    RtResultVoid add_endfilter();
    RtResultVoid add_leave(size_t next_offset, ptrdiff_t target_offset);
    RtResultVoid add_arglist();
    RtResultVoid add_localloc();
    RtResultVoid add_cpblk();
    RtResultVoid add_initblk();
    RtResultVoid add_sizeof(const metadata::RtTypeSig* type_sig);
    RtResultVoid transform_body();

    metadata::RtModuleDef* _mod;
    const metadata::RtMethodInfo* _method;
    const metadata::RtMethodBody* _method_body;
    alloc::MemPool* _pool;

    metadata::RtGenericContainerContext _generic_container_context{};
    const metadata::RtGenericContext* _generic_context{nullptr};

    const Variable** _arg_vars{nullptr};
    size_t _arg_vars_count{0};
    size_t _total_arg_stack_object_size{0};

    const Variable** _local_vars{nullptr};
    size_t _local_vars_count{0};
    size_t _total_arg_and_local_stack_object_size{0};

    BasicBlock* _basic_blocks{nullptr};
    size_t _basic_block_count{0};
    utils::HashMap<uint32_t, BasicBlock*> _il_offset_to_basic_block;
    BasicBlock* _cur_bb{nullptr};

    size_t _eval_stack_base_offset{0};
    size_t _max_eval_stack_size{0};
    size_t _next_var_id{0};
    const Variable* _ret_var{nullptr};
    utils::NotFreeList<const Variable*> _vars;
    il::OpCodePrefix _prefix{il::OpCodePrefix::None};
    metadata::RtClass* _constrained_class{nullptr};
    bool _not_retset_prefix_after_cur_il{false};
    utils::HashMap<uint32_t, metadata::RtRuntimeHandle> _runtime_handle_cache;

    int32_t _cur_il_offset{-1};
    enum class LeaveSurroundingBlockType
    {
        None,
        Try,
        CatchOrFilter,
    };

    size_t get_resolved_data_index(const void* data);
    void setup_inst_resolved_data(GeneralInst* output, const void* data);
    void setup_inst_klass(GeneralInst* output, const GeneralInst* input);
    void setup_inst_method(GeneralInst* output, const GeneralInst* input);
    utils::NotFreeList<size_t> find_finally_clause_idx_of_leave_target(const BasicBlock* leave_src, const BasicBlock* leave_target);
    LeaveSurroundingBlockType get_leave_surrounding_block_type(const BasicBlock* leave_src, const BasicBlock* leave_target);
    metadata::RtILExceptionClauseType get_cur_endfinally_or_fault_clause_type(const BasicBlock* cur_bb);

    RtResultVoid transform_condition_branch(GeneralInst* output, const GeneralInst* input, ll::OpCodeEnum opcode_i4, ll::OpCodeEnum opcode_i8,
                                            ll::OpCodeEnum opcode_r4, ll::OpCodeEnum opcode_r8);

    RtResultVoid transform_brtruefalse(GeneralInst* output, const GeneralInst* input, ll::OpCodeEnum opcode_i4, ll::OpCodeEnum opcode_i8);

    RtResultVoid transform_bin_arith_op(GeneralInst* output, const GeneralInst* input, ll::OpCodeEnum opcode_i4, ll::OpCodeEnum opcode_i8,
                                        ll::OpCodeEnum opcode_r4, ll::OpCodeEnum opcode_r8);

    RtResultVoid transform_bin_arith_ovf_op(GeneralInst* output, const GeneralInst* input, ll::OpCodeEnum opcode_i4, ll::OpCodeEnum opcode_i8);

    RtResultVoid transform_bin_bit_op(GeneralInst* output, const GeneralInst* input, ll::OpCodeEnum opcode_i4, ll::OpCodeEnum opcode_i8);

    RtResultVoid transform_bin_bit_shift_op(GeneralInst* output, const GeneralInst* input, ll::OpCodeEnum opcode_i4, ll::OpCodeEnum opcode_i8);

    RtResultVoid transform_conv(GeneralInst* output, const GeneralInst* input, ll::OpCodeEnum opcode_i4, ll::OpCodeEnum opcode_i8, ll::OpCodeEnum opcode_r4,
                                ll::OpCodeEnum opcode_r8);

    RtResultVoid transform_cmp_op(GeneralInst* output, const GeneralInst* input, ll::OpCodeEnum opcode_i4, ll::OpCodeEnum opcode_i8, ll::OpCodeEnum opcode_r4,
                                  ll::OpCodeEnum opcode_r8);

    RtResult<bool> transform_special_call_methods(GeneralInst* output, const GeneralInst* input);
    RtResult<bool> transform_special_newobj_methods(GeneralInst* output, const GeneralInst* input);
    RtResultVoid build_exception_clauses(RtInterpMethodInfo* interp_method);
    RtResult<uint32_t> translate_il_offset_to_ir_offset(uint32_t il_offset);
    RtResultVoid build_codes(RtInterpMethodInfo* interp_method);
    RtResultVoid emit_Nop(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ProfileAddCost(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Arglist(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_InitLocals(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdArg(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdLoc(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdArga(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdLoca(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_StArg(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_StLoc(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdNull(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdcI4(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdcI8(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdcR4(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdcR8(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdStr(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Dup(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Ret(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Br(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_BrTrue(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_BrFalse(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Beq(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Bge(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Bgt(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Ble(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Blt(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_BneUn(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_BgeUn(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_BgtUn(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_BleUn(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_BltUn(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Switch(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdIndI1(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdIndU1(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdIndI2(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdIndU2(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdIndI4(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdIndR4(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdIndI8(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdIndR8(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdIndRef(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_StIndI1(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_StIndI2(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_StIndI4(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_StIndR4(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_StIndI8(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_StIndR8(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_StIndRef(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Add(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Sub(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Mul(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Div(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Rem(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_DivUn(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_RemUn(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_And(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Or(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Xor(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Shl(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Shr(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ShrUn(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Neg(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Not(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_AddOvf(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_AddOvfUn(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_SubOvf(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_SubOvfUn(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_MulOvf(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_MulOvfUn(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ConvI1(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ConvU1(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ConvI2(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ConvU2(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ConvI4(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ConvU4(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ConvI8(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ConvU8(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ConvI(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ConvU(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ConvR4(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ConvR8(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ConvOvfI1(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ConvOvfU1(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ConvOvfI2(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ConvOvfU2(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ConvOvfI4(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ConvOvfU4(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ConvOvfI8(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ConvOvfU8(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ConvOvfI(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ConvOvfU(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ConvOvfI1Un(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ConvOvfU1Un(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ConvOvfI2Un(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ConvOvfU2Un(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ConvOvfI4Un(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ConvOvfU4Un(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ConvOvfI8Un(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ConvOvfU8Un(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ConvOvfIUn(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_ConvOvfUUn(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Ceq(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Cgt(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Clt(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_CgtUn(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_CltUn(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_InitObjI1(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_InitObjI2(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_InitObjI4(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_InitObjI8(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_InitObjRef(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_InitObjAny(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_CpObjI1(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_CpObjI2(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_CpObjI4(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_CpObjI8(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_CpObjRef(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_CpObjAny(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdObjAny(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_StObjAny(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_CastClass(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_IsInst(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Box(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_BoxRefInplace(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Unbox(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_UnboxAny(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_NewArr(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdLen(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Ldelema(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdelemI1(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdelemU1(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdelemI2(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdelemU2(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdelemI4(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdelemI8(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdelemI(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdelemR4(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdelemR8(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdelemRef(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdelemAny(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_StelemI1(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_StelemI2(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_StelemI4(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_StelemI8(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_StelemI(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_StelemR4(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_StelemR8(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_StelemRef(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_StelemAny(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_MkRefAny(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_RefAnyVal(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_RefAnyType(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LdToken(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Ckfinite(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_LocAlloc(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_InitBlk(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_CpBlk(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Ldfld(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Ldvfld(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Ldflda(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Stfld(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Ldsfld(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Ldsflda(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Stsfld(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Call(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_CallVirt(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_CallInternalCall(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_CallIntrinsic(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_CallPInvoke(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_CallAot(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_CallRuntimeImplemented(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Calli(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_NewObj(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_NewObjInternalCall(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_NewObjIntrinsic(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_NewObjAot(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Ldftn(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Ldvirtftn(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Throw(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Rethrow(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_Leave(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_EndFilter(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_EndFinallyOrFault(GeneralInst* output, const GeneralInst* input);
    RtResultVoid emit_GetEnumLongHashCode(GeneralInst* output, const GeneralInst* input);
};
} // namespace interp
} // namespace leanclr
