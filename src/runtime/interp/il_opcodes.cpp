#include "il_opcodes.h"
#include "utils/mem_op.h"

namespace leanclr
{
namespace interp
{
namespace il
{

static const OpCodeInfo g_opcodes[] = {
    //{{OPCODE_INFO
    {OpCodeEnum::Nop, ArgType::None, 0, 0xFF, 0x00, FlowType::Next, 0},
    {OpCodeEnum::Break, ArgType::None, 0, 0xFF, 0x01, FlowType::Break, 0},
    {OpCodeEnum::LdArg0, ArgType::None, 0, 0xFF, 0x02, FlowType::Next, 0},
    {OpCodeEnum::LdArg1, ArgType::None, 0, 0xFF, 0x03, FlowType::Next, 1},
    {OpCodeEnum::LdArg2, ArgType::None, 0, 0xFF, 0x04, FlowType::Next, 2},
    {OpCodeEnum::LdArg3, ArgType::None, 0, 0xFF, 0x05, FlowType::Next, 3},
    {OpCodeEnum::LdLoc0, ArgType::None, 0, 0xFF, 0x06, FlowType::Next, 0},
    {OpCodeEnum::LdLoc1, ArgType::None, 0, 0xFF, 0x07, FlowType::Next, 1},
    {OpCodeEnum::LdLoc2, ArgType::None, 0, 0xFF, 0x08, FlowType::Next, 2},
    {OpCodeEnum::LdLoc3, ArgType::None, 0, 0xFF, 0x09, FlowType::Next, 3},
    {OpCodeEnum::StLoc0, ArgType::None, 0, 0xFF, 0x0A, FlowType::Next, 0},
    {OpCodeEnum::StLoc1, ArgType::None, 0, 0xFF, 0x0B, FlowType::Next, 1},
    {OpCodeEnum::StLoc2, ArgType::None, 0, 0xFF, 0x0C, FlowType::Next, 2},
    {OpCodeEnum::StLoc3, ArgType::None, 0, 0xFF, 0x0D, FlowType::Next, 3},
    {OpCodeEnum::LdArgS, ArgType::Data, 1, 0xFF, 0x0E, FlowType::Next, 0},
    {OpCodeEnum::LdArgaS, ArgType::Data, 1, 0xFF, 0x0F, FlowType::Next, 0},
    {OpCodeEnum::StArgS, ArgType::Data, 1, 0xFF, 0x10, FlowType::Next, 0},
    {OpCodeEnum::LdLocS, ArgType::Data, 1, 0xFF, 0x11, FlowType::Next, 0},
    {OpCodeEnum::LdLocaS, ArgType::Data, 1, 0xFF, 0x12, FlowType::Next, 0},
    {OpCodeEnum::StLocS, ArgType::Data, 1, 0xFF, 0x13, FlowType::Next, 0},
    {OpCodeEnum::LdNull, ArgType::None, 0, 0xFF, 0x14, FlowType::Next, 0},
    {OpCodeEnum::LdcI4M1, ArgType::None, 0, 0xFF, 0x15, FlowType::Next, -1},
    {OpCodeEnum::LdcI40, ArgType::None, 0, 0xFF, 0x16, FlowType::Next, 0},
    {OpCodeEnum::LdcI41, ArgType::None, 0, 0xFF, 0x17, FlowType::Next, 1},
    {OpCodeEnum::LdcI42, ArgType::None, 0, 0xFF, 0x18, FlowType::Next, 2},
    {OpCodeEnum::LdcI43, ArgType::None, 0, 0xFF, 0x19, FlowType::Next, 3},
    {OpCodeEnum::LdcI44, ArgType::None, 0, 0xFF, 0x1A, FlowType::Next, 4},
    {OpCodeEnum::LdcI45, ArgType::None, 0, 0xFF, 0x1B, FlowType::Next, 5},
    {OpCodeEnum::LdcI46, ArgType::None, 0, 0xFF, 0x1C, FlowType::Next, 6},
    {OpCodeEnum::LdcI47, ArgType::None, 0, 0xFF, 0x1D, FlowType::Next, 7},
    {OpCodeEnum::LdcI48, ArgType::None, 0, 0xFF, 0x1E, FlowType::Next, 8},
    {OpCodeEnum::LdcI4S, ArgType::Data, 1, 0xFF, 0x1F, FlowType::Next, 0},
    {OpCodeEnum::LdcI4, ArgType::Data, 4, 0xFF, 0x20, FlowType::Next, 0},
    {OpCodeEnum::LdcI8, ArgType::Data, 8, 0xFF, 0x21, FlowType::Next, 0},
    {OpCodeEnum::LdcR4, ArgType::Data, 4, 0xFF, 0x22, FlowType::Next, 0},
    {OpCodeEnum::LdcR8, ArgType::Data, 8, 0xFF, 0x23, FlowType::Next, 0},
    {OpCodeEnum::Unused99, ArgType::None, 0, 0xFF, 0x24, FlowType::Next, 0},
    {OpCodeEnum::Dup, ArgType::None, 0, 0xFF, 0x25, FlowType::Next, 0},
    {OpCodeEnum::Pop, ArgType::None, 0, 0xFF, 0x26, FlowType::Next, 0},
    {OpCodeEnum::Jmp, ArgType::Data, 4, 0xFF, 0x27, FlowType::Call, 0},
    {OpCodeEnum::Call, ArgType::Data, 4, 0xFF, 0x28, FlowType::Call, 0},
    {OpCodeEnum::Calli, ArgType::Data, 4, 0xFF, 0x29, FlowType::Call, 0},
    {OpCodeEnum::Ret, ArgType::StaticBranch, 0, 0xFF, 0x2A, FlowType::Return, 0},
    {OpCodeEnum::BrS, ArgType::BranchTarget, 1, 0xFF, 0x2B, FlowType::Branch, 0},
    {OpCodeEnum::BrfalseS, ArgType::BranchTarget, 1, 0xFF, 0x2C, FlowType::CondBranch, 0},
    {OpCodeEnum::BrtrueS, ArgType::BranchTarget, 1, 0xFF, 0x2D, FlowType::CondBranch, 0},
    {OpCodeEnum::BeqS, ArgType::BranchTarget, 1, 0xFF, 0x2E, FlowType::CondBranch, 0},
    {OpCodeEnum::BgeS, ArgType::BranchTarget, 1, 0xFF, 0x2F, FlowType::CondBranch, 0},
    {OpCodeEnum::BgtS, ArgType::BranchTarget, 1, 0xFF, 0x30, FlowType::CondBranch, 0},
    {OpCodeEnum::BleS, ArgType::BranchTarget, 1, 0xFF, 0x31, FlowType::CondBranch, 0},
    {OpCodeEnum::BltS, ArgType::BranchTarget, 1, 0xFF, 0x32, FlowType::CondBranch, 0},
    {OpCodeEnum::BneUnS, ArgType::BranchTarget, 1, 0xFF, 0x33, FlowType::CondBranch, 0},
    {OpCodeEnum::BgeUnS, ArgType::BranchTarget, 1, 0xFF, 0x34, FlowType::CondBranch, 0},
    {OpCodeEnum::BgtUnS, ArgType::BranchTarget, 1, 0xFF, 0x35, FlowType::CondBranch, 0},
    {OpCodeEnum::BleUnS, ArgType::BranchTarget, 1, 0xFF, 0x36, FlowType::CondBranch, 0},
    {OpCodeEnum::BltUnS, ArgType::BranchTarget, 1, 0xFF, 0x37, FlowType::CondBranch, 0},
    {OpCodeEnum::Br, ArgType::BranchTarget, 4, 0xFF, 0x38, FlowType::Branch, 0},
    {OpCodeEnum::Brfalse, ArgType::BranchTarget, 4, 0xFF, 0x39, FlowType::CondBranch, 0},
    {OpCodeEnum::Brtrue, ArgType::BranchTarget, 4, 0xFF, 0x3A, FlowType::CondBranch, 0},
    {OpCodeEnum::Beq, ArgType::BranchTarget, 4, 0xFF, 0x3B, FlowType::CondBranch, 0},
    {OpCodeEnum::Bge, ArgType::BranchTarget, 4, 0xFF, 0x3C, FlowType::CondBranch, 0},
    {OpCodeEnum::Bgt, ArgType::BranchTarget, 4, 0xFF, 0x3D, FlowType::CondBranch, 0},
    {OpCodeEnum::Ble, ArgType::BranchTarget, 4, 0xFF, 0x3E, FlowType::CondBranch, 0},
    {OpCodeEnum::Blt, ArgType::BranchTarget, 4, 0xFF, 0x3F, FlowType::CondBranch, 0},
    {OpCodeEnum::BneUn, ArgType::BranchTarget, 4, 0xFF, 0x40, FlowType::CondBranch, 0},
    {OpCodeEnum::BgeUn, ArgType::BranchTarget, 4, 0xFF, 0x41, FlowType::CondBranch, 0},
    {OpCodeEnum::BgtUn, ArgType::BranchTarget, 4, 0xFF, 0x42, FlowType::CondBranch, 0},
    {OpCodeEnum::BleUn, ArgType::BranchTarget, 4, 0xFF, 0x43, FlowType::CondBranch, 0},
    {OpCodeEnum::BltUn, ArgType::BranchTarget, 4, 0xFF, 0x44, FlowType::CondBranch, 0},
    {OpCodeEnum::Switch, ArgType::Switch, -1, 0xFF, 0x45, FlowType::CondBranch, 0},
    {OpCodeEnum::LdIndI1, ArgType::None, 0, 0xFF, 0x46, FlowType::Next, 0},
    {OpCodeEnum::LdIndU1, ArgType::None, 0, 0xFF, 0x47, FlowType::Next, 0},
    {OpCodeEnum::LdIndI2, ArgType::None, 0, 0xFF, 0x48, FlowType::Next, 0},
    {OpCodeEnum::LdIndU2, ArgType::None, 0, 0xFF, 0x49, FlowType::Next, 0},
    {OpCodeEnum::LdIndI4, ArgType::None, 0, 0xFF, 0x4A, FlowType::Next, 0},
    {OpCodeEnum::LdIndU4, ArgType::None, 0, 0xFF, 0x4B, FlowType::Next, 0},
    {OpCodeEnum::LdIndI8, ArgType::None, 0, 0xFF, 0x4C, FlowType::Next, 0},
    {OpCodeEnum::LdIndI, ArgType::None, 0, 0xFF, 0x4D, FlowType::Next, 0},
    {OpCodeEnum::LdIndR4, ArgType::None, 0, 0xFF, 0x4E, FlowType::Next, 0},
    {OpCodeEnum::LdIndR8, ArgType::None, 0, 0xFF, 0x4F, FlowType::Next, 0},
    {OpCodeEnum::LdIndRef, ArgType::None, 0, 0xFF, 0x50, FlowType::Next, 0},
    {OpCodeEnum::StIndRef, ArgType::None, 0, 0xFF, 0x51, FlowType::Next, 0},
    {OpCodeEnum::StIndI1, ArgType::None, 0, 0xFF, 0x52, FlowType::Next, 0},
    {OpCodeEnum::StIndI2, ArgType::None, 0, 0xFF, 0x53, FlowType::Next, 0},
    {OpCodeEnum::StIndI4, ArgType::None, 0, 0xFF, 0x54, FlowType::Next, 0},
    {OpCodeEnum::StIndI8, ArgType::None, 0, 0xFF, 0x55, FlowType::Next, 0},
    {OpCodeEnum::StIndR4, ArgType::None, 0, 0xFF, 0x56, FlowType::Next, 0},
    {OpCodeEnum::StIndR8, ArgType::None, 0, 0xFF, 0x57, FlowType::Next, 0},
    {OpCodeEnum::Add, ArgType::None, 0, 0xFF, 0x58, FlowType::Next, 0},
    {OpCodeEnum::Sub, ArgType::None, 0, 0xFF, 0x59, FlowType::Next, 0},
    {OpCodeEnum::Mul, ArgType::None, 0, 0xFF, 0x5A, FlowType::Next, 0},
    {OpCodeEnum::Div, ArgType::None, 0, 0xFF, 0x5B, FlowType::Next, 0},
    {OpCodeEnum::DivUn, ArgType::None, 0, 0xFF, 0x5C, FlowType::Next, 0},
    {OpCodeEnum::Rem, ArgType::None, 0, 0xFF, 0x5D, FlowType::Next, 0},
    {OpCodeEnum::RemUn, ArgType::None, 0, 0xFF, 0x5E, FlowType::Next, 0},
    {OpCodeEnum::And, ArgType::None, 0, 0xFF, 0x5F, FlowType::Next, 0},
    {OpCodeEnum::Or, ArgType::None, 0, 0xFF, 0x60, FlowType::Next, 0},
    {OpCodeEnum::Xor, ArgType::None, 0, 0xFF, 0x61, FlowType::Next, 0},
    {OpCodeEnum::Shl, ArgType::None, 0, 0xFF, 0x62, FlowType::Next, 0},
    {OpCodeEnum::Shr, ArgType::None, 0, 0xFF, 0x63, FlowType::Next, 0},
    {OpCodeEnum::ShrUn, ArgType::None, 0, 0xFF, 0x64, FlowType::Next, 0},
    {OpCodeEnum::Neg, ArgType::None, 0, 0xFF, 0x65, FlowType::Next, 0},
    {OpCodeEnum::Not, ArgType::None, 0, 0xFF, 0x66, FlowType::Next, 0},
    {OpCodeEnum::ConvI1, ArgType::None, 0, 0xFF, 0x67, FlowType::Next, 0},
    {OpCodeEnum::ConvI2, ArgType::None, 0, 0xFF, 0x68, FlowType::Next, 0},
    {OpCodeEnum::ConvI4, ArgType::None, 0, 0xFF, 0x69, FlowType::Next, 0},
    {OpCodeEnum::ConvI8, ArgType::None, 0, 0xFF, 0x6A, FlowType::Next, 0},
    {OpCodeEnum::ConvR4, ArgType::None, 0, 0xFF, 0x6B, FlowType::Next, 0},
    {OpCodeEnum::ConvR8, ArgType::None, 0, 0xFF, 0x6C, FlowType::Next, 0},
    {OpCodeEnum::ConvU4, ArgType::None, 0, 0xFF, 0x6D, FlowType::Next, 0},
    {OpCodeEnum::ConvU8, ArgType::None, 0, 0xFF, 0x6E, FlowType::Next, 0},
    {OpCodeEnum::Callvirt, ArgType::Data, 4, 0xFF, 0x6F, FlowType::Call, 0},
    {OpCodeEnum::Cpobj, ArgType::Data, 4, 0xFF, 0x70, FlowType::Next, 0},
    {OpCodeEnum::Ldobj, ArgType::Data, 4, 0xFF, 0x71, FlowType::Next, 0},
    {OpCodeEnum::Ldstr, ArgType::Data, 4, 0xFF, 0x72, FlowType::Next, 0},
    {OpCodeEnum::Newobj, ArgType::Data, 4, 0xFF, 0x73, FlowType::Call, 0},
    {OpCodeEnum::Castclass, ArgType::Data, 4, 0xFF, 0x74, FlowType::Next, 0},
    {OpCodeEnum::Isinst, ArgType::Data, 4, 0xFF, 0x75, FlowType::Next, 0},
    {OpCodeEnum::ConvRUn, ArgType::None, 0, 0xFF, 0x76, FlowType::Next, 0},
    {OpCodeEnum::Unused58, ArgType::None, 0, 0xFF, 0x77, FlowType::Next, 0},
    {OpCodeEnum::Unused1, ArgType::None, 0, 0xFF, 0x78, FlowType::Next, 0},
    {OpCodeEnum::Unbox, ArgType::Data, 4, 0xFF, 0x79, FlowType::Next, 0},
    {OpCodeEnum::Throw, ArgType::StaticBranch, 0, 0xFF, 0x7A, FlowType::Throw, 0},
    {OpCodeEnum::Ldfld, ArgType::Data, 4, 0xFF, 0x7B, FlowType::Next, 0},
    {OpCodeEnum::Ldflda, ArgType::Data, 4, 0xFF, 0x7C, FlowType::Next, 0},
    {OpCodeEnum::Stfld, ArgType::Data, 4, 0xFF, 0x7D, FlowType::Next, 0},
    {OpCodeEnum::Ldsfld, ArgType::Data, 4, 0xFF, 0x7E, FlowType::Next, 0},
    {OpCodeEnum::Ldsflda, ArgType::Data, 4, 0xFF, 0x7F, FlowType::Next, 0},
    {OpCodeEnum::Stsfld, ArgType::Data, 4, 0xFF, 0x80, FlowType::Next, 0},
    {OpCodeEnum::Stobj, ArgType::Data, 4, 0xFF, 0x81, FlowType::Next, 0},
    {OpCodeEnum::ConvOvfI1Un, ArgType::None, 0, 0xFF, 0x82, FlowType::Next, 0},
    {OpCodeEnum::ConvOvfI2Un, ArgType::None, 0, 0xFF, 0x83, FlowType::Next, 0},
    {OpCodeEnum::ConvOvfI4Un, ArgType::None, 0, 0xFF, 0x84, FlowType::Next, 0},
    {OpCodeEnum::ConvOvfI8Un, ArgType::None, 0, 0xFF, 0x85, FlowType::Next, 0},
    {OpCodeEnum::ConvOvfU1Un, ArgType::None, 0, 0xFF, 0x86, FlowType::Next, 0},
    {OpCodeEnum::ConvOvfU2Un, ArgType::None, 0, 0xFF, 0x87, FlowType::Next, 0},
    {OpCodeEnum::ConvOvfU4Un, ArgType::None, 0, 0xFF, 0x88, FlowType::Next, 0},
    {OpCodeEnum::ConvOvfU8Un, ArgType::None, 0, 0xFF, 0x89, FlowType::Next, 0},
    {OpCodeEnum::ConvOvfIUn, ArgType::None, 0, 0xFF, 0x8A, FlowType::Next, 0},
    {OpCodeEnum::ConvOvfUUn, ArgType::None, 0, 0xFF, 0x8B, FlowType::Next, 0},
    {OpCodeEnum::Box, ArgType::Data, 4, 0xFF, 0x8C, FlowType::Next, 0},
    {OpCodeEnum::Newarr, ArgType::Data, 4, 0xFF, 0x8D, FlowType::Next, 0},
    {OpCodeEnum::Ldlen, ArgType::None, 0, 0xFF, 0x8E, FlowType::Next, 0},
    {OpCodeEnum::Ldelema, ArgType::Data, 4, 0xFF, 0x8F, FlowType::Next, 0},
    {OpCodeEnum::LdelemI1, ArgType::None, 0, 0xFF, 0x90, FlowType::Next, 0},
    {OpCodeEnum::LdelemU1, ArgType::None, 0, 0xFF, 0x91, FlowType::Next, 0},
    {OpCodeEnum::LdelemI2, ArgType::None, 0, 0xFF, 0x92, FlowType::Next, 0},
    {OpCodeEnum::LdelemU2, ArgType::None, 0, 0xFF, 0x93, FlowType::Next, 0},
    {OpCodeEnum::LdelemI4, ArgType::None, 0, 0xFF, 0x94, FlowType::Next, 0},
    {OpCodeEnum::LdelemU4, ArgType::None, 0, 0xFF, 0x95, FlowType::Next, 0},
    {OpCodeEnum::LdelemI8, ArgType::None, 0, 0xFF, 0x96, FlowType::Next, 0},
    {OpCodeEnum::LdelemI, ArgType::None, 0, 0xFF, 0x97, FlowType::Next, 0},
    {OpCodeEnum::LdelemR4, ArgType::None, 0, 0xFF, 0x98, FlowType::Next, 0},
    {OpCodeEnum::LdelemR8, ArgType::None, 0, 0xFF, 0x99, FlowType::Next, 0},
    {OpCodeEnum::LdelemRef, ArgType::None, 0, 0xFF, 0x9A, FlowType::Next, 0},
    {OpCodeEnum::StelemI, ArgType::None, 0, 0xFF, 0x9B, FlowType::Next, 0},
    {OpCodeEnum::StelemI1, ArgType::None, 0, 0xFF, 0x9C, FlowType::Next, 0},
    {OpCodeEnum::StelemI2, ArgType::None, 0, 0xFF, 0x9D, FlowType::Next, 0},
    {OpCodeEnum::StelemI4, ArgType::None, 0, 0xFF, 0x9E, FlowType::Next, 0},
    {OpCodeEnum::StelemI8, ArgType::None, 0, 0xFF, 0x9F, FlowType::Next, 0},
    {OpCodeEnum::StelemR4, ArgType::None, 0, 0xFF, 0xA0, FlowType::Next, 0},
    {OpCodeEnum::StelemR8, ArgType::None, 0, 0xFF, 0xA1, FlowType::Next, 0},
    {OpCodeEnum::StelemRef, ArgType::None, 0, 0xFF, 0xA2, FlowType::Next, 0},
    {OpCodeEnum::Ldelem, ArgType::Data, 4, 0xFF, 0xA3, FlowType::Next, 0},
    {OpCodeEnum::Stelem, ArgType::Data, 4, 0xFF, 0xA4, FlowType::Next, 0},
    {OpCodeEnum::UnboxAny, ArgType::Data, 4, 0xFF, 0xA5, FlowType::Next, 0},
    {OpCodeEnum::Unused5, ArgType::None, 0, 0xFF, 0xA6, FlowType::Next, 0},
    {OpCodeEnum::Unused6, ArgType::None, 0, 0xFF, 0xA7, FlowType::Next, 0},
    {OpCodeEnum::Unused7, ArgType::None, 0, 0xFF, 0xA8, FlowType::Next, 0},
    {OpCodeEnum::Unused8, ArgType::None, 0, 0xFF, 0xA9, FlowType::Next, 0},
    {OpCodeEnum::Unused9, ArgType::None, 0, 0xFF, 0xAA, FlowType::Next, 0},
    {OpCodeEnum::Unused10, ArgType::None, 0, 0xFF, 0xAB, FlowType::Next, 0},
    {OpCodeEnum::Unused11, ArgType::None, 0, 0xFF, 0xAC, FlowType::Next, 0},
    {OpCodeEnum::Unused12, ArgType::None, 0, 0xFF, 0xAD, FlowType::Next, 0},
    {OpCodeEnum::Unused13, ArgType::None, 0, 0xFF, 0xAE, FlowType::Next, 0},
    {OpCodeEnum::Unused14, ArgType::None, 0, 0xFF, 0xAF, FlowType::Next, 0},
    {OpCodeEnum::Unused15, ArgType::None, 0, 0xFF, 0xB0, FlowType::Next, 0},
    {OpCodeEnum::Unused16, ArgType::None, 0, 0xFF, 0xB1, FlowType::Next, 0},
    {OpCodeEnum::Unused17, ArgType::None, 0, 0xFF, 0xB2, FlowType::Next, 0},
    {OpCodeEnum::ConvOvfI1, ArgType::None, 0, 0xFF, 0xB3, FlowType::Next, 0},
    {OpCodeEnum::ConvOvfU1, ArgType::None, 0, 0xFF, 0xB4, FlowType::Next, 0},
    {OpCodeEnum::ConvOvfI2, ArgType::None, 0, 0xFF, 0xB5, FlowType::Next, 0},
    {OpCodeEnum::ConvOvfU2, ArgType::None, 0, 0xFF, 0xB6, FlowType::Next, 0},
    {OpCodeEnum::ConvOvfI4, ArgType::None, 0, 0xFF, 0xB7, FlowType::Next, 0},
    {OpCodeEnum::ConvOvfU4, ArgType::None, 0, 0xFF, 0xB8, FlowType::Next, 0},
    {OpCodeEnum::ConvOvfI8, ArgType::None, 0, 0xFF, 0xB9, FlowType::Next, 0},
    {OpCodeEnum::ConvOvfU8, ArgType::None, 0, 0xFF, 0xBA, FlowType::Next, 0},
    {OpCodeEnum::Unused50, ArgType::None, 0, 0xFF, 0xBB, FlowType::Next, 0},
    {OpCodeEnum::Unused18, ArgType::None, 0, 0xFF, 0xBC, FlowType::Next, 0},
    {OpCodeEnum::Unused19, ArgType::None, 0, 0xFF, 0xBD, FlowType::Next, 0},
    {OpCodeEnum::Unused20, ArgType::None, 0, 0xFF, 0xBE, FlowType::Next, 0},
    {OpCodeEnum::Unused21, ArgType::None, 0, 0xFF, 0xBF, FlowType::Next, 0},
    {OpCodeEnum::Unused22, ArgType::None, 0, 0xFF, 0xC0, FlowType::Next, 0},
    {OpCodeEnum::Unused23, ArgType::None, 0, 0xFF, 0xC1, FlowType::Next, 0},
    {OpCodeEnum::Refanyval, ArgType::Data, 4, 0xFF, 0xC2, FlowType::Next, 0},
    {OpCodeEnum::Ckfinite, ArgType::None, 0, 0xFF, 0xC3, FlowType::Next, 0},
    {OpCodeEnum::Unused24, ArgType::None, 0, 0xFF, 0xC4, FlowType::Next, 0},
    {OpCodeEnum::Unused25, ArgType::None, 0, 0xFF, 0xC5, FlowType::Next, 0},
    {OpCodeEnum::Mkrefany, ArgType::Data, 4, 0xFF, 0xC6, FlowType::Next, 0},
    {OpCodeEnum::Unused59, ArgType::None, 0, 0xFF, 0xC7, FlowType::Next, 0},
    {OpCodeEnum::Unused60, ArgType::None, 0, 0xFF, 0xC8, FlowType::Next, 0},
    {OpCodeEnum::Unused61, ArgType::None, 0, 0xFF, 0xC9, FlowType::Next, 0},
    {OpCodeEnum::Unused62, ArgType::None, 0, 0xFF, 0xCA, FlowType::Next, 0},
    {OpCodeEnum::Unused63, ArgType::None, 0, 0xFF, 0xCB, FlowType::Next, 0},
    {OpCodeEnum::Unused64, ArgType::None, 0, 0xFF, 0xCC, FlowType::Next, 0},
    {OpCodeEnum::Unused65, ArgType::None, 0, 0xFF, 0xCD, FlowType::Next, 0},
    {OpCodeEnum::Unused66, ArgType::None, 0, 0xFF, 0xCE, FlowType::Next, 0},
    {OpCodeEnum::Unused67, ArgType::None, 0, 0xFF, 0xCF, FlowType::Next, 0},
    {OpCodeEnum::Ldtoken, ArgType::Data, 4, 0xFF, 0xD0, FlowType::Next, 0},
    {OpCodeEnum::ConvU2, ArgType::None, 0, 0xFF, 0xD1, FlowType::Next, 0},
    {OpCodeEnum::ConvU1, ArgType::None, 0, 0xFF, 0xD2, FlowType::Next, 0},
    {OpCodeEnum::ConvI, ArgType::None, 0, 0xFF, 0xD3, FlowType::Next, 0},
    {OpCodeEnum::ConvOvfI, ArgType::None, 0, 0xFF, 0xD4, FlowType::Next, 0},
    {OpCodeEnum::ConvOvfU, ArgType::None, 0, 0xFF, 0xD5, FlowType::Next, 0},
    {OpCodeEnum::AddOvf, ArgType::None, 0, 0xFF, 0xD6, FlowType::Next, 0},
    {OpCodeEnum::AddOvfUn, ArgType::None, 0, 0xFF, 0xD7, FlowType::Next, 0},
    {OpCodeEnum::MulOvf, ArgType::None, 0, 0xFF, 0xD8, FlowType::Next, 0},
    {OpCodeEnum::MulOvfUn, ArgType::None, 0, 0xFF, 0xD9, FlowType::Next, 0},
    {OpCodeEnum::SubOvf, ArgType::None, 0, 0xFF, 0xDA, FlowType::Next, 0},
    {OpCodeEnum::SubOvfUn, ArgType::None, 0, 0xFF, 0xDB, FlowType::Next, 0},
    {OpCodeEnum::Endfinally, ArgType::StaticBranch, 0, 0xFF, 0xDC, FlowType::Return, 0},
    {OpCodeEnum::Leave, ArgType::BranchTarget, 4, 0xFF, 0xDD, FlowType::Branch, 0},
    {OpCodeEnum::LeaveS, ArgType::BranchTarget, 1, 0xFF, 0xDE, FlowType::Branch, 0},
    {OpCodeEnum::StIndI, ArgType::None, 0, 0xFF, 0xDF, FlowType::Next, 0},
    {OpCodeEnum::ConvU, ArgType::None, 0, 0xFF, 0xE0, FlowType::Next, 0},
    {OpCodeEnum::Unused26, ArgType::None, 0, 0xFF, 0xE1, FlowType::Next, 0},
    {OpCodeEnum::Unused27, ArgType::None, 0, 0xFF, 0xE2, FlowType::Next, 0},
    {OpCodeEnum::Unused28, ArgType::None, 0, 0xFF, 0xE3, FlowType::Next, 0},
    {OpCodeEnum::Unused29, ArgType::None, 0, 0xFF, 0xE4, FlowType::Next, 0},
    {OpCodeEnum::Unused30, ArgType::None, 0, 0xFF, 0xE5, FlowType::Next, 0},
    {OpCodeEnum::Unused31, ArgType::None, 0, 0xFF, 0xE6, FlowType::Next, 0},
    {OpCodeEnum::Unused32, ArgType::None, 0, 0xFF, 0xE7, FlowType::Next, 0},
    {OpCodeEnum::Unused33, ArgType::None, 0, 0xFF, 0xE8, FlowType::Next, 0},
    {OpCodeEnum::Unused34, ArgType::None, 0, 0xFF, 0xE9, FlowType::Next, 0},
    {OpCodeEnum::Unused35, ArgType::None, 0, 0xFF, 0xEA, FlowType::Next, 0},
    {OpCodeEnum::Unused36, ArgType::None, 0, 0xFF, 0xEB, FlowType::Next, 0},
    {OpCodeEnum::Unused37, ArgType::None, 0, 0xFF, 0xEC, FlowType::Next, 0},
    {OpCodeEnum::Unused38, ArgType::None, 0, 0xFF, 0xED, FlowType::Next, 0},
    {OpCodeEnum::Unused39, ArgType::None, 0, 0xFF, 0xEE, FlowType::Next, 0},
    {OpCodeEnum::Unused40, ArgType::None, 0, 0xFF, 0xEF, FlowType::Next, 0},
    {OpCodeEnum::Unused41, ArgType::None, 0, 0xFF, 0xF0, FlowType::Next, 0},
    {OpCodeEnum::Unused42, ArgType::None, 0, 0xFF, 0xF1, FlowType::Next, 0},
    {OpCodeEnum::Unused43, ArgType::None, 0, 0xFF, 0xF2, FlowType::Next, 0},
    {OpCodeEnum::Unused44, ArgType::None, 0, 0xFF, 0xF3, FlowType::Next, 0},
    {OpCodeEnum::Unused45, ArgType::None, 0, 0xFF, 0xF4, FlowType::Next, 0},
    {OpCodeEnum::Unused46, ArgType::None, 0, 0xFF, 0xF5, FlowType::Next, 0},
    {OpCodeEnum::Unused47, ArgType::None, 0, 0xFF, 0xF6, FlowType::Next, 0},
    {OpCodeEnum::Unused48, ArgType::None, 0, 0xFF, 0xF7, FlowType::Next, 0},
    {OpCodeEnum::Prefix7, ArgType::None, 0, 0xFF, 0xF8, FlowType::Meta, 0},
    {OpCodeEnum::Prefix6, ArgType::None, 0, 0xFF, 0xF9, FlowType::Meta, 0},
    {OpCodeEnum::Prefix5, ArgType::None, 0, 0xFF, 0xFA, FlowType::Meta, 0},
    {OpCodeEnum::Prefix4, ArgType::None, 0, 0xFF, 0xFB, FlowType::Meta, 0},
    {OpCodeEnum::Prefix3, ArgType::None, 0, 0xFF, 0xFC, FlowType::Meta, 0},
    {OpCodeEnum::Prefix2, ArgType::None, 0, 0xFF, 0xFD, FlowType::Meta, 0},
    {OpCodeEnum::Prefix1, ArgType::None, 0, 0xFF, 0xFE, FlowType::Meta, 0},
    {OpCodeEnum::Prefixref, ArgType::None, 0, 0xFF, 0xFF, FlowType::Meta, 0},
    {OpCodeEnum::Arglist, ArgType::None, 0, 0xFE, 0x00, FlowType::Next, 0},
    {OpCodeEnum::Ceq, ArgType::None, 0, 0xFE, 0x01, FlowType::Next, 0},
    {OpCodeEnum::Cgt, ArgType::None, 0, 0xFE, 0x02, FlowType::Next, 0},
    {OpCodeEnum::CgtUn, ArgType::None, 0, 0xFE, 0x03, FlowType::Next, 0},
    {OpCodeEnum::Clt, ArgType::None, 0, 0xFE, 0x04, FlowType::Next, 0},
    {OpCodeEnum::CltUn, ArgType::None, 0, 0xFE, 0x05, FlowType::Next, 0},
    {OpCodeEnum::Ldftn, ArgType::Data, 4, 0xFE, 0x06, FlowType::Next, 0},
    {OpCodeEnum::Ldvirtftn, ArgType::Data, 4, 0xFE, 0x07, FlowType::Next, 0},
    {OpCodeEnum::Unused56, ArgType::None, 0, 0xFE, 0x08, FlowType::Next, 0},
    {OpCodeEnum::Ldarg, ArgType::Data, 2, 0xFE, 0x09, FlowType::Next, 0},
    {OpCodeEnum::Ldarga, ArgType::Data, 2, 0xFE, 0x0A, FlowType::Next, 0},
    {OpCodeEnum::Starg, ArgType::Data, 2, 0xFE, 0x0B, FlowType::Next, 0},
    {OpCodeEnum::Ldloc, ArgType::Data, 2, 0xFE, 0x0C, FlowType::Next, 0},
    {OpCodeEnum::Ldloca, ArgType::Data, 2, 0xFE, 0x0D, FlowType::Next, 0},
    {OpCodeEnum::Stloc, ArgType::Data, 2, 0xFE, 0x0E, FlowType::Next, 0},
    {OpCodeEnum::Localloc, ArgType::None, 0, 0xFE, 0x0F, FlowType::Next, 0},
    {OpCodeEnum::Unused57, ArgType::None, 0, 0xFE, 0x10, FlowType::Next, 0},
    {OpCodeEnum::Endfilter, ArgType::None, 0, 0xFE, 0x11, FlowType::Return, 0},
    {OpCodeEnum::Unaligned, ArgType::Data, 1, 0xFE, 0x12, FlowType::Meta, 0},
    {OpCodeEnum::Volatile, ArgType::None, 0, 0xFE, 0x13, FlowType::Meta, 0},
    {OpCodeEnum::Tail, ArgType::None, 0, 0xFE, 0x14, FlowType::Meta, 0},
    {OpCodeEnum::Initobj, ArgType::Data, 4, 0xFE, 0x15, FlowType::Next, 0},
    {OpCodeEnum::Constrained, ArgType::Data, 4, 0xFE, 0x16, FlowType::Meta, 0},
    {OpCodeEnum::Cpblk, ArgType::None, 0, 0xFE, 0x17, FlowType::Next, 0},
    {OpCodeEnum::Initblk, ArgType::None, 0, 0xFE, 0x18, FlowType::Next, 0},
    {OpCodeEnum::No, ArgType::Data, 1, 0xFE, 0x19, FlowType::Next, 0},
    {OpCodeEnum::Rethrow, ArgType::StaticBranch, 0, 0xFE, 0x1A, FlowType::Throw, 0},
    {OpCodeEnum::Unused, ArgType::None, 0, 0xFE, 0x1B, FlowType::Next, 0},
    {OpCodeEnum::Sizeof, ArgType::Data, 4, 0xFE, 0x1C, FlowType::Next, 0},
    {OpCodeEnum::Refanytype, ArgType::None, 0, 0xFE, 0x1D, FlowType::Next, 0},
    {OpCodeEnum::Readonly, ArgType::None, 0, 0xFE, 0x1E, FlowType::Meta, 0},
    {OpCodeEnum::Unused53, ArgType::None, 0, 0xFE, 0x1F, FlowType::Next, 0},
    {OpCodeEnum::Unused54, ArgType::None, 0, 0xFE, 0x20, FlowType::Next, 0},
    {OpCodeEnum::Unused55, ArgType::None, 0, 0xFE, 0x21, FlowType::Next, 0},
    {OpCodeEnum::Unused70, ArgType::None, 0, 0xFE, 0x22, FlowType::Next, 0},
    {OpCodeEnum::Illegal, ArgType::None, 0, 0x00, 0x00, FlowType::Meta, 0},
    {OpCodeEnum::Endmac, ArgType::None, 0, 0x00, 0x00, FlowType::Meta, 0},

    //}}OPCODE_INFO
};

const OpCodeInfo* OpCodes::get_opcode_info(OpCodeEnum opcode)
{
    assert(static_cast<size_t>(opcode) < sizeof(g_opcodes) / sizeof(g_opcodes[0]));
    return &g_opcodes[static_cast<size_t>(opcode)];
}

bool OpCodes::try_decode_opcode_info(const uint8_t* codes_cur, const uint8_t* codes_end, const OpCodeInfo*& out_opcode_info)
{
    out_opcode_info = nullptr;
    if (codes_cur == nullptr || codes_cur >= codes_end)
    {
        return false;
    }

    uint8_t first_byte = *codes_cur;
    if (first_byte < static_cast<uint8_t>(OpCodeValue::Prefix7))
    {
        out_opcode_info = &g_opcodes[static_cast<size_t>(first_byte)];
        return true;
    }

    if (first_byte == static_cast<uint8_t>(OpCodeValue::Prefix1))
    {
        const uint8_t* pc = codes_cur + 1;
        if (pc >= codes_end)
        {
            return false;
        }
        uint8_t second_byte = *pc;
        if (second_byte >= static_cast<uint8_t>(OpCodeValueExt::Unused53))
        {
            return false;
        }
        out_opcode_info = &g_opcodes[static_cast<size_t>(second_byte) + 0x100];
        return true;
    }

    return false;
}

size_t OpCodes::get_opcode_size(const uint8_t* codes_cur, const OpCodeInfo* opcode_info)
{
    if (opcode_info->inline_type != ArgType::Switch)
    {
        size_t inline_param = static_cast<size_t>(opcode_info->inline_param);
        size_t prefix_size = (*codes_cur < static_cast<uint8_t>(OpCodeValue::Prefix1)) ? 1 : 2;
        return inline_param + prefix_size;
    }

    uint32_t count = 0;
    std::memcpy(&count, codes_cur + 1, sizeof(uint32_t));
    return 1 + sizeof(uint32_t) + (sizeof(uint32_t) * utils::MemOp::read_u32_may_unaligned(codes_cur + 1));
}

} // namespace il
} // namespace interp
} // namespace leanclr
