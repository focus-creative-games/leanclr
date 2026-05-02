#include "system_reflection_assemblyname.h"

#include "vm/reflection.h"
#include "vm/type.h"
#include <cstring>

namespace leanclr
{
namespace icalls
{

RtResult<bool> SystemReflectionAssemblyName::parse_assembly_name(intptr_t name_cstr, metadata::RtMonoAssemblyName* aname, bool* is_version_defined,
                                                                 bool* is_token_defined) noexcept
{
    const char* name_str = reinterpret_cast<const char*>(name_cstr);
    size_t name_len = std::strlen(name_str);

    RET_ERR_ON_FAIL(vm::Type::parse_assembly_name(name_str, name_len, aname, is_version_defined, is_token_defined));

    RET_OK(true);
}

/// @icall: System.Reflection.AssemblyName::ParseAssemblyName(System.IntPtr,Mono.MonoAssemblyName&,System.Boolean&,System.Boolean&)
static RtResultVoid parse_assembly_name_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto blob = EvalStackOp::get_param<intptr_t>(params, 0);
    auto aname = EvalStackOp::get_param<metadata::RtMonoAssemblyName*>(params, 1);
    auto is_ver_ptr = EvalStackOp::get_param<bool*>(params, 2);
    auto is_tok_ptr = EvalStackOp::get_param<bool*>(params, 3);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemReflectionAssemblyName::parse_assembly_name(blob, aname, is_ver_ptr, is_tok_ptr));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

RtResultVoid SystemReflectionAssemblyName::get_public_token(const uint8_t* public_key, uint8_t* public_token, int32_t len) noexcept
{
    (void)public_key;
    (void)public_token;
    (void)len;
    // TODO: implement public key token generation
    RETURN_NOT_IMPLEMENTED_ERROR();
}

/// @icall: System.Reflection.AssemblyName::get_public_token(System.Byte*,System.Byte*,System.Int32)
static RtResultVoid get_public_token_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                             const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    auto pk = EvalStackOp::get_param<const uint8_t*>(params, 0);
    auto pkt = EvalStackOp::get_param<uint8_t*>(params, 1);
    auto len = EvalStackOp::get_param<int32_t>(params, 2);

    RET_ERR_ON_FAIL(SystemReflectionAssemblyName::get_public_token(pk, pkt, len));
    RET_VOID_OK();
}

RtResult<metadata::RtMonoAssemblyName*> SystemReflectionAssemblyName::get_native_name(metadata::RtAssembly* ass) noexcept
{
    return vm::Reflection::get_assembly_name_object(ass);
}

/// @icall: System.Reflection.AssemblyName::GetNativeName(System.IntPtr)
static RtResultVoid get_native_name_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                            const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto h = EvalStackOp::get_param<metadata::RtAssembly*>(params, 0);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtMonoAssemblyName*, result, SystemReflectionAssemblyName::get_native_name(h));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

utils::Span<vm::InternalCallEntry> SystemReflectionAssemblyName::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.Reflection.AssemblyName::ParseAssemblyName(System.IntPtr,Mono.MonoAssemblyName&,System.Boolean&,System.Boolean&)",
         (vm::InternalCallFunction)&SystemReflectionAssemblyName::parse_assembly_name, parse_assembly_name_invoker},
        {"System.Reflection.AssemblyName::get_public_token(System.Byte*,System.Byte*,System.Int32)",
         (vm::InternalCallFunction)&SystemReflectionAssemblyName::get_public_token, get_public_token_invoker},
        {"System.Reflection.AssemblyName::GetNativeName(System.IntPtr)", (vm::InternalCallFunction)&SystemReflectionAssemblyName::get_native_name,
         get_native_name_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr
