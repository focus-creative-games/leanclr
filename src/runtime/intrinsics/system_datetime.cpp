#include "system_datetime.h"

#include "interp/eval_stack_op.h"
#include "platform/rt_time.h"

namespace leanclr
{
namespace intrinsics
{

RtResult<uint64_t> SystemDateTime::utc_now() noexcept
{
    constexpr uint64_t UNIX_EPOCH_TICKS = 621355968000000000ULL;
    constexpr uint64_t KIND_UTC = 0x4000000000000000ULL;
    uint64_t ticks = UNIX_EPOCH_TICKS + static_cast<uint64_t>(os::Time::get_ticks_100nanos());
    RET_OK(ticks | KIND_UTC);
}

/// @intrinsic: System.DateTime::get_UtcNow()
static RtResultVoid utc_now_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                    const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)params;

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint64_t, date_data, SystemDateTime::utc_now());
    interp::EvalStackOp::set_return(ret, date_data);
    RET_VOID_OK();
}

static vm::IntrinsicEntry s_intrinsic_entries_system_datetime[] = {
    {"System.DateTime::get_UtcNow()", (vm::IntrinsicFunction)&SystemDateTime::utc_now, utc_now_invoker},
};

utils::Span<vm::IntrinsicEntry> SystemDateTime::get_intrinsic_entries() noexcept
{
    return utils::Span<vm::IntrinsicEntry>(s_intrinsic_entries_system_datetime,
                                           sizeof(s_intrinsic_entries_system_datetime) / sizeof(vm::IntrinsicEntry));
}

} // namespace intrinsics
} // namespace leanclr
