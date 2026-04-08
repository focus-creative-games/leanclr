#include "runtime.h"
#include "alloc/general_allocation.h"

namespace leanclr
{
namespace il2cpp
{

static Il2CppSetFindPlugInCallback s_find_plugin_callback = nullptr;
static Il2CppLogCallback s_log_callback = nullptr;

void Runtime::set_unhandled_exception_policy(Il2CppRuntimeUnhandledExceptionPolicy policy)
{
    // TODO
}

Il2CppManagedMemorySnapshot* Runtime::capture_memory_snapshot()
{
    Il2CppManagedMemorySnapshot* snapshot = alloc::GeneralAllocation::malloc_any_zeroed<Il2CppManagedMemorySnapshot>();
    return snapshot;
}

void Runtime::free_captured_memory_snapshot(Il2CppManagedMemorySnapshot* s)
{
    alloc::GeneralAllocation::free(s);
}

void Runtime::set_find_plugin_callback(Il2CppSetFindPlugInCallback method)
{
    s_find_plugin_callback = method;
}

void Runtime::set_log_callback(Il2CppLogCallback method)
{
    s_log_callback = method;
}

Il2CppLogCallback Runtime::get_log_callback()
{
    return s_log_callback;
}

} // namespace il2cpp
} // namespace leanclr