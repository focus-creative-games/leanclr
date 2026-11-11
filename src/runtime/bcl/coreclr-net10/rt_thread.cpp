#include "managed_layouts.h"

#include "vm/rt_managed_types.h"
#include "vm/rt_thread.h"
#include "vm/object.h"
#include "vm/class.h"
#include "vm/appdomain.h"
#include "alloc/general_allocation.h"
#include "gc/gc_roots.h"

namespace leanclr
{
namespace vm
{
static RtThread* g_current_thread = nullptr;
static int32_t g_priority = static_cast<int32_t>(ThreadPriority::Normal);

void register_threading_gc_roots()
{
    gc::GcRoots::register_slot(reinterpret_cast<RtObject**>(&g_current_thread));
}

RtThread* Thread::get_current_thread()
{
    assert(g_current_thread != nullptr);
    return g_current_thread;
}

RtThread* Thread::get_main_thread()
{
    assert(g_current_thread != nullptr);
    return g_current_thread;
}

bool Thread::is_vm_thread(RtThread* thread)
{
    return true;
}

void Thread::setup_internal_thread(RtThread* thread)
{
    assert(thread != nullptr);
    // This profile has one attached host thread and no Mono InternalThread.
    thread->managed_thread_id = 1;
    thread->priority = static_cast<int32_t>(ThreadPriority::Normal);
}

RtThread* Thread::attach_current_thread(RtAppDomain* app_domain)
{
    assert(g_current_thread == nullptr);

    // Get thread class from corlib
    auto thread_class = Class::get_corlib_types().cls_thread;
    assert(thread_class != nullptr);
    auto init_result = Class::initialize_all(thread_class);
    assert(init_result.is_ok());
    (void)init_result;

    // Create thread object
    auto thread_obj = static_cast<RtThread*>(LEANCLR_NEWOBJ_INTERNAL(thread_class, "Thread::attach_current_thread").unwrap());

    setup_internal_thread(thread_obj);
    g_current_thread = thread_obj;
    return thread_obj;
}

void Thread::detach(RtThread* thread)
{
    assert(thread != nullptr && thread == g_current_thread);
    g_current_thread = nullptr;
}

RtThread** Thread::get_all_attached_threads(size_t* size)
{
    assert(size != nullptr);
    assert(g_current_thread != nullptr);
    *size = 1;
    return &g_current_thread;
}

RtResultVoid Thread::construct_internal_thread(RtThread*)
{
    RET_ERR(RtErr::NotSupported);
}

RtResultVoid Thread::free_internal_thread(vm::RtInternalThread* this_thread)
{
    if (this_thread == nullptr)
    {
        RET_VOID_OK();
    }

    // Free name string (UTF-16) if allocated
    if (this_thread->name_chars != nullptr)
    {
        alloc::GeneralAllocation::free((void*)this_thread->name_chars);
        this_thread->name_chars = nullptr;
    }
    this_thread->name_length = 0;

    // Free native thread handle if allocated
    if (this_thread->handle != nullptr)
    {
        alloc::GeneralAllocation::free(this_thread->handle);
        this_thread->handle = nullptr;
    }
    this_thread->state = RtThreadState::Stopped;

    // NOTE: Other resources not freed as per Rust implementation:
    // - long_lived->sync_block_cache
    // - long_lived->culture_info

    RET_VOID_OK();
}

void Thread::sleep(int32_t milliseconds)
{
    // No-op for wasm
}

bool Thread::yield_internal()
{
    // No-op for wasm
    return false;
}

void Thread::set_state(RtInternalThread* thread, RtThreadState state)
{
    assert(thread != nullptr);
    thread->state = state;
}

void Thread::clear_state(RtInternalThread* thread, RtThreadState state)
{
    assert(thread != nullptr);
    auto current = static_cast<int32_t>(thread->state);
    auto state_value = static_cast<int32_t>(state);
    thread->state = static_cast<RtThreadState>(current & ~state_value);
}

RtThreadState Thread::get_state(RtInternalThread* thread)
{
    assert(thread != nullptr);
    return thread->state;
}

int32_t Thread::get_system_max_stack_size()
{
    return 2 * 1024 * 1024; // 2 MB
}

int32_t Thread::get_priority_native(RtThread* thread)
{
    // thread parameter unused for now
    return g_priority;
}

void Thread::set_priority_native(RtThread* thread, int32_t priority)
{
    // thread parameter unused for now
    g_priority = priority;
}

void Thread::set_default_affinity_mask(int64_t affinity_mask)
{
}

struct StartData
{
    RtThread* m_Thread;
    RtAppDomain* m_Domain;
    RtMulticastDelegate* m_Delegate;
    RtObject* m_StartArg;
    void* m_Semaphore;
};

static int32_t g_thread_id_counter = 0;

static int32_t get_next_thread_id()
{
    return ++g_thread_id_counter;
}

bool Thread::start_thread(RtThread*, vm::RtMulticastDelegate*)
{
    // Native worker threads are outside this single-thread profile.
    return false;
}

void Thread::abort(RtThread* thread)
{
    // TODO: Implement
}

RtResult<vm::RtObject*> Thread::get_abort_exception_state()
{
    RET_OK(nullptr);
}

} // namespace vm
} // namespace leanclr
