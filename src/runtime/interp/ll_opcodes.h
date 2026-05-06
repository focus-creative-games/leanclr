#pragma once

#include "vm/rt_managed_types.h"
#include "interp_defs.h"
#include "vm/field.h"

namespace leanclr
{
namespace interp
{
namespace ll
{

enum class OpCodeEnum
{
    //{{LOW_LEVEL_OPCODE_ENUM
    Illegal,
    Nop,
    InitLocals1Short,
    InitLocals2Short,
    InitLocals3Short,
    InitLocals4Short,
    InitLocals,
    InitLocalsShort,
    Arglist,
    LdLocI1,
    LdLocU1,
    LdLocI2,
    LdLocU2,
    LdLocI4,
    LdLocI8,
    LdLocAny,
    LdLocI1Short,
    LdLocU1Short,
    LdLocI2Short,
    LdLocU2Short,
    LdLocI4Short,
    LdLocI8Short,
    LdLocAnyShort,
    LdLoca,
    LdLocaShort,
    StLocI1,
    StLocI2,
    StLocI4,
    StLocI8,
    StLocAny,
    StLocI1Short,
    StLocI2Short,
    StLocI4Short,
    StLocI8Short,
    StLocAnyShort,
    LdNull,
    LdNullShort,
    LdcI4I2,
    LdcI4I2Short,
    LdcI4I4,
    LdcI4I4Short,
    LdcI8I2,
    LdcI8I2Short,
    LdcI8I4,
    LdcI8I4Short,
    LdcI8I8,
    LdcI8I8Short,
    LdStr,
    LdStrShort,
    Br,
    BrShort,
    BrTrueI4,
    BrTrueI4Short,
    BrTrueI8,
    BrTrueI8Short,
    BrFalseI4,
    BrFalseI4Short,
    BrFalseI8,
    BrFalseI8Short,
    BeqI4,
    BeqI8,
    BeqR4,
    BeqR8,
    BeqI4Short,
    BeqI8Short,
    BgeI4,
    BgeI8,
    BgeR4,
    BgeR8,
    BgeI4Short,
    BgeI8Short,
    BgtI4,
    BgtI8,
    BgtR4,
    BgtR8,
    BgtI4Short,
    BgtI8Short,
    BleI4,
    BleI8,
    BleR4,
    BleR8,
    BleI4Short,
    BleI8Short,
    BltI4,
    BltI8,
    BltR4,
    BltR8,
    BltI4Short,
    BltI8Short,
    BneUnI4,
    BneUnI8,
    BneUnR4,
    BneUnR8,
    BneUnI4Short,
    BneUnI8Short,
    BgeUnI4,
    BgeUnI8,
    BgeUnR4,
    BgeUnR8,
    BgeUnI4Short,
    BgeUnI8Short,
    BgtUnI4,
    BgtUnI8,
    BgtUnR4,
    BgtUnR8,
    BgtUnI4Short,
    BgtUnI8Short,
    BleUnI4,
    BleUnI8,
    BleUnR4,
    BleUnR8,
    BleUnI4Short,
    BleUnI8Short,
    BltUnI4,
    BltUnI8,
    BltUnR4,
    BltUnR8,
    BltUnI4Short,
    BltUnI8Short,
    Switch,
    LdIndI1,
    LdIndI1Short,
    LdIndU1,
    LdIndU1Short,
    LdIndI2,
    LdIndI2Short,
    LdIndI2Unaligned,
    LdIndU2,
    LdIndU2Short,
    LdIndU2Unaligned,
    LdIndI4,
    LdIndI4Short,
    LdIndI4Unaligned,
    LdIndI8,
    LdIndI8Short,
    LdIndI8Unaligned,
    StIndI1,
    StIndI1Short,
    StIndI2,
    StIndI2Short,
    StIndI2Unaligned,
    StIndI4,
    StIndI4Short,
    StIndI4Unaligned,
    StIndI8,
    StIndI8Short,
    StIndI8Unaligned,
    StIndI8I4,
    StIndI8I4Short,
    StIndI8I4Unaligned,
    StIndI8U4,
    StIndI8U4Short,
    StIndI8U4Unaligned,
    AddI4,
    AddI8,
    AddR4,
    AddR8,
    AddI4Short,
    AddI8Short,
    AddR4Short,
    AddR8Short,
    SubI4,
    SubI8,
    SubR4,
    SubR8,
    SubI4Short,
    SubI8Short,
    SubR4Short,
    SubR8Short,
    MulI4,
    MulI8,
    MulR4,
    MulR8,
    MulI4Short,
    MulI8Short,
    MulR4Short,
    MulR8Short,
    DivI4,
    DivI8,
    DivR4,
    DivR8,
    DivI4Short,
    DivI8Short,
    DivR4Short,
    DivR8Short,
    DivUnI4,
    DivUnI8,
    DivUnI4Short,
    DivUnI8Short,
    RemI4,
    RemI8,
    RemR4,
    RemR8,
    RemI4Short,
    RemI8Short,
    RemR4Short,
    RemR8Short,
    RemUnI4,
    RemUnI8,
    RemUnI4Short,
    RemUnI8Short,
    AndI4,
    AndI8,
    AndI4Short,
    AndI8Short,
    OrI4,
    OrI8,
    OrI4Short,
    OrI8Short,
    XorI4,
    XorI8,
    XorI4Short,
    XorI8Short,
    ShlI4,
    ShlI8,
    ShlI4Short,
    ShrI4,
    ShrI8,
    ShrI4Short,
    ShrUnI4,
    ShrUnI8,
    ShrUnI4Short,
    NegI4,
    NegI8,
    NegR4,
    NegR8,
    NegI4Short,
    NegI8Short,
    NegR4Short,
    NegR8Short,
    NotI4,
    NotI8,
    NotI4Short,
    NotI8Short,
    AddOvfI4,
    AddOvfI8,
    AddOvfUnI4,
    AddOvfUnI8,
    MulOvfI4,
    MulOvfI8,
    MulOvfUnI4,
    MulOvfUnI8,
    SubOvfI4,
    SubOvfI8,
    SubOvfUnI4,
    SubOvfUnI8,
    ConvI1I4,
    ConvI1I8,
    ConvI1R4,
    ConvI1R8,
    ConvI1I4Short,
    ConvI1I8Short,
    ConvI1R4Short,
    ConvI1R8Short,
    ConvU1I4,
    ConvU1I8,
    ConvU1R4,
    ConvU1R8,
    ConvU1I4Short,
    ConvU1I8Short,
    ConvU1R4Short,
    ConvU1R8Short,
    ConvI2I4,
    ConvI2I8,
    ConvI2R4,
    ConvI2R8,
    ConvI2I4Short,
    ConvI2I8Short,
    ConvI2R4Short,
    ConvI2R8Short,
    ConvU2I4,
    ConvU2I8,
    ConvU2R4,
    ConvU2R8,
    ConvU2I4Short,
    ConvU2I8Short,
    ConvU2R4Short,
    ConvU2R8Short,
    ConvI4I8,
    ConvI4R4,
    ConvI4R8,
    ConvI4I8Short,
    ConvI4R4Short,
    ConvI4R8Short,
    ConvU4I8,
    ConvU4R4,
    ConvU4R8,
    ConvU4I8Short,
    ConvU4R4Short,
    ConvU4R8Short,
    ConvI8I4,
    ConvI8U4,
    ConvI8R4,
    ConvI8R8,
    ConvI8I4Short,
    ConvI8R4Short,
    ConvI8R8Short,
    ConvU8I4,
    ConvU8R4,
    ConvU8R8,
    ConvR4I4,
    ConvR4I8,
    ConvR4R8,
    ConvR4I4Short,
    ConvR4I8Short,
    ConvR4R8Short,
    ConvR8I4,
    ConvR8I8,
    ConvR8R4,
    ConvR8I4Short,
    ConvR8I8Short,
    ConvR8R4Short,
    ConvOvfI1I4,
    ConvOvfI1I8,
    ConvOvfI1R4,
    ConvOvfI1R8,
    ConvOvfU1I4,
    ConvOvfU1I8,
    ConvOvfU1R4,
    ConvOvfU1R8,
    ConvOvfI2I4,
    ConvOvfI2I8,
    ConvOvfI2R4,
    ConvOvfI2R8,
    ConvOvfU2I4,
    ConvOvfU2I8,
    ConvOvfU2R4,
    ConvOvfU2R8,
    ConvOvfI4I8,
    ConvOvfI4R4,
    ConvOvfI4R8,
    ConvOvfU4I4,
    ConvOvfU4I8,
    ConvOvfU4R4,
    ConvOvfU4R8,
    ConvOvfI8R4,
    ConvOvfI8R8,
    ConvOvfU8I4,
    ConvOvfU8I8,
    ConvOvfU8R4,
    ConvOvfU8R8,
    ConvOvfI1UnI4,
    ConvOvfI1UnI8,
    ConvOvfI1UnR4,
    ConvOvfI1UnR8,
    ConvOvfU1UnI4,
    ConvOvfU1UnI8,
    ConvOvfU1UnR4,
    ConvOvfU1UnR8,
    ConvOvfI2UnI4,
    ConvOvfI2UnI8,
    ConvOvfI2UnR4,
    ConvOvfI2UnR8,
    ConvOvfU2UnI4,
    ConvOvfU2UnI8,
    ConvOvfU2UnR4,
    ConvOvfU2UnR8,
    ConvOvfI4UnI4,
    ConvOvfI4UnI8,
    ConvOvfI4UnR4,
    ConvOvfI4UnR8,
    ConvOvfU4UnI8,
    ConvOvfU4UnR4,
    ConvOvfU4UnR8,
    ConvOvfI8UnI8,
    ConvOvfI8UnR4,
    ConvOvfI8UnR8,
    ConvOvfU8UnR4,
    ConvOvfU8UnR8,
    CeqI4,
    CeqI8,
    CeqR4,
    CeqR8,
    CeqI4Short,
    CeqI8Short,
    CeqR4Short,
    CeqR8Short,
    CgtI4,
    CgtI8,
    CgtR4,
    CgtR8,
    CgtI4Short,
    CgtI8Short,
    CgtUnI4,
    CgtUnI8,
    CgtUnR4,
    CgtUnR8,
    CgtUnI4Short,
    CgtUnI8Short,
    CltI4,
    CltI8,
    CltR4,
    CltR8,
    CltI4Short,
    CltI8Short,
    CltUnI4,
    CltUnI8,
    CltUnR4,
    CltUnR8,
    CltUnI4Short,
    CltUnI8Short,
    InitObjI1,
    InitObjI1Short,
    InitObjI2,
    InitObjI2Short,
    InitObjI2Unaligned,
    InitObjI4,
    InitObjI4Short,
    InitObjI4Unaligned,
    InitObjI8,
    InitObjI8Short,
    InitObjI8Unaligned,
    InitObjAny,
    InitObjAnyShort,
    CpObjI1,
    CpObjI1Short,
    CpObjI2,
    CpObjI2Short,
    CpObjI4,
    CpObjI4Short,
    CpObjI8,
    CpObjI8Short,
    CpObjAny,
    CpObjAnyShort,
    LdObjAny,
    LdObjAnyShort,
    StObjAny,
    StObjAnyShort,
    CastClass,
    CastClassShort,
    IsInst,
    IsInstShort,
    Box,
    BoxShort,
    Unbox,
    UnboxShort,
    UnboxAny,
    UnboxAnyShort,
    NewArr,
    NewArrShort,
    LdLen,
    LdLenShort,
    Ldelema,
    LdelemaShort,
    LdelemaReadOnly,
    LdelemI1,
    LdelemI1Short,
    LdelemU1,
    LdelemU1Short,
    LdelemI2,
    LdelemI2Short,
    LdelemU2,
    LdelemU2Short,
    LdelemI4,
    LdelemI4Short,
    LdelemI8,
    LdelemI8Short,
    LdelemI,
    LdelemIShort,
    LdelemR4,
    LdelemR4Short,
    LdelemR8,
    LdelemR8Short,
    LdelemRef,
    LdelemRefShort,
    LdelemAnyRef,
    LdelemAnyRefShort,
    LdelemAnyVal,
    LdelemAnyValShort,
    StelemI1,
    StelemI1Short,
    StelemI2,
    StelemI2Short,
    StelemI4,
    StelemI4Short,
    StelemI8,
    StelemI8Short,
    StelemI,
    StelemIShort,
    StelemR4,
    StelemR4Short,
    StelemR8,
    StelemR8Short,
    StelemRef,
    StelemRefShort,
    StelemAnyRef,
    StelemAnyRefShort,
    StelemAnyVal,
    StelemAnyValShort,
    MkRefAny,
    RefAnyVal,
    RefAnyType,
    LdToken,
    CkfiniteR4,
    CkfiniteR8,
    LocAlloc,
    InitBlk,
    CpBlk,
    Ldftn,
    LdftnShort,
    Ldvirtftn,
    LdvirtftnShort,
    LdfldI1,
    LdfldI1Short,
    LdfldI1Large,
    LdfldU1,
    LdfldU1Short,
    LdfldU1Large,
    LdfldI2,
    LdfldI2Short,
    LdfldI2Large,
    LdfldI2Unaligned,
    LdfldU2,
    LdfldU2Short,
    LdfldU2Large,
    LdfldU2Unaligned,
    LdfldI4,
    LdfldI4Short,
    LdfldI4Large,
    LdfldI4Unaligned,
    LdfldI8,
    LdfldI8Short,
    LdfldI8Large,
    LdfldI8Unaligned,
    LdfldAny,
    LdfldAnyShort,
    LdfldAnyLarge,
    LdvfldI1,
    LdvfldI1Short,
    LdvfldI1Large,
    LdvfldU1,
    LdvfldU1Short,
    LdvfldU1Large,
    LdvfldI2,
    LdvfldI2Short,
    LdvfldI2Large,
    LdvfldI2Unaligned,
    LdvfldU2,
    LdvfldU2Short,
    LdvfldU2Large,
    LdvfldU2Unaligned,
    LdvfldI4,
    LdvfldI4Short,
    LdvfldI4Large,
    LdvfldI4Unaligned,
    LdvfldI8,
    LdvfldI8Short,
    LdvfldI8Large,
    LdvfldI8Unaligned,
    LdvfldAny,
    LdvfldAnyShort,
    LdvfldAnyLarge,
    Ldflda,
    LdfldaShort,
    LdfldaLarge,
    StfldI1,
    StfldI1Short,
    StfldI1Large,
    StfldI2,
    StfldI2Short,
    StfldI2Large,
    StfldI2Unaligned,
    StfldI4,
    StfldI4Short,
    StfldI4Large,
    StfldI4Unaligned,
    StfldI8,
    StfldI8Short,
    StfldI8Large,
    StfldI8Unaligned,
    StfldAny,
    StfldAnyShort,
    StfldAnyLarge,
    LdsfldI1,
    LdsfldI1Short,
    LdsfldU1,
    LdsfldU1Short,
    LdsfldI2,
    LdsfldI2Short,
    LdsfldU2,
    LdsfldU2Short,
    LdsfldI4,
    LdsfldI4Short,
    LdsfldI8,
    LdsfldI8Short,
    LdsfldAny,
    LdsfldAnyShort,
    Ldsflda,
    LdsfldaShort,
    LdsfldRvaData,
    LdsfldRvaDataShort,
    StsfldI1,
    StsfldI1Short,
    StsfldI2,
    StsfldI2Short,
    StsfldI4,
    StsfldI4Short,
    StsfldI8,
    StsfldI8Short,
    StsfldAny,
    StsfldAnyShort,
    RetVoid,
    RetVoidShort,
    RetI4,
    RetI8,
    RetAny,
    RetI4Short,
    RetI8Short,
    RetAnyShort,
    RetNopShort,
    CallInterp,
    CallInterpShort,
    CallVirtInterp,
    CallVirtInterpShort,
    CallInternalCall,
    CallInternalCallShort,
    CallIntrinsic,
    CallIntrinsicShort,
    CallPInvoke,
    CallPInvokeShort,
    CallAot,
    CallAotShort,
    CallRuntimeImplemented,
    CallRuntimeImplementedShort,
    CalliInterp,
    CalliInterpShort,
    BoxRefInplace,
    BoxRefInplaceShort,
    NewObjInterp,
    NewObjInterpShort,
    NewValueTypeInterp,
    NewValueTypeInterpShort,
    NewObjInternalCall,
    NewObjInternalCallShort,
    NewObjIntrinsic,
    NewObjIntrinsicShort,
    NewObjAot,
    NewObjAotShort,
    NewValueTypeAot,
    NewValueTypeAotShort,
    Throw,
    ThrowShort,
    Rethrow,
    RethrowShort,
    LeaveTryWithFinally,
    LeaveTryWithFinallyShort,
    LeaveCatchWithFinally,
    LeaveCatchWithFinallyShort,
    LeaveCatchWithoutFinally,
    LeaveCatchWithoutFinallyShort,
    EndFilter,
    EndFilterShort,
    EndFinally,
    EndFinallyShort,
    EndFault,
    EndFaultShort,
    GetEnumLongHashCode,

    //}}LOW_LEVEL_OPCODE_ENUMM
    __Count,
};

enum class OpCodeValue0 : uint8_t
{
    //{{LOW_LEVEL_OPCODE0
    InitLocals1Short = 0x00,
    InitLocals2Short = 0x01,
    InitLocals3Short = 0x02,
    InitLocals4Short = 0x03,
    InitLocalsShort = 0x04,
    LdLocI1Short = 0x05,
    LdLocU1Short = 0x06,
    LdLocI2Short = 0x07,
    LdLocU2Short = 0x08,
    LdLocI4Short = 0x09,
    LdLocI8Short = 0x0A,
    LdLocAnyShort = 0x0B,
    LdLocaShort = 0x0C,
    StLocI1Short = 0x0D,
    StLocI2Short = 0x0E,
    StLocI4Short = 0x0F,
    StLocI8Short = 0x10,
    StLocAnyShort = 0x11,
    LdNullShort = 0x12,
    LdcI4I2Short = 0x13,
    LdcI4I4Short = 0x14,
    LdcI8I2Short = 0x15,
    LdcI8I4Short = 0x16,
    LdcI8I8Short = 0x17,
    LdStrShort = 0x18,
    BrShort = 0x19,
    BrTrueI4Short = 0x1A,
    BrTrueI8Short = 0x1B,
    BrFalseI4Short = 0x1C,
    BrFalseI8Short = 0x1D,
    BeqI4Short = 0x1E,
    BeqI8Short = 0x1F,
    BgeI4Short = 0x20,
    BgeI8Short = 0x21,
    BgtI4Short = 0x22,
    BgtI8Short = 0x23,
    BleI4Short = 0x24,
    BleI8Short = 0x25,
    BltI4Short = 0x26,
    BltI8Short = 0x27,
    BneUnI4Short = 0x28,
    BneUnI8Short = 0x29,
    BgeUnI4Short = 0x2A,
    BgeUnI8Short = 0x2B,
    BgtUnI4Short = 0x2C,
    BgtUnI8Short = 0x2D,
    BleUnI4Short = 0x2E,
    BleUnI8Short = 0x2F,
    BltUnI4Short = 0x30,
    BltUnI8Short = 0x31,
    AddI4Short = 0x32,
    AddI8Short = 0x33,
    AddR4Short = 0x34,
    AddR8Short = 0x35,
    SubI4Short = 0x36,
    SubI8Short = 0x37,
    SubR4Short = 0x38,
    SubR8Short = 0x39,
    MulI4Short = 0x3A,
    MulI8Short = 0x3B,
    MulR4Short = 0x3C,
    MulR8Short = 0x3D,
    DivI4Short = 0x3E,
    DivI8Short = 0x3F,
    DivR4Short = 0x40,
    DivR8Short = 0x41,
    DivUnI4Short = 0x42,
    DivUnI8Short = 0x43,
    RemI4Short = 0x44,
    RemI8Short = 0x45,
    RemR4Short = 0x46,
    RemR8Short = 0x47,
    RemUnI4Short = 0x48,
    RemUnI8Short = 0x49,
    AndI4Short = 0x4A,
    AndI8Short = 0x4B,
    OrI4Short = 0x4C,
    OrI8Short = 0x4D,
    XorI4Short = 0x4E,
    XorI8Short = 0x4F,
    ShlI4Short = 0x50,
    ShrI4Short = 0x51,
    ShrUnI4Short = 0x52,
    NegI4Short = 0x53,
    NegI8Short = 0x54,
    NegR4Short = 0x55,
    NegR8Short = 0x56,
    NotI4Short = 0x57,
    NotI8Short = 0x58,
    ConvI1I4Short = 0x59,
    ConvI1I8Short = 0x5A,
    ConvI1R4Short = 0x5B,
    ConvI1R8Short = 0x5C,
    ConvU1I4Short = 0x5D,
    ConvU1I8Short = 0x5E,
    ConvU1R4Short = 0x5F,
    ConvU1R8Short = 0x60,
    ConvI2I4Short = 0x61,
    ConvI2I8Short = 0x62,
    ConvI2R4Short = 0x63,
    ConvI2R8Short = 0x64,
    ConvU2I4Short = 0x65,
    ConvU2I8Short = 0x66,
    ConvU2R4Short = 0x67,
    ConvU2R8Short = 0x68,
    ConvI4I8Short = 0x69,
    ConvI4R4Short = 0x6A,
    ConvI4R8Short = 0x6B,
    ConvU4I8Short = 0x6C,
    ConvU4R4Short = 0x6D,
    ConvU4R8Short = 0x6E,
    ConvI8I4Short = 0x6F,
    ConvI8R4Short = 0x70,
    ConvI8R8Short = 0x71,
    ConvR4I4Short = 0x72,
    ConvR4I8Short = 0x73,
    ConvR4R8Short = 0x74,
    ConvR8I4Short = 0x75,
    ConvR8I8Short = 0x76,
    ConvR8R4Short = 0x77,
    CeqI4Short = 0x78,
    CeqI8Short = 0x79,
    CeqR4Short = 0x7A,
    CeqR8Short = 0x7B,
    CgtI4Short = 0x7C,
    CgtI8Short = 0x7D,
    CgtUnI4Short = 0x7E,
    CgtUnI8Short = 0x7F,
    CltI4Short = 0x80,
    CltI8Short = 0x81,
    CltUnI4Short = 0x82,
    CltUnI8Short = 0x83,
    InitObjI1Short = 0x84,
    InitObjI2Short = 0x85,
    InitObjI4Short = 0x86,
    InitObjI8Short = 0x87,
    InitObjAnyShort = 0x88,
    CpObjI1Short = 0x89,
    CpObjI2Short = 0x8A,
    CpObjI4Short = 0x8B,
    CpObjI8Short = 0x8C,
    CpObjAnyShort = 0x8D,
    LdObjAnyShort = 0x8E,
    StObjAnyShort = 0x8F,
    CastClassShort = 0x90,
    IsInstShort = 0x91,
    BoxShort = 0x92,
    UnboxShort = 0x93,
    UnboxAnyShort = 0x94,
    NewArrShort = 0x95,
    LdLenShort = 0x96,
    LdelemaShort = 0x97,
    LdelemI1Short = 0x98,
    LdelemU1Short = 0x99,
    LdelemI2Short = 0x9A,
    LdelemU2Short = 0x9B,
    LdelemI4Short = 0x9C,
    LdelemI8Short = 0x9D,
    LdelemIShort = 0x9E,
    LdelemR4Short = 0x9F,
    LdelemR8Short = 0xA0,
    LdelemRefShort = 0xA1,
    LdelemAnyRefShort = 0xA2,
    LdelemAnyValShort = 0xA3,
    StelemI1Short = 0xA4,
    StelemI2Short = 0xA5,
    StelemI4Short = 0xA6,
    StelemI8Short = 0xA7,
    StelemIShort = 0xA8,
    StelemR4Short = 0xA9,
    StelemR8Short = 0xAA,
    StelemRefShort = 0xAB,
    StelemAnyRefShort = 0xAC,
    StelemAnyValShort = 0xAD,
    LdftnShort = 0xAE,
    LdvirtftnShort = 0xAF,
    LdfldI1Short = 0xB0,
    LdfldU1Short = 0xB1,
    LdfldI2Short = 0xB2,
    LdfldU2Short = 0xB3,
    LdfldI4Short = 0xB4,
    LdfldI8Short = 0xB5,
    LdfldAnyShort = 0xB6,
    LdvfldI1Short = 0xB7,
    LdvfldU1Short = 0xB8,
    LdvfldI2Short = 0xB9,
    LdvfldU2Short = 0xBA,
    LdvfldI4Short = 0xBB,
    LdvfldI8Short = 0xBC,
    LdvfldAnyShort = 0xBD,
    LdfldaShort = 0xBE,
    StfldI1Short = 0xBF,
    StfldI2Short = 0xC0,
    StfldI4Short = 0xC1,
    StfldI8Short = 0xC2,
    StfldAnyShort = 0xC3,
    LdsfldI1Short = 0xC4,
    LdsfldU1Short = 0xC5,
    LdsfldI2Short = 0xC6,
    LdsfldU2Short = 0xC7,
    LdsfldI4Short = 0xC8,
    LdsfldI8Short = 0xC9,
    LdsfldAnyShort = 0xCA,
    LdsfldaShort = 0xCB,
    LdsfldRvaDataShort = 0xCC,
    StsfldI1Short = 0xCD,
    StsfldI2Short = 0xCE,
    StsfldI4Short = 0xCF,
    StsfldI8Short = 0xD0,
    StsfldAnyShort = 0xD1,
    RetVoidShort = 0xD2,
    RetI4Short = 0xD3,
    RetI8Short = 0xD4,
    RetAnyShort = 0xD5,
    RetNopShort = 0xD6,
    CallInterpShort = 0xD7,
    CallVirtInterpShort = 0xD8,
    CallInternalCallShort = 0xD9,
    CallIntrinsicShort = 0xDA,
    CallPInvokeShort = 0xDB,
    CallAotShort = 0xDC,
    CallRuntimeImplementedShort = 0xDD,
    CalliInterpShort = 0xDE,
    BoxRefInplaceShort = 0xDF,
    NewObjInterpShort = 0xE0,
    NewValueTypeInterpShort = 0xE1,
    NewObjInternalCallShort = 0xE2,
    NewObjIntrinsicShort = 0xE3,
    NewObjAotShort = 0xE4,
    NewValueTypeAotShort = 0xE5,
    ThrowShort = 0xE6,
    RethrowShort = 0xE7,
    LeaveTryWithFinallyShort = 0xE8,
    LeaveCatchWithFinallyShort = 0xE9,
    LeaveCatchWithoutFinallyShort = 0xEA,
    EndFilterShort = 0xEB,
    EndFinallyShort = 0xEC,
    EndFaultShort = 0xED,
    __UnusedEE = 0xEE,
    __UnusedEF = 0xEF,
    __UnusedF0 = 0xF0,
    __UnusedF1 = 0xF1,
    __UnusedF2 = 0xF2,
    __UnusedF3 = 0xF3,
    __UnusedF4 = 0xF4,
    __UnusedF5 = 0xF5,
    __UnusedF6 = 0xF6,
    __UnusedF7 = 0xF7,
    __UnusedF8 = 0xF8,
    __UnusedF9 = 0xF9,
    __UnusedFA = 0xFA,

    //}}LOW_LEVEL_OPCODE00
    Prefix1 = 0xFB,
    Prefix2 = 0xFC,
    Prefix3 = 0xFD,
    Prefix4 = 0xFE,
    Prefix5 = 0xFF,
};

enum class OpCodeValue1 : uint8_t
{
    //{{LOW_LEVEL_OPCODE1
    InitLocals = 0x00,
    LdLocI1 = 0x01,
    LdLocU1 = 0x02,
    LdLocI2 = 0x03,
    LdLocU2 = 0x04,
    LdLocI4 = 0x05,
    LdLocI8 = 0x06,
    LdLocAny = 0x07,
    LdLoca = 0x08,
    StLocI1 = 0x09,
    StLocI2 = 0x0A,
    StLocI4 = 0x0B,
    StLocI8 = 0x0C,
    StLocAny = 0x0D,
    LdNull = 0x0E,
    LdcI4I2 = 0x0F,
    LdcI4I4 = 0x10,
    LdcI8I2 = 0x11,
    LdcI8I4 = 0x12,
    LdcI8I8 = 0x13,
    LdStr = 0x14,
    Br = 0x15,
    BrTrueI4 = 0x16,
    BrTrueI8 = 0x17,
    BrFalseI4 = 0x18,
    BrFalseI8 = 0x19,
    BeqI4 = 0x1A,
    BeqI8 = 0x1B,
    BeqR4 = 0x1C,
    BeqR8 = 0x1D,
    BgeI4 = 0x1E,
    BgeI8 = 0x1F,
    BgeR4 = 0x20,
    BgeR8 = 0x21,
    BgtI4 = 0x22,
    BgtI8 = 0x23,
    BgtR4 = 0x24,
    BgtR8 = 0x25,
    BleI4 = 0x26,
    BleI8 = 0x27,
    BleR4 = 0x28,
    BleR8 = 0x29,
    BltI4 = 0x2A,
    BltI8 = 0x2B,
    BltR4 = 0x2C,
    BltR8 = 0x2D,
    BneUnI4 = 0x2E,
    BneUnI8 = 0x2F,
    BneUnR4 = 0x30,
    BneUnR8 = 0x31,
    BgeUnI4 = 0x32,
    BgeUnI8 = 0x33,
    BgeUnR4 = 0x34,
    BgeUnR8 = 0x35,
    BgtUnI4 = 0x36,
    BgtUnI8 = 0x37,
    BgtUnR4 = 0x38,
    BgtUnR8 = 0x39,
    BleUnI4 = 0x3A,
    BleUnI8 = 0x3B,
    BleUnR4 = 0x3C,
    BleUnR8 = 0x3D,
    BltUnI4 = 0x3E,
    BltUnI8 = 0x3F,
    BltUnR4 = 0x40,
    BltUnR8 = 0x41,
    Switch = 0x42,
    LdIndI1Short = 0x43,
    LdIndU1Short = 0x44,
    LdIndI2Short = 0x45,
    LdIndU2Short = 0x46,
    LdIndI4Short = 0x47,
    LdIndI8Short = 0x48,
    StIndI1Short = 0x49,
    StIndI2Short = 0x4A,
    StIndI4Short = 0x4B,
    StIndI8Short = 0x4C,
    StIndI8I4Short = 0x4D,
    StIndI8U4Short = 0x4E,
    AddI4 = 0x4F,
    AddI8 = 0x50,
    AddR4 = 0x51,
    AddR8 = 0x52,
    SubI4 = 0x53,
    SubI8 = 0x54,
    SubR4 = 0x55,
    SubR8 = 0x56,
    MulI4 = 0x57,
    MulI8 = 0x58,
    MulR4 = 0x59,
    MulR8 = 0x5A,
    DivI4 = 0x5B,
    DivI8 = 0x5C,
    DivR4 = 0x5D,
    DivR8 = 0x5E,
    DivUnI4 = 0x5F,
    DivUnI8 = 0x60,
    RemI4 = 0x61,
    RemI8 = 0x62,
    RemR4 = 0x63,
    RemR8 = 0x64,
    RemUnI4 = 0x65,
    RemUnI8 = 0x66,
    AndI4 = 0x67,
    AndI8 = 0x68,
    OrI4 = 0x69,
    OrI8 = 0x6A,
    XorI4 = 0x6B,
    XorI8 = 0x6C,
    ShlI4 = 0x6D,
    ShlI8 = 0x6E,
    ShrI4 = 0x6F,
    ShrI8 = 0x70,
    ShrUnI4 = 0x71,
    ShrUnI8 = 0x72,
    NegI4 = 0x73,
    NegI8 = 0x74,
    NegR4 = 0x75,
    NegR8 = 0x76,
    NotI4 = 0x77,
    NotI8 = 0x78,
    CeqI4 = 0x79,
    CeqI8 = 0x7A,
    CeqR4 = 0x7B,
    CeqR8 = 0x7C,
    CgtI4 = 0x7D,
    CgtI8 = 0x7E,
    CgtR4 = 0x7F,
    CgtR8 = 0x80,
    CgtUnI4 = 0x81,
    CgtUnI8 = 0x82,
    CgtUnR4 = 0x83,
    CgtUnR8 = 0x84,
    CltI4 = 0x85,
    CltI8 = 0x86,
    CltR4 = 0x87,
    CltR8 = 0x88,
    CltUnI4 = 0x89,
    CltUnI8 = 0x8A,
    CltUnR4 = 0x8B,
    CltUnR8 = 0x8C,
    InitObjI1 = 0x8D,
    InitObjI2 = 0x8E,
    InitObjI4 = 0x8F,
    InitObjI8 = 0x90,
    InitObjAny = 0x91,
    CpObjI1 = 0x92,
    CpObjI2 = 0x93,
    CpObjI4 = 0x94,
    CpObjI8 = 0x95,
    CpObjAny = 0x96,
    LdObjAny = 0x97,
    StObjAny = 0x98,
    CastClass = 0x99,
    IsInst = 0x9A,
    Box = 0x9B,
    Unbox = 0x9C,
    UnboxAny = 0x9D,
    NewArr = 0x9E,
    LdLen = 0x9F,
    Ldelema = 0xA0,
    LdelemI1 = 0xA1,
    LdelemU1 = 0xA2,
    LdelemI2 = 0xA3,
    LdelemU2 = 0xA4,
    LdelemI4 = 0xA5,
    LdelemI8 = 0xA6,
    LdelemI = 0xA7,
    LdelemR4 = 0xA8,
    LdelemR8 = 0xA9,
    LdelemRef = 0xAA,
    LdelemAnyRef = 0xAB,
    LdelemAnyVal = 0xAC,
    StelemI1 = 0xAD,
    StelemI2 = 0xAE,
    StelemI4 = 0xAF,
    StelemI8 = 0xB0,
    StelemI = 0xB1,
    StelemR4 = 0xB2,
    StelemR8 = 0xB3,
    StelemRef = 0xB4,
    StelemAnyRef = 0xB5,
    StelemAnyVal = 0xB6,
    MkRefAny = 0xB7,
    RefAnyVal = 0xB8,
    RefAnyType = 0xB9,
    LdToken = 0xBA,
    CkfiniteR4 = 0xBB,
    CkfiniteR8 = 0xBC,
    LocAlloc = 0xBD,
    Ldftn = 0xBE,
    Ldvirtftn = 0xBF,
    LdfldI1 = 0xC0,
    LdfldU1 = 0xC1,
    LdfldI2 = 0xC2,
    LdfldU2 = 0xC3,
    LdfldI4 = 0xC4,
    LdfldI8 = 0xC5,
    LdfldAny = 0xC6,
    LdvfldI1 = 0xC7,
    LdvfldU1 = 0xC8,
    LdvfldI2 = 0xC9,
    LdvfldU2 = 0xCA,
    LdvfldI4 = 0xCB,
    LdvfldI8 = 0xCC,
    LdvfldAny = 0xCD,
    Ldflda = 0xCE,
    StfldI1 = 0xCF,
    StfldI2 = 0xD0,
    StfldI4 = 0xD1,
    StfldI8 = 0xD2,
    StfldAny = 0xD3,
    LdsfldI1 = 0xD4,
    LdsfldU1 = 0xD5,
    LdsfldI2 = 0xD6,
    LdsfldU2 = 0xD7,
    LdsfldI4 = 0xD8,
    LdsfldI8 = 0xD9,
    LdsfldAny = 0xDA,
    Ldsflda = 0xDB,
    LdsfldRvaData = 0xDC,
    StsfldI1 = 0xDD,
    StsfldI2 = 0xDE,
    StsfldI4 = 0xDF,
    StsfldI8 = 0xE0,
    StsfldAny = 0xE1,
    RetVoid = 0xE2,
    RetI4 = 0xE3,
    RetI8 = 0xE4,
    RetAny = 0xE5,
    CallInterp = 0xE6,
    CallVirtInterp = 0xE7,
    CallInternalCall = 0xE8,
    CallIntrinsic = 0xE9,
    CallPInvoke = 0xEA,
    CallAot = 0xEB,
    CallRuntimeImplemented = 0xEC,
    CalliInterp = 0xED,
    BoxRefInplace = 0xEE,
    NewObjInterp = 0xEF,
    NewValueTypeInterp = 0xF0,
    NewObjInternalCall = 0xF1,
    NewObjIntrinsic = 0xF2,
    NewObjAot = 0xF3,
    NewValueTypeAot = 0xF4,
    Throw = 0xF5,
    Rethrow = 0xF6,
    LeaveTryWithFinally = 0xF7,
    LeaveCatchWithFinally = 0xF8,
    LeaveCatchWithoutFinally = 0xF9,
    EndFilter = 0xFA,
    EndFinally = 0xFB,
    EndFault = 0xFC,

    //}}LOW_LEVEL_OPCODE1
};

enum class OpCodeValue2 : uint8_t
{
    //{{LOW_LEVEL_OPCODE2
    LdIndI1 = 0x00,
    LdIndU1 = 0x01,
    LdIndI2 = 0x02,
    LdIndU2 = 0x03,
    LdIndI4 = 0x04,
    LdIndI8 = 0x05,
    StIndI1 = 0x06,
    StIndI2 = 0x07,
    StIndI4 = 0x08,
    StIndI8 = 0x09,
    StIndI8I4 = 0x0A,
    StIndI8U4 = 0x0B,
    ConvI1I4 = 0x0C,
    ConvI1I8 = 0x0D,
    ConvI1R4 = 0x0E,
    ConvI1R8 = 0x0F,
    ConvU1I4 = 0x10,
    ConvU1I8 = 0x11,
    ConvU1R4 = 0x12,
    ConvU1R8 = 0x13,
    ConvI2I4 = 0x14,
    ConvI2I8 = 0x15,
    ConvI2R4 = 0x16,
    ConvI2R8 = 0x17,
    ConvU2I4 = 0x18,
    ConvU2I8 = 0x19,
    ConvU2R4 = 0x1A,
    ConvU2R8 = 0x1B,
    ConvI4I8 = 0x1C,
    ConvI4R4 = 0x1D,
    ConvI4R8 = 0x1E,
    ConvU4I8 = 0x1F,
    ConvU4R4 = 0x20,
    ConvU4R8 = 0x21,
    ConvI8I4 = 0x22,
    ConvI8U4 = 0x23,
    ConvI8R4 = 0x24,
    ConvI8R8 = 0x25,
    ConvU8I4 = 0x26,
    ConvU8R4 = 0x27,
    ConvU8R8 = 0x28,
    ConvR4I4 = 0x29,
    ConvR4I8 = 0x2A,
    ConvR4R8 = 0x2B,
    ConvR8I4 = 0x2C,
    ConvR8I8 = 0x2D,
    ConvR8R4 = 0x2E,
    LdelemaReadOnly = 0x2F,
    InitBlk = 0x30,
    CpBlk = 0x31,
    GetEnumLongHashCode = 0x32,

    //}}LOW_LEVEL_OPCODE2
};

enum class OpCodeValue3 : uint8_t
{
    //{{LOW_LEVEL_OPCODE3
    LdIndI2Unaligned = 0x00,
    LdIndU2Unaligned = 0x01,
    LdIndI4Unaligned = 0x02,
    LdIndI8Unaligned = 0x03,
    StIndI2Unaligned = 0x04,
    StIndI4Unaligned = 0x05,
    StIndI8Unaligned = 0x06,
    StIndI8I4Unaligned = 0x07,
    StIndI8U4Unaligned = 0x08,
    AddOvfI4 = 0x09,
    AddOvfI8 = 0x0A,
    AddOvfUnI4 = 0x0B,
    AddOvfUnI8 = 0x0C,
    MulOvfI4 = 0x0D,
    MulOvfI8 = 0x0E,
    MulOvfUnI4 = 0x0F,
    MulOvfUnI8 = 0x10,
    SubOvfI4 = 0x11,
    SubOvfI8 = 0x12,
    SubOvfUnI4 = 0x13,
    SubOvfUnI8 = 0x14,
    ConvOvfI1I4 = 0x15,
    ConvOvfI1I8 = 0x16,
    ConvOvfI1R4 = 0x17,
    ConvOvfI1R8 = 0x18,
    ConvOvfU1I4 = 0x19,
    ConvOvfU1I8 = 0x1A,
    ConvOvfU1R4 = 0x1B,
    ConvOvfU1R8 = 0x1C,
    ConvOvfI2I4 = 0x1D,
    ConvOvfI2I8 = 0x1E,
    ConvOvfI2R4 = 0x1F,
    ConvOvfI2R8 = 0x20,
    ConvOvfU2I4 = 0x21,
    ConvOvfU2I8 = 0x22,
    ConvOvfU2R4 = 0x23,
    ConvOvfU2R8 = 0x24,
    ConvOvfI4I8 = 0x25,
    ConvOvfI4R4 = 0x26,
    ConvOvfI4R8 = 0x27,
    ConvOvfU4I4 = 0x28,
    ConvOvfU4I8 = 0x29,
    ConvOvfU4R4 = 0x2A,
    ConvOvfU4R8 = 0x2B,
    ConvOvfI8R4 = 0x2C,
    ConvOvfI8R8 = 0x2D,
    ConvOvfU8I4 = 0x2E,
    ConvOvfU8I8 = 0x2F,
    ConvOvfU8R4 = 0x30,
    ConvOvfU8R8 = 0x31,
    ConvOvfI1UnI4 = 0x32,
    ConvOvfI1UnI8 = 0x33,
    ConvOvfI1UnR4 = 0x34,
    ConvOvfI1UnR8 = 0x35,
    ConvOvfU1UnI4 = 0x36,
    ConvOvfU1UnI8 = 0x37,
    ConvOvfU1UnR4 = 0x38,
    ConvOvfU1UnR8 = 0x39,
    ConvOvfI2UnI4 = 0x3A,
    ConvOvfI2UnI8 = 0x3B,
    ConvOvfI2UnR4 = 0x3C,
    ConvOvfI2UnR8 = 0x3D,
    ConvOvfU2UnI4 = 0x3E,
    ConvOvfU2UnI8 = 0x3F,
    ConvOvfU2UnR4 = 0x40,
    ConvOvfU2UnR8 = 0x41,
    ConvOvfI4UnI4 = 0x42,
    ConvOvfI4UnI8 = 0x43,
    ConvOvfI4UnR4 = 0x44,
    ConvOvfI4UnR8 = 0x45,
    ConvOvfU4UnI8 = 0x46,
    ConvOvfU4UnR4 = 0x47,
    ConvOvfU4UnR8 = 0x48,
    ConvOvfI8UnI8 = 0x49,
    ConvOvfI8UnR4 = 0x4A,
    ConvOvfI8UnR8 = 0x4B,
    ConvOvfU8UnR4 = 0x4C,
    ConvOvfU8UnR8 = 0x4D,
    InitObjI2Unaligned = 0x4E,
    InitObjI4Unaligned = 0x4F,
    InitObjI8Unaligned = 0x50,
    LdfldI1Large = 0x51,
    LdfldU1Large = 0x52,
    LdfldI2Large = 0x53,
    LdfldI2Unaligned = 0x54,
    LdfldU2Large = 0x55,
    LdfldU2Unaligned = 0x56,
    LdfldI4Large = 0x57,
    LdfldI4Unaligned = 0x58,
    LdfldI8Large = 0x59,
    LdfldI8Unaligned = 0x5A,
    LdfldAnyLarge = 0x5B,
    LdvfldI1Large = 0x5C,
    LdvfldU1Large = 0x5D,
    LdvfldI2Large = 0x5E,
    LdvfldI2Unaligned = 0x5F,
    LdvfldU2Large = 0x60,
    LdvfldU2Unaligned = 0x61,
    LdvfldI4Large = 0x62,
    LdvfldI4Unaligned = 0x63,
    LdvfldI8Large = 0x64,
    LdvfldI8Unaligned = 0x65,
    LdvfldAnyLarge = 0x66,
    LdfldaLarge = 0x67,
    StfldI1Large = 0x68,
    StfldI2Large = 0x69,
    StfldI2Unaligned = 0x6A,
    StfldI4Large = 0x6B,
    StfldI4Unaligned = 0x6C,
    StfldI8Large = 0x6D,
    StfldI8Unaligned = 0x6E,
    StfldAnyLarge = 0x6F,

    //}}LOW_LEVEL_OPCODE3
};

enum class OpCodeValue4 : uint8_t
{
    //{{LOW_LEVEL_OPCODE4
    Illegal = 0x00,
    Nop = 0x01,
    Arglist = 0x02,

    //}}LOW_LEVEL_OPCODE4
};

enum class OpCodeValue5 : uint8_t
{
    //{{LOW_LEVEL_OPCODE5
    Nop = 0x00,
    //}}LOW_LEVEL_OPCODE
};

//{{LOW_LEVEL_INSTRUCTION_STRUCTS
struct Illegal
{
    uint8_t __prefix;
    uint8_t __code;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct Nop
{
    uint8_t __prefix;
    uint8_t __code;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct InitLocals1Short
{
    uint8_t __code;
    uint8_t offset;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct InitLocals2Short
{
    uint8_t __code;
    uint8_t offset;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct InitLocals3Short
{
    uint8_t __code;
    uint8_t offset;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct InitLocals4Short
{
    uint8_t __code;
    uint8_t offset;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct InitLocals
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t offset;
    uint16_t size;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct InitLocalsShort
{
    uint8_t __code;
    uint8_t offset;
    uint8_t size;
    uint8_t __padding_3;
};

struct Arglist
{
    uint8_t __prefix;
    uint8_t __code;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct LdLocI1
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdLocU1
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdLocI2
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdLocU2
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdLocI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdLocI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdLocAny
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint16_t size;
};

struct LdLocI1Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct LdLocU1Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct LdLocI2Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct LdLocU2Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct LdLocI4Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct LdLocI8Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct LdLocAnyShort
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t size;
};

struct LdLoca
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdLocaShort
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct StLocI1
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StLocI2
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StLocI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StLocI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StLocAny
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint16_t size;
};

struct StLocI1Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct StLocI2Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct StLocI4Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct StLocI8Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct StLocAnyShort
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t size;
};

struct LdNull
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t dst;
};

struct LdNullShort
{
    uint8_t __code;
    uint8_t dst;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct LdcI4I2
{
    uint8_t __prefix;
    uint8_t __code;
    int16_t value;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdcI4I2Short
{
    uint8_t __code;
    uint8_t dst;
    int16_t value;
};

struct LdcI4I4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t dst;
    int32_t value;
};

struct LdcI4I4Short
{
    uint8_t __code;
    uint8_t dst;
    uint8_t __padding_2;
    uint8_t __padding_3;
    int32_t value;
};

struct LdcI8I2
{
    uint8_t __prefix;
    uint8_t __code;
    int16_t value;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdcI8I2Short
{
    uint8_t __code;
    uint8_t dst;
    int16_t value;
};

struct LdcI8I4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t dst;
    int32_t value;
};

struct LdcI8I4Short
{
    uint8_t __code;
    uint8_t dst;
    uint8_t __padding_2;
    uint8_t __padding_3;
    int32_t value;
};

struct LdcI8I8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t dst;
    int32_t value_low;
    int32_t value_high;
};

struct LdcI8I8Short
{
    uint8_t __code;
    uint8_t dst;
    uint8_t __padding_2;
    uint8_t __padding_3;
    int32_t value_low;
    int32_t value_high;
};

struct LdStr
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t str_idx;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdStrShort
{
    uint8_t __code;
    uint8_t str_idx;
    uint8_t dst;
    uint8_t __padding_3;
};

struct Br
{
    uint8_t __prefix;
    uint8_t __code;
    uint8_t __padding_2;
    uint8_t __padding_3;
    int32_t target_offset;
};

struct BrShort
{
    uint8_t __code;
    int8_t target_offset;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct BrTrueI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t condition;
    int32_t target_offset;
};

struct BrTrueI4Short
{
    uint8_t __code;
    uint8_t condition;
    int8_t target_offset;
    uint8_t __padding_3;
};

struct BrTrueI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t condition;
    int32_t target_offset;
};

struct BrTrueI8Short
{
    uint8_t __code;
    uint8_t condition;
    int8_t target_offset;
    uint8_t __padding_3;
};

struct BrFalseI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t condition;
    int32_t target_offset;
};

struct BrFalseI4Short
{
    uint8_t __code;
    uint8_t condition;
    int8_t target_offset;
    uint8_t __padding_3;
};

struct BrFalseI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t condition;
    int32_t target_offset;
};

struct BrFalseI8Short
{
    uint8_t __code;
    uint8_t condition;
    int8_t target_offset;
    uint8_t __padding_3;
};

struct BeqI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BeqI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BeqR4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BeqR8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BeqI4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    int8_t target_offset;
};

struct BeqI8Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    int8_t target_offset;
};

struct BgeI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BgeI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BgeR4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BgeR8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BgeI4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    int8_t target_offset;
};

struct BgeI8Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    int8_t target_offset;
};

struct BgtI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BgtI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BgtR4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BgtR8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BgtI4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    int8_t target_offset;
};

struct BgtI8Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    int8_t target_offset;
};

struct BleI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BleI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BleR4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BleR8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BleI4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    int8_t target_offset;
};

struct BleI8Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    int8_t target_offset;
};

struct BltI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BltI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BltR4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BltR8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BltI4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    int8_t target_offset;
};

struct BltI8Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    int8_t target_offset;
};

struct BneUnI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BneUnI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BneUnR4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BneUnR8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BneUnI4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    int8_t target_offset;
};

struct BneUnI8Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    int8_t target_offset;
};

struct BgeUnI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BgeUnI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BgeUnR4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BgeUnR8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BgeUnI4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    int8_t target_offset;
};

struct BgeUnI8Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    int8_t target_offset;
};

struct BgtUnI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BgtUnI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BgtUnR4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BgtUnR8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BgtUnI4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    int8_t target_offset;
};

struct BgtUnI8Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    int8_t target_offset;
};

struct BleUnI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BleUnI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BleUnR4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BleUnR8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BleUnI4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    int8_t target_offset;
};

struct BleUnI8Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    int8_t target_offset;
};

struct BltUnI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BltUnI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BltUnR4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BltUnR8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BltUnI4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    int8_t target_offset;
};

struct BltUnI8Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    int8_t target_offset;
};

struct Switch
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t index;
    uint32_t num_targets;
};

struct LdIndI1
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdIndI1Short
{
    uint8_t __prefix;
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
};

struct LdIndU1
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdIndU1Short
{
    uint8_t __prefix;
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
};

struct LdIndI2
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdIndI2Short
{
    uint8_t __prefix;
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
};

struct LdIndI2Unaligned
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdIndU2
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdIndU2Short
{
    uint8_t __prefix;
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
};

struct LdIndU2Unaligned
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdIndI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdIndI4Short
{
    uint8_t __prefix;
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
};

struct LdIndI4Unaligned
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdIndI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdIndI8Short
{
    uint8_t __prefix;
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
};

struct LdIndI8Unaligned
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StIndI1
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StIndI1Short
{
    uint8_t __prefix;
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
};

struct StIndI2
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StIndI2Short
{
    uint8_t __prefix;
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
};

struct StIndI2Unaligned
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StIndI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StIndI4Short
{
    uint8_t __prefix;
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
};

struct StIndI4Unaligned
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StIndI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StIndI8Short
{
    uint8_t __prefix;
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
};

struct StIndI8Unaligned
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StIndI8I4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StIndI8I4Short
{
    uint8_t __prefix;
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
};

struct StIndI8I4Unaligned
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StIndI8U4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StIndI8U4Short
{
    uint8_t __prefix;
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
};

struct StIndI8U4Unaligned
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct AddI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct AddI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct AddR4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct AddR8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct AddI4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct AddI8Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct AddR4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct AddR8Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct SubI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct SubI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct SubR4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct SubR8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct SubI4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct SubI8Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct SubR4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct SubR8Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct MulI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct MulI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct MulR4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct MulR8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct MulI4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct MulI8Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct MulR4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct MulR8Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct DivI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct DivI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct DivR4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct DivR8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct DivI4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct DivI8Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct DivR4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct DivR8Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct DivUnI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct DivUnI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct DivUnI4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct DivUnI8Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct RemI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct RemI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct RemR4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct RemR8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct RemI4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct RemI8Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct RemR4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct RemR8Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct RemUnI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct RemUnI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct RemUnI4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct RemUnI8Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct AndI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct AndI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct AndI4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct AndI8Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct OrI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct OrI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct OrI4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct OrI8Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct XorI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct XorI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct XorI4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct XorI8Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct ShlI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct ShlI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct ShlI4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct ShrI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct ShrI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct ShrI4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct ShrUnI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct ShrUnI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct ShrUnI4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct NegI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct NegI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct NegR4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct NegR8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct NegI4Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct NegI8Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct NegR4Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct NegR8Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct NotI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct NotI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct NotI4Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct NotI8Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct AddOvfI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct AddOvfI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct AddOvfUnI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct AddOvfUnI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct MulOvfI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct MulOvfI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct MulOvfUnI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct MulOvfUnI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct SubOvfI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct SubOvfI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct SubOvfUnI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct SubOvfUnI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct ConvI1I4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvI1I8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvI1R4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvI1R8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvI1I4Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct ConvI1I8Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct ConvI1R4Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct ConvI1R8Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct ConvU1I4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvU1I8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvU1R4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvU1R8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvU1I4Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct ConvU1I8Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct ConvU1R4Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct ConvU1R8Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct ConvI2I4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvI2I8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvI2R4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvI2R8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvI2I4Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct ConvI2I8Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct ConvI2R4Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct ConvI2R8Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct ConvU2I4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvU2I8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvU2R4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvU2R8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvU2I4Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct ConvU2I8Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct ConvU2R4Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct ConvU2R8Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct ConvI4I8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvI4R4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvI4R8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvI4I8Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct ConvI4R4Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct ConvI4R8Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct ConvU4I8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvU4R4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvU4R8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvU4I8Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct ConvU4R4Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct ConvU4R8Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct ConvI8I4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvI8U4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvI8R4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvI8R8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvI8I4Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct ConvI8R4Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct ConvI8R8Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct ConvU8I4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvU8R4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvU8R8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvR4I4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvR4I8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvR4R8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvR4I4Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct ConvR4I8Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct ConvR4R8Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct ConvR8I4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvR8I8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvR8R4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvR8I4Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct ConvR8I8Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct ConvR8R4Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct ConvOvfI1I4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI1I8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI1R4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI1R8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU1I4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU1I8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU1R4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU1R8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI2I4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI2I8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI2R4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI2R8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU2I4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU2I8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU2R4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU2R8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI4I8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI4R4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI4R8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU4I4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU4I8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU4R4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU4R8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI8R4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI8R8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU8I4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU8I8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU8R4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU8R8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI1UnI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI1UnI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI1UnR4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI1UnR8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU1UnI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU1UnI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU1UnR4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU1UnR8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI2UnI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI2UnI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI2UnR4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI2UnR8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU2UnI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU2UnI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU2UnR4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU2UnR8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI4UnI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI4UnI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI4UnR4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI4UnR8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU4UnI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU4UnR4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU4UnR8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI8UnI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI8UnR4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI8UnR8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU8UnR4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU8UnR8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct CeqI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CeqI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CeqR4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CeqR8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CeqI4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct CeqI8Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct CeqR4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct CeqR8Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct CgtI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CgtI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CgtR4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CgtR8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CgtI4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct CgtI8Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct CgtUnI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CgtUnI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CgtUnR4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CgtUnR8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CgtUnI4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct CgtUnI8Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct CltI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CltI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CltR4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CltR8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CltI4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct CltI8Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct CltUnI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CltUnI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CltUnR4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CltUnR8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CltUnI4Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct CltUnI8Short
{
    uint8_t __code;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t dst;
};

struct InitObjI1
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t addr;
};

struct InitObjI1Short
{
    uint8_t __code;
    uint8_t addr;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct InitObjI2
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t addr;
};

struct InitObjI2Short
{
    uint8_t __code;
    uint8_t addr;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct InitObjI2Unaligned
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t addr;
};

struct InitObjI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t addr;
};

struct InitObjI4Short
{
    uint8_t __code;
    uint8_t addr;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct InitObjI4Unaligned
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t addr;
};

struct InitObjI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t addr;
};

struct InitObjI8Short
{
    uint8_t __code;
    uint8_t addr;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct InitObjI8Unaligned
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t addr;
};

struct InitObjAny
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t addr;
    uint32_t size;
};

struct InitObjAnyShort
{
    uint8_t __code;
    uint8_t addr;
    uint8_t __padding_2;
    uint8_t __padding_3;
    uint32_t size;
};

struct CpObjI1
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct CpObjI1Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct CpObjI2
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct CpObjI2Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct CpObjI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct CpObjI4Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct CpObjI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct CpObjI8Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t __padding_3;
};

struct CpObjAny
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint16_t size;
};

struct CpObjAnyShort
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t size;
};

struct LdObjAny
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t addr;
    uint16_t dst;
    uint16_t size;
};

struct LdObjAnyShort
{
    uint8_t __code;
    uint8_t addr;
    uint8_t dst;
    uint8_t size;
};

struct StObjAny
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t addr;
    uint16_t size;
};

struct StObjAnyShort
{
    uint8_t __code;
    uint8_t src;
    uint8_t addr;
    uint8_t size;
};

struct CastClass
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t klass_idx;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct CastClassShort
{
    uint8_t __code;
    uint8_t obj;
    uint8_t klass_idx;
    uint8_t __padding_3;
};

struct IsInst
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t klass_idx;
};

struct IsInstShort
{
    uint8_t __code;
    uint8_t obj;
    uint8_t dst;
    uint8_t klass_idx;
};

struct Box
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint16_t klass_idx;
};

struct BoxShort
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t klass_idx;
};

struct Unbox
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t klass_idx;
};

struct UnboxShort
{
    uint8_t __code;
    uint8_t obj;
    uint8_t dst;
    uint8_t klass_idx;
};

struct UnboxAny
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t klass_idx;
};

struct UnboxAnyShort
{
    uint8_t __code;
    uint8_t obj;
    uint8_t dst;
    uint8_t klass_idx;
};

struct NewArr
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t length;
    uint16_t dst;
    uint16_t arr_klass_idx;
};

struct NewArrShort
{
    uint8_t __code;
    uint8_t length;
    uint8_t dst;
    uint8_t arr_klass_idx;
};

struct LdLen
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arr;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdLenShort
{
    uint8_t __code;
    uint8_t arr;
    uint8_t dst;
    uint8_t __padding_3;
};

struct Ldelema
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t dst;
    uint16_t ele_klass_idx;
    uint8_t __padding_10;
    uint8_t __padding_11;
};

struct LdelemaShort
{
    uint8_t __code;
    uint8_t arr;
    uint8_t index;
    uint8_t dst;
    uint8_t ele_klass_idx;
    uint8_t __padding_5;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdelemaReadOnly
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t dst;
};

struct LdelemI1
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t dst;
};

struct LdelemI1Short
{
    uint8_t __code;
    uint8_t arr;
    uint8_t index;
    uint8_t dst;
};

struct LdelemU1
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t dst;
};

struct LdelemU1Short
{
    uint8_t __code;
    uint8_t arr;
    uint8_t index;
    uint8_t dst;
};

struct LdelemI2
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t dst;
};

struct LdelemI2Short
{
    uint8_t __code;
    uint8_t arr;
    uint8_t index;
    uint8_t dst;
};

struct LdelemU2
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t dst;
};

struct LdelemU2Short
{
    uint8_t __code;
    uint8_t arr;
    uint8_t index;
    uint8_t dst;
};

struct LdelemI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t dst;
};

struct LdelemI4Short
{
    uint8_t __code;
    uint8_t arr;
    uint8_t index;
    uint8_t dst;
};

struct LdelemI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t dst;
};

struct LdelemI8Short
{
    uint8_t __code;
    uint8_t arr;
    uint8_t index;
    uint8_t dst;
};

struct LdelemI
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t dst;
};

struct LdelemIShort
{
    uint8_t __code;
    uint8_t arr;
    uint8_t index;
    uint8_t dst;
};

struct LdelemR4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t dst;
};

struct LdelemR4Short
{
    uint8_t __code;
    uint8_t arr;
    uint8_t index;
    uint8_t dst;
};

struct LdelemR8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t dst;
};

struct LdelemR8Short
{
    uint8_t __code;
    uint8_t arr;
    uint8_t index;
    uint8_t dst;
};

struct LdelemRef
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t dst;
};

struct LdelemRefShort
{
    uint8_t __code;
    uint8_t arr;
    uint8_t index;
    uint8_t dst;
};

struct LdelemAnyRef
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t dst;
    uint16_t ele_klass_idx;
    uint8_t __padding_10;
    uint8_t __padding_11;
};

struct LdelemAnyRefShort
{
    uint8_t __code;
    uint8_t arr;
    uint8_t index;
    uint8_t dst;
    uint8_t ele_klass_idx;
    uint8_t __padding_5;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdelemAnyVal
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t dst;
    uint16_t ele_klass_idx;
    uint8_t __padding_10;
    uint8_t __padding_11;
    uint32_t ele_size;
};

struct LdelemAnyValShort
{
    uint8_t __code;
    uint8_t arr;
    uint8_t index;
    uint8_t dst;
    uint8_t ele_klass_idx;
    uint8_t __padding_5;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t ele_size;
};

struct StelemI1
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t value;
};

struct StelemI1Short
{
    uint8_t __code;
    uint8_t arr;
    uint8_t index;
    uint8_t value;
};

struct StelemI2
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t value;
};

struct StelemI2Short
{
    uint8_t __code;
    uint8_t arr;
    uint8_t index;
    uint8_t value;
};

struct StelemI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t value;
};

struct StelemI4Short
{
    uint8_t __code;
    uint8_t arr;
    uint8_t index;
    uint8_t value;
};

struct StelemI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t value;
};

struct StelemI8Short
{
    uint8_t __code;
    uint8_t arr;
    uint8_t index;
    uint8_t value;
};

struct StelemI
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t value;
};

struct StelemIShort
{
    uint8_t __code;
    uint8_t arr;
    uint8_t index;
    uint8_t value;
};

struct StelemR4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t value;
};

struct StelemR4Short
{
    uint8_t __code;
    uint8_t arr;
    uint8_t index;
    uint8_t value;
};

struct StelemR8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t value;
};

struct StelemR8Short
{
    uint8_t __code;
    uint8_t arr;
    uint8_t index;
    uint8_t value;
};

struct StelemRef
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t value;
};

struct StelemRefShort
{
    uint8_t __code;
    uint8_t arr;
    uint8_t index;
    uint8_t value;
};

struct StelemAnyRef
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t value;
    uint16_t ele_klass_idx;
    uint8_t __padding_10;
    uint8_t __padding_11;
};

struct StelemAnyRefShort
{
    uint8_t __code;
    uint8_t arr;
    uint8_t index;
    uint8_t value;
    uint8_t ele_klass_idx;
    uint8_t __padding_5;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StelemAnyVal
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t value;
    uint16_t ele_klass_idx;
    uint8_t __padding_10;
    uint8_t __padding_11;
    uint32_t ele_size;
};

struct StelemAnyValShort
{
    uint8_t __code;
    uint8_t arr;
    uint8_t index;
    uint8_t value;
    uint8_t ele_klass_idx;
    uint8_t __padding_5;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t ele_size;
};

struct MkRefAny
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t addr;
    uint16_t dst;
    uint16_t klass_idx;
};

struct RefAnyVal
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint16_t klass_idx;
};

struct RefAnyType
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdToken
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t dst;
    uint16_t token_idx;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct CkfiniteR4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
};

struct CkfiniteR8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
};

struct LocAlloc
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t size;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct InitBlk
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t addr;
    uint16_t value;
    uint16_t size;
};

struct CpBlk
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t dst;
    uint16_t src;
    uint16_t size;
};

struct Ldftn
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t dst;
    uint16_t method_idx;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdftnShort
{
    uint8_t __code;
    uint8_t dst;
    uint8_t method_idx;
    uint8_t __padding_3;
};

struct Ldvirtftn
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t method_idx;
};

struct LdvirtftnShort
{
    uint8_t __code;
    uint8_t obj;
    uint8_t dst;
    uint8_t method_idx;
};

struct LdfldI1
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdfldI1Short
{
    uint8_t __code;
    uint8_t obj;
    uint8_t dst;
    uint8_t offset;
};

struct LdfldI1Large
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct LdfldU1
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdfldU1Short
{
    uint8_t __code;
    uint8_t obj;
    uint8_t dst;
    uint8_t offset;
};

struct LdfldU1Large
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct LdfldI2
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdfldI2Short
{
    uint8_t __code;
    uint8_t obj;
    uint8_t dst;
    uint8_t offset;
};

struct LdfldI2Large
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct LdfldI2Unaligned
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdfldU2
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdfldU2Short
{
    uint8_t __code;
    uint8_t obj;
    uint8_t dst;
    uint8_t offset;
};

struct LdfldU2Large
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct LdfldU2Unaligned
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdfldI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdfldI4Short
{
    uint8_t __code;
    uint8_t obj;
    uint8_t dst;
    uint8_t offset;
};

struct LdfldI4Large
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct LdfldI4Unaligned
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdfldI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdfldI8Short
{
    uint8_t __code;
    uint8_t obj;
    uint8_t dst;
    uint8_t offset;
};

struct LdfldI8Large
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct LdfldI8Unaligned
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdfldAny
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
    uint16_t size;
    uint8_t __padding_10;
    uint8_t __padding_11;
};

struct LdfldAnyShort
{
    uint8_t __code;
    uint8_t obj;
    uint8_t dst;
    uint8_t offset;
    uint8_t size;
    uint8_t __padding_5;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdfldAnyLarge
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
    uint32_t size;
};

struct LdvfldI1
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdvfldI1Short
{
    uint8_t __code;
    uint8_t obj;
    uint8_t dst;
    uint8_t offset;
};

struct LdvfldI1Large
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct LdvfldU1
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdvfldU1Short
{
    uint8_t __code;
    uint8_t obj;
    uint8_t dst;
    uint8_t offset;
};

struct LdvfldU1Large
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct LdvfldI2
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdvfldI2Short
{
    uint8_t __code;
    uint8_t obj;
    uint8_t dst;
    uint8_t offset;
};

struct LdvfldI2Large
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct LdvfldI2Unaligned
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdvfldU2
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdvfldU2Short
{
    uint8_t __code;
    uint8_t obj;
    uint8_t dst;
    uint8_t offset;
};

struct LdvfldU2Large
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct LdvfldU2Unaligned
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdvfldI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdvfldI4Short
{
    uint8_t __code;
    uint8_t obj;
    uint8_t dst;
    uint8_t offset;
};

struct LdvfldI4Large
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct LdvfldI4Unaligned
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdvfldI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdvfldI8Short
{
    uint8_t __code;
    uint8_t obj;
    uint8_t dst;
    uint8_t offset;
};

struct LdvfldI8Large
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct LdvfldI8Unaligned
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdvfldAny
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
    uint16_t size;
    uint8_t __padding_10;
    uint8_t __padding_11;
};

struct LdvfldAnyShort
{
    uint8_t __code;
    uint8_t obj;
    uint8_t dst;
    uint8_t offset;
    uint8_t size;
    uint8_t __padding_5;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdvfldAnyLarge
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
    uint32_t size;
};

struct Ldflda
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdfldaShort
{
    uint8_t __code;
    uint8_t obj;
    uint8_t dst;
    uint8_t offset;
};

struct LdfldaLarge
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct StfldI1
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t value;
    uint16_t offset;
};

struct StfldI1Short
{
    uint8_t __code;
    uint8_t obj;
    uint8_t value;
    uint8_t offset;
};

struct StfldI1Large
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t value;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct StfldI2
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t value;
    uint16_t offset;
};

struct StfldI2Short
{
    uint8_t __code;
    uint8_t obj;
    uint8_t value;
    uint8_t offset;
};

struct StfldI2Large
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t value;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct StfldI2Unaligned
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t value;
    uint16_t offset;
};

struct StfldI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t value;
    uint16_t offset;
};

struct StfldI4Short
{
    uint8_t __code;
    uint8_t obj;
    uint8_t value;
    uint8_t offset;
};

struct StfldI4Large
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t value;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct StfldI4Unaligned
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t value;
    uint16_t offset;
};

struct StfldI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t value;
    uint16_t offset;
};

struct StfldI8Short
{
    uint8_t __code;
    uint8_t obj;
    uint8_t value;
    uint8_t offset;
};

struct StfldI8Large
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t value;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct StfldI8Unaligned
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t value;
    uint16_t offset;
};

struct StfldAny
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t value;
    uint16_t offset;
    uint16_t size;
    uint8_t __padding_10;
    uint8_t __padding_11;
};

struct StfldAnyShort
{
    uint8_t __code;
    uint8_t obj;
    uint8_t value;
    uint8_t offset;
    uint8_t size;
    uint8_t __padding_5;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StfldAnyLarge
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t obj;
    uint16_t value;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
    uint32_t size;
};

struct LdsfldI1
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t dst;
    uint16_t field_idx;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdsfldI1Short
{
    uint8_t __code;
    uint8_t dst;
    uint8_t field_idx;
    uint8_t __padding_3;
};

struct LdsfldU1
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t dst;
    uint16_t field_idx;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdsfldU1Short
{
    uint8_t __code;
    uint8_t dst;
    uint8_t field_idx;
    uint8_t __padding_3;
};

struct LdsfldI2
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t dst;
    uint16_t field_idx;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdsfldI2Short
{
    uint8_t __code;
    uint8_t dst;
    uint8_t field_idx;
    uint8_t __padding_3;
};

struct LdsfldU2
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t dst;
    uint16_t field_idx;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdsfldU2Short
{
    uint8_t __code;
    uint8_t dst;
    uint8_t field_idx;
    uint8_t __padding_3;
};

struct LdsfldI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t dst;
    uint16_t field_idx;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdsfldI4Short
{
    uint8_t __code;
    uint8_t dst;
    uint8_t field_idx;
    uint8_t __padding_3;
};

struct LdsfldI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t dst;
    uint16_t field_idx;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdsfldI8Short
{
    uint8_t __code;
    uint8_t dst;
    uint8_t field_idx;
    uint8_t __padding_3;
};

struct LdsfldAny
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t field_idx;
    uint16_t size;
    uint16_t dst;
};

struct LdsfldAnyShort
{
    uint8_t __code;
    uint8_t field_idx;
    uint8_t size;
    uint8_t dst;
};

struct Ldsflda
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t field_idx;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdsfldaShort
{
    uint8_t __code;
    uint8_t field_idx;
    uint8_t dst;
    uint8_t __padding_3;
};

struct LdsfldRvaData
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t data;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdsfldRvaDataShort
{
    uint8_t __code;
    uint8_t data;
    uint8_t dst;
    uint8_t __padding_3;
};

struct StsfldI1
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t field_idx;
    uint16_t value;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StsfldI1Short
{
    uint8_t __code;
    uint8_t field_idx;
    uint8_t value;
    uint8_t __padding_3;
};

struct StsfldI2
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t field_idx;
    uint16_t value;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StsfldI2Short
{
    uint8_t __code;
    uint8_t field_idx;
    uint8_t value;
    uint8_t __padding_3;
};

struct StsfldI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t field_idx;
    uint16_t value;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StsfldI4Short
{
    uint8_t __code;
    uint8_t field_idx;
    uint8_t value;
    uint8_t __padding_3;
};

struct StsfldI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t field_idx;
    uint16_t value;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StsfldI8Short
{
    uint8_t __code;
    uint8_t field_idx;
    uint8_t value;
    uint8_t __padding_3;
};

struct StsfldAny
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t field_idx;
    uint16_t size;
    uint16_t value;
};

struct StsfldAnyShort
{
    uint8_t __code;
    uint8_t field_idx;
    uint8_t size;
    uint8_t value;
};

struct RetVoid
{
    uint8_t __prefix;
    uint8_t __code;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct RetVoidShort
{
    uint8_t __code;
    uint8_t __padding_1;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct RetI4
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
};

struct RetI8
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
};

struct RetAny
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t size;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct RetI4Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct RetI8Short
{
    uint8_t __code;
    uint8_t src;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct RetAnyShort
{
    uint8_t __code;
    uint8_t src;
    uint8_t size;
    uint8_t __padding_3;
};

struct RetNopShort
{
    uint8_t __code;
    uint8_t __padding_1;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct CallInterp
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t method_idx;
    uint16_t frame_base;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct CallInterpShort
{
    uint8_t __code;
    uint8_t method_idx;
    uint8_t frame_base;
    uint8_t __padding_3;
};

struct CallVirtInterp
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t method_idx;
    uint16_t frame_base;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct CallVirtInterpShort
{
    uint8_t __code;
    uint8_t method_idx;
    uint8_t frame_base;
    uint8_t __padding_3;
};

struct CallInternalCall
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t method_idx;
    uint16_t frame_base;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct CallInternalCallShort
{
    uint8_t __code;
    uint8_t method_idx;
    uint8_t frame_base;
    uint8_t __padding_3;
};

struct CallIntrinsic
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t method_idx;
    uint16_t frame_base;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct CallIntrinsicShort
{
    uint8_t __code;
    uint8_t method_idx;
    uint8_t frame_base;
    uint8_t __padding_3;
};

struct CallPInvoke
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t method_idx;
    uint16_t frame_base;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct CallPInvokeShort
{
    uint8_t __code;
    uint8_t method_idx;
    uint8_t frame_base;
    uint8_t __padding_3;
};

struct CallAot
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t method_idx;
    uint16_t frame_base;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct CallAotShort
{
    uint8_t __code;
    uint8_t method_idx;
    uint8_t frame_base;
    uint8_t __padding_3;
};

struct CallRuntimeImplemented
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t method_idx;
    uint16_t frame_base;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct CallRuntimeImplementedShort
{
    uint8_t __code;
    uint8_t method_idx;
    uint8_t frame_base;
    uint8_t __padding_3;
};

struct CalliInterp
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t method_sig_idx;
    uint16_t method_idx;
    uint16_t frame_base;
};

struct CalliInterpShort
{
    uint8_t __code;
    uint8_t method_sig_idx;
    uint8_t method_idx;
    uint8_t frame_base;
};

struct BoxRefInplace
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t src;
    uint16_t dst;
    uint16_t klass_idx;
};

struct BoxRefInplaceShort
{
    uint8_t __code;
    uint8_t src;
    uint8_t dst;
    uint8_t klass_idx;
};

struct NewObjInterp
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t method_idx;
    uint16_t frame_base;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t total_params_stack_object_size;
};

struct NewObjInterpShort
{
    uint8_t __code;
    uint8_t method_idx;
    uint8_t frame_base;
    uint8_t __padding_3;
    uint32_t total_params_stack_object_size;
};

struct NewValueTypeInterp
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t method_idx;
    uint16_t frame_base;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t total_params_stack_object_size;
};

struct NewValueTypeInterpShort
{
    uint8_t __code;
    uint8_t method_idx;
    uint8_t frame_base;
    uint8_t __padding_3;
    uint32_t total_params_stack_object_size;
};

struct NewObjInternalCall
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t method_idx;
    uint16_t invoker_idx;
    uint16_t frame_base;
};

struct NewObjInternalCallShort
{
    uint8_t __code;
    uint8_t method_idx;
    uint8_t invoker_idx;
    uint8_t frame_base;
};

struct NewObjIntrinsic
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t method_idx;
    uint16_t invoker_idx;
    uint16_t frame_base;
};

struct NewObjIntrinsicShort
{
    uint8_t __code;
    uint8_t method_idx;
    uint8_t invoker_idx;
    uint8_t frame_base;
};

struct NewObjAot
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t method_idx;
    uint16_t frame_base;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t total_params_stack_object_size;
};

struct NewObjAotShort
{
    uint8_t __code;
    uint8_t method_idx;
    uint8_t frame_base;
    uint8_t __padding_3;
    uint32_t total_params_stack_object_size;
};

struct NewValueTypeAot
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t method_idx;
    uint16_t frame_base;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t total_params_stack_object_size;
};

struct NewValueTypeAotShort
{
    uint8_t __code;
    uint8_t method_idx;
    uint8_t frame_base;
    uint8_t __padding_3;
    uint32_t total_params_stack_object_size;
};

struct Throw
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t ex;
};

struct ThrowShort
{
    uint8_t __code;
    uint8_t ex;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct Rethrow
{
    uint8_t __prefix;
    uint8_t __code;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct RethrowShort
{
    uint8_t __code;
    uint8_t __padding_1;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct LeaveTryWithFinally
{
    uint8_t __prefix;
    uint8_t __code;
    uint8_t first_finally_clause_index;
    uint8_t finally_clauses_count;
    int32_t target_offset;
};

struct LeaveTryWithFinallyShort
{
    uint8_t __code;
    uint8_t first_finally_clause_index;
    int8_t target_offset;
    uint8_t finally_clauses_count;
};

struct LeaveCatchWithFinally
{
    uint8_t __prefix;
    uint8_t __code;
    uint8_t first_finally_clause_index;
    uint8_t finally_clauses_count;
    int32_t target_offset;
};

struct LeaveCatchWithFinallyShort
{
    uint8_t __code;
    uint8_t first_finally_clause_index;
    int8_t target_offset;
    uint8_t finally_clauses_count;
};

struct LeaveCatchWithoutFinally
{
    uint8_t __prefix;
    uint8_t __code;
    uint8_t __padding_2;
    uint8_t __padding_3;
    int32_t target_offset;
};

struct LeaveCatchWithoutFinallyShort
{
    uint8_t __code;
    int8_t target_offset;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct EndFilter
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t cond;
};

struct EndFilterShort
{
    uint8_t __code;
    uint8_t cond;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct EndFinally
{
    uint8_t __prefix;
    uint8_t __code;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct EndFinallyShort
{
    uint8_t __code;
    uint8_t __padding_1;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct EndFault
{
    uint8_t __prefix;
    uint8_t __code;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct EndFaultShort
{
    uint8_t __code;
    uint8_t __padding_1;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct GetEnumLongHashCode
{
    uint8_t __prefix;
    uint8_t __code;
    uint16_t value_ptr;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

//}}LOW_LEVEL_INSTRUCTION_STRUCTSS

struct GeneralInst;

class OpCodes
{
  public:
    static size_t get_instruction_size(OpCodeEnum opcode, const GeneralInst& inst)
    {
        if (opcode != OpCodeEnum::Switch)
        {
            return s_opsizes[static_cast<size_t>(opcode)];
        }
        else
        {
            return get_switch_instruction_size(inst);
        }
    }

    static uint8_t* write_instruction_to_data(uint8_t* codes_cur, const GeneralInst& inst);

  private:
    static size_t get_switch_instruction_size(const GeneralInst& inst);
    static size_t s_opsizes[static_cast<size_t>(OpCodeEnum::__Count)];
};

} // namespace ll
} // namespace interp
} // namespace leanclr
