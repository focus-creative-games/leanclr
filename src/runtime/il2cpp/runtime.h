#pragma once

#include "il2cpp_api_types.h"

namespace leanclr
{
namespace il2cpp
{
class Runtime
{
  public:
    static void set_unhandled_exception_policy(Il2CppRuntimeUnhandledExceptionPolicy policy);
    static Il2CppManagedMemorySnapshot* capture_memory_snapshot();
    static void free_captured_memory_snapshot(Il2CppManagedMemorySnapshot* s);

    static void set_find_plugin_callback(Il2CppSetFindPlugInCallback method);
    static void set_log_callback(Il2CppLogCallback method);
    static Il2CppLogCallback get_log_callback();
};
} // namespace il2cpp
} // namespace leanclr