#include "bcl/adapter.h"
#include "pinvokes/coreclr_qcall.h"
#include "icalls/internal_call_stubs.h"
#include "intrinsics/intrinsic_stubs.h"
#include "vm/internal_calls.h"
#include "vm/intrinsics.h"
#include "vm/method.h"
#include "metadata/module_def.h"
#include "intrinsics/system_numerics_vector.h"

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

static RtResultVoid hardware_unavailable(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject*,
                                         interp::RtStackObject* ret) noexcept
{
    interp::EvalStackOp::set_return(ret, int32_t{0});
    RET_VOID_OK();
}

static RtResultVoid hardware_not_supported(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject*,
                                           interp::RtStackObject*) noexcept
{
    RET_ERR(RtErr::NotSupported);
}

const vm::IntrinsicRegistry* Adapter::resolve_intrinsic(const metadata::RtMethodInfo* method)
{
    if (!method || !method->parent->image->is_corlib())
        return nullptr;
    const auto* owner = method->parent;
    while (owner->declaring_class)
        owner = owner->declaring_class;
    const char* ns = owner->namespaze;
    if (std::strcmp(ns, "System.Runtime.Intrinsics.X86") != 0 && std::strcmp(ns, "System.Runtime.Intrinsics.Arm") != 0 &&
        std::strcmp(ns, "System.Runtime.Intrinsics.Wasm") != 0)
        return nullptr;
    static const vm::IntrinsicRegistry unavailable{reinterpret_cast<vm::IntrinsicFunction>(intrinsics::SystemNumericsVector::get_is_hardware_accelerated),
                                                   hardware_unavailable};
    static const vm::IntrinsicRegistry unsupported{nullptr, hardware_not_supported};
    if (std::strcmp(method->name, "get_IsSupported") == 0 && vm::Method::is_static(method) && method->parameter_count == 0 &&
        method->return_type->ele_type == metadata::RtElementType::Boolean)
        return &unavailable;
    return &unsupported;
}

void Adapter::register_pinvokes()
{
    pinvokes::register_coreclr_qcall_pinvokes();
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
