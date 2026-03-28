#include "vm/runtime.h"
#include "vm/settings.h"

using namespace leanclr;

typedef leanclr::Utf16Char Il2CppChar;
#if _MSC_VER
typedef wchar_t Il2CppNativeChar;
#else
typedef char Il2CppNativeChar;
#endif

struct Il2CppArray;
struct Il2CppAssembly;
struct Il2CppClass;
struct Il2CppCustomAttrInfo;
struct Il2CppDebuggerTransport;
struct Il2CppDomain;
struct Il2CppException;
struct Il2CppImage;
struct Il2CppManagedMemorySnapshot;
struct Il2CppMemoryCallbacks;
struct Il2CppMethodDebugInfo;
struct Il2CppObject;
struct Il2CppProfiler;
struct Il2CppReflectionMethod;
struct Il2CppReflectionType;
struct Il2CppRuntimeStats;
struct Il2CppStackFrameInfo;
struct Il2CppString;
struct Il2CppThread;
struct Il2CppType;
struct EventInfo;
struct FieldInfo;
struct MethodInfo;
struct PropertyInfo;

typedef enum
{
    IL2CPP_PROFILE_NONE = 0,
    IL2CPP_PROFILE_APPDOMAIN_EVENTS = 1 << 0,
    IL2CPP_PROFILE_ASSEMBLY_EVENTS = 1 << 1,
    IL2CPP_PROFILE_MODULE_EVENTS = 1 << 2,
    IL2CPP_PROFILE_CLASS_EVENTS = 1 << 3,
    IL2CPP_PROFILE_JIT_COMPILATION = 1 << 4,
    IL2CPP_PROFILE_INLINING = 1 << 5,
    IL2CPP_PROFILE_EXCEPTIONS = 1 << 6,
    IL2CPP_PROFILE_ALLOCATIONS = 1 << 7,
    IL2CPP_PROFILE_GC = 1 << 8,
    IL2CPP_PROFILE_THREADS = 1 << 9,
    IL2CPP_PROFILE_REMOTING = 1 << 10,
    IL2CPP_PROFILE_TRANSITIONS = 1 << 11,
    IL2CPP_PROFILE_ENTER_LEAVE = 1 << 12,
    IL2CPP_PROFILE_COVERAGE = 1 << 13,
    IL2CPP_PROFILE_INS_COVERAGE = 1 << 14,
    IL2CPP_PROFILE_STATISTICAL = 1 << 15,
    IL2CPP_PROFILE_METHOD_EVENTS = 1 << 16,
    IL2CPP_PROFILE_MONITOR_EVENTS = 1 << 17,
    IL2CPP_PROFILE_IOMAP_EVENTS = 1 << 18, /* this should likely be removed, too */
    IL2CPP_PROFILE_GC_MOVES = 1 << 19,
    IL2CPP_PROFILE_FILEIO = 1 << 20
} Il2CppProfileFlags;

typedef enum
{
    IL2CPP_PROFILE_FILEIO_WRITE = 0,
    IL2CPP_PROFILE_FILEIO_READ
} Il2CppProfileFileIOKind;

typedef enum
{
    IL2CPP_GC_EVENT_START,
    IL2CPP_GC_EVENT_MARK_START,
    IL2CPP_GC_EVENT_MARK_END,
    IL2CPP_GC_EVENT_RECLAIM_START,
    IL2CPP_GC_EVENT_RECLAIM_END,
    IL2CPP_GC_EVENT_END,
    IL2CPP_GC_EVENT_PRE_STOP_WORLD,
    IL2CPP_GC_EVENT_POST_STOP_WORLD,
    IL2CPP_GC_EVENT_PRE_START_WORLD,
    IL2CPP_GC_EVENT_POST_START_WORLD
} Il2CppGCEvent;

typedef enum
{
    IL2CPP_GC_MODE_DISABLED = 0,
    IL2CPP_GC_MODE_ENABLED = 1,
    IL2CPP_GC_MODE_MANUAL = 2
} Il2CppGCMode;

