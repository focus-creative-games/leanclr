#include "system_gc.h"

#include "icall_base.h"
#include "vm/class.h"
#include "vm/field.h"
#include "vm/gc.h"

namespace leanclr
{
namespace icalls
{
namespace
{

void set_int64_field_if_present(vm::RtObject* obj, const char* field_name, int64_t value) noexcept
{
    if (obj == nullptr || obj->klass == nullptr)
    {
        return;
    }

    const metadata::RtFieldInfo* field = vm::Class::get_field_for_name(obj->klass, field_name, true);
    if (field == nullptr)
    {
        return;
    }

    uint8_t* data = reinterpret_cast<uint8_t*>(obj) + vm::Field::get_instance_field_offset_includes_object_header_for_all_type(field);
    *reinterpret_cast<int64_t*>(data) = value;
}

void set_int32_field_if_present(vm::RtObject* obj, const char* field_name, int32_t value) noexcept
{
    if (obj == nullptr || obj->klass == nullptr)
    {
        return;
    }

    const metadata::RtFieldInfo* field = vm::Class::get_field_for_name(obj->klass, field_name, true);
    if (field == nullptr)
    {
        return;
    }

    uint8_t* data = reinterpret_cast<uint8_t*>(obj) + vm::Field::get_instance_field_offset_includes_object_header_for_all_type(field);
    *reinterpret_cast<int32_t*>(data) = value;
}

void set_byte_field_if_present(vm::RtObject* obj, const char* field_name, uint8_t value) noexcept
{
    if (obj == nullptr || obj->klass == nullptr)
    {
        return;
    }

    const metadata::RtFieldInfo* field = vm::Class::get_field_for_name(obj->klass, field_name, true);
    if (field == nullptr)
    {
        return;
    }

    uint8_t* data = reinterpret_cast<uint8_t*>(obj) + vm::Field::get_instance_field_offset_includes_object_header_for_all_type(field);
    *data = value;
}

} // namespace

RtResult<vm::RtObject*> SystemGC::get_ephemeron_tombstone() noexcept
{
    RET_OK(vm::GC::get_ephemeron_tombstone());
}

/// @icall: System.GC::get_ephemeron_tombstone
static RtResultVoid get_ephemeron_tombstone_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                    const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)params;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, tombstone, SystemGC::get_ephemeron_tombstone());
    EvalStackOp::set_return(ret, tombstone);
    RET_VOID_OK();
}

RtResultVoid SystemGC::register_ephemeron_array(vm::RtObject* arr) noexcept
{
    vm::GC::register_ephemeron_array(arr);
    RET_VOID_OK();
}

/// @icall: System.GC::register_ephemeron_array
static RtResultVoid register_ephemeron_array_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                     const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    auto arr = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    RET_ERR_ON_FAIL(SystemGC::register_ephemeron_array(arr));
    RET_VOID_OK();
}

RtResult<int32_t> SystemGC::get_collection_count(int32_t generation) noexcept
{
    RET_OK(vm::GC::get_collection_count(generation));
}

/// @icall: System.GC::GetCollectionCount(System.Int32)
static RtResultVoid get_collection_count_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                 const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto generation = EvalStackOp::get_param<int32_t>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, count, SystemGC::get_collection_count(generation));
    EvalStackOp::set_return(ret, count);
    RET_VOID_OK();
}

RtResult<int32_t> SystemGC::get_max_generation() noexcept
{
    RET_OK(vm::GC::get_max_generation());
}

/// @icall: System.GC::GetMaxGeneration()
static RtResultVoid get_max_generation_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)params;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, generation, SystemGC::get_max_generation());
    EvalStackOp::set_return(ret, generation);
    RET_VOID_OK();
}

RtResultVoid SystemGC::internal_collect(int32_t generation) noexcept
{
    vm::GC::internal_collect(generation);
    RET_VOID_OK();
}

/// @icall: System.GC::InternalCollect(System.Int32)
static RtResultVoid internal_collect_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                             const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    auto generation = EvalStackOp::get_param<int32_t>(params, 0);
    RET_ERR_ON_FAIL(SystemGC::internal_collect(generation));
    RET_VOID_OK();
}

RtResultVoid SystemGC::record_pressure(int64_t bytes) noexcept
{
    vm::GC::record_pressure(bytes);
    RET_VOID_OK();
}

/// @icall: System.GC::RecordPressure(System.Int64)
static RtResultVoid record_pressure_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                            const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    auto bytes = EvalStackOp::get_param<int64_t>(params, 0);
    RET_ERR_ON_FAIL(SystemGC::record_pressure(bytes));
    RET_VOID_OK();
}

RtResult<int64_t> SystemGC::get_allocated_bytes_for_current_thread() noexcept
{
    RET_OK(vm::GC::get_allocated_bytes_for_current_thread());
}

/// @icall: System.GC::GetAllocatedBytesForCurrentThread()
static RtResultVoid get_allocated_bytes_for_current_thread_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                                   const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)params;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int64_t, bytes, SystemGC::get_allocated_bytes_for_current_thread());
    EvalStackOp::set_return(ret, bytes);
    RET_VOID_OK();
}

RtResult<int32_t> SystemGC::get_generation(vm::RtObject* obj) noexcept
{
    RET_OK(vm::GC::get_generation(obj));
}

/// @icall: System.GC::GetGeneration(System.Object)
static RtResultVoid get_generation_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                           const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto obj = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, generation, SystemGC::get_generation(obj));
    EvalStackOp::set_return(ret, generation);
    RET_VOID_OK();
}

RtResultVoid SystemGC::wait_for_pending_finalizers() noexcept
{
    vm::GC::wait_for_pending_finalizers();
    RET_VOID_OK();
}

/// @icall: System.GC::WaitForPendingFinalizers()
static RtResultVoid wait_for_pending_finalizers_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                        const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)params;
    (void)ret;
    RET_ERR_ON_FAIL(SystemGC::wait_for_pending_finalizers());
    RET_VOID_OK();
}

RtResultVoid SystemGC::suppress_finalize(vm::RtObject* obj) noexcept
{
    if (obj == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }
    vm::GC::suppress_finalize(obj);
    RET_VOID_OK();
}

/// @icall: System.GC::SuppressFinalizeInternal(System.Object)
static RtResultVoid suppress_finalize_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                              const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    auto obj = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    RET_ERR_ON_FAIL(SystemGC::suppress_finalize(obj));
    RET_VOID_OK();
}

RtResultVoid SystemGC::reregister_for_finalize(vm::RtObject* obj) noexcept
{
    if (obj == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }
    vm::GC::reregister_for_finalize(obj);
    RET_VOID_OK();
}

/// @icall: System.GC::_ReRegisterForFinalize(System.Object)
static RtResultVoid reregister_for_finalize_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                    const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    auto obj = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    RET_ERR_ON_FAIL(SystemGC::reregister_for_finalize(obj));
    RET_VOID_OK();
}

RtResult<int64_t> SystemGC::get_total_memory(bool force_full_collection) noexcept
{
    RET_OK(vm::GC::get_total_memory(force_full_collection));
}

RtResultVoid SystemGC::get_memory_info(vm::RtObject* data, int32_t kind) noexcept
{
    (void)kind;
    if (data == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    int64_t heap_size = vm::GC::get_total_memory(false);
    constexpr int64_t total_available_memory = 1024LL * 1024LL * 1024LL;
    constexpr int64_t high_memory_load_threshold = 900LL * 1024LL * 1024LL;

    set_int64_field_if_present(data, "_highMemoryLoadThresholdBytes", high_memory_load_threshold);
    set_int64_field_if_present(data, "_totalAvailableMemoryBytes", total_available_memory);
    set_int64_field_if_present(data, "_memoryLoadBytes", heap_size);
    set_int64_field_if_present(data, "_heapSizeBytes", heap_size);
    set_int64_field_if_present(data, "_fragmentedBytes", 0);
    set_int64_field_if_present(data, "_totalCommittedBytes", heap_size);
    set_int64_field_if_present(data, "_promotedBytes", 0);
    set_int64_field_if_present(data, "_pinnedObjectsCount", 0);
    set_int64_field_if_present(data, "_finalizationPendingCount", 0);
    set_int64_field_if_present(data, "_index", vm::GC::get_collection_count(0));
    set_int32_field_if_present(data, "_generation", vm::GC::get_max_generation());
    set_int32_field_if_present(data, "_pauseTimePercentage", 0);
    set_byte_field_if_present(data, "_compacted", 0);
    set_byte_field_if_present(data, "_concurrent", 0);
    RET_VOID_OK();
}

/// @icall: System.GC::GetTotalMemory(System.Boolean)
static RtResultVoid get_total_memory_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                             const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto force = EvalStackOp::get_param<bool>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int64_t, memory, SystemGC::get_total_memory(force));
    EvalStackOp::set_return(ret, memory);
    RET_VOID_OK();
}

/// @icall: System.GC::GetMemoryInfo(System.GCMemoryInfoData,System.Int32)
static RtResultVoid get_memory_info_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                            const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    vm::RtObject* data = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    int32_t kind = EvalStackOp::get_param<int32_t>(params, 1);
    RET_ERR_ON_FAIL(SystemGC::get_memory_info(data, kind));
    RET_VOID_OK();
}

utils::Span<vm::InternalCallEntry> SystemGC::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.GC::get_ephemeron_tombstone", (vm::InternalCallFunction)&SystemGC::get_ephemeron_tombstone, get_ephemeron_tombstone_invoker},
        {"System.GC::register_ephemeron_array", (vm::InternalCallFunction)&SystemGC::register_ephemeron_array, register_ephemeron_array_invoker},
        {"System.GC::_CollectionCount(System.Int32,System.Int32)", (vm::InternalCallFunction)&SystemGC::get_collection_count,
         get_collection_count_invoker},
        {"System.GC::GetCollectionCount(System.Int32)", (vm::InternalCallFunction)&SystemGC::get_collection_count, get_collection_count_invoker},
        {"System.GC::GetMaxGeneration()", (vm::InternalCallFunction)&SystemGC::get_max_generation, get_max_generation_invoker},
        {"System.GC::InternalCollect(System.Int32)", (vm::InternalCallFunction)&SystemGC::internal_collect, internal_collect_invoker},
        {"System.GC::RecordPressure(System.Int64)", (vm::InternalCallFunction)&SystemGC::record_pressure, record_pressure_invoker},
        {"System.GC::GetAllocatedBytesForCurrentThread()", (vm::InternalCallFunction)&SystemGC::get_allocated_bytes_for_current_thread,
         get_allocated_bytes_for_current_thread_invoker},
        {"System.GC::GetGeneration(System.Object)", (vm::InternalCallFunction)&SystemGC::get_generation, get_generation_invoker},
        {"System.GC::WaitForPendingFinalizers()", (vm::InternalCallFunction)&SystemGC::wait_for_pending_finalizers, wait_for_pending_finalizers_invoker},
        {"System.GC::SuppressFinalize(System.Object)", (vm::InternalCallFunction)&SystemGC::suppress_finalize, suppress_finalize_invoker},
        {"System.GC::SuppressFinalizeInternal(System.Object)", (vm::InternalCallFunction)&SystemGC::suppress_finalize, suppress_finalize_invoker},
        {"System.GC::_SuppressFinalize(System.Object)", (vm::InternalCallFunction)&SystemGC::suppress_finalize, suppress_finalize_invoker},
        {"System.GC::ReRegisterForFinalize(System.Object)", (vm::InternalCallFunction)&SystemGC::reregister_for_finalize, reregister_for_finalize_invoker},
        {"System.GC::_ReRegisterForFinalize(System.Object)", (vm::InternalCallFunction)&SystemGC::reregister_for_finalize, reregister_for_finalize_invoker},
        {"System.GC::GetTotalMemory(System.Boolean)", (vm::InternalCallFunction)&SystemGC::get_total_memory, get_total_memory_invoker},
        {"System.GC::GetMemoryInfo(System.GCMemoryInfoData,System.Int32)", (vm::InternalCallFunction)&SystemGC::get_memory_info, get_memory_info_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr
