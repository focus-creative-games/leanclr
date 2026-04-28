#include "hl_transformer.h"
#include "ll_transformer.h"
#include "il_opcodes.h"
#include "metadata/module_def.h"
#include "metadata/generic_metadata.h"
#include "metadata/aot_module.h"
#include "basic_block_splitter.h"
#include "vm/method.h"
#include "vm/class.h"
#include "vm/rt_string.h"
#include "vm/intrinsics.h"
#include "vm/internal_calls.h"
#include "vm/delegate.h"
#include "vm/type.h"
#include "utils/rt_vector.h"
#include "utils/mem_op.h"
#include "const_strs.h"

namespace leanclr
{
namespace interp
{
namespace hl
{

Transformer::Transformer(metadata::RtModuleDef* mod, const metadata::RtMethodInfo* method_info, const metadata::RtMethodBody& method_body,
                         alloc::MemPool& mem_pool)
    : _mod(mod), _method(method_info), _method_body(&method_body), _pool(&mem_pool), _vars(&mem_pool)
{
    _generic_context = _method->generic_method ? &_method->generic_method->generic_context : nullptr;
    _generic_container_context.klass = method_info->parent->generic_container;
    _generic_container_context.method = method_info->generic_container;
}

const metadata::RtMethodInfo* Transformer::get_method_info() const
{
    return _method;
}

const metadata::RtMethodBody* Transformer::get_method_body() const
{
    return _method_body;
}

metadata::RtModuleDef* Transformer::get_module() const
{
    return _mod;
}

const metadata::RtGenericContainerContext& Transformer::get_generic_container_context() const
{
    return _generic_container_context;
}

const metadata::RtGenericContext* Transformer::get_generic_context() const
{
    return _generic_context;
}

size_t Transformer::get_total_arg_and_local_stack_object_size() const
{
    return _total_arg_and_local_stack_object_size;
}

size_t Transformer::get_max_stack_size() const
{
    return _max_eval_stack_size;
}

bool Transformer::need_init_locals() const
{
    return (_method_body->flags & static_cast<uint16_t>(metadata::RtILMethodFormat::InitLocals)) != 0;
}

const Variable* Transformer::get_ret_var() const
{
    return _ret_var;
}

const BasicBlock* Transformer::get_basic_blocks() const
{
    return _basic_blocks;
}

size_t Transformer::get_cur_eval_stack_top() const
{
    return _cur_bb->eval_stack_top;
}

size_t Transformer::alloc_var_id()
{
    const size_t id = _next_var_id;
    _next_var_id += 1;
    return id;
}

RtResult<const Variable*> Transformer::alloc_variable_by_typesig(const metadata::RtTypeSig* type_sig, size_t eval_stack_offset)
{
    Variable* var = _pool->malloc_any_zeroed<Variable>();
    _vars.push_back(var);
    var->id = alloc_var_id();
    var->type_ = type_sig;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(ReduceTypeAndSize, reduce_type_and_size, InterpDefs::get_reduce_type_and_size_by_typesig(type_sig));
    var->reduce_type = reduce_type_and_size.reduce_type;
    var->data_type = InterpDefs::get_eval_stack_data_type_by_reduce_type(reduce_type_and_size.reduce_type);
    var->byte_size = reduce_type_and_size.byte_size;
    var->stack_object_size = InterpDefs::get_stack_object_size_by_byte_size(var->byte_size);
    var->eval_stack_offset = eval_stack_offset;

    RET_OK(static_cast<const Variable*>(var));
}

Variable* Transformer::create_eval_stack_variable(RtEvalStackDataType data_type, metadata::RtArgOrLocOrFieldReduceType reduce_type, size_t data_size,
                                                  size_t eval_stack_offset)
{
    Variable* eval_stack_var = _pool->malloc_any_zeroed<Variable>();
    _vars.push_back(eval_stack_var);
    eval_stack_var->id = alloc_var_id();
    eval_stack_var->type_ = nullptr;
    eval_stack_var->reduce_type = reduce_type;
    eval_stack_var->data_type = data_type;
    assert(data_size != 0);
    eval_stack_var->byte_size = data_size;
    eval_stack_var->stack_object_size = InterpDefs::get_stack_object_size_by_byte_size(data_size);
    eval_stack_var->eval_stack_offset = eval_stack_offset;
    return eval_stack_var;
}

RtResult<Variable*> Transformer::create_eval_stack_variable_from_type_sig(const metadata::RtTypeSig* type_sig, size_t eval_stack_offset)
{
    Variable* eval_stack_var = _pool->malloc_any_zeroed<Variable>();
    _vars.push_back(eval_stack_var);
    eval_stack_var->id = alloc_var_id();
    eval_stack_var->type_ = type_sig;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(ReduceTypeAndSize, reduce_type_and_size, InterpDefs::get_reduce_type_and_size_by_typesig(type_sig));
    eval_stack_var->reduce_type = reduce_type_and_size.reduce_type;
    eval_stack_var->data_type = InterpDefs::get_eval_stack_data_type_by_reduce_type(reduce_type_and_size.reduce_type);
    eval_stack_var->byte_size = reduce_type_and_size.byte_size;
    eval_stack_var->stack_object_size = InterpDefs::get_stack_object_size_by_byte_size(eval_stack_var->byte_size);
    eval_stack_var->eval_stack_offset = eval_stack_offset;

    RET_OK(eval_stack_var);
}

Variable* Transformer::create_eval_stack_variable_from_var(const Variable* var, size_t eval_stack_offset)
{
    Variable* eval_stack_var = _pool->malloc_any_zeroed<Variable>();
    _vars.push_back(eval_stack_var);
    eval_stack_var->id = alloc_var_id();
    eval_stack_var->type_ = var->type_;
    eval_stack_var->reduce_type = var->reduce_type;
    eval_stack_var->data_type = var->data_type;
    assert(var->byte_size != 0);
    eval_stack_var->byte_size = var->byte_size;
    eval_stack_var->stack_object_size = var->stack_object_size;
    eval_stack_var->eval_stack_offset = eval_stack_offset;
    return eval_stack_var;
}

const Variable* Transformer::alloc_exception_variable(size_t eval_stack_offset)
{
    return create_eval_stack_variable(RtEvalStackDataType::RefOrPtr, metadata::RtArgOrLocOrFieldReduceType::Ref, PTR_SIZE, eval_stack_offset);
}

RtResult<metadata::RtRuntimeHandle> Transformer::get_cached_runtime_handle(uint32_t token)
{
    auto it = _runtime_handle_cache.find(token);
    if (it != _runtime_handle_cache.end())
    {
        RET_OK(it->second);
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(metadata::RtRuntimeHandle, handle, get_raw_runtime_handle_from_token(token));
    _runtime_handle_cache[token] = handle;
    RET_OK(handle);
}

RtResult<const metadata::RtMethodInfo*> Transformer::get_method_from_token(uint32_t raw_token)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(metadata::RtRuntimeHandle, handle, get_cached_runtime_handle(raw_token));
    if (handle.is_method())
    {
        RET_OK(handle.method);
    }
    RET_ASSERT_ERR(RtErr::BadImageFormat);
}

RtResult<metadata::RtMethodSig> Transformer::get_standalone_method_sig_from_token(uint32_t token)
{
    metadata::RtModuleDef* mod = get_module();
    auto retMethodSig = mod->read_stadalone_method_sig(token, _generic_container_context, _generic_context);
    RET_ERR_ON_FAIL(retMethodSig);
    metadata::RtMethodSig& method_sig = retMethodSig.unwrap();
    if (_generic_context)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(method_sig.return_type, metadata::GenericMetadata::inflate_typesig(method_sig.return_type, _generic_context));
        RET_ERR_ON_FAIL(metadata::GenericMetadata::inflate_typesigs(method_sig.params, _generic_context));
    }
    return retMethodSig;
}

RtResult<const metadata::RtTypeSig*> Transformer::get_type_from_token(uint32_t raw_token)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(metadata::RtRuntimeHandle, handle, get_cached_runtime_handle(raw_token));
    if (handle.is_type())
    {
        RET_OK(handle.typeSig);
    }
    RET_ASSERT_ERR(RtErr::BadImageFormat);
}

RtResult<metadata::RtClass*> Transformer::get_class_from_token(uint32_t raw_token)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(metadata::RtRuntimeHandle, handle, get_cached_runtime_handle(raw_token));
    if (handle.is_type())
    {
        return vm::Class::get_class_from_typesig(handle.typeSig);
    }
    RET_ASSERT_ERR(RtErr::BadImageFormat);
}

RtResult<const metadata::RtFieldInfo*> Transformer::get_field_from_token(uint32_t raw_token)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(metadata::RtRuntimeHandle, handle, get_cached_runtime_handle(raw_token));
    if (handle.is_field())
    {
        RET_OK(handle.field);
    }
    RET_ASSERT_ERR(RtErr::BadImageFormat);
}

RtResult<metadata::RtRuntimeHandle> Transformer::get_raw_runtime_handle_from_token(uint32_t raw_token)
{
    metadata::RtModuleDef* mod = get_module();
    metadata::RtToken token = metadata::RtToken::decode(raw_token);

    // Decode and resolve the handle based on token type
    switch (token.table_type)
    {
    case metadata::TableType::TypeDef:
    case metadata::TableType::TypeRef:
    case metadata::TableType::TypeSpec:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(const metadata::RtTypeSig*, type,
                                                 mod->get_typesig_by_type_def_ref_spec_token(token, _generic_container_context, _generic_context));
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type));
        RET_ERR_ON_FAIL(vm::Class::initialize_all(klass));
        RET_OK(metadata::RtRuntimeHandle(type));
    }
    case metadata::TableType::Method:
    case metadata::TableType::MethodSpec:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(const metadata::RtMethodInfo*, method,
                                                 mod->get_method_by_token(token, _generic_container_context, _generic_context));
        RET_ERR_ON_FAIL(vm::Class::initialize_all(const_cast<metadata::RtClass*>(method->parent)));
        RET_OK(metadata::RtRuntimeHandle(method));
    }
    case metadata::TableType::MemberRef:
    {
        return mod->get_member_ref_by_rid(token.rid, _generic_container_context, _generic_context);
    }
    case metadata::TableType::Field:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(const metadata::RtFieldInfo*, field,
                                                 mod->get_field_by_token(token, _generic_container_context, _generic_context));
        RET_ERR_ON_FAIL(vm::Class::initialize_all(const_cast<metadata::RtClass*>(field->parent)));
        RET_OK(metadata::RtRuntimeHandle(field));
    }
    default:
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    }
}

GeneralInst* Transformer::create_inst(OpCodeEnum opcode)
{
    GeneralInst* ir_inst = _pool->malloc_any_zeroed<GeneralInst>();
    ir_inst->set_opcode(opcode);
    ir_inst->set_il_offset(_cur_il_offset);
    return ir_inst;
}

GeneralInst* Transformer::create_add_inst(OpCodeEnum opcode)
{
    GeneralInst* ir_inst = create_inst(opcode);
    _cur_bb->insts.push_back(ir_inst);
    return ir_inst;
}

void Transformer::internal_push_eval_stack_var_and_update_max(const Variable* var)
{
    _cur_bb->push(var);
    update_max_eval_stack_size(_cur_bb->eval_stack_top);
}

void Transformer::update_max_eval_stack_size(size_t new_size)
{
    if (new_size > _max_eval_stack_size)
    {
        _max_eval_stack_size = new_size;
    }
}

const Variable* Transformer::push_var_to_eval_stack(const Variable* var)
{
    Variable* eval_stack_var = create_eval_stack_variable_from_var(var, get_cur_eval_stack_top());
    internal_push_eval_stack_var_and_update_max(eval_stack_var);
    return eval_stack_var;
}

void Transformer::clear_eval_stack()
{
    _cur_bb->eval_stack.clear();
    _cur_bb->eval_stack_top = _eval_stack_base_offset;
}

Variable* Transformer::push_primitive_to_eval_stack(RtEvalStackDataType data_type)
{
    metadata::RtArgOrLocOrFieldReduceType reduce_type;
    size_t data_size;

    switch (data_type)
    {
    case RtEvalStackDataType::I4:
        reduce_type = metadata::RtArgOrLocOrFieldReduceType::I4;
        data_size = 4;
        break;
    case RtEvalStackDataType::I8:
        reduce_type = metadata::RtArgOrLocOrFieldReduceType::I8;
        data_size = 8;
        break;
    case RtEvalStackDataType::R4:
        reduce_type = metadata::RtArgOrLocOrFieldReduceType::R4;
        data_size = 4;
        break;
    case RtEvalStackDataType::R8:
        reduce_type = metadata::RtArgOrLocOrFieldReduceType::R8;
        data_size = 8;
        break;
    case RtEvalStackDataType::RefOrPtr:
        reduce_type = metadata::RtArgOrLocOrFieldReduceType::I;
        data_size = PTR_SIZE;
        break;
    case RtEvalStackDataType::Other:
        assert(false && "Cannot push 'Other' type to eval stack");
        return nullptr;
    }

    Variable* eval_stack_var = create_eval_stack_variable(data_type, reduce_type, data_size, get_cur_eval_stack_top());
    internal_push_eval_stack_var_and_update_max(eval_stack_var);
    return eval_stack_var;
}

Variable* Transformer::push_i4_to_eval_stack()
{
    Variable* eval_stack_var = create_eval_stack_variable(RtEvalStackDataType::I4, metadata::RtArgOrLocOrFieldReduceType::I4, 4, get_cur_eval_stack_top());
    internal_push_eval_stack_var_and_update_max(eval_stack_var);
    return eval_stack_var;
}

Variable* Transformer::push_ref_or_ptr_to_eval_stack()
{
    Variable* eval_stack_var =
        create_eval_stack_variable(RtEvalStackDataType::RefOrPtr, metadata::RtArgOrLocOrFieldReduceType::I, PTR_SIZE, get_cur_eval_stack_top());
    internal_push_eval_stack_var_and_update_max(eval_stack_var);
    return eval_stack_var;
}

RtResult<Variable*> Transformer::push_typesig_to_eval_stack(const metadata::RtTypeSig* type_sig)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(Variable*, eval_stack_var, create_eval_stack_variable_from_type_sig(type_sig, get_cur_eval_stack_top()));
    internal_push_eval_stack_var_and_update_max(eval_stack_var);
    RET_OK(eval_stack_var);
}

RtResult<Variable*> Transformer::push_class_to_eval_stack(const metadata::RtClass* klass)
{
    return push_typesig_to_eval_stack(klass->by_val);
}

RtResult<const Variable*> Transformer::get_top_var() const
{
    size_t stack_len = _cur_bb->eval_stack.size();
    if (stack_len == 0)
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    const Variable* top_var = _cur_bb->eval_stack.get(stack_len - 1);
    RET_OK(top_var);
}

RtResult<const Variable*> Transformer::pop_eval_stack()
{
    size_t stack_len = _cur_bb->eval_stack.size();
    if (stack_len == 0)
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    const Variable* top_var = _cur_bb->eval_stack.get(stack_len - 1);

    _cur_bb->eval_stack_top -= top_var->stack_object_size;
    _cur_bb->eval_stack.pop_unchecked();

    RET_OK(top_var);
}

void Transformer::add_prefix(il::OpCodePrefix prefix)
{
    _prefix = static_cast<il::OpCodePrefix>(static_cast<uint32_t>(prefix) | static_cast<uint32_t>(_prefix));
    _not_retset_prefix_after_cur_il = true;
}

void Transformer::set_prefix(il::OpCodePrefix prefix)
{
    _prefix = prefix;
    _not_retset_prefix_after_cur_il = true;
}

void Transformer::set_constrainted_prefix(metadata::RtClass* klass)
{
    _prefix = il::OpCodePrefix::Constrained;
    _not_retset_prefix_after_cur_il = true;
    _constrained_class = klass;
}

void Transformer::clear_prefix()
{
    _prefix = il::OpCodePrefix::None;
    _not_retset_prefix_after_cur_il = false;
}

RtResultVoid Transformer::add_init_locals()
{
    if (!need_init_locals() || _local_vars_count == 0)
        RET_VOID_OK();

    GeneralInst* ir = create_add_inst(OpCodeEnum::InitLocals);
    ir->set_locals_offset(_total_arg_stack_object_size);
    ir->set_size(_total_arg_and_local_stack_object_size - _total_arg_stack_object_size);
    RET_VOID_OK();
}

