#pragma once
#include <cassert>
#include <variant>

namespace leanclr
{
namespace core
{

struct Unit
{
};

template <typename T, typename E>
class Result
{
    std::variant<T, E> _data;
#ifndef NDEBUG
    mutable bool _checked = false;
#endif

  public:
    Result(const T& value) noexcept : _data(value)
    {
    }
    Result(T&& value) noexcept : _data(std::move(value))
    {
    }

    Result(const E& error) noexcept : _data(error)
    {
    }

    Result(E&& error) noexcept : _data(std::move(error))
    {
    }

    Result(const Result<T, E>& other) = delete;
    Result<T, E>& operator=(const Result<T, E>& other) = delete;

    Result(Result<T, E>&& other) noexcept : _data(std::move(other._data))
    {
#ifndef NDEBUG
        _checked = other._checked;
        other._checked = true;
#endif // !NDEBUG
    }

    static Result<T, E> Ok(const T& value)
    {
        return Result<T, E>(value);
    }

    static Result<T, E> Err(const E& error)
    {
        return Result<T, E>(error);
    }

#ifndef NDEBUG
    ~Result()
    {
        assert(_checked && "Result value was not checked before destruction");
    }
#endif

    bool is_ok() const
    {
#ifndef NDEBUG
        _checked = true;
#endif
        return std::holds_alternative<T>(_data);
    }

    bool is_err() const
    {
#ifndef NDEBUG
        _checked = true;
#endif
        return std::holds_alternative<E>(_data);
    }

    T& unwrap()
    {
        assert(is_ok() && "Result::unwrap() called on error value");
        return std::get<T>(_data);
    }

    const T& unwrap() const
    {
        assert(is_ok() && "Result::unwrap() called on error value");
        return std::get<T>(_data);
    }

    // T& unwrap_ref()
    // {
    //     assert(is_ok() && "Result::unwrap_ref() called on error value");
    //     return std::get<T>(data);
    // }

    E unwrap_err() const
    {
        assert(is_err() && "Result::unwrap_err() called on ok value");
        return std::get<E>(_data);
    }

    template <typename F>
    auto map_err(F f) -> Result<T, decltype(f(std::declval<E>()))>
    {
        return is_err() ? Result<T, F>::Err(f(unwrap_err())) : Result<T, F>::Ok(unwrap());
    }

    template <typename F>
    auto map(F f) -> Result<decltype(f(std::get<T>(_data))), E>
    {
        if (is_ok())
            return Result(f(unwrap()));
        return Result(unwrap_err());
    }

    template <typename U>
    Result<U, E> cast()
    {
        if (is_ok())
            return Result<U, E>::Ok((U)(unwrap()));
        return Result<U, E>::Err(unwrap_err());
    }
};

template <typename E>
class ResultVoid
{
    E _err;
    bool _is_ok;
#ifndef NDEBUG
    mutable bool _checked = false;
#endif

  public:
    ResultVoid(const Unit& value) noexcept : _is_ok(true)
    {
    }

    ResultVoid(const E& error) noexcept : _err(error), _is_ok(false)
    {
    }

    ResultVoid(const ResultVoid<E>& other) = delete;
    ResultVoid<E>& operator=(const ResultVoid<E>& other) = delete;

    ResultVoid(ResultVoid<E>&& other) noexcept : _err(other._err), _is_ok(other._is_ok)
    {
#ifndef NDEBUG
        _checked = other._checked;
        other._checked = true;
#endif // !NDEBUG
    }

#ifndef NDEBUG
    ~ResultVoid()
    {
        assert(_checked && "Result value was not checked before destruction");
    }
#endif

    bool is_ok() const
    {
#ifndef NDEBUG
        _checked = true;
#endif
        return _is_ok;
    }

    bool is_err() const
    {
#ifndef NDEBUG
        _checked = true;
#endif
        return !_is_ok;
    }

    E unwrap_err()
    {
        assert(is_err() && "Result::unwrap_err() called on ok value");
        return _err;
    }
};

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
