#pragma once
#include <type_traits>
#include <utility>

namespace leanclr
{
namespace utils
{
// One statically typed cleanup per guard: installing rollback must not itself
// allocate or fail while handling an allocation error.
template <typename F>
class ScopeExit
{
    F cleanup_;
    bool active_ = true;

  public:
    explicit ScopeExit(F cleanup) : cleanup_(std::move(cleanup))
    {
    }

    ScopeExit(const ScopeExit&) = delete;
    ScopeExit& operator=(const ScopeExit&) = delete;

    ScopeExit(ScopeExit&& other) noexcept(std::is_nothrow_move_constructible<F>::value) : cleanup_(std::move(other.cleanup_)), active_(other.active_)
    {
        other.active_ = false;
    }

    ScopeExit& operator=(ScopeExit&&) = delete;

    ~ScopeExit() noexcept
    {
        if (active_)
            cleanup_();
    }

    void dismiss() noexcept
    {
        active_ = false;
    }
};

template <typename F>
ScopeExit<typename std::decay<F>::type> make_scope_exit(F&& cleanup)
{
    return ScopeExit<typename std::decay<F>::type>(std::forward<F>(cleanup));
}
} // namespace utils
} // namespace leanclr
