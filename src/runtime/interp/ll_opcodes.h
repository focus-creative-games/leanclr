#pragma once

#include "vm/rt_managed_types.h"
#include "interp_defs.h"
#include "vm/field.h"

namespace leanclr
{
namespace interp
{
struct InstructionOperands;

namespace ll
{

enum class OpCodeEnum : uint16_t
{
    //{{LOW_LEVEL_OPCODE_ENUM
    Illegal = 0,
    Nop = 1,
    ProfileAddCost = 2,
    InitLocals = 3,
    Arglist = 4,
    LdLocI1 = 5,
    LdLocU1 = 6,
    LdLocI2 = 7,
    LdLocU2 = 8,
    LdLocI4 = 9,
    LdLocI8 = 10,
    LdLocAny = 11,
    LdLoca = 12,
    StLocI1 = 13,
    StLocI2 = 14,
    StLocI4 = 15,
    StLocI8 = 16,
    StLocAny = 17,
    LdNull = 18,
    LdcI4I2 = 19,
    LdcI4I4 = 20,
    LdcI8I2 = 21,
    LdcI8I4 = 22,
    LdcI8I8 = 23,
    LdStr = 24,
    Br = 25,
    BrTrueI4 = 26,
    BrTrueI8 = 27,
    BrFalseI4 = 28,
    BrFalseI8 = 29,
    BeqI4 = 30,
    BeqI8 = 31,
    BeqR4 = 32,
    BeqR8 = 33,
    BgeI4 = 34,
    BgeI8 = 35,
    BgeR4 = 36,
    BgeR8 = 37,
    BgtI4 = 38,
    BgtI8 = 39,
    BgtR4 = 40,
    BgtR8 = 41,
    BleI4 = 42,
    BleI8 = 43,
    BleR4 = 44,
    BleR8 = 45,
    BltI4 = 46,
    BltI8 = 47,
    BltR4 = 48,
    BltR8 = 49,
    BneUnI4 = 50,
    BneUnI8 = 51,
    BneUnR4 = 52,
    BneUnR8 = 53,
    BgeUnI4 = 54,
    BgeUnI8 = 55,
    BgeUnR4 = 56,
    BgeUnR8 = 57,
    BgtUnI4 = 58,
    BgtUnI8 = 59,
    BgtUnR4 = 60,
    BgtUnR8 = 61,
    BleUnI4 = 62,
    BleUnI8 = 63,
    BleUnR4 = 64,
    BleUnR8 = 65,
    BltUnI4 = 66,
    BltUnI8 = 67,
    BltUnR4 = 68,
    BltUnR8 = 69,
    Switch = 70,
    LdIndI1 = 71,
    LdIndU1 = 72,
    LdIndI2 = 73,
    LdIndI2Unaligned = 74,
    LdIndU2 = 75,
    LdIndU2Unaligned = 76,
    LdIndI4 = 77,
    LdIndI4Unaligned = 78,
    LdIndI8 = 79,
    LdIndI8Unaligned = 80,
    StIndI1 = 81,
    StIndI2 = 82,
    StIndI2Unaligned = 83,
    StIndI4 = 84,
    StIndI4Unaligned = 85,
    StIndI8 = 86,
    StIndI8Unaligned = 87,
    StIndI8I4 = 88,
    StIndI8I4Unaligned = 89,
    StIndI8U4 = 90,
    StIndI8U4Unaligned = 91,
    AddI4 = 92,
    AddI8 = 93,
    AddR4 = 94,
    AddR8 = 95,
    SubI4 = 96,
    SubI8 = 97,
    SubR4 = 98,
    SubR8 = 99,
    MulI4 = 100,
    MulI8 = 101,
    MulR4 = 102,
    MulR8 = 103,
    DivI4 = 104,
    DivI8 = 105,
    DivR4 = 106,
    DivR8 = 107,
    DivUnI4 = 108,
    DivUnI8 = 109,
    RemI4 = 110,
    RemI8 = 111,
    RemR4 = 112,
    RemR8 = 113,
    RemUnI4 = 114,
    RemUnI8 = 115,
    AndI4 = 116,
    AndI8 = 117,
    OrI4 = 118,
    OrI8 = 119,
    XorI4 = 120,
    XorI8 = 121,
    ShlI4 = 122,
    ShlI8 = 123,
    ShrI4 = 124,
    ShrI8 = 125,
    ShrUnI4 = 126,
    ShrUnI8 = 127,
    NegI4 = 128,
    NegI8 = 129,
    NegR4 = 130,
    NegR8 = 131,
    NotI4 = 132,
    NotI8 = 133,
    AddOvfI4 = 134,
    AddOvfI8 = 135,
    AddOvfUnI4 = 136,
    AddOvfUnI8 = 137,
    MulOvfI4 = 138,
    MulOvfI8 = 139,
    MulOvfUnI4 = 140,
    MulOvfUnI8 = 141,
    SubOvfI4 = 142,
    SubOvfI8 = 143,
    SubOvfUnI4 = 144,
    SubOvfUnI8 = 145,
    ConvI1I4 = 146,
    ConvI1I8 = 147,
    ConvI1R4 = 148,
    ConvI1R8 = 149,
    ConvU1I4 = 150,
    ConvU1I8 = 151,
    ConvU1R4 = 152,
    ConvU1R8 = 153,
    ConvI2I4 = 154,
    ConvI2I8 = 155,
    ConvI2R4 = 156,
    ConvI2R8 = 157,
    ConvU2I4 = 158,
    ConvU2I8 = 159,
    ConvU2R4 = 160,
    ConvU2R8 = 161,
    ConvI4I8 = 162,
    ConvI4R4 = 163,
    ConvI4R8 = 164,
    ConvU4I8 = 165,
    ConvU4R4 = 166,
    ConvU4R8 = 167,
    ConvI8I4 = 168,
    ConvI8U4 = 169,
    ConvI8R4 = 170,
    ConvI8R8 = 171,
    ConvU8I4 = 172,
    ConvU8R4 = 173,
    ConvU8R8 = 174,
    ConvR4I4 = 175,
    ConvR4I8 = 176,
    ConvR4R8 = 177,
    ConvR8I4 = 178,
    ConvR8I8 = 179,
    ConvR8R4 = 180,
    ConvOvfI1I4 = 181,
    ConvOvfI1I8 = 182,
    ConvOvfI1R4 = 183,
    ConvOvfI1R8 = 184,
    ConvOvfU1I4 = 185,
    ConvOvfU1I8 = 186,
    ConvOvfU1R4 = 187,
    ConvOvfU1R8 = 188,
    ConvOvfI2I4 = 189,
    ConvOvfI2I8 = 190,
    ConvOvfI2R4 = 191,
    ConvOvfI2R8 = 192,
    ConvOvfU2I4 = 193,
    ConvOvfU2I8 = 194,
    ConvOvfU2R4 = 195,
    ConvOvfU2R8 = 196,
    ConvOvfI4I8 = 197,
    ConvOvfI4R4 = 198,
    ConvOvfI4R8 = 199,
    ConvOvfU4I4 = 200,
    ConvOvfU4I8 = 201,
    ConvOvfU4R4 = 202,
    ConvOvfU4R8 = 203,
    ConvOvfI8R4 = 204,
    ConvOvfI8R8 = 205,
    ConvOvfU8I4 = 206,
    ConvOvfU8I8 = 207,
    ConvOvfU8R4 = 208,
    ConvOvfU8R8 = 209,
    ConvOvfI1UnI4 = 210,
    ConvOvfI1UnI8 = 211,
    ConvOvfI1UnR4 = 212,
    ConvOvfI1UnR8 = 213,
    ConvOvfU1UnI4 = 214,
    ConvOvfU1UnI8 = 215,
    ConvOvfU1UnR4 = 216,
    ConvOvfU1UnR8 = 217,
    ConvOvfI2UnI4 = 218,
    ConvOvfI2UnI8 = 219,
    ConvOvfI2UnR4 = 220,
    ConvOvfI2UnR8 = 221,
    ConvOvfU2UnI4 = 222,
    ConvOvfU2UnI8 = 223,
    ConvOvfU2UnR4 = 224,
    ConvOvfU2UnR8 = 225,
    ConvOvfI4UnI4 = 226,
    ConvOvfI4UnI8 = 227,
    ConvOvfI4UnR4 = 228,
    ConvOvfI4UnR8 = 229,
    ConvOvfU4UnI8 = 230,
    ConvOvfU4UnR4 = 231,
    ConvOvfU4UnR8 = 232,
    ConvOvfI8UnI8 = 233,
    ConvOvfI8UnR4 = 234,
    ConvOvfI8UnR8 = 235,
    ConvOvfU8UnR4 = 236,
    ConvOvfU8UnR8 = 237,
    CeqI4 = 238,
    CeqI8 = 239,
    CeqR4 = 240,
    CeqR8 = 241,
    CgtI4 = 242,
    CgtI8 = 243,
    CgtR4 = 244,
    CgtR8 = 245,
    CgtUnI4 = 246,
    CgtUnI8 = 247,
    CgtUnR4 = 248,
    CgtUnR8 = 249,
    CltI4 = 250,
    CltI8 = 251,
    CltR4 = 252,
    CltR8 = 253,
    CltUnI4 = 254,
    CltUnI8 = 255,
    CltUnR4 = 256,
    CltUnR8 = 257,
    InitObjI1 = 258,
    InitObjI2 = 259,
    InitObjI2Unaligned = 260,
    InitObjI4 = 261,
    InitObjI4Unaligned = 262,
    InitObjI8 = 263,
    InitObjI8Unaligned = 264,
    InitObjAny = 265,
    CpObjI1 = 266,
    CpObjI2 = 267,
    CpObjI4 = 268,
    CpObjI8 = 269,
    CpObjAny = 270,
    LdObjAny = 271,
    StObjAny = 272,
    CastClass = 273,
    IsInst = 274,
    Box = 275,
    Unbox = 276,
    UnboxAny = 277,
    NewArr = 278,
    LdLen = 279,
    Ldelema = 280,
    LdelemaReadOnly = 281,
    LdelemI1 = 282,
    LdelemU1 = 283,
    LdelemI2 = 284,
    LdelemU2 = 285,
    LdelemI4 = 286,
    LdelemI8 = 287,
    LdelemI = 288,
    LdelemR4 = 289,
    LdelemR8 = 290,
    LdelemRef = 291,
    LdelemAnyRef = 292,
    LdelemAnyVal = 293,
    StelemI1 = 294,
    StelemI2 = 295,
    StelemI4 = 296,
    StelemI8 = 297,
    StelemI = 298,
    StelemR4 = 299,
    StelemR8 = 300,
    StelemRef = 301,
    StelemAnyRef = 302,
    StelemAnyVal = 303,
    MkRefAny = 304,
    RefAnyVal = 305,
    RefAnyType = 306,
    LdToken = 307,
    CkfiniteR4 = 308,
    CkfiniteR8 = 309,
    LocAlloc = 310,
    InitBlk = 311,
    CpBlk = 312,
    Ldftn = 313,
    Ldvirtftn = 314,
    LdfldI1 = 315,
    LdfldI1Large = 316,
    LdfldU1 = 317,
    LdfldU1Large = 318,
    LdfldI2 = 319,
    LdfldI2Large = 320,
    LdfldI2Unaligned = 321,
    LdfldU2 = 322,
    LdfldU2Large = 323,
    LdfldU2Unaligned = 324,
    LdfldI4 = 325,
    LdfldI4Large = 326,
    LdfldI4Unaligned = 327,
    LdfldI8 = 328,
    LdfldI8Large = 329,
    LdfldI8Unaligned = 330,
    LdfldAny = 331,
    LdfldAnyLarge = 332,
    LdvfldI1 = 333,
    LdvfldI1Large = 334,
    LdvfldU1 = 335,
    LdvfldU1Large = 336,
    LdvfldI2 = 337,
    LdvfldI2Large = 338,
    LdvfldI2Unaligned = 339,
    LdvfldU2 = 340,
    LdvfldU2Large = 341,
    LdvfldU2Unaligned = 342,
    LdvfldI4 = 343,
    LdvfldI4Large = 344,
    LdvfldI4Unaligned = 345,
    LdvfldI8 = 346,
    LdvfldI8Large = 347,
    LdvfldI8Unaligned = 348,
    LdvfldAny = 349,
    LdvfldAnyLarge = 350,
    Ldflda = 351,
    LdfldaLarge = 352,
    StfldI1 = 353,
    StfldI1Large = 354,
    StfldI2 = 355,
    StfldI2Large = 356,
    StfldI2Unaligned = 357,
    StfldI4 = 358,
    StfldI4Large = 359,
    StfldI4Unaligned = 360,
    StfldI8 = 361,
    StfldI8Large = 362,
    StfldI8Unaligned = 363,
    StfldAny = 364,
    StfldAnyLarge = 365,
    LdsfldI1 = 366,
    LdsfldU1 = 367,
    LdsfldI2 = 368,
    LdsfldU2 = 369,
    LdsfldI4 = 370,
    LdsfldI8 = 371,
    LdsfldAny = 372,
    Ldsflda = 373,
    LdsfldRvaData = 374,
    StsfldI1 = 375,
    StsfldI2 = 376,
    StsfldI4 = 377,
    StsfldI8 = 378,
    StsfldAny = 379,
    RetVoid = 380,
    RetI4 = 381,
    RetI8 = 382,
    RetAny = 383,
    RetNop = 384,
    CallInterp = 385,
    CallVirtInterp = 386,
    CallInternalCall = 387,
    CallIntrinsic = 388,
    CallPInvoke = 389,
    CallAot = 390,
    CallRuntimeImplemented = 391,
    CalliInterp = 392,
    BoxRefInplace = 393,
    NewObjInterp = 394,
    NewValueTypeInterp = 395,
    NewObjInternalCall = 396,
    NewObjIntrinsic = 397,
    NewObjAot = 398,
    NewValueTypeAot = 399,
    Throw = 400,
    Rethrow = 401,
    LeaveTryWithFinally = 402,
    LeaveCatchWithFinally = 403,
    LeaveCatchWithoutFinally = 404,
    EndFilter = 405,
    EndFinally = 406,
    EndFault = 407,
    GetEnumLongHashCode = 408,

    //}}LOW_LEVEL_OPCODE_ENUMM
    __Count,
};

//{{LOW_LEVEL_INSTRUCTION_STRUCTS
struct Illegal
{
    uint16_t __code;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct Nop
{
    uint16_t __code;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct ProfileAddCost
{
    uint16_t __code;
    uint8_t __padding_2;
    uint8_t __padding_3;
    uint32_t cost;
};

struct InitLocals
{
    uint16_t __code;
    uint16_t offset;
    uint16_t size;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct Arglist
{
    uint16_t __code;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct LdLocI1
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdLocU1
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdLocI2
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdLocU2
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdLocI4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdLocI8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdLocAny
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint16_t size;
};

struct LdLoca
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StLocI1
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StLocI2
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StLocI4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StLocI8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StLocAny
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint16_t size;
};

struct LdNull
{
    uint16_t __code;
    uint16_t dst;
};

struct LdcI4I2
{
    uint16_t __code;
    int16_t value;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdcI4I4
{
    uint16_t __code;
    uint16_t dst;
    int32_t value;
};

struct LdcI8I2
{
    uint16_t __code;
    int16_t value;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdcI8I4
{
    uint16_t __code;
    uint16_t dst;
    int32_t value;
};

struct LdcI8I8
{
    uint16_t __code;
    uint16_t dst;
    int32_t value_low;
    int32_t value_high;
};

struct LdStr
{
    uint16_t __code;
    uint16_t str_idx;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct Br
{
    uint16_t __code;
    uint8_t __padding_2;
    uint8_t __padding_3;
    int32_t target_offset;
};

struct BrTrueI4
{
    uint16_t __code;
    uint16_t condition;
    int32_t target_offset;
};

struct BrTrueI8
{
    uint16_t __code;
    uint16_t condition;
    int32_t target_offset;
};

struct BrFalseI4
{
    uint16_t __code;
    uint16_t condition;
    int32_t target_offset;
};

struct BrFalseI8
{
    uint16_t __code;
    uint16_t condition;
    int32_t target_offset;
};

struct BeqI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BeqI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BeqR4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BeqR8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BgeI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BgeI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BgeR4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BgeR8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BgtI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BgtI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BgtR4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BgtR8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BleI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BleI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BleR4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BleR8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BltI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BltI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BltR4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BltR8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BneUnI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BneUnI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BneUnR4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BneUnR8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BgeUnI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BgeUnI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BgeUnR4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BgeUnR8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BgtUnI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BgtUnI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BgtUnR4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BgtUnR8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BleUnI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BleUnI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BleUnR4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BleUnR8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BltUnI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BltUnI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BltUnR4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct BltUnR8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t __padding_6;
    uint8_t __padding_7;
    int32_t target_offset;
};

struct Switch
{
    uint16_t __code;
    uint16_t index;
    uint32_t num_targets;
};

struct LdIndI1
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdIndU1
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdIndI2
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdIndI2Unaligned
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdIndU2
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdIndU2Unaligned
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdIndI4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdIndI4Unaligned
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdIndI8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdIndI8Unaligned
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StIndI1
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StIndI2
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StIndI2Unaligned
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StIndI4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StIndI4Unaligned
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StIndI8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StIndI8Unaligned
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StIndI8I4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StIndI8I4Unaligned
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StIndI8U4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StIndI8U4Unaligned
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct AddI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct AddI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct AddR4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct AddR8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct SubI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct SubI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct SubR4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct SubR8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct MulI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct MulI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct MulR4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct MulR8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct DivI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct DivI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct DivR4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct DivR8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct DivUnI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct DivUnI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct RemI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct RemI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct RemR4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct RemR8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct RemUnI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct RemUnI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct AndI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct AndI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct OrI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct OrI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct XorI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct XorI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct ShlI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct ShlI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct ShrI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct ShrI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct ShrUnI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct ShrUnI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct NegI4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct NegI8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct NegR4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct NegR8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct NotI4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct NotI8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct AddOvfI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct AddOvfI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct AddOvfUnI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct AddOvfUnI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct MulOvfI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct MulOvfI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct MulOvfUnI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct MulOvfUnI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct SubOvfI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct SubOvfI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct SubOvfUnI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct SubOvfUnI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct ConvI1I4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvI1I8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvI1R4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvI1R8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvU1I4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvU1I8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvU1R4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvU1R8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvI2I4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvI2I8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvI2R4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvI2R8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvU2I4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvU2I8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvU2R4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvU2R8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvI4I8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvI4R4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvI4R8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvU4I8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvU4R4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvU4R8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvI8I4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvI8U4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvI8R4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvI8R8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvU8I4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvU8R4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvU8R8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvR4I4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvR4I8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvR4R8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvR8I4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvR8I8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvR8R4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI1I4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI1I8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI1R4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI1R8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU1I4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU1I8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU1R4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU1R8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI2I4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI2I8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI2R4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI2R8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU2I4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU2I8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU2R4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU2R8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI4I8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI4R4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI4R8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU4I4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU4I8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU4R4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU4R8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI8R4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI8R8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU8I4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU8I8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU8R4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU8R8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI1UnI4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI1UnI8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI1UnR4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI1UnR8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU1UnI4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU1UnI8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU1UnR4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU1UnR8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI2UnI4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI2UnI8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI2UnR4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI2UnR8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU2UnI4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU2UnI8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU2UnR4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU2UnR8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI4UnI4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI4UnI8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI4UnR4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI4UnR8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU4UnI8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU4UnR4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU4UnR8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI8UnI8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI8UnR4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfI8UnR8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU8UnR4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct ConvOvfU8UnR8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct CeqI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CeqI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CeqR4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CeqR8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CgtI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CgtI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CgtR4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CgtR8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CgtUnI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CgtUnI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CgtUnR4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CgtUnR8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CltI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CltI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CltR4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CltR8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CltUnI4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CltUnI8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CltUnR4
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct CltUnR8
{
    uint16_t __code;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t dst;
};

struct InitObjI1
{
    uint16_t __code;
    uint16_t addr;
};

struct InitObjI2
{
    uint16_t __code;
    uint16_t addr;
};

struct InitObjI2Unaligned
{
    uint16_t __code;
    uint16_t addr;
};

struct InitObjI4
{
    uint16_t __code;
    uint16_t addr;
};

struct InitObjI4Unaligned
{
    uint16_t __code;
    uint16_t addr;
};

struct InitObjI8
{
    uint16_t __code;
    uint16_t addr;
};

struct InitObjI8Unaligned
{
    uint16_t __code;
    uint16_t addr;
};

struct InitObjAny
{
    uint16_t __code;
    uint16_t addr;
    uint32_t size;
};

struct CpObjI1
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct CpObjI2
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct CpObjI4
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct CpObjI8
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct CpObjAny
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint16_t size;
};

struct LdObjAny
{
    uint16_t __code;
    uint16_t addr;
    uint16_t dst;
    uint16_t size;
};

struct StObjAny
{
    uint16_t __code;
    uint16_t src;
    uint16_t addr;
    uint16_t size;
};

struct CastClass
{
    uint16_t __code;
    uint16_t obj;
    uint16_t klass_idx;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct IsInst
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t klass_idx;
};

struct Box
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint16_t klass_idx;
};

struct Unbox
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t klass_idx;
};

struct UnboxAny
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t klass_idx;
};

struct NewArr
{
    uint16_t __code;
    uint16_t length;
    uint16_t dst;
    uint16_t arr_klass_idx;
};

struct LdLen
{
    uint16_t __code;
    uint16_t arr;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct Ldelema
{
    uint16_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t dst;
    uint16_t ele_klass_idx;
    uint8_t __padding_10;
    uint8_t __padding_11;
};

struct LdelemaReadOnly
{
    uint16_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t dst;
};

struct LdelemI1
{
    uint16_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t dst;
};

struct LdelemU1
{
    uint16_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t dst;
};

struct LdelemI2
{
    uint16_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t dst;
};

struct LdelemU2
{
    uint16_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t dst;
};

struct LdelemI4
{
    uint16_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t dst;
};

struct LdelemI8
{
    uint16_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t dst;
};

struct LdelemI
{
    uint16_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t dst;
};

struct LdelemR4
{
    uint16_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t dst;
};

struct LdelemR8
{
    uint16_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t dst;
};

struct LdelemRef
{
    uint16_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t dst;
};

struct LdelemAnyRef
{
    uint16_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t dst;
    uint16_t ele_klass_idx;
    uint8_t __padding_10;
    uint8_t __padding_11;
};

struct LdelemAnyVal
{
    uint16_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t dst;
    uint16_t ele_klass_idx;
    uint8_t __padding_10;
    uint8_t __padding_11;
    uint32_t ele_size;
};

struct StelemI1
{
    uint16_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t value;
};

struct StelemI2
{
    uint16_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t value;
};

struct StelemI4
{
    uint16_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t value;
};

struct StelemI8
{
    uint16_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t value;
};

struct StelemI
{
    uint16_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t value;
};

struct StelemR4
{
    uint16_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t value;
};

struct StelemR8
{
    uint16_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t value;
};

struct StelemRef
{
    uint16_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t value;
};

struct StelemAnyRef
{
    uint16_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t value;
    uint16_t ele_klass_idx;
    uint8_t __padding_10;
    uint8_t __padding_11;
};

struct StelemAnyVal
{
    uint16_t __code;
    uint16_t arr;
    uint16_t index;
    uint16_t value;
    uint16_t ele_klass_idx;
    uint8_t __padding_10;
    uint8_t __padding_11;
    uint32_t ele_size;
};

struct MkRefAny
{
    uint16_t __code;
    uint16_t addr;
    uint16_t dst;
    uint16_t klass_idx;
};

struct RefAnyVal
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint16_t klass_idx;
};

struct RefAnyType
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdToken
{
    uint16_t __code;
    uint16_t dst;
    uint16_t token_idx;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct CkfiniteR4
{
    uint16_t __code;
    uint16_t src;
};

struct CkfiniteR8
{
    uint16_t __code;
    uint16_t src;
};

struct LocAlloc
{
    uint16_t __code;
    uint16_t size;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct InitBlk
{
    uint16_t __code;
    uint16_t addr;
    uint16_t value;
    uint16_t size;
};

struct CpBlk
{
    uint16_t __code;
    uint16_t dst;
    uint16_t src;
    uint16_t size;
};

struct Ldftn
{
    uint16_t __code;
    uint16_t dst;
    uint16_t method_idx;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct Ldvirtftn
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t method_idx;
};

struct LdfldI1
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdfldI1Large
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct LdfldU1
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdfldU1Large
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct LdfldI2
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdfldI2Large
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct LdfldI2Unaligned
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdfldU2
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdfldU2Large
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct LdfldU2Unaligned
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdfldI4
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdfldI4Large
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct LdfldI4Unaligned
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdfldI8
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdfldI8Large
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct LdfldI8Unaligned
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdfldAny
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
    uint16_t size;
    uint8_t __padding_10;
    uint8_t __padding_11;
};

struct LdfldAnyLarge
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
    uint32_t size;
};

struct LdvfldI1
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdvfldI1Large
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct LdvfldU1
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdvfldU1Large
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct LdvfldI2
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdvfldI2Large
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct LdvfldI2Unaligned
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdvfldU2
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdvfldU2Large
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct LdvfldU2Unaligned
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdvfldI4
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdvfldI4Large
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct LdvfldI4Unaligned
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdvfldI8
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdvfldI8Large
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct LdvfldI8Unaligned
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdvfldAny
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
    uint16_t size;
    uint8_t __padding_10;
    uint8_t __padding_11;
};

struct LdvfldAnyLarge
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
    uint32_t size;
};

struct Ldflda
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint16_t offset;
};

