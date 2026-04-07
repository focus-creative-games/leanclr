#include "profiler.h"

namespace leanclr
{
namespace il2cpp
{
#if IL2CPP_ENABLE_PROFILER

void Profiler::install(Il2CppProfiler* prof, Il2CppProfileFunc shutdown_callback)
{
}

void Profiler::set_events(Il2CppProfileFlags events)
{
}

void Profiler::install_enter_leave(Il2CppProfileMethodFunc enter, Il2CppProfileMethodFunc flee)
{
}

void Profiler::install_allocation(Il2CppProfileAllocFunc callback)
{
}

void Profiler::install_gc(Il2CppProfileGCFunc cb, Il2CppProfileGCResizeFunc resize_cb)
{
}

void Profiler::install_fileio(Il2CppProfileFileIOFunc callback)
{
}

void Profiler::install_thread(Il2CppProfileThreadFunc start, Il2CppProfileThreadFunc end)
{
}

#endif
} // namespace il2cpp
} // namespace leanclr
