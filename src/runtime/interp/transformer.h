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
struct EncodedInstruction;

struct BasicBlock
{
    int32_t ir_offset = 0;
    bool visited;
    bool inited_eval_stack;
    size_t il_begin_offset;
    size_t il_end_offset;
    utils::NotFreeList<EncodedInstruction*> insts;
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

// Instructions are encoded during IL translation; layout only patches/copies bytes.
struct alignas(8) EncodedInstruction
{
    size_t byte_size;
    int32_t il_offset;
    int32_t ir_offset;

    uint8_t* data()
    {
        return reinterpret_cast<uint8_t*>(this + 1);
    }

    const uint8_t* data() const
    {
        return reinterpret_cast<const uint8_t*>(this + 1);
    }
};

struct InstructionOperands
{
    // Stack-local emission state. Named fields keep registers, metadata and
    // immediates distinct; none of these construction pointers enter bytecode.
    ll::OpCodeEnum opcode = ll::OpCodeEnum::Illegal;
    const Variable* first = nullptr;
    const Variable* second = nullptr;
    const Variable* third = nullptr;
    const Variable* destination = nullptr;
    const Variable** parameters = nullptr;
    metadata::RtClass* klass = nullptr;
    const metadata::RtFieldInfo* field = nullptr;
    const metadata::RtMethodInfo* method = nullptr;
    const metadata::RtMethodSig* signature = nullptr;
    const vm::RtString* string_value = nullptr;
    const BasicBlock* target = nullptr;

    struct SwitchTargets
    {
        const BasicBlock** targets;
        size_t count;

        SwitchTargets(const BasicBlock** targets = nullptr, size_t count = 0) : targets(targets), count(count)
        {
        }
    } switch_targets;

    il::OpCodePrefix prefix = il::OpCodePrefix::None;
    uint64_t immediate = 0;
    size_t runtime_handle = 0;
    size_t size = 0;
    size_t element_size = 0;
    size_t resolved_index = 0;
    size_t frame_base = 0;
    size_t parameter_size = 0;
    size_t invoker_index = 0;
    size_t locals_offset = 0;
    size_t first_finally = 0;
    size_t finally_count = 0;
    size_t field_offset = 0;
    size_t field_size = 0;
};

struct NumericOpcodes
{
    ll::OpCodeEnum i4, i8, r4, r8;
};

class Transformer
{
  public:
    Transformer(metadata::RtModuleDef* mod, const metadata::RtMethodInfo* method_info, const metadata::RtMethodBody& method_body, alloc::MemPool& mem_pool);

    RtResultVoid transform();
    RtResult<const RtInterpMethodInfo*> build_interp_method_info();

  private:
    bool need_init_locals() const;
    RtResult<BasicBlock*> get_branch_target_bb(size_t global_target_offset);

    struct BranchPatch
    {
        EncodedInstruction* source;
        const BasicBlock* target;
        size_t byte_offset;
    };

    utils::NotFreeList<BranchPatch> _branch_patches;
    RtResultVoid append_instruction(InstructionOperands& operands);
    RtResultVoid emit_numeric(InstructionOperands& operands, const NumericOpcodes& opcodes, RtEvalStackDataType type);

    static bool has_prefix(il::OpCodePrefix value, il::OpCodePrefix flag)
    {
        return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flag)) != 0;
    }

    utils::NotFreeList<const void*> _resolved_datas;
    utils::HashMap<const void*, size_t> _resolved_data_2_index_map;

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
    RtResultVoid add_condition_branch(uint32_t next_offset, int32_t target_offset, const NumericOpcodes& opcode);
    RtResultVoid add_switch(uint32_t next_offset, const void* case_targets, size_t count);

    RtResultVoid add_ldind(ll::OpCodeEnum opcode, metadata::RtArgOrLocOrFieldReduceType reduce_type);
    RtResultVoid add_stind(ll::OpCodeEnum opcode);
    RtResult<RtEvalStackDataType> calc_bin_arith_result_type(RtEvalStackDataType left, RtEvalStackDataType right);
    RtResult<size_t> calc_arith_op_eval_data_size_by_data_type(RtEvalStackDataType data_type);
    RtResultVoid insert_conv_from_to(const Variable* from_var, const Variable* to_var);
    RtResultVoid insert_conv_for_bin_arith_op(const Variable* left, const Variable* right);
    RtResultVoid add_bin_arith_op(const NumericOpcodes& opcode);
    RtResult<RtEvalStackDataType> calc_bitop_result_type(RtEvalStackDataType left, RtEvalStackDataType right);
    RtResultVoid add_bin_bit_op(const NumericOpcodes& opcode);
    RtResultVoid add_bit_shift_op(const NumericOpcodes& opcode);
    RtResultVoid add_neg();
    RtResultVoid add_not();
    RtResultVoid add_bin_compare_op(const NumericOpcodes& opcode);
    RtResultVoid add_conv(const NumericOpcodes& opcode, RtEvalStackDataType data_type);
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
    RtResultVoid add_ldelem(ll::OpCodeEnum opcode, RtEvalStackDataType data_type);
    RtResultVoid add_ldelem_any(metadata::RtClass* ele_klass);
    RtResultVoid add_stelem(ll::OpCodeEnum opcode, RtEvalStackDataType data_type);
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
    void setup_inst_resolved_data(InstructionOperands& output, const void* data);
    void setup_inst_klass(InstructionOperands& output);
    void setup_inst_method(InstructionOperands& output);
    utils::NotFreeList<size_t> find_finally_clause_idx_of_leave_target(const BasicBlock* leave_src, const BasicBlock* leave_target);
    LeaveSurroundingBlockType get_leave_surrounding_block_type(const BasicBlock* leave_src, const BasicBlock* leave_target);
    metadata::RtILExceptionClauseType get_cur_endfinally_or_fault_clause_type(const BasicBlock* cur_bb);

    RtResult<bool> transform_special_call_methods(InstructionOperands& output);
    RtResult<bool> transform_special_newobj_methods(InstructionOperands& output);
    RtResultVoid build_exception_clauses(RtInterpMethodInfo* interp_method);
    RtResult<uint32_t> translate_il_offset_to_ir_offset(uint32_t il_offset);
    RtResultVoid build_codes(RtInterpMethodInfo* interp_method);
    RtResultVoid emit_InitLocals(InstructionOperands& output);
    RtResultVoid emit_LdLoc(InstructionOperands& output);
    RtResultVoid emit_StLoc(InstructionOperands& output);
    RtResultVoid emit_LdcI4(InstructionOperands& output);
    RtResultVoid emit_LdcI8(InstructionOperands& output);
    RtResultVoid emit_LdStr(InstructionOperands& output);
    RtResultVoid emit_Dup(InstructionOperands& output);
    RtResultVoid emit_Ret(InstructionOperands& output);
    RtResultVoid emit_Br(InstructionOperands& output);
    RtResultVoid emit_Switch(InstructionOperands& output);
    RtResultVoid emit_Neg(InstructionOperands& output);
    RtResultVoid emit_Not(InstructionOperands& output);
    RtResultVoid emit_CastClass(InstructionOperands& output);
    RtResultVoid emit_IsInst(InstructionOperands& output);
    RtResultVoid emit_Box(InstructionOperands& output);
    RtResultVoid emit_BoxRefInplace(InstructionOperands& output);
    RtResultVoid emit_Unbox(InstructionOperands& output);
    RtResultVoid emit_UnboxAny(InstructionOperands& output);
    RtResultVoid emit_NewArr(InstructionOperands& output);
    RtResultVoid emit_Ldelema(InstructionOperands& output);
    RtResultVoid emit_LdelemAny(InstructionOperands& output);
    RtResultVoid emit_StelemAny(InstructionOperands& output);
    RtResultVoid emit_MkRefAny(InstructionOperands& output);
    RtResultVoid emit_RefAnyVal(InstructionOperands& output);
    RtResultVoid emit_LdToken(InstructionOperands& output);
    RtResultVoid emit_Ckfinite(InstructionOperands& output);
    RtResultVoid emit_Ldfld(InstructionOperands& output);
    RtResultVoid emit_Ldvfld(InstructionOperands& output);
    RtResultVoid emit_Ldflda(InstructionOperands& output);
    RtResultVoid emit_Stfld(InstructionOperands& output);
    RtResultVoid emit_Ldsfld(InstructionOperands& output);
    RtResultVoid emit_Ldsflda(InstructionOperands& output);
    RtResultVoid emit_Stsfld(InstructionOperands& output);
    RtResultVoid emit_Call(InstructionOperands& output);
    RtResultVoid emit_CallVirt(InstructionOperands& output);
    RtResultVoid emit_CallInternalCall(InstructionOperands& output);
    RtResultVoid emit_CallIntrinsic(InstructionOperands& output);
    RtResultVoid emit_CallPInvoke(InstructionOperands& output);
    RtResultVoid emit_CallAot(InstructionOperands& output);
    RtResultVoid emit_CallRuntimeImplemented(InstructionOperands& output);
    RtResultVoid emit_Calli(InstructionOperands& output);
    RtResultVoid emit_NewObj(InstructionOperands& output);
    RtResultVoid emit_NewObjInternalCall(InstructionOperands& output);
    RtResultVoid emit_NewObjIntrinsic(InstructionOperands& output);
    RtResultVoid emit_NewObjAot(InstructionOperands& output);
    RtResultVoid emit_Ldftn(InstructionOperands& output);
    RtResultVoid emit_Ldvirtftn(InstructionOperands& output);
    RtResultVoid emit_Leave(InstructionOperands& output);
    RtResultVoid emit_EndFinallyOrFault(InstructionOperands& output);
};
} // namespace interp
} // namespace leanclr
