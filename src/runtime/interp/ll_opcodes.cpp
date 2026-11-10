#include "ll_opcodes.h"
#include "transformer.h"

namespace leanclr
{
namespace interp
{
namespace ll
{

size_t OpCodes::get_switch_instruction_size(const GeneralInst& inst)
{
    return sizeof(Switch) + sizeof(int32_t) * (inst.get_switch_targets().count);
}

size_t OpCodes::s_opsizes[static_cast<size_t>(OpCodeEnum::__Count)] = {
    //{{LOW_LEVEL_INSTRUCTION_SIZES
    sizeof(Illegal),
    sizeof(Nop),
    sizeof(ProfileAddCost),
    sizeof(InitLocals),
    sizeof(Arglist),
    sizeof(LdLocI1),
    sizeof(LdLocU1),
    sizeof(LdLocI2),
    sizeof(LdLocU2),
    sizeof(LdLocI4),
    sizeof(LdLocI8),
    sizeof(LdLocAny),
    sizeof(LdLoca),
    sizeof(StLocI1),
    sizeof(StLocI2),
    sizeof(StLocI4),
    sizeof(StLocI8),
    sizeof(StLocAny),
    sizeof(LdNull),
    sizeof(LdcI4I2),
    sizeof(LdcI4I4),
    sizeof(LdcI8I2),
    sizeof(LdcI8I4),
    sizeof(LdcI8I8),
    sizeof(LdStr),
    sizeof(Br),
    sizeof(BrTrueI4),
    sizeof(BrTrueI8),
    sizeof(BrFalseI4),
    sizeof(BrFalseI8),
    sizeof(BeqI4),
    sizeof(BeqI8),
    sizeof(BeqR4),
    sizeof(BeqR8),
    sizeof(BgeI4),
    sizeof(BgeI8),
    sizeof(BgeR4),
    sizeof(BgeR8),
    sizeof(BgtI4),
    sizeof(BgtI8),
    sizeof(BgtR4),
    sizeof(BgtR8),
    sizeof(BleI4),
    sizeof(BleI8),
    sizeof(BleR4),
    sizeof(BleR8),
    sizeof(BltI4),
    sizeof(BltI8),
    sizeof(BltR4),
    sizeof(BltR8),
    sizeof(BneUnI4),
    sizeof(BneUnI8),
    sizeof(BneUnR4),
    sizeof(BneUnR8),
    sizeof(BgeUnI4),
    sizeof(BgeUnI8),
    sizeof(BgeUnR4),
    sizeof(BgeUnR8),
    sizeof(BgtUnI4),
    sizeof(BgtUnI8),
    sizeof(BgtUnR4),
    sizeof(BgtUnR8),
    sizeof(BleUnI4),
    sizeof(BleUnI8),
    sizeof(BleUnR4),
    sizeof(BleUnR8),
    sizeof(BltUnI4),
    sizeof(BltUnI8),
    sizeof(BltUnR4),
    sizeof(BltUnR8),
    sizeof(Switch),
    sizeof(LdIndI1),
    sizeof(LdIndU1),
    sizeof(LdIndI2),
    sizeof(LdIndI2Unaligned),
    sizeof(LdIndU2),
    sizeof(LdIndU2Unaligned),
    sizeof(LdIndI4),
    sizeof(LdIndI4Unaligned),
    sizeof(LdIndI8),
    sizeof(LdIndI8Unaligned),
    sizeof(StIndI1),
    sizeof(StIndI2),
    sizeof(StIndI2Unaligned),
    sizeof(StIndI4),
    sizeof(StIndI4Unaligned),
    sizeof(StIndI8),
    sizeof(StIndI8Unaligned),
    sizeof(StIndI8I4),
    sizeof(StIndI8I4Unaligned),
    sizeof(StIndI8U4),
    sizeof(StIndI8U4Unaligned),
    sizeof(AddI4),
    sizeof(AddI8),
    sizeof(AddR4),
    sizeof(AddR8),
    sizeof(SubI4),
    sizeof(SubI8),
    sizeof(SubR4),
    sizeof(SubR8),
    sizeof(MulI4),
    sizeof(MulI8),
    sizeof(MulR4),
    sizeof(MulR8),
    sizeof(DivI4),
    sizeof(DivI8),
    sizeof(DivR4),
    sizeof(DivR8),
    sizeof(DivUnI4),
    sizeof(DivUnI8),
    sizeof(RemI4),
    sizeof(RemI8),
    sizeof(RemR4),
    sizeof(RemR8),
    sizeof(RemUnI4),
    sizeof(RemUnI8),
    sizeof(AndI4),
    sizeof(AndI8),
    sizeof(OrI4),
    sizeof(OrI8),
    sizeof(XorI4),
    sizeof(XorI8),
    sizeof(ShlI4),
    sizeof(ShlI8),
    sizeof(ShrI4),
    sizeof(ShrI8),
    sizeof(ShrUnI4),
    sizeof(ShrUnI8),
    sizeof(NegI4),
    sizeof(NegI8),
    sizeof(NegR4),
    sizeof(NegR8),
    sizeof(NotI4),
    sizeof(NotI8),
    sizeof(AddOvfI4),
    sizeof(AddOvfI8),
    sizeof(AddOvfUnI4),
    sizeof(AddOvfUnI8),
    sizeof(MulOvfI4),
    sizeof(MulOvfI8),
    sizeof(MulOvfUnI4),
    sizeof(MulOvfUnI8),
    sizeof(SubOvfI4),
    sizeof(SubOvfI8),
    sizeof(SubOvfUnI4),
    sizeof(SubOvfUnI8),
    sizeof(ConvI1I4),
    sizeof(ConvI1I8),
    sizeof(ConvI1R4),
    sizeof(ConvI1R8),
    sizeof(ConvU1I4),
    sizeof(ConvU1I8),
    sizeof(ConvU1R4),
    sizeof(ConvU1R8),
    sizeof(ConvI2I4),
    sizeof(ConvI2I8),
    sizeof(ConvI2R4),
    sizeof(ConvI2R8),
    sizeof(ConvU2I4),
    sizeof(ConvU2I8),
    sizeof(ConvU2R4),
    sizeof(ConvU2R8),
    sizeof(ConvI4I8),
    sizeof(ConvI4R4),
    sizeof(ConvI4R8),
    sizeof(ConvU4I8),
    sizeof(ConvU4R4),
    sizeof(ConvU4R8),
    sizeof(ConvI8I4),
    sizeof(ConvI8U4),
    sizeof(ConvI8R4),
    sizeof(ConvI8R8),
    sizeof(ConvU8I4),
    sizeof(ConvU8R4),
    sizeof(ConvU8R8),
    sizeof(ConvR4I4),
    sizeof(ConvR4I8),
    sizeof(ConvR4R8),
    sizeof(ConvR8I4),
    sizeof(ConvR8I8),
    sizeof(ConvR8R4),
    sizeof(ConvOvfI1I4),
    sizeof(ConvOvfI1I8),
    sizeof(ConvOvfI1R4),
    sizeof(ConvOvfI1R8),
    sizeof(ConvOvfU1I4),
    sizeof(ConvOvfU1I8),
    sizeof(ConvOvfU1R4),
    sizeof(ConvOvfU1R8),
    sizeof(ConvOvfI2I4),
    sizeof(ConvOvfI2I8),
    sizeof(ConvOvfI2R4),
    sizeof(ConvOvfI2R8),
    sizeof(ConvOvfU2I4),
    sizeof(ConvOvfU2I8),
    sizeof(ConvOvfU2R4),
    sizeof(ConvOvfU2R8),
    sizeof(ConvOvfI4I8),
    sizeof(ConvOvfI4R4),
    sizeof(ConvOvfI4R8),
    sizeof(ConvOvfU4I4),
    sizeof(ConvOvfU4I8),
    sizeof(ConvOvfU4R4),
    sizeof(ConvOvfU4R8),
    sizeof(ConvOvfI8R4),
    sizeof(ConvOvfI8R8),
    sizeof(ConvOvfU8I4),
    sizeof(ConvOvfU8I8),
    sizeof(ConvOvfU8R4),
    sizeof(ConvOvfU8R8),
    sizeof(ConvOvfI1UnI4),
    sizeof(ConvOvfI1UnI8),
    sizeof(ConvOvfI1UnR4),
    sizeof(ConvOvfI1UnR8),
    sizeof(ConvOvfU1UnI4),
    sizeof(ConvOvfU1UnI8),
    sizeof(ConvOvfU1UnR4),
    sizeof(ConvOvfU1UnR8),
    sizeof(ConvOvfI2UnI4),
    sizeof(ConvOvfI2UnI8),
    sizeof(ConvOvfI2UnR4),
    sizeof(ConvOvfI2UnR8),
    sizeof(ConvOvfU2UnI4),
    sizeof(ConvOvfU2UnI8),
    sizeof(ConvOvfU2UnR4),
    sizeof(ConvOvfU2UnR8),
    sizeof(ConvOvfI4UnI4),
    sizeof(ConvOvfI4UnI8),
    sizeof(ConvOvfI4UnR4),
    sizeof(ConvOvfI4UnR8),
    sizeof(ConvOvfU4UnI8),
    sizeof(ConvOvfU4UnR4),
    sizeof(ConvOvfU4UnR8),
    sizeof(ConvOvfI8UnI8),
    sizeof(ConvOvfI8UnR4),
    sizeof(ConvOvfI8UnR8),
    sizeof(ConvOvfU8UnR4),
    sizeof(ConvOvfU8UnR8),
    sizeof(CeqI4),
    sizeof(CeqI8),
    sizeof(CeqR4),
    sizeof(CeqR8),
    sizeof(CgtI4),
    sizeof(CgtI8),
    sizeof(CgtR4),
    sizeof(CgtR8),
    sizeof(CgtUnI4),
    sizeof(CgtUnI8),
    sizeof(CgtUnR4),
    sizeof(CgtUnR8),
    sizeof(CltI4),
    sizeof(CltI8),
    sizeof(CltR4),
    sizeof(CltR8),
    sizeof(CltUnI4),
    sizeof(CltUnI8),
    sizeof(CltUnR4),
    sizeof(CltUnR8),
    sizeof(InitObjI1),
    sizeof(InitObjI2),
    sizeof(InitObjI2Unaligned),
    sizeof(InitObjI4),
    sizeof(InitObjI4Unaligned),
    sizeof(InitObjI8),
    sizeof(InitObjI8Unaligned),
    sizeof(InitObjAny),
    sizeof(CpObjI1),
    sizeof(CpObjI2),
    sizeof(CpObjI4),
    sizeof(CpObjI8),
    sizeof(CpObjAny),
    sizeof(LdObjAny),
    sizeof(StObjAny),
    sizeof(CastClass),
    sizeof(IsInst),
    sizeof(Box),
    sizeof(Unbox),
    sizeof(UnboxAny),
    sizeof(NewArr),
    sizeof(LdLen),
    sizeof(Ldelema),
    sizeof(LdelemaReadOnly),
    sizeof(LdelemI1),
    sizeof(LdelemU1),
    sizeof(LdelemI2),
    sizeof(LdelemU2),
    sizeof(LdelemI4),
    sizeof(LdelemI8),
    sizeof(LdelemI),
    sizeof(LdelemR4),
    sizeof(LdelemR8),
    sizeof(LdelemRef),
    sizeof(LdelemAnyRef),
    sizeof(LdelemAnyVal),
    sizeof(StelemI1),
    sizeof(StelemI2),
    sizeof(StelemI4),
    sizeof(StelemI8),
    sizeof(StelemI),
    sizeof(StelemR4),
    sizeof(StelemR8),
    sizeof(StelemRef),
    sizeof(StelemAnyRef),
    sizeof(StelemAnyVal),
    sizeof(MkRefAny),
    sizeof(RefAnyVal),
    sizeof(RefAnyType),
    sizeof(LdToken),
    sizeof(CkfiniteR4),
    sizeof(CkfiniteR8),
    sizeof(LocAlloc),
    sizeof(InitBlk),
    sizeof(CpBlk),
    sizeof(Ldftn),
    sizeof(Ldvirtftn),
    sizeof(LdfldI1),
    sizeof(LdfldI1Large),
    sizeof(LdfldU1),
    sizeof(LdfldU1Large),
    sizeof(LdfldI2),
    sizeof(LdfldI2Large),
    sizeof(LdfldI2Unaligned),
    sizeof(LdfldU2),
    sizeof(LdfldU2Large),
    sizeof(LdfldU2Unaligned),
    sizeof(LdfldI4),
    sizeof(LdfldI4Large),
    sizeof(LdfldI4Unaligned),
    sizeof(LdfldI8),
    sizeof(LdfldI8Large),
    sizeof(LdfldI8Unaligned),
    sizeof(LdfldAny),
    sizeof(LdfldAnyLarge),
    sizeof(LdvfldI1),
    sizeof(LdvfldI1Large),
    sizeof(LdvfldU1),
    sizeof(LdvfldU1Large),
    sizeof(LdvfldI2),
    sizeof(LdvfldI2Large),
    sizeof(LdvfldI2Unaligned),
    sizeof(LdvfldU2),
    sizeof(LdvfldU2Large),
    sizeof(LdvfldU2Unaligned),
    sizeof(LdvfldI4),
    sizeof(LdvfldI4Large),
    sizeof(LdvfldI4Unaligned),
    sizeof(LdvfldI8),
    sizeof(LdvfldI8Large),
    sizeof(LdvfldI8Unaligned),
    sizeof(LdvfldAny),
    sizeof(LdvfldAnyLarge),
    sizeof(Ldflda),
    sizeof(LdfldaLarge),
    sizeof(StfldI1),
    sizeof(StfldI1Large),
    sizeof(StfldI2),
    sizeof(StfldI2Large),
    sizeof(StfldI2Unaligned),
    sizeof(StfldI4),
    sizeof(StfldI4Large),
    sizeof(StfldI4Unaligned),
    sizeof(StfldI8),
    sizeof(StfldI8Large),
    sizeof(StfldI8Unaligned),
    sizeof(StfldAny),
    sizeof(StfldAnyLarge),
    sizeof(LdsfldI1),
    sizeof(LdsfldU1),
    sizeof(LdsfldI2),
    sizeof(LdsfldU2),
    sizeof(LdsfldI4),
    sizeof(LdsfldI8),
    sizeof(LdsfldAny),
    sizeof(Ldsflda),
    sizeof(LdsfldRvaData),
    sizeof(StsfldI1),
    sizeof(StsfldI2),
    sizeof(StsfldI4),
    sizeof(StsfldI8),
    sizeof(StsfldAny),
    sizeof(RetVoid),
    sizeof(RetI4),
    sizeof(RetI8),
    sizeof(RetAny),
    sizeof(RetNop),
    sizeof(CallInterp),
    sizeof(CallVirtInterp),
    sizeof(CallInternalCall),
    sizeof(CallIntrinsic),
    sizeof(CallPInvoke),
    sizeof(CallAot),
    sizeof(CallRuntimeImplemented),
    sizeof(CalliInterp),
    sizeof(BoxRefInplace),
    sizeof(NewObjInterp),
    sizeof(NewValueTypeInterp),
    sizeof(NewObjInternalCall),
    sizeof(NewObjIntrinsic),
    sizeof(NewObjAot),
    sizeof(NewValueTypeAot),
    sizeof(Throw),
    sizeof(Rethrow),
    sizeof(LeaveTryWithFinally),
    sizeof(LeaveCatchWithFinally),
    sizeof(LeaveCatchWithoutFinally),
    sizeof(EndFilter),
    sizeof(EndFinally),
    sizeof(EndFault),
    sizeof(GetEnumLongHashCode),

    //}}LOW_LEVEL_INSTRUCTION_SIZESS
};

uint8_t* OpCodes::write_instruction_to_data(uint8_t* codes, const GeneralInst& inst)
{
    switch (inst.get_opcode())
    {
    //{{LOW_LEVEL_INSTRUCTION_WRITE_TO_DATA
    case OpCodeEnum::Illegal:
    {
        auto ir = (Illegal*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::Illegal);
        return codes + sizeof(Illegal);
    }
    case OpCodeEnum::Nop:
    {
        auto ir = (Nop*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::Nop);
        return codes + sizeof(Nop);
    }
    case OpCodeEnum::ProfileAddCost:
    {
        auto ir = (ProfileAddCost*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ProfileAddCost);
        ir->cost = (uint32_t)inst.get_i4();
        return codes + sizeof(ProfileAddCost);
    }
    case OpCodeEnum::InitLocals:
    {
        auto ir = (InitLocals*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::InitLocals);
        ir->offset = (uint16_t)inst.get_locals_offset();
        ir->size = (uint16_t)inst.get_size();
        return codes + sizeof(InitLocals);
    }
    case OpCodeEnum::Arglist:
    {
        auto ir = (Arglist*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::Arglist);
        return codes + sizeof(Arglist);
    }
    case OpCodeEnum::LdLocI1:
    {
        auto ir = (LdLocI1*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdLocI1);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdLocI1);
    }
    case OpCodeEnum::LdLocU1:
    {
        auto ir = (LdLocU1*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdLocU1);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdLocU1);
    }
    case OpCodeEnum::LdLocI2:
    {
        auto ir = (LdLocI2*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdLocI2);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdLocI2);
    }
    case OpCodeEnum::LdLocU2:
    {
        auto ir = (LdLocU2*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdLocU2);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdLocU2);
    }
    case OpCodeEnum::LdLocI4:
    {
        auto ir = (LdLocI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdLocI4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdLocI4);
    }
    case OpCodeEnum::LdLocI8:
    {
        auto ir = (LdLocI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdLocI8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdLocI8);
    }
    case OpCodeEnum::LdLocAny:
    {
        auto ir = (LdLocAny*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdLocAny);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->size = (uint16_t)inst.get_size();
        return codes + sizeof(LdLocAny);
    }
    case OpCodeEnum::LdLoca:
    {
        auto ir = (LdLoca*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdLoca);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdLoca);
    }
    case OpCodeEnum::StLocI1:
    {
        auto ir = (StLocI1*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StLocI1);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StLocI1);
    }
    case OpCodeEnum::StLocI2:
    {
        auto ir = (StLocI2*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StLocI2);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StLocI2);
    }
    case OpCodeEnum::StLocI4:
    {
        auto ir = (StLocI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StLocI4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StLocI4);
    }
    case OpCodeEnum::StLocI8:
    {
        auto ir = (StLocI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StLocI8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StLocI8);
    }
    case OpCodeEnum::StLocAny:
    {
        auto ir = (StLocAny*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StLocAny);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->size = (uint16_t)inst.get_size();
        return codes + sizeof(StLocAny);
    }
    case OpCodeEnum::LdNull:
    {
        auto ir = (LdNull*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdNull);
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdNull);
    }
    case OpCodeEnum::LdcI4I2:
    {
        auto ir = (LdcI4I2*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdcI4I2);
        ir->value = (int16_t)inst.get_i2();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdcI4I2);
    }
    case OpCodeEnum::LdcI4I4:
    {
        auto ir = (LdcI4I4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdcI4I4);
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->value = (int32_t)inst.get_i4();
        return codes + sizeof(LdcI4I4);
    }
    case OpCodeEnum::LdcI8I2:
    {
        auto ir = (LdcI8I2*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdcI8I2);
        ir->value = (int16_t)inst.get_i2();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdcI8I2);
    }
    case OpCodeEnum::LdcI8I4:
    {
        auto ir = (LdcI8I4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdcI8I4);
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->value = (int32_t)inst.get_i4();
        return codes + sizeof(LdcI8I4);
    }
    case OpCodeEnum::LdcI8I8:
    {
        auto ir = (LdcI8I8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdcI8I8);
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->value_low = (int32_t)inst.get_i8_low();
        ir->value_high = (int32_t)inst.get_i8_high();
        return codes + sizeof(LdcI8I8);
    }
    case OpCodeEnum::LdStr:
    {
        auto ir = (LdStr*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdStr);
        ir->str_idx = (uint16_t)inst.get_resolved_data_index();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdStr);
    }
    case OpCodeEnum::Br:
    {
        auto ir = (Br*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::Br);
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(Br);
    }
    case OpCodeEnum::BrTrueI4:
    {
        auto ir = (BrTrueI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BrTrueI4);
        ir->condition = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BrTrueI4);
    }
    case OpCodeEnum::BrTrueI8:
    {
        auto ir = (BrTrueI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BrTrueI8);
        ir->condition = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BrTrueI8);
    }
    case OpCodeEnum::BrFalseI4:
    {
        auto ir = (BrFalseI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BrFalseI4);
        ir->condition = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BrFalseI4);
    }
    case OpCodeEnum::BrFalseI8:
    {
        auto ir = (BrFalseI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BrFalseI8);
        ir->condition = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BrFalseI8);
    }
    case OpCodeEnum::BeqI4:
    {
        auto ir = (BeqI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BeqI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BeqI4);
    }
    case OpCodeEnum::BeqI8:
    {
        auto ir = (BeqI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BeqI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BeqI8);
    }
    case OpCodeEnum::BeqR4:
    {
        auto ir = (BeqR4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BeqR4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BeqR4);
    }
    case OpCodeEnum::BeqR8:
    {
        auto ir = (BeqR8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BeqR8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BeqR8);
    }
    case OpCodeEnum::BgeI4:
    {
        auto ir = (BgeI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BgeI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BgeI4);
    }
    case OpCodeEnum::BgeI8:
    {
        auto ir = (BgeI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BgeI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BgeI8);
    }
    case OpCodeEnum::BgeR4:
    {
        auto ir = (BgeR4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BgeR4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BgeR4);
    }
    case OpCodeEnum::BgeR8:
    {
        auto ir = (BgeR8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BgeR8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BgeR8);
    }
    case OpCodeEnum::BgtI4:
    {
        auto ir = (BgtI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BgtI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BgtI4);
    }
    case OpCodeEnum::BgtI8:
    {
        auto ir = (BgtI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BgtI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BgtI8);
    }
    case OpCodeEnum::BgtR4:
    {
        auto ir = (BgtR4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BgtR4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BgtR4);
    }
    case OpCodeEnum::BgtR8:
    {
        auto ir = (BgtR8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BgtR8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BgtR8);
    }
    case OpCodeEnum::BleI4:
    {
        auto ir = (BleI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BleI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BleI4);
    }
    case OpCodeEnum::BleI8:
    {
        auto ir = (BleI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BleI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BleI8);
    }
    case OpCodeEnum::BleR4:
    {
        auto ir = (BleR4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BleR4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BleR4);
    }
    case OpCodeEnum::BleR8:
    {
        auto ir = (BleR8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BleR8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BleR8);
    }
    case OpCodeEnum::BltI4:
    {
        auto ir = (BltI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BltI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BltI4);
    }
    case OpCodeEnum::BltI8:
    {
        auto ir = (BltI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BltI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BltI8);
    }
    case OpCodeEnum::BltR4:
    {
        auto ir = (BltR4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BltR4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BltR4);
    }
    case OpCodeEnum::BltR8:
    {
        auto ir = (BltR8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BltR8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BltR8);
    }
    case OpCodeEnum::BneUnI4:
    {
        auto ir = (BneUnI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BneUnI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BneUnI4);
    }
    case OpCodeEnum::BneUnI8:
    {
        auto ir = (BneUnI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BneUnI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BneUnI8);
    }
    case OpCodeEnum::BneUnR4:
    {
        auto ir = (BneUnR4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BneUnR4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BneUnR4);
    }
    case OpCodeEnum::BneUnR8:
    {
        auto ir = (BneUnR8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BneUnR8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BneUnR8);
    }
    case OpCodeEnum::BgeUnI4:
    {
        auto ir = (BgeUnI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BgeUnI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BgeUnI4);
    }
    case OpCodeEnum::BgeUnI8:
    {
        auto ir = (BgeUnI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BgeUnI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BgeUnI8);
    }
    case OpCodeEnum::BgeUnR4:
    {
        auto ir = (BgeUnR4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BgeUnR4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BgeUnR4);
    }
    case OpCodeEnum::BgeUnR8:
    {
        auto ir = (BgeUnR8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BgeUnR8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BgeUnR8);
    }
    case OpCodeEnum::BgtUnI4:
    {
        auto ir = (BgtUnI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BgtUnI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BgtUnI4);
    }
    case OpCodeEnum::BgtUnI8:
    {
        auto ir = (BgtUnI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BgtUnI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BgtUnI8);
    }
    case OpCodeEnum::BgtUnR4:
    {
        auto ir = (BgtUnR4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BgtUnR4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BgtUnR4);
    }
    case OpCodeEnum::BgtUnR8:
    {
        auto ir = (BgtUnR8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BgtUnR8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BgtUnR8);
    }
    case OpCodeEnum::BleUnI4:
    {
        auto ir = (BleUnI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BleUnI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BleUnI4);
    }
    case OpCodeEnum::BleUnI8:
    {
        auto ir = (BleUnI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BleUnI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BleUnI8);
    }
    case OpCodeEnum::BleUnR4:
    {
        auto ir = (BleUnR4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BleUnR4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BleUnR4);
    }
    case OpCodeEnum::BleUnR8:
    {
        auto ir = (BleUnR8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BleUnR8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BleUnR8);
    }
    case OpCodeEnum::BltUnI4:
    {
        auto ir = (BltUnI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BltUnI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BltUnI4);
    }
    case OpCodeEnum::BltUnI8:
    {
        auto ir = (BltUnI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BltUnI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BltUnI8);
    }
    case OpCodeEnum::BltUnR4:
    {
        auto ir = (BltUnR4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BltUnR4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BltUnR4);
    }
    case OpCodeEnum::BltUnR8:
    {
        auto ir = (BltUnR8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BltUnR8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BltUnR8);
    }
    case OpCodeEnum::Switch:
    {
        auto ir = (Switch*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::Switch);
        ir->index = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->num_targets = (uint32_t)inst.get_num_targets();
        auto targetsInfo = inst.get_switch_targets();
        int32_t self_ir_offset = inst.get_ir_offset();
        int32_t* target_offsets = (int32_t*)(ir + 1);
        for (int i = 0; i < targetsInfo.count; i++)
        {
            target_offsets[i] = (int32_t)targetsInfo.targets[i]->ir_offset - self_ir_offset;
        }
        return codes + sizeof(Switch) + (targetsInfo.count * sizeof(int32_t));
    }
    case OpCodeEnum::LdIndI1:
    {
        auto ir = (LdIndI1*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdIndI1);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdIndI1);
    }
    case OpCodeEnum::LdIndU1:
    {
        auto ir = (LdIndU1*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdIndU1);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdIndU1);
    }
    case OpCodeEnum::LdIndI2:
    {
        auto ir = (LdIndI2*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdIndI2);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdIndI2);
    }
    case OpCodeEnum::LdIndI2Unaligned:
    {
        auto ir = (LdIndI2Unaligned*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdIndI2Unaligned);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdIndI2Unaligned);
    }
    case OpCodeEnum::LdIndU2:
    {
        auto ir = (LdIndU2*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdIndU2);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdIndU2);
    }
    case OpCodeEnum::LdIndU2Unaligned:
    {
        auto ir = (LdIndU2Unaligned*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdIndU2Unaligned);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdIndU2Unaligned);
    }
    case OpCodeEnum::LdIndI4:
    {
        auto ir = (LdIndI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdIndI4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdIndI4);
    }
    case OpCodeEnum::LdIndI4Unaligned:
    {
        auto ir = (LdIndI4Unaligned*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdIndI4Unaligned);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdIndI4Unaligned);
    }
    case OpCodeEnum::LdIndI8:
    {
        auto ir = (LdIndI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdIndI8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdIndI8);
    }
    case OpCodeEnum::LdIndI8Unaligned:
    {
        auto ir = (LdIndI8Unaligned*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdIndI8Unaligned);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdIndI8Unaligned);
    }
    case OpCodeEnum::StIndI1:
    {
        auto ir = (StIndI1*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StIndI1);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StIndI1);
    }
    case OpCodeEnum::StIndI2:
    {
        auto ir = (StIndI2*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StIndI2);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StIndI2);
    }
    case OpCodeEnum::StIndI2Unaligned:
    {
        auto ir = (StIndI2Unaligned*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StIndI2Unaligned);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StIndI2Unaligned);
    }
    case OpCodeEnum::StIndI4:
    {
        auto ir = (StIndI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StIndI4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StIndI4);
    }
    case OpCodeEnum::StIndI4Unaligned:
    {
        auto ir = (StIndI4Unaligned*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StIndI4Unaligned);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StIndI4Unaligned);
    }
    case OpCodeEnum::StIndI8:
    {
        auto ir = (StIndI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StIndI8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StIndI8);
    }
    case OpCodeEnum::StIndI8Unaligned:
    {
        auto ir = (StIndI8Unaligned*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StIndI8Unaligned);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StIndI8Unaligned);
    }
    case OpCodeEnum::StIndI8I4:
    {
        auto ir = (StIndI8I4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StIndI8I4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StIndI8I4);
    }
    case OpCodeEnum::StIndI8I4Unaligned:
    {
        auto ir = (StIndI8I4Unaligned*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StIndI8I4Unaligned);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StIndI8I4Unaligned);
    }
    case OpCodeEnum::StIndI8U4:
    {
        auto ir = (StIndI8U4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StIndI8U4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StIndI8U4);
    }
    case OpCodeEnum::StIndI8U4Unaligned:
    {
        auto ir = (StIndI8U4Unaligned*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StIndI8U4Unaligned);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StIndI8U4Unaligned);
    }
    case OpCodeEnum::AddI4:
    {
        auto ir = (AddI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::AddI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(AddI4);
    }
    case OpCodeEnum::AddI8:
    {
        auto ir = (AddI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::AddI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(AddI8);
    }
    case OpCodeEnum::AddR4:
    {
        auto ir = (AddR4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::AddR4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(AddR4);
    }
    case OpCodeEnum::AddR8:
    {
        auto ir = (AddR8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::AddR8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(AddR8);
    }
    case OpCodeEnum::SubI4:
    {
        auto ir = (SubI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::SubI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(SubI4);
    }
    case OpCodeEnum::SubI8:
    {
        auto ir = (SubI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::SubI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(SubI8);
    }
    case OpCodeEnum::SubR4:
    {
        auto ir = (SubR4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::SubR4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(SubR4);
    }
    case OpCodeEnum::SubR8:
    {
        auto ir = (SubR8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::SubR8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(SubR8);
    }
    case OpCodeEnum::MulI4:
    {
        auto ir = (MulI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::MulI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(MulI4);
    }
    case OpCodeEnum::MulI8:
    {
        auto ir = (MulI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::MulI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(MulI8);
    }
    case OpCodeEnum::MulR4:
    {
        auto ir = (MulR4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::MulR4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(MulR4);
    }
    case OpCodeEnum::MulR8:
    {
        auto ir = (MulR8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::MulR8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(MulR8);
    }
    case OpCodeEnum::DivI4:
    {
        auto ir = (DivI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::DivI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(DivI4);
    }
    case OpCodeEnum::DivI8:
    {
        auto ir = (DivI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::DivI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(DivI8);
    }
    case OpCodeEnum::DivR4:
    {
        auto ir = (DivR4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::DivR4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(DivR4);
    }
    case OpCodeEnum::DivR8:
    {
        auto ir = (DivR8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::DivR8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(DivR8);
    }
    case OpCodeEnum::DivUnI4:
    {
        auto ir = (DivUnI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::DivUnI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(DivUnI4);
    }
    case OpCodeEnum::DivUnI8:
    {
        auto ir = (DivUnI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::DivUnI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(DivUnI8);
    }
    case OpCodeEnum::RemI4:
    {
        auto ir = (RemI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::RemI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(RemI4);
    }
    case OpCodeEnum::RemI8:
    {
        auto ir = (RemI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::RemI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(RemI8);
    }
    case OpCodeEnum::RemR4:
    {
        auto ir = (RemR4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::RemR4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(RemR4);
    }
    case OpCodeEnum::RemR8:
    {
        auto ir = (RemR8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::RemR8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(RemR8);
    }
    case OpCodeEnum::RemUnI4:
    {
        auto ir = (RemUnI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::RemUnI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(RemUnI4);
    }
    case OpCodeEnum::RemUnI8:
    {
        auto ir = (RemUnI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::RemUnI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(RemUnI8);
    }
    case OpCodeEnum::AndI4:
    {
        auto ir = (AndI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::AndI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(AndI4);
    }
    case OpCodeEnum::AndI8:
    {
        auto ir = (AndI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::AndI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(AndI8);
    }
    case OpCodeEnum::OrI4:
    {
        auto ir = (OrI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::OrI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(OrI4);
    }
    case OpCodeEnum::OrI8:
    {
        auto ir = (OrI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::OrI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(OrI8);
    }
    case OpCodeEnum::XorI4:
    {
        auto ir = (XorI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::XorI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(XorI4);
    }
    case OpCodeEnum::XorI8:
    {
        auto ir = (XorI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::XorI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(XorI8);
    }
    case OpCodeEnum::ShlI4:
    {
        auto ir = (ShlI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ShlI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ShlI4);
    }
    case OpCodeEnum::ShlI8:
    {
        auto ir = (ShlI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ShlI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ShlI8);
    }
    case OpCodeEnum::ShrI4:
    {
        auto ir = (ShrI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ShrI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ShrI4);
    }
    case OpCodeEnum::ShrI8:
    {
        auto ir = (ShrI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ShrI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ShrI8);
    }
    case OpCodeEnum::ShrUnI4:
    {
        auto ir = (ShrUnI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ShrUnI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ShrUnI4);
    }
    case OpCodeEnum::ShrUnI8:
    {
        auto ir = (ShrUnI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ShrUnI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ShrUnI8);
    }
    case OpCodeEnum::NegI4:
    {
        auto ir = (NegI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::NegI4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(NegI4);
    }
    case OpCodeEnum::NegI8:
    {
        auto ir = (NegI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::NegI8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(NegI8);
    }
    case OpCodeEnum::NegR4:
    {
        auto ir = (NegR4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::NegR4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(NegR4);
    }
    case OpCodeEnum::NegR8:
    {
        auto ir = (NegR8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::NegR8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(NegR8);
    }
    case OpCodeEnum::NotI4:
    {
        auto ir = (NotI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::NotI4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(NotI4);
    }
    case OpCodeEnum::NotI8:
    {
        auto ir = (NotI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::NotI8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(NotI8);
    }
    case OpCodeEnum::AddOvfI4:
    {
        auto ir = (AddOvfI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::AddOvfI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(AddOvfI4);
    }
    case OpCodeEnum::AddOvfI8:
    {
        auto ir = (AddOvfI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::AddOvfI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(AddOvfI8);
    }
    case OpCodeEnum::AddOvfUnI4:
    {
        auto ir = (AddOvfUnI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::AddOvfUnI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(AddOvfUnI4);
    }
    case OpCodeEnum::AddOvfUnI8:
    {
        auto ir = (AddOvfUnI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::AddOvfUnI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(AddOvfUnI8);
    }
    case OpCodeEnum::MulOvfI4:
    {
        auto ir = (MulOvfI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::MulOvfI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(MulOvfI4);
    }
    case OpCodeEnum::MulOvfI8:
    {
        auto ir = (MulOvfI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::MulOvfI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(MulOvfI8);
    }
    case OpCodeEnum::MulOvfUnI4:
    {
        auto ir = (MulOvfUnI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::MulOvfUnI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(MulOvfUnI4);
    }
    case OpCodeEnum::MulOvfUnI8:
    {
        auto ir = (MulOvfUnI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::MulOvfUnI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(MulOvfUnI8);
    }
    case OpCodeEnum::SubOvfI4:
    {
        auto ir = (SubOvfI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::SubOvfI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(SubOvfI4);
    }
    case OpCodeEnum::SubOvfI8:
    {
        auto ir = (SubOvfI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::SubOvfI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(SubOvfI8);
    }
    case OpCodeEnum::SubOvfUnI4:
    {
        auto ir = (SubOvfUnI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::SubOvfUnI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(SubOvfUnI4);
    }
    case OpCodeEnum::SubOvfUnI8:
    {
        auto ir = (SubOvfUnI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::SubOvfUnI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(SubOvfUnI8);
    }
    case OpCodeEnum::ConvI1I4:
    {
        auto ir = (ConvI1I4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvI1I4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI1I4);
    }
    case OpCodeEnum::ConvI1I8:
    {
        auto ir = (ConvI1I8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvI1I8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI1I8);
    }
    case OpCodeEnum::ConvI1R4:
    {
        auto ir = (ConvI1R4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvI1R4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI1R4);
    }
    case OpCodeEnum::ConvI1R8:
    {
        auto ir = (ConvI1R8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvI1R8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI1R8);
    }
    case OpCodeEnum::ConvU1I4:
    {
        auto ir = (ConvU1I4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvU1I4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU1I4);
    }
    case OpCodeEnum::ConvU1I8:
    {
        auto ir = (ConvU1I8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvU1I8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU1I8);
    }
    case OpCodeEnum::ConvU1R4:
    {
        auto ir = (ConvU1R4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvU1R4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU1R4);
    }
    case OpCodeEnum::ConvU1R8:
    {
        auto ir = (ConvU1R8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvU1R8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU1R8);
    }
    case OpCodeEnum::ConvI2I4:
    {
        auto ir = (ConvI2I4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvI2I4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI2I4);
    }
    case OpCodeEnum::ConvI2I8:
    {
        auto ir = (ConvI2I8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvI2I8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI2I8);
    }
    case OpCodeEnum::ConvI2R4:
    {
        auto ir = (ConvI2R4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvI2R4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI2R4);
    }
    case OpCodeEnum::ConvI2R8:
    {
        auto ir = (ConvI2R8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvI2R8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI2R8);
    }
    case OpCodeEnum::ConvU2I4:
    {
        auto ir = (ConvU2I4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvU2I4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU2I4);
    }
    case OpCodeEnum::ConvU2I8:
    {
        auto ir = (ConvU2I8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvU2I8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU2I8);
    }
    case OpCodeEnum::ConvU2R4:
    {
        auto ir = (ConvU2R4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvU2R4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU2R4);
    }
    case OpCodeEnum::ConvU2R8:
    {
        auto ir = (ConvU2R8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvU2R8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU2R8);
    }
    case OpCodeEnum::ConvI4I8:
    {
        auto ir = (ConvI4I8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvI4I8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI4I8);
    }
    case OpCodeEnum::ConvI4R4:
    {
        auto ir = (ConvI4R4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvI4R4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI4R4);
    }
    case OpCodeEnum::ConvI4R8:
    {
        auto ir = (ConvI4R8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvI4R8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI4R8);
    }
    case OpCodeEnum::ConvU4I8:
    {
        auto ir = (ConvU4I8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvU4I8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU4I8);
    }
    case OpCodeEnum::ConvU4R4:
    {
        auto ir = (ConvU4R4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvU4R4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU4R4);
    }
    case OpCodeEnum::ConvU4R8:
    {
        auto ir = (ConvU4R8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvU4R8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU4R8);
    }
    case OpCodeEnum::ConvI8I4:
    {
        auto ir = (ConvI8I4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvI8I4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI8I4);
    }
    case OpCodeEnum::ConvI8U4:
    {
        auto ir = (ConvI8U4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvI8U4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI8U4);
    }
    case OpCodeEnum::ConvI8R4:
    {
        auto ir = (ConvI8R4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvI8R4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI8R4);
    }
    case OpCodeEnum::ConvI8R8:
    {
        auto ir = (ConvI8R8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvI8R8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI8R8);
    }
    case OpCodeEnum::ConvU8I4:
    {
        auto ir = (ConvU8I4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvU8I4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU8I4);
    }
    case OpCodeEnum::ConvU8R4:
    {
        auto ir = (ConvU8R4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvU8R4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU8R4);
    }
    case OpCodeEnum::ConvU8R8:
    {
        auto ir = (ConvU8R8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvU8R8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU8R8);
    }
    case OpCodeEnum::ConvR4I4:
    {
        auto ir = (ConvR4I4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvR4I4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvR4I4);
    }
    case OpCodeEnum::ConvR4I8:
    {
        auto ir = (ConvR4I8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvR4I8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvR4I8);
    }
    case OpCodeEnum::ConvR4R8:
    {
        auto ir = (ConvR4R8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvR4R8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvR4R8);
    }
    case OpCodeEnum::ConvR8I4:
    {
        auto ir = (ConvR8I4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvR8I4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvR8I4);
    }
    case OpCodeEnum::ConvR8I8:
    {
        auto ir = (ConvR8I8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvR8I8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvR8I8);
    }
    case OpCodeEnum::ConvR8R4:
    {
        auto ir = (ConvR8R4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvR8R4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvR8R4);
    }
    case OpCodeEnum::ConvOvfI1I4:
    {
        auto ir = (ConvOvfI1I4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfI1I4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI1I4);
    }
    case OpCodeEnum::ConvOvfI1I8:
    {
        auto ir = (ConvOvfI1I8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfI1I8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI1I8);
    }
    case OpCodeEnum::ConvOvfI1R4:
    {
        auto ir = (ConvOvfI1R4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfI1R4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI1R4);
    }
    case OpCodeEnum::ConvOvfI1R8:
    {
        auto ir = (ConvOvfI1R8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfI1R8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI1R8);
    }
    case OpCodeEnum::ConvOvfU1I4:
    {
        auto ir = (ConvOvfU1I4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfU1I4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU1I4);
    }
    case OpCodeEnum::ConvOvfU1I8:
    {
        auto ir = (ConvOvfU1I8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfU1I8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU1I8);
    }
    case OpCodeEnum::ConvOvfU1R4:
    {
        auto ir = (ConvOvfU1R4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfU1R4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU1R4);
    }
    case OpCodeEnum::ConvOvfU1R8:
    {
        auto ir = (ConvOvfU1R8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfU1R8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU1R8);
    }
    case OpCodeEnum::ConvOvfI2I4:
    {
        auto ir = (ConvOvfI2I4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfI2I4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI2I4);
    }
    case OpCodeEnum::ConvOvfI2I8:
    {
        auto ir = (ConvOvfI2I8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfI2I8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI2I8);
    }
    case OpCodeEnum::ConvOvfI2R4:
    {
        auto ir = (ConvOvfI2R4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfI2R4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI2R4);
    }
    case OpCodeEnum::ConvOvfI2R8:
    {
        auto ir = (ConvOvfI2R8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfI2R8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI2R8);
    }
    case OpCodeEnum::ConvOvfU2I4:
    {
        auto ir = (ConvOvfU2I4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfU2I4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU2I4);
    }
    case OpCodeEnum::ConvOvfU2I8:
    {
        auto ir = (ConvOvfU2I8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfU2I8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU2I8);
    }
    case OpCodeEnum::ConvOvfU2R4:
    {
        auto ir = (ConvOvfU2R4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfU2R4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU2R4);
    }
    case OpCodeEnum::ConvOvfU2R8:
    {
        auto ir = (ConvOvfU2R8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfU2R8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU2R8);
    }
    case OpCodeEnum::ConvOvfI4I8:
    {
        auto ir = (ConvOvfI4I8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfI4I8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI4I8);
    }
    case OpCodeEnum::ConvOvfI4R4:
    {
        auto ir = (ConvOvfI4R4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfI4R4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI4R4);
    }
    case OpCodeEnum::ConvOvfI4R8:
    {
        auto ir = (ConvOvfI4R8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfI4R8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI4R8);
    }
    case OpCodeEnum::ConvOvfU4I4:
    {
        auto ir = (ConvOvfU4I4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfU4I4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU4I4);
    }
    case OpCodeEnum::ConvOvfU4I8:
    {
        auto ir = (ConvOvfU4I8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfU4I8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU4I8);
    }
    case OpCodeEnum::ConvOvfU4R4:
    {
        auto ir = (ConvOvfU4R4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfU4R4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU4R4);
    }
    case OpCodeEnum::ConvOvfU4R8:
    {
        auto ir = (ConvOvfU4R8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfU4R8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU4R8);
    }
    case OpCodeEnum::ConvOvfI8R4:
    {
        auto ir = (ConvOvfI8R4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfI8R4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI8R4);
    }
    case OpCodeEnum::ConvOvfI8R8:
    {
        auto ir = (ConvOvfI8R8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfI8R8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI8R8);
    }
    case OpCodeEnum::ConvOvfU8I4:
    {
        auto ir = (ConvOvfU8I4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfU8I4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU8I4);
    }
    case OpCodeEnum::ConvOvfU8I8:
    {
        auto ir = (ConvOvfU8I8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfU8I8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU8I8);
    }
    case OpCodeEnum::ConvOvfU8R4:
    {
        auto ir = (ConvOvfU8R4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfU8R4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU8R4);
    }
    case OpCodeEnum::ConvOvfU8R8:
    {
        auto ir = (ConvOvfU8R8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfU8R8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU8R8);
    }
    case OpCodeEnum::ConvOvfI1UnI4:
    {
        auto ir = (ConvOvfI1UnI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfI1UnI4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI1UnI4);
    }
    case OpCodeEnum::ConvOvfI1UnI8:
    {
        auto ir = (ConvOvfI1UnI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfI1UnI8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI1UnI8);
    }
    case OpCodeEnum::ConvOvfI1UnR4:
    {
        auto ir = (ConvOvfI1UnR4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfI1UnR4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI1UnR4);
    }
    case OpCodeEnum::ConvOvfI1UnR8:
    {
        auto ir = (ConvOvfI1UnR8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfI1UnR8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI1UnR8);
    }
    case OpCodeEnum::ConvOvfU1UnI4:
    {
        auto ir = (ConvOvfU1UnI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfU1UnI4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU1UnI4);
    }
    case OpCodeEnum::ConvOvfU1UnI8:
    {
        auto ir = (ConvOvfU1UnI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfU1UnI8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU1UnI8);
    }
    case OpCodeEnum::ConvOvfU1UnR4:
    {
        auto ir = (ConvOvfU1UnR4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfU1UnR4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU1UnR4);
    }
    case OpCodeEnum::ConvOvfU1UnR8:
    {
        auto ir = (ConvOvfU1UnR8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfU1UnR8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU1UnR8);
    }
    case OpCodeEnum::ConvOvfI2UnI4:
    {
        auto ir = (ConvOvfI2UnI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfI2UnI4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI2UnI4);
    }
    case OpCodeEnum::ConvOvfI2UnI8:
    {
        auto ir = (ConvOvfI2UnI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfI2UnI8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI2UnI8);
    }
    case OpCodeEnum::ConvOvfI2UnR4:
    {
        auto ir = (ConvOvfI2UnR4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfI2UnR4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI2UnR4);
    }
    case OpCodeEnum::ConvOvfI2UnR8:
    {
        auto ir = (ConvOvfI2UnR8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfI2UnR8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI2UnR8);
    }
    case OpCodeEnum::ConvOvfU2UnI4:
    {
        auto ir = (ConvOvfU2UnI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfU2UnI4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU2UnI4);
    }
    case OpCodeEnum::ConvOvfU2UnI8:
    {
        auto ir = (ConvOvfU2UnI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfU2UnI8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU2UnI8);
    }
    case OpCodeEnum::ConvOvfU2UnR4:
    {
        auto ir = (ConvOvfU2UnR4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfU2UnR4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU2UnR4);
    }
    case OpCodeEnum::ConvOvfU2UnR8:
    {
        auto ir = (ConvOvfU2UnR8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfU2UnR8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU2UnR8);
    }
    case OpCodeEnum::ConvOvfI4UnI4:
    {
        auto ir = (ConvOvfI4UnI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfI4UnI4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI4UnI4);
    }
    case OpCodeEnum::ConvOvfI4UnI8:
    {
        auto ir = (ConvOvfI4UnI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfI4UnI8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI4UnI8);
    }
    case OpCodeEnum::ConvOvfI4UnR4:
    {
        auto ir = (ConvOvfI4UnR4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfI4UnR4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI4UnR4);
    }
    case OpCodeEnum::ConvOvfI4UnR8:
    {
        auto ir = (ConvOvfI4UnR8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfI4UnR8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI4UnR8);
    }
    case OpCodeEnum::ConvOvfU4UnI8:
    {
        auto ir = (ConvOvfU4UnI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfU4UnI8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU4UnI8);
    }
    case OpCodeEnum::ConvOvfU4UnR4:
    {
        auto ir = (ConvOvfU4UnR4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfU4UnR4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU4UnR4);
    }
    case OpCodeEnum::ConvOvfU4UnR8:
    {
        auto ir = (ConvOvfU4UnR8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfU4UnR8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU4UnR8);
    }
    case OpCodeEnum::ConvOvfI8UnI8:
    {
        auto ir = (ConvOvfI8UnI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfI8UnI8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI8UnI8);
    }
    case OpCodeEnum::ConvOvfI8UnR4:
    {
        auto ir = (ConvOvfI8UnR4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfI8UnR4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI8UnR4);
    }
    case OpCodeEnum::ConvOvfI8UnR8:
    {
        auto ir = (ConvOvfI8UnR8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfI8UnR8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI8UnR8);
    }
    case OpCodeEnum::ConvOvfU8UnR4:
    {
        auto ir = (ConvOvfU8UnR4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfU8UnR4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU8UnR4);
    }
    case OpCodeEnum::ConvOvfU8UnR8:
    {
        auto ir = (ConvOvfU8UnR8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::ConvOvfU8UnR8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU8UnR8);
    }
    case OpCodeEnum::CeqI4:
    {
        auto ir = (CeqI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CeqI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CeqI4);
    }
    case OpCodeEnum::CeqI8:
    {
        auto ir = (CeqI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CeqI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CeqI8);
    }
    case OpCodeEnum::CeqR4:
    {
        auto ir = (CeqR4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CeqR4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CeqR4);
    }
    case OpCodeEnum::CeqR8:
    {
        auto ir = (CeqR8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CeqR8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CeqR8);
    }
    case OpCodeEnum::CgtI4:
    {
        auto ir = (CgtI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CgtI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CgtI4);
    }
    case OpCodeEnum::CgtI8:
    {
        auto ir = (CgtI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CgtI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CgtI8);
    }
    case OpCodeEnum::CgtR4:
    {
        auto ir = (CgtR4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CgtR4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CgtR4);
    }
    case OpCodeEnum::CgtR8:
    {
        auto ir = (CgtR8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CgtR8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CgtR8);
    }
    case OpCodeEnum::CgtUnI4:
    {
        auto ir = (CgtUnI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CgtUnI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CgtUnI4);
    }
    case OpCodeEnum::CgtUnI8:
    {
        auto ir = (CgtUnI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CgtUnI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CgtUnI8);
    }
    case OpCodeEnum::CgtUnR4:
    {
        auto ir = (CgtUnR4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CgtUnR4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CgtUnR4);
    }
    case OpCodeEnum::CgtUnR8:
    {
        auto ir = (CgtUnR8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CgtUnR8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CgtUnR8);
    }
    case OpCodeEnum::CltI4:
    {
        auto ir = (CltI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CltI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CltI4);
    }
    case OpCodeEnum::CltI8:
    {
        auto ir = (CltI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CltI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CltI8);
    }
    case OpCodeEnum::CltR4:
    {
        auto ir = (CltR4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CltR4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CltR4);
    }
    case OpCodeEnum::CltR8:
    {
        auto ir = (CltR8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CltR8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CltR8);
    }
    case OpCodeEnum::CltUnI4:
    {
        auto ir = (CltUnI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CltUnI4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CltUnI4);
    }
    case OpCodeEnum::CltUnI8:
    {
        auto ir = (CltUnI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CltUnI8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CltUnI8);
    }
    case OpCodeEnum::CltUnR4:
    {
        auto ir = (CltUnR4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CltUnR4);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CltUnR4);
    }
    case OpCodeEnum::CltUnR8:
    {
        auto ir = (CltUnR8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CltUnR8);
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CltUnR8);
    }
    case OpCodeEnum::InitObjI1:
    {
        auto ir = (InitObjI1*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::InitObjI1);
        ir->addr = (uint16_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(InitObjI1);
    }
    case OpCodeEnum::InitObjI2:
    {
        auto ir = (InitObjI2*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::InitObjI2);
        ir->addr = (uint16_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(InitObjI2);
    }
    case OpCodeEnum::InitObjI2Unaligned:
    {
        auto ir = (InitObjI2Unaligned*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::InitObjI2Unaligned);
        ir->addr = (uint16_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(InitObjI2Unaligned);
    }
    case OpCodeEnum::InitObjI4:
    {
        auto ir = (InitObjI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::InitObjI4);
        ir->addr = (uint16_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(InitObjI4);
    }
    case OpCodeEnum::InitObjI4Unaligned:
    {
        auto ir = (InitObjI4Unaligned*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::InitObjI4Unaligned);
        ir->addr = (uint16_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(InitObjI4Unaligned);
    }
    case OpCodeEnum::InitObjI8:
    {
        auto ir = (InitObjI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::InitObjI8);
        ir->addr = (uint16_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(InitObjI8);
    }
    case OpCodeEnum::InitObjI8Unaligned:
    {
        auto ir = (InitObjI8Unaligned*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::InitObjI8Unaligned);
        ir->addr = (uint16_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(InitObjI8Unaligned);
    }
    case OpCodeEnum::InitObjAny:
    {
        auto ir = (InitObjAny*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::InitObjAny);
        ir->addr = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->size = (uint32_t)inst.get_size();
        return codes + sizeof(InitObjAny);
    }
    case OpCodeEnum::CpObjI1:
    {
        auto ir = (CpObjI1*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CpObjI1);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CpObjI1);
    }
    case OpCodeEnum::CpObjI2:
    {
        auto ir = (CpObjI2*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CpObjI2);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CpObjI2);
    }
    case OpCodeEnum::CpObjI4:
    {
        auto ir = (CpObjI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CpObjI4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CpObjI4);
    }
    case OpCodeEnum::CpObjI8:
    {
        auto ir = (CpObjI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CpObjI8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CpObjI8);
    }
    case OpCodeEnum::CpObjAny:
    {
        auto ir = (CpObjAny*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CpObjAny);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->size = (uint16_t)inst.get_size();
        return codes + sizeof(CpObjAny);
    }
    case OpCodeEnum::LdObjAny:
    {
        auto ir = (LdObjAny*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdObjAny);
        ir->addr = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->size = (uint16_t)inst.get_size();
        return codes + sizeof(LdObjAny);
    }
    case OpCodeEnum::StObjAny:
    {
        auto ir = (StObjAny*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StObjAny);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->addr = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->size = (uint16_t)inst.get_size();
        return codes + sizeof(StObjAny);
    }
    case OpCodeEnum::CastClass:
    {
        auto ir = (CastClass*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CastClass);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->klass_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(CastClass);
    }
    case OpCodeEnum::IsInst:
    {
        auto ir = (IsInst*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::IsInst);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->klass_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(IsInst);
    }
    case OpCodeEnum::Box:
    {
        auto ir = (Box*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::Box);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->klass_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(Box);
    }
    case OpCodeEnum::Unbox:
    {
        auto ir = (Unbox*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::Unbox);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->klass_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(Unbox);
    }
    case OpCodeEnum::UnboxAny:
    {
        auto ir = (UnboxAny*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::UnboxAny);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->klass_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(UnboxAny);
    }
    case OpCodeEnum::NewArr:
    {
        auto ir = (NewArr*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::NewArr);
        ir->length = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->arr_klass_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(NewArr);
    }
    case OpCodeEnum::LdLen:
    {
        auto ir = (LdLen*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdLen);
        ir->arr = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdLen);
    }
    case OpCodeEnum::Ldelema:
    {
        auto ir = (Ldelema*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::Ldelema);
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->ele_klass_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(Ldelema);
    }
    case OpCodeEnum::LdelemaReadOnly:
    {
        auto ir = (LdelemaReadOnly*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdelemaReadOnly);
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdelemaReadOnly);
    }
    case OpCodeEnum::LdelemI1:
    {
        auto ir = (LdelemI1*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdelemI1);
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdelemI1);
    }
    case OpCodeEnum::LdelemU1:
    {
        auto ir = (LdelemU1*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdelemU1);
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdelemU1);
    }
    case OpCodeEnum::LdelemI2:
    {
        auto ir = (LdelemI2*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdelemI2);
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdelemI2);
    }
    case OpCodeEnum::LdelemU2:
    {
        auto ir = (LdelemU2*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdelemU2);
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdelemU2);
    }
    case OpCodeEnum::LdelemI4:
    {
        auto ir = (LdelemI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdelemI4);
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdelemI4);
    }
    case OpCodeEnum::LdelemI8:
    {
        auto ir = (LdelemI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdelemI8);
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdelemI8);
    }
    case OpCodeEnum::LdelemI:
    {
        auto ir = (LdelemI*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdelemI);
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdelemI);
    }
    case OpCodeEnum::LdelemR4:
    {
        auto ir = (LdelemR4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdelemR4);
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdelemR4);
    }
    case OpCodeEnum::LdelemR8:
    {
        auto ir = (LdelemR8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdelemR8);
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdelemR8);
    }
    case OpCodeEnum::LdelemRef:
    {
        auto ir = (LdelemRef*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdelemRef);
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdelemRef);
    }
    case OpCodeEnum::LdelemAnyRef:
    {
        auto ir = (LdelemAnyRef*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdelemAnyRef);
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->ele_klass_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(LdelemAnyRef);
    }
    case OpCodeEnum::LdelemAnyVal:
    {
        auto ir = (LdelemAnyVal*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdelemAnyVal);
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->ele_klass_idx = (uint16_t)inst.get_resolved_data_index();
        ir->ele_size = (uint32_t)inst.get_element_size();
        return codes + sizeof(LdelemAnyVal);
    }
    case OpCodeEnum::StelemI1:
    {
        auto ir = (StelemI1*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StelemI1);
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg3_eval_stack_idx();
        return codes + sizeof(StelemI1);
    }
    case OpCodeEnum::StelemI2:
    {
        auto ir = (StelemI2*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StelemI2);
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg3_eval_stack_idx();
        return codes + sizeof(StelemI2);
    }
    case OpCodeEnum::StelemI4:
    {
        auto ir = (StelemI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StelemI4);
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg3_eval_stack_idx();
        return codes + sizeof(StelemI4);
    }
    case OpCodeEnum::StelemI8:
    {
        auto ir = (StelemI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StelemI8);
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg3_eval_stack_idx();
        return codes + sizeof(StelemI8);
    }
    case OpCodeEnum::StelemI:
    {
        auto ir = (StelemI*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StelemI);
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg3_eval_stack_idx();
        return codes + sizeof(StelemI);
    }
    case OpCodeEnum::StelemR4:
    {
        auto ir = (StelemR4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StelemR4);
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg3_eval_stack_idx();
        return codes + sizeof(StelemR4);
    }
    case OpCodeEnum::StelemR8:
    {
        auto ir = (StelemR8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StelemR8);
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg3_eval_stack_idx();
        return codes + sizeof(StelemR8);
    }
    case OpCodeEnum::StelemRef:
    {
        auto ir = (StelemRef*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StelemRef);
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg3_eval_stack_idx();
        return codes + sizeof(StelemRef);
    }
    case OpCodeEnum::StelemAnyRef:
    {
        auto ir = (StelemAnyRef*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StelemAnyRef);
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg3_eval_stack_idx();
        ir->ele_klass_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(StelemAnyRef);
    }
    case OpCodeEnum::StelemAnyVal:
    {
        auto ir = (StelemAnyVal*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StelemAnyVal);
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg3_eval_stack_idx();
        ir->ele_klass_idx = (uint16_t)inst.get_resolved_data_index();
        ir->ele_size = (uint32_t)inst.get_element_size();
        return codes + sizeof(StelemAnyVal);
    }
    case OpCodeEnum::MkRefAny:
    {
        auto ir = (MkRefAny*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::MkRefAny);
        ir->addr = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->klass_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(MkRefAny);
    }
    case OpCodeEnum::RefAnyVal:
    {
        auto ir = (RefAnyVal*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::RefAnyVal);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->klass_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(RefAnyVal);
    }
    case OpCodeEnum::RefAnyType:
    {
        auto ir = (RefAnyType*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::RefAnyType);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(RefAnyType);
    }
    case OpCodeEnum::LdToken:
    {
        auto ir = (LdToken*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdToken);
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->token_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(LdToken);
    }
    case OpCodeEnum::CkfiniteR4:
    {
        auto ir = (CkfiniteR4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CkfiniteR4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(CkfiniteR4);
    }
    case OpCodeEnum::CkfiniteR8:
    {
        auto ir = (CkfiniteR8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CkfiniteR8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(CkfiniteR8);
    }
    case OpCodeEnum::LocAlloc:
    {
        auto ir = (LocAlloc*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LocAlloc);
        ir->size = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LocAlloc);
    }
    case OpCodeEnum::InitBlk:
    {
        auto ir = (InitBlk*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::InitBlk);
        ir->addr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->size = (uint16_t)inst.get_var_arg3_eval_stack_idx();
        return codes + sizeof(InitBlk);
    }
    case OpCodeEnum::CpBlk:
    {
        auto ir = (CpBlk*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CpBlk);
        ir->dst = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->src = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->size = (uint16_t)inst.get_var_arg3_eval_stack_idx();
        return codes + sizeof(CpBlk);
    }
    case OpCodeEnum::Ldftn:
    {
        auto ir = (Ldftn*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::Ldftn);
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->method_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(Ldftn);
    }
    case OpCodeEnum::Ldvirtftn:
    {
        auto ir = (Ldvirtftn*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::Ldvirtftn);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->method_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(Ldvirtftn);
    }
    case OpCodeEnum::LdfldI1:
    {
        auto ir = (LdfldI1*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdfldI1);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdfldI1);
    }
    case OpCodeEnum::LdfldI1Large:
    {
        auto ir = (LdfldI1Large*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdfldI1Large);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(LdfldI1Large);
    }
    case OpCodeEnum::LdfldU1:
    {
        auto ir = (LdfldU1*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdfldU1);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdfldU1);
    }
    case OpCodeEnum::LdfldU1Large:
    {
        auto ir = (LdfldU1Large*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdfldU1Large);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(LdfldU1Large);
    }
    case OpCodeEnum::LdfldI2:
    {
        auto ir = (LdfldI2*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdfldI2);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdfldI2);
    }
    case OpCodeEnum::LdfldI2Large:
    {
        auto ir = (LdfldI2Large*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdfldI2Large);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(LdfldI2Large);
    }
    case OpCodeEnum::LdfldI2Unaligned:
    {
        auto ir = (LdfldI2Unaligned*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdfldI2Unaligned);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdfldI2Unaligned);
    }
    case OpCodeEnum::LdfldU2:
    {
        auto ir = (LdfldU2*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdfldU2);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdfldU2);
    }
    case OpCodeEnum::LdfldU2Large:
    {
        auto ir = (LdfldU2Large*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdfldU2Large);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(LdfldU2Large);
    }
    case OpCodeEnum::LdfldU2Unaligned:
    {
        auto ir = (LdfldU2Unaligned*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdfldU2Unaligned);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdfldU2Unaligned);
    }
    case OpCodeEnum::LdfldI4:
    {
        auto ir = (LdfldI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdfldI4);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdfldI4);
    }
    case OpCodeEnum::LdfldI4Large:
    {
        auto ir = (LdfldI4Large*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdfldI4Large);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(LdfldI4Large);
    }
    case OpCodeEnum::LdfldI4Unaligned:
    {
        auto ir = (LdfldI4Unaligned*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdfldI4Unaligned);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdfldI4Unaligned);
    }
    case OpCodeEnum::LdfldI8:
    {
        auto ir = (LdfldI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdfldI8);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdfldI8);
    }
    case OpCodeEnum::LdfldI8Large:
    {
        auto ir = (LdfldI8Large*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdfldI8Large);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(LdfldI8Large);
    }
    case OpCodeEnum::LdfldI8Unaligned:
    {
        auto ir = (LdfldI8Unaligned*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdfldI8Unaligned);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdfldI8Unaligned);
    }
    case OpCodeEnum::LdfldAny:
    {
        auto ir = (LdfldAny*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdfldAny);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        ir->size = (uint16_t)inst.get_field_size();
        return codes + sizeof(LdfldAny);
    }
    case OpCodeEnum::LdfldAnyLarge:
    {
        auto ir = (LdfldAnyLarge*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdfldAnyLarge);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        ir->size = (uint32_t)inst.get_field_size();
        return codes + sizeof(LdfldAnyLarge);
    }
    case OpCodeEnum::LdvfldI1:
    {
        auto ir = (LdvfldI1*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdvfldI1);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdvfldI1);
    }
    case OpCodeEnum::LdvfldI1Large:
    {
        auto ir = (LdvfldI1Large*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdvfldI1Large);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(LdvfldI1Large);
    }
    case OpCodeEnum::LdvfldU1:
    {
        auto ir = (LdvfldU1*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdvfldU1);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdvfldU1);
    }
    case OpCodeEnum::LdvfldU1Large:
    {
        auto ir = (LdvfldU1Large*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdvfldU1Large);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(LdvfldU1Large);
    }
    case OpCodeEnum::LdvfldI2:
    {
        auto ir = (LdvfldI2*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdvfldI2);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdvfldI2);
    }
    case OpCodeEnum::LdvfldI2Large:
    {
        auto ir = (LdvfldI2Large*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdvfldI2Large);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(LdvfldI2Large);
    }
    case OpCodeEnum::LdvfldI2Unaligned:
    {
        auto ir = (LdvfldI2Unaligned*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdvfldI2Unaligned);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdvfldI2Unaligned);
    }
    case OpCodeEnum::LdvfldU2:
    {
        auto ir = (LdvfldU2*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdvfldU2);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdvfldU2);
    }
    case OpCodeEnum::LdvfldU2Large:
    {
        auto ir = (LdvfldU2Large*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdvfldU2Large);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(LdvfldU2Large);
    }
    case OpCodeEnum::LdvfldU2Unaligned:
    {
        auto ir = (LdvfldU2Unaligned*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdvfldU2Unaligned);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdvfldU2Unaligned);
    }
    case OpCodeEnum::LdvfldI4:
    {
        auto ir = (LdvfldI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdvfldI4);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdvfldI4);
    }
    case OpCodeEnum::LdvfldI4Large:
    {
        auto ir = (LdvfldI4Large*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdvfldI4Large);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(LdvfldI4Large);
    }
    case OpCodeEnum::LdvfldI4Unaligned:
    {
        auto ir = (LdvfldI4Unaligned*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdvfldI4Unaligned);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdvfldI4Unaligned);
    }
    case OpCodeEnum::LdvfldI8:
    {
        auto ir = (LdvfldI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdvfldI8);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdvfldI8);
    }
    case OpCodeEnum::LdvfldI8Large:
    {
        auto ir = (LdvfldI8Large*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdvfldI8Large);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(LdvfldI8Large);
    }
    case OpCodeEnum::LdvfldI8Unaligned:
    {
        auto ir = (LdvfldI8Unaligned*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdvfldI8Unaligned);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdvfldI8Unaligned);
    }
    case OpCodeEnum::LdvfldAny:
    {
        auto ir = (LdvfldAny*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdvfldAny);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        ir->size = (uint16_t)inst.get_field_size();
        return codes + sizeof(LdvfldAny);
    }
    case OpCodeEnum::LdvfldAnyLarge:
    {
        auto ir = (LdvfldAnyLarge*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdvfldAnyLarge);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        ir->size = (uint32_t)inst.get_field_size();
        return codes + sizeof(LdvfldAnyLarge);
    }
    case OpCodeEnum::Ldflda:
    {
        auto ir = (Ldflda*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::Ldflda);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(Ldflda);
    }
    case OpCodeEnum::LdfldaLarge:
    {
        auto ir = (LdfldaLarge*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdfldaLarge);
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(LdfldaLarge);
    }
    case OpCodeEnum::StfldI1:
    {
        auto ir = (StfldI1*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StfldI1);
        ir->obj = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(StfldI1);
    }
    case OpCodeEnum::StfldI1Large:
    {
        auto ir = (StfldI1Large*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StfldI1Large);
        ir->obj = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(StfldI1Large);
    }
    case OpCodeEnum::StfldI2:
    {
        auto ir = (StfldI2*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StfldI2);
        ir->obj = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(StfldI2);
    }
    case OpCodeEnum::StfldI2Large:
    {
        auto ir = (StfldI2Large*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StfldI2Large);
        ir->obj = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(StfldI2Large);
    }
    case OpCodeEnum::StfldI2Unaligned:
    {
        auto ir = (StfldI2Unaligned*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StfldI2Unaligned);
        ir->obj = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(StfldI2Unaligned);
    }
    case OpCodeEnum::StfldI4:
    {
        auto ir = (StfldI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StfldI4);
        ir->obj = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(StfldI4);
    }
    case OpCodeEnum::StfldI4Large:
    {
        auto ir = (StfldI4Large*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StfldI4Large);
        ir->obj = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(StfldI4Large);
    }
    case OpCodeEnum::StfldI4Unaligned:
    {
        auto ir = (StfldI4Unaligned*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StfldI4Unaligned);
        ir->obj = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(StfldI4Unaligned);
    }
    case OpCodeEnum::StfldI8:
    {
        auto ir = (StfldI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StfldI8);
        ir->obj = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(StfldI8);
    }
    case OpCodeEnum::StfldI8Large:
    {
        auto ir = (StfldI8Large*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StfldI8Large);
        ir->obj = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(StfldI8Large);
    }
    case OpCodeEnum::StfldI8Unaligned:
    {
        auto ir = (StfldI8Unaligned*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StfldI8Unaligned);
        ir->obj = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(StfldI8Unaligned);
    }
    case OpCodeEnum::StfldAny:
    {
        auto ir = (StfldAny*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StfldAny);
        ir->obj = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        ir->size = (uint16_t)inst.get_field_size();
        return codes + sizeof(StfldAny);
    }
    case OpCodeEnum::StfldAnyLarge:
    {
        auto ir = (StfldAnyLarge*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StfldAnyLarge);
        ir->obj = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        ir->size = (uint32_t)inst.get_field_size();
        return codes + sizeof(StfldAnyLarge);
    }
    case OpCodeEnum::LdsfldI1:
    {
        auto ir = (LdsfldI1*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdsfldI1);
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->field_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(LdsfldI1);
    }
    case OpCodeEnum::LdsfldU1:
    {
        auto ir = (LdsfldU1*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdsfldU1);
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->field_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(LdsfldU1);
    }
    case OpCodeEnum::LdsfldI2:
    {
        auto ir = (LdsfldI2*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdsfldI2);
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->field_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(LdsfldI2);
    }
    case OpCodeEnum::LdsfldU2:
    {
        auto ir = (LdsfldU2*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdsfldU2);
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->field_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(LdsfldU2);
    }
    case OpCodeEnum::LdsfldI4:
    {
        auto ir = (LdsfldI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdsfldI4);
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->field_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(LdsfldI4);
    }
    case OpCodeEnum::LdsfldI8:
    {
        auto ir = (LdsfldI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdsfldI8);
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->field_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(LdsfldI8);
    }
    case OpCodeEnum::LdsfldAny:
    {
        auto ir = (LdsfldAny*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdsfldAny);
        ir->field_idx = (uint16_t)inst.get_resolved_data_index();
        ir->size = (uint16_t)inst.get_field_size();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdsfldAny);
    }
    case OpCodeEnum::Ldsflda:
    {
        auto ir = (Ldsflda*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::Ldsflda);
        ir->field_idx = (uint16_t)inst.get_resolved_data_index();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(Ldsflda);
    }
    case OpCodeEnum::LdsfldRvaData:
    {
        auto ir = (LdsfldRvaData*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LdsfldRvaData);
        ir->data = (uint16_t)inst.get_resolved_data_index();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdsfldRvaData);
    }
    case OpCodeEnum::StsfldI1:
    {
        auto ir = (StsfldI1*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StsfldI1);
        ir->field_idx = (uint16_t)inst.get_resolved_data_index();
        ir->value = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        return codes + sizeof(StsfldI1);
    }
    case OpCodeEnum::StsfldI2:
    {
        auto ir = (StsfldI2*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StsfldI2);
        ir->field_idx = (uint16_t)inst.get_resolved_data_index();
        ir->value = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        return codes + sizeof(StsfldI2);
    }
    case OpCodeEnum::StsfldI4:
    {
        auto ir = (StsfldI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StsfldI4);
        ir->field_idx = (uint16_t)inst.get_resolved_data_index();
        ir->value = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        return codes + sizeof(StsfldI4);
    }
    case OpCodeEnum::StsfldI8:
    {
        auto ir = (StsfldI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StsfldI8);
        ir->field_idx = (uint16_t)inst.get_resolved_data_index();
        ir->value = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        return codes + sizeof(StsfldI8);
    }
    case OpCodeEnum::StsfldAny:
    {
        auto ir = (StsfldAny*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::StsfldAny);
        ir->field_idx = (uint16_t)inst.get_resolved_data_index();
        ir->size = (uint16_t)inst.get_field_size();
        ir->value = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        return codes + sizeof(StsfldAny);
    }
    case OpCodeEnum::RetVoid:
    {
        auto ir = (RetVoid*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::RetVoid);
        return codes + sizeof(RetVoid);
    }
    case OpCodeEnum::RetI4:
    {
        auto ir = (RetI4*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::RetI4);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(RetI4);
    }
    case OpCodeEnum::RetI8:
    {
        auto ir = (RetI8*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::RetI8);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(RetI8);
    }
    case OpCodeEnum::RetAny:
    {
        auto ir = (RetAny*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::RetAny);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->size = (uint16_t)inst.get_size();
        return codes + sizeof(RetAny);
    }
    case OpCodeEnum::RetNop:
    {
        auto ir = (RetNop*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::RetNop);
        return codes + sizeof(RetNop);
    }
    case OpCodeEnum::CallInterp:
    {
        auto ir = (CallInterp*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CallInterp);
        ir->method_idx = (uint16_t)inst.get_resolved_data_index();
        ir->frame_base = (uint16_t)inst.get_frame_base();
        return codes + sizeof(CallInterp);
    }
    case OpCodeEnum::CallVirtInterp:
    {
        auto ir = (CallVirtInterp*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CallVirtInterp);
        ir->method_idx = (uint16_t)inst.get_resolved_data_index();
        ir->frame_base = (uint16_t)inst.get_frame_base();
        return codes + sizeof(CallVirtInterp);
    }
    case OpCodeEnum::CallInternalCall:
    {
        auto ir = (CallInternalCall*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CallInternalCall);
        ir->method_idx = (uint16_t)inst.get_resolved_data_index();
        ir->frame_base = (uint16_t)inst.get_frame_base();
        return codes + sizeof(CallInternalCall);
    }
    case OpCodeEnum::CallIntrinsic:
    {
        auto ir = (CallIntrinsic*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CallIntrinsic);
        ir->method_idx = (uint16_t)inst.get_resolved_data_index();
        ir->frame_base = (uint16_t)inst.get_frame_base();
        return codes + sizeof(CallIntrinsic);
    }
    case OpCodeEnum::CallPInvoke:
    {
        auto ir = (CallPInvoke*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CallPInvoke);
        ir->method_idx = (uint16_t)inst.get_resolved_data_index();
        ir->frame_base = (uint16_t)inst.get_frame_base();
        return codes + sizeof(CallPInvoke);
    }
    case OpCodeEnum::CallAot:
    {
        auto ir = (CallAot*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CallAot);
        ir->method_idx = (uint16_t)inst.get_resolved_data_index();
        ir->frame_base = (uint16_t)inst.get_frame_base();
        return codes + sizeof(CallAot);
    }
    case OpCodeEnum::CallRuntimeImplemented:
    {
        auto ir = (CallRuntimeImplemented*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CallRuntimeImplemented);
        ir->method_idx = (uint16_t)inst.get_resolved_data_index();
        ir->frame_base = (uint16_t)inst.get_frame_base();
        return codes + sizeof(CallRuntimeImplemented);
    }
    case OpCodeEnum::CalliInterp:
    {
        auto ir = (CalliInterp*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::CalliInterp);
        ir->method_sig_idx = (uint16_t)inst.get_resolved_data_index();
        ir->method_idx = (uint16_t)inst.get_var_arg3_eval_stack_idx();
        ir->frame_base = (uint16_t)inst.get_frame_base();
        return codes + sizeof(CalliInterp);
    }
    case OpCodeEnum::BoxRefInplace:
    {
        auto ir = (BoxRefInplace*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::BoxRefInplace);
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->klass_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(BoxRefInplace);
    }
    case OpCodeEnum::NewObjInterp:
    {
        auto ir = (NewObjInterp*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::NewObjInterp);
        ir->method_idx = (uint16_t)inst.get_resolved_data_index();
        ir->frame_base = (uint16_t)inst.get_frame_base();
        ir->total_params_stack_object_size = (uint32_t)inst.get_total_params_stack_object_size();
        return codes + sizeof(NewObjInterp);
    }
    case OpCodeEnum::NewValueTypeInterp:
    {
        auto ir = (NewValueTypeInterp*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::NewValueTypeInterp);
        ir->method_idx = (uint16_t)inst.get_resolved_data_index();
        ir->frame_base = (uint16_t)inst.get_frame_base();
        ir->total_params_stack_object_size = (uint32_t)inst.get_total_params_stack_object_size();
        return codes + sizeof(NewValueTypeInterp);
    }
    case OpCodeEnum::NewObjInternalCall:
    {
        auto ir = (NewObjInternalCall*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::NewObjInternalCall);
        ir->method_idx = (uint16_t)inst.get_resolved_data_index();
        ir->invoker_idx = (uint16_t)inst.get_invoker_idx();
        ir->frame_base = (uint16_t)inst.get_frame_base();
        return codes + sizeof(NewObjInternalCall);
    }
    case OpCodeEnum::NewObjIntrinsic:
    {
        auto ir = (NewObjIntrinsic*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::NewObjIntrinsic);
        ir->method_idx = (uint16_t)inst.get_resolved_data_index();
        ir->invoker_idx = (uint16_t)inst.get_invoker_idx();
        ir->frame_base = (uint16_t)inst.get_frame_base();
        return codes + sizeof(NewObjIntrinsic);
    }
    case OpCodeEnum::NewObjAot:
    {
        auto ir = (NewObjAot*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::NewObjAot);
        ir->method_idx = (uint16_t)inst.get_resolved_data_index();
        ir->frame_base = (uint16_t)inst.get_frame_base();
        ir->total_params_stack_object_size = (uint32_t)inst.get_total_params_stack_object_size();
        return codes + sizeof(NewObjAot);
    }
    case OpCodeEnum::NewValueTypeAot:
    {
        auto ir = (NewValueTypeAot*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::NewValueTypeAot);
        ir->method_idx = (uint16_t)inst.get_resolved_data_index();
        ir->frame_base = (uint16_t)inst.get_frame_base();
        ir->total_params_stack_object_size = (uint32_t)inst.get_total_params_stack_object_size();
        return codes + sizeof(NewValueTypeAot);
    }
    case OpCodeEnum::Throw:
    {
        auto ir = (Throw*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::Throw);
        ir->ex = (uint16_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(Throw);
    }
    case OpCodeEnum::Rethrow:
    {
        auto ir = (Rethrow*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::Rethrow);
        return codes + sizeof(Rethrow);
    }
    case OpCodeEnum::LeaveTryWithFinally:
    {
        auto ir = (LeaveTryWithFinally*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LeaveTryWithFinally);
        ir->first_finally_clause_index = (uint8_t)inst.get_first_finally_clause_index();
        ir->finally_clauses_count = (uint8_t)inst.get_finally_clauses_count();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(LeaveTryWithFinally);
    }
    case OpCodeEnum::LeaveCatchWithFinally:
    {
        auto ir = (LeaveCatchWithFinally*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LeaveCatchWithFinally);
        ir->first_finally_clause_index = (uint8_t)inst.get_first_finally_clause_index();
        ir->finally_clauses_count = (uint8_t)inst.get_finally_clauses_count();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(LeaveCatchWithFinally);
    }
    case OpCodeEnum::LeaveCatchWithoutFinally:
    {
        auto ir = (LeaveCatchWithoutFinally*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::LeaveCatchWithoutFinally);
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(LeaveCatchWithoutFinally);
    }
    case OpCodeEnum::EndFilter:
    {
        auto ir = (EndFilter*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::EndFilter);
        ir->cond = (uint16_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(EndFilter);
    }
    case OpCodeEnum::EndFinally:
    {
        auto ir = (EndFinally*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::EndFinally);
        return codes + sizeof(EndFinally);
    }
    case OpCodeEnum::EndFault:
    {
        auto ir = (EndFault*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::EndFault);
        return codes + sizeof(EndFault);
    }
    case OpCodeEnum::GetEnumLongHashCode:
    {
        auto ir = (GetEnumLongHashCode*)codes;
        ir->__code = static_cast<uint16_t>(OpCodeEnum::GetEnumLongHashCode);
        ir->value_ptr = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(GetEnumLongHashCode);
    }

    //}}LOW_LEVEL_INSTRUCTION_WRITE_TO_DATA_DATA
    default:
        assert(false && "Unhandled opcode in write_instruction_to_data");
        return nullptr;
    }
}

} // namespace ll
} // namespace interp
} // namespace leanclr
