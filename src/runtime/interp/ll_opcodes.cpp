#include "ll_opcodes.h"
#include "ll_transformer.h"

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
    sizeof(InitLocals1Short),
    sizeof(InitLocals2Short),
    sizeof(InitLocals3Short),
    sizeof(InitLocals4Short),
    sizeof(InitLocals),
    sizeof(InitLocalsShort),
    sizeof(Arglist),
    sizeof(LdLocI1),
    sizeof(LdLocU1),
    sizeof(LdLocI2),
    sizeof(LdLocU2),
    sizeof(LdLocI4),
    sizeof(LdLocI8),
    sizeof(LdLocAny),
    sizeof(LdLocI1Short),
    sizeof(LdLocU1Short),
    sizeof(LdLocI2Short),
    sizeof(LdLocU2Short),
    sizeof(LdLocI4Short),
    sizeof(LdLocI8Short),
    sizeof(LdLocAnyShort),
    sizeof(LdLoca),
    sizeof(LdLocaShort),
    sizeof(StLocI1),
    sizeof(StLocI2),
    sizeof(StLocI4),
    sizeof(StLocI8),
    sizeof(StLocAny),
    sizeof(StLocI1Short),
    sizeof(StLocI2Short),
    sizeof(StLocI4Short),
    sizeof(StLocI8Short),
    sizeof(StLocAnyShort),
    sizeof(LdNull),
    sizeof(LdNullShort),
    sizeof(LdcI4I2),
    sizeof(LdcI4I2Short),
    sizeof(LdcI4I4),
    sizeof(LdcI4I4Short),
    sizeof(LdcI8I2),
    sizeof(LdcI8I2Short),
    sizeof(LdcI8I4),
    sizeof(LdcI8I4Short),
    sizeof(LdcI8I8),
    sizeof(LdcI8I8Short),
    sizeof(LdStr),
    sizeof(LdStrShort),
    sizeof(Br),
    sizeof(BrShort),
    sizeof(BrTrueI4),
    sizeof(BrTrueI4Short),
    sizeof(BrTrueI8),
    sizeof(BrTrueI8Short),
    sizeof(BrFalseI4),
    sizeof(BrFalseI4Short),
    sizeof(BrFalseI8),
    sizeof(BrFalseI8Short),
    sizeof(BeqI4),
    sizeof(BeqI8),
    sizeof(BeqR4),
    sizeof(BeqR8),
    sizeof(BeqI4Short),
    sizeof(BeqI8Short),
    sizeof(BgeI4),
    sizeof(BgeI8),
    sizeof(BgeR4),
    sizeof(BgeR8),
    sizeof(BgeI4Short),
    sizeof(BgeI8Short),
    sizeof(BgtI4),
    sizeof(BgtI8),
    sizeof(BgtR4),
    sizeof(BgtR8),
    sizeof(BgtI4Short),
    sizeof(BgtI8Short),
    sizeof(BleI4),
    sizeof(BleI8),
    sizeof(BleR4),
    sizeof(BleR8),
    sizeof(BleI4Short),
    sizeof(BleI8Short),
    sizeof(BltI4),
    sizeof(BltI8),
    sizeof(BltR4),
    sizeof(BltR8),
    sizeof(BltI4Short),
    sizeof(BltI8Short),
    sizeof(BneUnI4),
    sizeof(BneUnI8),
    sizeof(BneUnR4),
    sizeof(BneUnR8),
    sizeof(BneUnI4Short),
    sizeof(BneUnI8Short),
    sizeof(BgeUnI4),
    sizeof(BgeUnI8),
    sizeof(BgeUnR4),
    sizeof(BgeUnR8),
    sizeof(BgeUnI4Short),
    sizeof(BgeUnI8Short),
    sizeof(BgtUnI4),
    sizeof(BgtUnI8),
    sizeof(BgtUnR4),
    sizeof(BgtUnR8),
    sizeof(BgtUnI4Short),
    sizeof(BgtUnI8Short),
    sizeof(BleUnI4),
    sizeof(BleUnI8),
    sizeof(BleUnR4),
    sizeof(BleUnR8),
    sizeof(BleUnI4Short),
    sizeof(BleUnI8Short),
    sizeof(BltUnI4),
    sizeof(BltUnI8),
    sizeof(BltUnR4),
    sizeof(BltUnR8),
    sizeof(BltUnI4Short),
    sizeof(BltUnI8Short),
    sizeof(Switch),
    sizeof(LdIndI1),
    sizeof(LdIndI1Short),
    sizeof(LdIndU1),
    sizeof(LdIndU1Short),
    sizeof(LdIndI2),
    sizeof(LdIndI2Short),
    sizeof(LdIndI2Unaligned),
    sizeof(LdIndU2),
    sizeof(LdIndU2Short),
    sizeof(LdIndU2Unaligned),
    sizeof(LdIndI4),
    sizeof(LdIndI4Short),
    sizeof(LdIndI4Unaligned),
    sizeof(LdIndI8),
    sizeof(LdIndI8Short),
    sizeof(LdIndI8Unaligned),
    sizeof(StIndI1),
    sizeof(StIndI1Short),
    sizeof(StIndI2),
    sizeof(StIndI2Short),
    sizeof(StIndI2Unaligned),
    sizeof(StIndI4),
    sizeof(StIndI4Short),
    sizeof(StIndI4Unaligned),
    sizeof(StIndI8),
    sizeof(StIndI8Short),
    sizeof(StIndI8Unaligned),
    sizeof(StIndI8I4),
    sizeof(StIndI8I4Short),
    sizeof(StIndI8I4Unaligned),
    sizeof(StIndI8U4),
    sizeof(StIndI8U4Short),
    sizeof(StIndI8U4Unaligned),
    sizeof(AddI4),
    sizeof(AddI8),
    sizeof(AddR4),
    sizeof(AddR8),
    sizeof(AddI4Short),
    sizeof(AddI8Short),
    sizeof(AddR4Short),
    sizeof(AddR8Short),
    sizeof(SubI4),
    sizeof(SubI8),
    sizeof(SubR4),
    sizeof(SubR8),
    sizeof(SubI4Short),
    sizeof(SubI8Short),
    sizeof(SubR4Short),
    sizeof(SubR8Short),
    sizeof(MulI4),
    sizeof(MulI8),
    sizeof(MulR4),
    sizeof(MulR8),
    sizeof(MulI4Short),
    sizeof(MulI8Short),
    sizeof(MulR4Short),
    sizeof(MulR8Short),
    sizeof(DivI4),
    sizeof(DivI8),
    sizeof(DivR4),
    sizeof(DivR8),
    sizeof(DivI4Short),
    sizeof(DivI8Short),
    sizeof(DivR4Short),
    sizeof(DivR8Short),
    sizeof(DivUnI4),
    sizeof(DivUnI8),
    sizeof(DivUnI4Short),
    sizeof(DivUnI8Short),
    sizeof(RemI4),
    sizeof(RemI8),
    sizeof(RemR4),
    sizeof(RemR8),
    sizeof(RemI4Short),
    sizeof(RemI8Short),
    sizeof(RemR4Short),
    sizeof(RemR8Short),
    sizeof(RemUnI4),
    sizeof(RemUnI8),
    sizeof(RemUnI4Short),
    sizeof(RemUnI8Short),
    sizeof(AndI4),
    sizeof(AndI8),
    sizeof(AndI4Short),
    sizeof(AndI8Short),
    sizeof(OrI4),
    sizeof(OrI8),
    sizeof(OrI4Short),
    sizeof(OrI8Short),
    sizeof(XorI4),
    sizeof(XorI8),
    sizeof(XorI4Short),
    sizeof(XorI8Short),
    sizeof(ShlI4),
    sizeof(ShlI8),
    sizeof(ShlI4Short),
    sizeof(ShrI4),
    sizeof(ShrI8),
    sizeof(ShrI4Short),
    sizeof(ShrUnI4),
    sizeof(ShrUnI8),
    sizeof(ShrUnI4Short),
    sizeof(NegI4),
    sizeof(NegI8),
    sizeof(NegR4),
    sizeof(NegR8),
    sizeof(NegI4Short),
    sizeof(NegI8Short),
    sizeof(NegR4Short),
    sizeof(NegR8Short),
    sizeof(NotI4),
    sizeof(NotI8),
    sizeof(NotI4Short),
    sizeof(NotI8Short),
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
    sizeof(ConvI1I4Short),
    sizeof(ConvI1I8Short),
    sizeof(ConvI1R4Short),
    sizeof(ConvI1R8Short),
    sizeof(ConvU1I4),
    sizeof(ConvU1I8),
    sizeof(ConvU1R4),
    sizeof(ConvU1R8),
    sizeof(ConvU1I4Short),
    sizeof(ConvU1I8Short),
    sizeof(ConvU1R4Short),
    sizeof(ConvU1R8Short),
    sizeof(ConvI2I4),
    sizeof(ConvI2I8),
    sizeof(ConvI2R4),
    sizeof(ConvI2R8),
    sizeof(ConvI2I4Short),
    sizeof(ConvI2I8Short),
    sizeof(ConvI2R4Short),
    sizeof(ConvI2R8Short),
    sizeof(ConvU2I4),
    sizeof(ConvU2I8),
    sizeof(ConvU2R4),
    sizeof(ConvU2R8),
    sizeof(ConvU2I4Short),
    sizeof(ConvU2I8Short),
    sizeof(ConvU2R4Short),
    sizeof(ConvU2R8Short),
    sizeof(ConvI4I8),
    sizeof(ConvI4R4),
    sizeof(ConvI4R8),
    sizeof(ConvI4I8Short),
    sizeof(ConvI4R4Short),
    sizeof(ConvI4R8Short),
    sizeof(ConvU4I8),
    sizeof(ConvU4R4),
    sizeof(ConvU4R8),
    sizeof(ConvU4I8Short),
    sizeof(ConvU4R4Short),
    sizeof(ConvU4R8Short),
    sizeof(ConvI8I4),
    sizeof(ConvI8U4),
    sizeof(ConvI8R4),
    sizeof(ConvI8R8),
    sizeof(ConvI8I4Short),
    sizeof(ConvI8R4Short),
    sizeof(ConvI8R8Short),
    sizeof(ConvU8I4),
    sizeof(ConvU8R4),
    sizeof(ConvU8R8),
    sizeof(ConvR4I4),
    sizeof(ConvR4I8),
    sizeof(ConvR4R8),
    sizeof(ConvR4I4Short),
    sizeof(ConvR4I8Short),
    sizeof(ConvR4R8Short),
    sizeof(ConvR8I4),
    sizeof(ConvR8I8),
    sizeof(ConvR8R4),
    sizeof(ConvR8I4Short),
    sizeof(ConvR8I8Short),
    sizeof(ConvR8R4Short),
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
    sizeof(CeqI4Short),
    sizeof(CeqI8Short),
    sizeof(CeqR4Short),
    sizeof(CeqR8Short),
    sizeof(CgtI4),
    sizeof(CgtI8),
    sizeof(CgtR4),
    sizeof(CgtR8),
    sizeof(CgtI4Short),
    sizeof(CgtI8Short),
    sizeof(CgtUnI4),
    sizeof(CgtUnI8),
    sizeof(CgtUnR4),
    sizeof(CgtUnR8),
    sizeof(CgtUnI4Short),
    sizeof(CgtUnI8Short),
    sizeof(CltI4),
    sizeof(CltI8),
    sizeof(CltR4),
    sizeof(CltR8),
    sizeof(CltI4Short),
    sizeof(CltI8Short),
    sizeof(CltUnI4),
    sizeof(CltUnI8),
    sizeof(CltUnR4),
    sizeof(CltUnR8),
    sizeof(CltUnI4Short),
    sizeof(CltUnI8Short),
    sizeof(InitObjI1),
    sizeof(InitObjI1Short),
    sizeof(InitObjI2),
    sizeof(InitObjI2Short),
    sizeof(InitObjI2Unaligned),
    sizeof(InitObjI4),
    sizeof(InitObjI4Short),
    sizeof(InitObjI4Unaligned),
    sizeof(InitObjI8),
    sizeof(InitObjI8Short),
    sizeof(InitObjI8Unaligned),
    sizeof(InitObjAny),
    sizeof(InitObjAnyShort),
    sizeof(CpObjI1),
    sizeof(CpObjI1Short),
    sizeof(CpObjI2),
    sizeof(CpObjI2Short),
    sizeof(CpObjI4),
    sizeof(CpObjI4Short),
    sizeof(CpObjI8),
    sizeof(CpObjI8Short),
    sizeof(CpObjAny),
    sizeof(CpObjAnyShort),
    sizeof(LdObjAny),
    sizeof(LdObjAnyShort),
    sizeof(StObjAny),
    sizeof(StObjAnyShort),
    sizeof(CastClass),
    sizeof(CastClassShort),
    sizeof(IsInst),
    sizeof(IsInstShort),
    sizeof(Box),
    sizeof(BoxShort),
    sizeof(Unbox),
    sizeof(UnboxShort),
    sizeof(UnboxAny),
    sizeof(UnboxAnyShort),
    sizeof(NewArr),
    sizeof(NewArrShort),
    sizeof(LdLen),
    sizeof(LdLenShort),
    sizeof(Ldelema),
    sizeof(LdelemaShort),
    sizeof(LdelemaReadOnly),
    sizeof(LdelemI1),
    sizeof(LdelemI1Short),
    sizeof(LdelemU1),
    sizeof(LdelemU1Short),
    sizeof(LdelemI2),
    sizeof(LdelemI2Short),
    sizeof(LdelemU2),
    sizeof(LdelemU2Short),
    sizeof(LdelemI4),
    sizeof(LdelemI4Short),
    sizeof(LdelemI8),
    sizeof(LdelemI8Short),
    sizeof(LdelemI),
    sizeof(LdelemIShort),
    sizeof(LdelemR4),
    sizeof(LdelemR4Short),
    sizeof(LdelemR8),
    sizeof(LdelemR8Short),
    sizeof(LdelemRef),
    sizeof(LdelemRefShort),
    sizeof(LdelemAnyRef),
    sizeof(LdelemAnyRefShort),
    sizeof(LdelemAnyVal),
    sizeof(LdelemAnyValShort),
    sizeof(StelemI1),
    sizeof(StelemI1Short),
    sizeof(StelemI2),
    sizeof(StelemI2Short),
    sizeof(StelemI4),
    sizeof(StelemI4Short),
    sizeof(StelemI8),
    sizeof(StelemI8Short),
    sizeof(StelemI),
    sizeof(StelemIShort),
    sizeof(StelemR4),
    sizeof(StelemR4Short),
    sizeof(StelemR8),
    sizeof(StelemR8Short),
    sizeof(StelemRef),
    sizeof(StelemRefShort),
    sizeof(StelemAnyRef),
    sizeof(StelemAnyRefShort),
    sizeof(StelemAnyVal),
    sizeof(StelemAnyValShort),
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
    sizeof(LdftnShort),
    sizeof(Ldvirtftn),
    sizeof(LdvirtftnShort),
    sizeof(LdfldI1),
    sizeof(LdfldI1Short),
    sizeof(LdfldI1Large),
    sizeof(LdfldU1),
    sizeof(LdfldU1Short),
    sizeof(LdfldU1Large),
    sizeof(LdfldI2),
    sizeof(LdfldI2Short),
    sizeof(LdfldI2Large),
    sizeof(LdfldI2Unaligned),
    sizeof(LdfldU2),
    sizeof(LdfldU2Short),
    sizeof(LdfldU2Large),
    sizeof(LdfldU2Unaligned),
    sizeof(LdfldI4),
    sizeof(LdfldI4Short),
    sizeof(LdfldI4Large),
    sizeof(LdfldI4Unaligned),
    sizeof(LdfldI8),
    sizeof(LdfldI8Short),
    sizeof(LdfldI8Large),
    sizeof(LdfldI8Unaligned),
    sizeof(LdfldAny),
    sizeof(LdfldAnyShort),
    sizeof(LdfldAnyLarge),
    sizeof(LdvfldI1),
    sizeof(LdvfldI1Short),
    sizeof(LdvfldI1Large),
    sizeof(LdvfldU1),
    sizeof(LdvfldU1Short),
    sizeof(LdvfldU1Large),
    sizeof(LdvfldI2),
    sizeof(LdvfldI2Short),
    sizeof(LdvfldI2Large),
    sizeof(LdvfldI2Unaligned),
    sizeof(LdvfldU2),
    sizeof(LdvfldU2Short),
    sizeof(LdvfldU2Large),
    sizeof(LdvfldU2Unaligned),
    sizeof(LdvfldI4),
    sizeof(LdvfldI4Short),
    sizeof(LdvfldI4Large),
    sizeof(LdvfldI4Unaligned),
    sizeof(LdvfldI8),
    sizeof(LdvfldI8Short),
    sizeof(LdvfldI8Large),
    sizeof(LdvfldI8Unaligned),
    sizeof(LdvfldAny),
    sizeof(LdvfldAnyShort),
    sizeof(LdvfldAnyLarge),
    sizeof(Ldflda),
    sizeof(LdfldaShort),
    sizeof(LdfldaLarge),
    sizeof(StfldI1),
    sizeof(StfldI1Short),
    sizeof(StfldI1Large),
    sizeof(StfldI2),
    sizeof(StfldI2Short),
    sizeof(StfldI2Large),
    sizeof(StfldI2Unaligned),
    sizeof(StfldI4),
    sizeof(StfldI4Short),
    sizeof(StfldI4Large),
    sizeof(StfldI4Unaligned),
    sizeof(StfldI8),
    sizeof(StfldI8Short),
    sizeof(StfldI8Large),
    sizeof(StfldI8Unaligned),
    sizeof(StfldAny),
    sizeof(StfldAnyShort),
    sizeof(StfldAnyLarge),
    sizeof(LdsfldI1),
    sizeof(LdsfldI1Short),
    sizeof(LdsfldU1),
    sizeof(LdsfldU1Short),
    sizeof(LdsfldI2),
    sizeof(LdsfldI2Short),
    sizeof(LdsfldU2),
    sizeof(LdsfldU2Short),
    sizeof(LdsfldI4),
    sizeof(LdsfldI4Short),
    sizeof(LdsfldI8),
    sizeof(LdsfldI8Short),
    sizeof(LdsfldAny),
    sizeof(LdsfldAnyShort),
    sizeof(Ldsflda),
    sizeof(LdsfldaShort),
    sizeof(LdsfldRvaData),
    sizeof(LdsfldRvaDataShort),
    sizeof(StsfldI1),
    sizeof(StsfldI1Short),
    sizeof(StsfldI2),
    sizeof(StsfldI2Short),
    sizeof(StsfldI4),
    sizeof(StsfldI4Short),
    sizeof(StsfldI8),
    sizeof(StsfldI8Short),
    sizeof(StsfldAny),
    sizeof(StsfldAnyShort),
    sizeof(RetVoid),
    sizeof(RetVoidShort),
    sizeof(RetI4),
    sizeof(RetI8),
    sizeof(RetAny),
    sizeof(RetI4Short),
    sizeof(RetI8Short),
    sizeof(RetAnyShort),
    sizeof(RetNopShort),
    sizeof(CallInterp),
    sizeof(CallInterpShort),
    sizeof(CallVirtInterp),
    sizeof(CallVirtInterpShort),
    sizeof(CallInternalCall),
    sizeof(CallInternalCallShort),
    sizeof(CallIntrinsic),
    sizeof(CallIntrinsicShort),
    sizeof(CallPInvoke),
    sizeof(CallPInvokeShort),
    sizeof(CallAot),
    sizeof(CallAotShort),
    sizeof(CallRuntimeImplemented),
    sizeof(CallRuntimeImplementedShort),
    sizeof(CalliInterp),
    sizeof(CalliInterpShort),
    sizeof(BoxRefInplace),
    sizeof(BoxRefInplaceShort),
    sizeof(NewObjInterp),
    sizeof(NewObjInterpShort),
    sizeof(NewValueTypeInterp),
    sizeof(NewValueTypeInterpShort),
    sizeof(NewObjInternalCall),
    sizeof(NewObjInternalCallShort),
    sizeof(NewObjIntrinsic),
    sizeof(NewObjIntrinsicShort),
    sizeof(NewObjAot),
    sizeof(NewObjAotShort),
    sizeof(NewValueTypeAot),
    sizeof(NewValueTypeAotShort),
    sizeof(Throw),
    sizeof(ThrowShort),
    sizeof(Rethrow),
    sizeof(RethrowShort),
    sizeof(LeaveTryWithFinally),
    sizeof(LeaveTryWithFinallyShort),
    sizeof(LeaveCatchWithFinally),
    sizeof(LeaveCatchWithFinallyShort),
    sizeof(LeaveCatchWithoutFinally),
    sizeof(LeaveCatchWithoutFinallyShort),
    sizeof(EndFilter),
    sizeof(EndFilterShort),
    sizeof(EndFinally),
    sizeof(EndFinallyShort),
    sizeof(EndFault),
    sizeof(EndFaultShort),
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
        ir->__prefix = 254;
        ir->__code = 0;
        return codes + sizeof(Illegal);
    }
    case OpCodeEnum::Nop:
    {
        auto ir = (Nop*)codes;
        ir->__prefix = 254;
        ir->__code = 1;
        return codes + sizeof(Nop);
    }
    case OpCodeEnum::InitLocals1Short:
    {
        auto ir = (InitLocals1Short*)codes;
        ir->__code = 0;
        ir->offset = (uint8_t)inst.get_locals_offset();
        return codes + sizeof(InitLocals1Short);
    }
    case OpCodeEnum::InitLocals2Short:
    {
        auto ir = (InitLocals2Short*)codes;
        ir->__code = 1;
        ir->offset = (uint8_t)inst.get_locals_offset();
        return codes + sizeof(InitLocals2Short);
    }
    case OpCodeEnum::InitLocals3Short:
    {
        auto ir = (InitLocals3Short*)codes;
        ir->__code = 2;
        ir->offset = (uint8_t)inst.get_locals_offset();
        return codes + sizeof(InitLocals3Short);
    }
    case OpCodeEnum::InitLocals4Short:
    {
        auto ir = (InitLocals4Short*)codes;
        ir->__code = 3;
        ir->offset = (uint8_t)inst.get_locals_offset();
        return codes + sizeof(InitLocals4Short);
    }
    case OpCodeEnum::InitLocals:
    {
        auto ir = (InitLocals*)codes;
        ir->__prefix = 251;
        ir->__code = 0;
        ir->offset = (uint16_t)inst.get_locals_offset();
        ir->size = (uint16_t)inst.get_size();
        return codes + sizeof(InitLocals);
    }
    case OpCodeEnum::InitLocalsShort:
    {
        auto ir = (InitLocalsShort*)codes;
        ir->__code = 4;
        ir->offset = (uint8_t)inst.get_locals_offset();
        ir->size = (uint8_t)inst.get_size();
        return codes + sizeof(InitLocalsShort);
    }
    case OpCodeEnum::Arglist:
    {
        auto ir = (Arglist*)codes;
        ir->__prefix = 254;
        ir->__code = 2;
        return codes + sizeof(Arglist);
    }
    case OpCodeEnum::LdLocI1:
    {
        auto ir = (LdLocI1*)codes;
        ir->__prefix = 251;
        ir->__code = 1;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdLocI1);
    }
    case OpCodeEnum::LdLocU1:
    {
        auto ir = (LdLocU1*)codes;
        ir->__prefix = 251;
        ir->__code = 2;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdLocU1);
    }
    case OpCodeEnum::LdLocI2:
    {
        auto ir = (LdLocI2*)codes;
        ir->__prefix = 251;
        ir->__code = 3;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdLocI2);
    }
    case OpCodeEnum::LdLocU2:
    {
        auto ir = (LdLocU2*)codes;
        ir->__prefix = 251;
        ir->__code = 4;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdLocU2);
    }
    case OpCodeEnum::LdLocI4:
    {
        auto ir = (LdLocI4*)codes;
        ir->__prefix = 251;
        ir->__code = 5;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdLocI4);
    }
    case OpCodeEnum::LdLocI8:
    {
        auto ir = (LdLocI8*)codes;
        ir->__prefix = 251;
        ir->__code = 6;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdLocI8);
    }
    case OpCodeEnum::LdLocAny:
    {
        auto ir = (LdLocAny*)codes;
        ir->__prefix = 251;
        ir->__code = 7;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->size = (uint16_t)inst.get_size();
        return codes + sizeof(LdLocAny);
    }
    case OpCodeEnum::LdLocI1Short:
    {
        auto ir = (LdLocI1Short*)codes;
        ir->__code = 5;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdLocI1Short);
    }
    case OpCodeEnum::LdLocU1Short:
    {
        auto ir = (LdLocU1Short*)codes;
        ir->__code = 6;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdLocU1Short);
    }
    case OpCodeEnum::LdLocI2Short:
    {
        auto ir = (LdLocI2Short*)codes;
        ir->__code = 7;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdLocI2Short);
    }
    case OpCodeEnum::LdLocU2Short:
    {
        auto ir = (LdLocU2Short*)codes;
        ir->__code = 8;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdLocU2Short);
    }
    case OpCodeEnum::LdLocI4Short:
    {
        auto ir = (LdLocI4Short*)codes;
        ir->__code = 9;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdLocI4Short);
    }
    case OpCodeEnum::LdLocI8Short:
    {
        auto ir = (LdLocI8Short*)codes;
        ir->__code = 10;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdLocI8Short);
    }
    case OpCodeEnum::LdLocAnyShort:
    {
        auto ir = (LdLocAnyShort*)codes;
        ir->__code = 11;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->size = (uint8_t)inst.get_size();
        return codes + sizeof(LdLocAnyShort);
    }
    case OpCodeEnum::LdLoca:
    {
        auto ir = (LdLoca*)codes;
        ir->__prefix = 251;
        ir->__code = 8;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdLoca);
    }
    case OpCodeEnum::LdLocaShort:
    {
        auto ir = (LdLocaShort*)codes;
        ir->__code = 12;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdLocaShort);
    }
    case OpCodeEnum::StLocI1:
    {
        auto ir = (StLocI1*)codes;
        ir->__prefix = 251;
        ir->__code = 9;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StLocI1);
    }
    case OpCodeEnum::StLocI2:
    {
        auto ir = (StLocI2*)codes;
        ir->__prefix = 251;
        ir->__code = 10;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StLocI2);
    }
    case OpCodeEnum::StLocI4:
    {
        auto ir = (StLocI4*)codes;
        ir->__prefix = 251;
        ir->__code = 11;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StLocI4);
    }
    case OpCodeEnum::StLocI8:
    {
        auto ir = (StLocI8*)codes;
        ir->__prefix = 251;
        ir->__code = 12;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StLocI8);
    }
    case OpCodeEnum::StLocAny:
    {
        auto ir = (StLocAny*)codes;
        ir->__prefix = 251;
        ir->__code = 13;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->size = (uint16_t)inst.get_size();
        return codes + sizeof(StLocAny);
    }
    case OpCodeEnum::StLocI1Short:
    {
        auto ir = (StLocI1Short*)codes;
        ir->__code = 13;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StLocI1Short);
    }
    case OpCodeEnum::StLocI2Short:
    {
        auto ir = (StLocI2Short*)codes;
        ir->__code = 14;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StLocI2Short);
    }
    case OpCodeEnum::StLocI4Short:
    {
        auto ir = (StLocI4Short*)codes;
        ir->__code = 15;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StLocI4Short);
    }
    case OpCodeEnum::StLocI8Short:
    {
        auto ir = (StLocI8Short*)codes;
        ir->__code = 16;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StLocI8Short);
    }
    case OpCodeEnum::StLocAnyShort:
    {
        auto ir = (StLocAnyShort*)codes;
        ir->__code = 17;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->size = (uint8_t)inst.get_size();
        return codes + sizeof(StLocAnyShort);
    }
    case OpCodeEnum::LdNull:
    {
        auto ir = (LdNull*)codes;
        ir->__prefix = 251;
        ir->__code = 14;
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdNull);
    }
    case OpCodeEnum::LdNullShort:
    {
        auto ir = (LdNullShort*)codes;
        ir->__code = 18;
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdNullShort);
    }
    case OpCodeEnum::LdcI4I2:
    {
        auto ir = (LdcI4I2*)codes;
        ir->__prefix = 251;
        ir->__code = 15;
        ir->value = (int16_t)inst.get_i2();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdcI4I2);
    }
    case OpCodeEnum::LdcI4I2Short:
    {
        auto ir = (LdcI4I2Short*)codes;
        ir->__code = 19;
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->value = (int16_t)inst.get_i2();
        return codes + sizeof(LdcI4I2Short);
    }
    case OpCodeEnum::LdcI4I4:
    {
        auto ir = (LdcI4I4*)codes;
        ir->__prefix = 251;
        ir->__code = 16;
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->value = (int32_t)inst.get_i4();
        return codes + sizeof(LdcI4I4);
    }
    case OpCodeEnum::LdcI4I4Short:
    {
        auto ir = (LdcI4I4Short*)codes;
        ir->__code = 20;
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->value = (int32_t)inst.get_i4();
        return codes + sizeof(LdcI4I4Short);
    }
    case OpCodeEnum::LdcI8I2:
    {
        auto ir = (LdcI8I2*)codes;
        ir->__prefix = 251;
        ir->__code = 17;
        ir->value = (int16_t)inst.get_i2();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdcI8I2);
    }
    case OpCodeEnum::LdcI8I2Short:
    {
        auto ir = (LdcI8I2Short*)codes;
        ir->__code = 21;
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->value = (int16_t)inst.get_i2();
        return codes + sizeof(LdcI8I2Short);
    }
    case OpCodeEnum::LdcI8I4:
    {
        auto ir = (LdcI8I4*)codes;
        ir->__prefix = 251;
        ir->__code = 18;
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->value = (int32_t)inst.get_i4();
        return codes + sizeof(LdcI8I4);
    }
    case OpCodeEnum::LdcI8I4Short:
    {
        auto ir = (LdcI8I4Short*)codes;
        ir->__code = 22;
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->value = (int32_t)inst.get_i4();
        return codes + sizeof(LdcI8I4Short);
    }
    case OpCodeEnum::LdcI8I8:
    {
        auto ir = (LdcI8I8*)codes;
        ir->__prefix = 251;
        ir->__code = 19;
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->value_low = (int32_t)inst.get_i8_low();
        ir->value_high = (int32_t)inst.get_i8_high();
        return codes + sizeof(LdcI8I8);
    }
    case OpCodeEnum::LdcI8I8Short:
    {
        auto ir = (LdcI8I8Short*)codes;
        ir->__code = 23;
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->value_low = (int32_t)inst.get_i8_low();
        ir->value_high = (int32_t)inst.get_i8_high();
        return codes + sizeof(LdcI8I8Short);
    }
    case OpCodeEnum::LdStr:
    {
        auto ir = (LdStr*)codes;
        ir->__prefix = 251;
        ir->__code = 20;
        ir->str_idx = (uint16_t)inst.get_resolved_data_index();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdStr);
    }
    case OpCodeEnum::LdStrShort:
    {
        auto ir = (LdStrShort*)codes;
        ir->__code = 24;
        ir->str_idx = (uint8_t)inst.get_resolved_data_index();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdStrShort);
    }
    case OpCodeEnum::Br:
    {
        auto ir = (Br*)codes;
        ir->__prefix = 251;
        ir->__code = 21;
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(Br);
    }
    case OpCodeEnum::BrShort:
    {
        auto ir = (BrShort*)codes;
        ir->__code = 25;
        ir->target_offset = (int8_t)inst.get_branch_target_offset();
        return codes + sizeof(BrShort);
    }
    case OpCodeEnum::BrTrueI4:
    {
        auto ir = (BrTrueI4*)codes;
        ir->__prefix = 251;
        ir->__code = 22;
        ir->condition = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BrTrueI4);
    }
    case OpCodeEnum::BrTrueI4Short:
    {
        auto ir = (BrTrueI4Short*)codes;
        ir->__code = 26;
        ir->condition = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->target_offset = (int8_t)inst.get_branch_target_offset();
        return codes + sizeof(BrTrueI4Short);
    }
    case OpCodeEnum::BrTrueI8:
    {
        auto ir = (BrTrueI8*)codes;
        ir->__prefix = 251;
        ir->__code = 23;
        ir->condition = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BrTrueI8);
    }
    case OpCodeEnum::BrTrueI8Short:
    {
        auto ir = (BrTrueI8Short*)codes;
        ir->__code = 27;
        ir->condition = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->target_offset = (int8_t)inst.get_branch_target_offset();
        return codes + sizeof(BrTrueI8Short);
    }
    case OpCodeEnum::BrFalseI4:
    {
        auto ir = (BrFalseI4*)codes;
        ir->__prefix = 251;
        ir->__code = 24;
        ir->condition = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BrFalseI4);
    }
    case OpCodeEnum::BrFalseI4Short:
    {
        auto ir = (BrFalseI4Short*)codes;
        ir->__code = 28;
        ir->condition = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->target_offset = (int8_t)inst.get_branch_target_offset();
        return codes + sizeof(BrFalseI4Short);
    }
    case OpCodeEnum::BrFalseI8:
    {
        auto ir = (BrFalseI8*)codes;
        ir->__prefix = 251;
        ir->__code = 25;
        ir->condition = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BrFalseI8);
    }
    case OpCodeEnum::BrFalseI8Short:
    {
        auto ir = (BrFalseI8Short*)codes;
        ir->__code = 29;
        ir->condition = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->target_offset = (int8_t)inst.get_branch_target_offset();
        return codes + sizeof(BrFalseI8Short);
    }
    case OpCodeEnum::BeqI4:
    {
        auto ir = (BeqI4*)codes;
        ir->__prefix = 251;
        ir->__code = 26;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BeqI4);
    }
    case OpCodeEnum::BeqI8:
    {
        auto ir = (BeqI8*)codes;
        ir->__prefix = 251;
        ir->__code = 27;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BeqI8);
    }
    case OpCodeEnum::BeqR4:
    {
        auto ir = (BeqR4*)codes;
        ir->__prefix = 251;
        ir->__code = 28;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BeqR4);
    }
    case OpCodeEnum::BeqR8:
    {
        auto ir = (BeqR8*)codes;
        ir->__prefix = 251;
        ir->__code = 29;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BeqR8);
    }
    case OpCodeEnum::BeqI4Short:
    {
        auto ir = (BeqI4Short*)codes;
        ir->__code = 30;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int8_t)inst.get_branch_target_offset();
        return codes + sizeof(BeqI4Short);
    }
    case OpCodeEnum::BeqI8Short:
    {
        auto ir = (BeqI8Short*)codes;
        ir->__code = 31;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int8_t)inst.get_branch_target_offset();
        return codes + sizeof(BeqI8Short);
    }
    case OpCodeEnum::BgeI4:
    {
        auto ir = (BgeI4*)codes;
        ir->__prefix = 251;
        ir->__code = 30;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BgeI4);
    }
    case OpCodeEnum::BgeI8:
    {
        auto ir = (BgeI8*)codes;
        ir->__prefix = 251;
        ir->__code = 31;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BgeI8);
    }
    case OpCodeEnum::BgeR4:
    {
        auto ir = (BgeR4*)codes;
        ir->__prefix = 251;
        ir->__code = 32;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BgeR4);
    }
    case OpCodeEnum::BgeR8:
    {
        auto ir = (BgeR8*)codes;
        ir->__prefix = 251;
        ir->__code = 33;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BgeR8);
    }
    case OpCodeEnum::BgeI4Short:
    {
        auto ir = (BgeI4Short*)codes;
        ir->__code = 32;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int8_t)inst.get_branch_target_offset();
        return codes + sizeof(BgeI4Short);
    }
    case OpCodeEnum::BgeI8Short:
    {
        auto ir = (BgeI8Short*)codes;
        ir->__code = 33;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int8_t)inst.get_branch_target_offset();
        return codes + sizeof(BgeI8Short);
    }
    case OpCodeEnum::BgtI4:
    {
        auto ir = (BgtI4*)codes;
        ir->__prefix = 251;
        ir->__code = 34;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BgtI4);
    }
    case OpCodeEnum::BgtI8:
    {
        auto ir = (BgtI8*)codes;
        ir->__prefix = 251;
        ir->__code = 35;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BgtI8);
    }
    case OpCodeEnum::BgtR4:
    {
        auto ir = (BgtR4*)codes;
        ir->__prefix = 251;
        ir->__code = 36;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BgtR4);
    }
    case OpCodeEnum::BgtR8:
    {
        auto ir = (BgtR8*)codes;
        ir->__prefix = 251;
        ir->__code = 37;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BgtR8);
    }
    case OpCodeEnum::BgtI4Short:
    {
        auto ir = (BgtI4Short*)codes;
        ir->__code = 34;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int8_t)inst.get_branch_target_offset();
        return codes + sizeof(BgtI4Short);
    }
    case OpCodeEnum::BgtI8Short:
    {
        auto ir = (BgtI8Short*)codes;
        ir->__code = 35;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int8_t)inst.get_branch_target_offset();
        return codes + sizeof(BgtI8Short);
    }
    case OpCodeEnum::BleI4:
    {
        auto ir = (BleI4*)codes;
        ir->__prefix = 251;
        ir->__code = 38;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BleI4);
    }
    case OpCodeEnum::BleI8:
    {
        auto ir = (BleI8*)codes;
        ir->__prefix = 251;
        ir->__code = 39;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BleI8);
    }
    case OpCodeEnum::BleR4:
    {
        auto ir = (BleR4*)codes;
        ir->__prefix = 251;
        ir->__code = 40;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BleR4);
    }
    case OpCodeEnum::BleR8:
    {
        auto ir = (BleR8*)codes;
        ir->__prefix = 251;
        ir->__code = 41;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BleR8);
    }
    case OpCodeEnum::BleI4Short:
    {
        auto ir = (BleI4Short*)codes;
        ir->__code = 36;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int8_t)inst.get_branch_target_offset();
        return codes + sizeof(BleI4Short);
    }
    case OpCodeEnum::BleI8Short:
    {
        auto ir = (BleI8Short*)codes;
        ir->__code = 37;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int8_t)inst.get_branch_target_offset();
        return codes + sizeof(BleI8Short);
    }
    case OpCodeEnum::BltI4:
    {
        auto ir = (BltI4*)codes;
        ir->__prefix = 251;
        ir->__code = 42;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BltI4);
    }
    case OpCodeEnum::BltI8:
    {
        auto ir = (BltI8*)codes;
        ir->__prefix = 251;
        ir->__code = 43;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BltI8);
    }
    case OpCodeEnum::BltR4:
    {
        auto ir = (BltR4*)codes;
        ir->__prefix = 251;
        ir->__code = 44;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BltR4);
    }
    case OpCodeEnum::BltR8:
    {
        auto ir = (BltR8*)codes;
        ir->__prefix = 251;
        ir->__code = 45;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BltR8);
    }
    case OpCodeEnum::BltI4Short:
    {
        auto ir = (BltI4Short*)codes;
        ir->__code = 38;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int8_t)inst.get_branch_target_offset();
        return codes + sizeof(BltI4Short);
    }
    case OpCodeEnum::BltI8Short:
    {
        auto ir = (BltI8Short*)codes;
        ir->__code = 39;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int8_t)inst.get_branch_target_offset();
        return codes + sizeof(BltI8Short);
    }
    case OpCodeEnum::BneUnI4:
    {
        auto ir = (BneUnI4*)codes;
        ir->__prefix = 251;
        ir->__code = 46;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BneUnI4);
    }
    case OpCodeEnum::BneUnI8:
    {
        auto ir = (BneUnI8*)codes;
        ir->__prefix = 251;
        ir->__code = 47;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BneUnI8);
    }
    case OpCodeEnum::BneUnR4:
    {
        auto ir = (BneUnR4*)codes;
        ir->__prefix = 251;
        ir->__code = 48;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BneUnR4);
    }
    case OpCodeEnum::BneUnR8:
    {
        auto ir = (BneUnR8*)codes;
        ir->__prefix = 251;
        ir->__code = 49;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BneUnR8);
    }
    case OpCodeEnum::BneUnI4Short:
    {
        auto ir = (BneUnI4Short*)codes;
        ir->__code = 40;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int8_t)inst.get_branch_target_offset();
        return codes + sizeof(BneUnI4Short);
    }
    case OpCodeEnum::BneUnI8Short:
    {
        auto ir = (BneUnI8Short*)codes;
        ir->__code = 41;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int8_t)inst.get_branch_target_offset();
        return codes + sizeof(BneUnI8Short);
    }
    case OpCodeEnum::BgeUnI4:
    {
        auto ir = (BgeUnI4*)codes;
        ir->__prefix = 251;
        ir->__code = 50;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BgeUnI4);
    }
    case OpCodeEnum::BgeUnI8:
    {
        auto ir = (BgeUnI8*)codes;
        ir->__prefix = 251;
        ir->__code = 51;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BgeUnI8);
    }
    case OpCodeEnum::BgeUnR4:
    {
        auto ir = (BgeUnR4*)codes;
        ir->__prefix = 251;
        ir->__code = 52;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BgeUnR4);
    }
    case OpCodeEnum::BgeUnR8:
    {
        auto ir = (BgeUnR8*)codes;
        ir->__prefix = 251;
        ir->__code = 53;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BgeUnR8);
    }
    case OpCodeEnum::BgeUnI4Short:
    {
        auto ir = (BgeUnI4Short*)codes;
        ir->__code = 42;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int8_t)inst.get_branch_target_offset();
        return codes + sizeof(BgeUnI4Short);
    }
    case OpCodeEnum::BgeUnI8Short:
    {
        auto ir = (BgeUnI8Short*)codes;
        ir->__code = 43;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int8_t)inst.get_branch_target_offset();
        return codes + sizeof(BgeUnI8Short);
    }
    case OpCodeEnum::BgtUnI4:
    {
        auto ir = (BgtUnI4*)codes;
        ir->__prefix = 251;
        ir->__code = 54;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BgtUnI4);
    }
    case OpCodeEnum::BgtUnI8:
    {
        auto ir = (BgtUnI8*)codes;
        ir->__prefix = 251;
        ir->__code = 55;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BgtUnI8);
    }
    case OpCodeEnum::BgtUnR4:
    {
        auto ir = (BgtUnR4*)codes;
        ir->__prefix = 251;
        ir->__code = 56;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BgtUnR4);
    }
    case OpCodeEnum::BgtUnR8:
    {
        auto ir = (BgtUnR8*)codes;
        ir->__prefix = 251;
        ir->__code = 57;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BgtUnR8);
    }
    case OpCodeEnum::BgtUnI4Short:
    {
        auto ir = (BgtUnI4Short*)codes;
        ir->__code = 44;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int8_t)inst.get_branch_target_offset();
        return codes + sizeof(BgtUnI4Short);
    }
    case OpCodeEnum::BgtUnI8Short:
    {
        auto ir = (BgtUnI8Short*)codes;
        ir->__code = 45;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int8_t)inst.get_branch_target_offset();
        return codes + sizeof(BgtUnI8Short);
    }
    case OpCodeEnum::BleUnI4:
    {
        auto ir = (BleUnI4*)codes;
        ir->__prefix = 251;
        ir->__code = 58;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BleUnI4);
    }
    case OpCodeEnum::BleUnI8:
    {
        auto ir = (BleUnI8*)codes;
        ir->__prefix = 251;
        ir->__code = 59;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BleUnI8);
    }
    case OpCodeEnum::BleUnR4:
    {
        auto ir = (BleUnR4*)codes;
        ir->__prefix = 251;
        ir->__code = 60;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BleUnR4);
    }
    case OpCodeEnum::BleUnR8:
    {
        auto ir = (BleUnR8*)codes;
        ir->__prefix = 251;
        ir->__code = 61;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BleUnR8);
    }
    case OpCodeEnum::BleUnI4Short:
    {
        auto ir = (BleUnI4Short*)codes;
        ir->__code = 46;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int8_t)inst.get_branch_target_offset();
        return codes + sizeof(BleUnI4Short);
    }
    case OpCodeEnum::BleUnI8Short:
    {
        auto ir = (BleUnI8Short*)codes;
        ir->__code = 47;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int8_t)inst.get_branch_target_offset();
        return codes + sizeof(BleUnI8Short);
    }
    case OpCodeEnum::BltUnI4:
    {
        auto ir = (BltUnI4*)codes;
        ir->__prefix = 251;
        ir->__code = 62;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BltUnI4);
    }
    case OpCodeEnum::BltUnI8:
    {
        auto ir = (BltUnI8*)codes;
        ir->__prefix = 251;
        ir->__code = 63;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BltUnI8);
    }
    case OpCodeEnum::BltUnR4:
    {
        auto ir = (BltUnR4*)codes;
        ir->__prefix = 251;
        ir->__code = 64;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BltUnR4);
    }
    case OpCodeEnum::BltUnR8:
    {
        auto ir = (BltUnR8*)codes;
        ir->__prefix = 251;
        ir->__code = 65;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(BltUnR8);
    }
    case OpCodeEnum::BltUnI4Short:
    {
        auto ir = (BltUnI4Short*)codes;
        ir->__code = 48;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int8_t)inst.get_branch_target_offset();
        return codes + sizeof(BltUnI4Short);
    }
    case OpCodeEnum::BltUnI8Short:
    {
        auto ir = (BltUnI8Short*)codes;
        ir->__code = 49;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->target_offset = (int8_t)inst.get_branch_target_offset();
        return codes + sizeof(BltUnI8Short);
    }
    case OpCodeEnum::Switch:
    {
        auto ir = (Switch*)codes;
        ir->__prefix = 251;
        ir->__code = 66;
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
        ir->__prefix = 252;
        ir->__code = 0;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdIndI1);
    }
    case OpCodeEnum::LdIndI1Short:
    {
        auto ir = (LdIndI1Short*)codes;
        ir->__prefix = 251;
        ir->__code = 67;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdIndI1Short);
    }
    case OpCodeEnum::LdIndU1:
    {
        auto ir = (LdIndU1*)codes;
        ir->__prefix = 252;
        ir->__code = 1;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdIndU1);
    }
    case OpCodeEnum::LdIndU1Short:
    {
        auto ir = (LdIndU1Short*)codes;
        ir->__prefix = 251;
        ir->__code = 68;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdIndU1Short);
    }
    case OpCodeEnum::LdIndI2:
    {
        auto ir = (LdIndI2*)codes;
        ir->__prefix = 252;
        ir->__code = 2;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdIndI2);
    }
    case OpCodeEnum::LdIndI2Short:
    {
        auto ir = (LdIndI2Short*)codes;
        ir->__prefix = 251;
        ir->__code = 69;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdIndI2Short);
    }
    case OpCodeEnum::LdIndI2Unaligned:
    {
        auto ir = (LdIndI2Unaligned*)codes;
        ir->__prefix = 253;
        ir->__code = 0;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdIndI2Unaligned);
    }
    case OpCodeEnum::LdIndU2:
    {
        auto ir = (LdIndU2*)codes;
        ir->__prefix = 252;
        ir->__code = 3;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdIndU2);
    }
    case OpCodeEnum::LdIndU2Short:
    {
        auto ir = (LdIndU2Short*)codes;
        ir->__prefix = 251;
        ir->__code = 70;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdIndU2Short);
    }
    case OpCodeEnum::LdIndU2Unaligned:
    {
        auto ir = (LdIndU2Unaligned*)codes;
        ir->__prefix = 253;
        ir->__code = 1;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdIndU2Unaligned);
    }
    case OpCodeEnum::LdIndI4:
    {
        auto ir = (LdIndI4*)codes;
        ir->__prefix = 252;
        ir->__code = 4;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdIndI4);
    }
    case OpCodeEnum::LdIndI4Short:
    {
        auto ir = (LdIndI4Short*)codes;
        ir->__prefix = 251;
        ir->__code = 71;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdIndI4Short);
    }
    case OpCodeEnum::LdIndI4Unaligned:
    {
        auto ir = (LdIndI4Unaligned*)codes;
        ir->__prefix = 253;
        ir->__code = 2;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdIndI4Unaligned);
    }
    case OpCodeEnum::LdIndI8:
    {
        auto ir = (LdIndI8*)codes;
        ir->__prefix = 252;
        ir->__code = 5;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdIndI8);
    }
    case OpCodeEnum::LdIndI8Short:
    {
        auto ir = (LdIndI8Short*)codes;
        ir->__prefix = 251;
        ir->__code = 72;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdIndI8Short);
    }
    case OpCodeEnum::LdIndI8Unaligned:
    {
        auto ir = (LdIndI8Unaligned*)codes;
        ir->__prefix = 253;
        ir->__code = 3;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdIndI8Unaligned);
    }
    case OpCodeEnum::StIndI1:
    {
        auto ir = (StIndI1*)codes;
        ir->__prefix = 252;
        ir->__code = 6;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StIndI1);
    }
    case OpCodeEnum::StIndI1Short:
    {
        auto ir = (StIndI1Short*)codes;
        ir->__prefix = 251;
        ir->__code = 73;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StIndI1Short);
    }
    case OpCodeEnum::StIndI2:
    {
        auto ir = (StIndI2*)codes;
        ir->__prefix = 252;
        ir->__code = 7;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StIndI2);
    }
    case OpCodeEnum::StIndI2Short:
    {
        auto ir = (StIndI2Short*)codes;
        ir->__prefix = 251;
        ir->__code = 74;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StIndI2Short);
    }
    case OpCodeEnum::StIndI2Unaligned:
    {
        auto ir = (StIndI2Unaligned*)codes;
        ir->__prefix = 253;
        ir->__code = 4;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StIndI2Unaligned);
    }
    case OpCodeEnum::StIndI4:
    {
        auto ir = (StIndI4*)codes;
        ir->__prefix = 252;
        ir->__code = 8;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StIndI4);
    }
    case OpCodeEnum::StIndI4Short:
    {
        auto ir = (StIndI4Short*)codes;
        ir->__prefix = 251;
        ir->__code = 75;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StIndI4Short);
    }
    case OpCodeEnum::StIndI4Unaligned:
    {
        auto ir = (StIndI4Unaligned*)codes;
        ir->__prefix = 253;
        ir->__code = 5;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StIndI4Unaligned);
    }
    case OpCodeEnum::StIndI8:
    {
        auto ir = (StIndI8*)codes;
        ir->__prefix = 252;
        ir->__code = 9;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StIndI8);
    }
    case OpCodeEnum::StIndI8Short:
    {
        auto ir = (StIndI8Short*)codes;
        ir->__prefix = 251;
        ir->__code = 76;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StIndI8Short);
    }
    case OpCodeEnum::StIndI8Unaligned:
    {
        auto ir = (StIndI8Unaligned*)codes;
        ir->__prefix = 253;
        ir->__code = 6;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StIndI8Unaligned);
    }
    case OpCodeEnum::StIndI8I4:
    {
        auto ir = (StIndI8I4*)codes;
        ir->__prefix = 252;
        ir->__code = 10;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StIndI8I4);
    }
    case OpCodeEnum::StIndI8I4Short:
    {
        auto ir = (StIndI8I4Short*)codes;
        ir->__prefix = 251;
        ir->__code = 77;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StIndI8I4Short);
    }
    case OpCodeEnum::StIndI8I4Unaligned:
    {
        auto ir = (StIndI8I4Unaligned*)codes;
        ir->__prefix = 253;
        ir->__code = 7;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StIndI8I4Unaligned);
    }
    case OpCodeEnum::StIndI8U4:
    {
        auto ir = (StIndI8U4*)codes;
        ir->__prefix = 252;
        ir->__code = 11;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StIndI8U4);
    }
    case OpCodeEnum::StIndI8U4Short:
    {
        auto ir = (StIndI8U4Short*)codes;
        ir->__prefix = 251;
        ir->__code = 78;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StIndI8U4Short);
    }
    case OpCodeEnum::StIndI8U4Unaligned:
    {
        auto ir = (StIndI8U4Unaligned*)codes;
        ir->__prefix = 253;
        ir->__code = 8;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(StIndI8U4Unaligned);
    }
    case OpCodeEnum::AddI4:
    {
        auto ir = (AddI4*)codes;
        ir->__prefix = 251;
        ir->__code = 79;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(AddI4);
    }
    case OpCodeEnum::AddI8:
    {
        auto ir = (AddI8*)codes;
        ir->__prefix = 251;
        ir->__code = 80;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(AddI8);
    }
    case OpCodeEnum::AddR4:
    {
        auto ir = (AddR4*)codes;
        ir->__prefix = 251;
        ir->__code = 81;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(AddR4);
    }
    case OpCodeEnum::AddR8:
    {
        auto ir = (AddR8*)codes;
        ir->__prefix = 251;
        ir->__code = 82;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(AddR8);
    }
    case OpCodeEnum::AddI4Short:
    {
        auto ir = (AddI4Short*)codes;
        ir->__code = 50;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(AddI4Short);
    }
    case OpCodeEnum::AddI8Short:
    {
        auto ir = (AddI8Short*)codes;
        ir->__code = 51;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(AddI8Short);
    }
    case OpCodeEnum::AddR4Short:
    {
        auto ir = (AddR4Short*)codes;
        ir->__code = 52;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(AddR4Short);
    }
    case OpCodeEnum::AddR8Short:
    {
        auto ir = (AddR8Short*)codes;
        ir->__code = 53;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(AddR8Short);
    }
    case OpCodeEnum::SubI4:
    {
        auto ir = (SubI4*)codes;
        ir->__prefix = 251;
        ir->__code = 83;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(SubI4);
    }
    case OpCodeEnum::SubI8:
    {
        auto ir = (SubI8*)codes;
        ir->__prefix = 251;
        ir->__code = 84;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(SubI8);
    }
    case OpCodeEnum::SubR4:
    {
        auto ir = (SubR4*)codes;
        ir->__prefix = 251;
        ir->__code = 85;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(SubR4);
    }
    case OpCodeEnum::SubR8:
    {
        auto ir = (SubR8*)codes;
        ir->__prefix = 251;
        ir->__code = 86;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(SubR8);
    }
    case OpCodeEnum::SubI4Short:
    {
        auto ir = (SubI4Short*)codes;
        ir->__code = 54;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(SubI4Short);
    }
    case OpCodeEnum::SubI8Short:
    {
        auto ir = (SubI8Short*)codes;
        ir->__code = 55;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(SubI8Short);
    }
    case OpCodeEnum::SubR4Short:
    {
        auto ir = (SubR4Short*)codes;
        ir->__code = 56;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(SubR4Short);
    }
    case OpCodeEnum::SubR8Short:
    {
        auto ir = (SubR8Short*)codes;
        ir->__code = 57;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(SubR8Short);
    }
    case OpCodeEnum::MulI4:
    {
        auto ir = (MulI4*)codes;
        ir->__prefix = 251;
        ir->__code = 87;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(MulI4);
    }
    case OpCodeEnum::MulI8:
    {
        auto ir = (MulI8*)codes;
        ir->__prefix = 251;
        ir->__code = 88;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(MulI8);
    }
    case OpCodeEnum::MulR4:
    {
        auto ir = (MulR4*)codes;
        ir->__prefix = 251;
        ir->__code = 89;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(MulR4);
    }
    case OpCodeEnum::MulR8:
    {
        auto ir = (MulR8*)codes;
        ir->__prefix = 251;
        ir->__code = 90;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(MulR8);
    }
    case OpCodeEnum::MulI4Short:
    {
        auto ir = (MulI4Short*)codes;
        ir->__code = 58;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(MulI4Short);
    }
    case OpCodeEnum::MulI8Short:
    {
        auto ir = (MulI8Short*)codes;
        ir->__code = 59;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(MulI8Short);
    }
    case OpCodeEnum::MulR4Short:
    {
        auto ir = (MulR4Short*)codes;
        ir->__code = 60;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(MulR4Short);
    }
    case OpCodeEnum::MulR8Short:
    {
        auto ir = (MulR8Short*)codes;
        ir->__code = 61;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(MulR8Short);
    }
    case OpCodeEnum::DivI4:
    {
        auto ir = (DivI4*)codes;
        ir->__prefix = 251;
        ir->__code = 91;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(DivI4);
    }
    case OpCodeEnum::DivI8:
    {
        auto ir = (DivI8*)codes;
        ir->__prefix = 251;
        ir->__code = 92;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(DivI8);
    }
    case OpCodeEnum::DivR4:
    {
        auto ir = (DivR4*)codes;
        ir->__prefix = 251;
        ir->__code = 93;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(DivR4);
    }
    case OpCodeEnum::DivR8:
    {
        auto ir = (DivR8*)codes;
        ir->__prefix = 251;
        ir->__code = 94;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(DivR8);
    }
    case OpCodeEnum::DivI4Short:
    {
        auto ir = (DivI4Short*)codes;
        ir->__code = 62;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(DivI4Short);
    }
    case OpCodeEnum::DivI8Short:
    {
        auto ir = (DivI8Short*)codes;
        ir->__code = 63;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(DivI8Short);
    }
    case OpCodeEnum::DivR4Short:
    {
        auto ir = (DivR4Short*)codes;
        ir->__code = 64;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(DivR4Short);
    }
    case OpCodeEnum::DivR8Short:
    {
        auto ir = (DivR8Short*)codes;
        ir->__code = 65;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(DivR8Short);
    }
    case OpCodeEnum::DivUnI4:
    {
        auto ir = (DivUnI4*)codes;
        ir->__prefix = 251;
        ir->__code = 95;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(DivUnI4);
    }
    case OpCodeEnum::DivUnI8:
    {
        auto ir = (DivUnI8*)codes;
        ir->__prefix = 251;
        ir->__code = 96;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(DivUnI8);
    }
    case OpCodeEnum::DivUnI4Short:
    {
        auto ir = (DivUnI4Short*)codes;
        ir->__code = 66;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(DivUnI4Short);
    }
    case OpCodeEnum::DivUnI8Short:
    {
        auto ir = (DivUnI8Short*)codes;
        ir->__code = 67;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(DivUnI8Short);
    }
    case OpCodeEnum::RemI4:
    {
        auto ir = (RemI4*)codes;
        ir->__prefix = 251;
        ir->__code = 97;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(RemI4);
    }
    case OpCodeEnum::RemI8:
    {
        auto ir = (RemI8*)codes;
        ir->__prefix = 251;
        ir->__code = 98;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(RemI8);
    }
    case OpCodeEnum::RemR4:
    {
        auto ir = (RemR4*)codes;
        ir->__prefix = 251;
        ir->__code = 99;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(RemR4);
    }
    case OpCodeEnum::RemR8:
    {
        auto ir = (RemR8*)codes;
        ir->__prefix = 251;
        ir->__code = 100;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(RemR8);
    }
    case OpCodeEnum::RemI4Short:
    {
        auto ir = (RemI4Short*)codes;
        ir->__code = 68;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(RemI4Short);
    }
    case OpCodeEnum::RemI8Short:
    {
        auto ir = (RemI8Short*)codes;
        ir->__code = 69;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(RemI8Short);
    }
    case OpCodeEnum::RemR4Short:
    {
        auto ir = (RemR4Short*)codes;
        ir->__code = 70;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(RemR4Short);
    }
    case OpCodeEnum::RemR8Short:
    {
        auto ir = (RemR8Short*)codes;
        ir->__code = 71;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(RemR8Short);
    }
    case OpCodeEnum::RemUnI4:
    {
        auto ir = (RemUnI4*)codes;
        ir->__prefix = 251;
        ir->__code = 101;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(RemUnI4);
    }
    case OpCodeEnum::RemUnI8:
    {
        auto ir = (RemUnI8*)codes;
        ir->__prefix = 251;
        ir->__code = 102;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(RemUnI8);
    }
    case OpCodeEnum::RemUnI4Short:
    {
        auto ir = (RemUnI4Short*)codes;
        ir->__code = 72;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(RemUnI4Short);
    }
    case OpCodeEnum::RemUnI8Short:
    {
        auto ir = (RemUnI8Short*)codes;
        ir->__code = 73;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(RemUnI8Short);
    }
    case OpCodeEnum::AndI4:
    {
        auto ir = (AndI4*)codes;
        ir->__prefix = 251;
        ir->__code = 103;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(AndI4);
    }
    case OpCodeEnum::AndI8:
    {
        auto ir = (AndI8*)codes;
        ir->__prefix = 251;
        ir->__code = 104;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(AndI8);
    }
    case OpCodeEnum::AndI4Short:
    {
        auto ir = (AndI4Short*)codes;
        ir->__code = 74;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(AndI4Short);
    }
    case OpCodeEnum::AndI8Short:
    {
        auto ir = (AndI8Short*)codes;
        ir->__code = 75;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(AndI8Short);
    }
    case OpCodeEnum::OrI4:
    {
        auto ir = (OrI4*)codes;
        ir->__prefix = 251;
        ir->__code = 105;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(OrI4);
    }
    case OpCodeEnum::OrI8:
    {
        auto ir = (OrI8*)codes;
        ir->__prefix = 251;
        ir->__code = 106;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(OrI8);
    }
    case OpCodeEnum::OrI4Short:
    {
        auto ir = (OrI4Short*)codes;
        ir->__code = 76;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(OrI4Short);
    }
    case OpCodeEnum::OrI8Short:
    {
        auto ir = (OrI8Short*)codes;
        ir->__code = 77;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(OrI8Short);
    }
    case OpCodeEnum::XorI4:
    {
        auto ir = (XorI4*)codes;
        ir->__prefix = 251;
        ir->__code = 107;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(XorI4);
    }
    case OpCodeEnum::XorI8:
    {
        auto ir = (XorI8*)codes;
        ir->__prefix = 251;
        ir->__code = 108;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(XorI8);
    }
    case OpCodeEnum::XorI4Short:
    {
        auto ir = (XorI4Short*)codes;
        ir->__code = 78;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(XorI4Short);
    }
    case OpCodeEnum::XorI8Short:
    {
        auto ir = (XorI8Short*)codes;
        ir->__code = 79;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(XorI8Short);
    }
    case OpCodeEnum::ShlI4:
    {
        auto ir = (ShlI4*)codes;
        ir->__prefix = 251;
        ir->__code = 109;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ShlI4);
    }
    case OpCodeEnum::ShlI8:
    {
        auto ir = (ShlI8*)codes;
        ir->__prefix = 251;
        ir->__code = 110;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ShlI8);
    }
    case OpCodeEnum::ShlI4Short:
    {
        auto ir = (ShlI4Short*)codes;
        ir->__code = 80;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ShlI4Short);
    }
    case OpCodeEnum::ShrI4:
    {
        auto ir = (ShrI4*)codes;
        ir->__prefix = 251;
        ir->__code = 111;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ShrI4);
    }
    case OpCodeEnum::ShrI8:
    {
        auto ir = (ShrI8*)codes;
        ir->__prefix = 251;
        ir->__code = 112;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ShrI8);
    }
    case OpCodeEnum::ShrI4Short:
    {
        auto ir = (ShrI4Short*)codes;
        ir->__code = 81;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ShrI4Short);
    }
    case OpCodeEnum::ShrUnI4:
    {
        auto ir = (ShrUnI4*)codes;
        ir->__prefix = 251;
        ir->__code = 113;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ShrUnI4);
    }
    case OpCodeEnum::ShrUnI8:
    {
        auto ir = (ShrUnI8*)codes;
        ir->__prefix = 251;
        ir->__code = 114;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ShrUnI8);
    }
    case OpCodeEnum::ShrUnI4Short:
    {
        auto ir = (ShrUnI4Short*)codes;
        ir->__code = 82;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ShrUnI4Short);
    }
    case OpCodeEnum::NegI4:
    {
        auto ir = (NegI4*)codes;
        ir->__prefix = 251;
        ir->__code = 115;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(NegI4);
    }
    case OpCodeEnum::NegI8:
    {
        auto ir = (NegI8*)codes;
        ir->__prefix = 251;
        ir->__code = 116;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(NegI8);
    }
    case OpCodeEnum::NegR4:
    {
        auto ir = (NegR4*)codes;
        ir->__prefix = 251;
        ir->__code = 117;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(NegR4);
    }
    case OpCodeEnum::NegR8:
    {
        auto ir = (NegR8*)codes;
        ir->__prefix = 251;
        ir->__code = 118;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(NegR8);
    }
    case OpCodeEnum::NegI4Short:
    {
        auto ir = (NegI4Short*)codes;
        ir->__code = 83;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(NegI4Short);
    }
    case OpCodeEnum::NegI8Short:
    {
        auto ir = (NegI8Short*)codes;
        ir->__code = 84;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(NegI8Short);
    }
    case OpCodeEnum::NegR4Short:
    {
        auto ir = (NegR4Short*)codes;
        ir->__code = 85;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(NegR4Short);
    }
    case OpCodeEnum::NegR8Short:
    {
        auto ir = (NegR8Short*)codes;
        ir->__code = 86;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(NegR8Short);
    }
    case OpCodeEnum::NotI4:
    {
        auto ir = (NotI4*)codes;
        ir->__prefix = 251;
        ir->__code = 119;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(NotI4);
    }
    case OpCodeEnum::NotI8:
    {
        auto ir = (NotI8*)codes;
        ir->__prefix = 251;
        ir->__code = 120;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(NotI8);
    }
    case OpCodeEnum::NotI4Short:
    {
        auto ir = (NotI4Short*)codes;
        ir->__code = 87;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(NotI4Short);
    }
    case OpCodeEnum::NotI8Short:
    {
        auto ir = (NotI8Short*)codes;
        ir->__code = 88;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(NotI8Short);
    }
    case OpCodeEnum::AddOvfI4:
    {
        auto ir = (AddOvfI4*)codes;
        ir->__prefix = 253;
        ir->__code = 9;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(AddOvfI4);
    }
    case OpCodeEnum::AddOvfI8:
    {
        auto ir = (AddOvfI8*)codes;
        ir->__prefix = 253;
        ir->__code = 10;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(AddOvfI8);
    }
    case OpCodeEnum::AddOvfUnI4:
    {
        auto ir = (AddOvfUnI4*)codes;
        ir->__prefix = 253;
        ir->__code = 11;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(AddOvfUnI4);
    }
    case OpCodeEnum::AddOvfUnI8:
    {
        auto ir = (AddOvfUnI8*)codes;
        ir->__prefix = 253;
        ir->__code = 12;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(AddOvfUnI8);
    }
    case OpCodeEnum::MulOvfI4:
    {
        auto ir = (MulOvfI4*)codes;
        ir->__prefix = 253;
        ir->__code = 13;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(MulOvfI4);
    }
    case OpCodeEnum::MulOvfI8:
    {
        auto ir = (MulOvfI8*)codes;
        ir->__prefix = 253;
        ir->__code = 14;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(MulOvfI8);
    }
    case OpCodeEnum::MulOvfUnI4:
    {
        auto ir = (MulOvfUnI4*)codes;
        ir->__prefix = 253;
        ir->__code = 15;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(MulOvfUnI4);
    }
    case OpCodeEnum::MulOvfUnI8:
    {
        auto ir = (MulOvfUnI8*)codes;
        ir->__prefix = 253;
        ir->__code = 16;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(MulOvfUnI8);
    }
    case OpCodeEnum::SubOvfI4:
    {
        auto ir = (SubOvfI4*)codes;
        ir->__prefix = 253;
        ir->__code = 17;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(SubOvfI4);
    }
    case OpCodeEnum::SubOvfI8:
    {
        auto ir = (SubOvfI8*)codes;
        ir->__prefix = 253;
        ir->__code = 18;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(SubOvfI8);
    }
    case OpCodeEnum::SubOvfUnI4:
    {
        auto ir = (SubOvfUnI4*)codes;
        ir->__prefix = 253;
        ir->__code = 19;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(SubOvfUnI4);
    }
    case OpCodeEnum::SubOvfUnI8:
    {
        auto ir = (SubOvfUnI8*)codes;
        ir->__prefix = 253;
        ir->__code = 20;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(SubOvfUnI8);
    }
    case OpCodeEnum::ConvI1I4:
    {
        auto ir = (ConvI1I4*)codes;
        ir->__prefix = 252;
        ir->__code = 12;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI1I4);
    }
    case OpCodeEnum::ConvI1I8:
    {
        auto ir = (ConvI1I8*)codes;
        ir->__prefix = 252;
        ir->__code = 13;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI1I8);
    }
    case OpCodeEnum::ConvI1R4:
    {
        auto ir = (ConvI1R4*)codes;
        ir->__prefix = 252;
        ir->__code = 14;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI1R4);
    }
    case OpCodeEnum::ConvI1R8:
    {
        auto ir = (ConvI1R8*)codes;
        ir->__prefix = 252;
        ir->__code = 15;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI1R8);
    }
    case OpCodeEnum::ConvI1I4Short:
    {
        auto ir = (ConvI1I4Short*)codes;
        ir->__code = 89;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI1I4Short);
    }
    case OpCodeEnum::ConvI1I8Short:
    {
        auto ir = (ConvI1I8Short*)codes;
        ir->__code = 90;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI1I8Short);
    }
    case OpCodeEnum::ConvI1R4Short:
    {
        auto ir = (ConvI1R4Short*)codes;
        ir->__code = 91;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI1R4Short);
    }
    case OpCodeEnum::ConvI1R8Short:
    {
        auto ir = (ConvI1R8Short*)codes;
        ir->__code = 92;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI1R8Short);
    }
    case OpCodeEnum::ConvU1I4:
    {
        auto ir = (ConvU1I4*)codes;
        ir->__prefix = 252;
        ir->__code = 16;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU1I4);
    }
    case OpCodeEnum::ConvU1I8:
    {
        auto ir = (ConvU1I8*)codes;
        ir->__prefix = 252;
        ir->__code = 17;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU1I8);
    }
    case OpCodeEnum::ConvU1R4:
    {
        auto ir = (ConvU1R4*)codes;
        ir->__prefix = 252;
        ir->__code = 18;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU1R4);
    }
    case OpCodeEnum::ConvU1R8:
    {
        auto ir = (ConvU1R8*)codes;
        ir->__prefix = 252;
        ir->__code = 19;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU1R8);
    }
    case OpCodeEnum::ConvU1I4Short:
    {
        auto ir = (ConvU1I4Short*)codes;
        ir->__code = 93;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU1I4Short);
    }
    case OpCodeEnum::ConvU1I8Short:
    {
        auto ir = (ConvU1I8Short*)codes;
        ir->__code = 94;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU1I8Short);
    }
    case OpCodeEnum::ConvU1R4Short:
    {
        auto ir = (ConvU1R4Short*)codes;
        ir->__code = 95;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU1R4Short);
    }
    case OpCodeEnum::ConvU1R8Short:
    {
        auto ir = (ConvU1R8Short*)codes;
        ir->__code = 96;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU1R8Short);
    }
    case OpCodeEnum::ConvI2I4:
    {
        auto ir = (ConvI2I4*)codes;
        ir->__prefix = 252;
        ir->__code = 20;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI2I4);
    }
    case OpCodeEnum::ConvI2I8:
    {
        auto ir = (ConvI2I8*)codes;
        ir->__prefix = 252;
        ir->__code = 21;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI2I8);
    }
    case OpCodeEnum::ConvI2R4:
    {
        auto ir = (ConvI2R4*)codes;
        ir->__prefix = 252;
        ir->__code = 22;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI2R4);
    }
    case OpCodeEnum::ConvI2R8:
    {
        auto ir = (ConvI2R8*)codes;
        ir->__prefix = 252;
        ir->__code = 23;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI2R8);
    }
    case OpCodeEnum::ConvI2I4Short:
    {
        auto ir = (ConvI2I4Short*)codes;
        ir->__code = 97;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI2I4Short);
    }
    case OpCodeEnum::ConvI2I8Short:
    {
        auto ir = (ConvI2I8Short*)codes;
        ir->__code = 98;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI2I8Short);
    }
    case OpCodeEnum::ConvI2R4Short:
    {
        auto ir = (ConvI2R4Short*)codes;
        ir->__code = 99;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI2R4Short);
    }
    case OpCodeEnum::ConvI2R8Short:
    {
        auto ir = (ConvI2R8Short*)codes;
        ir->__code = 100;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI2R8Short);
    }
    case OpCodeEnum::ConvU2I4:
    {
        auto ir = (ConvU2I4*)codes;
        ir->__prefix = 252;
        ir->__code = 24;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU2I4);
    }
    case OpCodeEnum::ConvU2I8:
    {
        auto ir = (ConvU2I8*)codes;
        ir->__prefix = 252;
        ir->__code = 25;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU2I8);
    }
    case OpCodeEnum::ConvU2R4:
    {
        auto ir = (ConvU2R4*)codes;
        ir->__prefix = 252;
        ir->__code = 26;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU2R4);
    }
    case OpCodeEnum::ConvU2R8:
    {
        auto ir = (ConvU2R8*)codes;
        ir->__prefix = 252;
        ir->__code = 27;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU2R8);
    }
    case OpCodeEnum::ConvU2I4Short:
    {
        auto ir = (ConvU2I4Short*)codes;
        ir->__code = 101;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU2I4Short);
    }
    case OpCodeEnum::ConvU2I8Short:
    {
        auto ir = (ConvU2I8Short*)codes;
        ir->__code = 102;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU2I8Short);
    }
    case OpCodeEnum::ConvU2R4Short:
    {
        auto ir = (ConvU2R4Short*)codes;
        ir->__code = 103;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU2R4Short);
    }
    case OpCodeEnum::ConvU2R8Short:
    {
        auto ir = (ConvU2R8Short*)codes;
        ir->__code = 104;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU2R8Short);
    }
    case OpCodeEnum::ConvI4I8:
    {
        auto ir = (ConvI4I8*)codes;
        ir->__prefix = 252;
        ir->__code = 28;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI4I8);
    }
    case OpCodeEnum::ConvI4R4:
    {
        auto ir = (ConvI4R4*)codes;
        ir->__prefix = 252;
        ir->__code = 29;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI4R4);
    }
    case OpCodeEnum::ConvI4R8:
    {
        auto ir = (ConvI4R8*)codes;
        ir->__prefix = 252;
        ir->__code = 30;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI4R8);
    }
    case OpCodeEnum::ConvI4I8Short:
    {
        auto ir = (ConvI4I8Short*)codes;
        ir->__code = 105;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI4I8Short);
    }
    case OpCodeEnum::ConvI4R4Short:
    {
        auto ir = (ConvI4R4Short*)codes;
        ir->__code = 106;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI4R4Short);
    }
    case OpCodeEnum::ConvI4R8Short:
    {
        auto ir = (ConvI4R8Short*)codes;
        ir->__code = 107;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI4R8Short);
    }
    case OpCodeEnum::ConvU4I8:
    {
        auto ir = (ConvU4I8*)codes;
        ir->__prefix = 252;
        ir->__code = 31;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU4I8);
    }
    case OpCodeEnum::ConvU4R4:
    {
        auto ir = (ConvU4R4*)codes;
        ir->__prefix = 252;
        ir->__code = 32;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU4R4);
    }
    case OpCodeEnum::ConvU4R8:
    {
        auto ir = (ConvU4R8*)codes;
        ir->__prefix = 252;
        ir->__code = 33;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU4R8);
    }
    case OpCodeEnum::ConvU4I8Short:
    {
        auto ir = (ConvU4I8Short*)codes;
        ir->__code = 108;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU4I8Short);
    }
    case OpCodeEnum::ConvU4R4Short:
    {
        auto ir = (ConvU4R4Short*)codes;
        ir->__code = 109;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU4R4Short);
    }
    case OpCodeEnum::ConvU4R8Short:
    {
        auto ir = (ConvU4R8Short*)codes;
        ir->__code = 110;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU4R8Short);
    }
    case OpCodeEnum::ConvI8I4:
    {
        auto ir = (ConvI8I4*)codes;
        ir->__prefix = 252;
        ir->__code = 34;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI8I4);
    }
    case OpCodeEnum::ConvI8U4:
    {
        auto ir = (ConvI8U4*)codes;
        ir->__prefix = 252;
        ir->__code = 35;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI8U4);
    }
    case OpCodeEnum::ConvI8R4:
    {
        auto ir = (ConvI8R4*)codes;
        ir->__prefix = 252;
        ir->__code = 36;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI8R4);
    }
    case OpCodeEnum::ConvI8R8:
    {
        auto ir = (ConvI8R8*)codes;
        ir->__prefix = 252;
        ir->__code = 37;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI8R8);
    }
    case OpCodeEnum::ConvI8I4Short:
    {
        auto ir = (ConvI8I4Short*)codes;
        ir->__code = 111;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI8I4Short);
    }
    case OpCodeEnum::ConvI8R4Short:
    {
        auto ir = (ConvI8R4Short*)codes;
        ir->__code = 112;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI8R4Short);
    }
    case OpCodeEnum::ConvI8R8Short:
    {
        auto ir = (ConvI8R8Short*)codes;
        ir->__code = 113;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvI8R8Short);
    }
    case OpCodeEnum::ConvU8I4:
    {
        auto ir = (ConvU8I4*)codes;
        ir->__prefix = 252;
        ir->__code = 38;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU8I4);
    }
    case OpCodeEnum::ConvU8R4:
    {
        auto ir = (ConvU8R4*)codes;
        ir->__prefix = 252;
        ir->__code = 39;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU8R4);
    }
    case OpCodeEnum::ConvU8R8:
    {
        auto ir = (ConvU8R8*)codes;
        ir->__prefix = 252;
        ir->__code = 40;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvU8R8);
    }
    case OpCodeEnum::ConvR4I4:
    {
        auto ir = (ConvR4I4*)codes;
        ir->__prefix = 252;
        ir->__code = 41;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvR4I4);
    }
    case OpCodeEnum::ConvR4I8:
    {
        auto ir = (ConvR4I8*)codes;
        ir->__prefix = 252;
        ir->__code = 42;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvR4I8);
    }
    case OpCodeEnum::ConvR4R8:
    {
        auto ir = (ConvR4R8*)codes;
        ir->__prefix = 252;
        ir->__code = 43;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvR4R8);
    }
    case OpCodeEnum::ConvR4I4Short:
    {
        auto ir = (ConvR4I4Short*)codes;
        ir->__code = 114;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvR4I4Short);
    }
    case OpCodeEnum::ConvR4I8Short:
    {
        auto ir = (ConvR4I8Short*)codes;
        ir->__code = 115;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvR4I8Short);
    }
    case OpCodeEnum::ConvR4R8Short:
    {
        auto ir = (ConvR4R8Short*)codes;
        ir->__code = 116;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvR4R8Short);
    }
    case OpCodeEnum::ConvR8I4:
    {
        auto ir = (ConvR8I4*)codes;
        ir->__prefix = 252;
        ir->__code = 44;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvR8I4);
    }
    case OpCodeEnum::ConvR8I8:
    {
        auto ir = (ConvR8I8*)codes;
        ir->__prefix = 252;
        ir->__code = 45;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvR8I8);
    }
    case OpCodeEnum::ConvR8R4:
    {
        auto ir = (ConvR8R4*)codes;
        ir->__prefix = 252;
        ir->__code = 46;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvR8R4);
    }
    case OpCodeEnum::ConvR8I4Short:
    {
        auto ir = (ConvR8I4Short*)codes;
        ir->__code = 117;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvR8I4Short);
    }
    case OpCodeEnum::ConvR8I8Short:
    {
        auto ir = (ConvR8I8Short*)codes;
        ir->__code = 118;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvR8I8Short);
    }
    case OpCodeEnum::ConvR8R4Short:
    {
        auto ir = (ConvR8R4Short*)codes;
        ir->__code = 119;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvR8R4Short);
    }
    case OpCodeEnum::ConvOvfI1I4:
    {
        auto ir = (ConvOvfI1I4*)codes;
        ir->__prefix = 253;
        ir->__code = 21;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI1I4);
    }
    case OpCodeEnum::ConvOvfI1I8:
    {
        auto ir = (ConvOvfI1I8*)codes;
        ir->__prefix = 253;
        ir->__code = 22;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI1I8);
    }
    case OpCodeEnum::ConvOvfI1R4:
    {
        auto ir = (ConvOvfI1R4*)codes;
        ir->__prefix = 253;
        ir->__code = 23;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI1R4);
    }
    case OpCodeEnum::ConvOvfI1R8:
    {
        auto ir = (ConvOvfI1R8*)codes;
        ir->__prefix = 253;
        ir->__code = 24;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI1R8);
    }
    case OpCodeEnum::ConvOvfU1I4:
    {
        auto ir = (ConvOvfU1I4*)codes;
        ir->__prefix = 253;
        ir->__code = 25;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU1I4);
    }
    case OpCodeEnum::ConvOvfU1I8:
    {
        auto ir = (ConvOvfU1I8*)codes;
        ir->__prefix = 253;
        ir->__code = 26;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU1I8);
    }
    case OpCodeEnum::ConvOvfU1R4:
    {
        auto ir = (ConvOvfU1R4*)codes;
        ir->__prefix = 253;
        ir->__code = 27;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU1R4);
    }
    case OpCodeEnum::ConvOvfU1R8:
    {
        auto ir = (ConvOvfU1R8*)codes;
        ir->__prefix = 253;
        ir->__code = 28;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU1R8);
    }
    case OpCodeEnum::ConvOvfI2I4:
    {
        auto ir = (ConvOvfI2I4*)codes;
        ir->__prefix = 253;
        ir->__code = 29;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI2I4);
    }
    case OpCodeEnum::ConvOvfI2I8:
    {
        auto ir = (ConvOvfI2I8*)codes;
        ir->__prefix = 253;
        ir->__code = 30;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI2I8);
    }
    case OpCodeEnum::ConvOvfI2R4:
    {
        auto ir = (ConvOvfI2R4*)codes;
        ir->__prefix = 253;
        ir->__code = 31;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI2R4);
    }
    case OpCodeEnum::ConvOvfI2R8:
    {
        auto ir = (ConvOvfI2R8*)codes;
        ir->__prefix = 253;
        ir->__code = 32;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI2R8);
    }
    case OpCodeEnum::ConvOvfU2I4:
    {
        auto ir = (ConvOvfU2I4*)codes;
        ir->__prefix = 253;
        ir->__code = 33;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU2I4);
    }
    case OpCodeEnum::ConvOvfU2I8:
    {
        auto ir = (ConvOvfU2I8*)codes;
        ir->__prefix = 253;
        ir->__code = 34;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU2I8);
    }
    case OpCodeEnum::ConvOvfU2R4:
    {
        auto ir = (ConvOvfU2R4*)codes;
        ir->__prefix = 253;
        ir->__code = 35;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU2R4);
    }
    case OpCodeEnum::ConvOvfU2R8:
    {
        auto ir = (ConvOvfU2R8*)codes;
        ir->__prefix = 253;
        ir->__code = 36;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU2R8);
    }
    case OpCodeEnum::ConvOvfI4I8:
    {
        auto ir = (ConvOvfI4I8*)codes;
        ir->__prefix = 253;
        ir->__code = 37;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI4I8);
    }
    case OpCodeEnum::ConvOvfI4R4:
    {
        auto ir = (ConvOvfI4R4*)codes;
        ir->__prefix = 253;
        ir->__code = 38;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI4R4);
    }
    case OpCodeEnum::ConvOvfI4R8:
    {
        auto ir = (ConvOvfI4R8*)codes;
        ir->__prefix = 253;
        ir->__code = 39;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI4R8);
    }
    case OpCodeEnum::ConvOvfU4I4:
    {
        auto ir = (ConvOvfU4I4*)codes;
        ir->__prefix = 253;
        ir->__code = 40;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU4I4);
    }
    case OpCodeEnum::ConvOvfU4I8:
    {
        auto ir = (ConvOvfU4I8*)codes;
        ir->__prefix = 253;
        ir->__code = 41;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU4I8);
    }
    case OpCodeEnum::ConvOvfU4R4:
    {
        auto ir = (ConvOvfU4R4*)codes;
        ir->__prefix = 253;
        ir->__code = 42;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU4R4);
    }
    case OpCodeEnum::ConvOvfU4R8:
    {
        auto ir = (ConvOvfU4R8*)codes;
        ir->__prefix = 253;
        ir->__code = 43;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU4R8);
    }
    case OpCodeEnum::ConvOvfI8R4:
    {
        auto ir = (ConvOvfI8R4*)codes;
        ir->__prefix = 253;
        ir->__code = 44;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI8R4);
    }
    case OpCodeEnum::ConvOvfI8R8:
    {
        auto ir = (ConvOvfI8R8*)codes;
        ir->__prefix = 253;
        ir->__code = 45;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI8R8);
    }
    case OpCodeEnum::ConvOvfU8I4:
    {
        auto ir = (ConvOvfU8I4*)codes;
        ir->__prefix = 253;
        ir->__code = 46;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU8I4);
    }
    case OpCodeEnum::ConvOvfU8I8:
    {
        auto ir = (ConvOvfU8I8*)codes;
        ir->__prefix = 253;
        ir->__code = 47;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU8I8);
    }
    case OpCodeEnum::ConvOvfU8R4:
    {
        auto ir = (ConvOvfU8R4*)codes;
        ir->__prefix = 253;
        ir->__code = 48;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU8R4);
    }
    case OpCodeEnum::ConvOvfU8R8:
    {
        auto ir = (ConvOvfU8R8*)codes;
        ir->__prefix = 253;
        ir->__code = 49;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU8R8);
    }
    case OpCodeEnum::ConvOvfI1UnI4:
    {
        auto ir = (ConvOvfI1UnI4*)codes;
        ir->__prefix = 253;
        ir->__code = 50;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI1UnI4);
    }
    case OpCodeEnum::ConvOvfI1UnI8:
    {
        auto ir = (ConvOvfI1UnI8*)codes;
        ir->__prefix = 253;
        ir->__code = 51;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI1UnI8);
    }
    case OpCodeEnum::ConvOvfI1UnR4:
    {
        auto ir = (ConvOvfI1UnR4*)codes;
        ir->__prefix = 253;
        ir->__code = 52;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI1UnR4);
    }
    case OpCodeEnum::ConvOvfI1UnR8:
    {
        auto ir = (ConvOvfI1UnR8*)codes;
        ir->__prefix = 253;
        ir->__code = 53;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI1UnR8);
    }
    case OpCodeEnum::ConvOvfU1UnI4:
    {
        auto ir = (ConvOvfU1UnI4*)codes;
        ir->__prefix = 253;
        ir->__code = 54;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU1UnI4);
    }
    case OpCodeEnum::ConvOvfU1UnI8:
    {
        auto ir = (ConvOvfU1UnI8*)codes;
        ir->__prefix = 253;
        ir->__code = 55;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU1UnI8);
    }
    case OpCodeEnum::ConvOvfU1UnR4:
    {
        auto ir = (ConvOvfU1UnR4*)codes;
        ir->__prefix = 253;
        ir->__code = 56;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU1UnR4);
    }
    case OpCodeEnum::ConvOvfU1UnR8:
    {
        auto ir = (ConvOvfU1UnR8*)codes;
        ir->__prefix = 253;
        ir->__code = 57;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU1UnR8);
    }
    case OpCodeEnum::ConvOvfI2UnI4:
    {
        auto ir = (ConvOvfI2UnI4*)codes;
        ir->__prefix = 253;
        ir->__code = 58;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI2UnI4);
    }
    case OpCodeEnum::ConvOvfI2UnI8:
    {
        auto ir = (ConvOvfI2UnI8*)codes;
        ir->__prefix = 253;
        ir->__code = 59;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI2UnI8);
    }
    case OpCodeEnum::ConvOvfI2UnR4:
    {
        auto ir = (ConvOvfI2UnR4*)codes;
        ir->__prefix = 253;
        ir->__code = 60;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI2UnR4);
    }
    case OpCodeEnum::ConvOvfI2UnR8:
    {
        auto ir = (ConvOvfI2UnR8*)codes;
        ir->__prefix = 253;
        ir->__code = 61;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI2UnR8);
    }
    case OpCodeEnum::ConvOvfU2UnI4:
    {
        auto ir = (ConvOvfU2UnI4*)codes;
        ir->__prefix = 253;
        ir->__code = 62;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU2UnI4);
    }
    case OpCodeEnum::ConvOvfU2UnI8:
    {
        auto ir = (ConvOvfU2UnI8*)codes;
        ir->__prefix = 253;
        ir->__code = 63;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU2UnI8);
    }
    case OpCodeEnum::ConvOvfU2UnR4:
    {
        auto ir = (ConvOvfU2UnR4*)codes;
        ir->__prefix = 253;
        ir->__code = 64;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU2UnR4);
    }
    case OpCodeEnum::ConvOvfU2UnR8:
    {
        auto ir = (ConvOvfU2UnR8*)codes;
        ir->__prefix = 253;
        ir->__code = 65;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU2UnR8);
    }
    case OpCodeEnum::ConvOvfI4UnI4:
    {
        auto ir = (ConvOvfI4UnI4*)codes;
        ir->__prefix = 253;
        ir->__code = 66;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI4UnI4);
    }
    case OpCodeEnum::ConvOvfI4UnI8:
    {
        auto ir = (ConvOvfI4UnI8*)codes;
        ir->__prefix = 253;
        ir->__code = 67;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI4UnI8);
    }
    case OpCodeEnum::ConvOvfI4UnR4:
    {
        auto ir = (ConvOvfI4UnR4*)codes;
        ir->__prefix = 253;
        ir->__code = 68;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI4UnR4);
    }
    case OpCodeEnum::ConvOvfI4UnR8:
    {
        auto ir = (ConvOvfI4UnR8*)codes;
        ir->__prefix = 253;
        ir->__code = 69;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI4UnR8);
    }
    case OpCodeEnum::ConvOvfU4UnI8:
    {
        auto ir = (ConvOvfU4UnI8*)codes;
        ir->__prefix = 253;
        ir->__code = 70;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU4UnI8);
    }
    case OpCodeEnum::ConvOvfU4UnR4:
    {
        auto ir = (ConvOvfU4UnR4*)codes;
        ir->__prefix = 253;
        ir->__code = 71;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU4UnR4);
    }
    case OpCodeEnum::ConvOvfU4UnR8:
    {
        auto ir = (ConvOvfU4UnR8*)codes;
        ir->__prefix = 253;
        ir->__code = 72;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU4UnR8);
    }
    case OpCodeEnum::ConvOvfI8UnI8:
    {
        auto ir = (ConvOvfI8UnI8*)codes;
        ir->__prefix = 253;
        ir->__code = 73;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI8UnI8);
    }
    case OpCodeEnum::ConvOvfI8UnR4:
    {
        auto ir = (ConvOvfI8UnR4*)codes;
        ir->__prefix = 253;
        ir->__code = 74;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI8UnR4);
    }
    case OpCodeEnum::ConvOvfI8UnR8:
    {
        auto ir = (ConvOvfI8UnR8*)codes;
        ir->__prefix = 253;
        ir->__code = 75;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfI8UnR8);
    }
    case OpCodeEnum::ConvOvfU8UnR4:
    {
        auto ir = (ConvOvfU8UnR4*)codes;
        ir->__prefix = 253;
        ir->__code = 76;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU8UnR4);
    }
    case OpCodeEnum::ConvOvfU8UnR8:
    {
        auto ir = (ConvOvfU8UnR8*)codes;
        ir->__prefix = 253;
        ir->__code = 77;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(ConvOvfU8UnR8);
    }
    case OpCodeEnum::CeqI4:
    {
        auto ir = (CeqI4*)codes;
        ir->__prefix = 251;
        ir->__code = 121;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CeqI4);
    }
    case OpCodeEnum::CeqI8:
    {
        auto ir = (CeqI8*)codes;
        ir->__prefix = 251;
        ir->__code = 122;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CeqI8);
    }
    case OpCodeEnum::CeqR4:
    {
        auto ir = (CeqR4*)codes;
        ir->__prefix = 251;
        ir->__code = 123;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CeqR4);
    }
    case OpCodeEnum::CeqR8:
    {
        auto ir = (CeqR8*)codes;
        ir->__prefix = 251;
        ir->__code = 124;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CeqR8);
    }
    case OpCodeEnum::CeqI4Short:
    {
        auto ir = (CeqI4Short*)codes;
        ir->__code = 120;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CeqI4Short);
    }
    case OpCodeEnum::CeqI8Short:
    {
        auto ir = (CeqI8Short*)codes;
        ir->__code = 121;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CeqI8Short);
    }
    case OpCodeEnum::CeqR4Short:
    {
        auto ir = (CeqR4Short*)codes;
        ir->__code = 122;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CeqR4Short);
    }
    case OpCodeEnum::CeqR8Short:
    {
        auto ir = (CeqR8Short*)codes;
        ir->__code = 123;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CeqR8Short);
    }
    case OpCodeEnum::CgtI4:
    {
        auto ir = (CgtI4*)codes;
        ir->__prefix = 251;
        ir->__code = 125;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CgtI4);
    }
    case OpCodeEnum::CgtI8:
    {
        auto ir = (CgtI8*)codes;
        ir->__prefix = 251;
        ir->__code = 126;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CgtI8);
    }
    case OpCodeEnum::CgtR4:
    {
        auto ir = (CgtR4*)codes;
        ir->__prefix = 251;
        ir->__code = 127;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CgtR4);
    }
    case OpCodeEnum::CgtR8:
    {
        auto ir = (CgtR8*)codes;
        ir->__prefix = 251;
        ir->__code = 128;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CgtR8);
    }
    case OpCodeEnum::CgtI4Short:
    {
        auto ir = (CgtI4Short*)codes;
        ir->__code = 124;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CgtI4Short);
    }
    case OpCodeEnum::CgtI8Short:
    {
        auto ir = (CgtI8Short*)codes;
        ir->__code = 125;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CgtI8Short);
    }
    case OpCodeEnum::CgtUnI4:
    {
        auto ir = (CgtUnI4*)codes;
        ir->__prefix = 251;
        ir->__code = 129;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CgtUnI4);
    }
    case OpCodeEnum::CgtUnI8:
    {
        auto ir = (CgtUnI8*)codes;
        ir->__prefix = 251;
        ir->__code = 130;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CgtUnI8);
    }
    case OpCodeEnum::CgtUnR4:
    {
        auto ir = (CgtUnR4*)codes;
        ir->__prefix = 251;
        ir->__code = 131;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CgtUnR4);
    }
    case OpCodeEnum::CgtUnR8:
    {
        auto ir = (CgtUnR8*)codes;
        ir->__prefix = 251;
        ir->__code = 132;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CgtUnR8);
    }
    case OpCodeEnum::CgtUnI4Short:
    {
        auto ir = (CgtUnI4Short*)codes;
        ir->__code = 126;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CgtUnI4Short);
    }
    case OpCodeEnum::CgtUnI8Short:
    {
        auto ir = (CgtUnI8Short*)codes;
        ir->__code = 127;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CgtUnI8Short);
    }
    case OpCodeEnum::CltI4:
    {
        auto ir = (CltI4*)codes;
        ir->__prefix = 251;
        ir->__code = 133;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CltI4);
    }
    case OpCodeEnum::CltI8:
    {
        auto ir = (CltI8*)codes;
        ir->__prefix = 251;
        ir->__code = 134;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CltI8);
    }
    case OpCodeEnum::CltR4:
    {
        auto ir = (CltR4*)codes;
        ir->__prefix = 251;
        ir->__code = 135;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CltR4);
    }
    case OpCodeEnum::CltR8:
    {
        auto ir = (CltR8*)codes;
        ir->__prefix = 251;
        ir->__code = 136;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CltR8);
    }
    case OpCodeEnum::CltI4Short:
    {
        auto ir = (CltI4Short*)codes;
        ir->__code = 128;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CltI4Short);
    }
    case OpCodeEnum::CltI8Short:
    {
        auto ir = (CltI8Short*)codes;
        ir->__code = 129;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CltI8Short);
    }
    case OpCodeEnum::CltUnI4:
    {
        auto ir = (CltUnI4*)codes;
        ir->__prefix = 251;
        ir->__code = 137;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CltUnI4);
    }
    case OpCodeEnum::CltUnI8:
    {
        auto ir = (CltUnI8*)codes;
        ir->__prefix = 251;
        ir->__code = 138;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CltUnI8);
    }
    case OpCodeEnum::CltUnR4:
    {
        auto ir = (CltUnR4*)codes;
        ir->__prefix = 251;
        ir->__code = 139;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CltUnR4);
    }
    case OpCodeEnum::CltUnR8:
    {
        auto ir = (CltUnR8*)codes;
        ir->__prefix = 251;
        ir->__code = 140;
        ir->arg1 = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CltUnR8);
    }
    case OpCodeEnum::CltUnI4Short:
    {
        auto ir = (CltUnI4Short*)codes;
        ir->__code = 130;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CltUnI4Short);
    }
    case OpCodeEnum::CltUnI8Short:
    {
        auto ir = (CltUnI8Short*)codes;
        ir->__code = 131;
        ir->arg1 = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->arg2 = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CltUnI8Short);
    }
    case OpCodeEnum::InitObjI1:
    {
        auto ir = (InitObjI1*)codes;
        ir->__prefix = 251;
        ir->__code = 141;
        ir->addr = (uint16_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(InitObjI1);
    }
    case OpCodeEnum::InitObjI1Short:
    {
        auto ir = (InitObjI1Short*)codes;
        ir->__code = 132;
        ir->addr = (uint8_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(InitObjI1Short);
    }
    case OpCodeEnum::InitObjI2:
    {
        auto ir = (InitObjI2*)codes;
        ir->__prefix = 251;
        ir->__code = 142;
        ir->addr = (uint16_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(InitObjI2);
    }
    case OpCodeEnum::InitObjI2Short:
    {
        auto ir = (InitObjI2Short*)codes;
        ir->__code = 133;
        ir->addr = (uint8_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(InitObjI2Short);
    }
    case OpCodeEnum::InitObjI2Unaligned:
    {
        auto ir = (InitObjI2Unaligned*)codes;
        ir->__prefix = 253;
        ir->__code = 78;
        ir->addr = (uint16_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(InitObjI2Unaligned);
    }
    case OpCodeEnum::InitObjI4:
    {
        auto ir = (InitObjI4*)codes;
        ir->__prefix = 251;
        ir->__code = 143;
        ir->addr = (uint16_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(InitObjI4);
    }
    case OpCodeEnum::InitObjI4Short:
    {
        auto ir = (InitObjI4Short*)codes;
        ir->__code = 134;
        ir->addr = (uint8_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(InitObjI4Short);
    }
    case OpCodeEnum::InitObjI4Unaligned:
    {
        auto ir = (InitObjI4Unaligned*)codes;
        ir->__prefix = 253;
        ir->__code = 79;
        ir->addr = (uint16_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(InitObjI4Unaligned);
    }
    case OpCodeEnum::InitObjI8:
    {
        auto ir = (InitObjI8*)codes;
        ir->__prefix = 251;
        ir->__code = 144;
        ir->addr = (uint16_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(InitObjI8);
    }
    case OpCodeEnum::InitObjI8Short:
    {
        auto ir = (InitObjI8Short*)codes;
        ir->__code = 135;
        ir->addr = (uint8_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(InitObjI8Short);
    }
    case OpCodeEnum::InitObjI8Unaligned:
    {
        auto ir = (InitObjI8Unaligned*)codes;
        ir->__prefix = 253;
        ir->__code = 80;
        ir->addr = (uint16_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(InitObjI8Unaligned);
    }
    case OpCodeEnum::InitObjAny:
    {
        auto ir = (InitObjAny*)codes;
        ir->__prefix = 251;
        ir->__code = 145;
        ir->addr = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->size = (uint32_t)inst.get_size();
        return codes + sizeof(InitObjAny);
    }
    case OpCodeEnum::InitObjAnyShort:
    {
        auto ir = (InitObjAnyShort*)codes;
        ir->__code = 136;
        ir->addr = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->size = (uint32_t)inst.get_size();
        return codes + sizeof(InitObjAnyShort);
    }
    case OpCodeEnum::CpObjI1:
    {
        auto ir = (CpObjI1*)codes;
        ir->__prefix = 251;
        ir->__code = 146;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CpObjI1);
    }
    case OpCodeEnum::CpObjI1Short:
    {
        auto ir = (CpObjI1Short*)codes;
        ir->__code = 137;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CpObjI1Short);
    }
    case OpCodeEnum::CpObjI2:
    {
        auto ir = (CpObjI2*)codes;
        ir->__prefix = 251;
        ir->__code = 147;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CpObjI2);
    }
    case OpCodeEnum::CpObjI2Short:
    {
        auto ir = (CpObjI2Short*)codes;
        ir->__code = 138;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CpObjI2Short);
    }
    case OpCodeEnum::CpObjI4:
    {
        auto ir = (CpObjI4*)codes;
        ir->__prefix = 251;
        ir->__code = 148;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CpObjI4);
    }
    case OpCodeEnum::CpObjI4Short:
    {
        auto ir = (CpObjI4Short*)codes;
        ir->__code = 139;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CpObjI4Short);
    }
    case OpCodeEnum::CpObjI8:
    {
        auto ir = (CpObjI8*)codes;
        ir->__prefix = 251;
        ir->__code = 149;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CpObjI8);
    }
    case OpCodeEnum::CpObjI8Short:
    {
        auto ir = (CpObjI8Short*)codes;
        ir->__code = 140;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(CpObjI8Short);
    }
    case OpCodeEnum::CpObjAny:
    {
        auto ir = (CpObjAny*)codes;
        ir->__prefix = 251;
        ir->__code = 150;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->size = (uint16_t)inst.get_size();
        return codes + sizeof(CpObjAny);
    }
    case OpCodeEnum::CpObjAnyShort:
    {
        auto ir = (CpObjAnyShort*)codes;
        ir->__code = 141;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->size = (uint8_t)inst.get_size();
        return codes + sizeof(CpObjAnyShort);
    }
    case OpCodeEnum::LdObjAny:
    {
        auto ir = (LdObjAny*)codes;
        ir->__prefix = 251;
        ir->__code = 151;
        ir->addr = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->size = (uint16_t)inst.get_size();
        return codes + sizeof(LdObjAny);
    }
    case OpCodeEnum::LdObjAnyShort:
    {
        auto ir = (LdObjAnyShort*)codes;
        ir->__code = 142;
        ir->addr = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->size = (uint8_t)inst.get_size();
        return codes + sizeof(LdObjAnyShort);
    }
    case OpCodeEnum::StObjAny:
    {
        auto ir = (StObjAny*)codes;
        ir->__prefix = 251;
        ir->__code = 152;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->addr = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->size = (uint16_t)inst.get_size();
        return codes + sizeof(StObjAny);
    }
    case OpCodeEnum::StObjAnyShort:
    {
        auto ir = (StObjAnyShort*)codes;
        ir->__code = 143;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->addr = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->size = (uint8_t)inst.get_size();
        return codes + sizeof(StObjAnyShort);
    }
    case OpCodeEnum::CastClass:
    {
        auto ir = (CastClass*)codes;
        ir->__prefix = 251;
        ir->__code = 153;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->klass_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(CastClass);
    }
    case OpCodeEnum::CastClassShort:
    {
        auto ir = (CastClassShort*)codes;
        ir->__code = 144;
        ir->obj = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->klass_idx = (uint8_t)inst.get_resolved_data_index();
        return codes + sizeof(CastClassShort);
    }
    case OpCodeEnum::IsInst:
    {
        auto ir = (IsInst*)codes;
        ir->__prefix = 251;
        ir->__code = 154;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->klass_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(IsInst);
    }
    case OpCodeEnum::IsInstShort:
    {
        auto ir = (IsInstShort*)codes;
        ir->__code = 145;
        ir->obj = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->klass_idx = (uint8_t)inst.get_resolved_data_index();
        return codes + sizeof(IsInstShort);
    }
    case OpCodeEnum::Box:
    {
        auto ir = (Box*)codes;
        ir->__prefix = 251;
        ir->__code = 155;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->klass_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(Box);
    }
    case OpCodeEnum::BoxShort:
    {
        auto ir = (BoxShort*)codes;
        ir->__code = 146;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->klass_idx = (uint8_t)inst.get_resolved_data_index();
        return codes + sizeof(BoxShort);
    }
    case OpCodeEnum::Unbox:
    {
        auto ir = (Unbox*)codes;
        ir->__prefix = 251;
        ir->__code = 156;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->klass_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(Unbox);
    }
    case OpCodeEnum::UnboxShort:
    {
        auto ir = (UnboxShort*)codes;
        ir->__code = 147;
        ir->obj = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->klass_idx = (uint8_t)inst.get_resolved_data_index();
        return codes + sizeof(UnboxShort);
    }
    case OpCodeEnum::UnboxAny:
    {
        auto ir = (UnboxAny*)codes;
        ir->__prefix = 251;
        ir->__code = 157;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->klass_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(UnboxAny);
    }
    case OpCodeEnum::UnboxAnyShort:
    {
        auto ir = (UnboxAnyShort*)codes;
        ir->__code = 148;
        ir->obj = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->klass_idx = (uint8_t)inst.get_resolved_data_index();
        return codes + sizeof(UnboxAnyShort);
    }
    case OpCodeEnum::NewArr:
    {
        auto ir = (NewArr*)codes;
        ir->__prefix = 251;
        ir->__code = 158;
        ir->length = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->arr_klass_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(NewArr);
    }
    case OpCodeEnum::NewArrShort:
    {
        auto ir = (NewArrShort*)codes;
        ir->__code = 149;
        ir->length = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->arr_klass_idx = (uint8_t)inst.get_resolved_data_index();
        return codes + sizeof(NewArrShort);
    }
    case OpCodeEnum::LdLen:
    {
        auto ir = (LdLen*)codes;
        ir->__prefix = 251;
        ir->__code = 159;
        ir->arr = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdLen);
    }
    case OpCodeEnum::LdLenShort:
    {
        auto ir = (LdLenShort*)codes;
        ir->__code = 150;
        ir->arr = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdLenShort);
    }
    case OpCodeEnum::Ldelema:
    {
        auto ir = (Ldelema*)codes;
        ir->__prefix = 251;
        ir->__code = 160;
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->ele_klass_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(Ldelema);
    }
    case OpCodeEnum::LdelemaShort:
    {
        auto ir = (LdelemaShort*)codes;
        ir->__code = 151;
        ir->arr = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->ele_klass_idx = (uint8_t)inst.get_resolved_data_index();
        return codes + sizeof(LdelemaShort);
    }
    case OpCodeEnum::LdelemaReadOnly:
    {
        auto ir = (LdelemaReadOnly*)codes;
        ir->__prefix = 252;
        ir->__code = 47;
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdelemaReadOnly);
    }
    case OpCodeEnum::LdelemI1:
    {
        auto ir = (LdelemI1*)codes;
        ir->__prefix = 251;
        ir->__code = 161;
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdelemI1);
    }
    case OpCodeEnum::LdelemI1Short:
    {
        auto ir = (LdelemI1Short*)codes;
        ir->__code = 152;
        ir->arr = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdelemI1Short);
    }
    case OpCodeEnum::LdelemU1:
    {
        auto ir = (LdelemU1*)codes;
        ir->__prefix = 251;
        ir->__code = 162;
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdelemU1);
    }
    case OpCodeEnum::LdelemU1Short:
    {
        auto ir = (LdelemU1Short*)codes;
        ir->__code = 153;
        ir->arr = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdelemU1Short);
    }
    case OpCodeEnum::LdelemI2:
    {
        auto ir = (LdelemI2*)codes;
        ir->__prefix = 251;
        ir->__code = 163;
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdelemI2);
    }
    case OpCodeEnum::LdelemI2Short:
    {
        auto ir = (LdelemI2Short*)codes;
        ir->__code = 154;
        ir->arr = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdelemI2Short);
    }
    case OpCodeEnum::LdelemU2:
    {
        auto ir = (LdelemU2*)codes;
        ir->__prefix = 251;
        ir->__code = 164;
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdelemU2);
    }
    case OpCodeEnum::LdelemU2Short:
    {
        auto ir = (LdelemU2Short*)codes;
        ir->__code = 155;
        ir->arr = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdelemU2Short);
    }
    case OpCodeEnum::LdelemI4:
    {
        auto ir = (LdelemI4*)codes;
        ir->__prefix = 251;
        ir->__code = 165;
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdelemI4);
    }
    case OpCodeEnum::LdelemI4Short:
    {
        auto ir = (LdelemI4Short*)codes;
        ir->__code = 156;
        ir->arr = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdelemI4Short);
    }
    case OpCodeEnum::LdelemI8:
    {
        auto ir = (LdelemI8*)codes;
        ir->__prefix = 251;
        ir->__code = 166;
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdelemI8);
    }
    case OpCodeEnum::LdelemI8Short:
    {
        auto ir = (LdelemI8Short*)codes;
        ir->__code = 157;
        ir->arr = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdelemI8Short);
    }
    case OpCodeEnum::LdelemI:
    {
        auto ir = (LdelemI*)codes;
        ir->__prefix = 251;
        ir->__code = 167;
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdelemI);
    }
    case OpCodeEnum::LdelemIShort:
    {
        auto ir = (LdelemIShort*)codes;
        ir->__code = 158;
        ir->arr = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdelemIShort);
    }
    case OpCodeEnum::LdelemR4:
    {
        auto ir = (LdelemR4*)codes;
        ir->__prefix = 251;
        ir->__code = 168;
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdelemR4);
    }
    case OpCodeEnum::LdelemR4Short:
    {
        auto ir = (LdelemR4Short*)codes;
        ir->__code = 159;
        ir->arr = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdelemR4Short);
    }
    case OpCodeEnum::LdelemR8:
    {
        auto ir = (LdelemR8*)codes;
        ir->__prefix = 251;
        ir->__code = 169;
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdelemR8);
    }
    case OpCodeEnum::LdelemR8Short:
    {
        auto ir = (LdelemR8Short*)codes;
        ir->__code = 160;
        ir->arr = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdelemR8Short);
    }
    case OpCodeEnum::LdelemRef:
    {
        auto ir = (LdelemRef*)codes;
        ir->__prefix = 251;
        ir->__code = 170;
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdelemRef);
    }
    case OpCodeEnum::LdelemRefShort:
    {
        auto ir = (LdelemRefShort*)codes;
        ir->__code = 161;
        ir->arr = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdelemRefShort);
    }
    case OpCodeEnum::LdelemAnyRef:
    {
        auto ir = (LdelemAnyRef*)codes;
        ir->__prefix = 251;
        ir->__code = 171;
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->ele_klass_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(LdelemAnyRef);
    }
    case OpCodeEnum::LdelemAnyRefShort:
    {
        auto ir = (LdelemAnyRefShort*)codes;
        ir->__code = 162;
        ir->arr = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->ele_klass_idx = (uint8_t)inst.get_resolved_data_index();
        return codes + sizeof(LdelemAnyRefShort);
    }
    case OpCodeEnum::LdelemAnyVal:
    {
        auto ir = (LdelemAnyVal*)codes;
        ir->__prefix = 251;
        ir->__code = 172;
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->ele_klass_idx = (uint16_t)inst.get_resolved_data_index();
        ir->ele_size = (uint32_t)inst.get_element_size();
        return codes + sizeof(LdelemAnyVal);
    }
    case OpCodeEnum::LdelemAnyValShort:
    {
        auto ir = (LdelemAnyValShort*)codes;
        ir->__code = 163;
        ir->arr = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->ele_klass_idx = (uint8_t)inst.get_resolved_data_index();
        ir->ele_size = (uint32_t)inst.get_element_size();
        return codes + sizeof(LdelemAnyValShort);
    }
    case OpCodeEnum::StelemI1:
    {
        auto ir = (StelemI1*)codes;
        ir->__prefix = 251;
        ir->__code = 173;
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg3_eval_stack_idx();
        return codes + sizeof(StelemI1);
    }
    case OpCodeEnum::StelemI1Short:
    {
        auto ir = (StelemI1Short*)codes;
        ir->__code = 164;
        ir->arr = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->value = (uint8_t)inst.get_var_arg3_eval_stack_idx();
        return codes + sizeof(StelemI1Short);
    }
    case OpCodeEnum::StelemI2:
    {
        auto ir = (StelemI2*)codes;
        ir->__prefix = 251;
        ir->__code = 174;
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg3_eval_stack_idx();
        return codes + sizeof(StelemI2);
    }
    case OpCodeEnum::StelemI2Short:
    {
        auto ir = (StelemI2Short*)codes;
        ir->__code = 165;
        ir->arr = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->value = (uint8_t)inst.get_var_arg3_eval_stack_idx();
        return codes + sizeof(StelemI2Short);
    }
    case OpCodeEnum::StelemI4:
    {
        auto ir = (StelemI4*)codes;
        ir->__prefix = 251;
        ir->__code = 175;
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg3_eval_stack_idx();
        return codes + sizeof(StelemI4);
    }
    case OpCodeEnum::StelemI4Short:
    {
        auto ir = (StelemI4Short*)codes;
        ir->__code = 166;
        ir->arr = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->value = (uint8_t)inst.get_var_arg3_eval_stack_idx();
        return codes + sizeof(StelemI4Short);
    }
    case OpCodeEnum::StelemI8:
    {
        auto ir = (StelemI8*)codes;
        ir->__prefix = 251;
        ir->__code = 176;
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg3_eval_stack_idx();
        return codes + sizeof(StelemI8);
    }
    case OpCodeEnum::StelemI8Short:
    {
        auto ir = (StelemI8Short*)codes;
        ir->__code = 167;
        ir->arr = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->value = (uint8_t)inst.get_var_arg3_eval_stack_idx();
        return codes + sizeof(StelemI8Short);
    }
    case OpCodeEnum::StelemI:
    {
        auto ir = (StelemI*)codes;
        ir->__prefix = 251;
        ir->__code = 177;
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg3_eval_stack_idx();
        return codes + sizeof(StelemI);
    }
    case OpCodeEnum::StelemIShort:
    {
        auto ir = (StelemIShort*)codes;
        ir->__code = 168;
        ir->arr = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->value = (uint8_t)inst.get_var_arg3_eval_stack_idx();
        return codes + sizeof(StelemIShort);
    }
    case OpCodeEnum::StelemR4:
    {
        auto ir = (StelemR4*)codes;
        ir->__prefix = 251;
        ir->__code = 178;
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg3_eval_stack_idx();
        return codes + sizeof(StelemR4);
    }
    case OpCodeEnum::StelemR4Short:
    {
        auto ir = (StelemR4Short*)codes;
        ir->__code = 169;
        ir->arr = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->value = (uint8_t)inst.get_var_arg3_eval_stack_idx();
        return codes + sizeof(StelemR4Short);
    }
    case OpCodeEnum::StelemR8:
    {
        auto ir = (StelemR8*)codes;
        ir->__prefix = 251;
        ir->__code = 179;
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg3_eval_stack_idx();
        return codes + sizeof(StelemR8);
    }
    case OpCodeEnum::StelemR8Short:
    {
        auto ir = (StelemR8Short*)codes;
        ir->__code = 170;
        ir->arr = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->value = (uint8_t)inst.get_var_arg3_eval_stack_idx();
        return codes + sizeof(StelemR8Short);
    }
    case OpCodeEnum::StelemRef:
    {
        auto ir = (StelemRef*)codes;
        ir->__prefix = 251;
        ir->__code = 180;
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg3_eval_stack_idx();
        return codes + sizeof(StelemRef);
    }
    case OpCodeEnum::StelemRefShort:
    {
        auto ir = (StelemRefShort*)codes;
        ir->__code = 171;
        ir->arr = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->value = (uint8_t)inst.get_var_arg3_eval_stack_idx();
        return codes + sizeof(StelemRefShort);
    }
    case OpCodeEnum::StelemAnyRef:
    {
        auto ir = (StelemAnyRef*)codes;
        ir->__prefix = 251;
        ir->__code = 181;
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg3_eval_stack_idx();
        ir->ele_klass_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(StelemAnyRef);
    }
    case OpCodeEnum::StelemAnyRefShort:
    {
        auto ir = (StelemAnyRefShort*)codes;
        ir->__code = 172;
        ir->arr = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->value = (uint8_t)inst.get_var_arg3_eval_stack_idx();
        ir->ele_klass_idx = (uint8_t)inst.get_resolved_data_index();
        return codes + sizeof(StelemAnyRefShort);
    }
    case OpCodeEnum::StelemAnyVal:
    {
        auto ir = (StelemAnyVal*)codes;
        ir->__prefix = 251;
        ir->__code = 182;
        ir->arr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg3_eval_stack_idx();
        ir->ele_klass_idx = (uint16_t)inst.get_resolved_data_index();
        ir->ele_size = (uint32_t)inst.get_element_size();
        return codes + sizeof(StelemAnyVal);
    }
    case OpCodeEnum::StelemAnyValShort:
    {
        auto ir = (StelemAnyValShort*)codes;
        ir->__code = 173;
        ir->arr = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->index = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->value = (uint8_t)inst.get_var_arg3_eval_stack_idx();
        ir->ele_klass_idx = (uint8_t)inst.get_resolved_data_index();
        ir->ele_size = (uint32_t)inst.get_element_size();
        return codes + sizeof(StelemAnyValShort);
    }
    case OpCodeEnum::MkRefAny:
    {
        auto ir = (MkRefAny*)codes;
        ir->__prefix = 251;
        ir->__code = 183;
        ir->addr = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->klass_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(MkRefAny);
    }
    case OpCodeEnum::RefAnyVal:
    {
        auto ir = (RefAnyVal*)codes;
        ir->__prefix = 251;
        ir->__code = 184;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->klass_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(RefAnyVal);
    }
    case OpCodeEnum::RefAnyType:
    {
        auto ir = (RefAnyType*)codes;
        ir->__prefix = 251;
        ir->__code = 185;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(RefAnyType);
    }
    case OpCodeEnum::LdToken:
    {
        auto ir = (LdToken*)codes;
        ir->__prefix = 251;
        ir->__code = 186;
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->token_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(LdToken);
    }
    case OpCodeEnum::CkfiniteR4:
    {
        auto ir = (CkfiniteR4*)codes;
        ir->__prefix = 251;
        ir->__code = 187;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(CkfiniteR4);
    }
    case OpCodeEnum::CkfiniteR8:
    {
        auto ir = (CkfiniteR8*)codes;
        ir->__prefix = 251;
        ir->__code = 188;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(CkfiniteR8);
    }
    case OpCodeEnum::LocAlloc:
    {
        auto ir = (LocAlloc*)codes;
        ir->__prefix = 251;
        ir->__code = 189;
        ir->size = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LocAlloc);
    }
    case OpCodeEnum::InitBlk:
    {
        auto ir = (InitBlk*)codes;
        ir->__prefix = 252;
        ir->__code = 48;
        ir->addr = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->size = (uint16_t)inst.get_var_arg3_eval_stack_idx();
        return codes + sizeof(InitBlk);
    }
    case OpCodeEnum::CpBlk:
    {
        auto ir = (CpBlk*)codes;
        ir->__prefix = 252;
        ir->__code = 49;
        ir->dst = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->src = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->size = (uint16_t)inst.get_var_arg3_eval_stack_idx();
        return codes + sizeof(CpBlk);
    }
    case OpCodeEnum::Ldftn:
    {
        auto ir = (Ldftn*)codes;
        ir->__prefix = 251;
        ir->__code = 190;
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->method_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(Ldftn);
    }
    case OpCodeEnum::LdftnShort:
    {
        auto ir = (LdftnShort*)codes;
        ir->__code = 174;
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->method_idx = (uint8_t)inst.get_resolved_data_index();
        return codes + sizeof(LdftnShort);
    }
    case OpCodeEnum::Ldvirtftn:
    {
        auto ir = (Ldvirtftn*)codes;
        ir->__prefix = 251;
        ir->__code = 191;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->method_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(Ldvirtftn);
    }
    case OpCodeEnum::LdvirtftnShort:
    {
        auto ir = (LdvirtftnShort*)codes;
        ir->__code = 175;
        ir->obj = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->method_idx = (uint8_t)inst.get_resolved_data_index();
        return codes + sizeof(LdvirtftnShort);
    }
    case OpCodeEnum::LdfldI1:
    {
        auto ir = (LdfldI1*)codes;
        ir->__prefix = 251;
        ir->__code = 192;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdfldI1);
    }
    case OpCodeEnum::LdfldI1Short:
    {
        auto ir = (LdfldI1Short*)codes;
        ir->__code = 176;
        ir->obj = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint8_t)inst.get_field_offset();
        return codes + sizeof(LdfldI1Short);
    }
    case OpCodeEnum::LdfldI1Large:
    {
        auto ir = (LdfldI1Large*)codes;
        ir->__prefix = 253;
        ir->__code = 81;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(LdfldI1Large);
    }
    case OpCodeEnum::LdfldU1:
    {
        auto ir = (LdfldU1*)codes;
        ir->__prefix = 251;
        ir->__code = 193;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdfldU1);
    }
    case OpCodeEnum::LdfldU1Short:
    {
        auto ir = (LdfldU1Short*)codes;
        ir->__code = 177;
        ir->obj = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint8_t)inst.get_field_offset();
        return codes + sizeof(LdfldU1Short);
    }
    case OpCodeEnum::LdfldU1Large:
    {
        auto ir = (LdfldU1Large*)codes;
        ir->__prefix = 253;
        ir->__code = 82;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(LdfldU1Large);
    }
    case OpCodeEnum::LdfldI2:
    {
        auto ir = (LdfldI2*)codes;
        ir->__prefix = 251;
        ir->__code = 194;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdfldI2);
    }
    case OpCodeEnum::LdfldI2Short:
    {
        auto ir = (LdfldI2Short*)codes;
        ir->__code = 178;
        ir->obj = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint8_t)inst.get_field_offset();
        return codes + sizeof(LdfldI2Short);
    }
    case OpCodeEnum::LdfldI2Large:
    {
        auto ir = (LdfldI2Large*)codes;
        ir->__prefix = 253;
        ir->__code = 83;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(LdfldI2Large);
    }
    case OpCodeEnum::LdfldI2Unaligned:
    {
        auto ir = (LdfldI2Unaligned*)codes;
        ir->__prefix = 253;
        ir->__code = 84;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdfldI2Unaligned);
    }
    case OpCodeEnum::LdfldU2:
    {
        auto ir = (LdfldU2*)codes;
        ir->__prefix = 251;
        ir->__code = 195;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdfldU2);
    }
    case OpCodeEnum::LdfldU2Short:
    {
        auto ir = (LdfldU2Short*)codes;
        ir->__code = 179;
        ir->obj = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint8_t)inst.get_field_offset();
        return codes + sizeof(LdfldU2Short);
    }
    case OpCodeEnum::LdfldU2Large:
    {
        auto ir = (LdfldU2Large*)codes;
        ir->__prefix = 253;
        ir->__code = 85;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(LdfldU2Large);
    }
    case OpCodeEnum::LdfldU2Unaligned:
    {
        auto ir = (LdfldU2Unaligned*)codes;
        ir->__prefix = 253;
        ir->__code = 86;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdfldU2Unaligned);
    }
    case OpCodeEnum::LdfldI4:
    {
        auto ir = (LdfldI4*)codes;
        ir->__prefix = 251;
        ir->__code = 196;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdfldI4);
    }
    case OpCodeEnum::LdfldI4Short:
    {
        auto ir = (LdfldI4Short*)codes;
        ir->__code = 180;
        ir->obj = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint8_t)inst.get_field_offset();
        return codes + sizeof(LdfldI4Short);
    }
    case OpCodeEnum::LdfldI4Large:
    {
        auto ir = (LdfldI4Large*)codes;
        ir->__prefix = 253;
        ir->__code = 87;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(LdfldI4Large);
    }
    case OpCodeEnum::LdfldI4Unaligned:
    {
        auto ir = (LdfldI4Unaligned*)codes;
        ir->__prefix = 253;
        ir->__code = 88;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdfldI4Unaligned);
    }
    case OpCodeEnum::LdfldI8:
    {
        auto ir = (LdfldI8*)codes;
        ir->__prefix = 251;
        ir->__code = 197;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdfldI8);
    }
    case OpCodeEnum::LdfldI8Short:
    {
        auto ir = (LdfldI8Short*)codes;
        ir->__code = 181;
        ir->obj = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint8_t)inst.get_field_offset();
        return codes + sizeof(LdfldI8Short);
    }
    case OpCodeEnum::LdfldI8Large:
    {
        auto ir = (LdfldI8Large*)codes;
        ir->__prefix = 253;
        ir->__code = 89;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(LdfldI8Large);
    }
    case OpCodeEnum::LdfldI8Unaligned:
    {
        auto ir = (LdfldI8Unaligned*)codes;
        ir->__prefix = 253;
        ir->__code = 90;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdfldI8Unaligned);
    }
    case OpCodeEnum::LdfldAny:
    {
        auto ir = (LdfldAny*)codes;
        ir->__prefix = 251;
        ir->__code = 198;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        ir->size = (uint16_t)inst.get_field_size();
        return codes + sizeof(LdfldAny);
    }
    case OpCodeEnum::LdfldAnyShort:
    {
        auto ir = (LdfldAnyShort*)codes;
        ir->__code = 182;
        ir->obj = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint8_t)inst.get_field_offset();
        ir->size = (uint8_t)inst.get_field_size();
        return codes + sizeof(LdfldAnyShort);
    }
    case OpCodeEnum::LdfldAnyLarge:
    {
        auto ir = (LdfldAnyLarge*)codes;
        ir->__prefix = 253;
        ir->__code = 91;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        ir->size = (uint32_t)inst.get_field_size();
        return codes + sizeof(LdfldAnyLarge);
    }
    case OpCodeEnum::LdvfldI1:
    {
        auto ir = (LdvfldI1*)codes;
        ir->__prefix = 251;
        ir->__code = 199;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdvfldI1);
    }
    case OpCodeEnum::LdvfldI1Short:
    {
        auto ir = (LdvfldI1Short*)codes;
        ir->__code = 183;
        ir->obj = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint8_t)inst.get_field_offset();
        return codes + sizeof(LdvfldI1Short);
    }
    case OpCodeEnum::LdvfldI1Large:
    {
        auto ir = (LdvfldI1Large*)codes;
        ir->__prefix = 253;
        ir->__code = 92;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(LdvfldI1Large);
    }
    case OpCodeEnum::LdvfldU1:
    {
        auto ir = (LdvfldU1*)codes;
        ir->__prefix = 251;
        ir->__code = 200;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdvfldU1);
    }
    case OpCodeEnum::LdvfldU1Short:
    {
        auto ir = (LdvfldU1Short*)codes;
        ir->__code = 184;
        ir->obj = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint8_t)inst.get_field_offset();
        return codes + sizeof(LdvfldU1Short);
    }
    case OpCodeEnum::LdvfldU1Large:
    {
        auto ir = (LdvfldU1Large*)codes;
        ir->__prefix = 253;
        ir->__code = 93;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(LdvfldU1Large);
    }
    case OpCodeEnum::LdvfldI2:
    {
        auto ir = (LdvfldI2*)codes;
        ir->__prefix = 251;
        ir->__code = 201;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdvfldI2);
    }
    case OpCodeEnum::LdvfldI2Short:
    {
        auto ir = (LdvfldI2Short*)codes;
        ir->__code = 185;
        ir->obj = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint8_t)inst.get_field_offset();
        return codes + sizeof(LdvfldI2Short);
    }
    case OpCodeEnum::LdvfldI2Large:
    {
        auto ir = (LdvfldI2Large*)codes;
        ir->__prefix = 253;
        ir->__code = 94;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(LdvfldI2Large);
    }
    case OpCodeEnum::LdvfldI2Unaligned:
    {
        auto ir = (LdvfldI2Unaligned*)codes;
        ir->__prefix = 253;
        ir->__code = 95;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdvfldI2Unaligned);
    }
    case OpCodeEnum::LdvfldU2:
    {
        auto ir = (LdvfldU2*)codes;
        ir->__prefix = 251;
        ir->__code = 202;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdvfldU2);
    }
    case OpCodeEnum::LdvfldU2Short:
    {
        auto ir = (LdvfldU2Short*)codes;
        ir->__code = 186;
        ir->obj = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint8_t)inst.get_field_offset();
        return codes + sizeof(LdvfldU2Short);
    }
    case OpCodeEnum::LdvfldU2Large:
    {
        auto ir = (LdvfldU2Large*)codes;
        ir->__prefix = 253;
        ir->__code = 96;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(LdvfldU2Large);
    }
    case OpCodeEnum::LdvfldU2Unaligned:
    {
        auto ir = (LdvfldU2Unaligned*)codes;
        ir->__prefix = 253;
        ir->__code = 97;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdvfldU2Unaligned);
    }
    case OpCodeEnum::LdvfldI4:
    {
        auto ir = (LdvfldI4*)codes;
        ir->__prefix = 251;
        ir->__code = 203;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdvfldI4);
    }
    case OpCodeEnum::LdvfldI4Short:
    {
        auto ir = (LdvfldI4Short*)codes;
        ir->__code = 187;
        ir->obj = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint8_t)inst.get_field_offset();
        return codes + sizeof(LdvfldI4Short);
    }
    case OpCodeEnum::LdvfldI4Large:
    {
        auto ir = (LdvfldI4Large*)codes;
        ir->__prefix = 253;
        ir->__code = 98;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(LdvfldI4Large);
    }
    case OpCodeEnum::LdvfldI4Unaligned:
    {
        auto ir = (LdvfldI4Unaligned*)codes;
        ir->__prefix = 253;
        ir->__code = 99;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdvfldI4Unaligned);
    }
    case OpCodeEnum::LdvfldI8:
    {
        auto ir = (LdvfldI8*)codes;
        ir->__prefix = 251;
        ir->__code = 204;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdvfldI8);
    }
    case OpCodeEnum::LdvfldI8Short:
    {
        auto ir = (LdvfldI8Short*)codes;
        ir->__code = 188;
        ir->obj = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint8_t)inst.get_field_offset();
        return codes + sizeof(LdvfldI8Short);
    }
    case OpCodeEnum::LdvfldI8Large:
    {
        auto ir = (LdvfldI8Large*)codes;
        ir->__prefix = 253;
        ir->__code = 100;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(LdvfldI8Large);
    }
    case OpCodeEnum::LdvfldI8Unaligned:
    {
        auto ir = (LdvfldI8Unaligned*)codes;
        ir->__prefix = 253;
        ir->__code = 101;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(LdvfldI8Unaligned);
    }
    case OpCodeEnum::LdvfldAny:
    {
        auto ir = (LdvfldAny*)codes;
        ir->__prefix = 251;
        ir->__code = 205;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        ir->size = (uint16_t)inst.get_field_size();
        return codes + sizeof(LdvfldAny);
    }
    case OpCodeEnum::LdvfldAnyShort:
    {
        auto ir = (LdvfldAnyShort*)codes;
        ir->__code = 189;
        ir->obj = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint8_t)inst.get_field_offset();
        ir->size = (uint8_t)inst.get_field_size();
        return codes + sizeof(LdvfldAnyShort);
    }
    case OpCodeEnum::LdvfldAnyLarge:
    {
        auto ir = (LdvfldAnyLarge*)codes;
        ir->__prefix = 253;
        ir->__code = 102;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        ir->size = (uint32_t)inst.get_field_size();
        return codes + sizeof(LdvfldAnyLarge);
    }
    case OpCodeEnum::Ldflda:
    {
        auto ir = (Ldflda*)codes;
        ir->__prefix = 251;
        ir->__code = 206;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(Ldflda);
    }
    case OpCodeEnum::LdfldaShort:
    {
        auto ir = (LdfldaShort*)codes;
        ir->__code = 190;
        ir->obj = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint8_t)inst.get_field_offset();
        return codes + sizeof(LdfldaShort);
    }
    case OpCodeEnum::LdfldaLarge:
    {
        auto ir = (LdfldaLarge*)codes;
        ir->__prefix = 253;
        ir->__code = 103;
        ir->obj = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(LdfldaLarge);
    }
    case OpCodeEnum::StfldI1:
    {
        auto ir = (StfldI1*)codes;
        ir->__prefix = 251;
        ir->__code = 207;
        ir->obj = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(StfldI1);
    }
    case OpCodeEnum::StfldI1Short:
    {
        auto ir = (StfldI1Short*)codes;
        ir->__code = 191;
        ir->obj = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->offset = (uint8_t)inst.get_field_offset();
        return codes + sizeof(StfldI1Short);
    }
    case OpCodeEnum::StfldI1Large:
    {
        auto ir = (StfldI1Large*)codes;
        ir->__prefix = 253;
        ir->__code = 104;
        ir->obj = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(StfldI1Large);
    }
    case OpCodeEnum::StfldI2:
    {
        auto ir = (StfldI2*)codes;
        ir->__prefix = 251;
        ir->__code = 208;
        ir->obj = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(StfldI2);
    }
    case OpCodeEnum::StfldI2Short:
    {
        auto ir = (StfldI2Short*)codes;
        ir->__code = 192;
        ir->obj = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->offset = (uint8_t)inst.get_field_offset();
        return codes + sizeof(StfldI2Short);
    }
    case OpCodeEnum::StfldI2Large:
    {
        auto ir = (StfldI2Large*)codes;
        ir->__prefix = 253;
        ir->__code = 105;
        ir->obj = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(StfldI2Large);
    }
    case OpCodeEnum::StfldI2Unaligned:
    {
        auto ir = (StfldI2Unaligned*)codes;
        ir->__prefix = 253;
        ir->__code = 106;
        ir->obj = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(StfldI2Unaligned);
    }
    case OpCodeEnum::StfldI4:
    {
        auto ir = (StfldI4*)codes;
        ir->__prefix = 251;
        ir->__code = 209;
        ir->obj = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(StfldI4);
    }
    case OpCodeEnum::StfldI4Short:
    {
        auto ir = (StfldI4Short*)codes;
        ir->__code = 193;
        ir->obj = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->offset = (uint8_t)inst.get_field_offset();
        return codes + sizeof(StfldI4Short);
    }
    case OpCodeEnum::StfldI4Large:
    {
        auto ir = (StfldI4Large*)codes;
        ir->__prefix = 253;
        ir->__code = 107;
        ir->obj = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(StfldI4Large);
    }
    case OpCodeEnum::StfldI4Unaligned:
    {
        auto ir = (StfldI4Unaligned*)codes;
        ir->__prefix = 253;
        ir->__code = 108;
        ir->obj = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(StfldI4Unaligned);
    }
    case OpCodeEnum::StfldI8:
    {
        auto ir = (StfldI8*)codes;
        ir->__prefix = 251;
        ir->__code = 210;
        ir->obj = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(StfldI8);
    }
    case OpCodeEnum::StfldI8Short:
    {
        auto ir = (StfldI8Short*)codes;
        ir->__code = 194;
        ir->obj = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->offset = (uint8_t)inst.get_field_offset();
        return codes + sizeof(StfldI8Short);
    }
    case OpCodeEnum::StfldI8Large:
    {
        auto ir = (StfldI8Large*)codes;
        ir->__prefix = 253;
        ir->__code = 109;
        ir->obj = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        return codes + sizeof(StfldI8Large);
    }
    case OpCodeEnum::StfldI8Unaligned:
    {
        auto ir = (StfldI8Unaligned*)codes;
        ir->__prefix = 253;
        ir->__code = 110;
        ir->obj = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        return codes + sizeof(StfldI8Unaligned);
    }
    case OpCodeEnum::StfldAny:
    {
        auto ir = (StfldAny*)codes;
        ir->__prefix = 251;
        ir->__code = 211;
        ir->obj = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->offset = (uint16_t)inst.get_field_offset();
        ir->size = (uint16_t)inst.get_field_size();
        return codes + sizeof(StfldAny);
    }
    case OpCodeEnum::StfldAnyShort:
    {
        auto ir = (StfldAnyShort*)codes;
        ir->__code = 195;
        ir->obj = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint8_t)inst.get_var_arg2_eval_stack_idx();
        ir->offset = (uint8_t)inst.get_field_offset();
        ir->size = (uint8_t)inst.get_field_size();
        return codes + sizeof(StfldAnyShort);
    }
    case OpCodeEnum::StfldAnyLarge:
    {
        auto ir = (StfldAnyLarge*)codes;
        ir->__prefix = 253;
        ir->__code = 111;
        ir->obj = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        ir->value = (uint16_t)inst.get_var_arg2_eval_stack_idx();
        ir->offset = (uint32_t)inst.get_field_offset();
        ir->size = (uint32_t)inst.get_field_size();
        return codes + sizeof(StfldAnyLarge);
    }
    case OpCodeEnum::LdsfldI1:
    {
        auto ir = (LdsfldI1*)codes;
        ir->__prefix = 251;
        ir->__code = 212;
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->field_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(LdsfldI1);
    }
    case OpCodeEnum::LdsfldI1Short:
    {
        auto ir = (LdsfldI1Short*)codes;
        ir->__code = 196;
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->field_idx = (uint8_t)inst.get_resolved_data_index();
        return codes + sizeof(LdsfldI1Short);
    }
    case OpCodeEnum::LdsfldU1:
    {
        auto ir = (LdsfldU1*)codes;
        ir->__prefix = 251;
        ir->__code = 213;
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->field_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(LdsfldU1);
    }
    case OpCodeEnum::LdsfldU1Short:
    {
        auto ir = (LdsfldU1Short*)codes;
        ir->__code = 197;
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->field_idx = (uint8_t)inst.get_resolved_data_index();
        return codes + sizeof(LdsfldU1Short);
    }
    case OpCodeEnum::LdsfldI2:
    {
        auto ir = (LdsfldI2*)codes;
        ir->__prefix = 251;
        ir->__code = 214;
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->field_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(LdsfldI2);
    }
    case OpCodeEnum::LdsfldI2Short:
    {
        auto ir = (LdsfldI2Short*)codes;
        ir->__code = 198;
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->field_idx = (uint8_t)inst.get_resolved_data_index();
        return codes + sizeof(LdsfldI2Short);
    }
    case OpCodeEnum::LdsfldU2:
    {
        auto ir = (LdsfldU2*)codes;
        ir->__prefix = 251;
        ir->__code = 215;
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->field_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(LdsfldU2);
    }
    case OpCodeEnum::LdsfldU2Short:
    {
        auto ir = (LdsfldU2Short*)codes;
        ir->__code = 199;
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->field_idx = (uint8_t)inst.get_resolved_data_index();
        return codes + sizeof(LdsfldU2Short);
    }
    case OpCodeEnum::LdsfldI4:
    {
        auto ir = (LdsfldI4*)codes;
        ir->__prefix = 251;
        ir->__code = 216;
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->field_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(LdsfldI4);
    }
    case OpCodeEnum::LdsfldI4Short:
    {
        auto ir = (LdsfldI4Short*)codes;
        ir->__code = 200;
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->field_idx = (uint8_t)inst.get_resolved_data_index();
        return codes + sizeof(LdsfldI4Short);
    }
    case OpCodeEnum::LdsfldI8:
    {
        auto ir = (LdsfldI8*)codes;
        ir->__prefix = 251;
        ir->__code = 217;
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->field_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(LdsfldI8);
    }
    case OpCodeEnum::LdsfldI8Short:
    {
        auto ir = (LdsfldI8Short*)codes;
        ir->__code = 201;
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->field_idx = (uint8_t)inst.get_resolved_data_index();
        return codes + sizeof(LdsfldI8Short);
    }
    case OpCodeEnum::LdsfldAny:
    {
        auto ir = (LdsfldAny*)codes;
        ir->__prefix = 251;
        ir->__code = 218;
        ir->field_idx = (uint16_t)inst.get_resolved_data_index();
        ir->size = (uint16_t)inst.get_field_size();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdsfldAny);
    }
    case OpCodeEnum::LdsfldAnyShort:
    {
        auto ir = (LdsfldAnyShort*)codes;
        ir->__code = 202;
        ir->field_idx = (uint8_t)inst.get_resolved_data_index();
        ir->size = (uint8_t)inst.get_field_size();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdsfldAnyShort);
    }
    case OpCodeEnum::Ldsflda:
    {
        auto ir = (Ldsflda*)codes;
        ir->__prefix = 251;
        ir->__code = 219;
        ir->field_idx = (uint16_t)inst.get_resolved_data_index();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(Ldsflda);
    }
    case OpCodeEnum::LdsfldaShort:
    {
        auto ir = (LdsfldaShort*)codes;
        ir->__code = 203;
        ir->field_idx = (uint8_t)inst.get_resolved_data_index();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdsfldaShort);
    }
    case OpCodeEnum::LdsfldRvaData:
    {
        auto ir = (LdsfldRvaData*)codes;
        ir->__prefix = 251;
        ir->__code = 220;
        ir->data = (uint16_t)inst.get_resolved_data_index();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdsfldRvaData);
    }
    case OpCodeEnum::LdsfldRvaDataShort:
    {
        auto ir = (LdsfldRvaDataShort*)codes;
        ir->__code = 204;
        ir->data = (uint8_t)inst.get_resolved_data_index();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        return codes + sizeof(LdsfldRvaDataShort);
    }
    case OpCodeEnum::StsfldI1:
    {
        auto ir = (StsfldI1*)codes;
        ir->__prefix = 251;
        ir->__code = 221;
        ir->field_idx = (uint16_t)inst.get_resolved_data_index();
        ir->value = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        return codes + sizeof(StsfldI1);
    }
    case OpCodeEnum::StsfldI1Short:
    {
        auto ir = (StsfldI1Short*)codes;
        ir->__code = 205;
        ir->field_idx = (uint8_t)inst.get_resolved_data_index();
        ir->value = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        return codes + sizeof(StsfldI1Short);
    }
    case OpCodeEnum::StsfldI2:
    {
        auto ir = (StsfldI2*)codes;
        ir->__prefix = 251;
        ir->__code = 222;
        ir->field_idx = (uint16_t)inst.get_resolved_data_index();
        ir->value = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        return codes + sizeof(StsfldI2);
    }
    case OpCodeEnum::StsfldI2Short:
    {
        auto ir = (StsfldI2Short*)codes;
        ir->__code = 206;
        ir->field_idx = (uint8_t)inst.get_resolved_data_index();
        ir->value = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        return codes + sizeof(StsfldI2Short);
    }
    case OpCodeEnum::StsfldI4:
    {
        auto ir = (StsfldI4*)codes;
        ir->__prefix = 251;
        ir->__code = 223;
        ir->field_idx = (uint16_t)inst.get_resolved_data_index();
        ir->value = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        return codes + sizeof(StsfldI4);
    }
    case OpCodeEnum::StsfldI4Short:
    {
        auto ir = (StsfldI4Short*)codes;
        ir->__code = 207;
        ir->field_idx = (uint8_t)inst.get_resolved_data_index();
        ir->value = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        return codes + sizeof(StsfldI4Short);
    }
    case OpCodeEnum::StsfldI8:
    {
        auto ir = (StsfldI8*)codes;
        ir->__prefix = 251;
        ir->__code = 224;
        ir->field_idx = (uint16_t)inst.get_resolved_data_index();
        ir->value = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        return codes + sizeof(StsfldI8);
    }
    case OpCodeEnum::StsfldI8Short:
    {
        auto ir = (StsfldI8Short*)codes;
        ir->__code = 208;
        ir->field_idx = (uint8_t)inst.get_resolved_data_index();
        ir->value = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        return codes + sizeof(StsfldI8Short);
    }
    case OpCodeEnum::StsfldAny:
    {
        auto ir = (StsfldAny*)codes;
        ir->__prefix = 251;
        ir->__code = 225;
        ir->field_idx = (uint16_t)inst.get_resolved_data_index();
        ir->size = (uint16_t)inst.get_field_size();
        ir->value = (uint16_t)inst.get_var_arg1_eval_stack_idx();
        return codes + sizeof(StsfldAny);
    }
    case OpCodeEnum::StsfldAnyShort:
    {
        auto ir = (StsfldAnyShort*)codes;
        ir->__code = 209;
        ir->field_idx = (uint8_t)inst.get_resolved_data_index();
        ir->size = (uint8_t)inst.get_field_size();
        ir->value = (uint8_t)inst.get_var_arg1_eval_stack_idx();
        return codes + sizeof(StsfldAnyShort);
    }
    case OpCodeEnum::RetVoid:
    {
        auto ir = (RetVoid*)codes;
        ir->__prefix = 251;
        ir->__code = 226;
        return codes + sizeof(RetVoid);
    }
    case OpCodeEnum::RetVoidShort:
    {
        auto ir = (RetVoidShort*)codes;
        ir->__code = 210;
        return codes + sizeof(RetVoidShort);
    }
    case OpCodeEnum::RetI4:
    {
        auto ir = (RetI4*)codes;
        ir->__prefix = 251;
        ir->__code = 227;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(RetI4);
    }
    case OpCodeEnum::RetI8:
    {
        auto ir = (RetI8*)codes;
        ir->__prefix = 251;
        ir->__code = 228;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(RetI8);
    }
    case OpCodeEnum::RetAny:
    {
        auto ir = (RetAny*)codes;
        ir->__prefix = 251;
        ir->__code = 229;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->size = (uint16_t)inst.get_size();
        return codes + sizeof(RetAny);
    }
    case OpCodeEnum::RetI4Short:
    {
        auto ir = (RetI4Short*)codes;
        ir->__code = 211;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(RetI4Short);
    }
    case OpCodeEnum::RetI8Short:
    {
        auto ir = (RetI8Short*)codes;
        ir->__code = 212;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(RetI8Short);
    }
    case OpCodeEnum::RetAnyShort:
    {
        auto ir = (RetAnyShort*)codes;
        ir->__code = 213;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->size = (uint8_t)inst.get_size();
        return codes + sizeof(RetAnyShort);
    }
    case OpCodeEnum::RetNopShort:
    {
        auto ir = (RetNopShort*)codes;
        ir->__code = 214;
        return codes + sizeof(RetNopShort);
    }
    case OpCodeEnum::CallInterp:
    {
        auto ir = (CallInterp*)codes;
        ir->__prefix = 251;
        ir->__code = 230;
        ir->method_idx = (uint16_t)inst.get_resolved_data_index();
        ir->frame_base = (uint16_t)inst.get_frame_base();
        return codes + sizeof(CallInterp);
    }
    case OpCodeEnum::CallInterpShort:
    {
        auto ir = (CallInterpShort*)codes;
        ir->__code = 215;
        ir->method_idx = (uint8_t)inst.get_resolved_data_index();
        ir->frame_base = (uint8_t)inst.get_frame_base();
        return codes + sizeof(CallInterpShort);
    }
    case OpCodeEnum::CallVirtInterp:
    {
        auto ir = (CallVirtInterp*)codes;
        ir->__prefix = 251;
        ir->__code = 231;
        ir->method_idx = (uint16_t)inst.get_resolved_data_index();
        ir->frame_base = (uint16_t)inst.get_frame_base();
        return codes + sizeof(CallVirtInterp);
    }
    case OpCodeEnum::CallVirtInterpShort:
    {
        auto ir = (CallVirtInterpShort*)codes;
        ir->__code = 216;
        ir->method_idx = (uint8_t)inst.get_resolved_data_index();
        ir->frame_base = (uint8_t)inst.get_frame_base();
        return codes + sizeof(CallVirtInterpShort);
    }
    case OpCodeEnum::CallInternalCall:
    {
        auto ir = (CallInternalCall*)codes;
        ir->__prefix = 251;
        ir->__code = 232;
        ir->method_idx = (uint16_t)inst.get_resolved_data_index();
        ir->frame_base = (uint16_t)inst.get_frame_base();
        return codes + sizeof(CallInternalCall);
    }
    case OpCodeEnum::CallInternalCallShort:
    {
        auto ir = (CallInternalCallShort*)codes;
        ir->__code = 217;
        ir->method_idx = (uint8_t)inst.get_resolved_data_index();
        ir->frame_base = (uint8_t)inst.get_frame_base();
        return codes + sizeof(CallInternalCallShort);
    }
    case OpCodeEnum::CallIntrinsic:
    {
        auto ir = (CallIntrinsic*)codes;
        ir->__prefix = 251;
        ir->__code = 233;
        ir->method_idx = (uint16_t)inst.get_resolved_data_index();
        ir->frame_base = (uint16_t)inst.get_frame_base();
        return codes + sizeof(CallIntrinsic);
    }
    case OpCodeEnum::CallIntrinsicShort:
    {
        auto ir = (CallIntrinsicShort*)codes;
        ir->__code = 218;
        ir->method_idx = (uint8_t)inst.get_resolved_data_index();
        ir->frame_base = (uint8_t)inst.get_frame_base();
        return codes + sizeof(CallIntrinsicShort);
    }
    case OpCodeEnum::CallPInvoke:
    {
        auto ir = (CallPInvoke*)codes;
        ir->__prefix = 251;
        ir->__code = 234;
        ir->method_idx = (uint16_t)inst.get_resolved_data_index();
        ir->frame_base = (uint16_t)inst.get_frame_base();
        return codes + sizeof(CallPInvoke);
    }
    case OpCodeEnum::CallPInvokeShort:
    {
        auto ir = (CallPInvokeShort*)codes;
        ir->__code = 219;
        ir->method_idx = (uint8_t)inst.get_resolved_data_index();
        ir->frame_base = (uint8_t)inst.get_frame_base();
        return codes + sizeof(CallPInvokeShort);
    }
    case OpCodeEnum::CallAot:
    {
        auto ir = (CallAot*)codes;
        ir->__prefix = 251;
        ir->__code = 235;
        ir->method_idx = (uint16_t)inst.get_resolved_data_index();
        ir->frame_base = (uint16_t)inst.get_frame_base();
        return codes + sizeof(CallAot);
    }
    case OpCodeEnum::CallAotShort:
    {
        auto ir = (CallAotShort*)codes;
        ir->__code = 220;
        ir->method_idx = (uint8_t)inst.get_resolved_data_index();
        ir->frame_base = (uint8_t)inst.get_frame_base();
        return codes + sizeof(CallAotShort);
    }
    case OpCodeEnum::CallRuntimeImplemented:
    {
        auto ir = (CallRuntimeImplemented*)codes;
        ir->__prefix = 251;
        ir->__code = 236;
        ir->method_idx = (uint16_t)inst.get_resolved_data_index();
        ir->frame_base = (uint16_t)inst.get_frame_base();
        return codes + sizeof(CallRuntimeImplemented);
    }
    case OpCodeEnum::CallRuntimeImplementedShort:
    {
        auto ir = (CallRuntimeImplementedShort*)codes;
        ir->__code = 221;
        ir->method_idx = (uint8_t)inst.get_resolved_data_index();
        ir->frame_base = (uint8_t)inst.get_frame_base();
        return codes + sizeof(CallRuntimeImplementedShort);
    }
    case OpCodeEnum::CalliInterp:
    {
        auto ir = (CalliInterp*)codes;
        ir->__prefix = 251;
        ir->__code = 237;
        ir->method_sig_idx = (uint16_t)inst.get_resolved_data_index();
        ir->method_idx = (uint16_t)inst.get_var_arg3_eval_stack_idx();
        ir->frame_base = (uint16_t)inst.get_frame_base();
        return codes + sizeof(CalliInterp);
    }
    case OpCodeEnum::CalliInterpShort:
    {
        auto ir = (CalliInterpShort*)codes;
        ir->__code = 222;
        ir->method_sig_idx = (uint8_t)inst.get_resolved_data_index();
        ir->method_idx = (uint8_t)inst.get_var_arg3_eval_stack_idx();
        ir->frame_base = (uint8_t)inst.get_frame_base();
        return codes + sizeof(CalliInterpShort);
    }
    case OpCodeEnum::BoxRefInplace:
    {
        auto ir = (BoxRefInplace*)codes;
        ir->__prefix = 251;
        ir->__code = 238;
        ir->src = (uint16_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint16_t)inst.get_var_dst_eval_stack_idx();
        ir->klass_idx = (uint16_t)inst.get_resolved_data_index();
        return codes + sizeof(BoxRefInplace);
    }
    case OpCodeEnum::BoxRefInplaceShort:
    {
        auto ir = (BoxRefInplaceShort*)codes;
        ir->__code = 223;
        ir->src = (uint8_t)inst.get_var_src_eval_stack_idx();
        ir->dst = (uint8_t)inst.get_var_dst_eval_stack_idx();
        ir->klass_idx = (uint8_t)inst.get_resolved_data_index();
        return codes + sizeof(BoxRefInplaceShort);
    }
    case OpCodeEnum::NewObjInterp:
    {
        auto ir = (NewObjInterp*)codes;
        ir->__prefix = 251;
        ir->__code = 239;
        ir->method_idx = (uint16_t)inst.get_resolved_data_index();
        ir->frame_base = (uint16_t)inst.get_frame_base();
        ir->total_params_stack_object_size = (uint32_t)inst.get_total_params_stack_object_size();
        return codes + sizeof(NewObjInterp);
    }
    case OpCodeEnum::NewObjInterpShort:
    {
        auto ir = (NewObjInterpShort*)codes;
        ir->__code = 224;
        ir->method_idx = (uint8_t)inst.get_resolved_data_index();
        ir->frame_base = (uint8_t)inst.get_frame_base();
        ir->total_params_stack_object_size = (uint32_t)inst.get_total_params_stack_object_size();
        return codes + sizeof(NewObjInterpShort);
    }
    case OpCodeEnum::NewValueTypeInterp:
    {
        auto ir = (NewValueTypeInterp*)codes;
        ir->__prefix = 251;
        ir->__code = 240;
        ir->method_idx = (uint16_t)inst.get_resolved_data_index();
        ir->frame_base = (uint16_t)inst.get_frame_base();
        ir->total_params_stack_object_size = (uint32_t)inst.get_total_params_stack_object_size();
        return codes + sizeof(NewValueTypeInterp);
    }
    case OpCodeEnum::NewValueTypeInterpShort:
    {
        auto ir = (NewValueTypeInterpShort*)codes;
        ir->__code = 225;
        ir->method_idx = (uint8_t)inst.get_resolved_data_index();
        ir->frame_base = (uint8_t)inst.get_frame_base();
        ir->total_params_stack_object_size = (uint32_t)inst.get_total_params_stack_object_size();
        return codes + sizeof(NewValueTypeInterpShort);
    }
    case OpCodeEnum::NewObjInternalCall:
    {
        auto ir = (NewObjInternalCall*)codes;
        ir->__prefix = 251;
        ir->__code = 241;
        ir->method_idx = (uint16_t)inst.get_resolved_data_index();
        ir->invoker_idx = (uint16_t)inst.get_invoker_idx();
        ir->frame_base = (uint16_t)inst.get_frame_base();
        return codes + sizeof(NewObjInternalCall);
    }
    case OpCodeEnum::NewObjInternalCallShort:
    {
        auto ir = (NewObjInternalCallShort*)codes;
        ir->__code = 226;
        ir->method_idx = (uint8_t)inst.get_resolved_data_index();
        ir->invoker_idx = (uint8_t)inst.get_invoker_idx();
        ir->frame_base = (uint8_t)inst.get_frame_base();
        return codes + sizeof(NewObjInternalCallShort);
    }
    case OpCodeEnum::NewObjIntrinsic:
    {
        auto ir = (NewObjIntrinsic*)codes;
        ir->__prefix = 251;
        ir->__code = 242;
        ir->method_idx = (uint16_t)inst.get_resolved_data_index();
        ir->invoker_idx = (uint16_t)inst.get_invoker_idx();
        ir->frame_base = (uint16_t)inst.get_frame_base();
        return codes + sizeof(NewObjIntrinsic);
    }
    case OpCodeEnum::NewObjIntrinsicShort:
    {
        auto ir = (NewObjIntrinsicShort*)codes;
        ir->__code = 227;
        ir->method_idx = (uint8_t)inst.get_resolved_data_index();
        ir->invoker_idx = (uint8_t)inst.get_invoker_idx();
        ir->frame_base = (uint8_t)inst.get_frame_base();
        return codes + sizeof(NewObjIntrinsicShort);
    }
    case OpCodeEnum::NewObjAot:
    {
        auto ir = (NewObjAot*)codes;
        ir->__prefix = 251;
        ir->__code = 243;
        ir->method_idx = (uint16_t)inst.get_resolved_data_index();
        ir->frame_base = (uint16_t)inst.get_frame_base();
        ir->total_params_stack_object_size = (uint32_t)inst.get_total_params_stack_object_size();
        return codes + sizeof(NewObjAot);
    }
    case OpCodeEnum::NewObjAotShort:
    {
        auto ir = (NewObjAotShort*)codes;
        ir->__code = 228;
        ir->method_idx = (uint8_t)inst.get_resolved_data_index();
        ir->frame_base = (uint8_t)inst.get_frame_base();
        ir->total_params_stack_object_size = (uint32_t)inst.get_total_params_stack_object_size();
        return codes + sizeof(NewObjAotShort);
    }
    case OpCodeEnum::NewValueTypeAot:
    {
        auto ir = (NewValueTypeAot*)codes;
        ir->__prefix = 251;
        ir->__code = 244;
        ir->method_idx = (uint16_t)inst.get_resolved_data_index();
        ir->frame_base = (uint16_t)inst.get_frame_base();
        ir->total_params_stack_object_size = (uint32_t)inst.get_total_params_stack_object_size();
        return codes + sizeof(NewValueTypeAot);
    }
    case OpCodeEnum::NewValueTypeAotShort:
    {
        auto ir = (NewValueTypeAotShort*)codes;
        ir->__code = 229;
        ir->method_idx = (uint8_t)inst.get_resolved_data_index();
        ir->frame_base = (uint8_t)inst.get_frame_base();
        ir->total_params_stack_object_size = (uint32_t)inst.get_total_params_stack_object_size();
        return codes + sizeof(NewValueTypeAotShort);
    }
    case OpCodeEnum::Throw:
    {
        auto ir = (Throw*)codes;
        ir->__prefix = 251;
        ir->__code = 245;
        ir->ex = (uint16_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(Throw);
    }
    case OpCodeEnum::ThrowShort:
    {
        auto ir = (ThrowShort*)codes;
        ir->__code = 230;
        ir->ex = (uint8_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(ThrowShort);
    }
    case OpCodeEnum::Rethrow:
    {
        auto ir = (Rethrow*)codes;
        ir->__prefix = 251;
        ir->__code = 246;
        return codes + sizeof(Rethrow);
    }
    case OpCodeEnum::RethrowShort:
    {
        auto ir = (RethrowShort*)codes;
        ir->__code = 231;
        return codes + sizeof(RethrowShort);
    }
    case OpCodeEnum::LeaveTryWithFinally:
    {
        auto ir = (LeaveTryWithFinally*)codes;
        ir->__prefix = 251;
        ir->__code = 247;
        ir->first_finally_clause_index = (uint8_t)inst.get_first_finally_clause_index();
        ir->finally_clauses_count = (uint8_t)inst.get_finally_clauses_count();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(LeaveTryWithFinally);
    }
    case OpCodeEnum::LeaveTryWithFinallyShort:
    {
        auto ir = (LeaveTryWithFinallyShort*)codes;
        ir->__code = 232;
        ir->first_finally_clause_index = (uint8_t)inst.get_first_finally_clause_index();
        ir->target_offset = (int8_t)inst.get_branch_target_offset();
        ir->finally_clauses_count = (uint8_t)inst.get_finally_clauses_count();
        return codes + sizeof(LeaveTryWithFinallyShort);
    }
    case OpCodeEnum::LeaveCatchWithFinally:
    {
        auto ir = (LeaveCatchWithFinally*)codes;
        ir->__prefix = 251;
        ir->__code = 248;
        ir->first_finally_clause_index = (uint8_t)inst.get_first_finally_clause_index();
        ir->finally_clauses_count = (uint8_t)inst.get_finally_clauses_count();
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(LeaveCatchWithFinally);
    }
    case OpCodeEnum::LeaveCatchWithFinallyShort:
    {
        auto ir = (LeaveCatchWithFinallyShort*)codes;
        ir->__code = 233;
        ir->first_finally_clause_index = (uint8_t)inst.get_first_finally_clause_index();
        ir->target_offset = (int8_t)inst.get_branch_target_offset();
        ir->finally_clauses_count = (uint8_t)inst.get_finally_clauses_count();
        return codes + sizeof(LeaveCatchWithFinallyShort);
    }
    case OpCodeEnum::LeaveCatchWithoutFinally:
    {
        auto ir = (LeaveCatchWithoutFinally*)codes;
        ir->__prefix = 251;
        ir->__code = 249;
        ir->target_offset = (int32_t)inst.get_branch_target_offset();
        return codes + sizeof(LeaveCatchWithoutFinally);
    }
    case OpCodeEnum::LeaveCatchWithoutFinallyShort:
    {
        auto ir = (LeaveCatchWithoutFinallyShort*)codes;
        ir->__code = 234;
        ir->target_offset = (int8_t)inst.get_branch_target_offset();
        return codes + sizeof(LeaveCatchWithoutFinallyShort);
    }
    case OpCodeEnum::EndFilter:
    {
        auto ir = (EndFilter*)codes;
        ir->__prefix = 251;
        ir->__code = 250;
        ir->cond = (uint16_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(EndFilter);
    }
    case OpCodeEnum::EndFilterShort:
    {
        auto ir = (EndFilterShort*)codes;
        ir->__code = 235;
        ir->cond = (uint8_t)inst.get_var_src_eval_stack_idx();
        return codes + sizeof(EndFilterShort);
    }
    case OpCodeEnum::EndFinally:
    {
        auto ir = (EndFinally*)codes;
        ir->__prefix = 251;
        ir->__code = 251;
        return codes + sizeof(EndFinally);
    }
    case OpCodeEnum::EndFinallyShort:
    {
        auto ir = (EndFinallyShort*)codes;
        ir->__code = 236;
        return codes + sizeof(EndFinallyShort);
    }
    case OpCodeEnum::EndFault:
    {
        auto ir = (EndFault*)codes;
        ir->__prefix = 251;
        ir->__code = 252;
        return codes + sizeof(EndFault);
    }
    case OpCodeEnum::EndFaultShort:
    {
        auto ir = (EndFaultShort*)codes;
        ir->__code = 237;
        return codes + sizeof(EndFaultShort);
    }
    case OpCodeEnum::GetEnumLongHashCode:
    {
        auto ir = (GetEnumLongHashCode*)codes;
        ir->__prefix = 252;
        ir->__code = 50;
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