typedef enum
{
    IL2CPP_STAT_NEW_OBJECT_COUNT,
    IL2CPP_STAT_INITIALIZED_CLASS_COUNT,
    // IL2CPP_STAT_GENERIC_VTABLE_COUNT,
    // IL2CPP_STAT_USED_CLASS_COUNT,
    IL2CPP_STAT_METHOD_COUNT,
    // IL2CPP_STAT_CLASS_VTABLE_SIZE,
    IL2CPP_STAT_CLASS_STATIC_DATA_SIZE,
    IL2CPP_STAT_GENERIC_INSTANCE_COUNT,
    IL2CPP_STAT_GENERIC_CLASS_COUNT,
    IL2CPP_STAT_INFLATED_METHOD_COUNT,
    IL2CPP_STAT_INFLATED_TYPE_COUNT,
    // IL2CPP_STAT_DELEGATE_CREATIONS,
    // IL2CPP_STAT_MINOR_GC_COUNT,
    // IL2CPP_STAT_MAJOR_GC_COUNT,
    // IL2CPP_STAT_MINOR_GC_TIME_USECS,
    // IL2CPP_STAT_MAJOR_GC_TIME_USECS
} Il2CppStat;

typedef enum
{
    IL2CPP_UNHANDLED_POLICY_LEGACY,
    IL2CPP_UNHANDLED_POLICY_CURRENT
} Il2CppRuntimeUnhandledExceptionPolicy;

typedef void (*il2cpp_register_object_callback)(Il2CppObject** arr, int size, void* userdata);
typedef void* (*il2cpp_liveness_reallocate_callback)(void* ptr, size_t size, void* userdata);
typedef void (*Il2CppFrameWalkFunc)(const Il2CppStackFrameInfo* info, void* user_data);
typedef void (*Il2CppProfileFunc)(Il2CppProfiler* prof);
typedef void (*Il2CppProfileMethodFunc)(Il2CppProfiler* prof, const MethodInfo* method);
typedef void (*Il2CppProfileAllocFunc)(Il2CppProfiler* prof, Il2CppObject* obj, Il2CppClass* klass);
typedef void (*Il2CppProfileGCFunc)(Il2CppProfiler* prof, Il2CppGCEvent event, int generation);
typedef void (*Il2CppProfileGCResizeFunc)(Il2CppProfiler* prof, int64_t new_size);
typedef void (*Il2CppProfileFileIOFunc)(Il2CppProfiler* prof, Il2CppProfileFileIOKind kind, int count);
typedef void (*Il2CppProfileThreadFunc)(Il2CppProfiler* prof, unsigned long tid);

typedef const Il2CppNativeChar* (*Il2CppSetFindPlugInCallback)(const Il2CppNativeChar*);
typedef void (*Il2CppLogCallback)(const char*);

typedef void (*Il2CppMethodPointer)();
typedef size_t (*Il2CppBacktraceFunc)(Il2CppMethodPointer* buffer, size_t maxSize);

struct Il2CppManagedMemorySnapshot;

typedef uintptr_t il2cpp_array_size_t;
#define ARRAY_LENGTH_AS_INT32(a) ((int32_t)a)

typedef uint8_t (*Il2CppAndroidUpStateFunc)(const char* ifName, uint8_t* is_up);

extern leanclr::metadata::RtAotModulesData g_aot_modules_data;

