#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "vm/runtime.h"
#include "vm/settings.h"
#include "vm/class.h"
#include "vm/object.h"
#include "vm/rt_array.h"
#include "vm/array_class.h"
#include "vm/field.h"
#include "vm/method.h"
#include "vm/rt_string.h"
#include "vm/reflection.h"
#include "vm/assembly.h"
#include "vm/rt_exception.h"
#include "vm/monitor.h"
#include "vm/property.h"
#include "vm/gc.h"
#include "vm/gchandle.h"
#include "vm/type.h"
#include "vm/appdomain.h"
#include "vm/rt_thread.h"
#include "vm/customattribute.h"
#include "vm/stacktrace.h"
#include "vm/internal_calls.h"
#include "metadata/module_def.h"

using namespace leanclr;

typedef leanclr::Utf16Char          Il2CppChar;
typedef metadata::RtClass           Il2CppClass;
typedef vm::RtArray                 Il2CppArray;
typedef vm::RtObject                Il2CppObject;
typedef vm::RtString                Il2CppString;
typedef vm::RtException             Il2CppException;
typedef metadata::RtModuleDef       Il2CppImage;
typedef metadata::RtAssembly        Il2CppAssembly;
typedef vm::RtAppDomain             Il2CppDomain;
typedef vm::RtThread                Il2CppThread;
typedef metadata::RtTypeSig         Il2CppType;
typedef vm::RtReflectionType        Il2CppReflectionType;
typedef vm::RtReflectionMethod      Il2CppReflectionMethod;
typedef metadata::RtFieldInfo       FieldInfo;
typedef metadata::RtMethodInfo      MethodInfo;
typedef metadata::RtPropertyInfo    PropertyInfo;
typedef metadata::RtEventInfo       EventInfo;

// Opaque types that have no leanclr equivalent (forward declarations).
struct Il2CppCustomAttrInfo;
struct Il2CppDebuggerTransport;
struct Il2CppManagedMemorySnapshot;
struct Il2CppMemoryCallbacks;
struct Il2CppMethodDebugInfo;
struct Il2CppProfiler;
struct Il2CppRuntimeStats;
struct Il2CppStackFrameInfo;

#if _MSC_VER
typedef wchar_t Il2CppNativeChar;
#else
typedef char Il2CppNativeChar;
#endif

