#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemSecurityCryptographyRNGCryptoServiceProvider
{
  public:
    // @icall: System.Security.Cryptography.RNGCryptoServiceProvider::RngOpen
    static RtResult<bool> rng_open();

    // @icall: System.Security.Cryptography.RNGCryptoServiceProvider::RngInitialize(System.Byte*,System.IntPtr)
    static RtResult<intptr_t> rng_initialize(const uint8_t* seed, intptr_t seed_len);

    // @icall: System.Security.Cryptography.RNGCryptoServiceProvider::RngGetBytes(System.IntPtr,System.Byte*,System.IntPtr)
    static RtResult<intptr_t> rng_get_bytes(intptr_t handle, uint8_t* buffer, intptr_t length);

    // @icall: System.Security.Cryptography.RNGCryptoServiceProvider::RngClose(System.IntPtr)
    static RtResultVoid rng_close(intptr_t handle);

    static utils::Span<vm::InternalCallEntry> get_internal_call_entries();
};

} // namespace icalls
} // namespace leanclr