RtResultVoid Transformer::add_ldarg(size_t arg_idx)
{
    if (arg_idx >= _arg_vars_count)
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    const Variable* arg_var = _arg_vars[arg_idx];
    GeneralInst* ir = create_add_inst(OpCodeEnum::LdArg);
    ir->set_var_src(arg_var);
    ir->set_var_dst(push_var_to_eval_stack(arg_var));
    RET_VOID_OK();
}

RtResultVoid Transformer::add_starg(size_t arg_idx)
{
    if (arg_idx >= _arg_vars_count)
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    const Variable* arg_var = _arg_vars[arg_idx];
    RtResult<const Variable*> src_result = pop_eval_stack();
    RET_ERR_ON_FAIL(src_result);

    GeneralInst* ir = create_add_inst(OpCodeEnum::StArg);
    ir->set_var_src(src_result.unwrap());
    ir->set_var_dst(arg_var);
    RET_VOID_OK();
}

RtResultVoid Transformer::add_ldarga(size_t arg_idx)
{
    if (arg_idx >= _arg_vars_count)
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    const Variable* arg_var = _arg_vars[arg_idx];
    GeneralInst* ir = create_add_inst(OpCodeEnum::LdArga);
    ir->set_var_src(arg_var);
    ir->set_var_dst(push_ref_or_ptr_to_eval_stack());
    RET_VOID_OK();
}

RtResultVoid Transformer::add_ldloc(size_t local_idx)
{
    if (local_idx >= _local_vars_count)
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    const Variable* local_var = _local_vars[local_idx];
    GeneralInst* ir = create_add_inst(OpCodeEnum::LdLoc);
    ir->set_var_src(local_var);
    ir->set_var_dst(push_var_to_eval_stack(local_var));
    RET_VOID_OK();
}

RtResultVoid Transformer::add_stloc(size_t local_idx)
{
    if (local_idx >= _local_vars_count)
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    const Variable* local_var = _local_vars[local_idx];
    RtResult<const Variable*> src_result = pop_eval_stack();
    RET_ERR_ON_FAIL(src_result);

    GeneralInst* ir = create_add_inst(OpCodeEnum::StLoc);
    ir->set_var_src(src_result.unwrap());
    ir->set_var_dst(local_var);
    RET_VOID_OK();
}

RtResultVoid Transformer::add_ldloca(size_t local_idx)
{
    if (local_idx >= _local_vars_count)
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    const Variable* local_var = _local_vars[local_idx];
    GeneralInst* ir = create_add_inst(OpCodeEnum::LdLoca);
    ir->set_var_src(local_var);
    ir->set_var_dst(push_ref_or_ptr_to_eval_stack());
    RET_VOID_OK();
}

RtResultVoid Transformer::add_ldnull()
{
    GeneralInst* ir = create_add_inst(OpCodeEnum::LdNull);
    ir->set_var_dst(push_ref_or_ptr_to_eval_stack());
    RET_VOID_OK();
}

RtResultVoid Transformer::add_ldci4(int32_t value)
{
    GeneralInst* ir = create_add_inst(OpCodeEnum::LdcI4);
    ir->set_i4(value);
    ir->set_var_dst(push_i4_to_eval_stack());
    RET_VOID_OK();
}

RtResultVoid Transformer::add_ldci8(int64_t value)
{
    GeneralInst* ir = create_add_inst(OpCodeEnum::LdcI8);
    ir->set_i8(value);
    ir->set_var_dst(push_primitive_to_eval_stack(RtEvalStackDataType::I8));
    RET_VOID_OK();
}

RtResultVoid Transformer::add_ldcr4(float value)
{
    GeneralInst* ir = create_add_inst(OpCodeEnum::LdcR4);
    ir->set_r4(value);
    ir->set_var_dst(push_primitive_to_eval_stack(RtEvalStackDataType::R4));
    RET_VOID_OK();
}

RtResultVoid Transformer::add_ldcr8(double value)
{
    GeneralInst* ir = create_add_inst(OpCodeEnum::LdcR8);
    ir->set_r8(value);
    ir->set_var_dst(push_primitive_to_eval_stack(RtEvalStackDataType::R8));
    RET_VOID_OK();
}

RtResultVoid Transformer::add_dup()
{
    auto src_result = get_top_var();
    RET_ERR_ON_FAIL(src_result);
    const Variable* src = src_result.unwrap();
    GeneralInst* ir = create_add_inst(OpCodeEnum::Dup);
    ir->set_var_src(src);
    ir->set_var_dst(push_var_to_eval_stack(src));
    RET_VOID_OK();
}

RtResultVoid Transformer::add_ret()
{
    GeneralInst* ir = create_add_inst(OpCodeEnum::Ret);
    if (!vm::Method::is_void_return(_method))
    {
        auto pop_result = pop_eval_stack();
        RET_ERR_ON_FAIL(pop_result);
        ir->set_var_src(pop_result.unwrap());
    }
    else
    {
        ir->set_var_src_invalid();
    }
    RET_VOID_OK();
}

RtResult<BasicBlock*> Transformer::get_branch_target_bb(size_t global_target_offset)
{
    auto it = _il_offset_to_basic_block.find(static_cast<uint32_t>(global_target_offset));
    if (it == _il_offset_to_basic_block.end())
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    RET_OK(it->second);
}

RtResultVoid Transformer::setup_target_branch_eval_stack(BasicBlock* target)
{
    size_t cur_eval_stack_size = _cur_bb->eval_stack.size();
    if (target->inited_eval_stack)
    {
        if (target->in_eval_stack_top != _cur_bb->eval_stack_top || target->in_eval_stack.size() != cur_eval_stack_size)
            RET_ASSERT_ERR(RtErr::ExecutionEngine);

        for (size_t i = 0; i < cur_eval_stack_size; ++i)
        {
            const Variable* target_var = target->in_eval_stack[i];
            const Variable* cur_var = _cur_bb->eval_stack[i];
            if (target_var->data_type != cur_var->data_type || target_var->eval_stack_offset != cur_var->eval_stack_offset)
                RET_ASSERT_ERR(RtErr::ExecutionEngine);
        }
    }
    else
    {
        assert(target->in_eval_stack.empty());
        target->inited_eval_stack = true;
        target->in_eval_stack_top = _cur_bb->eval_stack_top;
        target->in_eval_stack.push_range(_cur_bb->eval_stack.data(), cur_eval_stack_size);
    }
    RET_VOID_OK();
}

RtResultVoid Transformer::setup_next_and_target_branch_eval_stack(BasicBlock* target)
{
    RET_ERR_ON_FAIL(setup_target_branch_eval_stack(target));

    if (_cur_bb->next_bb)
    {
        RET_ERR_ON_FAIL(setup_target_branch_eval_stack(_cur_bb->next_bb));
    }
    RET_VOID_OK();
}

RtResultVoid Transformer::add_br(uint32_t next_offset, int32_t target_offset)
{
    if (target_offset == 0)
        RET_VOID_OK();
    // according to ECMA-335, the eval stack of branch target before current instruction should be empty.
    // but coreclr relaxes this rule, so we also relax it here.
    // if (target_offset < 0 && !_cur_bb->eval_stack.is_empty())
    //     RET_ASSERT_ERR(RtErr::ExecutionEngine);

    const size_t target_il_offset = static_cast<size_t>(static_cast<ptrdiff_t>(next_offset) + static_cast<ptrdiff_t>(target_offset));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(BasicBlock*, target_bb, get_branch_target_bb(target_il_offset));

    GeneralInst* ir = create_add_inst(OpCodeEnum::Br);
    ir->set_branch_target(target_bb);
    RET_ERR_ON_FAIL(setup_target_branch_eval_stack(target_bb));
    RET_VOID_OK();
}

RtResultVoid Transformer::add_brtrue_or_false(uint32_t next_offset, int32_t target_offset, bool is_true)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, cond_var, pop_eval_stack());
    if (target_offset == 0)
        RET_VOID_OK();

    const size_t target_il_offset = static_cast<size_t>(static_cast<ptrdiff_t>(next_offset) + static_cast<ptrdiff_t>(target_offset));
    auto target_bb_result = get_branch_target_bb(target_il_offset);
    RET_ERR_ON_FAIL(target_bb_result);
    BasicBlock* target_bb = target_bb_result.unwrap();

    GeneralInst* ir = create_add_inst(is_true ? OpCodeEnum::BrTrue : OpCodeEnum::BrFalse);
    ir->set_var_src(cond_var);
    ir->set_branch_target(target_bb);
    RET_ERR_ON_FAIL(setup_next_and_target_branch_eval_stack(target_bb));
    RET_VOID_OK();
}

RtResultVoid Transformer::add_condition_branch(uint32_t next_offset, int32_t target_offset, OpCodeEnum opcode)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, right_var, pop_eval_stack());
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, left_var, pop_eval_stack());
    if (target_offset == 0)
        RET_VOID_OK();

    if (left_var->data_type != right_var->data_type)
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    const size_t target_il_offset = static_cast<size_t>(static_cast<ptrdiff_t>(next_offset) + static_cast<ptrdiff_t>(target_offset));

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(BasicBlock*, target_bb, get_branch_target_bb(target_il_offset));

    GeneralInst* ir = create_add_inst(opcode);
    ir->set_var_arg1(left_var);
    ir->set_var_arg2(right_var);
    ir->set_branch_target(target_bb);
    RET_ERR_ON_FAIL(setup_next_and_target_branch_eval_stack(target_bb));
    RET_VOID_OK();
}

RtResultVoid Transformer::add_switch(uint32_t next_offset, const void* case_targets, size_t count)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, key_var, pop_eval_stack());

    if (key_var->data_type != RtEvalStackDataType::I4)
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    const BasicBlock** targets = _pool->calloc_any<const BasicBlock*>(count);
    bool any_not_zero_branch = false;
    for (size_t i = 0; i < count; ++i)
    {
        int32_t rel_offset = static_cast<int32_t>(utils::MemOp::read_u32_may_unaligned(((const int32_t*)case_targets) + i));
        if (rel_offset != 0)
            any_not_zero_branch = true;
    }

    // return directly if all branches are zero offset
    if (!any_not_zero_branch)
        RET_VOID_OK();

    GeneralInst* ir = create_add_inst(OpCodeEnum::Switch);
    ir->set_var_arg1(key_var);

    for (size_t i = 0; i < count; ++i)
    {
        const size_t target_il_offset = static_cast<size_t>(static_cast<ptrdiff_t>(next_offset) +
                                                            static_cast<ptrdiff_t>(utils::MemOp::read_u32_may_unaligned(((const int32_t*)case_targets) + i)));
        auto target_bb_result = get_branch_target_bb(target_il_offset);
        RET_ERR_ON_FAIL(target_bb_result);
        BasicBlock* target_bb = target_bb_result.unwrap();
        targets[i] = target_bb;
        RET_ERR_ON_FAIL(setup_target_branch_eval_stack(target_bb));
    }

    ir->set_switch_targets(targets, count);

    if (_cur_bb->next_bb)
    {
        RET_ERR_ON_FAIL(setup_target_branch_eval_stack(_cur_bb->next_bb));
    }
    RET_VOID_OK();
}

RtResultVoid Transformer::add_ldind(OpCodeEnum opcode, metadata::RtArgOrLocOrFieldReduceType reduce_type)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, src_var, pop_eval_stack());

    GeneralInst* ir = create_add_inst(opcode);

    ir->set_var_src(src_var);
    RtEvalStackDataType data_type = InterpDefs::get_eval_stack_data_type_by_reduce_type(reduce_type);
    ir->set_var_dst(push_primitive_to_eval_stack(data_type));
    ir->set_prefix(_prefix);
    RET_VOID_OK();
}

RtResultVoid Transformer::add_stind(OpCodeEnum opcode)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, value_var, pop_eval_stack());
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, addr_var, pop_eval_stack());

    GeneralInst* ir = create_add_inst(opcode);

    ir->set_var_src(value_var);
    ir->set_var_dst(addr_var);
    ir->set_prefix(_prefix);
    RET_VOID_OK();
}

RtResult<RtEvalStackDataType> Transformer::calc_bin_arith_result_type(RtEvalStackDataType left, RtEvalStackDataType right)
{
    RtEvalStackDataType ret_type;
    switch (left)
    {
    case RtEvalStackDataType::I4:
        switch (right)
        {
        case RtEvalStackDataType::I4:
            ret_type = RtEvalStackDataType::I4;
            break;
        case RtEvalStackDataType::I8:
            ret_type = RtEvalStackDataType::I8;
            break;
        case RtEvalStackDataType::RefOrPtr:
            ret_type = RtEvalStackDataType::RefOrPtr;
            break;
        default:
            RET_ASSERT_ERR(RtErr::ExecutionEngine);
        }
        break;
    case RtEvalStackDataType::I8:
        switch (right)
        {
        case RtEvalStackDataType::I4:
        case RtEvalStackDataType::I8:
        case RtEvalStackDataType::RefOrPtr:
            ret_type = RtEvalStackDataType::I8;
            break;
        default:
            RET_ASSERT_ERR(RtErr::ExecutionEngine);
        }
        break;
    case RtEvalStackDataType::R4:
        switch (right)
        {
        case RtEvalStackDataType::R4:
            ret_type = RtEvalStackDataType::R4;
            break;
        case RtEvalStackDataType::R8:
            ret_type = RtEvalStackDataType::R8;
            break;
        default:
            RET_ASSERT_ERR(RtErr::ExecutionEngine);
        }
        break;
    case RtEvalStackDataType::R8:
        switch (right)
        {
        case RtEvalStackDataType::R4:
        case RtEvalStackDataType::R8:
            ret_type = RtEvalStackDataType::R8;
            break;
        default:
            RET_ASSERT_ERR(RtErr::ExecutionEngine);
        }
        break;
    case RtEvalStackDataType::RefOrPtr:
        switch (right)
        {
        case RtEvalStackDataType::I4:
        case RtEvalStackDataType::RefOrPtr:
            ret_type = RtEvalStackDataType::RefOrPtr;
            break;
        case RtEvalStackDataType::I8:
            ret_type = RtEvalStackDataType::I8;
            break;
        default:
            RET_ASSERT_ERR(RtErr::ExecutionEngine);
        }
        break;
    default:
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    }
    RET_OK(ret_type);
}

RtResult<size_t> Transformer::calc_arith_op_eval_data_size_by_data_type(RtEvalStackDataType data_type)
{
    switch (data_type)
    {
    case RtEvalStackDataType::I4:
        RET_OK(1);
    case RtEvalStackDataType::I8:
        RET_OK(3);
    case RtEvalStackDataType::RefOrPtr:
        RET_OK(2);
    default:
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    }
}

RtResultVoid Transformer::insert_conv_from_to(const Variable* from_var, const Variable* to_var)
{
    OpCodeEnum opcode;
    switch (to_var->data_type)
    {
    case RtEvalStackDataType::I4:
        opcode = OpCodeEnum::ConvI4;
        break;
    case RtEvalStackDataType::I8:
        opcode = OpCodeEnum::ConvI8;
        break;
    case RtEvalStackDataType::RefOrPtr:
        opcode = OpCodeEnum::ConvI;
        break;
    default:
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    }

    GeneralInst* ir = create_add_inst(opcode);

    ir->set_var_src(from_var);
    Variable* converted_var = create_eval_stack_variable(to_var->data_type, to_var->reduce_type, to_var->byte_size, from_var->eval_stack_offset);
    ir->set_var_dst(converted_var);
    RET_VOID_OK();
}

RtResultVoid Transformer::insert_conv_for_bin_arith_op(const Variable* left, const Variable* right)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(size_t, left_size, calc_arith_op_eval_data_size_by_data_type(left->data_type));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(size_t, right_size, calc_arith_op_eval_data_size_by_data_type(right->data_type));

    if (left_size == right_size)
        RET_VOID_OK();

    if (left_size < right_size)
    {
        RET_ERR_ON_FAIL(insert_conv_from_to(left, right));
    }
    else
    {
        RET_ERR_ON_FAIL(insert_conv_from_to(right, left));
    }
    RET_VOID_OK();
}

RtResultVoid Transformer::add_bin_arith_op(OpCodeEnum opcode)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, right, pop_eval_stack());
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, left, pop_eval_stack());

    if (left->data_type != right->data_type)
    {
        RET_ERR_ON_FAIL(insert_conv_for_bin_arith_op(left, right));
    }

    GeneralInst* ir = create_add_inst(opcode);

    ir->set_var_arg1(left);
    ir->set_var_arg2(right);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtEvalStackDataType, result_type, calc_bin_arith_result_type(left->data_type, right->data_type));
    ir->set_var_dst(push_primitive_to_eval_stack(result_type));
    RET_VOID_OK();
}

RtResult<RtEvalStackDataType> Transformer::calc_bitop_result_type(RtEvalStackDataType left, RtEvalStackDataType right)
{
    RtEvalStackDataType ret_type;
    switch (left)
    {
    case RtEvalStackDataType::I4:
        switch (right)
        {
        case RtEvalStackDataType::I4:
            ret_type = RtEvalStackDataType::I4;
            break;
        case RtEvalStackDataType::I8:
            ret_type = RtEvalStackDataType::I8;
            break;
        case RtEvalStackDataType::RefOrPtr:
            ret_type = RtEvalStackDataType::RefOrPtr;
            break;
        default:
            RET_ASSERT_ERR(RtErr::ExecutionEngine);
        }
        break;
    case RtEvalStackDataType::I8:
        switch (right)
        {
        case RtEvalStackDataType::I4:
        case RtEvalStackDataType::I8:
        case RtEvalStackDataType::RefOrPtr:
            ret_type = RtEvalStackDataType::I8;
            break;
        default:
            RET_ASSERT_ERR(RtErr::ExecutionEngine);
        }
        break;
    case RtEvalStackDataType::RefOrPtr:
        switch (right)
        {
        case RtEvalStackDataType::I4:
        case RtEvalStackDataType::RefOrPtr:
            ret_type = RtEvalStackDataType::RefOrPtr;
            break;
        case RtEvalStackDataType::I8:
            ret_type = RtEvalStackDataType::I8;
            break;
        default:
            RET_ASSERT_ERR(RtErr::ExecutionEngine);
        }
        break;
    default:
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    }
    RET_OK(ret_type);
}

RtResultVoid Transformer::add_bin_bit_op(OpCodeEnum opcode)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, right, pop_eval_stack());
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, left, pop_eval_stack());

    GeneralInst* ir = create_add_inst(opcode);

    ir->set_var_arg1(left);
    ir->set_var_arg2(right);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtEvalStackDataType, result_type, calc_bitop_result_type(left->data_type, right->data_type));
    ir->set_var_dst(push_primitive_to_eval_stack(result_type));
    RET_VOID_OK();
}

RtResultVoid Transformer::add_bit_shift_op(OpCodeEnum opcode)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, shift_amount, pop_eval_stack());
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, value, pop_eval_stack());

    if (value->is_not_i32_and_i64_and_native_int())
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    if (shift_amount->is_not_i32_and_native_int())
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    GeneralInst* ir = create_add_inst(opcode);

    ir->set_var_arg1(value);
    ir->set_var_arg2(shift_amount);
    ir->set_var_dst(push_primitive_to_eval_stack(value->data_type));
    RET_VOID_OK();
}

RtResultVoid Transformer::add_neg()
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, src, pop_eval_stack());
    if (!src->is_primitive())
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    GeneralInst* ir = create_add_inst(OpCodeEnum::Neg);
    ir->set_var_src(src);
    ir->set_var_dst(push_primitive_to_eval_stack(src->data_type));
    RET_VOID_OK();
}

RtResultVoid Transformer::add_not()
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, src, pop_eval_stack());
    if (src->is_not_i32_and_i64_and_native_int())
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    GeneralInst* ir = create_add_inst(OpCodeEnum::Not);

    ir->set_var_src(src);
    ir->set_var_dst(push_primitive_to_eval_stack(src->data_type));
    RET_VOID_OK();
}

RtResultVoid Transformer::add_bin_compare_op(OpCodeEnum opcode)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, right, pop_eval_stack());
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, left, pop_eval_stack());

    if (left->data_type != right->data_type)
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    if (!left->is_primitive())
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    GeneralInst* ir = create_add_inst(opcode);

    ir->set_var_arg1(left);
    ir->set_var_arg2(right);
    ir->set_var_dst(push_primitive_to_eval_stack(RtEvalStackDataType::I4));
    RET_VOID_OK();
}

RtResultVoid Transformer::add_conv(OpCodeEnum opcode, RtEvalStackDataType data_type)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, src, pop_eval_stack());

    // If the source is already of the target type, just push it back
    if (src->data_type == data_type)
    {
        internal_push_eval_stack_var_and_update_max(src);
        RET_VOID_OK();
    }
    if (!src->is_primitive())
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    GeneralInst* ir = create_add_inst(opcode);

    ir->set_var_src(src);
    ir->set_var_dst(push_primitive_to_eval_stack(data_type));
    RET_VOID_OK();
}

RtResultVoid Transformer::add_ckfinite()
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, val, get_top_var());

    if (!val->is_f32_or_f64())
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    // Check if the value is a finite number (not NaN or Inf)
    GeneralInst* ir = create_add_inst(OpCodeEnum::Ckfinite);
    ir->set_var_src(val);
    RET_VOID_OK();
}

RtResult<const metadata::RtMethodInfo*> Transformer::try_redirect_newobj_method(const metadata::RtMethodInfo* method)
{
    const metadata::RtClass* klass = method->parent;
    metadata::RtModuleDef* mod = klass->image;

    if (!mod->is_corlib())
    {
        RET_OK((const metadata::RtMethodInfo*)nullptr);
    }

    auto& corlibTypes = vm::Class::get_corlib_types();
    uint32_t param_count = method->parameter_count;
    if (klass == corlibTypes.cls_string)
    {
        // redirect `System.Void System.String::.ctor(System.SByte*,System.Int32,System.Int32,System.Text.Encoding)` to String::Ctor
        if (std::strcmp(method->name, STR_CTOR) == 0 && method->parameter_count == 4)
        {
            RET_OK(vm::String::get_redirected_ctor_method());
        }
    }

    // For now, don't redirect any methods - return None
    // In the future, this could redirect to String constructor or other special cases
    RET_OK((const metadata::RtMethodInfo*)nullptr);
}

RtResultVoid Transformer::add_call_common(const metadata::RtMethodInfo* method, metadata::RtInvokerType invoker_type, metadata::RtInvokeMethodPointer invoker,
                                          bool is_new_obj, bool is_call_vir)
{
    // Get parameter count including 'this' if instance method
    size_t param_count = is_new_obj ? vm::Method::get_param_count_exclude_this(method) : vm::Method::get_param_count_include_this(method);

    // Pop parameters in reverse order
    const Variable** params = nullptr;
    if (param_count > 0)
    {
        params = _pool->calloc_any<const Variable*>(param_count);
        for (size_t i = param_count; i > 0; --i)
        {
            UNWRAP_OR_RET_ERR_ON_FAIL(params[i - 1], pop_eval_stack());
        }
    }

    // Create the call instruction
    OpCodeEnum opcode;
    switch (invoker_type)
    {
    case metadata::RtInvokerType::Interpreter:
    case metadata::RtInvokerType::InterpreterVirtualAdjustThunk:
        opcode = is_call_vir ? OpCodeEnum::CallVirt : OpCodeEnum::Call;
        break;
    case metadata::RtInvokerType::InternalCall:
        opcode = OpCodeEnum::CallInternalCall;
        break;
    case metadata::RtInvokerType::Intrinsic:
    case metadata::RtInvokerType::CustomInstrinsic:
        opcode = OpCodeEnum::CallIntrinsic;
        break;
    case metadata::RtInvokerType::PInvoke:
        opcode = OpCodeEnum::CallPInvoke;
        break;
    case metadata::RtInvokerType::Aot:
        opcode = OpCodeEnum::CallAot;
        break;
    case metadata::RtInvokerType::RuntimeImpl:
        opcode = OpCodeEnum::CallRuntimeImplemented;
        break;
    case metadata::RtInvokerType::NewObjInternalCall:
        opcode = OpCodeEnum::NewObjInternalCall;
        break;
    case metadata::RtInvokerType::NewObjIntrinsic:
        opcode = OpCodeEnum::NewObjIntrinsic;
        break;
    case metadata::RtInvokerType::NotImplemented:
        // we use CallIntrinsic opcode to mark NotImplemented methods
        opcode = OpCodeEnum::CallIntrinsic;
        break;
    default:
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    }

    GeneralInst* ir = create_add_inst(opcode);
    ir->set_method_and_params(method, get_cur_eval_stack_top(), params);
    ir->set_prefix(_prefix);
    if (invoker_type == metadata::RtInvokerType::NewObjIntrinsic)
    {
        ir->set_invoker_idx(vm::Intrinsics::register_intrinsic_invoker_id(invoker));
    }
    else if (invoker_type == metadata::RtInvokerType::NewObjInternalCall)
    {
        ir->set_invoker_idx(vm::InternalCalls::register_internal_call_invoker_id(invoker));
    }

    if (is_new_obj)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(Variable*, this_var, push_class_to_eval_stack(method->parent));
        ir->set_var_dst(this_var);
    }
    else if (!vm::Method::is_void_return(method))
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(Variable*, return_var, push_typesig_to_eval_stack(method->return_type));
        ir->set_var_dst(return_var);
    }

    RET_VOID_OK();
}

RtResultVoid Transformer::add_call(const metadata::RtMethodInfo* method)
{
    return add_call_common(method, method->invoker_type, method->invoke_method_ptr, false, false);
}

RtResult<bool> Transformer::try_handle_newobj_intrinsic(const metadata::RtMethodInfo* method)
{
    const metadata::RtClass* klass = method->parent;
    if (vm::Class::is_array_or_szarray(klass))
    {
        RET_ERR_ON_FAIL(add_call_common(method, metadata::RtInvokerType::NewObjIntrinsic, method->invoke_method_ptr, true, false));
        RET_OK(true);
    }
    if (vm::Class::is_multicastdelegate_subclass(klass))
    {
        RET_ERR_ON_FAIL(add_call_common(method, metadata::RtInvokerType::NewObjIntrinsic, vm::Delegate::newobj_delegate_invoker, true, false));
        RET_OK(true);
    }

    metadata::RtModuleDef* mod = klass->image;
    if (!mod->is_corlib())
    {
        RET_OK(false);
    }
    if (std::strcmp(klass->name, "ByReference`1") == 0)
    {
        // nothing to do for ByReference<T> constructor
        RET_OK(true);
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::InternalCallInvoker, icalls_invoker, vm::InternalCalls::get_newobj_internal_call_by_method(method));
    if (icalls_invoker)
    {
        RET_ERR_ON_FAIL(add_call_common(method, metadata::RtInvokerType::NewObjInternalCall, icalls_invoker, true, false));
        RET_OK(true);
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::IntrinsicInvoker, intrinsic_invoker, vm::Intrinsics::get_newobj_intrinsic_by_method(method));
    if (intrinsic_invoker)
    {
        RET_ERR_ON_FAIL(add_call_common(method, metadata::RtInvokerType::NewObjIntrinsic, intrinsic_invoker, true, false));
        RET_OK(true);
    }
    RET_OK(false);
}

RtResultVoid Transformer::add_newobj(const metadata::RtMethodInfo* method)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, target_method, try_redirect_newobj_method(method));
    if (target_method)
    {
        // if redirected to non-constructor method, treat as regular call
        if (std::strcmp(target_method->name, STR_CTOR) != 0)
        {
            // Special handling for String constructor redirection
            return add_call(target_method);
        }
    }
    else
    {
        target_method = method;
    }
    // Check if this is an intrinsic newobj
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, handle_succ, try_handle_newobj_intrinsic(target_method));
    if (handle_succ)
        RET_VOID_OK();

    assert(target_method->invoker_type == metadata::RtInvokerType::Interpreter || target_method->invoker_type == metadata::RtInvokerType::Aot);
    // For regular newobj, pop parameters in reverse order
    size_t param_count = vm::Method::get_param_count_exclude_this(target_method);
    const Variable** params = param_count > 0 ? _pool->calloc_any<const Variable*>(param_count) : nullptr;

    const metadata::RtClass* klass = target_method->parent;
    size_t old_eval_stack_top = get_cur_eval_stack_top();
    size_t new_eval_stack_top;
    if (vm::Class::is_value_type(klass))
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(new_eval_stack_top, vm::Type::get_size_of_type(klass->by_val));
    }
    else
    {
        new_eval_stack_top = 1;
    }
    new_eval_stack_top += old_eval_stack_top;
    update_max_eval_stack_size(new_eval_stack_top);

    for (size_t i = param_count; i > 0; --i)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(params[i - 1], pop_eval_stack());
    }

    size_t total_param_stack_object_size = old_eval_stack_top - get_cur_eval_stack_top();
    GeneralInst* ir = create_add_inst(target_method->invoker_type == metadata::RtInvokerType::Interpreter ? OpCodeEnum::NewObj : OpCodeEnum::NewObjAot);
    ir->set_newobj_method_and_params(target_method, get_cur_eval_stack_top(), total_param_stack_object_size, params);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(Variable*, obj_var, push_class_to_eval_stack(klass));
    ir->set_var_dst(obj_var);

    RET_VOID_OK();
}

RtResultVoid Transformer::add_enum_hash_code_call(metadata::RtClass* enum_klass)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(ReduceTypeAndSize, type_and_size, InterpDefs::get_reduce_type_and_size_by_typesig(enum_klass->by_val));
    OpCodeEnum opcode;
    switch (type_and_size.reduce_type)
    {
    case metadata::RtArgOrLocOrFieldReduceType::I1:
        opcode = OpCodeEnum::LdIndI1;
        break;
    case metadata::RtArgOrLocOrFieldReduceType::U1:
        opcode = OpCodeEnum::LdIndU1;
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I2:
        opcode = OpCodeEnum::LdIndU2; // in the implementation, short enum is always unsigned for load
        break;
    case metadata::RtArgOrLocOrFieldReduceType::U2:
        opcode = OpCodeEnum::LdIndU2;
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I4:
        opcode = OpCodeEnum::LdIndI4;
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I8:
    {
        GeneralInst* ir = create_add_inst(OpCodeEnum::GetEnumLongHashCode);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, addr_result, pop_eval_stack());
        const Variable* dst_var = push_i4_to_eval_stack();
        ir->set_var_src(addr_result);
        ir->set_var_dst(dst_var);
        RET_VOID_OK();
    }
    default:
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    }
    return add_ldind(opcode, type_and_size.reduce_type);
}

RtResultVoid Transformer::add_box_ref_inplace(metadata::RtClass* klass, size_t eval_stack_idx, const Variable* obj)
{
    GeneralInst* ir = create_add_inst(OpCodeEnum::BoxRefInplace);
    Variable* dst = create_eval_stack_variable(RtEvalStackDataType::RefOrPtr, metadata::RtArgOrLocOrFieldReduceType::Ref, PTR_SIZE, obj->eval_stack_offset);

    ir->set_var_src(obj);
    ir->set_var_dst(dst);
    ir->set_class(klass);
    RET_VOID_OK();
}

RtResultVoid Transformer::add_ldind_ref_inplace(size_t eval_stack_idx, const Variable* obj)
{
    GeneralInst* ir = create_add_inst(OpCodeEnum::LdIndRef);
    Variable* dst = create_eval_stack_variable(RtEvalStackDataType::RefOrPtr, metadata::RtArgOrLocOrFieldReduceType::Ref, PTR_SIZE, obj->eval_stack_offset);
    _cur_bb->eval_stack[eval_stack_idx] = dst;
    ir->set_var_src(obj);
    ir->set_var_dst(dst);
    RET_VOID_OK();
}

RtResultVoid Transformer::add_callvirt(const metadata::RtMethodInfo* method)
{
    // FIXME: coreclr supports callvir on static methods, we currently don't support that.
    if (!vm::Method::is_instance(method))
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    size_t param_count = vm::Method::get_param_count_include_this(method);
    if (_cur_bb->eval_stack.size() < param_count)
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    if (((uint32_t)_prefix & (uint32_t)il::OpCodePrefix::Constrained) != 0)
    {
        size_t obj_index = _cur_bb->eval_stack.size() - param_count;
        const Variable* obj_var = _cur_bb->eval_stack[obj_index];
        metadata::RtClass* cons_klass = _constrained_class;
        _constrained_class = nullptr;
        RET_ERR_ON_FAIL(vm::Class::initialize_all(cons_klass));
        if (!vm::Method::is_virtual(method) && !vm::Class::is_object_class(method->parent))
        {
            RET_ASSERT_ERR(RtErr::ExecutionEngine);
        }
        if (vm::Class::is_value_type(cons_klass))
        {
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, cons_method,
                                                    vm::Method::get_virtual_method_impl_on_klass(cons_klass, method));
            if (cons_method->parent == cons_klass)
            {
                return add_call(cons_method);
            }
            else if (vm::Class::is_enum_type(cons_klass) && std::strcmp(cons_method->name, STR_GETHASHCODE) == 0 && cons_method->parameter_count == 0)
            {
                return add_enum_hash_code_call(cons_klass);
            }
            else
            {
                RET_ERR_ON_FAIL(add_box_ref_inplace(cons_klass, obj_index, obj_var));
            }
        }
        else
        {
            RET_ERR_ON_FAIL(add_ldind_ref_inplace(obj_index, obj_var));
        }
    }

    if (vm::Method::is_devirtualed(method))
    {
        return add_call(method);
    }
    else
    {
        return add_call_common(method, metadata::RtInvokerType::Interpreter, method->invoke_method_ptr, false, true);
    }
}

RtResultVoid Transformer::add_calli(const metadata::RtMethodSig& method_sig)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, func_ptr, pop_eval_stack());

    // Pop parameters in reverse order
    size_t param_count = method_sig.params.size() + vm::Method::has_this(&method_sig);
    const Variable** params = nullptr;
    if (param_count > 0)
    {
        params = _pool->calloc_any<const Variable*>(param_count);
        for (size_t i = param_count; i > 0; --i)
        {
            UNWRAP_OR_RET_ERR_ON_FAIL(params[i - 1], pop_eval_stack());
        }
    }

    GeneralInst* ir = create_add_inst(OpCodeEnum::Calli);
    ir->set_prefix(_prefix);
    const metadata::RtMethodSig* sig_copy = new (get_module()->get_mem_pool().malloc_any_zeroed<metadata::RtMethodSig>()) metadata::RtMethodSig(method_sig);
    ir->set_method_sig_and_params(sig_copy, get_cur_eval_stack_top(), func_ptr, params);
    if (!method_sig.return_type->is_void())
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, ret_var, push_typesig_to_eval_stack(method_sig.return_type));
        ir->set_var_dst(ret_var);
    }
    RET_VOID_OK();
}

RtResultVoid Transformer::add_ldftn(const metadata::RtMethodInfo* method)
{
    GeneralInst* ir = create_add_inst(OpCodeEnum::Ldftn);
    ir->set_method(method);
    ir->set_var_dst(push_ref_or_ptr_to_eval_stack());
    RET_VOID_OK();
}

RtResultVoid Transformer::add_ldvirtftn(const metadata::RtMethodInfo* method)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, obj_result, pop_eval_stack());
    if (obj_result->is_not_reference())
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    if (vm::Method::is_devirtualed(method))
    {
        return add_ldftn(method);
    }
    GeneralInst* ir = create_add_inst(OpCodeEnum::Ldvirtftn);
    ir->set_var_src(obj_result);
    ir->set_method(method);
    ir->set_var_dst(push_ref_or_ptr_to_eval_stack());
    RET_VOID_OK();
}

RtResultVoid Transformer::add_initobj(metadata::RtClass* klass)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, addr_result, pop_eval_stack());
    if (addr_result->is_not_reference())
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(ReduceTypeAndSize, type_and_size, InterpDefs::get_reduce_type_and_size_by_typesig(klass->by_val));

    GeneralInst* ir;
    switch (type_and_size.reduce_type)
    {
    case metadata::RtArgOrLocOrFieldReduceType::I1:
    case metadata::RtArgOrLocOrFieldReduceType::U1:
        ir = create_add_inst(OpCodeEnum::InitObjI1);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I2:
    case metadata::RtArgOrLocOrFieldReduceType::U2:
        ir = create_add_inst(OpCodeEnum::InitObjI2);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I4:
    case metadata::RtArgOrLocOrFieldReduceType::R4:
        ir = create_add_inst(OpCodeEnum::InitObjI4);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I8:
    case metadata::RtArgOrLocOrFieldReduceType::R8:
        ir = create_add_inst(OpCodeEnum::InitObjI8);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::Ref:
    case metadata::RtArgOrLocOrFieldReduceType::I:
        ir = create_add_inst(OpCodeEnum::InitObjRef);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::Other:
    {
        ir = create_add_inst(OpCodeEnum::InitObjAny);
        ir->set_size(type_and_size.byte_size);
        break;
    }
    default:
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    }
    ir->set_var_src(addr_result);
    RET_VOID_OK();
}

RtResultVoid Transformer::add_cpobj(metadata::RtClass* klass)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, src, pop_eval_stack());
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, dst, pop_eval_stack());
    if (src->is_not_reference() || dst->is_not_reference())
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(ReduceTypeAndSize, type_and_size, InterpDefs::get_reduce_type_and_size_by_typesig(klass->by_val));
    GeneralInst* ir;
    switch (type_and_size.reduce_type)
    {
    case metadata::RtArgOrLocOrFieldReduceType::I1:
    case metadata::RtArgOrLocOrFieldReduceType::U1:
        ir = create_add_inst(OpCodeEnum::CpObjI1);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I2:
    case metadata::RtArgOrLocOrFieldReduceType::U2:
        ir = create_add_inst(OpCodeEnum::CpObjI2);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I4:
    case metadata::RtArgOrLocOrFieldReduceType::R4:
        ir = create_add_inst(OpCodeEnum::CpObjI4);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I8:
    case metadata::RtArgOrLocOrFieldReduceType::R8:
        ir = create_add_inst(OpCodeEnum::CpObjI8);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::Ref:
    case metadata::RtArgOrLocOrFieldReduceType::I:
        ir = create_add_inst(OpCodeEnum::CpObjRef);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::Other:
    {
        ir = create_add_inst(OpCodeEnum::CpObjAny);
        ir->set_size(type_and_size.byte_size);
        break;
    }
    default:
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    }
    ir->set_var_src(src);
    ir->set_var_dst(dst);
    RET_VOID_OK();
}

RtResultVoid Transformer::add_ldobj(metadata::RtClass* klass)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, addr, pop_eval_stack());
    if (addr->is_not_reference())
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(ReduceTypeAndSize, type_and_size, InterpDefs::get_reduce_type_and_size_by_typesig(klass->by_val));
    GeneralInst* ir;
    switch (type_and_size.reduce_type)
    {
    case metadata::RtArgOrLocOrFieldReduceType::I1:
        ir = create_add_inst(OpCodeEnum::LdIndI1);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::U1:
        ir = create_add_inst(OpCodeEnum::LdIndU1);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I2:
        ir = create_add_inst(OpCodeEnum::LdIndI2);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::U2:
        ir = create_add_inst(OpCodeEnum::LdIndU2);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I4:
    case metadata::RtArgOrLocOrFieldReduceType::R4:
        ir = create_add_inst(OpCodeEnum::LdIndI4);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I8:
    case metadata::RtArgOrLocOrFieldReduceType::R8:
        ir = create_add_inst(OpCodeEnum::LdIndI8);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::Ref:
    case metadata::RtArgOrLocOrFieldReduceType::I:
        ir = create_add_inst(OpCodeEnum::LdIndRef);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::Other:
    {
        ir = create_add_inst(OpCodeEnum::LdObjAny);
        ir->set_size(type_and_size.byte_size);
        break;
    }
    default:
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    }
    ir->set_prefix(_prefix);
    ir->set_var_src(addr);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(Variable*, dst, push_typesig_to_eval_stack(klass->by_val));
    ir->set_var_dst(dst);
    RET_VOID_OK();
}

RtResultVoid Transformer::add_stobj(metadata::RtClass* klass)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, val, pop_eval_stack());
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, addr, pop_eval_stack());
    if (addr->is_not_reference())
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(ReduceTypeAndSize, type_and_size, InterpDefs::get_reduce_type_and_size_by_typesig(klass->by_val));
    GeneralInst* ir;
    switch (type_and_size.reduce_type)
    {
    case metadata::RtArgOrLocOrFieldReduceType::I1:
    case metadata::RtArgOrLocOrFieldReduceType::U1:
        ir = create_add_inst(OpCodeEnum::StIndI1);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I2:
    case metadata::RtArgOrLocOrFieldReduceType::U2:
        ir = create_add_inst(OpCodeEnum::StIndI2);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I4:
    case metadata::RtArgOrLocOrFieldReduceType::R4:
        ir = create_add_inst(OpCodeEnum::StIndI4);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I8:
    case metadata::RtArgOrLocOrFieldReduceType::R8:
        ir = create_add_inst(OpCodeEnum::StIndI8);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::Ref:
    case metadata::RtArgOrLocOrFieldReduceType::I:
        ir = create_add_inst(OpCodeEnum::StIndRef);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::Other:
    {
        ir = create_add_inst(OpCodeEnum::StObjAny);
        ir->set_size(type_and_size.byte_size);
        break;
    }
    default:
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    }
    ir->set_prefix(_prefix);
    ir->set_var_src(val);
    ir->set_var_dst(addr);
    RET_VOID_OK();
}

RtResultVoid Transformer::add_ldstr(uint32_t user_string_index)
{
    // Create instruction to load string
    GeneralInst* ir = create_add_inst(OpCodeEnum::LdStr);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, str, get_module()->get_user_string(user_string_index));

    ir->set_user_string(str);

    // Push string reference to eval stack
    ir->set_var_dst(push_ref_or_ptr_to_eval_stack());
    RET_VOID_OK();
}

RtResultVoid Transformer::add_castclass(metadata::RtClass* klass)
{
    if (vm::Class::is_nullable_type(klass))
    {
        klass = vm::Class::get_nullable_underlying_class(klass);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, obj, pop_eval_stack());

    GeneralInst* ir = create_add_inst(OpCodeEnum::CastClass);

    ir->set_class(klass);
    ir->set_var_src(obj);
    ir->set_var_dst(push_ref_or_ptr_to_eval_stack());
    RET_VOID_OK();
}

RtResultVoid Transformer::add_isinst(metadata::RtClass* klass)
{
    if (vm::Class::is_nullable_type(klass))
    {
        klass = vm::Class::get_nullable_underlying_class(klass);
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, obj, get_top_var());
    if (obj->is_not_reference())
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    GeneralInst* ir = create_add_inst(OpCodeEnum::IsInst);

    ir->set_class(klass);
    ir->set_var_src(obj);
    ir->set_var_dst(obj);
    RET_VOID_OK();
}

RtResultVoid Transformer::add_box(metadata::RtClass* klass)
{
    if (!vm::Class::is_value_type(klass))
    {
        // return add_isinst(klass);
        // do nothing for boxing of reference types, just push the reference back to eval stack
        RET_ERR_ON_FAIL(pop_eval_stack());
        RET_ERR_ON_FAIL(push_typesig_to_eval_stack(klass->by_val));
        RET_VOID_OK();
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, val, pop_eval_stack());

    GeneralInst* ir = create_add_inst(OpCodeEnum::Box);

    ir->set_class(klass);
    ir->set_var_src(val);
    ir->set_var_dst(push_ref_or_ptr_to_eval_stack());
    RET_VOID_OK();
}

RtResultVoid Transformer::add_unbox(metadata::RtClass* klass)
{
    if (vm::Class::is_nullable_type(klass))
    {
        klass = vm::Class::get_nullable_underlying_class(klass);
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, obj, pop_eval_stack());
    if (!vm::Class::is_value_type(klass))
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    GeneralInst* ir = create_add_inst(OpCodeEnum::Unbox);

    ir->set_class(klass);
    ir->set_var_src(obj);
    ir->set_var_dst(push_ref_or_ptr_to_eval_stack());
    RET_VOID_OK();
}

RtResultVoid Transformer::add_unbox_any(metadata::RtClass* klass)
{
    if (!vm::Class::is_value_type(klass))
    {
        return add_castclass(klass);
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, obj, pop_eval_stack());
    if (obj->is_not_reference())
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    GeneralInst* ir = create_add_inst(OpCodeEnum::UnboxAny);

    ir->set_class(klass);
    ir->set_var_src(obj);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(Variable*, dst, push_class_to_eval_stack(klass));
    ir->set_var_dst(dst);
    RET_VOID_OK();
}

RtResultVoid Transformer::add_ldfld(const metadata::RtFieldInfo* field)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, obj, pop_eval_stack()); // Pop object reference
    OpCodeEnum opcode;
    switch (obj->data_type)
    {
    case RtEvalStackDataType::RefOrPtr:
        opcode = OpCodeEnum::Ldfld;
        break;
    case RtEvalStackDataType::Other:
        opcode = OpCodeEnum::Ldvfld;
        break;
    default:
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    }

    GeneralInst* ir = create_add_inst(opcode);
    ir->set_var_src(obj);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, dst_var, push_typesig_to_eval_stack(field->type_sig));
    ir->set_var_dst(dst_var);
    ir->set_field(field);
    ir->set_prefix(_prefix);
    RET_VOID_OK();
}

RtResultVoid Transformer::add_ldflda(const metadata::RtFieldInfo* field)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, obj, pop_eval_stack());
    if (obj->is_not_reference())
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    GeneralInst* ir = create_add_inst(OpCodeEnum::Ldflda);

    ir->set_field(field);
    ir->set_var_src(obj);
    ir->set_var_dst(push_ref_or_ptr_to_eval_stack());
    RET_VOID_OK();
}

RtResultVoid Transformer::add_stfld(const metadata::RtFieldInfo* field)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, value, pop_eval_stack());
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, obj, pop_eval_stack());
    if (obj->is_not_reference())
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    GeneralInst* ir = create_add_inst(OpCodeEnum::Stfld);

    ir->set_field(field);
    ir->set_var_arg1(obj);
    ir->set_var_arg2(value);
    ir->set_prefix(_prefix);
    RET_VOID_OK();
}

RtResultVoid Transformer::add_ldsfld(const metadata::RtFieldInfo* field)
{
    GeneralInst* ir = create_add_inst(OpCodeEnum::Ldsfld);

    ir->set_field(field);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(Variable*, dst_var, push_typesig_to_eval_stack(field->type_sig));
    ir->set_var_dst(dst_var);
    RET_VOID_OK();
}

RtResultVoid Transformer::add_ldsflda(const metadata::RtFieldInfo* field)
{
    GeneralInst* ir = create_add_inst(OpCodeEnum::Ldsflda);
    ir->set_field(field);
    ir->set_var_dst(push_ref_or_ptr_to_eval_stack());
    RET_VOID_OK();
}

RtResultVoid Transformer::add_stsfld(const metadata::RtFieldInfo* field)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, value, pop_eval_stack());
    GeneralInst* ir = create_add_inst(OpCodeEnum::Stsfld);

    ir->set_field(field);
    ir->set_var_src(value);
    RET_VOID_OK();
}

RtResultVoid Transformer::add_newarr(metadata::RtClass* ele_klass)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, count, pop_eval_stack());
    if (!count->is_i32_or_native_int())
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    GeneralInst* ir = create_add_inst(OpCodeEnum::NewArr);

    ir->set_class(ele_klass);
    ir->set_var_src(count);
    ir->set_var_dst(push_ref_or_ptr_to_eval_stack());
    RET_VOID_OK();
}

RtResultVoid Transformer::add_ldlen()
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, arr, pop_eval_stack());

    GeneralInst* ir = create_add_inst(OpCodeEnum::LdLen);

    ir->set_var_src(arr);
    // in ECMA 335, ldlen always return native int, but in our runtime, we use i4 to represent array length
    ir->set_var_dst(push_i4_to_eval_stack());
    RET_VOID_OK();
}

RtResultVoid Transformer::add_ldelema(metadata::RtClass* ele_klass)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, idx, pop_eval_stack());
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, arr, pop_eval_stack());
    if (!idx->is_i32_or_native_int())
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    if (arr->is_not_reference())
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    GeneralInst* ir = create_add_inst(OpCodeEnum::Ldelema);

    ir->set_class(ele_klass);
    ir->set_var_arg1(arr);
    ir->set_var_arg2(idx);
    ir->set_var_dst(push_ref_or_ptr_to_eval_stack());
    ir->set_prefix(_prefix);
    RET_VOID_OK();
}

RtResultVoid Transformer::add_ldelem(OpCodeEnum opcode, RtEvalStackDataType data_type)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, idx, pop_eval_stack());
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, arr, pop_eval_stack());
    if (!idx->is_i32_or_native_int())
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    if (arr->is_not_reference())
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    GeneralInst* ir = create_add_inst(opcode);

    ir->set_var_arg1(arr);
    ir->set_var_arg2(idx);
    ir->set_var_dst(push_primitive_to_eval_stack(data_type));
    RET_VOID_OK();
}

RtResultVoid Transformer::add_ldelem_any(metadata::RtClass* ele_klass)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(ReduceTypeAndSize, type_and_size, InterpDefs::get_reduce_type_and_size_by_typesig(ele_klass->by_val));
    switch (type_and_size.reduce_type)
    {
    case metadata::RtArgOrLocOrFieldReduceType::I1:
    case metadata::RtArgOrLocOrFieldReduceType::U1:
        return add_ldelem(OpCodeEnum::LdelemI1, RtEvalStackDataType::I4);
    case metadata::RtArgOrLocOrFieldReduceType::I2:
    case metadata::RtArgOrLocOrFieldReduceType::U2:
        return add_ldelem(OpCodeEnum::LdelemI2, RtEvalStackDataType::I4);
    case metadata::RtArgOrLocOrFieldReduceType::I4:
        return add_ldelem(OpCodeEnum::LdelemI4, RtEvalStackDataType::I4);
    case metadata::RtArgOrLocOrFieldReduceType::I8:
        return add_ldelem(OpCodeEnum::LdelemI8, RtEvalStackDataType::I8);
    case metadata::RtArgOrLocOrFieldReduceType::R4:
        return add_ldelem(OpCodeEnum::LdelemR4, RtEvalStackDataType::R4);
    case metadata::RtArgOrLocOrFieldReduceType::R8:
        return add_ldelem(OpCodeEnum::LdelemR8, RtEvalStackDataType::R8);
    case metadata::RtArgOrLocOrFieldReduceType::Ref:
        return add_ldelem(OpCodeEnum::LdelemRef, RtEvalStackDataType::RefOrPtr);
    case metadata::RtArgOrLocOrFieldReduceType::I:
        return add_ldelem(OpCodeEnum::LdelemI, RtEvalStackDataType::RefOrPtr);
    case metadata::RtArgOrLocOrFieldReduceType::Other:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, index, pop_eval_stack());
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, arr, pop_eval_stack());
        if (!index->is_i32_or_native_int())
            RET_ASSERT_ERR(RtErr::ExecutionEngine);
        if (arr->is_not_reference())
            RET_ASSERT_ERR(RtErr::ExecutionEngine);
        GeneralInst* ir = create_add_inst(OpCodeEnum::LdelemAny);
        ir->set_class(ele_klass);
        ir->set_var_arg1(arr);
        ir->set_var_arg2(index);
        // ir->set_size(type_and_size.byte_size);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(Variable*, dst, push_class_to_eval_stack(ele_klass));
        ir->set_var_dst(dst);
        RET_VOID_OK();
    }
    default:
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    }
}

RtResultVoid Transformer::add_stelem(OpCodeEnum opcode, RtEvalStackDataType data_type)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, val, pop_eval_stack());
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, idx, pop_eval_stack());
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, arr, pop_eval_stack());
    if (!idx->is_i32_or_native_int())
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    if (arr->is_not_reference())
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    if (val->data_type != data_type)
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    GeneralInst* ir = create_add_inst(opcode);

    ir->set_var_arg1(arr);
    ir->set_var_arg2(idx);
    ir->set_var_arg3(val);
    RET_VOID_OK();
}

RtResultVoid Transformer::add_stelem_any(metadata::RtClass* ele_klass)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(ReduceTypeAndSize, type_and_size, InterpDefs::get_reduce_type_and_size_by_typesig(ele_klass->by_val));
    switch (type_and_size.reduce_type)
    {
    case metadata::RtArgOrLocOrFieldReduceType::I1:
    case metadata::RtArgOrLocOrFieldReduceType::U1:
        return add_stelem(OpCodeEnum::StelemI1, RtEvalStackDataType::I4);
    case metadata::RtArgOrLocOrFieldReduceType::I2:
    case metadata::RtArgOrLocOrFieldReduceType::U2:
        return add_stelem(OpCodeEnum::StelemI2, RtEvalStackDataType::I4);
    case metadata::RtArgOrLocOrFieldReduceType::I4:
        return add_stelem(OpCodeEnum::StelemI4, RtEvalStackDataType::I4);
    case metadata::RtArgOrLocOrFieldReduceType::I8:
        return add_stelem(OpCodeEnum::StelemI8, RtEvalStackDataType::I8);
    case metadata::RtArgOrLocOrFieldReduceType::R4:
        return add_stelem(OpCodeEnum::StelemR4, RtEvalStackDataType::R4);
    case metadata::RtArgOrLocOrFieldReduceType::R8:
        return add_stelem(OpCodeEnum::StelemR8, RtEvalStackDataType::R8);
    case metadata::RtArgOrLocOrFieldReduceType::Ref:
        return add_stelem(OpCodeEnum::StelemRef, RtEvalStackDataType::RefOrPtr);
    case metadata::RtArgOrLocOrFieldReduceType::I:
        return add_stelem(OpCodeEnum::StelemI, RtEvalStackDataType::RefOrPtr);
    case metadata::RtArgOrLocOrFieldReduceType::Other:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, val, pop_eval_stack());
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, idx, pop_eval_stack());
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, arr, pop_eval_stack());
        if (!idx->is_i32_or_native_int())
            RET_ASSERT_ERR(RtErr::ExecutionEngine);
        if (arr->is_not_reference())
            RET_ASSERT_ERR(RtErr::ExecutionEngine);
        if (val->data_type != RtEvalStackDataType::Other)
            RET_ASSERT_ERR(RtErr::ExecutionEngine);
        GeneralInst* ir = create_add_inst(OpCodeEnum::StelemAny);

        ir->set_class(ele_klass);
        ir->set_var_arg1(arr);
        ir->set_var_arg2(idx);
        ir->set_var_arg3(val);
        // ir->set_size(type_and_size.byte_size);
        RET_VOID_OK();
        break;
    }
    default:
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    }
}

RtResultVoid Transformer::add_mkrefany(metadata::RtClass* klass)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, addr, pop_eval_stack());
    if (addr->is_not_reference())
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    // Create mkrefany instruction - this creates a TypedRef from address and type
    GeneralInst* ir = create_add_inst(OpCodeEnum::MkRefAny);

    ir->set_class(klass);
    ir->set_var_src(addr);
    // Push TypedRef value
    ir->set_var_dst(push_ref_or_ptr_to_eval_stack());
    RET_VOID_OK();
}

RtResultVoid Transformer::add_refanytype()
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, typedref_var, pop_eval_stack());
    if (!typedref_var->is_typedbyref())
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    // Create refanytype instruction - extracts type from TypedRef
    GeneralInst* ir = create_add_inst(OpCodeEnum::RefAnyType);

    ir->set_var_src(typedref_var);
    ir->set_var_dst(push_ref_or_ptr_to_eval_stack());
    RET_VOID_OK();
}

RtResultVoid Transformer::add_refanyval(metadata::RtClass* klass)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, typedref_var, pop_eval_stack());
    if (!typedref_var->is_typedbyref())
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    // Create refanyval instruction - extracts address from TypedRef
    GeneralInst* ir = create_add_inst(OpCodeEnum::RefAnyVal);

    ir->set_class(klass);
    ir->set_var_src(typedref_var);
    ir->set_var_dst(push_ref_or_ptr_to_eval_stack());
    RET_VOID_OK();
}

RtResultVoid Transformer::add_ldtoken(uint32_t token)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtRuntimeHandle, handle, get_cached_runtime_handle(token));

    GeneralInst* ir = create_add_inst(OpCodeEnum::LdToken);

    ir->set_runtime_handle(metadata::RtEncodedRuntimeHandle::encode(handle));
    ir->set_var_dst(push_ref_or_ptr_to_eval_stack());
    RET_VOID_OK();
}

RtResultVoid Transformer::add_throw()
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, exe, pop_eval_stack());

    GeneralInst* ir = create_add_inst(OpCodeEnum::Throw);

    ir->set_var_src(exe);
    clear_eval_stack();
    RET_VOID_OK();
}

RtResultVoid Transformer::add_rethrow()
{
    clear_eval_stack();
    create_add_inst(OpCodeEnum::Rethrow);
    RET_VOID_OK();
}

RtResultVoid Transformer::add_endfinally()
{
    clear_eval_stack();
    create_add_inst(OpCodeEnum::EndFinallyOrFault);
    RET_VOID_OK();
}

RtResultVoid Transformer::add_endfilter()
{
    if (_cur_bb->eval_stack.size() != 1)
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    GeneralInst* ir = create_add_inst(OpCodeEnum::EndFilter);
    ir->set_var_src(_cur_bb->eval_stack[0]);
    clear_eval_stack();
    RET_VOID_OK();
}

RtResultVoid Transformer::add_leave(size_t next_offset, ptrdiff_t target_offset)
{
    const size_t target_il_offset = static_cast<size_t>(static_cast<ptrdiff_t>(next_offset) + target_offset);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(BasicBlock*, target_bb, get_branch_target_bb(target_il_offset));

    GeneralInst* ir = create_add_inst(OpCodeEnum::Leave);
    clear_eval_stack();
    ir->set_branch_target(target_bb);
    RET_ERR_ON_FAIL(setup_target_branch_eval_stack(target_bb));
    RET_VOID_OK();
}

RtResultVoid Transformer::add_arglist()
{
    GeneralInst* ir = create_add_inst(OpCodeEnum::Arglist);
    ir->set_var_dst(push_ref_or_ptr_to_eval_stack());
    RET_VOID_OK();
}

RtResultVoid Transformer::add_localloc()
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, size, pop_eval_stack());
    if (!size->is_i32_or_native_int())
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    GeneralInst* ir = create_add_inst(OpCodeEnum::LocAlloc);
    ir->set_var_src(size);
    ir->set_var_dst(push_ref_or_ptr_to_eval_stack());
    RET_VOID_OK();
}

RtResultVoid Transformer::add_cpblk()
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, size, pop_eval_stack());
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, src_addr, pop_eval_stack());
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, dst_addr, pop_eval_stack());

    if (size->is_not_i32_and_native_int() || src_addr->is_not_reference() || dst_addr->is_not_reference())
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    GeneralInst* ir = create_add_inst(OpCodeEnum::CpBlk);
    ir->set_var_arg1(dst_addr);
    ir->set_var_arg2(src_addr);
    ir->set_var_arg3(size);
    RET_VOID_OK();
}

RtResultVoid Transformer::add_initblk()
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, size, pop_eval_stack());
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, value, pop_eval_stack());
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, dst_addr, pop_eval_stack());
    if (size->is_not_i32_and_native_int() || value->is_not_i32_and_native_int() || dst_addr->is_not_reference())
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    GeneralInst* ir = create_add_inst(OpCodeEnum::InitBlk);
    ir->set_var_arg1(dst_addr);
    ir->set_var_arg2(value);
    ir->set_var_arg3(size);
    RET_VOID_OK();
}

RtResultVoid Transformer::add_sizeof(const metadata::RtTypeSig* type_sig)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(ReduceTypeAndSize, type_and_size, InterpDefs::get_reduce_type_and_size_by_typesig(type_sig));
    return add_ldci4(static_cast<int32_t>(type_and_size.byte_size));
}

RtResultVoid Transformer::transform()
{
    // Setup arguments and locals
    RET_ERR_ON_FAIL(setup_args());
    RET_ERR_ON_FAIL(setup_locals());

    // Initialize stack frame offset
    _eval_stack_base_offset = _total_arg_and_local_stack_object_size;
    _max_eval_stack_size = _total_arg_and_local_stack_object_size;

    // Setup basic blocks from IL code
    RET_ERR_ON_FAIL(setup_basic_blocks());
    RET_ERR_ON_FAIL(setup_exception_clauses());

    // Add initialization code for locals if needed
    RET_ERR_ON_FAIL(add_init_locals());

    // Transform IL body to HL opcodes
    RET_ERR_ON_FAIL(transform_body());

    RET_VOID_OK();
}

RtResultVoid Transformer::setup_args()
{
    if (!vm::Method::is_void_return(_method))
    {
        auto ret_var_result = alloc_variable_by_typesig(_method->return_type, INVALID_EVAL_STACK_OFFSET);
        RET_ERR_ON_FAIL(ret_var_result);
        _ret_var = ret_var_result.unwrap();
        assert(_ret_var->stack_object_size == _method->ret_stack_object_size);
    }

    const bool is_instance_method = vm::Method::is_instance(_method);
    const size_t actual_arg_count = _method->parameter_count + (is_instance_method ? 1 : 0);

    size_t total_arg_stack_object_size = 0;
    _arg_vars = _pool->calloc_any<const Variable*>(actual_arg_count);
    _arg_vars_count = actual_arg_count;

    if (is_instance_method)
    {
        const metadata::RtTypeSig* this_type_sig = vm::Class::is_value_type(_method->parent) ? _method->parent->by_ref : _method->parent->by_val;
        auto arg_result = alloc_variable_by_typesig(this_type_sig, total_arg_stack_object_size);
        RET_ERR_ON_FAIL(arg_result);
        const Variable* arg_var = arg_result.unwrap();
        assert(arg_var->stack_object_size == 1);
        _arg_vars[0] = arg_var;
        total_arg_stack_object_size += 1;
    }

    const size_t arg_base_idx = is_instance_method ? 1 : 0;
    for (size_t i = 0; i < _method->parameter_count; ++i)
    {
        const metadata::RtTypeSig* param_type = _method->parameters[i];
        auto arg_result = alloc_variable_by_typesig(param_type, total_arg_stack_object_size);
        RET_ERR_ON_FAIL(arg_result);
        const Variable* arg_var = arg_result.unwrap();
        total_arg_stack_object_size += arg_var->stack_object_size;
        _arg_vars[arg_base_idx + i] = arg_var;
    }

    _total_arg_stack_object_size = total_arg_stack_object_size;
    assert(_method->total_arg_stack_object_size == total_arg_stack_object_size);
    RET_VOID_OK();
}

RtResultVoid Transformer::setup_locals()
{
    if (_method_body->local_var_sig_token == 0)
    {
        _total_arg_and_local_stack_object_size = _total_arg_stack_object_size;
        _local_vars = nullptr;
        _local_vars_count = 0;
        RET_VOID_OK();
    }

    metadata::RtModuleDef* mod = get_module();

    _total_arg_and_local_stack_object_size = _total_arg_stack_object_size;
    utils::Vector<const metadata::RtTypeSig*> local_var_type_sigs;
    RET_ERR_ON_FAIL(mod->read_local_var_sig(_method_body->local_var_sig_token, _generic_container_context, _generic_context, local_var_type_sigs));
    if (local_var_type_sigs.empty())
    {
        _local_vars = nullptr;
        _local_vars_count = 0;
        RET_VOID_OK();
    }

    _local_vars = _pool->calloc_any<const Variable*>(local_var_type_sigs.size());
    _local_vars_count = local_var_type_sigs.size();
    for (size_t i = 0; i < local_var_type_sigs.size(); ++i)
    {
        const metadata::RtTypeSig* type_sig = local_var_type_sigs[i];
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const Variable*, local_var, alloc_variable_by_typesig(type_sig, _total_arg_and_local_stack_object_size));
        _local_vars[i] = local_var;
        _total_arg_and_local_stack_object_size += local_var->stack_object_size;
    }

    RET_VOID_OK();
}

RtResultVoid Transformer::setup_exception_clauses()
{
    const auto& clauses = _method_body->exception_clauses;

    for (size_t i = 0; i < clauses.size(); ++i)
    {
        const auto& src = clauses[i];

        // try block must start at a known basic block
        if (_il_offset_to_basic_block.find(src.try_offset) == _il_offset_to_basic_block.end())
            RET_ASSERT_ERR(RtErr::ExecutionEngine);

        // handler block must exist and start exactly at handler_offset
        auto handler_it = _il_offset_to_basic_block.find(src.handler_offset);
        if (handler_it == _il_offset_to_basic_block.end() || handler_it->second->il_begin_offset != src.handler_offset)
            RET_ASSERT_ERR(RtErr::ExecutionEngine);

        BasicBlock* handler_bb = handler_it->second;
        if (src.flags == metadata::RtILExceptionClauseType::Exception || src.flags == metadata::RtILExceptionClauseType::Filter)
        {
            const Variable* exc_var = alloc_exception_variable(handler_bb->in_eval_stack_top);
            handler_bb->push_in_val_stack(exc_var);
        }

        if (src.flags == metadata::RtILExceptionClauseType::Filter)
        {
            const uint32_t filter_begin_offset = src.class_token_or_filter_offset;
            auto filter_it = _il_offset_to_basic_block.find(filter_begin_offset);
            if (filter_it == _il_offset_to_basic_block.end() || filter_it->second->il_begin_offset != filter_begin_offset)
                RET_ASSERT_ERR(RtErr::ExecutionEngine);

            BasicBlock* filter_bb = filter_it->second;
            const Variable* exc_var = alloc_exception_variable(filter_bb->in_eval_stack_top);
            filter_bb->push_in_val_stack(exc_var);

            if (!(filter_begin_offset >= src.try_offset + src.try_length && filter_begin_offset < src.handler_offset))
                RET_ASSERT_ERR(RtErr::ExecutionEngine);
        }

        if (i > 0)
        {
            const auto& last = clauses[i - 1];
            if (!(src.try_offset >= last.try_offset + last.try_length ||
                  (src.try_offset <= last.try_offset && src.try_offset + src.try_length >= last.try_offset + last.try_length) ||
                  (src.handler_offset <= last.try_offset && src.handler_offset + src.handler_length >= last.try_offset + last.try_length)))
            {
                RET_ASSERT_ERR(RtErr::ExecutionEngine);
            }

            if (!(src.handler_offset >= last.handler_offset + last.handler_length ||
                  (src.handler_offset <= last.handler_offset && src.handler_offset + src.handler_length >= last.handler_offset + last.handler_length)))
            {
                RET_ASSERT_ERR(RtErr::ExecutionEngine);
            }
        }
    }

    RET_VOID_OK();
}

RtResultVoid Transformer::setup_basic_blocks()
{
    BasicBlockSplitter splitter(_method_body, _pool);
    RET_ERR_ON_FAIL(splitter.split());

    const auto& split_offsets = splitter.get_split_offsets();
    if (split_offsets.empty())
        RET_ASSERT_ERR(RtErr::ExecutionEngine);

    utils::NotFreeList<uint32_t> offsets_vec(_pool);
    offsets_vec.push_range(split_offsets.begin(), split_offsets.end());
    std::sort(offsets_vec.begin(), offsets_vec.end());

    size_t start_index = (offsets_vec[0] == 0) ? 1 : 0;
    const auto& offsets_slice = offsets_vec.data() + start_index;
    const size_t bb_count = offsets_vec.size() - start_index;

    _basic_blocks = _pool->calloc_any<BasicBlock>(bb_count);
    _basic_block_count = bb_count;

    size_t last_split_offset = 0;
    for (size_t i = 0; i < bb_count; ++i)
    {
        const size_t split_offset = offsets_slice[i];
        BasicBlock* cur_bb = _basic_blocks + i;
        new (cur_bb) BasicBlock(_pool); // Placement new to call constructor
        _il_offset_to_basic_block[static_cast<uint32_t>(last_split_offset)] = cur_bb;

        cur_bb->visited = false;
        cur_bb->inited_eval_stack = false;
        cur_bb->il_begin_offset = last_split_offset;
        cur_bb->il_end_offset = split_offset;
        cur_bb->eval_stack_top = 0;
        cur_bb->in_eval_stack_top = _eval_stack_base_offset;
        cur_bb->insts.reserve(split_offset - last_split_offset);
        cur_bb->next_bb = (i + 1 < bb_count) ? &_basic_blocks[i + 1] : nullptr;
        last_split_offset = split_offset;
    }

    _cur_bb = &_basic_blocks[0];
    RET_VOID_OK();
}

RtResultVoid Transformer::transform_body()
{
    const uint8_t* codes_begin = _method_body->code;
    for (;;)
    {
        BasicBlock* bb = _cur_bb;
        assert(!bb->visited);
        bb->visited = true;
        assert(bb->eval_stack.empty());
        assert(bb->eval_stack_top == 0);
        bb->eval_stack_top = bb->in_eval_stack_top;
        if (!bb->in_eval_stack.empty())
        {
            bb->eval_stack.push_range(bb->in_eval_stack.data(), bb->in_eval_stack.size());
        }
        bb->inited_eval_stack = true;

        for (size_t il_offset_cur = bb->il_begin_offset, il_offset_end = bb->il_end_offset; il_offset_cur < il_offset_end;)
        {
            il::OpCodeValue opcode = *(const il::OpCodeValue*)(codes_begin + il_offset_cur);
            _cur_il_offset = static_cast<int32_t>(il_offset_cur);
            if (!_not_retset_prefix_after_cur_il)
            {
                clear_prefix();
            }
            else
            {
                _not_retset_prefix_after_cur_il = false;
            }
            switch (opcode)
            {
            case il::OpCodeValue::Nop:
            case il::OpCodeValue::Break:
            {
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdArg0:
            {
                RET_ERR_ON_FAIL(add_ldarg(0));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdArg1:
            {
                RET_ERR_ON_FAIL(add_ldarg(1));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdArg2:
            {
                RET_ERR_ON_FAIL(add_ldarg(2));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdArg3:
            {
                RET_ERR_ON_FAIL(add_ldarg(3));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdLoc0:
            {
                RET_ERR_ON_FAIL(add_ldloc(0));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdLoc1:
            {
                RET_ERR_ON_FAIL(add_ldloc(1));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdLoc2:
            {
                RET_ERR_ON_FAIL(add_ldloc(2));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdLoc3:
            {
                RET_ERR_ON_FAIL(add_ldloc(3));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::StLoc0:
            {
                RET_ERR_ON_FAIL(add_stloc(0));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::StLoc1:
            {
                RET_ERR_ON_FAIL(add_stloc(1));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::StLoc2:
            {
                RET_ERR_ON_FAIL(add_stloc(2));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::StLoc3:
            {
                RET_ERR_ON_FAIL(add_stloc(3));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdArgS:
            {
                uint8_t arg_idx = *(codes_begin + il_offset_cur + 1);
                RET_ERR_ON_FAIL(add_ldarg(arg_idx));
                il_offset_cur += 2;
                break;
            }
            case il::OpCodeValue::LdArgaS:
            {
                uint8_t arg_idx = *(codes_begin + il_offset_cur + 1);
                RET_ERR_ON_FAIL(add_ldarga(arg_idx));
                il_offset_cur += 2;
                break;
            }
            case il::OpCodeValue::StArgS:
            {
                uint8_t arg_idx = *(codes_begin + il_offset_cur + 1);
                RET_ERR_ON_FAIL(add_starg(arg_idx));
                il_offset_cur += 2;
                break;
            }
            case il::OpCodeValue::LdLocS:
            {
                uint8_t local_idx = *(codes_begin + il_offset_cur + 1);
                RET_ERR_ON_FAIL(add_ldloc(local_idx));
                il_offset_cur += 2;
                break;
            }
            case il::OpCodeValue::LdLocaS:
            {
                uint8_t local_idx = *(codes_begin + il_offset_cur + 1);
                RET_ERR_ON_FAIL(add_ldloca(local_idx));
                il_offset_cur += 2;
                break;
            }
            case il::OpCodeValue::StLocS:
            {
                uint8_t local_idx = *(codes_begin + il_offset_cur + 1);
                RET_ERR_ON_FAIL(add_stloc(local_idx));
                il_offset_cur += 2;
                break;
            }
            case il::OpCodeValue::LdNull:
            {
                RET_ERR_ON_FAIL(add_ldnull());
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdcI4M1:
            {
                RET_ERR_ON_FAIL(add_ldci4(-1));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdcI40:
            {
                RET_ERR_ON_FAIL(add_ldci4(0));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdcI41:
            {
                RET_ERR_ON_FAIL(add_ldci4(1));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdcI42:
            {
                RET_ERR_ON_FAIL(add_ldci4(2));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdcI43:
            {
                RET_ERR_ON_FAIL(add_ldci4(3));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdcI44:
            {
                RET_ERR_ON_FAIL(add_ldci4(4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdcI45:
            {
                RET_ERR_ON_FAIL(add_ldci4(5));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdcI46:
            {
                RET_ERR_ON_FAIL(add_ldci4(6));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdcI47:
            {
                RET_ERR_ON_FAIL(add_ldci4(7));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdcI48:
            {
                RET_ERR_ON_FAIL(add_ldci4(8));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdcI4S:
            {
                int32_t value = *(const int8_t*)(codes_begin + il_offset_cur + 1);
                RET_ERR_ON_FAIL(add_ldci4(value));
                il_offset_cur += 2;
                break;
            }
            case il::OpCodeValue::LdcI4:
            {
                int32_t value = (int32_t)utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                RET_ERR_ON_FAIL(add_ldci4(value));
                il_offset_cur += 5;
                break;
            }
            case il::OpCodeValue::LdcI8:
            {
                int64_t value = static_cast<int64_t>(utils::MemOp::read_u64_may_unaligned(codes_begin + il_offset_cur + 1));
                RET_ERR_ON_FAIL(add_ldci8(value));
                il_offset_cur += 9;
                break;
            }
            case il::OpCodeValue::LdcR4:
            {
                float value = static_cast<float>(utils::MemOp::read_f32_may_unaligned(codes_begin + il_offset_cur + 1));
                RET_ERR_ON_FAIL(add_ldcr4(value));
                il_offset_cur += 5;
                break;
            }
            case il::OpCodeValue::LdcR8:
            {
                double value = static_cast<double>(utils::MemOp::read_f64_may_unaligned(codes_begin + il_offset_cur + 1));
                RET_ERR_ON_FAIL(add_ldcr8(value));
                il_offset_cur += 9;
                break;
            }
            case il::OpCodeValue::Unused99:
            {
                RET_ASSERT_ERR(RtErr::ExecutionEngine);
            }
            case il::OpCodeValue::Dup:
            {
                RET_ERR_ON_FAIL(add_dup());
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::Pop:
            {
                RET_ERR_ON_FAIL(pop_eval_stack());
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::Jmp:
            {
                RET_ERR(RtErr::NotSupported);
            }
            case il::OpCodeValue::Call:
            {
                uint32_t method_token = utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method, get_method_from_token(method_token));
                RET_ERR_ON_FAIL(add_call(method));
                il_offset_cur += 5;
                break;
            }
            case il::OpCodeValue::Calli:
            {
                uint32_t standalone_method_sig_token = utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                auto ret_standalone_method_sig = get_standalone_method_sig_from_token(standalone_method_sig_token);
                RET_ERR_ON_FAIL(ret_standalone_method_sig);
                RET_ERR_ON_FAIL(add_calli(ret_standalone_method_sig.unwrap()));
                il_offset_cur += 5;
                break;
            }
            case il::OpCodeValue::Ret:
            {
                RET_ERR_ON_FAIL(add_ret());
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::BrS:
            {
                int8_t target_byte = *(const int8_t*)(codes_begin + il_offset_cur + 1);
                il_offset_cur += 2;
                RET_ERR_ON_FAIL(add_br(static_cast<uint32_t>(il_offset_cur), target_byte));
                break;
            }
            case il::OpCodeValue::BrfalseS:
            {
                int8_t target_byte = *(const int8_t*)(codes_begin + il_offset_cur + 1);
                il_offset_cur += 2;
                RET_ERR_ON_FAIL(add_brtrue_or_false(static_cast<uint32_t>(il_offset_cur), target_byte, false));
                break;
            }
            case il::OpCodeValue::BrtrueS:
            {
                int8_t target_byte = *(const int8_t*)(codes_begin + il_offset_cur + 1);
                il_offset_cur += 2;
                RET_ERR_ON_FAIL(add_brtrue_or_false(static_cast<uint32_t>(il_offset_cur), target_byte, true));
                break;
            }
            case il::OpCodeValue::BeqS:
            {
                int8_t target_byte = *(const int8_t*)(codes_begin + il_offset_cur + 1);
                il_offset_cur += 2;
                RET_ERR_ON_FAIL(add_condition_branch(static_cast<uint32_t>(il_offset_cur), target_byte, hl::OpCodeEnum::Beq));
                break;
            }
            case il::OpCodeValue::BgeS:
            {
                int8_t target_byte = *(const int8_t*)(codes_begin + il_offset_cur + 1);
                il_offset_cur += 2;
                RET_ERR_ON_FAIL(add_condition_branch(static_cast<uint32_t>(il_offset_cur), target_byte, hl::OpCodeEnum::Bge));
                break;
            }
            case il::OpCodeValue::BgtS:
            {
                int8_t target_byte = *(const int8_t*)(codes_begin + il_offset_cur + 1);
                il_offset_cur += 2;
                RET_ERR_ON_FAIL(add_condition_branch(static_cast<uint32_t>(il_offset_cur), target_byte, hl::OpCodeEnum::Bgt));
                break;
            }
            case il::OpCodeValue::BleS:
            {
                int8_t target_byte = *(const int8_t*)(codes_begin + il_offset_cur + 1);
                il_offset_cur += 2;
                RET_ERR_ON_FAIL(add_condition_branch(static_cast<uint32_t>(il_offset_cur), target_byte, hl::OpCodeEnum::Ble));
                break;
            }
            case il::OpCodeValue::BltS:
            {
                int8_t target_byte = *(const int8_t*)(codes_begin + il_offset_cur + 1);
                il_offset_cur += 2;
                RET_ERR_ON_FAIL(add_condition_branch(static_cast<uint32_t>(il_offset_cur), target_byte, hl::OpCodeEnum::Blt));
                break;
            }
            case il::OpCodeValue::BneUnS:
            {
                int8_t target_byte = *(const int8_t*)(codes_begin + il_offset_cur + 1);
                il_offset_cur += 2;
                RET_ERR_ON_FAIL(add_condition_branch(static_cast<uint32_t>(il_offset_cur), target_byte, hl::OpCodeEnum::BneUn));
                break;
            }
            case il::OpCodeValue::BgeUnS:
            {
                int8_t target_byte = *(const int8_t*)(codes_begin + il_offset_cur + 1);
                il_offset_cur += 2;
                RET_ERR_ON_FAIL(add_condition_branch(static_cast<uint32_t>(il_offset_cur), target_byte, hl::OpCodeEnum::BgeUn));
                break;
            }
            case il::OpCodeValue::BgtUnS:
            {
                int8_t target_byte = *(const int8_t*)(codes_begin + il_offset_cur + 1);
                il_offset_cur += 2;
                RET_ERR_ON_FAIL(add_condition_branch(static_cast<uint32_t>(il_offset_cur), target_byte, hl::OpCodeEnum::BgtUn));
                break;
            }
            case il::OpCodeValue::BleUnS:
            {
                int8_t target_byte = *(const int8_t*)(codes_begin + il_offset_cur + 1);
                il_offset_cur += 2;
                RET_ERR_ON_FAIL(add_condition_branch(static_cast<uint32_t>(il_offset_cur), target_byte, hl::OpCodeEnum::BleUn));
                break;
            }
            case il::OpCodeValue::BltUnS:
            {
                int8_t target_byte = *(const int8_t*)(codes_begin + il_offset_cur + 1);
                il_offset_cur += 2;
                RET_ERR_ON_FAIL(add_condition_branch(static_cast<uint32_t>(il_offset_cur), target_byte, hl::OpCodeEnum::BltUn));
                break;
            }
            case il::OpCodeValue::Br:
            {
                int32_t target = (int32_t)utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                il_offset_cur += 5;
                RET_ERR_ON_FAIL(add_br(static_cast<uint32_t>(il_offset_cur), target));
                break;
            }
            case il::OpCodeValue::Brfalse:
            {
                int32_t target = (int32_t)utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                il_offset_cur += 5;
                RET_ERR_ON_FAIL(add_brtrue_or_false(static_cast<uint32_t>(il_offset_cur), target, false));
                break;
            }
            case il::OpCodeValue::Brtrue:
            {
                int32_t target = (int32_t)utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                il_offset_cur += 5;
                RET_ERR_ON_FAIL(add_brtrue_or_false(static_cast<uint32_t>(il_offset_cur), target, true));
                break;
            }
            case il::OpCodeValue::Beq:
            {
                int32_t target = (int32_t)utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                il_offset_cur += 5;
                RET_ERR_ON_FAIL(add_condition_branch(static_cast<uint32_t>(il_offset_cur), target, hl::OpCodeEnum::Beq));
                break;
            }
            case il::OpCodeValue::Bge:
            {
                int32_t target = (int32_t)utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                il_offset_cur += 5;
                RET_ERR_ON_FAIL(add_condition_branch(static_cast<uint32_t>(il_offset_cur), target, hl::OpCodeEnum::Bge));
                break;
            }
            case il::OpCodeValue::Bgt:
            {
                int32_t target = (int32_t)utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                il_offset_cur += 5;
                RET_ERR_ON_FAIL(add_condition_branch(static_cast<uint32_t>(il_offset_cur), target, hl::OpCodeEnum::Bgt));
                break;
            }
            case il::OpCodeValue::Ble:
            {
                int32_t target = (int32_t)utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                il_offset_cur += 5;
                RET_ERR_ON_FAIL(add_condition_branch(static_cast<uint32_t>(il_offset_cur), target, hl::OpCodeEnum::Ble));
                break;
            }
            case il::OpCodeValue::Blt:
            {
                int32_t target = (int32_t)utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                il_offset_cur += 5;
                RET_ERR_ON_FAIL(add_condition_branch(static_cast<uint32_t>(il_offset_cur), target, hl::OpCodeEnum::Blt));
                break;
            }
            case il::OpCodeValue::BneUn:
            {
                int32_t target = (int32_t)utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                il_offset_cur += 5;
                RET_ERR_ON_FAIL(add_condition_branch(static_cast<uint32_t>(il_offset_cur), target, hl::OpCodeEnum::BneUn));
                break;
            }
            case il::OpCodeValue::BgeUn:
            {
                int32_t target = (int32_t)utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                il_offset_cur += 5;
                RET_ERR_ON_FAIL(add_condition_branch(static_cast<uint32_t>(il_offset_cur), target, hl::OpCodeEnum::BgeUn));
                break;
            }
            case il::OpCodeValue::BgtUn:
            {
                int32_t target = (int32_t)utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                il_offset_cur += 5;
                RET_ERR_ON_FAIL(add_condition_branch(static_cast<uint32_t>(il_offset_cur), target, hl::OpCodeEnum::BgtUn));
                break;
            }
            case il::OpCodeValue::BleUn:
            {
                int32_t target = (int32_t)utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                il_offset_cur += 5;
                RET_ERR_ON_FAIL(add_condition_branch(static_cast<uint32_t>(il_offset_cur), target, hl::OpCodeEnum::BleUn));
                break;
            }
            case il::OpCodeValue::BltUn:
            {
                int32_t target = (int32_t)utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                il_offset_cur += 5;
                RET_ERR_ON_FAIL(add_condition_branch(static_cast<uint32_t>(il_offset_cur), target, hl::OpCodeEnum::BltUn));
                break;
            }
            case il::OpCodeValue::Switch:
            {
                uint32_t case_count = utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                il_offset_cur += 5;
                const int8_t* cases_begin = (const int8_t*)(codes_begin + il_offset_cur);
                il_offset_cur += case_count * 4;
                if (il_offset_cur > il_offset_end)
                    RET_ASSERT_ERR(RtErr::ExecutionEngine);
                RET_ERR_ON_FAIL(add_switch(static_cast<uint32_t>(il_offset_cur), cases_begin, case_count));
                break;
            }
            case il::OpCodeValue::LdIndI1:
            {
                RET_ERR_ON_FAIL(add_ldind(hl::OpCodeEnum::LdIndI1, metadata::RtArgOrLocOrFieldReduceType::I1));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdIndU1:
            {
                RET_ERR_ON_FAIL(add_ldind(hl::OpCodeEnum::LdIndU1, metadata::RtArgOrLocOrFieldReduceType::U1));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdIndI2:
            {
                RET_ERR_ON_FAIL(add_ldind(hl::OpCodeEnum::LdIndI2, metadata::RtArgOrLocOrFieldReduceType::I2));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdIndU2:
            {
                RET_ERR_ON_FAIL(add_ldind(hl::OpCodeEnum::LdIndU2, metadata::RtArgOrLocOrFieldReduceType::U2));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdIndI4:
            {
                RET_ERR_ON_FAIL(add_ldind(hl::OpCodeEnum::LdIndI4, metadata::RtArgOrLocOrFieldReduceType::I4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdIndU4:
            {
                RET_ERR_ON_FAIL(add_ldind(hl::OpCodeEnum::LdIndI4, metadata::RtArgOrLocOrFieldReduceType::I4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdIndI8:
            {
                RET_ERR_ON_FAIL(add_ldind(hl::OpCodeEnum::LdIndI8, metadata::RtArgOrLocOrFieldReduceType::I8));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdIndI:
            {
                // FIXME: we maybe need to add LdIntI for native int type
                RET_ERR_ON_FAIL(add_ldind(hl::OpCodeEnum::LdIndRef, metadata::RtArgOrLocOrFieldReduceType::I));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdIndR4:
            {
                RET_ERR_ON_FAIL(add_ldind(hl::OpCodeEnum::LdIndR4, metadata::RtArgOrLocOrFieldReduceType::R4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdIndR8:
            {
                RET_ERR_ON_FAIL(add_ldind(hl::OpCodeEnum::LdIndR8, metadata::RtArgOrLocOrFieldReduceType::R8));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdIndRef:
            {
                RET_ERR_ON_FAIL(add_ldind(hl::OpCodeEnum::LdIndRef, metadata::RtArgOrLocOrFieldReduceType::Ref));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::StIndRef:
            {
                RET_ERR_ON_FAIL(add_stind(hl::OpCodeEnum::StIndRef));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::StIndI1:
            {
                RET_ERR_ON_FAIL(add_stind(hl::OpCodeEnum::StIndI1));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::StIndI2:
            {
                RET_ERR_ON_FAIL(add_stind(hl::OpCodeEnum::StIndI2));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::StIndI4:
            {
                RET_ERR_ON_FAIL(add_stind(hl::OpCodeEnum::StIndI4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::StIndI8:
            {
                RET_ERR_ON_FAIL(add_stind(hl::OpCodeEnum::StIndI8));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::StIndR4:
            {
                RET_ERR_ON_FAIL(add_stind(hl::OpCodeEnum::StIndR4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::StIndR8:
            {
                RET_ERR_ON_FAIL(add_stind(hl::OpCodeEnum::StIndR8));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::Add:
            {
                RET_ERR_ON_FAIL(add_bin_arith_op(hl::OpCodeEnum::Add));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::Sub:
            {
                RET_ERR_ON_FAIL(add_bin_arith_op(hl::OpCodeEnum::Sub));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::Mul:
            {
                RET_ERR_ON_FAIL(add_bin_arith_op(hl::OpCodeEnum::Mul));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::Div:
            {
                RET_ERR_ON_FAIL(add_bin_arith_op(hl::OpCodeEnum::Div));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::DivUn:
            {
                RET_ERR_ON_FAIL(add_bin_arith_op(hl::OpCodeEnum::DivUn));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::Rem:
            {
                RET_ERR_ON_FAIL(add_bin_arith_op(hl::OpCodeEnum::Rem));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::RemUn:
            {
                RET_ERR_ON_FAIL(add_bin_arith_op(hl::OpCodeEnum::RemUn));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::And:
            {
                RET_ERR_ON_FAIL(add_bin_bit_op(hl::OpCodeEnum::And));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::Or:
            {
                RET_ERR_ON_FAIL(add_bin_bit_op(hl::OpCodeEnum::Or));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::Xor:
            {
                RET_ERR_ON_FAIL(add_bin_bit_op(hl::OpCodeEnum::Xor));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::Shl:
            {
                RET_ERR_ON_FAIL(add_bit_shift_op(hl::OpCodeEnum::Shl));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::Shr:
            {
                RET_ERR_ON_FAIL(add_bit_shift_op(hl::OpCodeEnum::Shr));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::ShrUn:
            {
                RET_ERR_ON_FAIL(add_bit_shift_op(hl::OpCodeEnum::ShrUn));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::Neg:
            {
                RET_ERR_ON_FAIL(add_neg());
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::Not:
            {
                RET_ERR_ON_FAIL(add_not());
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::ConvI1:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvI1, RtEvalStackDataType::I4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::ConvI2:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvI2, RtEvalStackDataType::I4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::ConvI4:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvI4, RtEvalStackDataType::I4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::ConvI8:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvI8, RtEvalStackDataType::I8));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::ConvR4:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvR4, RtEvalStackDataType::R4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::ConvR8:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvR8, RtEvalStackDataType::R8));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::ConvU4:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvU4, RtEvalStackDataType::I4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::ConvU8:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvU8, RtEvalStackDataType::I8));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::Callvirt:
            {
                uint32_t method_token = utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method, get_method_from_token(method_token));
                RET_ERR_ON_FAIL(add_callvirt(method));
                il_offset_cur += 5;
                break;
            }
            case il::OpCodeValue::Cpobj:
            {
                uint32_t type_token = utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, get_class_from_token(type_token));
                RET_ERR_ON_FAIL(add_cpobj(klass));
                il_offset_cur += 5;
                break;
            }
            case il::OpCodeValue::Ldobj:
            {
                uint32_t type_token = utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, get_class_from_token(type_token));
                RET_ERR_ON_FAIL(add_ldobj(klass));
                il_offset_cur += 5;
                break;
            }
            case il::OpCodeValue::Ldstr:
            {
                uint32_t str_token = utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                uint32_t user_string_index = metadata::RtToken::decode_rid(str_token);
                RET_ERR_ON_FAIL(add_ldstr(user_string_index));
                il_offset_cur += 5;
                break;
            }
            case il::OpCodeValue::Newobj:
            {
                uint32_t method_token = utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method, get_method_from_token(method_token));
                RET_ERR_ON_FAIL(add_newobj(method));
                il_offset_cur += 5;
                break;
            }
            case il::OpCodeValue::Castclass:
            {
                uint32_t type_token = utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, get_class_from_token(type_token));
                RET_ERR_ON_FAIL(add_castclass(klass));
                il_offset_cur += 5;
                break;
            }
            case il::OpCodeValue::Isinst:
            {
                uint32_t type_token = utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, get_class_from_token(type_token));
                RET_ERR_ON_FAIL(add_isinst(klass));
                il_offset_cur += 5;
                break;
            }
            case il::OpCodeValue::ConvRUn:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvR8, RtEvalStackDataType::R8));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::Unbox:
            {
                uint32_t type_token = utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, get_class_from_token(type_token));
                RET_ERR_ON_FAIL(add_unbox(klass));
                il_offset_cur += 5;
                break;
            }
            case il::OpCodeValue::Unused58:
            case il::OpCodeValue::Unused1:
            {
                RET_ASSERT_ERR(RtErr::ExecutionEngine);
            }
            case il::OpCodeValue::Throw:
            {
                RET_ERR_ON_FAIL(add_throw());
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::Ldfld:
            {
                uint32_t field_token = utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtFieldInfo*, field, get_field_from_token(field_token));
                RET_ERR_ON_FAIL(add_ldfld(field));
                il_offset_cur += 5;
                break;
            }
            case il::OpCodeValue::Ldflda:
            {
                uint32_t field_token = utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtFieldInfo*, field, get_field_from_token(field_token));
                RET_ERR_ON_FAIL(add_ldflda(field));
                il_offset_cur += 5;
                break;
            }
            case il::OpCodeValue::Stfld:
            {
                uint32_t field_token = utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtFieldInfo*, field, get_field_from_token(field_token));
                RET_ERR_ON_FAIL(add_stfld(field));
                il_offset_cur += 5;
                break;
            }
            case il::OpCodeValue::Ldsfld:
            {
                uint32_t field_token = utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtFieldInfo*, field, get_field_from_token(field_token));
                RET_ERR_ON_FAIL(add_ldsfld(field));
                il_offset_cur += 5;
                break;
            }
            case il::OpCodeValue::Ldsflda:
            {
                uint32_t field_token = utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtFieldInfo*, field, get_field_from_token(field_token));
                RET_ERR_ON_FAIL(add_ldsflda(field));
                il_offset_cur += 5;
                break;
            }
            case il::OpCodeValue::Stsfld:
            {
                uint32_t field_token = utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtFieldInfo*, field, get_field_from_token(field_token));
                RET_ERR_ON_FAIL(add_stsfld(field));
                il_offset_cur += 5;
                break;
            }
            case il::OpCodeValue::Stobj:
            {
                uint32_t type_token = utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, get_class_from_token(type_token));
                RET_ERR_ON_FAIL(add_stobj(klass));
                il_offset_cur += 5;
                break;
            }
            case il::OpCodeValue::ConvOvfI1Un:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvOvfI1Un, RtEvalStackDataType::I4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::ConvOvfU1Un:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvOvfU1Un, RtEvalStackDataType::I4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::ConvOvfI2Un:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvOvfI2Un, RtEvalStackDataType::I4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::ConvOvfU2Un:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvOvfU2Un, RtEvalStackDataType::I4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::ConvOvfI4Un:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvOvfI4Un, RtEvalStackDataType::I4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::ConvOvfU4Un:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvOvfU4Un, RtEvalStackDataType::I4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::ConvOvfI8Un:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvOvfI8Un, RtEvalStackDataType::I8));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::ConvOvfU8Un:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvOvfU8Un, RtEvalStackDataType::I8));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::ConvOvfIUn:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvOvfIUn, RtEvalStackDataType::RefOrPtr));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::ConvOvfUUn:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvOvfUUn, RtEvalStackDataType::RefOrPtr));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::Box:
            {
                uint32_t type_token = utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, get_class_from_token(type_token));
                RET_ERR_ON_FAIL(add_box(klass));
                il_offset_cur += 5;
                break;
            }
            case il::OpCodeValue::Newarr:
            {
                uint32_t type_token = utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, get_class_from_token(type_token));
                RET_ERR_ON_FAIL(add_newarr(klass));
                il_offset_cur += 5;
                break;
            }
            case il::OpCodeValue::Ldlen:
            {
                RET_ERR_ON_FAIL(add_ldlen());
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::Ldelema:
            {
                uint32_t type_token = utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, get_class_from_token(type_token));
                RET_ERR_ON_FAIL(add_ldelema(klass));
                il_offset_cur += 5;
                break;
            }
            case il::OpCodeValue::LdelemI1:
            {
                RET_ERR_ON_FAIL(add_ldelem(hl::OpCodeEnum::LdelemI1, RtEvalStackDataType::I4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdelemU1:
            {
                RET_ERR_ON_FAIL(add_ldelem(hl::OpCodeEnum::LdelemU1, RtEvalStackDataType::I4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdelemI2:
            {
                RET_ERR_ON_FAIL(add_ldelem(hl::OpCodeEnum::LdelemI2, RtEvalStackDataType::I4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdelemU2:
            {
                RET_ERR_ON_FAIL(add_ldelem(hl::OpCodeEnum::LdelemU2, RtEvalStackDataType::I4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdelemI4:
            {
                RET_ERR_ON_FAIL(add_ldelem(hl::OpCodeEnum::LdelemI4, RtEvalStackDataType::I4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdelemU4:
            {
                RET_ERR_ON_FAIL(add_ldelem(hl::OpCodeEnum::LdelemI4, RtEvalStackDataType::I4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdelemI8:
            {
                RET_ERR_ON_FAIL(add_ldelem(hl::OpCodeEnum::LdelemI8, RtEvalStackDataType::I8));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdelemI:
            {
                RET_ERR_ON_FAIL(add_ldelem(hl::OpCodeEnum::LdelemI, RtEvalStackDataType::RefOrPtr));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdelemR4:
            {
                RET_ERR_ON_FAIL(add_ldelem(hl::OpCodeEnum::LdelemR4, RtEvalStackDataType::R4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdelemR8:
            {
                RET_ERR_ON_FAIL(add_ldelem(hl::OpCodeEnum::LdelemR8, RtEvalStackDataType::R8));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::LdelemRef:
            {
                RET_ERR_ON_FAIL(add_ldelem(hl::OpCodeEnum::LdelemRef, RtEvalStackDataType::RefOrPtr));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::StelemI1:
            {
                RET_ERR_ON_FAIL(add_stelem(hl::OpCodeEnum::StelemI1, RtEvalStackDataType::I4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::StelemI2:
            {
                RET_ERR_ON_FAIL(add_stelem(hl::OpCodeEnum::StelemI2, RtEvalStackDataType::I4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::StelemI4:
            {
                RET_ERR_ON_FAIL(add_stelem(hl::OpCodeEnum::StelemI4, RtEvalStackDataType::I4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::StelemI8:
            {
                RET_ERR_ON_FAIL(add_stelem(hl::OpCodeEnum::StelemI8, RtEvalStackDataType::I8));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::StelemI:
            {
                RET_ERR_ON_FAIL(add_stelem(hl::OpCodeEnum::StelemI, RtEvalStackDataType::RefOrPtr));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::StelemR4:
            {
                RET_ERR_ON_FAIL(add_stelem(hl::OpCodeEnum::StelemR4, RtEvalStackDataType::R4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::StelemR8:
            {
                RET_ERR_ON_FAIL(add_stelem(hl::OpCodeEnum::StelemR8, RtEvalStackDataType::R8));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::StelemRef:
            {
                RET_ERR_ON_FAIL(add_stelem(hl::OpCodeEnum::StelemRef, RtEvalStackDataType::RefOrPtr));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::Ldelem:
            {
                uint32_t type_token = utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, get_class_from_token(type_token));
                RET_ERR_ON_FAIL(add_ldelem_any(klass));
                il_offset_cur += 5;
                break;
            }
            case il::OpCodeValue::Stelem:
            {
                uint32_t type_token = utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, get_class_from_token(type_token));
                RET_ERR_ON_FAIL(add_stelem_any(klass));
                il_offset_cur += 5;
                break;
            }
            case il::OpCodeValue::UnboxAny:
            {
                uint32_t type_token = utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, get_class_from_token(type_token));
                RET_ERR_ON_FAIL(add_unbox_any(klass));
                il_offset_cur += 5;
                break;
            }
            case il::OpCodeValue::Unused5:
            case il::OpCodeValue::Unused6:
            case il::OpCodeValue::Unused7:
            case il::OpCodeValue::Unused8:
            case il::OpCodeValue::Unused9:
            case il::OpCodeValue::Unused10:
            case il::OpCodeValue::Unused11:
            case il::OpCodeValue::Unused12:
            case il::OpCodeValue::Unused13:
            case il::OpCodeValue::Unused14:
            case il::OpCodeValue::Unused15:
            case il::OpCodeValue::Unused16:
            case il::OpCodeValue::Unused17:
            {
                RET_ASSERT_ERR(RtErr::ExecutionEngine);
            }
            case il::OpCodeValue::ConvOvfI1:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvOvfI1, RtEvalStackDataType::I4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::ConvOvfU1:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvOvfU1, RtEvalStackDataType::I4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::ConvOvfI2:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvOvfI2, RtEvalStackDataType::I4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::ConvOvfU2:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvOvfU2, RtEvalStackDataType::I4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::ConvOvfI4:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvOvfI4, RtEvalStackDataType::I4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::ConvOvfU4:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvOvfU4, RtEvalStackDataType::I4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::ConvOvfI8:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvOvfI8, RtEvalStackDataType::I8));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::ConvOvfU8:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvOvfU8, RtEvalStackDataType::I8));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::Unused50:
            case il::OpCodeValue::Unused18:
            case il::OpCodeValue::Unused19:
            case il::OpCodeValue::Unused20:
            case il::OpCodeValue::Unused21:
            case il::OpCodeValue::Unused22:
            case il::OpCodeValue::Unused23:
            {
                RET_ASSERT_ERR(RtErr::ExecutionEngine);
            }
            case il::OpCodeValue::Refanyval:
            {
                uint32_t type_token = utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, get_class_from_token(type_token));
                RET_ERR_ON_FAIL(add_refanyval(klass));
                il_offset_cur += 5;
                break;
            }
            case il::OpCodeValue::Ckfinite:
            {
                RET_ERR_ON_FAIL(add_ckfinite());
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::Unused24:
            case il::OpCodeValue::Unused25:
            {
                RET_ASSERT_ERR(RtErr::ExecutionEngine);
            }
            case il::OpCodeValue::Mkrefany:
            {
                uint32_t type_token = utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, get_class_from_token(type_token));
                RET_ERR_ON_FAIL(add_mkrefany(klass));
                il_offset_cur += 5;
                break;
            }
            case il::OpCodeValue::Unused59:
            case il::OpCodeValue::Unused60:
            case il::OpCodeValue::Unused61:
            case il::OpCodeValue::Unused62:
            case il::OpCodeValue::Unused63:
            case il::OpCodeValue::Unused64:
            case il::OpCodeValue::Unused65:
            case il::OpCodeValue::Unused66:
            case il::OpCodeValue::Unused67:
            {
                RET_ASSERT_ERR(RtErr::ExecutionEngine);
            }
            case il::OpCodeValue::Ldtoken:
            {
                uint32_t token = utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                RET_ERR_ON_FAIL(add_ldtoken(token));
                il_offset_cur += 5;
                break;
            }
            case il::OpCodeValue::ConvU1:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvU1, RtEvalStackDataType::I4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::ConvU2:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvU2, RtEvalStackDataType::I4));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::ConvI:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvI, RtEvalStackDataType::RefOrPtr));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::ConvOvfI:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvOvfI, RtEvalStackDataType::RefOrPtr));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::ConvOvfU:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvOvfU, RtEvalStackDataType::RefOrPtr));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::AddOvf:
            {
                RET_ERR_ON_FAIL(add_bin_arith_op(hl::OpCodeEnum::AddOvf));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::AddOvfUn:
            {
                RET_ERR_ON_FAIL(add_bin_arith_op(hl::OpCodeEnum::AddOvfUn));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::MulOvf:
            {
                RET_ERR_ON_FAIL(add_bin_arith_op(hl::OpCodeEnum::MulOvf));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::MulOvfUn:
            {
                RET_ERR_ON_FAIL(add_bin_arith_op(hl::OpCodeEnum::MulOvfUn));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::SubOvf:
            {
                RET_ERR_ON_FAIL(add_bin_arith_op(hl::OpCodeEnum::SubOvf));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::SubOvfUn:
            {
                RET_ERR_ON_FAIL(add_bin_arith_op(hl::OpCodeEnum::SubOvfUn));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::Endfinally:
            {
                RET_ERR_ON_FAIL(add_endfinally());
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::Leave:
            {
                int32_t target = (int32_t)utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                il_offset_cur += 5;
                RET_ERR_ON_FAIL(add_leave(il_offset_cur, target));
                break;
            }
            case il::OpCodeValue::LeaveS:
            {
                int8_t target = *(const int8_t*)(codes_begin + il_offset_cur + 1);
                il_offset_cur += 2;
                RET_ERR_ON_FAIL(add_leave(il_offset_cur, target));
                break;
            }
            case il::OpCodeValue::StIndI:
            {
                // FIXME: we maybe need to add StIndI for this.
                RET_ERR_ON_FAIL(add_stind(hl::OpCodeEnum::StIndRef));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::ConvU:
            {
                RET_ERR_ON_FAIL(add_conv(hl::OpCodeEnum::ConvU, RtEvalStackDataType::RefOrPtr));
                il_offset_cur += 1;
                break;
            }
            case il::OpCodeValue::Unused26:
            case il::OpCodeValue::Unused27:
            case il::OpCodeValue::Unused28:
            case il::OpCodeValue::Unused29:
            case il::OpCodeValue::Unused30:
            case il::OpCodeValue::Unused31:
            case il::OpCodeValue::Unused32:
            case il::OpCodeValue::Unused33:
            case il::OpCodeValue::Unused34:
            case il::OpCodeValue::Unused35:
            case il::OpCodeValue::Unused36:
            case il::OpCodeValue::Unused37:
            case il::OpCodeValue::Unused38:
            case il::OpCodeValue::Unused39:
            case il::OpCodeValue::Unused40:
            case il::OpCodeValue::Unused41:
            case il::OpCodeValue::Unused42:
            case il::OpCodeValue::Unused43:
            case il::OpCodeValue::Unused44:
            case il::OpCodeValue::Unused45:
            case il::OpCodeValue::Unused46:
            case il::OpCodeValue::Unused47:
            case il::OpCodeValue::Unused48:
            {
                RET_ASSERT_ERR(RtErr::ExecutionEngine);
            }
            case il::OpCodeValue::Prefix7:
            case il::OpCodeValue::Prefix6:
            case il::OpCodeValue::Prefix5:
            case il::OpCodeValue::Prefix4:
            case il::OpCodeValue::Prefix3:
            case il::OpCodeValue::Prefix2:
            case il::OpCodeValue::Prefix1:
            {
                il_offset_cur += 1;
                uint8_t ext_opcode_val = *(codes_begin + il_offset_cur);
                il::OpCodeValueExt ext_opcode = static_cast<il::OpCodeValueExt>(ext_opcode_val);
                switch (ext_opcode)
                {
                case il::OpCodeValueExt::Arglist:
                {
                    RET_ERR_ON_FAIL(add_arglist());
                    il_offset_cur += 1;
                    break;
                }
                case il::OpCodeValueExt::Ceq:
                {
                    RET_ERR_ON_FAIL(add_bin_compare_op(hl::OpCodeEnum::Ceq));
                    il_offset_cur += 1;
                    break;
                }
                case il::OpCodeValueExt::Cgt:
                {
                    RET_ERR_ON_FAIL(add_bin_compare_op(hl::OpCodeEnum::Cgt));
                    il_offset_cur += 1;
                    break;
                }
                case il::OpCodeValueExt::CgtUn:
                {
                    RET_ERR_ON_FAIL(add_bin_compare_op(hl::OpCodeEnum::CgtUn));
                    il_offset_cur += 1;
                    break;
                }
                case il::OpCodeValueExt::Clt:
                {
                    RET_ERR_ON_FAIL(add_bin_compare_op(hl::OpCodeEnum::Clt));
                    il_offset_cur += 1;
                    break;
                }
                case il::OpCodeValueExt::CltUn:
                {
                    RET_ERR_ON_FAIL(add_bin_compare_op(hl::OpCodeEnum::CltUn));
                    il_offset_cur += 1;
                    break;
                }
                case il::OpCodeValueExt::Ldftn:
                {
                    uint32_t method_token = utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method, get_method_from_token(method_token));
                    RET_ERR_ON_FAIL(add_ldftn(method));
                    il_offset_cur += 5;
                    break;
                }
                case il::OpCodeValueExt::Ldvirtftn:
                {
                    uint32_t method_token = utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method, get_method_from_token(method_token));
                    RET_ERR_ON_FAIL(add_ldvirtftn(method));
                    il_offset_cur += 5;
                    break;
                }
                case il::OpCodeValueExt::Unused56:
                {
                    RET_ASSERT_ERR(RtErr::ExecutionEngine);
                }
                case il::OpCodeValueExt::Ldarg:
                {
                    uint16_t arg_index = utils::MemOp::read_u16_may_unaligned(codes_begin + il_offset_cur + 1);
                    RET_ERR_ON_FAIL(add_ldarg(arg_index));
                    il_offset_cur += 3;
                    break;
                }
                case il::OpCodeValueExt::Ldarga:
                {
                    uint16_t arg_index = utils::MemOp::read_u16_may_unaligned(codes_begin + il_offset_cur + 1);
                    RET_ERR_ON_FAIL(add_ldarga(arg_index));
                    il_offset_cur += 3;
                    break;
                }
                case il::OpCodeValueExt::Starg:
                {
                    uint16_t arg_index = utils::MemOp::read_u16_may_unaligned(codes_begin + il_offset_cur + 1);
                    RET_ERR_ON_FAIL(add_starg(arg_index));
                    il_offset_cur += 3;
                    break;
                }
                case il::OpCodeValueExt::Ldloc:
                {
                    uint16_t loc_index = utils::MemOp::read_u16_may_unaligned(codes_begin + il_offset_cur + 1);
                    RET_ERR_ON_FAIL(add_ldloc(loc_index));
                    il_offset_cur += 3;
                    break;
                }
                case il::OpCodeValueExt::Ldloca:
                {
                    uint16_t loc_index = utils::MemOp::read_u16_may_unaligned(codes_begin + il_offset_cur + 1);
                    RET_ERR_ON_FAIL(add_ldloca(loc_index));
                    il_offset_cur += 3;
                    break;
                }
                case il::OpCodeValueExt::Stloc:
                {
                    uint16_t loc_index = utils::MemOp::read_u16_may_unaligned(codes_begin + il_offset_cur + 1);
                    RET_ERR_ON_FAIL(add_stloc(loc_index));
                    il_offset_cur += 3;
                    break;
                }
                case il::OpCodeValueExt::Localloc:
                {
                    RET_ERR_ON_FAIL(add_localloc());
                    il_offset_cur += 1;
                    break;
                }
                case il::OpCodeValueExt::Unused57:
                {
                    RET_ASSERT_ERR(RtErr::ExecutionEngine);
                }
                case il::OpCodeValueExt::Endfilter:
                {
                    RET_ERR_ON_FAIL(add_endfilter());
                    il_offset_cur += 1;
                    break;
                }
                case il::OpCodeValueExt::Unaligned:
                {
                    uint8_t alignment = *(codes_begin + il_offset_cur + 1);
                    if (alignment != 1 && alignment != 2 && alignment != 4)
                    {
                        RET_ASSERT_ERR(RtErr::ExecutionEngine);
                    }
                    add_prefix(il::OpCodePrefix::Unaligned);
                    il_offset_cur += 2;
                    break;
                }
                case il::OpCodeValueExt::Volatile:
                {
                    add_prefix(il::OpCodePrefix::Volatile);
                    il_offset_cur += 1;
                    break;
                }
                case il::OpCodeValueExt::Tail:
                {
                    set_prefix(il::OpCodePrefix::Tail);
                    il_offset_cur += 1;
                    break;
                }
                case il::OpCodeValueExt::Initobj:
                {
                    uint32_t type_token = utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, get_class_from_token(type_token));
                    RET_ERR_ON_FAIL(add_initobj(klass));
                    il_offset_cur += 5;
                    break;
                }
                case il::OpCodeValueExt::Constrained:
                {
                    uint32_t type_token = utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, get_class_from_token(type_token));
                    set_constrainted_prefix(klass);
                    il_offset_cur += 5;
                    break;
                }
                case il::OpCodeValueExt::Cpblk:
                {
                    RET_ERR_ON_FAIL(add_cpblk());
                    il_offset_cur += 1;
                    break;
                }
                case il::OpCodeValueExt::Initblk:
                {
                    RET_ERR_ON_FAIL(add_initblk());
                    il_offset_cur += 1;
                    break;
                }
                case il::OpCodeValueExt::No:
                {
                    uint8_t check_type = *(codes_begin + il_offset_cur + 1);
                    if ((check_type & 0xF8) != 0)
                    {
                        RET_ASSERT_ERR(RtErr::ExecutionEngine);
                    }
                    add_prefix(il::OpCodePrefix::No);
                    il_offset_cur += 2;
                    break;
                }
                case il::OpCodeValueExt::Rethrow:
                {
                    RET_ERR_ON_FAIL(add_rethrow());
                    il_offset_cur += 1;
                    break;
                }
                case il::OpCodeValueExt::Sizeof:
                {
                    uint32_t type_token = utils::MemOp::read_u32_may_unaligned(codes_begin + il_offset_cur + 1);
                    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, type_sig, get_type_from_token(type_token));
                    RET_ERR_ON_FAIL(add_sizeof(type_sig));
                    il_offset_cur += 5;
                    break;
                }
                case il::OpCodeValueExt::Refanytype:
                {
                    RET_ERR_ON_FAIL(add_refanytype());
                    il_offset_cur += 1;
                    break;
                }
                case il::OpCodeValueExt::Readonly:
                {
                    add_prefix(il::OpCodePrefix::ReadOnly);
                    il_offset_cur += 1;
                    break;
                }
                default:
                {
                    RET_ASSERT_ERR(RtErr::ExecutionEngine);
                }
                }
                break;
            }
            default:
            {
                // TODO: Additional opcodes to be implemented
                RETURN_NOT_IMPLEMENTED_ERROR();
            }
            }
        }

        if (!bb->next_bb)
        {
            break;
        }
        _cur_bb = bb->next_bb;
    }
    RET_VOID_OK();
}

} // namespace hl
} // namespace interp
} // namespace leanclr