struct LdfldaLarge
{
    uint16_t __code;
    uint16_t obj;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct StfldI1
{
    uint16_t __code;
    uint16_t obj;
    uint16_t value;
    uint16_t offset;
};

struct StfldI1Large
{
    uint16_t __code;
    uint16_t obj;
    uint16_t value;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct StfldI2
{
    uint16_t __code;
    uint16_t obj;
    uint16_t value;
    uint16_t offset;
};

struct StfldI2Large
{
    uint16_t __code;
    uint16_t obj;
    uint16_t value;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct StfldI2Unaligned
{
    uint16_t __code;
    uint16_t obj;
    uint16_t value;
    uint16_t offset;
};

struct StfldI4
{
    uint16_t __code;
    uint16_t obj;
    uint16_t value;
    uint16_t offset;
};

struct StfldI4Large
{
    uint16_t __code;
    uint16_t obj;
    uint16_t value;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct StfldI4Unaligned
{
    uint16_t __code;
    uint16_t obj;
    uint16_t value;
    uint16_t offset;
};

struct StfldI8
{
    uint16_t __code;
    uint16_t obj;
    uint16_t value;
    uint16_t offset;
};

struct StfldI8Large
{
    uint16_t __code;
    uint16_t obj;
    uint16_t value;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
};

struct StfldI8Unaligned
{
    uint16_t __code;
    uint16_t obj;
    uint16_t value;
    uint16_t offset;
};

struct StfldAny
{
    uint16_t __code;
    uint16_t obj;
    uint16_t value;
    uint16_t offset;
    uint16_t size;
    uint8_t __padding_10;
    uint8_t __padding_11;
};

struct StfldAnyLarge
{
    uint16_t __code;
    uint16_t obj;
    uint16_t value;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t offset;
    uint32_t size;
};

struct LdsfldI1
{
    uint16_t __code;
    uint16_t dst;
    uint16_t field_idx;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdsfldU1
{
    uint16_t __code;
    uint16_t dst;
    uint16_t field_idx;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdsfldI2
{
    uint16_t __code;
    uint16_t dst;
    uint16_t field_idx;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdsfldU2
{
    uint16_t __code;
    uint16_t dst;
    uint16_t field_idx;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdsfldI4
{
    uint16_t __code;
    uint16_t dst;
    uint16_t field_idx;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdsfldI8
{
    uint16_t __code;
    uint16_t dst;
    uint16_t field_idx;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdsfldAny
{
    uint16_t __code;
    uint16_t field_idx;
    uint16_t size;
    uint16_t dst;
};

struct Ldsflda
{
    uint16_t __code;
    uint16_t field_idx;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct LdsfldRvaData
{
    uint16_t __code;
    uint16_t data;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StsfldI1
{
    uint16_t __code;
    uint16_t field_idx;
    uint16_t value;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StsfldI2
{
    uint16_t __code;
    uint16_t field_idx;
    uint16_t value;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StsfldI4
{
    uint16_t __code;
    uint16_t field_idx;
    uint16_t value;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StsfldI8
{
    uint16_t __code;
    uint16_t field_idx;
    uint16_t value;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct StsfldAny
{
    uint16_t __code;
    uint16_t field_idx;
    uint16_t size;
    uint16_t value;
};

struct RetVoid
{
    uint16_t __code;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct RetI4
{
    uint16_t __code;
    uint16_t src;
};

struct RetI8
{
    uint16_t __code;
    uint16_t src;
};

struct RetAny
{
    uint16_t __code;
    uint16_t src;
    uint16_t size;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct RetNop
{
    uint16_t __code;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct CallInterp
{
    uint16_t __code;
    uint16_t method_idx;
    uint16_t frame_base;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct CallVirtInterp
{
    uint16_t __code;
    uint16_t method_idx;
    uint16_t frame_base;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct CallInternalCall
{
    uint16_t __code;
    uint16_t method_idx;
    uint16_t frame_base;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct CallIntrinsic
{
    uint16_t __code;
    uint16_t method_idx;
    uint16_t frame_base;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct CallPInvoke
{
    uint16_t __code;
    uint16_t method_idx;
    uint16_t frame_base;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct CallAot
{
    uint16_t __code;
    uint16_t method_idx;
    uint16_t frame_base;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct CallRuntimeImplemented
{
    uint16_t __code;
    uint16_t method_idx;
    uint16_t frame_base;
    uint8_t __padding_6;
    uint8_t __padding_7;
};

struct CalliInterp
{
    uint16_t __code;
    uint16_t method_sig_idx;
    uint16_t method_idx;
    uint16_t frame_base;
};

struct BoxRefInplace
{
    uint16_t __code;
    uint16_t src;
    uint16_t dst;
    uint16_t klass_idx;
};

struct NewObjInterp
{
    uint16_t __code;
    uint16_t method_idx;
    uint16_t frame_base;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t total_params_stack_object_size;
};

struct NewValueTypeInterp
{
    uint16_t __code;
    uint16_t method_idx;
    uint16_t frame_base;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t total_params_stack_object_size;
};

struct NewObjInternalCall
{
    uint16_t __code;
    uint16_t method_idx;
    uint16_t invoker_idx;
    uint16_t frame_base;
};

struct NewObjIntrinsic
{
    uint16_t __code;
    uint16_t method_idx;
    uint16_t invoker_idx;
    uint16_t frame_base;
};

struct NewObjAot
{
    uint16_t __code;
    uint16_t method_idx;
    uint16_t frame_base;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t total_params_stack_object_size;
};

struct NewValueTypeAot
{
    uint16_t __code;
    uint16_t method_idx;
    uint16_t frame_base;
    uint8_t __padding_6;
    uint8_t __padding_7;
    uint32_t total_params_stack_object_size;
};

struct Throw
{
    uint16_t __code;
    uint16_t ex;
};

struct Rethrow
{
    uint16_t __code;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct LeaveTryWithFinally
{
    uint16_t __code;
    uint8_t first_finally_clause_index;
    uint8_t finally_clauses_count;
    int32_t target_offset;
};

struct LeaveCatchWithFinally
{
    uint16_t __code;
    uint8_t first_finally_clause_index;
    uint8_t finally_clauses_count;
    int32_t target_offset;
};

struct LeaveCatchWithoutFinally
{
    uint16_t __code;
    uint8_t __padding_2;
    uint8_t __padding_3;
    int32_t target_offset;
};

struct EndFilter
{
    uint16_t __code;
    uint16_t cond;
};

struct EndFinally
{
    uint16_t __code;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct EndFault
{
    uint16_t __code;
    uint8_t __padding_2;
    uint8_t __padding_3;
};

struct GetEnumLongHashCode
{
    uint16_t __code;
    uint16_t value_ptr;
    uint16_t dst;
    uint8_t __padding_6;
    uint8_t __padding_7;
};


//}}LOW_LEVEL_INSTRUCTION_STRUCTS


class OpCodes
{
  public:
    static size_t get_instruction_size(OpCodeEnum opcode)
    {
        return s_opsizes[static_cast<size_t>(opcode)];
    }
    static uint8_t* write_instruction_to_data(uint8_t* codes, OpCodeEnum opcode, const InstructionOperands& operands);
    static size_t get_branch_patch_offset(OpCodeEnum opcode);
    static bool validate_operands(OpCodeEnum opcode, const InstructionOperands& operands);

  private:
    static size_t s_opsizes[static_cast<size_t>(OpCodeEnum::__Count)];
};

} // namespace ll
} // namespace interp
} // namespace leanclr