typedef enum
{
    IL2CPP_PROFILE_NONE = 0,
    IL2CPP_PROFILE_ENTER_LEAVE = 1 << 12,
    IL2CPP_PROFILE_ALLOCATIONS = 1 << 7,
    IL2CPP_PROFILE_GC = 1 << 8,
    IL2CPP_PROFILE_FILEIO = 1 << 20,
    IL2CPP_PROFILE_THREADS = 1 << 9,
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
    IL2CPP_STAT_METHOD_COUNT,
    IL2CPP_STAT_CLASS_STATIC_DATA_SIZE,
    IL2CPP_STAT_GENERIC_INSTANCE_COUNT,
    IL2CPP_STAT_GENERIC_CLASS_COUNT,
    IL2CPP_STAT_INFLATED_METHOD_COUNT,
    IL2CPP_STAT_INFLATED_TYPE_COUNT,
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

typedef uintptr_t il2cpp_array_size_t;
#define ARRAY_LENGTH_AS_INT32(a) ((int32_t)(a))

typedef uint8_t (*Il2CppAndroidUpStateFunc)(const char* ifName, uint8_t* is_up);

extern leanclr::metadata::RtAotModulesData g_aot_modules_data;

// -- Helpers ------------------------------------------------------------------

// Convert RtString (UTF-16) to a freshly malloc'd ASCII/UTF-8 char*.
// Caller must free() the returned buffer.
static char* rt_string_to_cstr_alloc(Il2CppString* s)
{
    if (!s) return nullptr;
    int32_t len = vm::String::get_length(s);
    char* buf = static_cast<char*>(std::malloc(static_cast<size_t>(len) + 1));
    if (!buf) return nullptr;
    const Utf16Char* chars = vm::String::get_chars_ptr(s);
    for (int32_t i = 0; i < len; i++)
        buf[i] = static_cast<char>(chars[i]);
    buf[len] = '\0';
    return buf;
}

// GCHandle: il2cpp uses uint32_t handles; leanclr uses void*.
// On 32-bit wasm sizeof(void*) == sizeof(uint32_t), so this is lossless.
static inline uint32_t handle_to_u32(void* h)
{
    uint32_t v;
    std::memcpy(&v, &h, sizeof(v));
    return v;
}
static inline void* u32_to_handle(uint32_t v)
{
    void* h;
    std::memcpy(&h, &v, sizeof(h));
    return h;
}

// -- extern "C" ---------------------------------------------------------------

extern "C"
{
    // -- init / shutdown ------------------------------------------------------

    int il2cpp_init(const char* domain_name)
    {
        vm::Settings::set_aot_modules_data(&g_aot_modules_data);
        auto ret = vm::Runtime::initialize();
        return ret.is_ok() ? 0 : -1;
    }

    int il2cpp_init_utf16(const Il2CppChar* domain_name)
    {
        return il2cpp_init("");
    }

    void il2cpp_shutdown()
    {
        vm::Runtime::shutdown();
    }

    void il2cpp_set_config_dir(const char* config_path) { }
    void il2cpp_set_data_dir(const char* data_path) { }
    void il2cpp_set_temp_dir(const char* temp_dir) { }
    void il2cpp_set_commandline_arguments(int argc, const char* const argv[], const char* basedir) { }
    void il2cpp_set_commandline_arguments_utf16(int argc, const Il2CppChar* const argv[], const char* basedir) { }
    void il2cpp_set_config_utf16(const Il2CppChar* executablePath) { }
    void il2cpp_set_config(const char* executablePath) { }
    void il2cpp_set_memory_callbacks(Il2CppMemoryCallbacks* callbacks) { }
    void il2cpp_memory_pool_set_region_size(size_t size) { }

    size_t il2cpp_memory_pool_get_region_size()
    {
        return 0;
    }

    // -- corlib ---------------------------------------------------------------

    const Il2CppImage* il2cpp_get_corlib()
    {
        auto* ass = vm::Assembly::get_corlib();
        return ass ? ass->mod : nullptr;
    }

    // -- internal calls -------------------------------------------------------

    void il2cpp_add_internal_call(const char* name, Il2CppMethodPointer method)
    {
        // TODO: leanclr's InternalCalls::register_internal_call also requires an invoker
    }

    Il2CppMethodPointer il2cpp_resolve_icall(const char* name)
    {
        auto* reg = vm::InternalCalls::get_internal_call(name);
        return reg ? reinterpret_cast<Il2CppMethodPointer>(reg->func) : nullptr;
    }

    // -- memory ---------------------------------------------------------------

    void* il2cpp_alloc(size_t size)
    {
        return std::malloc(size);
    }

    void il2cpp_free(void* ptr)
    {
        std::free(ptr);
    }

    // -- array ----------------------------------------------------------------

    Il2CppClass* il2cpp_array_class_get(Il2CppClass* element_class, uint32_t rank)
    {
        auto result = vm::ArrayClass::get_array_class_from_element_klass(element_class, static_cast<uint8_t>(rank));
        return result.is_ok() ? result.unwrap() : nullptr;
    }

    uint32_t il2cpp_array_length(Il2CppArray* array)
    {
        return static_cast<uint32_t>(vm::Array::get_array_length(array));
    }

    uint32_t il2cpp_array_get_byte_length(Il2CppArray* array)
    {
        return static_cast<uint32_t>(vm::Array::get_array_byte_length(array));
    }

    Il2CppArray* il2cpp_array_new(Il2CppClass* elementTypeInfo, il2cpp_array_size_t length)
    {
        auto result = vm::Array::new_szarray_from_ele_klass(elementTypeInfo, static_cast<int32_t>(length));
        return result.is_ok() ? result.unwrap() : nullptr;
    }

    Il2CppArray* il2cpp_array_new_specific(Il2CppClass* arrayTypeInfo, il2cpp_array_size_t length)
    {
        auto result = vm::Array::new_szarray_from_array_klass(arrayTypeInfo, static_cast<int32_t>(length));
        return result.is_ok() ? result.unwrap() : nullptr;
    }

    Il2CppArray* il2cpp_array_new_full(Il2CppClass* array_class, il2cpp_array_size_t* lengths, il2cpp_array_size_t* lower_bounds)
    {
        auto result = vm::Array::new_mdarray_from_array_klass(
            array_class,
            reinterpret_cast<const int32_t*>(lengths),
            reinterpret_cast<const int32_t*>(lower_bounds));
        return result.is_ok() ? result.unwrap() : nullptr;
    }

    Il2CppClass* il2cpp_bounded_array_class_get(Il2CppClass* element_class, uint32_t rank, bool bounded)
    {
        // bounded (true) == multi-dimensional array with explicit lower bounds
        auto result = vm::ArrayClass::get_array_class_from_element_klass(element_class, static_cast<uint8_t>(rank));
        return result.is_ok() ? result.unwrap() : nullptr;
    }

    int il2cpp_array_element_size(const Il2CppClass* klass)
    {
        return static_cast<int>(vm::Array::get_array_element_size_by_klass(const_cast<Il2CppClass*>(klass)));
    }

    // -- assembly -------------------------------------------------------------

    const Il2CppImage* il2cpp_assembly_get_image(const Il2CppAssembly* assembly)
    {
        return assembly->mod;
    }

    // -- class ----------------------------------------------------------------

    const Il2CppType* il2cpp_class_enum_basetype(Il2CppClass* klass)
    {
        // The first instance field of an enum holds the underlying value.
        for (uint16_t i = 0; i < klass->field_count; i++)
        {
            if (vm::Field::is_instance(&klass->fields[i]))
                return klass->fields[i].type_sig;
        }
        return nullptr;
    }

    Il2CppClass* il2cpp_class_from_system_type(Il2CppReflectionType* type)
    {
        if (!type) return nullptr;
        auto result = vm::Class::get_class_from_typesig(type->type_handle);
        return result.is_ok() ? result.unwrap() : nullptr;
    }

    bool il2cpp_class_is_inited(const Il2CppClass* klass)
    {
        return vm::Class::is_initialized(const_cast<Il2CppClass*>(klass));
    }

    bool il2cpp_class_is_generic(const Il2CppClass* klass)
    {
        return vm::Class::is_generic(const_cast<Il2CppClass*>(klass));
    }

    bool il2cpp_class_is_inflated(const Il2CppClass* klass)
    {
        return vm::Class::is_generic_inst(const_cast<Il2CppClass*>(klass));
    }

    bool il2cpp_class_is_assignable_from(Il2CppClass* klass, Il2CppClass* oklass)
    {
        return vm::Class::is_assignable_from(oklass, klass);
    }

    bool il2cpp_class_is_subclass_of(Il2CppClass* klass, Il2CppClass* klassc, bool check_interfaces)
    {
        return vm::Class::is_subclass_of_initialized(klass, klassc, check_interfaces);
    }

    bool il2cpp_class_has_parent(Il2CppClass* klass, Il2CppClass* klassc)
    {
        return vm::Class::has_class_parent_fast(klass, klassc);
    }

    Il2CppClass* il2cpp_class_from_il2cpp_type(const Il2CppType* type)
    {
        auto result = vm::Class::get_class_from_typesig(type);
        return result.is_ok() ? result.unwrap() : nullptr;
    }

    Il2CppClass* il2cpp_class_from_name(const Il2CppImage* image, const char* namespaze, const char* name)
    {
        auto result = const_cast<Il2CppImage*>(image)->get_class_by_name2(namespaze, name, false, false);
        return result.is_ok() ? result.unwrap() : nullptr;
    }

    Il2CppClass* il2cpp_class_get_element_class(Il2CppClass* klass)
    {
        return klass->element_class;
    }

    // Iterator pattern: *iter stores the next index as uintptr_t cast to void*.
    // NULL == 0 == index 0 (start). Caller passes &iter initialised to NULL.

    const EventInfo* il2cpp_class_get_events(Il2CppClass* klass, void** iter)
    {
        uintptr_t idx = reinterpret_cast<uintptr_t>(*iter);
        if (idx >= klass->event_count) return nullptr;
        *iter = reinterpret_cast<void*>(idx + 1);
        return &klass->events[idx];
    }

    FieldInfo* il2cpp_class_get_fields(Il2CppClass* klass, void** iter)
    {
        uintptr_t idx = reinterpret_cast<uintptr_t>(*iter);
        if (idx >= klass->field_count) return nullptr;
        *iter = reinterpret_cast<void*>(idx + 1);
        return const_cast<FieldInfo*>(&klass->fields[idx]);
    }

    Il2CppClass* il2cpp_class_get_nested_types(Il2CppClass* klass, void** iter)
    {
        uintptr_t idx = reinterpret_cast<uintptr_t>(*iter);
        if (idx >= klass->nested_class_count) return nullptr;
        *iter = reinterpret_cast<void*>(idx + 1);
        return klass->nested_classes[idx];
    }

    Il2CppClass* il2cpp_class_get_interfaces(Il2CppClass* klass, void** iter)
    {
        uintptr_t idx = reinterpret_cast<uintptr_t>(*iter);
        if (idx >= klass->interface_count) return nullptr;
        *iter = reinterpret_cast<void*>(idx + 1);
        return klass->interfaces[idx];
    }

    const PropertyInfo* il2cpp_class_get_properties(Il2CppClass* klass, void** iter)
    {
        uintptr_t idx = reinterpret_cast<uintptr_t>(*iter);
        if (idx >= klass->property_count) return nullptr;
        *iter = reinterpret_cast<void*>(idx + 1);
        return &klass->properties[idx];
    }

    const PropertyInfo* il2cpp_class_get_property_from_name(Il2CppClass* klass, const char* name)
    {
        return vm::Class::get_property_for_name(klass, name, true);
    }

    FieldInfo* il2cpp_class_get_field_from_name(Il2CppClass* klass, const char* name)
    {
        return const_cast<FieldInfo*>(vm::Class::get_field_for_name(klass, name, true));
    }

    const MethodInfo* il2cpp_class_get_methods(Il2CppClass* klass, void** iter)
    {
        uintptr_t idx = reinterpret_cast<uintptr_t>(*iter);
        if (idx >= klass->method_count) return nullptr;
        *iter = reinterpret_cast<void*>(idx + 1);
        return klass->methods[idx];
    }

    const MethodInfo* il2cpp_class_get_method_from_name(Il2CppClass* klass, const char* name, int argsCount)
    {
        for (Il2CppClass* c = klass; c != nullptr; c = c->parent)
        {
            const MethodInfo* m;
            if (argsCount < 0)
                m = vm::Method::find_matched_method_in_class_by_name(c, name);
            else
                m = vm::Method::find_matched_method_in_class_by_name_and_param_count(c, name, static_cast<size_t>(argsCount));
            if (m) return m;
        }
        return nullptr;
    }

    const char* il2cpp_class_get_name(Il2CppClass* klass)
    {
        return klass->name;
    }

    const char* il2cpp_class_get_namespace(Il2CppClass* klass)
    {
        return klass->namespaze;
    }

    Il2CppClass* il2cpp_class_get_parent(Il2CppClass* klass)
    {
        return klass->parent;
    }

    Il2CppClass* il2cpp_class_get_declaring_type(Il2CppClass* klass)
    {
        return klass->declaring_class;
    }

    int32_t il2cpp_class_instance_size(Il2CppClass* klass)
    {
        return static_cast<int32_t>(vm::Class::get_instance_size_with_object_header(klass));
    }

    size_t il2cpp_class_num_fields(const Il2CppClass* klass)
    {
        return klass->field_count;
    }

    bool il2cpp_class_is_valuetype(const Il2CppClass* klass)
    {
        return vm::Class::is_value_type(const_cast<Il2CppClass*>(klass));
    }

    bool il2cpp_class_is_blittable(const Il2CppClass* klass)
    {
        return vm::Class::is_blittable(const_cast<Il2CppClass*>(klass));
    }

    int32_t il2cpp_class_value_size(Il2CppClass* klass, uint32_t* align)
    {
        if (align) *align = klass->alignment;
        return vm::Class::is_value_type(klass)
            ? static_cast<int32_t>(klass->instance_size_without_header)
            : static_cast<int32_t>(sizeof(void*));
    }

    int il2cpp_class_get_flags(const Il2CppClass* klass)
    {
        return static_cast<int>(klass->flags);
    }

    bool il2cpp_class_is_abstract(const Il2CppClass* klass)
    {
        return vm::Class::is_abstract(const_cast<Il2CppClass*>(klass));
    }

    bool il2cpp_class_is_interface(const Il2CppClass* klass)
    {
        return vm::Class::is_interface(const_cast<Il2CppClass*>(klass));
    }

    int il2cpp_class_array_element_size(const Il2CppClass* klass)
    {
        return static_cast<int>(vm::Array::get_array_element_size_by_klass(const_cast<Il2CppClass*>(klass)));
    }

    Il2CppClass* il2cpp_class_from_type(const Il2CppType* type)
    {
        auto result = vm::Class::get_class_from_typesig(type);
        return result.is_ok() ? result.unwrap() : nullptr;
    }

    const Il2CppType* il2cpp_class_get_type(Il2CppClass* klass)
    {
        return klass->by_val;
    }

    uint32_t il2cpp_class_get_type_token(Il2CppClass* klass)
    {
        return klass->token;
    }

    bool il2cpp_class_has_attribute(Il2CppClass* klass, Il2CppClass* attr_class)
    {
        auto result = vm::CustomAttribute::has_customattribute_on_class(klass, attr_class);
        return result.is_ok() && result.unwrap();
    }

    bool il2cpp_class_has_references(Il2CppClass* klass)
    {
        return vm::Class::get_has_references(klass);
    }

    bool il2cpp_class_is_enum(const Il2CppClass* klass)
    {
        return vm::Class::is_enum_type(const_cast<Il2CppClass*>(klass));
    }

    const Il2CppImage* il2cpp_class_get_image(Il2CppClass* klass)
    {
        return klass->image;
    }

    const char* il2cpp_class_get_assemblyname(const Il2CppClass* klass)
    {
        return klass->image->get_assembly_name().name;
    }

    int il2cpp_class_get_rank(const Il2CppClass* klass)
    {
        return static_cast<int>(vm::Class::get_rank(const_cast<Il2CppClass*>(klass)));
    }

    uint32_t il2cpp_class_get_data_size(const Il2CppClass* klass)
    {
        return klass->static_size;
    }

    void* il2cpp_class_get_static_field_data(const Il2CppClass* klass)
    {
        return klass->static_fields_data;
    }

    // testing only
    size_t il2cpp_class_get_bitmap_size(const Il2CppClass* klass)
    {
        // TODO: GC bitmap not exposed in leanclr
        return 0;
    }

    void il2cpp_class_get_bitmap(Il2CppClass* klass, size_t* bitmap)
    {
        // TODO: GC bitmap not exposed in leanclr
    }

    // -- stats ----------------------------------------------------------------

    bool il2cpp_stats_dump_to_file(const char* path)
    {
        // TODO: runtime stats not implemented
        return false;
    }

    uint64_t il2cpp_stats_get_value(Il2CppStat stat)
    {
        // TODO: runtime stats not implemented
        return 0;
    }

    // -- domain ---------------------------------------------------------------

    Il2CppDomain* il2cpp_domain_get()
    {
        return vm::AppDomain::get_default_appdomain();
    }

    const Il2CppAssembly* il2cpp_domain_assembly_open(Il2CppDomain* domain, const char* name)
    {
        auto result = vm::Assembly::load_by_name(name);
        return result.is_ok() ? result.unwrap() : nullptr;
    }

    const Il2CppAssembly** il2cpp_domain_get_assemblies(const Il2CppDomain* domain, size_t* size)
    {
        auto modules = metadata::RtModuleDef::get_registered_modules();
        *size = modules.size();
        if (modules.size() == 0) return nullptr;

        // Single-threaded wasm: a static buffer is safe.
        static metadata::RtAssembly** s_buf = nullptr;
        static size_t s_buf_cap = 0;
        if (s_buf_cap < modules.size())
        {
            std::free(s_buf);
            s_buf = static_cast<metadata::RtAssembly**>(std::malloc(modules.size() * sizeof(void*)));
            s_buf_cap = modules.size();
        }
        for (size_t i = 0; i < modules.size(); i++)
            s_buf[i] = modules[i]->get_assembly();
        return const_cast<const Il2CppAssembly**>(s_buf);
    }

    // -- exception ------------------------------------------------------------

    void il2cpp_raise_exception(Il2CppException* exc)
    {
        vm::Exception::set_current_exception(exc);
    }

    Il2CppException* il2cpp_exception_from_name_msg(const Il2CppImage* image, const char* name_space, const char* name, const char* msg)
    {
        // TODO: create a new instance of the specified exception class with the given message
        return nullptr;
    }

    Il2CppException* il2cpp_get_exception_argument_null(const char* arg)
    {
        // TODO: create ArgumentNullException
        return nullptr;
    }

    void il2cpp_format_exception(const Il2CppException* ex, char* message, int message_size)
    {
        if (!ex || message_size <= 0) return;
        const char* klass_name = ex->klass ? ex->klass->name : "Exception";
        std::snprintf(message, static_cast<size_t>(message_size), "%s", klass_name);
    }

    void il2cpp_format_stack_trace(const Il2CppException* ex, char* output, int output_size)
    {
        // TODO: format managed stack trace
        if (output_size > 0) output[0] = '\0';
    }

    void il2cpp_unhandled_exception(Il2CppException* exc)
    {
        vm::Exception::report_unhandled_exception(exc);
    }

    void il2cpp_native_stack_trace(const Il2CppException* ex, uintptr_t** addresses, int* numFrames, char** imageUUID, char** imageName)
    {
        // TODO: native stack trace not implemented
        *numFrames = 0;
        *addresses = nullptr;
        if (imageUUID) *imageUUID = nullptr;
        if (imageName) *imageName = nullptr;
    }

    // -- field ----------------------------------------------------------------

    const char* il2cpp_field_get_name(FieldInfo* field)
    {
        return field->name;
    }

    int il2cpp_field_get_flags(FieldInfo* field)
    {
        return static_cast<int>(field->flags);
    }

    Il2CppClass* il2cpp_field_get_parent(FieldInfo* field)
    {
        return field->parent;
    }

    size_t il2cpp_field_get_offset(FieldInfo* field)
    {
        return vm::Field::get_field_offset_includes_object_header_for_all_type(field);
    }

    const Il2CppType* il2cpp_field_get_type(FieldInfo* field)
    {
        return field->type_sig;
    }

    void il2cpp_field_get_value(Il2CppObject* obj, FieldInfo* field, void* value)
    {
        auto sizeResult = vm::Field::get_field_size(field);
        if (sizeResult.is_err()) return;
        size_t sz = sizeResult.unwrap();

        if (vm::Field::is_instance(field))
        {
            size_t offset = vm::Field::get_field_offset_includes_object_header_for_all_type(field);
            std::memcpy(value, reinterpret_cast<const uint8_t*>(obj) + offset, sz);
        }
        else
        {
            if (!field->parent->static_fields_data) return; // TODO: thread-static
            std::memcpy(value, field->parent->static_fields_data + field->offset, sz);
        }
    }

    Il2CppObject* il2cpp_field_get_value_object(FieldInfo* field, Il2CppObject* obj)
    {
        auto result = vm::Field::get_value_object(field, obj);
        return result.is_ok() ? result.unwrap() : nullptr;
    }

    bool il2cpp_field_has_attribute(FieldInfo* field, Il2CppClass* attr_class)
    {
        auto result = vm::CustomAttribute::has_customattribute_on_field(field, attr_class);
        return result.is_ok() && result.unwrap();
    }

    void il2cpp_field_set_value(Il2CppObject* obj, FieldInfo* field, void* value)
    {
        if (vm::Field::is_instance(field))
            vm::Field::set_instance_value(field, obj, value);
        else
            vm::Field::set_static_value(field, value);
    }

    void il2cpp_field_set_value_object(Il2CppObject* objectInstance, FieldInfo* field, Il2CppObject* value)
    {
        vm::Field::set_value_object(field, objectInstance, value);
    }

    void il2cpp_field_static_get_value(FieldInfo* field, void* value)
    {
        auto sizeResult = vm::Field::get_field_size(field);
        if (sizeResult.is_err()) return;
        if (!field->parent->static_fields_data) return; // TODO: thread-static
        std::memcpy(value, field->parent->static_fields_data + field->offset, sizeResult.unwrap());
    }

    void il2cpp_field_static_set_value(FieldInfo* field, void* value)
    {
        vm::Field::set_static_value(field, value);
    }

    bool il2cpp_field_is_literal(FieldInfo* field)
    {
        return vm::Field::is_static_literal(field);
    }

    // -- gc -------------------------------------------------------------------

    void il2cpp_gc_collect(int maxGenerations)
    {
        vm::GC::internal_collect(maxGenerations);
    }

    int32_t il2cpp_gc_collect_a_little()
    {
        vm::GC::internal_collect(0);
        return 0;
    }

    void il2cpp_gc_start_incremental_collection() { /* TODO */ }
    void il2cpp_gc_enable()                        { /* TODO */ }
    void il2cpp_gc_disable()                       { /* TODO */ }

    bool il2cpp_gc_is_disabled()
    {
        return false; // TODO
    }

    void il2cpp_gc_set_mode(Il2CppGCMode mode)  { /* TODO */ }

    bool il2cpp_gc_is_incremental()
    {
        return false;
    }

    int64_t il2cpp_gc_get_max_time_slice_ns()
    {
        return 0;
    }

    void il2cpp_gc_set_max_time_slice_ns(int64_t maxTimeSlice) { /* TODO */ }

    int64_t il2cpp_gc_get_used_size()
    {
        return vm::GC::get_total_memory(false);
    }

    int64_t il2cpp_gc_get_heap_size()
    {
        return vm::GC::get_total_memory(false);
    }

    void il2cpp_gc_foreach_heap(void (*func)(void* data, void* context), void* userData) { /* TODO */ }
    void il2cpp_stop_gc_world()  { /* TODO */ }
    void il2cpp_start_gc_world() { /* TODO */ }

    void* il2cpp_gc_alloc_fixed(size_t size)
    {
        // TODO: leanclr GC does not expose a pinned allocator; malloc as fallback
        return std::malloc(size);
    }

    void il2cpp_gc_free_fixed(void* address)
    {
        std::free(address);
    }

    // -- gchandle -------------------------------------------------------------

    uint32_t il2cpp_gchandle_new(Il2CppObject* obj, bool pinned)
    {
        // handle_type: 0=normal, 1=pinned
        return handle_to_u32(vm::GCHandle::get_target_handle(obj, nullptr, pinned ? 1 : 0));
    }

    uint32_t il2cpp_gchandle_new_weakref(Il2CppObject* obj, bool track_resurrection)
    {
        // handle_type: 2=weak, 3=weak+track resurrection
        return handle_to_u32(vm::GCHandle::get_target_handle(obj, nullptr, track_resurrection ? 3 : 2));
    }

    Il2CppObject* il2cpp_gchandle_get_target(uint32_t gchandle)
    {
        return vm::GCHandle::get_target(u32_to_handle(gchandle));
    }

    void il2cpp_gchandle_foreach_get_target(void (*func)(void*, void*), void* userData)
    {
        // TODO: strong handle enumeration not exposed
    }

    void il2cpp_gc_wbarrier_set_field(Il2CppObject* obj, void** targetAddress, void* object)
    {
        // TODO: write barrier not exposed; bare pointer store as fallback
        *targetAddress = object;
    }

    bool il2cpp_gc_has_strict_wbarriers()
    {
        return false;
    }

    void il2cpp_gc_set_external_allocation_tracker(void (*func)(void*, size_t, int)) { /* TODO */ }
    void il2cpp_gc_set_external_wbarrier_tracker(void (*func)(void**))               { /* TODO */ }

    void il2cpp_gchandle_free(uint32_t gchandle)
    {
        vm::GCHandle::free_handle(u32_to_handle(gchandle));
    }

    // -- vm runtime info ------------------------------------------------------

    uint32_t il2cpp_object_header_size()
    {
        return static_cast<uint32_t>(sizeof(vm::RtObject));
    }

    uint32_t il2cpp_array_object_header_size()
    {
        return static_cast<uint32_t>(offsetof(vm::RtArray, first_data));
    }

    uint32_t il2cpp_offset_of_array_length_in_array_object_header()
    {
        return static_cast<uint32_t>(offsetof(vm::RtArray, length));
    }

    uint32_t il2cpp_offset_of_array_bounds_in_array_object_header()
    {
        return static_cast<uint32_t>(offsetof(vm::RtArray, bounds));
    }

    uint32_t il2cpp_allocation_granularity()
    {
        return static_cast<uint32_t>(2 * sizeof(void*));
    }

    // -- liveness -------------------------------------------------------------

    void* il2cpp_unity_liveness_allocate_struct(Il2CppClass* filter, int max_object_count,
                                                il2cpp_register_object_callback callback, void* userdata,
                                                il2cpp_liveness_reallocate_callback reallocate)
    {
        // TODO: liveness analysis not implemented
        return nullptr;
    }

    void il2cpp_unity_liveness_calculation_from_root(Il2CppObject* root, void* state)   { /* TODO */ }
    void il2cpp_unity_liveness_calculation_from_statics(void* state)                    { /* TODO */ }
    void il2cpp_unity_liveness_finalize(void* state)                                    { /* TODO */ }
    void il2cpp_unity_liveness_free_struct(void* state)                                 { /* TODO */ }

    // -- method ---------------------------------------------------------------

    const Il2CppType* il2cpp_method_get_return_type(const MethodInfo* method)
    {
        return method->return_type;
    }

    const MethodInfo* il2cpp_method_get_from_reflection(const Il2CppReflectionMethod* method)
    {
        return method->method;
    }

    Il2CppReflectionMethod* il2cpp_method_get_object(const MethodInfo* method, Il2CppClass* refclass)
    {
        auto result = vm::Reflection::get_method_reflection_object(method, refclass);
        return result.is_ok() ? result.unwrap() : nullptr;
    }

    const char* il2cpp_method_get_name(const MethodInfo* method)
    {
        return method->name;
    }

    bool il2cpp_method_is_generic(const MethodInfo* method)
    {
        return method->generic_container != nullptr && method->generic_method == nullptr;
    }

    bool il2cpp_method_is_inflated(const MethodInfo* method)
    {
        return method->generic_method != nullptr;
    }

    bool il2cpp_method_is_instance(const MethodInfo* method)
    {
        return vm::Method::is_instance(method);
    }

    uint32_t il2cpp_method_get_param_count(const MethodInfo* method)
    {
        return static_cast<uint32_t>(method->parameter_count);
    }

    const Il2CppType* il2cpp_method_get_param(const MethodInfo* method, uint32_t index)
    {
        if (index >= method->parameter_count) return nullptr;
        return method->parameters[index];
    }

    Il2CppClass* il2cpp_method_get_class(const MethodInfo* method)
    {
        return method->parent;
    }

    bool il2cpp_method_has_attribute(const MethodInfo* method, Il2CppClass* attr_class)
    {
        auto result = vm::CustomAttribute::has_customattribute_on_method(method, attr_class);
        return result.is_ok() && result.unwrap();
    }

    Il2CppClass* il2cpp_method_get_declaring_type(const MethodInfo* method)
    {
        return method->parent;
    }

    uint32_t il2cpp_method_get_flags(const MethodInfo* method, uint32_t* iflags)
    {
        if (iflags) *iflags = method->iflags;
        return method->flags;
    }

    uint32_t il2cpp_method_get_token(const MethodInfo* method)
    {
        return method->token;
    }

    const char* il2cpp_method_get_param_name(const MethodInfo* method, uint32_t index)
    {
        auto tokenResult = vm::Method::get_parameter_token(method, static_cast<int32_t>(index));
        if (tokenResult.is_err()) return nullptr;
        auto optToken = tokenResult.unwrap();
        if (!optToken.has_value()) return nullptr;
        auto nameResult = vm::Method::get_parameter_name_by_token(method->parent->image, optToken.value());
        if (nameResult.is_err()) return nullptr;
        // Single-threaded wasm: a static buffer is safe.
        static char s_nameBuf[256];
        Il2CppString* rtStr = nameResult.unwrap();
        const Utf16Char* chars = vm::String::get_chars_ptr(rtStr);
        int32_t len = vm::String::get_length(rtStr);
        if (len > 255) len = 255;
        for (int32_t i = 0; i < len; i++) s_nameBuf[i] = static_cast<char>(chars[i]);
        s_nameBuf[len] = '\0';
        return s_nameBuf;
    }

    // -- profiler (no-op stubs) ------------------------------------------------

#if IL2CPP_ENABLE_PROFILER
    void il2cpp_profiler_install(Il2CppProfiler* prof, Il2CppProfileFunc shutdown_callback)               { }
    void il2cpp_profiler_set_events(Il2CppProfileFlags events)                                            { }
    void il2cpp_profiler_install_enter_leave(Il2CppProfileMethodFunc enter, Il2CppProfileMethodFunc flee) { }
    void il2cpp_profiler_install_allocation(Il2CppProfileAllocFunc callback)                              { }
    void il2cpp_profiler_install_gc(Il2CppProfileGCFunc cb, Il2CppProfileGCResizeFunc resize_cb)          { }
    void il2cpp_profiler_install_fileio(Il2CppProfileFileIOFunc callback)                                 { }
    void il2cpp_profiler_install_thread(Il2CppProfileThreadFunc start, Il2CppProfileThreadFunc end)       { }
#endif

    // -- property -------------------------------------------------------------

    const char* il2cpp_property_get_name(PropertyInfo* prop)
    {
        return prop->name;
    }

    const MethodInfo* il2cpp_property_get_get_method(PropertyInfo* prop)
    {
        return prop->get_method;
    }

    const MethodInfo* il2cpp_property_get_set_method(PropertyInfo* prop)
    {
        return prop->set_method;
    }

    Il2CppClass* il2cpp_property_get_parent(PropertyInfo* prop)
    {
        return prop->parent;
    }

    uint32_t il2cpp_property_get_flags(PropertyInfo* prop)
    {
        return prop->flags;
    }

    // -- object ---------------------------------------------------------------

    Il2CppClass* il2cpp_object_get_class(Il2CppObject* obj)
    {
        return obj->klass;
    }

    uint32_t il2cpp_object_get_size(Il2CppObject* obj)
    {
        Il2CppClass* klass = obj->klass;
        if (vm::Class::is_array_or_szarray(klass))
        {
            auto* arr = static_cast<vm::RtArray*>(obj);
            return static_cast<uint32_t>(offsetof(vm::RtArray, first_data) + vm::Array::get_array_byte_length(arr));
        }
        return vm::Class::get_instance_size_with_object_header(klass);
    }

    const MethodInfo* il2cpp_object_get_virtual_method(Il2CppObject* obj, const MethodInfo* method)
    {
        auto result = vm::Method::get_virtual_method_impl(obj, method);
        return result.is_ok() ? result.unwrap() : nullptr;
    }

    Il2CppObject* il2cpp_object_new(const Il2CppClass* klass)
    {
        auto result = vm::Object::new_object(const_cast<Il2CppClass*>(klass));
        return result.is_ok() ? result.unwrap() : nullptr;
    }

    void* il2cpp_object_unbox(Il2CppObject* obj)
    {
        return const_cast<void*>(vm::Object::get_box_value_type_data_ptr(obj));
    }

    Il2CppObject* il2cpp_value_box(Il2CppClass* klass, void* data)
    {
        auto result = vm::Object::box_object(klass, data);
        return result.is_ok() ? result.unwrap() : nullptr;
    }

    // -- monitor --------------------------------------------------------------

    void il2cpp_monitor_enter(Il2CppObject* obj)
    {
        vm::Monitor::enter(obj);
    }

    bool il2cpp_monitor_try_enter(Il2CppObject* obj, uint32_t timeout)
    {
        bool lock_taken = false;
        vm::Monitor::monitor_try_enter_with_atomic_var(obj, static_cast<int32_t>(timeout), &lock_taken);
        return lock_taken;
    }

    void il2cpp_monitor_exit(Il2CppObject* obj)
    {
        vm::Monitor::exit(obj);
    }

    void il2cpp_monitor_pulse(Il2CppObject* obj)
    {
        vm::Monitor::monitor_pulse(obj);
    }

    void il2cpp_monitor_pulse_all(Il2CppObject* obj)
    {
        vm::Monitor::monitor_pulse_all(obj);
    }

    void il2cpp_monitor_wait(Il2CppObject* obj)
    {
        vm::Monitor::monitor_wait(obj, -1);
    }

    bool il2cpp_monitor_try_wait(Il2CppObject* obj, uint32_t timeout)
    {
        return vm::Monitor::monitor_wait(obj, static_cast<int32_t>(timeout));
    }

    // -- runtime --------------------------------------------------------------

    Il2CppObject* il2cpp_runtime_invoke_convert_args(const MethodInfo* method, void* obj,
                                                     Il2CppObject** params, int paramCount,
                                                     Il2CppException** exc)
    {
        // TODO: convert boxed Il2CppObject** params to raw value pointers before invoking
        return nullptr;
    }

    Il2CppObject* il2cpp_runtime_invoke(const MethodInfo* method, void* obj,
                                        void** params, Il2CppException** exc)
    {
        auto result = vm::Runtime::invoke_with_run_cctor(
            method,
            static_cast<vm::RtObject*>(obj),
            reinterpret_cast<const void* const*>(params));
        if (result.is_err())
        {
            if (exc)
                *exc = vm::Exception::get_and_clear_current_exception();
            return nullptr;
        }
        return result.unwrap();
    }

    void il2cpp_runtime_class_init(Il2CppClass* klass)
    {
        vm::Runtime::run_class_static_constructor(klass);
    }

    void il2cpp_runtime_object_init(Il2CppObject* obj)
    {
        const MethodInfo* ctor = vm::Method::find_matched_method_in_class_by_name_and_param_count(obj->klass, ".ctor", 0);
        if (ctor)
            vm::Runtime::invoke_with_run_cctor(ctor, obj, nullptr);
    }

    void il2cpp_runtime_object_init_exception(Il2CppObject* obj, Il2CppException** exc)
    {
        const MethodInfo* ctor = vm::Method::find_matched_method_in_class_by_name_and_param_count(obj->klass, ".ctor", 0);
        if (!ctor) return;
        auto result = vm::Runtime::invoke_with_run_cctor(ctor, obj, nullptr);
        if (result.is_err() && exc)
            *exc = vm::Exception::get_and_clear_current_exception();
    }

    void il2cpp_runtime_unhandled_exception_policy_set(Il2CppRuntimeUnhandledExceptionPolicy value)
    {
        // TODO
    }

    // -- string ---------------------------------------------------------------

    int32_t il2cpp_string_length(Il2CppString* str)
    {
        return vm::String::get_length(str);
    }

    Il2CppChar* il2cpp_string_chars(Il2CppString* str)
    {
        return const_cast<Il2CppChar*>(vm::String::get_chars_ptr(str));
    }

    Il2CppString* il2cpp_string_new(const char* str)
    {
        return vm::String::create_string_from_utf8cstr(str);
    }

    Il2CppString* il2cpp_string_new_wrapper(const char* str)
    {
        return vm::String::create_string_from_utf8cstr(str);
    }

    Il2CppString* il2cpp_string_new_utf16(const Il2CppChar* text, int32_t len)
    {
        return vm::String::create_string_from_utf16chars(text, len);
    }

    Il2CppString* il2cpp_string_new_len(const char* str, uint32_t length)
    {
        return vm::String::create_string_from_utf8chars(str, static_cast<int32_t>(length));
    }

    Il2CppString* il2cpp_string_intern(Il2CppString* str)
    {
        return vm::String::intern_string(str);
    }

    Il2CppString* il2cpp_string_is_interned(Il2CppString* str)
    {
        return vm::String::is_interned_string(str) ? str : nullptr;
    }

    // -- thread ---------------------------------------------------------------

    Il2CppThread* il2cpp_thread_current()
    {
        return vm::Thread::get_current_thread();
    }

    Il2CppThread* il2cpp_thread_attach(Il2CppDomain* domain)
    {
        return vm::Thread::attach_current_thread(domain);
    }

    void il2cpp_thread_detach(Il2CppThread* thread)                          { /* TODO */ }

    Il2CppThread** il2cpp_thread_get_all_attached_threads(size_t* size)
    {
        // TODO: thread enumeration not exposed
        *size = 0;
        return nullptr;
    }

    bool il2cpp_is_vm_thread(Il2CppThread* thread)
    {
        return false; // TODO
    }

    // -- stacktrace -----------------------------------------------------------

    void il2cpp_current_thread_walk_frame_stack(Il2CppFrameWalkFunc func, void* user_data) { /* TODO */ }
    void il2cpp_thread_walk_frame_stack(Il2CppThread* thread, Il2CppFrameWalkFunc func, void* user_data) { /* TODO */ }

    bool il2cpp_current_thread_get_top_frame(Il2CppStackFrameInfo* frame)                 { return false; /* TODO */ }
    bool il2cpp_thread_get_top_frame(Il2CppThread* thread, Il2CppStackFrameInfo* frame)   { return false; /* TODO */ }
    bool il2cpp_current_thread_get_frame_at(int32_t offset, Il2CppStackFrameInfo* frame)  { return false; /* TODO */ }
    bool il2cpp_thread_get_frame_at(Il2CppThread* thread, int32_t offset, Il2CppStackFrameInfo* frame) { return false; /* TODO */ }

    int32_t il2cpp_current_thread_get_stack_depth()                        { return 0; /* TODO */ }
    int32_t il2cpp_thread_get_stack_depth(Il2CppThread* thread)            { return 0; /* TODO */ }

    void il2cpp_set_default_thread_affinity(int64_t affinity_mask)         { /* TODO */ }
    void il2cpp_override_stack_backtrace(Il2CppBacktraceFunc stackBacktraceFunc) { /* TODO */ }

    // -- type -----------------------------------------------------------------

    Il2CppObject* il2cpp_type_get_object(const Il2CppType* type)
    {
        auto result = vm::Reflection::get_type_reflection_object(type);
        // RtReflectionType starts with RtObject header as its first member.
        return result.is_ok() ? reinterpret_cast<Il2CppObject*>(result.unwrap()) : nullptr;
    }

    int il2cpp_type_get_type(const Il2CppType* type)
    {
        return static_cast<int>(type->ele_type);
    }

    Il2CppClass* il2cpp_type_get_class_or_element_class(const Il2CppType* type)
    {
        auto result = vm::Class::get_class_from_typesig(type);
        return result.is_ok() ? result.unwrap() : nullptr;
    }

    char* il2cpp_type_get_name(const Il2CppType* type)
    {
        auto result = vm::Type::get_full_name(type, true, false);
        return result.is_ok() ? rt_string_to_cstr_alloc(result.unwrap()) : nullptr;
    }

    char* il2cpp_type_get_assembly_qualified_name(const Il2CppType* type)
    {
        auto result = vm::Type::get_full_name(type, true, true);
        return result.is_ok() ? rt_string_to_cstr_alloc(result.unwrap()) : nullptr;
    }

    char* il2cpp_type_get_reflection_name(const Il2CppType* type)
    {
        auto result = vm::Type::get_full_name(type, true, false);
        return result.is_ok() ? rt_string_to_cstr_alloc(result.unwrap()) : nullptr;
    }

    bool il2cpp_type_is_byref(const Il2CppType* type)
    {
        return type->is_by_ref();
    }

    uint32_t il2cpp_type_get_attrs(const Il2CppType* type)
    {
        return type->field_or_param_attrs;
    }

    bool il2cpp_type_equals(const Il2CppType* type, const Il2CppType* otherType)
    {
        if (type == otherType) return true;
        if (type->ele_type != otherType->ele_type) return false;
        if (type->by_ref   != otherType->by_ref)   return false;
        return type->data.dummy == otherType->data.dummy;
    }

    bool il2cpp_type_is_static(const Il2CppType* type)
    {
        constexpr uint8_t kStaticBit = 0x10; // RtFieldAttribute::Static
        return (type->field_or_param_attrs & kStaticBit) != 0;
    }

    bool il2cpp_type_is_pointer_type(const Il2CppType* type)
    {
        return type->ele_type == metadata::RtElementType::Ptr;
    }

    // -- image ----------------------------------------------------------------

    const Il2CppAssembly* il2cpp_image_get_assembly(const Il2CppImage* image)
    {
        return image->get_assembly();
    }

    const char* il2cpp_image_get_name(const Il2CppImage* image)
    {
        return image->get_name();
    }

    const char* il2cpp_image_get_filename(const Il2CppImage* image)
    {
        return image->get_name();
    }

    const MethodInfo* il2cpp_image_get_entry_point(const Il2CppImage* image)
    {
        // TODO: look up entry-point method from token
        return nullptr;
    }

    size_t il2cpp_image_get_class_count(const Il2CppImage* image)
    {
        return image->get_class_count();
    }

    const Il2CppClass* il2cpp_image_get_class(const Il2CppImage* image, size_t index)
    {
        // TypeDef RIDs are 1-based.
        auto result = const_cast<Il2CppImage*>(image)->get_class_by_type_def_rid(static_cast<uint32_t>(index) + 1);
        return result.is_ok() ? result.unwrap() : nullptr;
    }

    // -- memory snapshot ------------------------------------------------------

    Il2CppManagedMemorySnapshot* il2cpp_capture_memory_snapshot()              { return nullptr; /* TODO */ }
    void il2cpp_free_captured_memory_snapshot(Il2CppManagedMemorySnapshot* s)  { /* TODO */ }

    // -- plugin / log / debugger ----------------------------------------------

    void il2cpp_set_find_plugin_callback(Il2CppSetFindPlugInCallback method)         { /* TODO */ }
    void il2cpp_register_log_callback(Il2CppLogCallback method)                      { /* TODO */ }
    void il2cpp_debugger_set_agent_options(const char* options)                      { /* TODO */ }

    bool il2cpp_is_debugger_attached()
    {
        return false;
    }

    void il2cpp_register_debugger_agent_transport(Il2CppDebuggerTransport* t)        { /* TODO */ }

    bool il2cpp_debug_get_method_info(const MethodInfo* method, Il2CppMethodDebugInfo* info)
    {
        return false; // TODO
    }

    void il2cpp_unity_install_unitytls_interface(const void* unitytlsInterfaceStruct) { /* TODO */ }

    // -- custom attributes ----------------------------------------------------
    // Il2CppCustomAttrInfo* is an opaque tagged pointer encoding the provider:
    //   bits[1:0] == 0 => Il2CppClass*
    //   bits[1:0] == 1 => const MethodInfo*
    //   bits[1:0] == 2 => const FieldInfo*

    Il2CppCustomAttrInfo* il2cpp_custom_attrs_from_class(Il2CppClass* klass)
    {
        return reinterpret_cast<Il2CppCustomAttrInfo*>(reinterpret_cast<uintptr_t>(klass) | 0u);
    }

    Il2CppCustomAttrInfo* il2cpp_custom_attrs_from_method(const MethodInfo* method)
    {
        return reinterpret_cast<Il2CppCustomAttrInfo*>(reinterpret_cast<uintptr_t>(method) | 1u);
    }

    Il2CppCustomAttrInfo* il2cpp_custom_attrs_from_field(const FieldInfo* field)
    {
        return reinterpret_cast<Il2CppCustomAttrInfo*>(reinterpret_cast<uintptr_t>(field) | 2u);
    }

    bool il2cpp_custom_attrs_has_attr(Il2CppCustomAttrInfo* ainfo, Il2CppClass* attr_klass)
    {
        uintptr_t raw = reinterpret_cast<uintptr_t>(ainfo);
        int tag = static_cast<int>(raw & 3u);
        void* ptr = reinterpret_cast<void*>(raw & ~uintptr_t(3));
        if (tag == 0)
        {
            auto result = vm::CustomAttribute::has_customattribute_on_class(
                reinterpret_cast<Il2CppClass*>(ptr), attr_klass);
            return result.is_ok() && result.unwrap();
        }
        else if (tag == 1)
        {
            auto result = vm::CustomAttribute::has_customattribute_on_method(
                reinterpret_cast<const MethodInfo*>(ptr), attr_klass);
            return result.is_ok() && result.unwrap();
        }
        else if (tag == 2)
        {
            auto result = vm::CustomAttribute::has_customattribute_on_field(
                reinterpret_cast<const FieldInfo*>(ptr), attr_klass);
            return result.is_ok() && result.unwrap();
        }
        return false;
    }

    Il2CppObject* il2cpp_custom_attrs_get_attr(Il2CppCustomAttrInfo* ainfo, Il2CppClass* attr_klass)
    {
        // TODO: retrieve a single attribute instance
        return nullptr;
    }

    Il2CppArray* il2cpp_custom_attrs_construct(Il2CppCustomAttrInfo* ainfo)
    {
        // TODO: construct all attribute instances
        return nullptr;
    }

    void il2cpp_custom_attrs_free(Il2CppCustomAttrInfo* ainfo)
    {
        // nothing to free - handles are encoded pointers
    }

    // -- type name chunked ----------------------------------------------------

    void il2cpp_type_get_name_chunked(const Il2CppType* type,
                                      void (*chunkReportFunc)(void* data, void* userData),
                                      void* userData)
    {
        // Delegate to il2cpp_type_get_name and report as a single chunk.
        char* name = il2cpp_type_get_name(type);
        if (name)
        {
            chunkReportFunc(name, userData);
            std::free(name);
        }
    }

    // -- class user data ------------------------------------------------------

    void il2cpp_class_set_userdata(Il2CppClass* klass, void* userdata)
    {
        // TODO: RtClass does not have a unity_user_data field
    }

    int il2cpp_class_get_userdata_offset()
    {
        // TODO: RtClass does not have a unity_user_data field
        return -1;
    }

    // -- class enumeration ----------------------------------------------------

    void il2cpp_class_for_each(void (*klassReportFunc)(Il2CppClass* klass, void* userData), void* userData)
    {
        auto modules = metadata::RtModuleDef::get_registered_modules();
        for (size_t mi = 0; mi < modules.size(); mi++)
        {
            auto* mod = modules[mi];
            uint32_t count = mod->get_class_count();
            for (uint32_t i = 0; i < count; i++)
            {
                auto result = mod->get_class_by_type_def_rid(i + 1);
                if (result.is_ok())
                    klassReportFunc(result.unwrap(), userData);
            }
        }
    }

    // -- android --------------------------------------------------------------

    void il2cpp_unity_set_android_network_up_state_func(Il2CppAndroidUpStateFunc func) { /* TODO */ }
}
