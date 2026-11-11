#pragma once

#include <cstdint>
#include <cstring>
#include <type_traits>
#include "core/rt_result.h"

#if defined(_MSC_VER)
#define LEANCLR_CODEGEN_NOINLINE __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
#define LEANCLR_CODEGEN_NOINLINE __attribute__((noinline))
#else
#define LEANCLR_CODEGEN_NOINLINE
#endif

namespace leanclr
{
namespace codegen
{
// Internal opt-in AOT ABI. Keep these types trivially copyable in every build,
// including Debug: a diagnostic destructor would change the function ABI.
// Runtime boundaries reconstruct the ordinary checked Result types.
class AotVoidResult
{
    core::RtErr _error;

  public:
    AotVoidResult(core::Unit) noexcept : _error(core::RtErr::None)
    {
    }

    AotVoidResult(core::RtErr error) noexcept : _error(error)
    {
        assert(error != core::RtErr::None);
    }

    AotVoidResult(core::ResultVoid&& result) noexcept : _error(result.is_ok() ? core::RtErr::None : result.unwrap_err())
    {
    }

    bool is_ok() const noexcept
    {
        return _error == core::RtErr::None;
    }

    bool is_err() const noexcept
    {
        return !is_ok();
    }

    core::RtErr unwrap_err() const noexcept
    {
        assert(is_err());
        return _error;
    }

    operator core::ResultVoid() const noexcept
    {
        if (is_ok())
            return core::Unit{};
        return _error;
    }
};

template <typename T>
class AotPointerResult
{
    static_assert(std::is_pointer<T>::value, "AOT tagged result requires a pointer");
    static_assert(core::pointer_alignment<typename std::remove_pointer<T>::type>::value >= 2, "AOT tagged result requires a two-byte alignment contract");
    uintptr_t _bits;

  public:
    AotPointerResult(T value) noexcept : _bits(reinterpret_cast<uintptr_t>(value))
    {
        assert((_bits & 1) == 0);
    }

    AotPointerResult(core::RtErr error) noexcept : _bits((static_cast<uintptr_t>(error) << 1) | 1)
    {
        assert(error != core::RtErr::None);
    }

    AotPointerResult(core::Result<T>&& result) noexcept
        : _bits(result.is_ok() ? reinterpret_cast<uintptr_t>(result.unwrap()) : (static_cast<uintptr_t>(result.unwrap_err()) << 1) | 1)
    {
    }

    bool is_ok() const noexcept
    {
        return (_bits & 1) == 0;
    }

    bool is_err() const noexcept
    {
        return !is_ok();
    }

    T unwrap() const noexcept
    {
        assert(is_ok());
        return reinterpret_cast<T>(_bits);
    }

    core::RtErr unwrap_err() const noexcept
    {
        assert(is_err());
        return static_cast<core::RtErr>(_bits >> 1);
    }

    operator core::Result<T>() const noexcept
    {
        if (is_ok())
            return unwrap();
        return unwrap_err();
    }
};

// Full 32-bit payload in the low word; zero high word means success, otherwise
// the high word contains RtErr. No payload value is reserved as a sentinel.
template <typename T>
class AotInt32Result
{
    static_assert(std::is_same<T, int32_t>::value || std::is_same<T, uint32_t>::value, "AOT packed result supports only int32/uint32");
    uint64_t _bits;

  public:
    AotInt32Result(T value) noexcept : _bits(static_cast<uint32_t>(value))
    {
    }

    AotInt32Result(core::RtErr error) noexcept : _bits(static_cast<uint64_t>(error) << 32)
    {
        assert(error != core::RtErr::None);
    }

    AotInt32Result(core::Result<T>&& result) noexcept
        : _bits(result.is_ok() ? static_cast<uint32_t>(result.unwrap()) : static_cast<uint64_t>(result.unwrap_err()) << 32)
    {
        assert(result.is_ok() || result.unwrap_err() != core::RtErr::None);
    }

    bool is_ok() const noexcept
    {
        return (_bits >> 32) == 0;
    }

    bool is_err() const noexcept
    {
        return !is_ok();
    }

    T unwrap() const noexcept
    {
        assert(is_ok());
        const uint32_t bits = static_cast<uint32_t>(_bits);
        T value;
        std::memcpy(&value, &bits, sizeof(value));
        return value;
    }

    core::RtErr unwrap_err() const noexcept
    {
        assert(is_err());
        return static_cast<core::RtErr>(_bits >> 32);
    }

    operator core::Result<T>() const noexcept
    {
        if (is_ok())
            return unwrap();
        return unwrap_err();
    }
};

static_assert(std::is_trivially_copyable<AotInt32Result<int32_t>>::value, "AOT int32 ABI must stay trivial");
static_assert(sizeof(AotInt32Result<int32_t>) == sizeof(uint64_t), "AOT int32 result must be one 64-bit word");
static_assert(std::is_trivially_copyable<AotInt32Result<uint32_t>>::value, "AOT uint32 ABI must stay trivial");
static_assert(sizeof(AotInt32Result<uint32_t>) == sizeof(uint64_t), "AOT uint32 result must be one 64-bit word");
static_assert(std::is_trivially_copyable<AotVoidResult>::value, "AOT void ABI must stay trivial");
static_assert(sizeof(AotVoidResult) == sizeof(core::RtErr), "AOT void result must contain only its error code");
} // namespace codegen
} // namespace leanclr
