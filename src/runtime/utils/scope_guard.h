#pragma once

#include <functional>
#include <utility>
#include <vector>

namespace leanclr
{
namespace utils
{

// Runs registered cleanup callbacks in reverse registration order when the scope ends.
// Set not_cleanup to skip all callbacks on destruction (e.g. success path or after move).
class ScopeGuard
{
  public:

    ScopeGuard() = default;

    ~ScopeGuard()
    {
        run_cleanups_if_needed();
    }

    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;
    ScopeGuard(ScopeGuard&& other) = delete;

    ScopeGuard& operator=(ScopeGuard&& other) noexcept
    {
        if (this != &other)
        {
            run_cleanups_if_needed();
            not_cleanup = other.not_cleanup;
            cleanups_ = std::move(other.cleanups_);
            other.not_cleanup = true;
            other.cleanups_.clear();
        }
        return *this;
    }

    void register_cleanup(std::function<void()> cleanup_func)
    {
        cleanups_.push_back(std::move(cleanup_func));
    }

    void set_not_cleanup()
    {
        not_cleanup = true;
    }

  private:
      bool not_cleanup = false;
    std::vector<std::function<void()>> cleanups_;

    void run_cleanups_if_needed()
    {
        if (not_cleanup)
        {
            return;
        }
        for (auto it = cleanups_.rbegin(); it != cleanups_.rend(); ++it)
        {
            if (*it)
            {
                (*it)();
            }
        }
        cleanups_.clear();
    }
};

} // namespace utils
} // namespace leanclr
