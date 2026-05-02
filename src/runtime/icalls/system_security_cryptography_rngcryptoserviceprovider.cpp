#include "system_security_cryptography_rngcryptoserviceprovider.h"

#include "misc/rng_crypto_service_provider.h"

namespace leanclr
{
namespace icalls
{

using namespace interp;
using namespace metadata;

// @icall: System.Security.Cryptography.RNGCryptoServiceProvider::RngOpen
RtResult<bool> SystemSecurityCryptographyRNGCryptoServiceProvider::rng_open() noexcept
{
    RET_OK(misc::RngCryptoServiceProvider::open());
}

static RtResultVoid rng_open_invoker(RtManagedMethodPointer /*method_pointer*/, const RtMethodInfo* /*method*/, const interp::RtStackObject* /*params*/,
                                     interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemSecurityCryptographyRNGCryptoServiceProvider::rng_open());
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

// @icall: System.Security.Cryptography.RNGCryptoServiceProvider::RngInitialize(System.Byte*,System.IntPtr)
RtResult<intptr_t> SystemSecurityCryptographyRNGCryptoServiceProvider::rng_initialize(const uint8_t* seed, intptr_t seed_len) noexcept
{
    RET_OK(misc::RngCryptoServiceProvider::initialize(seed, seed_len));
}

static RtResultVoid rng_initialize_invoker(RtManagedMethodPointer /*method_pointer*/, const RtMethodInfo* /*method*/, const interp::RtStackObject* params,
                                           interp::RtStackObject* ret) noexcept
{
    const uint8_t* seed = EvalStackOp::get_param<const uint8_t*>(params, 0);
    intptr_t seed_len = EvalStackOp::get_param<intptr_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(intptr_t, handle, SystemSecurityCryptographyRNGCryptoServiceProvider::rng_initialize(seed, seed_len));
    EvalStackOp::set_return(ret, handle);
    RET_VOID_OK();
}

// @icall: System.Security.Cryptography.RNGCryptoServiceProvider::RngGetBytes(System.IntPtr,System.Byte*,System.IntPtr)
RtResult<intptr_t> SystemSecurityCryptographyRNGCryptoServiceProvider::rng_get_bytes(intptr_t handle, uint8_t* buffer, intptr_t length) noexcept
{
    RET_OK(misc::RngCryptoServiceProvider::get_bytes(handle, buffer, length));
}

static RtResultVoid rng_get_bytes_invoker(RtManagedMethodPointer /*method_pointer*/, const RtMethodInfo* /*method*/, const interp::RtStackObject* params,
                                          interp::RtStackObject* ret) noexcept
{
    intptr_t handle = EvalStackOp::get_param<intptr_t>(params, 0);
    uint8_t* buf = EvalStackOp::get_param<uint8_t*>(params, 1);
    intptr_t len = EvalStackOp::get_param<intptr_t>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(intptr_t, result_handle, SystemSecurityCryptographyRNGCryptoServiceProvider::rng_get_bytes(handle, buf, len));
    EvalStackOp::set_return(ret, result_handle);
    RET_VOID_OK();
}

// @icall: System.Security.Cryptography.RNGCryptoServiceProvider::RngClose(System.IntPtr)
RtResultVoid SystemSecurityCryptographyRNGCryptoServiceProvider::rng_close(intptr_t handle) noexcept
{
    misc::RngCryptoServiceProvider::close(handle);
    RET_VOID_OK();
}

static RtResultVoid rng_close_invoker(RtManagedMethodPointer /*method_pointer*/, const RtMethodInfo* /*method*/, const interp::RtStackObject* params,
                                      interp::RtStackObject* /*ret*/) noexcept
{
    intptr_t handle = EvalStackOp::get_param<intptr_t>(params, 0);
    return SystemSecurityCryptographyRNGCryptoServiceProvider::rng_close(handle);
}

utils::Span<vm::InternalCallEntry> SystemSecurityCryptographyRNGCryptoServiceProvider::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.Security.Cryptography.RNGCryptoServiceProvider::RngOpen",
         (vm::InternalCallFunction)&SystemSecurityCryptographyRNGCryptoServiceProvider::rng_open, rng_open_invoker},
        {"System.Security.Cryptography.RNGCryptoServiceProvider::RngInitialize(System.Byte*,System.IntPtr)",
         (vm::InternalCallFunction)&SystemSecurityCryptographyRNGCryptoServiceProvider::rng_initialize, rng_initialize_invoker},
        {"System.Security.Cryptography.RNGCryptoServiceProvider::RngGetBytes(System.IntPtr,System.Byte*,System.IntPtr)",
         (vm::InternalCallFunction)&SystemSecurityCryptographyRNGCryptoServiceProvider::rng_get_bytes, rng_get_bytes_invoker},
        {"System.Security.Cryptography.RNGCryptoServiceProvider::RngClose(System.IntPtr)",
         (vm::InternalCallFunction)&SystemSecurityCryptographyRNGCryptoServiceProvider::rng_close, rng_close_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr
