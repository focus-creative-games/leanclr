#include "rt_event.h"

namespace leanclr
{
namespace platform
{

bool Event::set() noexcept
{
    return true;
}

bool Event::reset() noexcept
{
    return true;
}

EventHandle::EventHandle(Event* ev) noexcept : m_event(ev)
{
}

EventHandle::~EventHandle() noexcept
{
    delete m_event;
    m_event = nullptr;
}

} // namespace platform
} // namespace leanclr
