#pragma once
#include <cassert>
#include <new>
#include <type_traits>
#include <utility>
#include "rt_err.h"

namespace leanclr
{
namespace core
{

struct Unit
{
};

template <typename T>
class Result
{
    typedef typename std::aligned_storage<(sizeof(T) > sizeof(RtErr) ? sizeof(T) : sizeof(RtErr)),
                                          (alignof(T) > alignof(RtErr) ? alignof(T) : alignof(RtErr))>::type StorageType;

    StorageType _data;
    bool _is_ok;
#if LEANCLR_DEBUG
    mutable bool _checked = false;
#endif

    T* ok_ptr()
    {
        return reinterpret_cast<T*>(&_data);
    }

    const T* ok_ptr() const
    {
        return reinterpret_cast<const T*>(&_data);
    }

    RtErr* err_ptr()
    {
        return reinterpret_cast<RtErr*>(&_data);
    }

    const RtErr* err_ptr() const
    {
        return reinterpret_cast<const RtErr*>(&_data);
    }

    void destroy_active()
    {
        if (_is_ok)
            ok_ptr()->~T();
        else
            err_ptr()->~RtErr();
    }

  public:
    Result(const T& value) noexcept : _is_ok(true)
    {
        new (&_data) T(value);
    }
    Result(T&& value) noexcept : _is_ok(true)
    {
        new (&_data) T(std::move(value));
    }

    Result(const RtErr& error) noexcept : _is_ok(false)
    {
        new (&_data) RtErr(error);
    }

    Result(RtErr&& error) noexcept : _is_ok(false)
    {
        new (&_data) RtErr(std::move(error));
    }

    Result(const Result<T>& other) = delete;
    Result<T>& operator=(const Result<T>& other) = delete;

    Result(Result<T>&& other) noexcept : _is_ok(other._is_ok)
    {
        if (_is_ok)
            new (&_data) T(std::move(*other.ok_ptr()));
        else
            new (&_data) RtErr(std::move(*other.err_ptr()));

#if LEANCLR_DEBUG
        _checked = other._checked;
        other._checked = true;
#endif
    }

    static Result<T> Ok(const T& value)
    {
        return Result<T>(value);
    }

    static Result<T> Err(const RtErr& error)
    {
        return Result<T>(error);
    }

#if LEANCLR_DEBUG
    ~Result()
    {
        assert(_checked && "Result value was not checked before destruction");
        destroy_active();
    }
#else
    ~Result()
    {
        destroy_active();
    }
#endif

    bool is_ok() const
    {
#if LEANCLR_DEBUG
        _checked = true;
#endif
        return _is_ok;
    }

    bool is_err() const
    {
#if LEANCLR_DEBUG
        _checked = true;
#endif
        return !_is_ok;
    }

    T& unwrap()
    {
        assert(is_ok() && "Result::unwrap() called on error value");
        return *ok_ptr();
    }

    const T& unwrap() const
    {
        assert(is_ok() && "Result::unwrap() called on error value");
        return *ok_ptr();
    }

    // T& unwrap_ref()
    // {
    //     assert(is_ok() && "Result::unwrap_ref() called on error value");
    //     return std::get<T>(data);
    // }

    RtErr unwrap_err() const
    {
        assert(is_err() && "Result::unwrap_err() called on ok value");
        return *err_ptr();
    }

    template <typename F>
    auto map_err(F f) -> Result<T>
    {
        return is_err() ? Result<T>::Err(f(unwrap_err())) : Result<T>::Ok(unwrap());
    }

    template <typename F>
    auto map(F f) -> Result<decltype(f(std::declval<T>()))>
    {
        typedef decltype(f(std::declval<T>())) NewValueType;
        if (is_ok())
            return Result<NewValueType>::Ok(f(unwrap()));
        return Result<NewValueType>::Err(unwrap_err());
    }

    template <typename U>
    Result<U> cast()
    {
        // Use tag dispatch so only the well-formed branch is instantiated.
        // `if constexpr` would be cleaner but requires C++17.
        return cast_impl<U>(std::integral_constant < bool, std::is_same<U, RtErr>::value&& std::is_same<T, RtErr>::value > {});
    }

  private:
    // Fast path: when T == U == RtErr the storage is already the desired Result<RtErr>,
    // so we can just move ourselves to avoid touching the payload.
    template <typename U>
    Result<U> cast_impl(std::true_type)
    {
        return std::move(*this);
    }

    // General path: rebuild a Result<U> from the current ok/err state.
    template <typename U>
    Result<U> cast_impl(std::false_type)
    {
        if (is_ok())
            return Result<U>::Ok((U)(unwrap()));
        return Result<U>::Err(unwrap_err());
    }
};

class ResultVoid
{
    RtErr _err;
    bool _is_ok;
#if LEANCLR_DEBUG
    mutable bool _checked = false;
#endif

  public:
    ResultVoid(const Unit& value) noexcept : _is_ok(true)
    {
    }

    ResultVoid(const RtErr& error) noexcept : _err(error), _is_ok(false)
    {
    }

    ResultVoid(const ResultVoid& other) = delete;
    ResultVoid& operator=(const ResultVoid& other) = delete;

    ResultVoid(ResultVoid&& other) noexcept : _err(other._err), _is_ok(other._is_ok)
    {
#if LEANCLR_DEBUG
        _checked = other._checked;
        other._checked = true;
#endif
    }

#if LEANCLR_DEBUG
    ~ResultVoid()
    {
        assert(_checked && "Result value was not checked before destruction");
    }
#endif

    bool is_ok() const
    {
#if LEANCLR_DEBUG
        _checked = true;
#endif
        return _is_ok;
    }

    bool is_err() const
    {
#if LEANCLR_DEBUG
        _checked = true;
#endif
        return !_is_ok;
    }

    RtErr unwrap_err()
    {
        assert(is_err() && "Result::unwrap_err() called on ok value");
        return _err;
    }
};

template <typename T>
struct function_return;
template <typename R, typename... Args>
struct function_return<R(Args...)>
{
    typedef R type;
};

#if defined(__cpp_noexcept_function_type) && (__cpp_noexcept_function_type >= 201510L)
template <typename R, typename... Args>
struct function_return<R(Args...) noexcept>
{
    typedef R type;
};
#endif

template <typename R, typename... Args>
struct function_return<R (*)(Args...)>
{
    typedef R type;
};

#if defined(__cpp_noexcept_function_type) && (__cpp_noexcept_function_type >= 201510L)
template <typename R, typename... Args>
struct function_return<R (*)(Args...) noexcept>
{
    typedef R type;
};
#endif

} // namespace core
} // namespace leanclr

#define RET_ERR_ON_FAIL(expr)         \
    do                                \
    {                                 \
        auto&& _res = (expr);         \
        if (_res.is_err())            \
            return _res.unwrap_err(); \
    } while (0)

#define UNWRAP_OR_RET_ERR_ON_FAIL(var, expr) \
    do                                       \
    {                                        \
        auto&& _res = (expr);                \
        if (_res.is_err())                   \
        {                                    \
            return _res.unwrap_err();        \
        }                                    \
        var = _res.unwrap();                 \
    } while (0)

#define DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(type, var, expr) \
    type var;                                                    \
    UNWRAP_OR_RET_ERR_ON_FAIL(var, expr)

#define DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL2(type, var, unique_temp_name) \
    if (unique_temp_name.is_err())                                            \
    {                                                                         \
        return unique_temp_name.unwrap_err();                                 \
    }                                                                         \
    type var = unique_temp_name.unwrap();

#define DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(type, var, expr) \
    auto&& __opt##var = (expr);                                   \
    if (__opt##var.is_err())                                      \
    {                                                             \
        return __opt##var.unwrap_err();                           \
    }                                                             \
    type& var = __opt##var.unwrap();

#define RET_OK(value) \
    do                \
    {                 \
        return value; \
    } while (0)

#define RET_ERR(err) \
    do               \
    {                \
        return err;  \
    } while (0)

#define RET_ASSERT_ERR(err)    \
    do                         \
    {                          \
        assert(false && #err); \
        return err;            \
    } while (0)

#define RET_ERR_ON_FALSE(expr, err) \
    do                              \
    {                               \
        if (!(expr))                \
            return err;             \
    } while (0)

#define RET_VOID_OK()  \
    do                 \
    {                  \
        return Unit{}; \
    } while (0)
