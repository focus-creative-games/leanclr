
#include "rt_managed_types.h"
#include "rt_thread.h"
#include "object.h"
#include "class.h"
#include "appdomain.h"
#include "alloc/general_allocation.h"
#include "gc/garbage_collector.h"

namespace leanclr
{
namespace vm
{
static RtThread* g_current_thread = nullptr;
static int32_t g_priority = static_cast<int32_t>(ThreadPriority::Normal);

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

    // Get internal thread class from corlib
    auto internal_thread_class = Class::get_corlib_types().cls_internal_thread;
    assert(internal_thread_class != nullptr);

    // Create internal thread object
    auto internal_thread_obj = static_cast<RtInternalThread*>(vm::Object::new_object(internal_thread_class).unwrap());

    internal_thread_obj->state = RtThreadState::Running;
    internal_thread_obj->handle = nullptr;
    internal_thread_obj->thread_id = 1;
    internal_thread_obj->managed_id = 1;
    thread->internal_thread = internal_thread_obj;
}

RtThread* Thread::attach_current_thread(RtAppDomain* app_domain)
{
    assert(g_current_thread == nullptr);

    // Get thread class from corlib
    auto thread_class = Class::get_corlib_types().cls_thread;
    assert(thread_class != nullptr);

    // Create thread object
    auto thread_obj = static_cast<RtThread*>(vm::Object::new_object(thread_class).unwrap());

    setup_internal_thread(thread_obj);
    g_current_thread = thread_obj;
    return thread_obj;
}

void Thread::detach(RtThread* thread)
{
    assert(thread != nullptr && thread == g_current_thread);
    free_internal_thread(thread->internal_thread);
    g_current_thread = nullptr;
}

RtThread** Thread::get_all_attached_threads(size_t* size)
{
    assert(size != nullptr);
    assert(g_current_thread != nullptr);
    *size = 1;
    return &g_current_thread;
}

RtResultVoid Thread::construct_internal_thread(RtThread* thread)
{
    auto internal_thread_class = Class::get_corlib_types().cls_internal_thread;

    // Create internal thread object
    auto internal_thread_obj = static_cast<RtInternalThread*>(vm::Object::new_object(internal_thread_class).unwrap());

    // Allocate native thread handle
    auto native_handle = alloc::GeneralAllocation::malloc_any_zeroed<RtNativeThread>();

    internal_thread_obj->handle = native_handle;
    internal_thread_obj->thread_id = 1;
    internal_thread_obj->managed_id = 1;
    internal_thread_obj->state = RtThreadState::Unstarted;
    thread->internal_thread = internal_thread_obj;

    RET_VOID_OK();
}

RtResultVoid Thread::free_internal_thread(vm::RtInternalThread* this_thread)
{
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

bool Thread::start_thread(RtThread* thread, vm::RtMulticastDelegate* start)
{
    RtInternalThread* internal_thread = thread->internal_thread;
    if ((int32_t)get_state(internal_thread) & (int32_t)RtThreadState::Aborted)
    {
        return internal_thread->handle != nullptr;
    }
    StartData* start_data = (StartData*)gc::GarbageCollector::allocate_fixed(sizeof(StartData));
    start_data->m_Thread = thread;
    start_data->m_Domain = AppDomain::get_default_appdomain();
    start_data->m_Delegate = start;
    start_data->m_StartArg = thread->thread_start_arg;
    start_data->m_Semaphore = nullptr;

    RtNativeThread* native_thread = internal_thread->handle;
    // TODO
    // native_thread->set_stack_size(internal_thread->stack_size);
    // native_thread->set_explicit_apartment_state(internal_thread->apartment_state);
    // auto ret = native_thread->run(start_data);
    // if (ret != 0)
    // {
    //     return false;
    // }
    // return true;
    internal_thread->state = (RtThreadState)((int32_t)internal_thread->state & ~(int32_t)RtThreadState::Unstarted);

    // TODO: Get thread id from native thread
    // internal_thread->thread_id = native_thread->thread_id;
    internal_thread->managed_id = get_next_thread_id();
    internal_thread->handle = native_thread;
    return true;
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
