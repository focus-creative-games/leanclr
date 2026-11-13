// Hand-written opcode handlers, included inside Interpreter::execute.
// Edit this file directly; it is not generated.

LEANCLR_CASE_BEGIN(InitLocals)
{
    std::memset(eval_stack_base + ir->offset, 0, ir->size * sizeof(RtStackObject));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdLocI1)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src = eval_stack_base + ir->src;
    dst->i32 = src->i8;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdLocU1)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src = eval_stack_base + ir->src;
    dst->i32 = src->u8;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdLocI2)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src = eval_stack_base + ir->src;
    dst->i32 = src->i16;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdLocU2)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src = eval_stack_base + ir->src;
    dst->i32 = src->u16;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdLocI4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src = eval_stack_base + ir->src;
    dst->i32 = src->i32;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdLocI8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src = eval_stack_base + ir->src;
    dst->i64 = src->i64;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdLocAny)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src = eval_stack_base + ir->src;
    std::memcpy(dst, src, ir->size * sizeof(RtStackObject));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdLoca)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src = eval_stack_base + ir->src;
    dst->ptr = src;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StLocI1)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src = eval_stack_base + ir->src;
    dst->i8 = src->i8;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StLocI2)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src = eval_stack_base + ir->src;
    dst->i16 = src->i16;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StLocI4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src = eval_stack_base + ir->src;
    dst->i32 = src->i32;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StLocI8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src = eval_stack_base + ir->src;
    dst->i64 = src->i64;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StLocAny)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src = eval_stack_base + ir->src;
    std::memcpy(dst, src, ir->size * sizeof(RtStackObject));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdNull)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    dst->ptr = nullptr;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdcI4I2)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    dst->i32 = static_cast<int32_t>(ir->value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdcI4I4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    dst->i32 = ir->value;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdcI8I2)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    dst->i64 = static_cast<int64_t>(ir->value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdcI8I4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    dst->i64 = static_cast<int64_t>(ir->value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdcI8I8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    dst->i64 = utils::IntegerMath::from_unsigned<int64_t>(static_cast<uint64_t>(static_cast<uint32_t>(ir->value_low)) |
                                                          (static_cast<uint64_t>(static_cast<uint32_t>(ir->value_high)) << 32));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdStr)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    vm::RtString* str = get_resolved_data<vm::RtString>(imi, ir->str_idx);
    dst->str = str;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN_LITE(Br)
{
    const auto* ir = (ll::Br*)ip;
    ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BrTrueI4)
{
    const auto* ir = (ll::BrTrueI4*)ip;
    RtStackObject* cond = eval_stack_base + ir->condition;
    if (cond->i32 != 0)
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BrTrueI8)
{
    const auto* ir = (ll::BrTrueI8*)ip;
    RtStackObject* cond = eval_stack_base + ir->condition;
    if (cond->i64 != 0)
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BrFalseI4)
{
    const auto* ir = (ll::BrFalseI4*)ip;
    RtStackObject* cond = eval_stack_base + ir->condition;
    if (cond->i32 == 0)
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BrFalseI8)
{
    const auto* ir = (ll::BrFalseI8*)ip;
    RtStackObject* cond = eval_stack_base + ir->condition;
    if (cond->i64 == 0)
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BeqI4)
{
    const auto* ir = (ll::BeqI4*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    if (op1->i32 == op2->i32)
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BeqI8)
{
    const auto* ir = (ll::BeqI8*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    if (op1->i64 == op2->i64)
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BeqR4)
{
    const auto* ir = (ll::BeqR4*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    float left = op1->f32;
    float right = op2->f32;
    if (left == right && !std::isunordered(left, right))
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BeqR8)
{
    const auto* ir = (ll::BeqR8*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    double left = op1->f64;
    double right = op2->f64;
    if (left == right && !std::isunordered(left, right))
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BgeI4)
{
    const auto* ir = (ll::BgeI4*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    if (op1->i32 >= op2->i32)
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BgeI8)
{
    const auto* ir = (ll::BgeI8*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    if (op1->i64 >= op2->i64)
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BgeR4)
{
    const auto* ir = (ll::BgeR4*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    float left = op1->f32;
    float right = op2->f32;
    if (left >= right && !std::isunordered(left, right))
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BgeR8)
{
    const auto* ir = (ll::BgeR8*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    double left = op1->f64;
    double right = op2->f64;
    if (left >= right && !std::isunordered(left, right))
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BgtI4)
{
    const auto* ir = (ll::BgtI4*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    if (op1->i32 > op2->i32)
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BgtI8)
{
    const auto* ir = (ll::BgtI8*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    if (op1->i64 > op2->i64)
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BgtR4)
{
    const auto* ir = (ll::BgtR4*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    float left = op1->f32;
    float right = op2->f32;
    if (left > right && !std::isunordered(left, right))
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BgtR8)
{
    const auto* ir = (ll::BgtR8*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    double left = op1->f64;
    double right = op2->f64;
    if (left > right && !std::isunordered(left, right))
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BleI4)
{
    const auto* ir = (ll::BleI4*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    if (op1->i32 <= op2->i32)
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BleI8)
{
    const auto* ir = (ll::BleI8*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    if (op1->i64 <= op2->i64)
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BleR4)
{
    const auto* ir = (ll::BleR4*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    float left = op1->f32;
    float right = op2->f32;
    if (left <= right && !std::isunordered(left, right))
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BleR8)
{
    const auto* ir = (ll::BleR8*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    double left = op1->f64;
    double right = op2->f64;
    if (left <= right && !std::isunordered(left, right))
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BltI4)
{
    const auto* ir = (ll::BltI4*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    if (op1->i32 < op2->i32)
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BltI8)
{
    const auto* ir = (ll::BltI8*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    if (op1->i64 < op2->i64)
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BltR4)
{
    const auto* ir = (ll::BltR4*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    float left = op1->f32;
    float right = op2->f32;
    if (left < right && !std::isunordered(left, right))
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BltR8)
{
    const auto* ir = (ll::BltR8*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    double left = op1->f64;
    double right = op2->f64;
    if (left < right && !std::isunordered(left, right))
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BneUnI4)
{
    const auto* ir = (ll::BneUnI4*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    if (op1->u32 != op2->u32)
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BneUnI8)
{
    const auto* ir = (ll::BneUnI8*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    if (op1->u64 != op2->u64)
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BneUnR4)
{
    const auto* ir = (ll::BneUnR4*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    float left = op1->f32;
    float right = op2->f32;
    if (left != right || std::isunordered(left, right))
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BneUnR8)
{
    const auto* ir = (ll::BneUnR8*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    double left = op1->f64;
    double right = op2->f64;
    if (left != right || std::isunordered(left, right))
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BgeUnI4)
{
    const auto* ir = (ll::BgeUnI4*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    if (op1->u32 >= op2->u32)
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BgeUnI8)
{
    const auto* ir = (ll::BgeUnI8*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    if (op1->u64 >= op2->u64)
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BgeUnR4)
{
    const auto* ir = (ll::BgeUnR4*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    float left = op1->f32;
    float right = op2->f32;
    if (left >= right || std::isunordered(left, right))
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BgeUnR8)
{
    const auto* ir = (ll::BgeUnR8*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    double left = op1->f64;
    double right = op2->f64;
    if (left >= right || std::isunordered(left, right))
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BgtUnI4)
{
    const auto* ir = (ll::BgtUnI4*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    if (op1->u32 > op2->u32)
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BgtUnI8)
{
    const auto* ir = (ll::BgtUnI8*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    if (op1->u64 > op2->u64)
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BgtUnR4)
{
    const auto* ir = (ll::BgtUnR4*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    float left = op1->f32;
    float right = op2->f32;
    if (left > right || std::isunordered(left, right))
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BgtUnR8)
{
    const auto* ir = (ll::BgtUnR8*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    double left = op1->f64;
    double right = op2->f64;
    if (left > right || std::isunordered(left, right))
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BleUnI4)
{
    const auto* ir = (ll::BleUnI4*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    if (op1->u32 <= op2->u32)
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BleUnI8)
{
    const auto* ir = (ll::BleUnI8*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    if (op1->u64 <= op2->u64)
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BleUnR4)
{
    const auto* ir = (ll::BleUnR4*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    float left = op1->f32;
    float right = op2->f32;
    if (left <= right || std::isunordered(left, right))
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BleUnR8)
{
    const auto* ir = (ll::BleUnR8*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    double left = op1->f64;
    double right = op2->f64;
    if (left <= right || std::isunordered(left, right))
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BltUnI4)
{
    const auto* ir = (ll::BltUnI4*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    if (op1->u32 < op2->u32)
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BltUnI8)
{
    const auto* ir = (ll::BltUnI8*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    if (op1->u64 < op2->u64)
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BltUnR4)
{
    const auto* ir = (ll::BltUnR4*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    float left = op1->f32;
    float right = op2->f32;
    if (left < right || std::isunordered(left, right))
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(BltUnR8)
{
    const auto* ir = (ll::BltUnR8*)ip;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    double left = op1->f64;
    double right = op2->f64;
    if (left < right || std::isunordered(left, right))
    {
        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(Switch)
{
    const auto* ir = (ll::Switch*)ip;
    RtStackObject* index_obj = eval_stack_base + ir->index;
    uint32_t idx = index_obj->u32;
    if (idx < ir->num_targets)
    {
        const int32_t* target_offsets = reinterpret_cast<const int32_t*>(ir + 1);
        int32_t target_offset = target_offsets[idx];
        ip = reinterpret_cast<const uint8_t*>(ip + target_offset);
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(reinterpret_cast<const uint8_t*>(ir + 1) + ir->num_targets * sizeof(int32_t));
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN(AddI4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    dst->i32 = utils::IntegerMath::add<int32_t>(src1->i32, src2->i32);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(AddI8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    dst->i64 = utils::IntegerMath::add<int64_t>(src1->i64, src2->i64);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(AddR4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    dst->f32 = src1->f32 + src2->f32;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(AddR8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    dst->f64 = src1->f64 + src2->f64;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(SubI4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    dst->i32 = utils::IntegerMath::sub<int32_t>(src1->i32, src2->i32);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(SubI8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    dst->i64 = utils::IntegerMath::sub<int64_t>(src1->i64, src2->i64);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(SubR4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    dst->f32 = src1->f32 - src2->f32;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(SubR8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    dst->f64 = src1->f64 - src2->f64;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(MulI4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    dst->i32 = utils::IntegerMath::mul<int32_t>(src1->i32, src2->i32);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(MulI8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    dst->i64 = utils::IntegerMath::mul<int64_t>(src1->i64, src2->i64);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(MulR4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    dst->f32 = src1->f32 * src2->f32;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(MulR8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    dst->f64 = src1->f64 * src2->f64;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(DivI4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    int32_t dividend = src1->i32;
    int32_t divisor = src2->i32;
    if (divisor == 0)
    {
        RAISE_RUNTIME_ERROR(RtErr::DivideByZero);
    }
    if (divisor == -1 && dividend == INT32_MIN)
    {
        RAISE_RUNTIME_ERROR(RtErr::Arithmetic);
    }
    dst->i32 = dividend / divisor;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(DivI8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    int64_t dividend = src1->i64;
    int64_t divisor = src2->i64;
    if (divisor == 0)
    {
        RAISE_RUNTIME_ERROR(RtErr::DivideByZero);
    }
    if (divisor == -1 && dividend == INT64_MIN)
    {
        RAISE_RUNTIME_ERROR(RtErr::Arithmetic);
    }
    dst->i64 = dividend / divisor;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(DivR4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    dst->f32 = src1->f32 / src2->f32;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(DivR8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    dst->f64 = src1->f64 / src2->f64;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(DivUnI4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    uint32_t dividend = src1->u32;
    uint32_t divisor = src2->u32;
    if (divisor == 0)
    {
        RAISE_RUNTIME_ERROR(RtErr::DivideByZero);
    }
    dst->u32 = dividend / divisor;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(DivUnI8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    uint64_t dividend = src1->u64;
    uint64_t divisor = src2->u64;
    if (divisor == 0)
    {
        RAISE_RUNTIME_ERROR(RtErr::DivideByZero);
    }
    dst->u64 = dividend / divisor;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(RemI4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    int32_t dividend = src1->i32;
    int32_t divisor = src2->i32;
    if (divisor == 0)
    {
        RAISE_RUNTIME_ERROR(RtErr::DivideByZero);
    }
    if (divisor == -1 && dividend == INT32_MIN)
    {
        RAISE_RUNTIME_ERROR(RtErr::Arithmetic);
    }
    else
    {
        dst->i32 = dividend % divisor;
    }
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(RemI8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    int64_t dividend = src1->i64;
    int64_t divisor = src2->i64;
    if (divisor == 0)
    {
        RAISE_RUNTIME_ERROR(RtErr::DivideByZero);
    }
    if (divisor == -1 && dividend == INT64_MIN)
    {
        RAISE_RUNTIME_ERROR(RtErr::Arithmetic);
    }
    else
    {
        dst->i64 = dividend % divisor;
    }
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(RemR4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    dst->f32 = std::fmod(src1->f32, src2->f32);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(RemR8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    dst->f64 = std::fmod(src1->f64, src2->f64);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(RemUnI4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    uint32_t dividend = src1->u32;
    uint32_t divisor = src2->u32;
    if (divisor == 0)
    {
        RAISE_RUNTIME_ERROR(RtErr::DivideByZero);
    }
    dst->u32 = dividend % divisor;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(RemUnI8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    uint64_t dividend = src1->u64;
    uint64_t divisor = src2->u64;
    if (divisor == 0)
    {
        RAISE_RUNTIME_ERROR(RtErr::DivideByZero);
    }
    dst->u64 = dividend % divisor;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(AndI4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    dst->i32 = src1->i32 & src2->i32;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(AndI8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    dst->i64 = src1->i64 & src2->i64;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(OrI4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    dst->i32 = src1->i32 | src2->i32;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(OrI8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    dst->i64 = src1->i64 | src2->i64;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(XorI4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    dst->i32 = src1->i32 ^ src2->i32;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(XorI8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    dst->i64 = src1->i64 ^ src2->i64;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ShlI4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    dst->i32 = utils::IntegerMath::shl<int32_t>(src1->i32, static_cast<uint32_t>(src2->i32));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ShlI8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    dst->i64 = utils::IntegerMath::shl<int64_t>(src1->i64, static_cast<uint32_t>(src2->i32));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ShrI4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    dst->i32 = utils::IntegerMath::shr<int32_t>(src1->i32, static_cast<uint32_t>(src2->i32));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ShrI8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    dst->i64 = utils::IntegerMath::shr<int64_t>(src1->i64, static_cast<uint32_t>(src2->i32));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ShrUnI4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    dst->u32 = utils::IntegerMath::shr_un<uint32_t>(src1->u32, static_cast<uint32_t>(src2->i32));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ShrUnI8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src1 = eval_stack_base + ir->arg1;
    RtStackObject* src2 = eval_stack_base + ir->arg2;
    dst->u64 = utils::IntegerMath::shr_un<uint64_t>(src1->u64, static_cast<uint32_t>(src2->i32));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(NegI4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src = eval_stack_base + ir->src;
    dst->i32 = utils::IntegerMath::neg<int32_t>(src->i32);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(NegI8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src = eval_stack_base + ir->src;
    dst->i64 = utils::IntegerMath::neg<int64_t>(src->i64);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(NegR4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src = eval_stack_base + ir->src;
    dst->f32 = -src->f32;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(NegR8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src = eval_stack_base + ir->src;
    dst->f64 = -src->f64;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(NotI4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src = eval_stack_base + ir->src;
    dst->i32 = ~src->i32;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(NotI8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* src = eval_stack_base + ir->src;
    dst->i64 = ~src->i64;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(CeqI4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    dst->i32 = (op1->i32 == op2->i32) ? 1 : 0;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(CeqI8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    dst->i32 = (op1->i64 == op2->i64) ? 1 : 0;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(CeqR4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    float left = op1->f32;
    float right = op2->f32;
    dst->i32 = (left == right && !std::isunordered(left, right)) ? 1 : 0;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(CeqR8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    double left = op1->f64;
    double right = op2->f64;
    dst->i32 = (left == right && !std::isunordered(left, right)) ? 1 : 0;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(CgtI4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    dst->i32 = (op1->i32 > op2->i32) ? 1 : 0;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(CgtI8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    dst->i32 = (op1->i64 > op2->i64) ? 1 : 0;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(CgtR4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    float left = op1->f32;
    float right = op2->f32;
    dst->i32 = (left > right && !std::isunordered(left, right)) ? 1 : 0;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(CgtR8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    double left = op1->f64;
    double right = op2->f64;
    dst->i32 = (left > right && !std::isunordered(left, right)) ? 1 : 0;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(CgtUnI4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    dst->i32 = (op1->u32 > op2->u32) ? 1 : 0;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(CgtUnI8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    dst->i32 = (op1->u64 > op2->u64) ? 1 : 0;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(CgtUnR4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    float left = op1->f32;
    float right = op2->f32;
    dst->i32 = (left > right || std::isunordered(left, right)) ? 1 : 0;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(CgtUnR8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    double left = op1->f64;
    double right = op2->f64;
    dst->i32 = (left > right || std::isunordered(left, right)) ? 1 : 0;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(CltI4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    dst->i32 = (op1->i32 < op2->i32) ? 1 : 0;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(CltI8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    dst->i32 = (op1->i64 < op2->i64) ? 1 : 0;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(CltR4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    float left = op1->f32;
    float right = op2->f32;
    dst->i32 = (left < right && !std::isunordered(left, right)) ? 1 : 0;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(CltR8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    double left = op1->f64;
    double right = op2->f64;
    dst->i32 = (left < right && !std::isunordered(left, right)) ? 1 : 0;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(CltUnI4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    dst->i32 = (op1->u32 < op2->u32) ? 1 : 0;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(CltUnI8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    dst->i32 = (op1->u64 < op2->u64) ? 1 : 0;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(CltUnR4)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    float left = op1->f32;
    float right = op2->f32;
    dst->i32 = (left < right || std::isunordered(left, right)) ? 1 : 0;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(CltUnR8)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* op1 = eval_stack_base + ir->arg1;
    RtStackObject* op2 = eval_stack_base + ir->arg2;
    double left = op1->f64;
    double right = op2->f64;
    dst->i32 = (left < right || std::isunordered(left, right)) ? 1 : 0;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(InitObjI1)
{
    RtStackObject* addr_obj = eval_stack_base + ir->addr;
    int8_t* addr = reinterpret_cast<int8_t*>(addr_obj->ptr);
    utils::MemOp::store_native(addr, 0);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(InitObjI2)
{
    RtStackObject* addr_obj = eval_stack_base + ir->addr;
    int16_t* addr = reinterpret_cast<int16_t*>(addr_obj->ptr);
    utils::MemOp::store_native(addr, 0);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(InitObjI4)
{
    RtStackObject* addr_obj = eval_stack_base + ir->addr;
    int32_t* addr = reinterpret_cast<int32_t*>(addr_obj->ptr);
    utils::MemOp::store_native(addr, 0);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(InitObjI8)
{
    RtStackObject* addr_obj = eval_stack_base + ir->addr;
    int64_t* addr = reinterpret_cast<int64_t*>(addr_obj->ptr);
    utils::MemOp::store_native(addr, 0);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(InitObjAny)
{
    RtStackObject* addr_obj = eval_stack_base + ir->addr;
    uint8_t* addr = reinterpret_cast<uint8_t*>(addr_obj->ptr);
    std::memset(addr, 0, ir->size);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(CpObjI1)
{
    RtStackObject* dst_obj = eval_stack_base + ir->dst;
    RtStackObject* src_obj = eval_stack_base + ir->src;
    int8_t* dst_addr = reinterpret_cast<int8_t*>(dst_obj->ptr);
    const int8_t* src_addr = reinterpret_cast<const int8_t*>(src_obj->ptr);
    utils::MemOp::store_native(dst_addr, utils::MemOp::load_native(src_addr));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(CpObjI2)
{
    RtStackObject* dst_obj = eval_stack_base + ir->dst;
    RtStackObject* src_obj = eval_stack_base + ir->src;
    int16_t* dst_addr = reinterpret_cast<int16_t*>(dst_obj->ptr);
    const int16_t* src_addr = reinterpret_cast<const int16_t*>(src_obj->ptr);
    utils::MemOp::store_native(dst_addr, utils::MemOp::load_native(src_addr));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(CpObjI4)
{
    RtStackObject* dst_obj = eval_stack_base + ir->dst;
    RtStackObject* src_obj = eval_stack_base + ir->src;
    int32_t* dst_addr = reinterpret_cast<int32_t*>(dst_obj->ptr);
    const int32_t* src_addr = reinterpret_cast<const int32_t*>(src_obj->ptr);
    utils::MemOp::store_native(dst_addr, utils::MemOp::load_native(src_addr));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(CpObjI8)
{
    RtStackObject* dst_obj = eval_stack_base + ir->dst;
    RtStackObject* src_obj = eval_stack_base + ir->src;
    int64_t* dst_addr = reinterpret_cast<int64_t*>(dst_obj->ptr);
    const int64_t* src_addr = reinterpret_cast<const int64_t*>(src_obj->ptr);
    utils::MemOp::store_native(dst_addr, utils::MemOp::load_native(src_addr));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(CpObjAny)
{
    RtStackObject* dst_obj = eval_stack_base + ir->dst;
    RtStackObject* src_obj = eval_stack_base + ir->src;
    std::memcpy(dst_obj->ptr, src_obj->cptr, ir->size);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdObjAny)
{
    RtStackObject* dst = eval_stack_base + ir->dst;
    RtStackObject* addr_obj = eval_stack_base + ir->addr;
    std::memcpy(dst, addr_obj->cptr, ir->size);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StObjAny)
{
    RtStackObject* addr_obj = eval_stack_base + ir->addr;
    RtStackObject* src = eval_stack_base + ir->src;
    std::memcpy(addr_obj->ptr, src, ir->size);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(CastClass)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (obj)
    {
        metadata::RtClass* to_class = get_resolved_data<metadata::RtClass>(imi, ir->klass_idx);
        if (!vm::Class::is_assignable_from(obj->klass, to_class))
        {
            RAISE_RUNTIME_ERROR(RtErr::InvalidCast);
        }
    }
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(IsInst)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    vm::RtObject* result;
    if (obj)
    {
        metadata::RtClass* to_class = get_resolved_data<metadata::RtClass>(imi, ir->klass_idx);
        result = vm::Class::is_assignable_from(obj->klass, to_class) ? obj : nullptr;
    }
    else
    {
        result = nullptr;
    }
    set_stack_value_at<vm::RtObject*>(eval_stack_base, ir->dst, result);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(Box)
{
    RtStackObject* src = eval_stack_base + ir->src;
    metadata::RtClass* to_class = get_resolved_data<metadata::RtClass>(imi, ir->klass_idx);
    HANDLE_RAISE_RUNTIME_ERROR(vm::RtObject*, boxed_obj, LEANCLR_BOX_OBJECT_INTERP(to_class, src));
    set_stack_value_at<vm::RtObject*>(eval_stack_base, ir->dst, boxed_obj);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(Unbox)
{
    vm::RtObject* boxed_obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!boxed_obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    metadata::RtClass* to_class = get_resolved_data<metadata::RtClass>(imi, ir->klass_idx);
    HANDLE_RAISE_RUNTIME_ERROR(const void*, unbox_addr, vm::Object::unbox_ex(boxed_obj, to_class));
    set_stack_value_at(eval_stack_base, ir->dst, unbox_addr);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(UnboxAny)
{
    vm::RtObject* boxed_obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    metadata::RtClass* to_class = get_resolved_data<metadata::RtClass>(imi, ir->klass_idx);
    RtStackObject* dst = eval_stack_base + ir->dst;
    HANDLE_RAISE_RUNTIME_ERROR_VOID(vm::Object::unbox_any(boxed_obj, to_class, dst, true));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(NewArr)
{
    int32_t length = get_stack_value_at<int32_t>(eval_stack_base, ir->length);
    metadata::RtClass* element_class = get_resolved_data<metadata::RtClass>(imi, ir->arr_klass_idx);
    HANDLE_RAISE_RUNTIME_ERROR(vm::RtArray*, new_array, LEANCLR_NEW_SZARRAY_FROM_ARRAY_KLASS_INTERP(element_class, length));
    set_stack_value_at<vm::RtArray*>(eval_stack_base, ir->dst, new_array);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdLen)
{
    vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
    if (!array)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    set_stack_value_at(eval_stack_base, ir->dst, vm::Array::get_array_length(array));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(Ldelema)
{
    vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
    if (!array)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
    if (vm::Array::is_out_of_range(array, index))
    {
        RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
    }
    const metadata::RtClass* arr_klass = array->klass;
    const metadata::RtClass* element_klass = vm::Class::get_array_element_class(arr_klass);
    const metadata::RtClass* check_klass = get_resolved_data<metadata::RtClass>(imi, ir->ele_klass_idx);
    if (!vm::Class::is_pointer_element_compatible_with(element_klass, check_klass))
    {
        RAISE_RUNTIME_ERROR(RtErr::ArrayTypeMismatch);
    }
    const void* element_addr = vm::Array::get_array_element_address_as_ptr_void(array, index);
    set_stack_value_at(eval_stack_base, ir->dst, element_addr);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdelemI1)
{
    vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
    if (!array)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    assert(vm::Array::get_array_element_size(array) == 1);
    int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
    if (vm::Array::is_out_of_range(array, index))
    {
        RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
    }
    int8_t value = vm::Array::get_array_data_at<int8_t>(array, index);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdelemU1)
{
    vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
    if (!array)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    assert(vm::Array::get_array_element_size(array) == 1);
    int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
    if (vm::Array::is_out_of_range(array, index))
    {
        RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
    }
    uint8_t value = vm::Array::get_array_data_at<uint8_t>(array, index);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdelemI2)
{
    vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
    if (!array)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    assert(vm::Array::get_array_element_size(array) == 2);
    int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
    if (vm::Array::is_out_of_range(array, index))
    {
        RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
    }
    int16_t value = vm::Array::get_array_data_at<int16_t>(array, index);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdelemU2)
{
    vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
    if (!array)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    assert(vm::Array::get_array_element_size(array) == 2);
    int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
    if (vm::Array::is_out_of_range(array, index))
    {
        RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
    }
    uint16_t value = vm::Array::get_array_data_at<uint16_t>(array, index);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdelemI4)
{
    vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
    if (!array)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    assert(vm::Array::get_array_element_size(array) == 4);
    int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
    if (vm::Array::is_out_of_range(array, index))
    {
        RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
    }
    int32_t value = vm::Array::get_array_data_at<int32_t>(array, index);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdelemI8)
{
    vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
    if (!array)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    assert(vm::Array::get_array_element_size(array) == 8);
    int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
    if (vm::Array::is_out_of_range(array, index))
    {
        RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
    }
    int64_t value = vm::Array::get_array_data_at<int64_t>(array, index);
    set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdelemI)
{
    vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
    if (!array)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    assert(vm::Array::get_array_element_size(array) == PTR_SIZE);
    int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
    if (vm::Array::is_out_of_range(array, index))
    {
        RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
    }
    intptr_t value = vm::Array::get_array_data_at<intptr_t>(array, index);
    set_stack_value_at<intptr_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdelemR4)
{
    vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
    if (!array)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    assert(vm::Array::get_array_element_size(array) == 4);
    int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
    if (vm::Array::is_out_of_range(array, index))
    {
        RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
    }
    float value = vm::Array::get_array_data_at<float>(array, index);
    set_stack_value_at<float>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdelemR8)
{
    vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
    if (!array)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    assert(vm::Array::get_array_element_size(array) == 8);
    int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
    if (vm::Array::is_out_of_range(array, index))
    {
        RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
    }
    double value = vm::Array::get_array_data_at<double>(array, index);
    set_stack_value_at<double>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdelemRef)
{
    vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
    if (!array)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    assert(vm::Array::get_array_element_size(array) == PTR_SIZE);
    int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
    if (vm::Array::is_out_of_range(array, index))
    {
        RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
    }
    vm::RtObject* value = vm::Array::get_array_data_at<vm::RtObject*>(array, index);
    set_stack_value_at<vm::RtObject*>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdelemAnyRef)
{
    vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
    if (!array)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
    if (vm::Array::is_out_of_range(array, index))
    {
        RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
    }
    vm::RtObject* value = vm::Array::get_array_data_at<vm::RtObject*>(array, index);
    const metadata::RtClass* check_klass = get_resolved_data<metadata::RtClass>(imi, ir->ele_klass_idx);
    if (value && !vm::Class::is_assignable_from(value->klass, check_klass))
    {
        RAISE_RUNTIME_ERROR(RtErr::ArrayTypeMismatch);
    }
    set_stack_value_at<vm::RtObject*>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdelemAnyVal)
{
    vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
    if (!array)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
    if (vm::Array::is_out_of_range(array, index))
    {
        RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
    }
    // const metadata::RtClass* ele_klass = vm::Array::get_array_element_class(array);
    // const metadata::RtClass* check_klass = get_resolved_data<metadata::RtClass>(imi, ir->ele_klass_idx);
    // if (!vm::Class::is_pointer_element_compatible_with(ele_klass, check_klass))
    // {
    //     RAISE_RUNTIME_ERROR(RtErr::ArrayTypeMismatch);
    // }
    assert(ir->ele_size == vm::Array::get_array_element_size(array));
    const void* src_addr = vm::Array::get_array_element_address_with_size_as_ptr_void(array, index, ir->ele_size);
    RtStackObject* dst = eval_stack_base + ir->dst;
    std::memcpy(dst, src_addr, ir->ele_size);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StelemI1)
{
    vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
    if (!array)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    assert(vm::Array::get_array_element_size(array) == 1);
    int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
    if (vm::Array::is_out_of_range(array, index))
    {
        RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
    }
    int8_t value = get_stack_value_at<int8_t>(eval_stack_base, ir->value);
    vm::Array::set_array_data_at<int8_t>(array, index, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StelemI2)
{
    vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
    if (!array)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    assert(vm::Array::get_array_element_size(array) == 2);
    int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
    if (vm::Array::is_out_of_range(array, index))
    {
        RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
    }
    int16_t value = get_stack_value_at<int16_t>(eval_stack_base, ir->value);
    vm::Array::set_array_data_at<int16_t>(array, index, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StelemI4)
{
    vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
    if (!array)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    assert(vm::Array::get_array_element_size(array) == 4);
    int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
    if (vm::Array::is_out_of_range(array, index))
    {
        RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
    }
    int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->value);
    vm::Array::set_array_data_at<int32_t>(array, index, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StelemI8)
{
    vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
    if (!array)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    assert(vm::Array::get_array_element_size(array) == 8);
    int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
    if (vm::Array::is_out_of_range(array, index))
    {
        RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
    }
    int64_t value = get_stack_value_at<int64_t>(eval_stack_base, ir->value);
    vm::Array::set_array_data_at<int64_t>(array, index, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StelemI)
{
    vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
    if (!array)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    assert(vm::Array::get_array_element_size(array) == PTR_SIZE);
    int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
    if (vm::Array::is_out_of_range(array, index))
    {
        RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
    }
    intptr_t value = get_stack_value_at<intptr_t>(eval_stack_base, ir->value);
    vm::Array::set_array_data_at<intptr_t>(array, index, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StelemR4)
{
    vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
    if (!array)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    assert(vm::Array::get_array_element_size(array) == 4);
    int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
    if (vm::Array::is_out_of_range(array, index))
    {
        RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
    }
    float value = get_stack_value_at<float>(eval_stack_base, ir->value);
    vm::Array::set_array_data_at<float>(array, index, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StelemR8)
{
    vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
    if (!array)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    assert(vm::Array::get_array_element_size(array) == 8);
    int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
    if (vm::Array::is_out_of_range(array, index))
    {
        RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
    }
    double value = get_stack_value_at<double>(eval_stack_base, ir->value);
    vm::Array::set_array_data_at<double>(array, index, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StelemRef)
{
    vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
    if (!array)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    assert(vm::Array::get_array_element_size(array) == PTR_SIZE);
    int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
    if (vm::Array::is_out_of_range(array, index))
    {
        RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
    }
    vm::RtObject* value = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->value);
    const metadata::RtClass* ele_klass = vm::Array::get_array_element_class(array);
    if (value && !vm::Class::is_assignable_from(value->klass, ele_klass))
    {
        RAISE_RUNTIME_ERROR(RtErr::ArrayTypeMismatch);
    }
    vm::Array::set_array_data_at<vm::RtObject*>(array, index, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StelemAnyRef)
{
    vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
    if (!array)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
    if (vm::Array::is_out_of_range(array, index))
    {
        RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
    }
    vm::RtObject* value = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->value);
    const metadata::RtClass* ele_klass = vm::Array::get_array_element_class(array);
    if (value && !vm::Class::is_assignable_from(value->klass, ele_klass))
    {
        RAISE_RUNTIME_ERROR(RtErr::ArrayTypeMismatch);
    }
    vm::Array::set_array_data_at<vm::RtObject*>(array, index, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StelemAnyVal)
{
    vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
    if (!array)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
    if (vm::Array::is_out_of_range(array, index))
    {
        RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
    }
    const metadata::RtClass* ele_klass = vm::Array::get_array_element_class(array);
    const metadata::RtClass* check_klass = get_resolved_data<metadata::RtClass>(imi, ir->ele_klass_idx);
    if (!vm::Class::is_pointer_element_compatible_with(ele_klass, check_klass))
    {
        RAISE_RUNTIME_ERROR(RtErr::ArrayTypeMismatch);
    }
    assert(ir->ele_size == vm::Array::get_array_element_size(array));
    RtStackObject* src = eval_stack_base + ir->value;
    void* dst_addr = const_cast<void*>(vm::Array::get_array_element_address_with_size_as_ptr_void(array, index, ir->ele_size));
    std::memcpy(dst_addr, src, ir->ele_size);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(MkRefAny)
{
    const metadata::RtClass* klass = get_resolved_data<metadata::RtClass>(imi, ir->klass_idx);
    const void* src_addr = get_stack_value_at<const void*>(eval_stack_base, ir->addr);
    vm::RtTypedReference* dst = get_ptr_stack_value_at<vm::RtTypedReference>(eval_stack_base, ir->dst);
    dst->type_handle = klass->by_val;
    dst->value = src_addr;
    dst->klass = klass;
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(RefAnyVal)
{
    vm::RtTypedReference* src = get_ptr_stack_value_at<vm::RtTypedReference>(eval_stack_base, ir->src);
    const metadata::RtClass* check_klass = get_resolved_data<metadata::RtClass>(imi, ir->klass_idx);
    if (src->klass != check_klass)
    {
        RAISE_RUNTIME_ERROR(RtErr::InvalidCast);
    }
    set_stack_value_at<const void*>(eval_stack_base, ir->dst, src->value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(RefAnyType)
{
    vm::RtTypedReference* src = get_ptr_stack_value_at<vm::RtTypedReference>(eval_stack_base, ir->src);
    set_stack_value_at<const metadata::RtTypeSig*>(eval_stack_base, ir->dst, src->type_handle);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdToken)
{
    const void* token_data = get_resolved_data<void>(imi, ir->token_idx);
    set_stack_value_at<const void*>(eval_stack_base, ir->dst, token_data);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(CkfiniteR4)
{
    float value = get_stack_value_at<float>(eval_stack_base, ir->src);
    if (!std::isfinite(value))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(CkfiniteR8)
{
    double value = get_stack_value_at<double>(eval_stack_base, ir->src);
    if (!std::isfinite(value))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LocAlloc)
{
    uint32_t size = get_stack_value_at<uint32_t>(eval_stack_base, ir->size);
    void* data;
    if (size > 0)
    {
        data = imi->init_locals ? alloc::GeneralAllocation::malloc_zeroed(size) : alloc::GeneralAllocation::malloc(size);
    }
    else
    {
        data = nullptr;
    }
    set_stack_value_at<void*>(eval_stack_base, ir->dst, data);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(Ldftn)
{
    metadata::RtMethodInfo* method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
    HANDLE_RAISE_RUNTIME_ERROR_VOID(vm::Calli::register_managed_target(method));
    set_stack_value_at(eval_stack_base, ir->dst, reinterpret_cast<void*>(method));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(Ldvirtftn)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    const metadata::RtMethodInfo* method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
    assert(method->slot < method->parent->vtable_count);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, actual_method, vm::Method::get_virtual_method_impl(obj, method));
    HANDLE_RAISE_RUNTIME_ERROR_VOID(vm::Calli::register_managed_target(actual_method));
    set_stack_value_at(eval_stack_base, ir->dst, actual_method);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdfldI1)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    const int8_t* field_addr = reinterpret_cast<const int8_t*>(reinterpret_cast<const uint8_t*>(obj) + ir->offset);
    int8_t value = utils::MemOp::load_native(field_addr);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdfldU1)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    const uint8_t* field_addr = reinterpret_cast<const uint8_t*>(obj) + ir->offset;
    uint8_t value = utils::MemOp::load_native(field_addr);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdfldI2)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    const int16_t* field_addr = reinterpret_cast<const int16_t*>(reinterpret_cast<const uint8_t*>(obj) + ir->offset);
    int16_t value = utils::MemOp::load_native(field_addr);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdfldU2)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    const uint16_t* field_addr = reinterpret_cast<const uint16_t*>(reinterpret_cast<const uint8_t*>(obj) + ir->offset);
    uint16_t value = utils::MemOp::load_native(field_addr);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdfldI4)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    const int32_t* field_addr = reinterpret_cast<const int32_t*>(reinterpret_cast<const uint8_t*>(obj) + ir->offset);
    int32_t value = utils::MemOp::load_native(field_addr);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdfldI8)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    const int64_t* field_addr = reinterpret_cast<const int64_t*>(reinterpret_cast<const uint8_t*>(obj) + ir->offset);
    int64_t value = utils::MemOp::load_native(field_addr);
    set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdfldAny)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    const uint8_t* field_addr = reinterpret_cast<const uint8_t*>(obj) + ir->offset;
    RtStackObject* dst = eval_stack_base + ir->dst;
    std::memcpy(dst, field_addr, ir->size);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdvfldI1)
{
    RtStackObject* addr_obj = eval_stack_base + ir->obj;
    const int8_t* field_addr = reinterpret_cast<const int8_t*>(reinterpret_cast<const uint8_t*>(addr_obj) + ir->offset);
    int8_t value = utils::MemOp::load_native(field_addr);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdvfldU1)
{
    RtStackObject* addr_obj = eval_stack_base + ir->obj;
    const uint8_t* field_addr = reinterpret_cast<const uint8_t*>(addr_obj) + ir->offset;
    uint8_t value = utils::MemOp::load_native(field_addr);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdvfldI2)
{
    RtStackObject* addr_obj = eval_stack_base + ir->obj;
    const int16_t* field_addr = reinterpret_cast<const int16_t*>(reinterpret_cast<const uint8_t*>(addr_obj) + ir->offset);
    int16_t value = utils::MemOp::load_native(field_addr);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdvfldU2)
{
    RtStackObject* addr_obj = eval_stack_base + ir->obj;
    const uint16_t* field_addr = reinterpret_cast<const uint16_t*>(reinterpret_cast<const uint8_t*>(addr_obj) + ir->offset);
    uint16_t value = utils::MemOp::load_native(field_addr);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdvfldI4)
{
    RtStackObject* addr_obj = eval_stack_base + ir->obj;
    const int32_t* field_addr = reinterpret_cast<const int32_t*>(reinterpret_cast<const uint8_t*>(addr_obj) + ir->offset);
    int32_t value = utils::MemOp::load_native(field_addr);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdvfldI8)
{
    RtStackObject* addr_obj = eval_stack_base + ir->obj;
    const int64_t* field_addr = reinterpret_cast<const int64_t*>(reinterpret_cast<const uint8_t*>(addr_obj) + ir->offset);
    int64_t value = utils::MemOp::load_native(field_addr);
    set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdvfldAny)
{
    RtStackObject* addr_obj = eval_stack_base + ir->obj;
    const uint8_t* field_addr = reinterpret_cast<const uint8_t*>(addr_obj) + ir->offset;
    RtStackObject* dst = eval_stack_base + ir->dst;
    std::memmove(dst, field_addr, ir->size);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(Ldflda)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    const uint8_t* field_addr = reinterpret_cast<const uint8_t*>(obj) + ir->offset;
    set_stack_value_at(eval_stack_base, ir->dst, field_addr);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StfldI1)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    int8_t value = get_stack_value_at<int8_t>(eval_stack_base, ir->value);
    int8_t* field_addr = reinterpret_cast<int8_t*>(reinterpret_cast<uint8_t*>(obj) + ir->offset);
    utils::MemOp::store_native(field_addr, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StfldI2)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    int16_t value = get_stack_value_at<int16_t>(eval_stack_base, ir->value);
    int16_t* field_addr = reinterpret_cast<int16_t*>(reinterpret_cast<uint8_t*>(obj) + ir->offset);
    utils::MemOp::store_native(field_addr, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StfldI4)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->value);
    int32_t* field_addr = reinterpret_cast<int32_t*>(reinterpret_cast<uint8_t*>(obj) + ir->offset);
    utils::MemOp::store_native(field_addr, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StfldI8)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    int64_t value = get_stack_value_at<int64_t>(eval_stack_base, ir->value);
    int64_t* field_addr = reinterpret_cast<int64_t*>(reinterpret_cast<uint8_t*>(obj) + ir->offset);
    utils::MemOp::store_native(field_addr, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StfldAny)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    RtStackObject* src = eval_stack_base + ir->value;
    uint8_t* field_addr = reinterpret_cast<uint8_t*>(obj) + ir->offset;
    std::memcpy(field_addr, src, ir->size);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdsfldI1)
{
    const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
    TRY_RUN_CLASS_STATIC_CCTOR(field->parent);
    const int8_t* field_addr = get_static_field_address<int8_t>(field);
    int8_t value = utils::MemOp::load_native(field_addr);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdsfldU1)
{
    const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
    TRY_RUN_CLASS_STATIC_CCTOR(field->parent);
    const uint8_t* field_addr = get_static_field_address<uint8_t>(field);
    uint8_t value = utils::MemOp::load_native(field_addr);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdsfldI2)
{
    const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
    TRY_RUN_CLASS_STATIC_CCTOR(field->parent);
    const int16_t* field_addr = get_static_field_address<int16_t>(field);
    int16_t value = utils::MemOp::load_native(field_addr);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdsfldU2)
{
    const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
    TRY_RUN_CLASS_STATIC_CCTOR(field->parent);
    const uint16_t* field_addr = get_static_field_address<uint16_t>(field);
    uint16_t value = utils::MemOp::load_native(field_addr);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdsfldI4)
{
    const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
    TRY_RUN_CLASS_STATIC_CCTOR(field->parent);
    const int32_t* field_addr = get_static_field_address<int32_t>(field);
    int32_t value = utils::MemOp::load_native(field_addr);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdsfldI8)
{
    const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
    TRY_RUN_CLASS_STATIC_CCTOR(field->parent);
    const int64_t* field_addr = get_static_field_address<int64_t>(field);
    int64_t value = utils::MemOp::load_native(field_addr);
    set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdsfldAny)
{
    const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
    TRY_RUN_CLASS_STATIC_CCTOR(field->parent);
    const uint8_t* field_addr = get_static_field_address<uint8_t>(field);
    RtStackObject* dst = eval_stack_base + ir->dst;
    std::memcpy(dst, field_addr, ir->size);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(Ldsflda)
{
    const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
    metadata::RtClass* klass = field->parent;
    TRY_RUN_CLASS_STATIC_CCTOR(klass);
    const void* field_addr = get_static_field_address<void>(field);
    set_stack_value_at(eval_stack_base, ir->dst, field_addr);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdsfldRvaData)
{
    const void* rva_data = get_resolved_data<void>(imi, ir->data);
    set_stack_value_at(eval_stack_base, ir->dst, rva_data);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StsfldI1)
{
    int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->value);
    const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
    metadata::RtClass* klass = field->parent;
    TRY_RUN_CLASS_STATIC_CCTOR(klass);
    int8_t* field_addr = get_static_field_address<int8_t>(field);
    utils::MemOp::store_native(field_addr, static_cast<int8_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StsfldI2)
{
    int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->value);
    const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
    metadata::RtClass* klass = field->parent;
    TRY_RUN_CLASS_STATIC_CCTOR(klass);
    int16_t* field_addr = get_static_field_address<int16_t>(field);
    utils::MemOp::store_native(field_addr, static_cast<int16_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StsfldI4)
{
    int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->value);
    const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
    metadata::RtClass* klass = field->parent;
    TRY_RUN_CLASS_STATIC_CCTOR(klass);
    int32_t* field_addr = get_static_field_address<int32_t>(field);
    utils::MemOp::store_native(field_addr, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StsfldI8)
{
    int64_t value = get_stack_value_at<int64_t>(eval_stack_base, ir->value);
    const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
    metadata::RtClass* klass = field->parent;
    TRY_RUN_CLASS_STATIC_CCTOR(klass);
    int64_t* field_addr = get_static_field_address<int64_t>(field);
    utils::MemOp::store_native(field_addr, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StsfldAny)
{
    RtStackObject* src = eval_stack_base + ir->value;
    const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
    metadata::RtClass* klass = field->parent;
    TRY_RUN_CLASS_STATIC_CCTOR(klass);
    uint8_t* field_addr = get_static_field_address<uint8_t>(field);
    std::memcpy(field_addr, src, ir->size);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(RetVoid)
{
    LEAVE_FRAME();
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(RetI4)
{
    eval_stack_base->i32 = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
    LEAVE_FRAME();
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(RetI8)
{
    eval_stack_base->i64 = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
    LEAVE_FRAME();
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(RetAny)
{
    std::memcpy(eval_stack_base, eval_stack_base + ir->src, ir->size * sizeof(RtStackObject));
    LEAVE_FRAME();
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN_LITE(CallInterp)
{
    const auto* ir = reinterpret_cast<const ll::CallInterp*>(ip);
    const metadata::RtMethodInfo* target_method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
    if (vm::Method::is_static(target_method))
    {
        TRY_RUN_CLASS_STATIC_CCTOR(target_method->parent);
    }
    const uint8_t* next_ip = reinterpret_cast<const uint8_t*>(ir + 1);
    ENTER_INTERP_FRAME(target_method, ir->frame_base, next_ip);
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(CallVirtInterp)
{
    const auto* ir = reinterpret_cast<const ll::CallVirtInterp*>(ip);
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->frame_base);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    const metadata::RtMethodInfo* original_method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, actual_method, vm::Method::get_virtual_method_impl(obj, original_method));
    if (actual_method->invoker_type == metadata::RtInvokerType::Interpreter)
    {
        if (vm::Class::is_value_type(actual_method->parent))
        {
            set_stack_value_at(eval_stack_base, ir->frame_base, obj + 1);
        }
        ENTER_INTERP_FRAME(actual_method, ir->frame_base, reinterpret_cast<const uint8_t*>(ir + 1));
    }
    else
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
        RtStackObject* frame_base = eval_stack_base + ir->frame_base;
        HANDLE_RAISE_RUNTIME_ERROR_VOID(
            CAST_AS_NOEXCEP_INVOKE_METHOD_POINTER(actual_method->invoke_method_ptr)(actual_method->virtual_method_ptr, actual_method, frame_base, frame_base));
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(CallInternalCall)
{
    const auto* ir = reinterpret_cast<const ll::CallInternalCall*>(ip);
    const metadata::RtMethodInfo* target_method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
    if (vm::Method::is_static(target_method))
    {
        TRY_RUN_CLASS_STATIC_CCTOR(target_method->parent);
    }
    ip = reinterpret_cast<const uint8_t*>(ir + 1);
    RtStackObject* frame_base = eval_stack_base + ir->frame_base;
    HANDLE_RAISE_RUNTIME_ERROR_VOID(target_method->invoke_method_ptr(target_method->method_ptr, target_method, frame_base, frame_base));
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(CallIntrinsic)
{
    const auto* ir = reinterpret_cast<const ll::CallIntrinsic*>(ip);
    const metadata::RtMethodInfo* target_method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
    if (vm::Method::is_static(target_method))
    {
        TRY_RUN_CLASS_STATIC_CCTOR(target_method->parent);
    }
    ip = reinterpret_cast<const uint8_t*>(ir + 1);
    RtStackObject* frame_base = eval_stack_base + ir->frame_base;
    HANDLE_RAISE_RUNTIME_ERROR_VOID(target_method->invoke_method_ptr(target_method->method_ptr, target_method, frame_base, frame_base));
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(CallPInvoke)
{
    const auto* ir = reinterpret_cast<const ll::CallPInvoke*>(ip);
    const metadata::RtMethodInfo* target_method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
    if (vm::Method::is_static(target_method))
    {
        TRY_RUN_CLASS_STATIC_CCTOR(target_method->parent);
    }
    ip = reinterpret_cast<const uint8_t*>(ir + 1);
    RtStackObject* frame_base = eval_stack_base + ir->frame_base;
    HANDLE_RAISE_RUNTIME_ERROR_VOID(target_method->invoke_method_ptr(target_method->method_ptr, target_method, frame_base, frame_base));
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(CallAot)
{
    const auto* ir = reinterpret_cast<const ll::CallAot*>(ip);
    const metadata::RtMethodInfo* target_method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
    if (vm::Method::is_static(target_method))
    {
        TRY_RUN_CLASS_STATIC_CCTOR(target_method->parent);
    }
    ip = reinterpret_cast<const uint8_t*>(ir + 1);
    RtStackObject* frame_base = eval_stack_base + ir->frame_base;
    HANDLE_RAISE_RUNTIME_ERROR_VOID(target_method->invoke_method_ptr(target_method->method_ptr, target_method, frame_base, frame_base));
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(CallRuntimeImplemented)
{
    const auto* ir = reinterpret_cast<const ll::CallRuntimeImplemented*>(ip);
    const metadata::RtMethodInfo* target_method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
    if (vm::Method::is_static(target_method))
    {
        TRY_RUN_CLASS_STATIC_CCTOR(target_method->parent);
    }
    ip = reinterpret_cast<const uint8_t*>(ir + 1);
    RtStackObject* frame_base = eval_stack_base + ir->frame_base;
    HANDLE_RAISE_RUNTIME_ERROR_VOID(target_method->invoke_method_ptr(target_method->method_ptr, target_method, frame_base, frame_base));
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(CalliNative)
{
    const auto* ir = reinterpret_cast<const ll::CalliNative*>(ip);
    const auto* bridge = get_resolved_data<const metadata::RtCalliBridge>(imi, ir->bridge_idx);
#if LEANCLR_BCL_CORECLR_NET10
    frame->gc_live_stack_size = frame->eval_stack_size;
#endif
    void* target = get_stack_value_at<void*>(eval_stack_base, ir->method_idx);
    RtStackObject* args = eval_stack_base + ir->frame_base;
    HANDLE_RAISE_RUNTIME_ERROR_VOID(vm::Calli::invoke_native(bridge, target, args, args));
    ip = reinterpret_cast<const uint8_t*>(ir + 1);
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(CalliInterp)
{
    const auto* ir = reinterpret_cast<const ll::CalliInterp*>(ip);
    auto* binding = get_resolved_data<vm::Calli::ManagedBinding>(imi, ir->method_sig_idx);
    const uint8_t* next_ip = reinterpret_cast<const uint8_t*>(ir + 1);
    const metadata::RtMethodInfo* target_method = get_stack_value_at<const metadata::RtMethodInfo*>(eval_stack_base, ir->method_idx);
    HANDLE_RAISE_RUNTIME_ERROR_VOID(vm::Calli::validate_managed_binding(*binding, target_method));
    if (vm::Method::is_static(target_method))
        TRY_RUN_CLASS_STATIC_CCTOR(target_method->parent);

    if (vm::Method::is_instance(target_method))
        (eval_stack_base + ir->frame_base)[0].ptr = vm::Calli::value_receiver(target_method, (eval_stack_base + ir->frame_base)[0].ptr);

    if (target_method->invoker_type == metadata::RtInvokerType::Interpreter ||
        (target_method->invoker_type == metadata::RtInvokerType::Aot && vm::Method::is_instance(target_method) && !(eval_stack_base + ir->frame_base)[0].ptr))
    {
        ENTER_INTERP_FRAME(target_method, ir->frame_base, reinterpret_cast<const uint8_t*>(ir + 1));
    }
    else
    {
        RtStackObject* frame_base = eval_stack_base + ir->frame_base;
        HANDLE_RAISE_RUNTIME_ERROR_VOID(target_method->invoke_method_ptr(target_method->method_ptr, target_method, frame_base, frame_base));
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN(BoxRefInplace)
{
    const void* src_ptr = get_stack_value_at<const void*>(eval_stack_base, ir->src);
    metadata::RtClass* to_klass = get_resolved_data<metadata::RtClass>(imi, ir->klass_idx);
    HANDLE_RAISE_RUNTIME_ERROR(vm::RtObject*, boxed_obj, LEANCLR_BOX_OBJECT_INTERP(to_klass, src_ptr));
    set_stack_value_at<vm::RtObject*>(eval_stack_base, ir->dst, boxed_obj);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN_LITE(NewObjInterp)
{
    const auto* ir = reinterpret_cast<const ll::NewObjInterp*>(ip);
    const metadata::RtMethodInfo* ctor = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
    const metadata::RtClass* klass = ctor->parent;
    TRY_RUN_CLASS_STATIC_CCTOR(klass);
    HANDLE_RAISE_RUNTIME_ERROR(vm::RtObject*, obj, LEANCLR_NEWOBJ_INTERP(klass));
    RtStackObject* frame_base = eval_stack_base + ir->frame_base;
    std::memmove(frame_base + 1, frame_base, static_cast<size_t>(ir->total_params_stack_object_size) * sizeof(RtStackObject));
    frame_base->obj = obj;
    ENTER_INTERP_FRAME(ctor, ir->frame_base, reinterpret_cast<const uint8_t*>(ir + 1));
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(NewValueTypeInterp)
{
    const auto* ir = reinterpret_cast<const ll::NewValueTypeInterp*>(ip);
    const metadata::RtMethodInfo* ctor = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
    const metadata::RtClass* klass = ctor->parent;
    RtStackObject* original_frame_base = eval_stack_base + ir->frame_base;
    const size_t value_stack_objects = InterpDefs::get_stack_object_size_by_byte_size(klass->instance_size_without_header);
    RtStackObject* final_frame_base = original_frame_base + value_stack_objects;
    std::memmove(final_frame_base + 1, original_frame_base, static_cast<size_t>(ir->total_params_stack_object_size) * sizeof(RtStackObject));
    final_frame_base->ptr = original_frame_base;
    std::memset(original_frame_base, 0, value_stack_objects * sizeof(RtStackObject));
    ENTER_INTERP_FRAME(ctor, ir->frame_base + value_stack_objects, reinterpret_cast<const uint8_t*>(ir + 1));
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN(NewObjInternalCall)
{
    const metadata::RtMethodInfo* target_method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
    TRY_RUN_CLASS_STATIC_CCTOR(target_method->parent);
    vm::InternalCallInvoker invoker = vm::InternalCalls::get_internal_call_invoker_by_id_unchecked(ir->invoker_idx);
    RtStackObject* frame_base = eval_stack_base + ir->frame_base;
    HANDLE_RAISE_RUNTIME_ERROR_VOID(invoker(target_method->method_ptr, target_method, frame_base, frame_base));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(NewObjIntrinsic)
{
    const metadata::RtMethodInfo* target_method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
    TRY_RUN_CLASS_STATIC_CCTOR(target_method->parent);
    vm::InternalCallInvoker invoker = vm::Intrinsics::get_intrinsic_invoker_by_id_unchecked(ir->invoker_idx);
    RtStackObject* frame_base = eval_stack_base + ir->frame_base;
    HANDLE_RAISE_RUNTIME_ERROR_VOID(invoker(target_method->method_ptr, target_method, frame_base, frame_base));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(NewObjAot)
{
    const metadata::RtMethodInfo* ctor = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
    const metadata::RtClass* klass = ctor->parent;
    TRY_RUN_CLASS_STATIC_CCTOR(klass);
    HANDLE_RAISE_RUNTIME_ERROR(vm::RtObject*, obj, LEANCLR_NEWOBJ_INTERP(klass));
    RtStackObject* frame_base = eval_stack_base + ir->frame_base;
    std::memmove(frame_base + 1, frame_base, static_cast<size_t>(ir->total_params_stack_object_size) * sizeof(RtStackObject));
    frame_base->obj = obj;
    auto invoker = CAST_AS_NOEXCEP_INVOKE_METHOD_POINTER(ctor->invoke_method_ptr);
    HANDLE_RAISE_RUNTIME_ERROR_VOID(invoker(ctor->method_ptr, ctor, frame_base, frame_base));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(NewValueTypeAot)
{
    const metadata::RtMethodInfo* ctor = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
    const metadata::RtClass* klass = ctor->parent;
    RtStackObject* original_frame_base = eval_stack_base + ir->frame_base;
    const size_t value_stack_objects = InterpDefs::get_stack_object_size_by_byte_size(klass->instance_size_without_header);
    RtStackObject* final_frame_base = original_frame_base + value_stack_objects;
    std::memmove(final_frame_base + 1, original_frame_base, static_cast<size_t>(ir->total_params_stack_object_size) * sizeof(RtStackObject));
    final_frame_base->ptr = original_frame_base;
    std::memset(original_frame_base, 0, value_stack_objects * sizeof(RtStackObject));
    auto invoker = CAST_AS_NOEXCEP_INVOKE_METHOD_POINTER(ctor->invoke_method_ptr);
    HANDLE_RAISE_RUNTIME_ERROR_VOID(invoker(ctor->method_ptr, ctor, final_frame_base, final_frame_base));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(Throw)
{
    vm::RtException* ex = get_stack_value_at<vm::RtException*>(eval_stack_base, ir->ex);
    if (!ex)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    if (!vm::Class::is_exception_sub_class(reinterpret_cast<vm::RtObject*>(ex)->klass))
    {
        RAISE_RUNTIME_ERROR(RtErr::InvalidCast);
    }
    RAISE_RUNTIME_EXCEPTION(ex);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(Rethrow)
{
    vm::RtException* ex = find_exception_in_enclosing_throw_flow(frame, static_cast<uint32_t>(ip - imi->codes));
    if (!ex)
    {
        RAISE_RUNTIME_ERROR(RtErr::ExecutionEngine);
    }
    RAISE_RUNTIME_EXCEPTION(ex);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN_LITE(LeaveTryWithFinally)
{
    const auto* ir = reinterpret_cast<const ll::LeaveTryWithFinally*>(ip);
    assert(ir->finally_clauses_count > 0);
    assert(ir->first_finally_clause_index < imi->exception_clause_count);
    const uint8_t* target_ip = ip + ir->target_offset;
    const RtInterpExceptionClause* finally_clause = &imi->exception_clauses[ir->first_finally_clause_index];
    push_leave_flow(frame, ip, target_ip, finally_clause, ir->first_finally_clause_index + 1, ir->finally_clauses_count - 1);
    ip = imi->codes + finally_clause->handler_begin_offset;
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(LeaveCatchWithFinally)
{
    const auto* ir = reinterpret_cast<const ll::LeaveCatchWithFinally*>(ip);
    assert(ir->finally_clauses_count > 0);
    assert(ir->first_finally_clause_index < imi->exception_clause_count);
    vm::RtException* ex = get_exception_in_last_throw_flow(frame, static_cast<uint32_t>(ip - imi->codes));
    pop_throw_flow(ex, frame);
    const uint8_t* target_ip = ip + ir->target_offset;
    const RtInterpExceptionClause* finally_clause = &imi->exception_clauses[ir->first_finally_clause_index];
    push_leave_flow(frame, ip, target_ip, finally_clause, ir->first_finally_clause_index + 1, ir->finally_clauses_count - 1);
    ip = imi->codes + finally_clause->handler_begin_offset;
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(LeaveCatchWithoutFinally)
{
    const auto* ir = reinterpret_cast<const ll::LeaveCatchWithoutFinally*>(ip);
    vm::RtException* ex = get_exception_in_last_throw_flow(frame, static_cast<uint32_t>(ip - imi->codes));
    pop_throw_flow(ex, frame);
    ip += ir->target_offset;
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(EndFilter)
{
    const auto* ir = reinterpret_cast<const ll::EndFilter*>(ip);
    int32_t cond = get_stack_value_at<int32_t>(eval_stack_base, ir->cond);
    if (cond)
    {
        ip = reinterpret_cast<const uint8_t*>(ir + 1);
        setup_filter_handler(imi, frame, ip);
        vm::RtException* ex = get_exception_in_last_throw_flow(frame, static_cast<uint32_t>(ip - imi->codes));
        set_stack_value_at<vm::RtException*>(eval_stack_base, imi->total_arg_and_local_stack_object_size, ex);
    }
    else
    {
        goto unwind_exception_handler;
    }
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(EndFinally)
{
    goto unwind_exception_handler;
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN_LITE(EndFault)
{
    goto unwind_exception_handler;
}
LEANCLR_CASE_END_LITE()

LEANCLR_CASE_BEGIN(LdIndI1)
{
    int8_t value = get_ind_stack_value_at<int8_t>(eval_stack_base, ir->src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdIndU1)
{
    uint8_t value = get_ind_stack_value_at<uint8_t>(eval_stack_base, ir->src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdIndI2)
{
    int16_t value = get_ind_stack_value_at<int16_t>(eval_stack_base, ir->src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdIndU2)
{
    uint16_t value = get_ind_stack_value_at<uint16_t>(eval_stack_base, ir->src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdIndI4)
{
    int32_t value = get_ind_stack_value_at<int32_t>(eval_stack_base, ir->src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdIndI8)
{
    int64_t value = get_ind_stack_value_at<int64_t>(eval_stack_base, ir->src);
    set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StIndI1)
{
    int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
    set_ind_stack_value_at<int8_t>(eval_stack_base, ir->dst, static_cast<int8_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StIndI2)
{
    int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
    set_ind_stack_value_at<int16_t>(eval_stack_base, ir->dst, static_cast<int16_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StIndI4)
{
    int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
    set_ind_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StIndI8)
{
    int64_t value = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
    set_ind_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StIndI8I4)
{
    int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
    set_ind_stack_value_at<int64_t>(eval_stack_base, ir->dst, static_cast<int64_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StIndI8U4)
{
    uint32_t value = get_stack_value_at<uint32_t>(eval_stack_base, ir->src);
    set_ind_stack_value_at<int64_t>(eval_stack_base, ir->dst, static_cast<int64_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvI1I4)
{
    int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
    int8_t value = static_cast<int8_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvI1I8)
{
    int64_t src = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
    int8_t value = static_cast<int8_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvI1R4)
{
    float src = get_stack_value_at<float>(eval_stack_base, ir->src);
    int32_t value = cast_float_to_small_int<float, int8_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvI1R8)
{
    double src = get_stack_value_at<double>(eval_stack_base, ir->src);
    int32_t value = cast_float_to_small_int<double, int8_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvU1I4)
{
    int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
    uint8_t value = static_cast<uint8_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvU1I8)
{
    int64_t src = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
    uint8_t value = static_cast<uint8_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvU1R4)
{
    float src = get_stack_value_at<float>(eval_stack_base, ir->src);
    int32_t value = cast_float_to_small_int<float, uint8_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvU1R8)
{
    double src = get_stack_value_at<double>(eval_stack_base, ir->src);
    int32_t value = cast_float_to_small_int<double, uint8_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvI2I4)
{
    int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
    int16_t value = static_cast<int16_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvI2I8)
{
    int64_t src = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
    int16_t value = static_cast<int16_t>(src);
    set_stack_value_at<int64_t>(eval_stack_base, ir->dst, static_cast<int64_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvI2R4)
{
    float src = get_stack_value_at<float>(eval_stack_base, ir->src);
    int32_t value = cast_float_to_small_int<float, int16_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvI2R8)
{
    double src = get_stack_value_at<double>(eval_stack_base, ir->src);
    int32_t value = cast_float_to_small_int<double, int16_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvU2I4)
{
    int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
    uint16_t value = static_cast<uint16_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvU2I8)
{
    int64_t src = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
    uint16_t value = static_cast<uint16_t>(src);
    set_stack_value_at<int64_t>(eval_stack_base, ir->dst, static_cast<int64_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvU2R4)
{
    float src = get_stack_value_at<float>(eval_stack_base, ir->src);
    int32_t value = cast_float_to_small_int<float, uint16_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvU2R8)
{
    double src = get_stack_value_at<double>(eval_stack_base, ir->src);
    int32_t value = cast_float_to_small_int<double, uint16_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvI4I8)
{
    int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, src);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvI4R4)
{
    float src = get_stack_value_at<float>(eval_stack_base, ir->src);
    int32_t value = cast_float_to_i32<float, int32_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvI4R8)
{
    double src = get_stack_value_at<double>(eval_stack_base, ir->src);
    int32_t value = cast_float_to_i32<double, int32_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvU4I8)
{
    uint32_t src = get_stack_value_at<uint32_t>(eval_stack_base, ir->src);
    set_stack_value_at<uint32_t>(eval_stack_base, ir->dst, src);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvU4R4)
{
    float src = get_stack_value_at<float>(eval_stack_base, ir->src);
    int32_t value = cast_float_to_i32<float, uint32_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvU4R8)
{
    double src = get_stack_value_at<double>(eval_stack_base, ir->src);
    int32_t value = cast_float_to_i32<double, uint32_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvI8I4)
{
    int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
    set_stack_value_at<int64_t>(eval_stack_base, ir->dst, static_cast<int64_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvI8U4)
{
    uint32_t src = get_stack_value_at<uint32_t>(eval_stack_base, ir->src);
    set_stack_value_at<int64_t>(eval_stack_base, ir->dst, static_cast<int64_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvI8R4)
{
    float src = get_stack_value_at<float>(eval_stack_base, ir->src);
    int64_t value = cast_float_to_i64<float, int64_t>(src);
    set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvI8R8)
{
    double src = get_stack_value_at<double>(eval_stack_base, ir->src);
    int64_t value = cast_float_to_i64<double, int64_t>(src);
    set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvU8I4)
{
    uint32_t src = get_stack_value_at<uint32_t>(eval_stack_base, ir->src);
    set_stack_value_at<uint64_t>(eval_stack_base, ir->dst, static_cast<uint64_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvU8R4)
{
    float src = get_stack_value_at<float>(eval_stack_base, ir->src);
    int64_t value = cast_float_to_i64<float, uint64_t>(src);
    set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvU8R8)
{
    double src = get_stack_value_at<double>(eval_stack_base, ir->src);
    int64_t value = cast_float_to_i64<double, uint64_t>(src);
    set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvR4I4)
{
    int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
    set_stack_value_at<float>(eval_stack_base, ir->dst, static_cast<float>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvR4I8)
{
    int64_t src = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
    set_stack_value_at<float>(eval_stack_base, ir->dst, static_cast<float>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvR4R8)
{
    double src = get_stack_value_at<double>(eval_stack_base, ir->src);
    set_stack_value_at<float>(eval_stack_base, ir->dst, static_cast<float>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvR8I4)
{
    int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
    set_stack_value_at<double>(eval_stack_base, ir->dst, static_cast<double>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvR8I8)
{
    int64_t src = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
    set_stack_value_at<double>(eval_stack_base, ir->dst, static_cast<double>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvR8R4)
{
    float src = get_stack_value_at<float>(eval_stack_base, ir->src);
    set_stack_value_at<double>(eval_stack_base, ir->dst, static_cast<double>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdelemaReadOnly)
{
    vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
    if (!array)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
    if (vm::Array::is_out_of_range(array, index))
    {
        RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
    }
    const void* element_addr = vm::Array::get_array_element_address_as_ptr_void(array, index);
    set_stack_value_at(eval_stack_base, ir->dst, element_addr);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(InitBlk)
{
    void* dst = get_stack_value_at<void*>(eval_stack_base, ir->addr);
    uint8_t value = get_stack_value_at<uint8_t>(eval_stack_base, ir->value);
    uint32_t size = get_stack_value_at<uint32_t>(eval_stack_base, ir->size);
    std::memset(dst, value, size);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(CpBlk)
{
    void* dst = get_stack_value_at<void*>(eval_stack_base, ir->dst);
    const void* src = get_stack_value_at<const void*>(eval_stack_base, ir->src);
    uint32_t size = get_stack_value_at<uint32_t>(eval_stack_base, ir->size);
    std::memcpy(dst, src, size);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(GetEnumLongHashCode)
{
    int64_t value = get_ind_stack_value_at<int64_t>(eval_stack_base, ir->value_ptr);
    int32_t hash = vm::Enum::get_enum_long_hash_code(value);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, hash);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdIndI2Unaligned)
{
    const uint8_t* addr = get_stack_value_at<const uint8_t*>(eval_stack_base, ir->src);
    int16_t value = utils::MemOp::read_i16_may_unaligned(addr);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdIndU2Unaligned)
{
    const uint8_t* addr = get_stack_value_at<const uint8_t*>(eval_stack_base, ir->src);
    uint16_t value = utils::MemOp::read_u16_may_unaligned(addr);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdIndI4Unaligned)
{
    const uint8_t* addr = get_stack_value_at<const uint8_t*>(eval_stack_base, ir->src);
    int32_t value = utils::MemOp::read_i32_may_unaligned(addr);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdIndI8Unaligned)
{
    const uint8_t* addr = get_stack_value_at<const uint8_t*>(eval_stack_base, ir->src);
    int64_t value = utils::MemOp::read_i64_may_unaligned(addr);
    set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StIndI2Unaligned)
{
    uint8_t* addr = get_stack_value_at<uint8_t*>(eval_stack_base, ir->dst);
    int16_t value = get_stack_value_at<int16_t>(eval_stack_base, ir->src);
    utils::MemOp::write_i16_may_unaligned(addr, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StIndI4Unaligned)
{
    uint8_t* addr = get_stack_value_at<uint8_t*>(eval_stack_base, ir->dst);
    int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
    utils::MemOp::write_i32_may_unaligned(addr, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StIndI8Unaligned)
{
    uint8_t* addr = get_stack_value_at<uint8_t*>(eval_stack_base, ir->dst);
    int64_t value = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
    utils::MemOp::write_i64_may_unaligned(addr, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StIndI8I4Unaligned)
{
    uint8_t* addr = get_stack_value_at<uint8_t*>(eval_stack_base, ir->dst);
    int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
    utils::MemOp::write_i64_may_unaligned(addr, static_cast<int64_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StIndI8U4Unaligned)
{
    uint8_t* addr = get_stack_value_at<uint8_t*>(eval_stack_base, ir->dst);
    uint32_t value = get_stack_value_at<uint32_t>(eval_stack_base, ir->src);
    utils::MemOp::write_i64_may_unaligned(addr, static_cast<int64_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(AddOvfI4)
{
    int32_t left = get_stack_value_at<int32_t>(eval_stack_base, ir->arg1);
    int32_t right = get_stack_value_at<int32_t>(eval_stack_base, ir->arg2);
    int32_t result;
    if (CHECK_ADD_OVERFLOW_I32(left, right, &result))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, result);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(AddOvfI8)
{
    int64_t left = get_stack_value_at<int64_t>(eval_stack_base, ir->arg1);
    int64_t right = get_stack_value_at<int64_t>(eval_stack_base, ir->arg2);
    int64_t result;
    if (CHECK_ADD_OVERFLOW_I64(left, right, &result))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int64_t>(eval_stack_base, ir->dst, result);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(AddOvfUnI4)
{
    uint32_t left = get_stack_value_at<uint32_t>(eval_stack_base, ir->arg1);
    uint32_t right = get_stack_value_at<uint32_t>(eval_stack_base, ir->arg2);
    uint32_t result;
    if (CHECK_ADD_OVERFLOW_U32(left, right, &result))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<uint32_t>(eval_stack_base, ir->dst, result);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(AddOvfUnI8)
{
    uint64_t left = get_stack_value_at<uint64_t>(eval_stack_base, ir->arg1);
    uint64_t right = get_stack_value_at<uint64_t>(eval_stack_base, ir->arg2);
    uint64_t result;
    if (CHECK_ADD_OVERFLOW_U64(left, right, &result))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<uint64_t>(eval_stack_base, ir->dst, result);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(MulOvfI4)
{
    int32_t left = get_stack_value_at<int32_t>(eval_stack_base, ir->arg1);
    int32_t right = get_stack_value_at<int32_t>(eval_stack_base, ir->arg2);
    int32_t result;
    if (CHECK_MUL_OVERFLOW_I32(left, right, &result))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, result);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(MulOvfI8)
{
    int64_t left = get_stack_value_at<int64_t>(eval_stack_base, ir->arg1);
    int64_t right = get_stack_value_at<int64_t>(eval_stack_base, ir->arg2);
    int64_t result;
    if (CHECK_MUL_OVERFLOW_I64(left, right, &result))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int64_t>(eval_stack_base, ir->dst, result);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(MulOvfUnI4)
{
    uint32_t left = get_stack_value_at<uint32_t>(eval_stack_base, ir->arg1);
    uint32_t right = get_stack_value_at<uint32_t>(eval_stack_base, ir->arg2);
    uint32_t result;
    if (CHECK_MUL_OVERFLOW_U32(left, right, &result))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<uint32_t>(eval_stack_base, ir->dst, result);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(MulOvfUnI8)
{
    uint64_t left = get_stack_value_at<uint64_t>(eval_stack_base, ir->arg1);
    uint64_t right = get_stack_value_at<uint64_t>(eval_stack_base, ir->arg2);
    uint64_t result;
    if (CHECK_MUL_OVERFLOW_U64(left, right, &result))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<uint64_t>(eval_stack_base, ir->dst, result);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(SubOvfI4)
{
    int32_t left = get_stack_value_at<int32_t>(eval_stack_base, ir->arg1);
    int32_t right = get_stack_value_at<int32_t>(eval_stack_base, ir->arg2);
    int32_t result;
    if (CHECK_SUB_OVERFLOW_I32(left, right, &result))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, result);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(SubOvfI8)
{
    int64_t left = get_stack_value_at<int64_t>(eval_stack_base, ir->arg1);
    int64_t right = get_stack_value_at<int64_t>(eval_stack_base, ir->arg2);
    int64_t result;
    if (CHECK_SUB_OVERFLOW_I64(left, right, &result))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int64_t>(eval_stack_base, ir->dst, result);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(SubOvfUnI4)
{
    uint32_t left = get_stack_value_at<uint32_t>(eval_stack_base, ir->arg1);
    uint32_t right = get_stack_value_at<uint32_t>(eval_stack_base, ir->arg2);
    uint32_t result;
    if (CHECK_SUB_OVERFLOW_U32(left, right, &result))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<uint32_t>(eval_stack_base, ir->dst, result);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(SubOvfUnI8)
{
    uint64_t left = get_stack_value_at<uint64_t>(eval_stack_base, ir->arg1);
    uint64_t right = get_stack_value_at<uint64_t>(eval_stack_base, ir->arg2);
    uint64_t result;
    if (CHECK_SUB_OVERFLOW_U64(left, right, &result))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<uint64_t>(eval_stack_base, ir->dst, result);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfI1I4)
{
    int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
    if (src < std::numeric_limits<int8_t>::min() || src > std::numeric_limits<int8_t>::max())
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    int8_t value = static_cast<int8_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfI1I8)
{
    int64_t src = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
    if (src < std::numeric_limits<int8_t>::min() || src > std::numeric_limits<int8_t>::max())
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    int8_t value = static_cast<int8_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfI1R4)
{
    float src = get_stack_value_at<float>(eval_stack_base, ir->src);
    if (src < static_cast<float>(std::numeric_limits<int8_t>::min()) || src > static_cast<float>(std::numeric_limits<int8_t>::max()) || std::isnan(src))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<float, int8_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfI1R8)
{
    double src = get_stack_value_at<double>(eval_stack_base, ir->src);
    if (src < static_cast<double>(std::numeric_limits<int8_t>::min()) || src > static_cast<double>(std::numeric_limits<int8_t>::max()) || std::isnan(src))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<double, int8_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfU1I4)
{
    int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
    if (src < std::numeric_limits<uint8_t>::min() || src > std::numeric_limits<uint8_t>::max())
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    uint8_t value = static_cast<uint8_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfU1I8)
{
    int64_t src = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
    if (src < std::numeric_limits<uint8_t>::min() || src > std::numeric_limits<uint8_t>::max())
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    uint8_t value = static_cast<uint8_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfU1R4)
{
    float src = get_stack_value_at<float>(eval_stack_base, ir->src);
    if (src < static_cast<float>(std::numeric_limits<uint8_t>::min()) || src > static_cast<float>(std::numeric_limits<uint8_t>::max()) || std::isnan(src))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<float, uint8_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfU1R8)
{
    double src = get_stack_value_at<double>(eval_stack_base, ir->src);
    if (src < static_cast<double>(std::numeric_limits<uint8_t>::min()) || src > static_cast<double>(std::numeric_limits<uint8_t>::max()) || std::isnan(src))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<double, uint8_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfI2I4)
{
    int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
    if (src < std::numeric_limits<int16_t>::min() || src > std::numeric_limits<int16_t>::max())
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    int16_t value = static_cast<int16_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfI2I8)
{
    int64_t src = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
    if (src < std::numeric_limits<int16_t>::min() || src > std::numeric_limits<int16_t>::max())
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    int16_t value = static_cast<int16_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfI2R4)
{
    float src = get_stack_value_at<float>(eval_stack_base, ir->src);
    if (src < static_cast<float>(std::numeric_limits<int16_t>::min()) || src > static_cast<float>(std::numeric_limits<int16_t>::max()) || std::isnan(src))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<float, int16_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfI2R8)
{
    double src = get_stack_value_at<double>(eval_stack_base, ir->src);
    if (src < static_cast<double>(std::numeric_limits<int16_t>::min()) || src > static_cast<double>(std::numeric_limits<int16_t>::max()) || std::isnan(src))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<double, int16_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfU2I4)
{
    int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
    if (src < std::numeric_limits<uint16_t>::min() || src > std::numeric_limits<uint16_t>::max())
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    uint16_t value = static_cast<uint16_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfU2I8)
{
    int64_t src = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
    if (src < std::numeric_limits<uint16_t>::min() || src > std::numeric_limits<uint16_t>::max())
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    uint16_t value = static_cast<uint16_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfU2R4)
{
    float src = get_stack_value_at<float>(eval_stack_base, ir->src);
    if (src < static_cast<float>(std::numeric_limits<uint16_t>::min()) || src > static_cast<float>(std::numeric_limits<uint16_t>::max()) || std::isnan(src))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<float, uint16_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfU2R8)
{
    double src = get_stack_value_at<double>(eval_stack_base, ir->src);
    if (src < static_cast<double>(std::numeric_limits<uint16_t>::min()) || src > static_cast<double>(std::numeric_limits<uint16_t>::max()) || std::isnan(src))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<double, uint16_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfI4I8)
{
    int64_t src = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
    if (src < std::numeric_limits<int32_t>::min() || src > std::numeric_limits<int32_t>::max())
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    int32_t value = static_cast<int32_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfI4R4)
{
    float src = get_stack_value_at<float>(eval_stack_base, ir->src);
    if (src < static_cast<float>(std::numeric_limits<int32_t>::min()) || src > static_cast<float>(std::numeric_limits<int32_t>::max()) || std::isnan(src))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<float, int32_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfI4R8)
{
    double src = get_stack_value_at<double>(eval_stack_base, ir->src);
    if (src < static_cast<double>(std::numeric_limits<int32_t>::min()) || src > static_cast<double>(std::numeric_limits<int32_t>::max()) || std::isnan(src))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<double, int32_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfU4I4)
{
    int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
    if (src < 0)
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    uint32_t value = static_cast<uint32_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfU4I8)
{
    int64_t src = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
    if (src < 0 || src > static_cast<int64_t>(std::numeric_limits<uint32_t>::max()))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    uint32_t value = static_cast<uint32_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfU4R4)
{
    float src = get_stack_value_at<float>(eval_stack_base, ir->src);
    if (src < 0.0f || src > static_cast<float>(std::numeric_limits<uint32_t>::max()) || std::isnan(src))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<float, uint32_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfU4R8)
{
    double src = get_stack_value_at<double>(eval_stack_base, ir->src);
    if (src < 0.0 || src > static_cast<double>(std::numeric_limits<uint32_t>::max()) || std::isnan(src))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<double, uint32_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfI8R4)
{
    float src = get_stack_value_at<float>(eval_stack_base, ir->src);
    if (src < static_cast<float>(std::numeric_limits<int64_t>::min()) || src > static_cast<float>(std::numeric_limits<int64_t>::max()) || std::isnan(src))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int64_t>(eval_stack_base, ir->dst, cast_float_to_i64<float, int64_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfI8R8)
{
    double src = get_stack_value_at<double>(eval_stack_base, ir->src);
    if (src < static_cast<double>(std::numeric_limits<int64_t>::min()) || src > static_cast<double>(std::numeric_limits<int64_t>::max()) || std::isnan(src))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int64_t>(eval_stack_base, ir->dst, cast_float_to_i64<double, int64_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfU8I4)
{
    int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
    if (src < 0)
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    uint64_t value = static_cast<uint64_t>(static_cast<uint32_t>(src));
    set_stack_value_at<int64_t>(eval_stack_base, ir->dst, static_cast<int64_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfU8I8)
{
    int64_t src = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
    if (src < 0)
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int64_t>(eval_stack_base, ir->dst, src);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfU8R4)
{
    float src = get_stack_value_at<float>(eval_stack_base, ir->src);
    if (src < 0.0f || src > static_cast<float>(std::numeric_limits<uint64_t>::max()) || std::isnan(src))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int64_t>(eval_stack_base, ir->dst, cast_float_to_i64<float, uint64_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfU8R8)
{
    double src = get_stack_value_at<double>(eval_stack_base, ir->src);
    if (src < 0.0 || src > static_cast<double>(std::numeric_limits<uint64_t>::max()) || std::isnan(src))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int64_t>(eval_stack_base, ir->dst, cast_float_to_i64<double, uint64_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfI1UnI4)
{
    uint32_t src = get_stack_value_at<uint32_t>(eval_stack_base, ir->src);
    if (src > static_cast<uint32_t>(std::numeric_limits<int8_t>::max()))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    int8_t value = static_cast<int8_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfI1UnI8)
{
    uint64_t src = get_stack_value_at<uint64_t>(eval_stack_base, ir->src);
    if (src > static_cast<uint64_t>(std::numeric_limits<int8_t>::max()))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    int8_t value = static_cast<int8_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfI1UnR4)
{
    float src = get_stack_value_at<float>(eval_stack_base, ir->src);
    if (src < 0.0f || src > static_cast<float>(std::numeric_limits<int8_t>::max()) || std::isnan(src))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<float, int8_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfI1UnR8)
{
    double src = get_stack_value_at<double>(eval_stack_base, ir->src);
    if (src < 0.0 || src > static_cast<double>(std::numeric_limits<int8_t>::max()) || std::isnan(src))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<double, int8_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfU1UnI4)
{
    uint32_t src = get_stack_value_at<uint32_t>(eval_stack_base, ir->src);
    if (src > static_cast<uint32_t>(std::numeric_limits<uint8_t>::max()))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    uint8_t value = static_cast<uint8_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfU1UnI8)
{
    uint64_t src = get_stack_value_at<uint64_t>(eval_stack_base, ir->src);
    if (src > static_cast<uint64_t>(std::numeric_limits<uint8_t>::max()))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    uint8_t value = static_cast<uint8_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfU1UnR4)
{
    float src = get_stack_value_at<float>(eval_stack_base, ir->src);
    if (src < 0.0f || src > static_cast<float>(std::numeric_limits<uint8_t>::max()) || std::isnan(src))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<float, uint8_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfU1UnR8)
{
    double src = get_stack_value_at<double>(eval_stack_base, ir->src);
    if (src < 0.0 || src > static_cast<double>(std::numeric_limits<uint8_t>::max()) || std::isnan(src))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<double, uint8_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfI2UnI4)
{
    uint32_t src = get_stack_value_at<uint32_t>(eval_stack_base, ir->src);
    if (src > static_cast<uint32_t>(std::numeric_limits<int16_t>::max()))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    int16_t value = static_cast<int16_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfI2UnI8)
{
    uint64_t src = get_stack_value_at<uint64_t>(eval_stack_base, ir->src);
    if (src > static_cast<uint64_t>(std::numeric_limits<int16_t>::max()))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    int16_t value = static_cast<int16_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfI2UnR4)
{
    float src = get_stack_value_at<float>(eval_stack_base, ir->src);
    if (src < 0.0f || src > static_cast<float>(std::numeric_limits<int16_t>::max()) || std::isnan(src))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<float, int16_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfI2UnR8)
{
    double src = get_stack_value_at<double>(eval_stack_base, ir->src);
    if (src < 0.0 || src > static_cast<double>(std::numeric_limits<int16_t>::max()) || std::isnan(src))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<double, int16_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfU2UnI4)
{
    uint32_t src = get_stack_value_at<uint32_t>(eval_stack_base, ir->src);
    if (src > static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    uint16_t value = static_cast<uint16_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfU2UnI8)
{
    uint64_t src = get_stack_value_at<uint64_t>(eval_stack_base, ir->src);
    if (src > static_cast<uint64_t>(std::numeric_limits<uint16_t>::max()))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    uint16_t value = static_cast<uint16_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfU2UnR4)
{
    float src = get_stack_value_at<float>(eval_stack_base, ir->src);
    if (src < 0.0f || src > static_cast<float>(std::numeric_limits<uint16_t>::max()) || std::isnan(src))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<float, uint16_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfU2UnR8)
{
    double src = get_stack_value_at<double>(eval_stack_base, ir->src);
    if (src < 0.0 || src > static_cast<double>(std::numeric_limits<uint16_t>::max()) || std::isnan(src))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<double, uint16_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfI4UnI4)
{
    uint32_t src = get_stack_value_at<uint32_t>(eval_stack_base, ir->src);
    if (src > static_cast<uint32_t>(std::numeric_limits<int32_t>::max()))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    int32_t value = static_cast<int32_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfI4UnI8)
{
    uint64_t src = get_stack_value_at<uint64_t>(eval_stack_base, ir->src);
    if (src > static_cast<uint64_t>(std::numeric_limits<int32_t>::max()))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    int32_t value = static_cast<int32_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfI4UnR4)
{
    float src = get_stack_value_at<float>(eval_stack_base, ir->src);
    if (src < 0.0f || src > static_cast<float>(std::numeric_limits<int32_t>::max()) || std::isnan(src))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<float, int32_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfI4UnR8)
{
    double src = get_stack_value_at<double>(eval_stack_base, ir->src);
    if (src < 0.0 || src > static_cast<double>(std::numeric_limits<int32_t>::max()) || std::isnan(src))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<double, int32_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfU4UnI8)
{
    uint64_t src = get_stack_value_at<uint64_t>(eval_stack_base, ir->src);
    if (src > static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    uint32_t value = static_cast<uint32_t>(src);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfU4UnR4)
{
    float src = get_stack_value_at<float>(eval_stack_base, ir->src);
    if (src < 0.0f || src > static_cast<float>(std::numeric_limits<uint32_t>::max()) || std::isnan(src))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<float, uint32_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfU4UnR8)
{
    double src = get_stack_value_at<double>(eval_stack_base, ir->src);
    if (src < 0.0 || src > static_cast<double>(std::numeric_limits<uint32_t>::max()) || std::isnan(src))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<double, uint32_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfI8UnI8)
{
    uint64_t src = get_stack_value_at<uint64_t>(eval_stack_base, ir->src);
    if (src > static_cast<uint64_t>(std::numeric_limits<int64_t>::max()))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    int64_t value = static_cast<int64_t>(src);
    set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfI8UnR4)
{
    float src = get_stack_value_at<float>(eval_stack_base, ir->src);
    if (src < 0.0f || src > static_cast<float>(std::numeric_limits<int64_t>::max()) || std::isnan(src))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int64_t>(eval_stack_base, ir->dst, cast_float_to_i64<float, int64_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfI8UnR8)
{
    double src = get_stack_value_at<double>(eval_stack_base, ir->src);
    if (src < 0.0 || src > static_cast<double>(std::numeric_limits<int64_t>::max()) || std::isnan(src))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<int64_t>(eval_stack_base, ir->dst, cast_float_to_i64<double, int64_t>(src));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfU8UnR4)
{
    float src = get_stack_value_at<float>(eval_stack_base, ir->src);
    if (src < 0.0f || src > static_cast<float>(std::numeric_limits<uint64_t>::max()) || std::isnan(src))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<uint64_t>(eval_stack_base, ir->dst, static_cast<uint64_t>(cast_float_to_i64<float, uint64_t>(src)));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ConvOvfU8UnR8)
{
    double src = get_stack_value_at<double>(eval_stack_base, ir->src);
    if (src < 0.0 || src > static_cast<double>(std::numeric_limits<uint64_t>::max()) || std::isnan(src))
    {
        RAISE_RUNTIME_ERROR(RtErr::Overflow);
    }
    set_stack_value_at<uint64_t>(eval_stack_base, ir->dst, static_cast<uint64_t>(cast_float_to_i64<double, uint64_t>(src)));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(InitObjI2Unaligned)
{
    uint8_t* addr = get_stack_value_at<uint8_t*>(eval_stack_base, ir->addr);
    utils::MemOp::write_i16_may_unaligned(addr, 0);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(InitObjI4Unaligned)
{
    uint8_t* addr = get_stack_value_at<uint8_t*>(eval_stack_base, ir->addr);
    utils::MemOp::write_i32_may_unaligned(addr, 0);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(InitObjI8Unaligned)
{
    uint8_t* addr = get_stack_value_at<uint8_t*>(eval_stack_base, ir->addr);
    utils::MemOp::write_i64_may_unaligned(addr, 0);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdfldI1Large)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    const int8_t* field_addr = reinterpret_cast<const int8_t*>(reinterpret_cast<const uint8_t*>(obj) + ir->offset);
    int8_t value = utils::MemOp::load_native(field_addr);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdfldU1Large)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    const uint8_t* field_addr = reinterpret_cast<const uint8_t*>(obj) + ir->offset;
    uint8_t value = utils::MemOp::load_native(field_addr);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdfldI2Large)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    const int16_t* field_addr = reinterpret_cast<const int16_t*>(reinterpret_cast<const uint8_t*>(obj) + ir->offset);
    int16_t value = utils::MemOp::load_native(field_addr);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdfldI2Unaligned)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    const uint8_t* field_addr = reinterpret_cast<const uint8_t*>(obj) + ir->offset;
    int16_t value = utils::MemOp::read_i16_may_unaligned(field_addr);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdfldU2Large)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    const uint16_t* field_addr = reinterpret_cast<const uint16_t*>(reinterpret_cast<const uint8_t*>(obj) + ir->offset);
    uint16_t value = utils::MemOp::load_native(field_addr);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdfldU2Unaligned)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    const uint8_t* field_addr = reinterpret_cast<const uint8_t*>(obj) + ir->offset;
    uint16_t value = utils::MemOp::read_u16_may_unaligned(field_addr);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdfldI4Large)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    const int32_t* field_addr = reinterpret_cast<const int32_t*>(reinterpret_cast<const uint8_t*>(obj) + ir->offset);
    int32_t value = utils::MemOp::load_native(field_addr);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdfldI4Unaligned)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    const uint8_t* field_addr = reinterpret_cast<const uint8_t*>(obj) + ir->offset;
    int32_t value = utils::MemOp::read_i32_may_unaligned(field_addr);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdfldI8Large)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    const int64_t* field_addr = reinterpret_cast<const int64_t*>(reinterpret_cast<const uint8_t*>(obj) + ir->offset);
    int64_t value = utils::MemOp::load_native(field_addr);
    set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdfldI8Unaligned)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    const uint8_t* field_addr = reinterpret_cast<const uint8_t*>(obj) + ir->offset;
    int64_t value = utils::MemOp::read_i64_may_unaligned(field_addr);
    set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdfldAnyLarge)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    const uint8_t* src_addr = reinterpret_cast<const uint8_t*>(obj) + ir->offset;
    RtStackObject* dst = eval_stack_base + ir->dst;
    std::memcpy(dst, src_addr, ir->size);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdvfldI1Large)
{
    const uint8_t* src_addr = reinterpret_cast<const uint8_t*>(eval_stack_base + ir->obj) + ir->offset;
    int8_t value = utils::MemOp::load_native(reinterpret_cast<const int8_t*>(src_addr));
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdvfldU1Large)
{
    const uint8_t* src_addr = reinterpret_cast<const uint8_t*>(eval_stack_base + ir->obj) + ir->offset;
    uint8_t value = utils::MemOp::load_native(reinterpret_cast<const uint8_t*>(src_addr));
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdvfldI2Large)
{
    const uint8_t* src_addr = reinterpret_cast<const uint8_t*>(eval_stack_base + ir->obj) + ir->offset;
    int16_t value = utils::MemOp::load_native(reinterpret_cast<const int16_t*>(src_addr));
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdvfldI2Unaligned)
{
    const uint8_t* src_addr = reinterpret_cast<const uint8_t*>(eval_stack_base + ir->obj) + ir->offset;
    int16_t value;
    std::memcpy(&value, src_addr, sizeof(value));
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdvfldU2Large)
{
    const uint8_t* src_addr = reinterpret_cast<const uint8_t*>(eval_stack_base + ir->obj) + ir->offset;
    uint16_t value = utils::MemOp::load_native(reinterpret_cast<const uint16_t*>(src_addr));
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdvfldU2Unaligned)
{
    const uint8_t* src_addr = reinterpret_cast<const uint8_t*>(eval_stack_base + ir->obj) + ir->offset;
    uint16_t value = utils::MemOp::read_u16_may_unaligned(src_addr);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdvfldI4Large)
{
    const uint8_t* src_addr = reinterpret_cast<const uint8_t*>(eval_stack_base + ir->obj) + ir->offset;
    int32_t value = utils::MemOp::load_native(reinterpret_cast<const int32_t*>(src_addr));
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdvfldI4Unaligned)
{
    const uint8_t* src_addr = reinterpret_cast<const uint8_t*>(eval_stack_base + ir->obj) + ir->offset;
    int32_t value = utils::MemOp::read_i32_may_unaligned(src_addr);
    set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdvfldI8Large)
{
    const uint8_t* src_addr = reinterpret_cast<const uint8_t*>(eval_stack_base + ir->obj) + ir->offset;
    int64_t value = utils::MemOp::load_native(reinterpret_cast<const int64_t*>(src_addr));
    set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdvfldI8Unaligned)
{
    const uint8_t* src_addr = reinterpret_cast<const uint8_t*>(eval_stack_base + ir->obj) + ir->offset;
    int64_t value = utils::MemOp::read_i64_may_unaligned(src_addr);
    set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
}

LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdvfldAnyLarge)
{
    const uint8_t* src_addr = reinterpret_cast<const uint8_t*>(eval_stack_base + ir->obj) + ir->offset;
    RtStackObject* dst = eval_stack_base + ir->dst;
    // may overlap
    std::memmove(dst, src_addr, ir->size);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(LdfldaLarge)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    const uint8_t* field_addr = reinterpret_cast<const uint8_t*>(obj) + ir->offset;
    set_stack_value_at(eval_stack_base, ir->dst, field_addr);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StfldI1Large)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    int8_t value = get_stack_value_at<int8_t>(eval_stack_base, ir->value);
    int8_t* field_addr = reinterpret_cast<int8_t*>(reinterpret_cast<uint8_t*>(obj) + ir->offset);
    utils::MemOp::store_native(field_addr, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StfldI2Large)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    int16_t value = get_stack_value_at<int16_t>(eval_stack_base, ir->value);
    int16_t* field_addr = reinterpret_cast<int16_t*>(reinterpret_cast<uint8_t*>(obj) + ir->offset);
    utils::MemOp::store_native(field_addr, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StfldI2Unaligned)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    int16_t value = get_stack_value_at<int16_t>(eval_stack_base, ir->value);
    uint8_t* field_addr = reinterpret_cast<uint8_t*>(obj) + ir->offset;
    utils::MemOp::write_i16_may_unaligned(field_addr, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StfldI4Large)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->value);
    int32_t* field_addr = reinterpret_cast<int32_t*>(reinterpret_cast<uint8_t*>(obj) + ir->offset);
    utils::MemOp::store_native(field_addr, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StfldI4Unaligned)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->value);
    uint8_t* field_addr = reinterpret_cast<uint8_t*>(obj) + ir->offset;
    utils::MemOp::write_i32_may_unaligned(field_addr, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StfldI8Large)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    int64_t value = get_stack_value_at<int64_t>(eval_stack_base, ir->value);
    int64_t* field_addr = reinterpret_cast<int64_t*>(reinterpret_cast<uint8_t*>(obj) + ir->offset);
    utils::MemOp::store_native(field_addr, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StfldI8Unaligned)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    int64_t value = get_stack_value_at<int64_t>(eval_stack_base, ir->value);
    uint8_t* field_addr = reinterpret_cast<uint8_t*>(obj) + ir->offset;
    utils::MemOp::write_i64_may_unaligned(field_addr, value);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(StfldAnyLarge)
{
    vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
    if (!obj)
    {
        RAISE_RUNTIME_ERROR(RtErr::NullReference);
    }
    const uint8_t* src_addr = reinterpret_cast<const uint8_t*>(eval_stack_base + ir->value);
    uint8_t* dst_addr = reinterpret_cast<uint8_t*>(obj) + ir->offset;
    std::memmove(dst_addr, src_addr, ir->size);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(ProfileAddCost)
{
#if LEANCLR_PGO_PROFILE
    profile::Profile::add_cost(frame->method, static_cast<uint64_t>(ir->cost));
#endif
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(Illegal)
{
    assert(false && "never reach here");
    RAISE_RUNTIME_ERROR(RtErr::ExecutionEngine);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(Nop)
{
    // do nothing
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN(Arglist)
{
    assert(false && "Not implemented");
    RAISE_RUNTIME_ERROR(RtErr::ExecutionEngine);
}
LEANCLR_CASE_END()

LEANCLR_CASE_BEGIN_LITE(RetNop)
{
    LEAVE_FRAME();
}
LEANCLR_CASE_END_LITE()
