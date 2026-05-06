#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>
#include <functional>
#include <new>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <cstdlib>

#if defined(_MSVC_LANG)
#define LEANCLR_CPLUSPLUS _MSVC_LANG
#else
#define LEANCLR_CPLUSPLUS __cplusplus
#endif

#if LEANCLR_CPLUSPLUS >= 201703L
#include <optional>
#endif

#if LEANCLR_CPLUSPLUS >= 201703L
#include <string_view>
#define LEANCLR_HAS_STD_STRING_VIEW 1
#elif !defined(_MSC_VER) && defined(__has_include)
#if __has_include(<string_view>)
#include <string_view>
#if defined(__cpp_lib_string_view) || defined(_LIBCPP_VERSION)
#define LEANCLR_HAS_STD_STRING_VIEW 1
#endif
#endif
#endif

#ifndef LEANCLR_HAS_STD_STRING_VIEW
#define LEANCLR_HAS_STD_STRING_VIEW 0
#endif

#if LEANCLR_CPLUSPLUS < 201703L
namespace std
{
struct nullopt_t
{
    explicit nullopt_t(int)
    {
    }
};

static const nullopt_t nullopt(0);

template <typename T>
class optional
{
  private:
    static_assert(!std::is_reference<T>::value, "optional<T&> is not supported");
    typedef typename std::aligned_storage<sizeof(T), alignof(T)>::type StorageType;

    StorageType _storage;
    bool _has_value;

    T* ptr()
    {
        return reinterpret_cast<T*>(&_storage);
    }

    const T* ptr() const
    {
        return reinterpret_cast<const T*>(&_storage);
    }

  public:
    optional() : _has_value(false)
    {
    }

    optional(nullopt_t) : _has_value(false)
    {
    }

    optional(const T& value) : _has_value(true)
    {
        new (&_storage) T(value);
    }

    optional(T&& value) : _has_value(true)
    {
        new (&_storage) T(std::move(value));
    }

    optional(const optional& other) : _has_value(other._has_value)
    {
        if (_has_value)
            new (&_storage) T(*other.ptr());
    }

    optional(optional&& other) : _has_value(other._has_value)
    {
        if (_has_value)
            new (&_storage) T(std::move(*other.ptr()));
    }

    optional& operator=(nullopt_t)
    {
        reset();
        return *this;
    }

    optional& operator=(const T& value)
    {
        if (_has_value)
            *ptr() = value;
        else
        {
            new (&_storage) T(value);
            _has_value = true;
        }
        return *this;
    }

    optional& operator=(T&& value)
    {
        if (_has_value)
            *ptr() = std::move(value);
        else
        {
            new (&_storage) T(std::move(value));
            _has_value = true;
        }
        return *this;
    }

    optional& operator=(const optional& other)
    {
        if (this == &other)
            return *this;
        if (_has_value && other._has_value)
        {
            *ptr() = *other.ptr();
        }
        else if (_has_value && !other._has_value)
        {
            reset();
        }
        else if (!_has_value && other._has_value)
        {
            new (&_storage) T(*other.ptr());
            _has_value = true;
        }
        return *this;
    }

    optional& operator=(optional&& other)
    {
        if (this == &other)
            return *this;
        if (_has_value && other._has_value)
        {
            *ptr() = std::move(*other.ptr());
        }
        else if (_has_value && !other._has_value)
        {
            reset();
        }
        else if (!_has_value && other._has_value)
        {
            new (&_storage) T(std::move(*other.ptr()));
            _has_value = true;
        }
        return *this;
    }

    ~optional()
    {
        reset();
    }

    void reset()
    {
        if (_has_value)
        {
            ptr()->~T();
            _has_value = false;
        }
    }

    bool has_value() const
    {
        return _has_value;
    }

    explicit operator bool() const
    {
        return _has_value;
    }

    T& value()
    {
        if (!_has_value)
        {
            assert(false && "bad optional access");
            std::abort();
        }
        return *ptr();
    }

    const T& value() const
    {
        if (!_has_value)
        {
            assert(false && "bad optional access");
            std::abort();
        }
        return *ptr();
    }

    T& operator*()
    {
        assert(_has_value);
        return *ptr();
    }

    const T& operator*() const
    {
        assert(_has_value);
        return *ptr();
    }

    T* operator->()
    {
        assert(_has_value);
        return ptr();
    }

    const T* operator->() const
    {
        assert(_has_value);
        return ptr();
    }

    T value_or(const T& default_value) const
    {
        return _has_value ? *ptr() : default_value;
    }
};

template <typename T>
optional<typename std::decay<T>::type> make_optional(T&& value)
{
    typedef typename std::decay<T>::type U;
    return optional<U>(std::forward<T>(value));
}
} // namespace std
#endif

#if !LEANCLR_HAS_STD_STRING_VIEW
namespace std
{
class string_view
{
  public:
    typedef const char* const_iterator;
    static const size_t npos = static_cast<size_t>(-1);

    string_view() : _data(nullptr), _size(0)
    {
    }

    string_view(const char* s) : _data(s), _size(s ? std::strlen(s) : 0)
    {
    }

    string_view(const char* s, size_t count) : _data(s), _size(count)
    {
    }

    const_iterator begin() const
    {
        return _data;
    }

    const_iterator end() const
    {
        return _data + _size;
    }

    const char* data() const
    {
        return _data;
    }

    size_t size() const
    {
        return _size;
    }

    bool empty() const
    {
        return _size == 0;
    }

    char operator[](size_t pos) const
    {
        return _data[pos];
    }

    string_view substr(size_t pos, size_t count = npos) const
    {
        assert(pos <= _size);
        size_t remaining = _size - pos;
        size_t len = count < remaining ? count : remaining;
        return string_view(_data + pos, len);
    }

    size_t find(char c, size_t pos = 0) const
    {
        if (pos >= _size)
            return npos;
        for (size_t i = pos; i < _size; ++i)
        {
            if (_data[i] == c)
                return i;
        }
        return npos;
    }

    operator std::string() const
    {
        return std::string(_data, _size);
    }

  private:
    const char* _data;
    size_t _size;
};

inline bool operator==(const string_view& lhs, const string_view& rhs)
{
    if (lhs.size() != rhs.size())
        return false;
    for (size_t i = 0; i < lhs.size(); ++i)
    {
        if (lhs[i] != rhs[i])
            return false;
    }
    return true;
}

inline bool operator!=(const string_view& lhs, const string_view& rhs)
{
    return !(lhs == rhs);
}

inline bool operator==(const string_view& lhs, const char* rhs)
{
    return lhs == string_view(rhs);
}

inline bool operator==(const char* lhs, const string_view& rhs)
{
    return string_view(lhs) == rhs;
}

inline bool operator!=(const string_view& lhs, const char* rhs)
{
    return !(lhs == rhs);
}

inline bool operator!=(const char* lhs, const string_view& rhs)
{
    return !(lhs == rhs);
}

template <>
struct hash<string_view>
{
    size_t operator()(const string_view& sv) const
    {
        size_t h = 1469598103934665603ull;
        for (size_t i = 0; i < sv.size(); ++i)
        {
            h ^= static_cast<unsigned char>(sv[i]);
            h *= 1099511628211ull;
        }
        return h;
    }
};
} // namespace std
#endif
