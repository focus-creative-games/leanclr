#include "bcl/adapter.h"
#include "icalls/internal_call_stubs.h"
#include "intrinsics/intrinsic_stubs.h"
#include "vm/internal_calls.h"
#include "vm/intrinsics.h"

namespace leanclr
{
namespace bcl
{
void Adapter::register_internal_calls()
{
    utils::Vector<vm::InternalCallEntry> entries;
    icalls::InternalCallStubs::get_internal_call_entries(entries);
    for (auto& entry : entries)
        vm::InternalCalls::register_internal_call(entry.name, entry.func, entry.invoker);

    utils::Vector<vm::NewobjInternalCallEntry> newobjEntries;
    icalls::InternalCallStubs::get_newobj_internal_call_entries(newobjEntries);
    for (auto& entry : newobjEntries)
        vm::InternalCalls::register_newobj_internal_call(entry.name, entry.invoker);
}

void Adapter::register_intrinsics()
{
    utils::Vector<vm::IntrinsicEntry> intrinsicEntries;
    intrinsics::IntrinsicStubs::get_intrinsic_entries(intrinsicEntries);
    for (auto entry : intrinsicEntries)
        vm::Intrinsics::register_intrinsic(entry.name, entry.func, entry.invoker);
    utils::Vector<vm::NewobjIntrinsicEntry> newobjIntrinsicEntries;
    intrinsics::IntrinsicStubs::get_newobj_intrinsic_entries(newobjIntrinsicEntries);
    for (auto entry : newobjIntrinsicEntries)
        vm::Intrinsics::register_newobj_intrinsic(entry.name, entry.invoker);
}
} // namespace bcl
} // namespace leanclr