extern "C"
{
    int il2cpp_init(const char* domain_name)
    {
        vm::Settings::set_aot_modules_data(&g_aot_modules_data);
        auto ret = vm::Runtime::initialize();
        return ret.is_ok() ? 0 : -1;
    }

    int il2cpp_init_utf16(const Il2CppChar* domain_name)
    {
        return 0;
    }

    void il2cpp_shutdown()
    {
    }

    void il2cpp_set_config_dir(const char* config_path)
    {
    }

    void il2cpp_set_data_dir(const char* data_path)
    {
    }

    void il2cpp_set_temp_dir(const char* temp_dir)
    {
    }

    void il2cpp_set_commandline_arguments(int argc, const char* const argv[], const char* basedir)
    {
    }

    void il2cpp_set_commandline_arguments_utf16(int argc, const Il2CppChar* const argv[], const char* basedir)
    {
    }

    void il2cpp_set_config_utf16(const Il2CppChar* executablePath)
    {
    }

    void il2cpp_set_config(const char* executablePath)
    {
    }

    void il2cpp_set_memory_callbacks(Il2CppMemoryCallbacks* callbacks)
    {
    }

    void il2cpp_memory_pool_set_region_size(size_t size)
    {
    }

    size_t il2cpp_memory_pool_get_region_size()
    {
        return 0;
    }

    const Il2CppImage* il2cpp_get_corlib()
    {
        return NULL;
    }

    void il2cpp_add_internal_call(const char* name, Il2CppMethodPointer method)
    {
    }

    Il2CppMethodPointer il2cpp_resolve_icall(const char* name)
    {
        return NULL;
    }

    void* il2cpp_alloc(size_t size)
    {
        return NULL;
    }

    void il2cpp_free(void* ptr)
    {
    }

    // array

    Il2CppClass* il2cpp_array_class_get(Il2CppClass* element_class, uint32_t rank)
    {
        return NULL;
    }

    uint32_t il2cpp_array_length(Il2CppArray* array)
    {
        return 0;
    }

    uint32_t il2cpp_array_get_byte_length(Il2CppArray* array)
    {
        return 0;
    }

    Il2CppArray* il2cpp_array_new(Il2CppClass* elementTypeInfo, il2cpp_array_size_t length)
    {
        return NULL;
    }

    Il2CppArray* il2cpp_array_new_specific(Il2CppClass* arrayTypeInfo, il2cpp_array_size_t length)
    {
        return NULL;
    }

    Il2CppArray* il2cpp_array_new_full(Il2CppClass* array_class, il2cpp_array_size_t* lengths, il2cpp_array_size_t* lower_bounds)
    {
        return NULL;
    }

    Il2CppClass* il2cpp_bounded_array_class_get(Il2CppClass* element_class, uint32_t rank, bool bounded)
    {
        return NULL;
    }

    int il2cpp_array_element_size(const Il2CppClass* klass)
    {
        return 0;
    }

    // assembly
    const Il2CppImage* il2cpp_assembly_get_image(const Il2CppAssembly* assembly)
    {
        return NULL;
    }

    // class

    const Il2CppType* il2cpp_class_enum_basetype(Il2CppClass* klass)
    {
        return NULL;
    }

    Il2CppClass* il2cpp_class_from_system_type(Il2CppReflectionType* type)
    {
        return NULL;
    }

    bool il2cpp_class_is_inited(const Il2CppClass* klass)
    {
        return false;
    }

    bool il2cpp_class_is_generic(const Il2CppClass* klass)
    {
        return false;
    }

    bool il2cpp_class_is_inflated(const Il2CppClass* klass)
    {
        return false;
    }

    bool il2cpp_class_is_assignable_from(Il2CppClass* klass, Il2CppClass* oklass)
    {
        return false;
    }

    bool il2cpp_class_is_subclass_of(Il2CppClass* klass, Il2CppClass* klassc, bool check_interfaces)
    {
        return false;
    }

    bool il2cpp_class_has_parent(Il2CppClass* klass, Il2CppClass* klassc)
    {
        return false;
    }

    Il2CppClass* il2cpp_class_from_il2cpp_type(const Il2CppType* type)
    {
        return NULL;
    }

    Il2CppClass* il2cpp_class_from_name(const Il2CppImage* image, const char* namespaze, const char* name)
    {
        return NULL;
    }

    Il2CppClass* il2cpp_class_get_element_class(Il2CppClass* klass)
    {
        return NULL;
    }

    const EventInfo* il2cpp_class_get_events(Il2CppClass* klass, void** iter)
    {
        return NULL;
    }

    FieldInfo* il2cpp_class_get_fields(Il2CppClass* klass, void** iter)
    {
        return NULL;
    }

    Il2CppClass* il2cpp_class_get_nested_types(Il2CppClass* klass, void** iter)
    {
        return NULL;
    }

    Il2CppClass* il2cpp_class_get_interfaces(Il2CppClass* klass, void** iter)
    {
        return NULL;
    }

    const PropertyInfo* il2cpp_class_get_properties(Il2CppClass* klass, void** iter)
    {
        return NULL;
    }

    const PropertyInfo* il2cpp_class_get_property_from_name(Il2CppClass* klass, const char* name)
    {
        return NULL;
    }

    FieldInfo* il2cpp_class_get_field_from_name(Il2CppClass* klass, const char* name)
    {
        return NULL;
    }

    const MethodInfo* il2cpp_class_get_methods(Il2CppClass* klass, void** iter)
    {
        return NULL;
    }

    const MethodInfo* il2cpp_class_get_method_from_name(Il2CppClass* klass, const char* name, int argsCount)
    {
        return NULL;
    }

    const char* il2cpp_class_get_name(Il2CppClass* klass)
    {
        return NULL;
    }

    const char* il2cpp_class_get_namespace(Il2CppClass* klass)
    {
        return NULL;
    }

    Il2CppClass* il2cpp_class_get_parent(Il2CppClass* klass)
    {
        return NULL;
    }

    Il2CppClass* il2cpp_class_get_declaring_type(Il2CppClass* klass)
    {
        return NULL;
    }

    int32_t il2cpp_class_instance_size(Il2CppClass* klass)
    {
        return 0;
    }

    size_t il2cpp_class_num_fields(const Il2CppClass* klass)
    {
        return 0;
    }

    bool il2cpp_class_is_valuetype(const Il2CppClass* klass)
    {
        return false;
    }

    bool il2cpp_class_is_blittable(const Il2CppClass* klass)
    {
        return false;
    }

    int32_t il2cpp_class_value_size(Il2CppClass* klass, uint32_t* align)
    {
        return 0;
    }

    int il2cpp_class_get_flags(const Il2CppClass* klass)
    {
        return 0;
    }

    bool il2cpp_class_is_abstract(const Il2CppClass* klass)
    {
        return false;
    }

    bool il2cpp_class_is_interface(const Il2CppClass* klass)
    {
        return false;
    }

    int il2cpp_class_array_element_size(const Il2CppClass* klass)
    {
        return 0;
    }

    Il2CppClass* il2cpp_class_from_type(const Il2CppType* type)
    {
        return NULL;
    }

    const Il2CppType* il2cpp_class_get_type(Il2CppClass* klass)
    {
        return NULL;
    }

    uint32_t il2cpp_class_get_type_token(Il2CppClass* klass)
    {
        return 0;
    }

    bool il2cpp_class_has_attribute(Il2CppClass* klass, Il2CppClass* attr_class)
    {
        return false;
    }

    bool il2cpp_class_has_references(Il2CppClass* klass)
    {
        return false;
    }

    bool il2cpp_class_is_enum(const Il2CppClass* klass)
    {
        return false;
    }

    const Il2CppImage* il2cpp_class_get_image(Il2CppClass* klass)
    {
        return NULL;
    }

    const char* il2cpp_class_get_assemblyname(const Il2CppClass* klass)
    {
        return NULL;
    }

    int il2cpp_class_get_rank(const Il2CppClass* klass)
    {
        return 0;
    }

    uint32_t il2cpp_class_get_data_size(const Il2CppClass* klass)
    {
        return 0;
    }

    void* il2cpp_class_get_static_field_data(const Il2CppClass* klass)
    {
        return NULL;
    }

    // testing only
    size_t il2cpp_class_get_bitmap_size(const Il2CppClass* klass)
    {
        return 0;
    }

    void il2cpp_class_get_bitmap(Il2CppClass* klass, size_t* bitmap)
    {
    }

    // stats

    extern Il2CppRuntimeStats il2cpp_runtime_stats;

    bool il2cpp_stats_dump_to_file(const char* path)
    {
        return false;
    }

    uint64_t il2cpp_stats_get_value(Il2CppStat stat)
    {
        return 0;
    }

    // domain
    Il2CppDomain* il2cpp_domain_get()
    {
        return NULL;
    }

    const Il2CppAssembly* il2cpp_domain_assembly_open(Il2CppDomain* domain, const char* name)
    {
        return NULL;
    }

    const Il2CppAssembly** il2cpp_domain_get_assemblies(const Il2CppDomain* domain, size_t* size)
    {
        return NULL;
    }

    // exception

    void il2cpp_raise_exception(Il2CppException* exc)
    {
    }

    Il2CppException* il2cpp_exception_from_name_msg(const Il2CppImage* image, const char* name_space, const char* name, const char* msg)
    {
        return NULL;
    }

    Il2CppException* il2cpp_get_exception_argument_null(const char* arg)
    {
        return NULL;
    }

    void il2cpp_format_exception(const Il2CppException* ex, char* message, int message_size)
    {
    }

    void il2cpp_format_stack_trace(const Il2CppException* ex, char* output, int output_size)
    {
    }

    void il2cpp_unhandled_exception(Il2CppException* exc)
    {
    }

    void il2cpp_native_stack_trace(const Il2CppException* ex, uintptr_t** addresses, int* numFrames, char** imageUUID, char** imageName)
    {
    }

    // field

    const char* il2cpp_field_get_name(FieldInfo* field)
    {
        return NULL;
    }

    int il2cpp_field_get_flags(FieldInfo* field)
    {
        return 0;
    }

    Il2CppClass* il2cpp_field_get_parent(FieldInfo* field)
    {
        return NULL;
    }

    size_t il2cpp_field_get_offset(FieldInfo* field)
    {
        return 0;
    }

    const Il2CppType* il2cpp_field_get_type(FieldInfo* field)
    {
        return NULL;
    }

    void il2cpp_field_get_value(Il2CppObject* obj, FieldInfo* field, void* value)
    {
    }

    Il2CppObject* il2cpp_field_get_value_object(FieldInfo* field, Il2CppObject* obj)
    {
        return NULL;
    }

    bool il2cpp_field_has_attribute(FieldInfo* field, Il2CppClass* attr_class)
    {
        return false;
    }

    void il2cpp_field_set_value(Il2CppObject* obj, FieldInfo* field, void* value)
    {
    }

    void il2cpp_field_set_value_object(Il2CppObject* objectInstance, FieldInfo* field, Il2CppObject* value)
    {
    }

    void il2cpp_field_static_get_value(FieldInfo* field, void* value)
    {
    }

    void il2cpp_field_static_set_value(FieldInfo* field, void* value)
    {
    }

    bool il2cpp_field_is_literal(FieldInfo* field)
    {
        return false;
    }

    // gc
    void il2cpp_gc_collect(int maxGenerations)
    {
    }

    int32_t il2cpp_gc_collect_a_little()
    {
        return 0;
    }

    void il2cpp_gc_start_incremental_collection()
    {
    }

    void il2cpp_gc_enable()
    {
    }

    void il2cpp_gc_disable()
    {
    }

    bool il2cpp_gc_is_disabled()
    {
        return false;
    }

    void il2cpp_gc_set_mode(Il2CppGCMode mode)
    {
    }

    bool il2cpp_gc_is_incremental()
    {
        return false;
    }

    int64_t il2cpp_gc_get_max_time_slice_ns()
    {
        return 0;
    }

    void il2cpp_gc_set_max_time_slice_ns(int64_t maxTimeSlice)
    {
    }

    int64_t il2cpp_gc_get_used_size()
    {
        return 0;
    }

    int64_t il2cpp_gc_get_heap_size()
    {
        return 0;
    }

    void il2cpp_gc_foreach_heap(void (*func)(void* data, void* context), void* userData)
    {
    }

    void il2cpp_stop_gc_world()
    {
    }

    void il2cpp_start_gc_world()
    {
    }

    void* il2cpp_gc_alloc_fixed(size_t size)
    {
        return NULL;
    }

    void il2cpp_gc_free_fixed(void* address)
    {
    }

    // gchandle

    uint32_t il2cpp_gchandle_new(Il2CppObject* obj, bool pinned)
    {
        return 0;
    }

    uint32_t il2cpp_gchandle_new_weakref(Il2CppObject* obj, bool track_resurrection)
    {
        return 0;
    }

    Il2CppObject* il2cpp_gchandle_get_target(uint32_t gchandle)
    {
        return NULL;
    }

    void il2cpp_gchandle_foreach_get_target(void (*func)(void*, void*), void* userData)
    {
    }

    void il2cpp_gc_wbarrier_set_field(Il2CppObject* obj, void** targetAddress, void* object)
    {
    }

    bool il2cpp_gc_has_strict_wbarriers()
    {
        return false;
    }

    void il2cpp_gc_set_external_allocation_tracker(void (*func)(void*, size_t, int))
    {
    }

    void il2cpp_gc_set_external_wbarrier_tracker(void (*func)(void**))
    {
    }

    void il2cpp_gchandle_free(uint32_t gchandle)
    {
    }

    // vm runtime info
    uint32_t il2cpp_object_header_size()
    {
        return 0;
    }

    uint32_t il2cpp_array_object_header_size()
    {
        return 0;
    }

    uint32_t il2cpp_offset_of_array_length_in_array_object_header()
    {
        return 0;
    }

    uint32_t il2cpp_offset_of_array_bounds_in_array_object_header()
    {
        return 0;
    }

    uint32_t il2cpp_allocation_granularity()
    {
        return 0;
    }

    // liveness

    void* il2cpp_unity_liveness_allocate_struct(Il2CppClass* filter, int max_object_count, il2cpp_register_object_callback callback, void* userdata,
                                                il2cpp_liveness_reallocate_callback reallocate)
    {
        return NULL;
    }

    void il2cpp_unity_liveness_calculation_from_root(Il2CppObject* root, void* state)
    {
    }

    void il2cpp_unity_liveness_calculation_from_statics(void* state)
    {
    }

    void il2cpp_unity_liveness_finalize(void* state)
    {
    }

    void il2cpp_unity_liveness_free_struct(void* state)
    {
    }

    // method

    const Il2CppType* il2cpp_method_get_return_type(const MethodInfo* method)
    {
        return NULL;
    }

    const MethodInfo* il2cpp_method_get_from_reflection(const Il2CppReflectionMethod* method)
    {
        return NULL;
    }

    Il2CppReflectionMethod* il2cpp_method_get_object(const MethodInfo* method, Il2CppClass* refclass)
    {
        return NULL;
    }

    const char* il2cpp_method_get_name(const MethodInfo* method)
    {
        return NULL;
    }

    bool il2cpp_method_is_generic(const MethodInfo* method)
    {
        return false;
    }

    bool il2cpp_method_is_inflated(const MethodInfo* method)
    {
        return false;
    }

    bool il2cpp_method_is_instance(const MethodInfo* method)
    {
        return false;
    }

    uint32_t il2cpp_method_get_param_count(const MethodInfo* method)
    {
        return 0;
    }

    const Il2CppType* il2cpp_method_get_param(const MethodInfo* method, uint32_t index)
    {
        return NULL;
    }

    Il2CppClass* il2cpp_method_get_class(const MethodInfo* method)
    {
        return NULL;
    }

    bool il2cpp_method_has_attribute(const MethodInfo* method, Il2CppClass* attr_class)
    {
        return false;
    }

    Il2CppClass* il2cpp_method_get_declaring_type(const MethodInfo* method)
    {
        return NULL;
    }

    uint32_t il2cpp_method_get_flags(const MethodInfo* method, uint32_t* iflags)
    {
        return 0;
    }

    uint32_t il2cpp_method_get_token(const MethodInfo* method)
    {
        return 0;
    }

    const char* il2cpp_method_get_param_name(const MethodInfo* method, uint32_t index)
    {
        return NULL;
    }

    // profiler

#if IL2CPP_ENABLE_PROFILER

    void il2cpp_profiler_install(Il2CppProfiler* prof, Il2CppProfileFunc shutdown_callback)
    {
    }

    void il2cpp_profiler_set_events(Il2CppProfileFlags events)
    {
    }

    void il2cpp_profiler_install_enter_leave(Il2CppProfileMethodFunc enter, Il2CppProfileMethodFunc fleave)
    {
    }

    void il2cpp_profiler_install_allocation(Il2CppProfileAllocFunc callback)
    {
    }

    void il2cpp_profiler_install_gc(Il2CppProfileGCFunc callback, Il2CppProfileGCResizeFunc heap_resize_callback)
    {
    }

    void il2cpp_profiler_install_fileio(Il2CppProfileFileIOFunc callback)
    {
    }

    void il2cpp_profiler_install_thread(Il2CppProfileThreadFunc start, Il2CppProfileThreadFunc end)
    {
    }

#endif

    // property

    const char* il2cpp_property_get_name(PropertyInfo* prop)
    {
        return NULL;
    }

    const MethodInfo* il2cpp_property_get_get_method(PropertyInfo* prop)
    {
        return NULL;
    }

    const MethodInfo* il2cpp_property_get_set_method(PropertyInfo* prop)
    {
        return NULL;
    }

    Il2CppClass* il2cpp_property_get_parent(PropertyInfo* prop)
    {
        return NULL;
    }

    uint32_t il2cpp_property_get_flags(PropertyInfo* prop)
    {
        return 0;
    }

    // object

    Il2CppClass* il2cpp_object_get_class(Il2CppObject* obj)
    {
        return NULL;
    }

    uint32_t il2cpp_object_get_size(Il2CppObject* obj)
    {
        return 0;
    }

    const MethodInfo* il2cpp_object_get_virtual_method(Il2CppObject* obj, const MethodInfo* method)
    {
        return NULL;
    }

    Il2CppObject* il2cpp_object_new(const Il2CppClass* klass)
    {
        return NULL;
    }

    void* il2cpp_object_unbox(Il2CppObject* obj)
    {
        return NULL;
    }

    Il2CppObject* il2cpp_value_box(Il2CppClass* klass, void* data)
    {
        return NULL;
    }

    // monitor
    void il2cpp_monitor_enter(Il2CppObject* obj)
    {
    }

    bool il2cpp_monitor_try_enter(Il2CppObject* obj, uint32_t timeout)
    {
        return false;
    }

    void il2cpp_monitor_exit(Il2CppObject* obj)
    {
    }

    void il2cpp_monitor_pulse(Il2CppObject* obj)
    {
    }

    void il2cpp_monitor_pulse_all(Il2CppObject* obj)
    {
    }

    void il2cpp_monitor_wait(Il2CppObject* obj)
    {
    }

    bool il2cpp_monitor_try_wait(Il2CppObject* obj, uint32_t timeout)
    {
        return false;
    }

    // runtime

    Il2CppObject* il2cpp_runtime_invoke_convert_args(const MethodInfo* method, void* obj, Il2CppObject** params, int paramCount, Il2CppException** exc)
    {
        return NULL;
    }

    Il2CppObject* il2cpp_runtime_invoke(const MethodInfo* method, void* obj, void** params, Il2CppException** exc)
    {
        return NULL;
    }

    void il2cpp_runtime_class_init(Il2CppClass* klass)
    {
    }

    void il2cpp_runtime_object_init(Il2CppObject* obj)
    {
    }

    void il2cpp_runtime_object_init_exception(Il2CppObject* obj, Il2CppException** exc)
    {
    }

    void il2cpp_runtime_unhandled_exception_policy_set(Il2CppRuntimeUnhandledExceptionPolicy value)
    {
    }

    // string

    int32_t il2cpp_string_length(Il2CppString* str)
    {
        return 0;
    }

    Il2CppChar* il2cpp_string_chars(Il2CppString* str)
    {
        return NULL;
    }

    // Same as il2cpp_string_new_wrapper, because other normally takes a domain
    Il2CppString* il2cpp_string_new(const char* str)
    {
        return NULL;
    }

    Il2CppString* il2cpp_string_new_wrapper(const char* str)
    {
        return NULL;
    }

    Il2CppString* il2cpp_string_new_utf16(const Il2CppChar* text, int32_t len)
    {
        return NULL;
    }

    Il2CppString* il2cpp_string_new_len(const char* str, uint32_t length)
    {
        return NULL;
    }

    Il2CppString* il2cpp_string_intern(Il2CppString* str)
    {
        return NULL;
    }

    Il2CppString* il2cpp_string_is_interned(Il2CppString* str)
    {
        return NULL;
    }

    // thread

    Il2CppThread* il2cpp_thread_current()
    {
        return NULL;
    }

    Il2CppThread* il2cpp_thread_attach(Il2CppDomain* domain)
    {
        return NULL;
    }

    void il2cpp_thread_detach(Il2CppThread* thread)
    {
    }

    Il2CppThread** il2cpp_thread_get_all_attached_threads(size_t* size)
    {
        return NULL;
    }

    bool il2cpp_is_vm_thread(Il2CppThread* thread)
    {
        return false;
    }

    // stacktrace

    void il2cpp_current_thread_walk_frame_stack(Il2CppFrameWalkFunc func, void* user_data)
    {
    }

    void il2cpp_thread_walk_frame_stack(Il2CppThread* thread, Il2CppFrameWalkFunc func, void* user_data)
    {
    }

    bool il2cpp_current_thread_get_top_frame(Il2CppStackFrameInfo* frame)
    {
        return false;
    }

    bool il2cpp_thread_get_top_frame(Il2CppThread* thread, Il2CppStackFrameInfo* frame)
    {
        return false;
    }

    bool il2cpp_current_thread_get_frame_at(int32_t offset, Il2CppStackFrameInfo* frame)
    {
        return false;
    }

    bool il2cpp_thread_get_frame_at(Il2CppThread* thread, int32_t offset, Il2CppStackFrameInfo* frame)
    {
        return false;
    }

    int32_t il2cpp_current_thread_get_stack_depth()
    {
        return 0;
    }

    int32_t il2cpp_thread_get_stack_depth(Il2CppThread* thread)
    {
        return 0;
    }

    void il2cpp_set_default_thread_affinity(int64_t affinity_mask)
    {
    }

    void il2cpp_override_stack_backtrace(Il2CppBacktraceFunc stackBacktraceFunc)
    {
    }

    // type

    Il2CppObject* il2cpp_type_get_object(const Il2CppType* type)
    {
        return NULL;
    }

    int il2cpp_type_get_type(const Il2CppType* type)
    {
        return 0;
    }

    Il2CppClass* il2cpp_type_get_class_or_element_class(const Il2CppType* type)
    {
        return NULL;
    }

    char* il2cpp_type_get_name(const Il2CppType* type)
    {
        return NULL;
    }

    char* il2cpp_type_get_assembly_qualified_name(const Il2CppType* type)
    {
        return NULL;
    }

    char* il2cpp_type_get_reflection_name(const Il2CppType* type)
    {
        return NULL;
    }

    bool il2cpp_type_is_byref(const Il2CppType* type)
    {
        return false;
    }

    uint32_t il2cpp_type_get_attrs(const Il2CppType* type)
    {
        return 0;
    }

    bool il2cpp_type_equals(const Il2CppType* type, const Il2CppType* otherType)
    {
        return false;
    }

    bool il2cpp_type_is_static(const Il2CppType* type)
    {
        return false;
    }

    bool il2cpp_type_is_pointer_type(const Il2CppType* type)
    {
        return false;
    }

    // image

    const Il2CppAssembly* il2cpp_image_get_assembly(const Il2CppImage* image)
    {
        return NULL;
    }

    const char* il2cpp_image_get_name(const Il2CppImage* image)
    {
        return NULL;
    }

    const char* il2cpp_image_get_filename(const Il2CppImage* image)
    {
        return NULL;
    }

    const MethodInfo* il2cpp_image_get_entry_point(const Il2CppImage* image)
    {
        return NULL;
    }

    size_t il2cpp_image_get_class_count(const Il2CppImage* image)
    {
        return 0;
    }

    const Il2CppClass* il2cpp_image_get_class(const Il2CppImage* image, size_t index)
    {
        return NULL;
    }

    Il2CppManagedMemorySnapshot* il2cpp_capture_memory_snapshot()
    {
        return NULL;
    }

    void il2cpp_free_captured_memory_snapshot(Il2CppManagedMemorySnapshot* snapshot)
    {
    }

    void il2cpp_set_find_plugin_callback(Il2CppSetFindPlugInCallback method)
    {
    }

    // Logging

    void il2cpp_register_log_callback(Il2CppLogCallback method)
    {
    }

    // Debugger
    void il2cpp_debugger_set_agent_options(const char* options)
    {
    }

    bool il2cpp_is_debugger_attached()
    {
        return false;
    }

    void il2cpp_register_debugger_agent_transport(Il2CppDebuggerTransport* debuggerTransport)
    {
    }

    bool il2cpp_debug_get_method_info(const MethodInfo* method, Il2CppMethodDebugInfo* methodDebugInfo)
    {
        return false;
    }

    void il2cpp_unity_install_unitytls_interface(const void* unitytlsInterfaceStruct)
    {
    }

    // Custom Attributes
    Il2CppCustomAttrInfo* il2cpp_custom_attrs_from_class(Il2CppClass* klass)
    {
        return NULL;
    }

    Il2CppCustomAttrInfo* il2cpp_custom_attrs_from_method(const MethodInfo* method)
    {
        return NULL;
    }

    Il2CppCustomAttrInfo* il2cpp_custom_attrs_from_field(const FieldInfo* field)
    {
        return NULL;
    }

    bool il2cpp_custom_attrs_has_attr(Il2CppCustomAttrInfo* ainfo, Il2CppClass* attr_klass)
    {
        return false;
    }

    Il2CppObject* il2cpp_custom_attrs_get_attr(Il2CppCustomAttrInfo* ainfo, Il2CppClass* attr_klass)
    {
        return NULL;
    }

    Il2CppArray* il2cpp_custom_attrs_construct(Il2CppCustomAttrInfo* ainfo)
    {
        return NULL;
    }

    void il2cpp_custom_attrs_free(Il2CppCustomAttrInfo* ainfo)
    {
    }

    void il2cpp_type_get_name_chunked(const Il2CppType* type, void (*chunkReportFunc)(void* data, void* userData), void* userData)
    {
    }

    void il2cpp_class_set_userdata(Il2CppClass* klass, void* userdata)
    {
    }

    int il2cpp_class_get_userdata_offset()
    {
        return 0;
    }

    void il2cpp_class_for_each(void (*klassReportFunc)(Il2CppClass* klass, void* userData), void* userData)
    {
    }

    // Android
    void il2cpp_unity_set_android_network_up_state_func(Il2CppAndroidUpStateFunc func)
    {
    }
}