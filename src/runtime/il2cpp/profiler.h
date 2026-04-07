#pragma once

#include "il2cpp_api_types.h"

namespace leanclr
{
namespace il2cpp
{
#if IL2CPP_ENABLE_PROFILER
class Profiler
{
  public:
    static void install(Il2CppProfiler* prof, Il2CppProfileFunc shutdown_callback);
    static void set_events(Il2CppProfileFlags events);
    static void install_enter_leave(Il2CppProfileMethodFunc enter, Il2CppProfileMethodFunc flee);
    static void install_allocation(Il2CppProfileAllocFunc callback);
    static void install_gc(Il2CppProfileGCFunc cb, Il2CppProfileGCResizeFunc resize_cb);
    static void install_fileio(Il2CppProfileFileIOFunc callback);
    static void install_thread(Il2CppProfileThreadFunc start, Il2CppProfileThreadFunc end);
};
#endif
} // namespace il2cpp
} // namespace leanclr