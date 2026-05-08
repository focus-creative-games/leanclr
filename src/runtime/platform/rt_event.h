#pragma once

#include "core/rt_base.h"

namespace leanclr
{
namespace platform
{

/// Single-threaded / WebGL stub: no OS wait/signal; `set`/`reset` are no-ops that still report success.
class Event
{
  public:
    Event(bool /*manual_reset*/, bool /*initial_state*/) noexcept {}
    ~Event() noexcept = default;

    bool set() noexcept;
    bool reset() noexcept;
};

/// Owns a heap-allocated `Event`; the managed `IntPtr` is a pointer to this wrapper object.
class EventHandle
{
  public:
    explicit EventHandle(Event* ev) noexcept;
    ~EventHandle() noexcept;

    Event& get() noexcept { return *m_event; }

    EventHandle(const EventHandle&) = delete;
    EventHandle& operator=(const EventHandle&) = delete;

  private:
    Event* m_event;
};

} // namespace platform
} // namespace leanclr
