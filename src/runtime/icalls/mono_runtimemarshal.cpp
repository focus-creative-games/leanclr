#include "mono_runtimemarshal.h"

#include "icall_base.h"
#include "alloc/general_allocation.h"

namespace leanclr
{
namespace icalls
{

RtResultVoid MonoRuntimeMarshal::free_assembly_name(metadata::RtMonoAssemblyName* aname, bool free_struct) noexcept
{
    alloc::GeneralAllocation::free(const_cast<char*>(aname->name));
    alloc::GeneralAllocation::free(const_cast<char*>(aname->culture));
    if (free_struct)
    {
        // should match allocation in System.Reflection.Assembly::InternalGetReferencedAssemblies(System.Reflection.Assembly)
        alloc::GeneralAllocation::free(aname);
    }
    RET_VOID_OK();
}

/// @icall: Mono.RuntimeMarshal::FreeAssemblyName(Mono.MonoAssemblyName&,System.Boolean)
static RtResultVoid free_assembly_name_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)ret;
    auto aname = EvalStackOp::get_param<metadata::RtMonoAssemblyName*>(params, 0);
    auto free_struct_int = EvalStackOp::get_param<int32_t>(params, 1);
    bool free_struct = (free_struct_int != 0);
    RET_ERR_ON_FAIL(MonoRuntimeMarshal::free_assembly_name(aname, free_struct));
    RET_VOID_OK();
}

utils::Span<vm::InternalCallEntry> MonoRuntimeMarshal::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"Mono.RuntimeMarshal::FreeAssemblyName(Mono.MonoAssemblyName&,System.Boolean)", (vm::InternalCallFunction)&MonoRuntimeMarshal::free_assembly_name,
         free_assembly_name_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr
