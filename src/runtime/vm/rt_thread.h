#pragma once

#include "rt_managed_types.h"

namespace leanclr
{
namespace vm
{

// Thread priority levels
enum class ThreadPriority : int32_t
{
    Lowest,
    BelowNormal,
    Normal,
    AboveNormal,
    Highest,
};

class Thread
{
  public:
    // Get current thread
    static RtThread* get_current_thread();

    // Get main thread
    static RtThread* get_main_thread();
    static bool is_vm_thread(RtThread* thread);

    // Attach current thread to app domain
    static RtThread* attach_current_thread(RtAppDomain* app_domain);
    static void detach(RtThread* thread);
    static RtThread** get_all_attached_threads(size_t* size);

    // Setup internal thread structure
    static void setup_internal_thread(RtThread* thread);
    // Construct internal thread with native handle
    static RtResultVoid construct_internal_thread(RtThread* thread);
    static RtResultVoid free_internal_thread(vm::RtInternalThread* this_thread);

    // Sleep for milliseconds (no-op on wasm)
    static void sleep(int32_t milliseconds);

    // Yield thread (returns false on wasm)
    static bool yield_internal();

    // Set thread state
    static void set_state(RtInternalThread* thread, RtThreadState state);

    // Clear thread state bits
    static void clear_state(RtInternalThread* thread, RtThreadState state);

    // Get thread state
    static RtThreadState get_state(RtInternalThread* thread);

    // Get system max stack size
    static int32_t get_system_max_stack_size();

    // Get thread priority
    static int32_t get_priority_native(RtThread* thread);

    // Set thread priority
    static void set_priority_native(RtThread* thread, int32_t priority);

    static void set_default_affinity_mask(int64_t affinity_mask);

    static bool start_thread(RtThread* thread, vm::RtMulticastDelegate* start);

    static void abort(RtThread* thread);

    static RtResult<vm::RtObject*> get_abort_exception_state();
};

} // namespace vm
} // namespace leanclr
