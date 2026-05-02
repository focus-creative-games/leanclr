#include "system_reflection_customattributedata.h"

#include "vm/customattribute.h"
#include "utils/binary_reader.h"

namespace leanclr
{
namespace icalls
{

RtResultVoid SystemReflectionCustomAttributeData::resolve_arguments_internal(vm::RtReflectionMethod* ctor, vm::RtReflectionAssembly* ctor_assembly,
                                                                             intptr_t data, uint32_t data_length, vm::RtArray** typed_arg_arr_ptr,
                                                                             vm::RtArray** named_arg_arr_ptr) noexcept
{
    const void* data_ptr = reinterpret_cast<const void*>(data);
    utils::BinaryReader reader(data_ptr, static_cast<size_t>(data_length));

    RET_ERR_ON_FAIL(
        vm::CustomAttribute::resolve_customattribute_data_arguments(&reader, ctor_assembly->assembly->mod, ctor->method, typed_arg_arr_ptr, named_arg_arr_ptr));
    RET_VOID_OK();
}

/// @icall:
/// System.Reflection.CustomAttributeData::ResolveArgumentsInternal(System.Reflection.ConstructorInfo,System.Reflection.Assembly,System.IntPtr,System.UInt32,System.Object[]&,System.Object[]&)
static RtResultVoid resolve_arguments_internal_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                       const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    auto ctor = EvalStackOp::get_param<vm::RtReflectionMethod*>(params, 0);
    auto ctor_assembly = EvalStackOp::get_param<vm::RtReflectionAssembly*>(params, 1);
    auto data = EvalStackOp::get_param<intptr_t>(params, 2);
    auto data_length = EvalStackOp::get_param<uint32_t>(params, 3);
    auto typed_arg_arr_ptr = EvalStackOp::get_param<vm::RtArray**>(params, 4);
    auto named_arg_arr_ptr = EvalStackOp::get_param<vm::RtArray**>(params, 5);

    RET_ERR_ON_FAIL(
        SystemReflectionCustomAttributeData::resolve_arguments_internal(ctor, ctor_assembly, data, data_length, typed_arg_arr_ptr, named_arg_arr_ptr));
    RET_VOID_OK();
}

utils::Span<vm::InternalCallEntry> SystemReflectionCustomAttributeData::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.Reflection.CustomAttributeData::ResolveArgumentsInternal(System.Reflection.ConstructorInfo,System.Reflection.Assembly,System.IntPtr,System."
         "UInt32,System.Object[]&,System.Object[]&)",
         (vm::InternalCallFunction)&SystemReflectionCustomAttributeData::resolve_arguments_internal, resolve_arguments_internal_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr
