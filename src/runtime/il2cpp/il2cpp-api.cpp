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
#include "utils/string_builder.h"
#include "metadata/module_def.h"
#include "gc/garbage_collector.h"
#include "fileloader.h"
#include "statistic.h"

using namespace leanclr;

typedef leanclr::Utf16Char Il2CppChar;
typedef metadata::RtClass Il2CppClass;
typedef vm::RtArray Il2CppArray;
typedef vm::RtObject Il2CppObject;
typedef vm::RtString Il2CppString;
typedef vm::RtException Il2CppException;
typedef metadata::RtModuleDef Il2CppImage;
typedef metadata::RtAssembly Il2CppAssembly;
typedef vm::RtAppDomain Il2CppDomain;
typedef vm::RtThread Il2CppThread;
typedef metadata::RtTypeSig Il2CppType;
typedef vm::RtReflectionType Il2CppReflectionType;
typedef vm::RtReflectionMethod Il2CppReflectionMethod;
typedef metadata::RtFieldInfo FieldInfo;
typedef metadata::RtMethodInfo MethodInfo;
typedef metadata::RtPropertyInfo PropertyInfo;
typedef metadata::RtEventInfo EventInfo;
typedef il2cpp::Il2CppStat Il2CppStat;

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

typedef vm::GCMode Il2CppGCMode;

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

using Il2CppExceptionWrapper = vm::AotExceptionWrapper;

extern leanclr::metadata::RtAotModulesData g_aot_modules_data;

// -- Helpers ------------------------------------------------------------------

// Convert RtString (UTF-16) to a freshly malloc'd ASCII/UTF-8 char*.
// Caller must free() the returned buffer.
static char* rt_string_to_cstr_alloc(Il2CppString* s)
{
    if (!s)
        return nullptr;
    int32_t len = vm::String::get_length(s);
    char* buf = static_cast<char*>(std::malloc(static_cast<size_t>(len) + 1));
    if (!buf)
        return nullptr;
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

extern "C"
{
    int il2cpp_init(const char* domain_name)
    {
        setlocale(LC_ALL, "");
        vm::Settings::set_domain_name(domain_name);
        vm::Settings::set_file_loader(il2cpp::assembly_file_loader);
        vm::Settings::set_aot_modules_data(&g_aot_modules_data);
        auto ret = vm::Runtime::initialize();
        return static_cast<int>(ret.unwrap_err());
    }

    int il2cpp_init_utf16(const Il2CppChar* domain_name)
    {
        utils::StringBuilder sb;
        sb.append_utf16_str(domain_name, utils::StringUtil::get_utf16chars_length(domain_name));
        return il2cpp_init(sb.as_cstr());
    }

    void il2cpp_shutdown()
    {
        vm::Runtime::shutdown();
    }

    void il2cpp_set_config_dir(const char* config_path)
    {
        vm::Settings::set_config_dir(config_path);
    }

    void il2cpp_set_config_utf16(const Il2CppChar* executablePath)
    {
        utils::StringBuilder sb;
        sb.append_utf16_str(executablePath, utils::StringUtil::get_utf16chars_length(executablePath));
        vm::Settings::set_config_dir(sb.as_cstr());
    }

    void il2cpp_set_data_dir(const char* data_path)
    {
        vm::Settings::set_data_dir(data_path);
    }

    void il2cpp_set_temp_dir(const char* temp_dir)
    {
        vm::Settings::set_temp_dir(temp_dir);
    }

    void il2cpp_set_commandline_arguments(int argc, const char* const argv[], const char* basedir)
    {
        vm::Settings::set_command_line_arguments(argc, (const char**)argv);
    }

    void il2cpp_set_commandline_arguments_utf16(int argc, const Il2CppChar* const argv[], const char* basedir)
    {
        vm::Settings::set_command_line_arguments_utf16(argc, (const Il2CppChar**)argv);
    }

    // -- internal calls -------------------------------------------------------

    void il2cpp_add_internal_call(const char* name, Il2CppMethodPointer method)
    {
        vm::InternalCalls::register_il2cpp_internal_call(name, method);
    }

    Il2CppMethodPointer il2cpp_resolve_icall(const char* name)
    {
        return vm::InternalCalls::get_il2cpp_internal_call(name);
    }

    // -- memory ---------------------------------------------------------------

    void* il2cpp_alloc(size_t size)
    {
        return alloc::GeneralAllocation::malloc(size);
    }

    void il2cpp_free(void* ptr)
    {
        alloc::GeneralAllocation::free(ptr);
    }

    // -- array ----------------------------------------------------------------

    Il2CppClass* il2cpp_array_class_get(Il2CppClass* element_class, uint32_t rank)
    {
        auto result = vm::ArrayClass::get_array_class_from_element_klass(element_class, static_cast<uint8_t>(rank));
        assert(result.is_ok());
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
        assert(result.is_ok());
        return result.is_ok() ? result.unwrap() : nullptr;
    }

    Il2CppArray* il2cpp_array_new_specific(Il2CppClass* arrayTypeInfo, il2cpp_array_size_t length)
    {
        auto result = vm::Array::new_szarray_from_array_klass(arrayTypeInfo, static_cast<int32_t>(length));
        return result.is_ok() ? result.unwrap() : nullptr;
    }

    Il2CppArray* il2cpp_array_new_full(Il2CppClass* array_class, il2cpp_array_size_t* lengths, il2cpp_array_size_t* lower_bounds)
    {
        switch (array_class->by_val->ele_type)
        {
        case metadata::RtElementType::SZArray:
        {
            auto result = vm::Array::new_szarray_from_ele_klass(array_class, static_cast<int32_t>(lengths[0]));
            assert(result.is_ok());
            return result.is_ok() ? result.unwrap() : nullptr;
        }
        case metadata::RtElementType::Array:
        {
            auto result =
                vm::Array::new_mdarray_from_array_klass(array_class, reinterpret_cast<const int32_t*>(lengths), reinterpret_cast<const int32_t*>(lower_bounds));
            assert(result.is_ok());
            return result.is_ok() ? result.unwrap() : nullptr;
        }
        default:
        {
            assert(false);
            return nullptr;
        }
        }
    }

    Il2CppClass* il2cpp_bounded_array_class_get(Il2CppClass* element_class, uint32_t rank, bool bounded)
    {
        if (rank == 1 && !bounded)
        {
            auto result = vm::ArrayClass::get_szarray_class_from_element_class(element_class);
            assert(result.is_ok());
            return result.is_ok() ? result.unwrap() : nullptr;
        }
        else
        {
            // bounded (true) == multi-dimensional array with explicit lower bounds
            auto result = vm::ArrayClass::get_array_class_from_element_klass(element_class, static_cast<uint8_t>(rank));
            assert(result.is_ok());
            return result.is_ok() ? result.unwrap() : nullptr;
        }
    }

    int il2cpp_array_element_size(const Il2CppClass* klass)
    {
        return static_cast<int>(vm::Array::get_array_element_size_by_klass(klass));
    }

    // -- assembly -------------------------------------------------------------

    const Il2CppImage* il2cpp_assembly_get_image(const Il2CppAssembly* assembly)
    {
        return assembly->mod;
    }

    // -- class ----------------------------------------------------------------

    const Il2CppType* il2cpp_class_enum_basetype(Il2CppClass* klass)
    {
        if (klass == klass->element_class)
        {
            return nullptr;
        }
        else
        {
            return klass->element_class->by_val;
        }
    }

    Il2CppClass* il2cpp_class_from_system_type(Il2CppReflectionType* type)
    {
        assert(type);
        auto result = vm::Class::get_class_from_typesig(type->type_handle);
        assert(result.is_ok());
        if (result.is_err())
        {
            assert(false && "Failed to get class from typesig in il2cpp_class_from_system_type");
            return nullptr;
        }
        Il2CppClass* il2cpp_class = result.unwrap();
        auto ret2 = vm::Class::initialize_all(il2cpp_class);
        assert(ret2.is_ok());
        return ret2.is_ok() ? il2cpp_class : nullptr;
    }

    bool il2cpp_class_is_inited(const Il2CppClass* klass)
    {
        return vm::Class::is_initialized(klass);
    }

    bool il2cpp_class_is_generic(const Il2CppClass* klass)
    {
        return vm::Class::is_generic(klass);
    }

    bool il2cpp_class_is_inflated(const Il2CppClass* klass)
    {
        return vm::Class::is_generic_inst(klass);
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
        return const_cast<Il2CppClass*>(klass->element_class);
    }

    // Iterator pattern: *iter stores the next index as uintptr_t cast to void*.
    // NULL == 0 == index 0 (start). Caller passes &iter initialised to NULL.

    const EventInfo* il2cpp_class_get_events(Il2CppClass* klass, void** iter)
    {
        if (!iter)
        {
            return nullptr;
        }
        if (!*iter)
        {
            auto ret = vm::Class::initialize_events(klass);
            if (ret.is_err())
            {
                return nullptr;
            }
            *iter = (void*)(klass->events);
            return klass->events;
        }
        const EventInfo* next_event = (const EventInfo*)(*iter) + 1;
        if (next_event < klass->events + klass->event_count)
        {
            *iter = (void*)next_event;
            return next_event;
        }
        return nullptr;
    }

    FieldInfo* il2cpp_class_get_fields(Il2CppClass* klass, void** iter)
    {
        if (!iter)
        {
            return nullptr;
        }
        if (!*iter)
        {
            auto ret = vm::Class::initialize_fields(klass);
            if (ret.is_err())
            {
                return nullptr;
            }
            *iter = (void*)(klass->fields);
            return const_cast<FieldInfo*>(klass->fields);
        }
        const FieldInfo* next_field = (const FieldInfo*)(*iter) + 1;
        if (next_field < klass->fields + klass->field_count)
        {
            *iter = (void*)next_field;
            return const_cast<FieldInfo*>(next_field);
        }
        return nullptr;
    }

    Il2CppClass* il2cpp_class_get_nested_types(Il2CppClass* klass, void** iter)
    {
        if (!iter)
        {
            return nullptr;
        }
        if (!*iter)
        {
            auto ret = vm::Class::initialize_nested_classes(klass);
            if (ret.is_err())
            {
                return nullptr;
            }
            *iter = (void*)klass->nested_classes;
            if (klass->nested_class_count == 0)
            {
                return nullptr;
            }
            return const_cast<Il2CppClass*>(klass->nested_classes[0]);
        }
        const Il2CppClass** next_nested_class_ptr = (const Il2CppClass**)(*iter) + 1;
        if (next_nested_class_ptr < klass->nested_classes + klass->nested_class_count)
        {
            *iter = (void*)next_nested_class_ptr;
            return const_cast<Il2CppClass*>(*next_nested_class_ptr);
        }
        return nullptr;
    }

    Il2CppClass* il2cpp_class_get_interfaces(Il2CppClass* klass, void** iter)
    {
        if (!iter)
        {
            return nullptr;
        }
        if (!*iter)
        {
            auto ret = vm::Class::initialize_interfaces(klass);
            if (ret.is_err())
            {
                return nullptr;
            }
            *iter = (void*)klass->interfaces;
            if (klass->interface_count == 0)
            {
                return nullptr;
            }
            return const_cast<Il2CppClass*>(klass->interfaces[0]);
        }
        const Il2CppClass** next_interface_ptr = (const Il2CppClass**)(*iter) + 1;
        if (next_interface_ptr < klass->interfaces + klass->interface_count)
        {
            *iter = (void*)next_interface_ptr;
            return const_cast<Il2CppClass*>(*next_interface_ptr);
        }
        return nullptr;
    }

    const PropertyInfo* il2cpp_class_get_properties(Il2CppClass* klass, void** iter)
    {
        if (!iter)
        {
            return nullptr;
        }
        if (!*iter)
        {
            auto ret = vm::Class::initialize_properties(klass);
            if (ret.is_err())
            {
                return nullptr;
            }
            if (klass->property_count == 0)
            {
                return nullptr;
            }
            *iter = (void*)(klass->properties);
            return klass->properties;
        }
        const PropertyInfo* next_property = (const PropertyInfo*)(*iter) + 1;
        if (next_property < klass->properties + klass->property_count)
        {
            *iter = (void*)next_property;
            return const_cast<PropertyInfo*>(next_property);
        }
        return nullptr;
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
        if (!iter)
        {
            return nullptr;
        }
        if (!*iter)
        {
            auto ret = vm::Class::initialize_methods(klass);
            if (ret.is_err())
            {
                return nullptr;
            }
            if (klass->method_count == 0)
            {
                return nullptr;
            }
            *iter = (void*)(klass->methods);
            return const_cast<MethodInfo*>(klass->methods[0]);
        }
        const MethodInfo** next_method_ptr = (const MethodInfo**)(*iter) + 1;
        if (next_method_ptr < klass->methods + klass->method_count)
        {
            *iter = (void*)next_method_ptr;
            return const_cast<MethodInfo*>(*next_method_ptr);
        }
        return nullptr;
    }

    const MethodInfo* il2cpp_class_get_method_from_name(Il2CppClass* klass, const char* name, int argsCount)
    {
        for (const Il2CppClass* c = klass; c != nullptr; c = c->parent)
        {
            const MethodInfo* m;
            if (argsCount < 0)
                m = vm::Method::find_matched_method_in_class_by_name(c, name);
            else
                m = vm::Method::find_matched_method_in_class_by_name_and_param_count(c, name, static_cast<size_t>(argsCount));
            if (m)
                return m;
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
        return const_cast<Il2CppClass*>(klass->parent);
    }

    Il2CppClass* il2cpp_class_get_declaring_type(Il2CppClass* klass)
    {
        return const_cast<Il2CppClass*>(klass->declaring_class);
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
        return vm::Class::is_value_type(klass);
    }

    bool il2cpp_class_is_blittable(const Il2CppClass* klass)
    {
        return vm::Class::is_blittable(klass);
    }

    int32_t il2cpp_class_value_size(Il2CppClass* klass, uint32_t* align)
    {
        auto ret = vm::Class::initialize_fields(klass);
        if (ret.is_err())
        {
            if (align)
                *align = 0;
            return 0;
        }
        if (align)
            *align = klass->alignment;
        return vm::Class::get_instance_size_without_object_header(klass);
    }

    int il2cpp_class_get_flags(const Il2CppClass* klass)
    {
        return static_cast<int>(klass->flags);
    }

    bool il2cpp_class_is_abstract(const Il2CppClass* klass)
    {
        return vm::Class::is_abstract(klass);
    }

    bool il2cpp_class_is_interface(const Il2CppClass* klass)
    {
        return vm::Class::is_interface(klass);
    }

    int il2cpp_class_array_element_size(const Il2CppClass* klass)
    {
        return static_cast<int>(vm::Array::get_array_element_size_by_klass(klass));
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
        return vm::Class::is_enum_type(klass);
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
        return static_cast<int>(vm::Class::get_rank(klass));
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
        return vm::Class::get_gc_bitmap_size(klass);
    }

    void il2cpp_class_get_bitmap(Il2CppClass* klass, size_t* bitmap)
    {
        size_t bitmap_size = 0;
        vm::Class::get_gc_bitmap(klass, bitmap, bitmap_size);
    }

    // -- stats ----------------------------------------------------------------

    bool il2cpp_stats_dump_to_file(const char* path)
    {
        return il2cpp::Statistic::dump_to_file(path);
    }

    uint64_t il2cpp_stats_get_value(Il2CppStat stat)
    {
        return il2cpp::Statistic::get_value(stat);
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

    const Il2CppAssembly** s_cached_assemblies = nullptr;
    size_t s_cached_assemblies_size = 0;

    const Il2CppAssembly** il2cpp_domain_get_assemblies(const Il2CppDomain* domain, size_t* size)
    {
        auto modules = metadata::RtModuleDef::get_registered_modules();
        if (modules.size() != s_cached_assemblies_size)
        {
            // we don't free last cached assemblies, because they are still in use
            // if (s_cached_assemblies)
            // {
            //     std::free(s_cached_assemblies);
            // }
            s_cached_assemblies = alloc::GeneralAllocation::calloc_any<const Il2CppAssembly*>(modules.size());
            s_cached_assemblies_size = modules.size();
            for (size_t i = 0; i < modules.size(); i++)
                s_cached_assemblies[i] = modules[i]->get_assembly();
        }
        *size = s_cached_assemblies_size;
        return s_cached_assemblies;
    }

    // -- exception ------------------------------------------------------------

    void il2cpp_raise_exception(Il2CppException* exc)
    {
        vm::Exception::raise_as_cpp_exception(exc);
    }

    Il2CppException* il2cpp_exception_from_name_msg(const Il2CppImage* image, const char* name_space, const char* name, const char* msg)
    {
        metadata::RtModuleDef* mod = const_cast<metadata::RtModuleDef*>(image);
        auto ret = mod->get_class_by_name2(name_space, name, false, false);
        if (ret.is_err())
        {
            return nullptr;
        }
        metadata::RtClass* klass = ret.unwrap();
        auto ex_ret = vm::Object::new_object(klass);
        if (ex_ret.is_err())
        {
            return nullptr;
        }
        vm::RtException* ex = reinterpret_cast<vm::RtException*>(ex_ret.unwrap());
        ex->message = vm::String::create_string_from_utf8cstr(msg);
        return ex;
    }

    Il2CppException* il2cpp_get_exception_argument_null(const char* arg)
    {
        metadata::RtClass* ex_class = vm::Class::get_corlib_types().cls_argument_null_exception;
        auto ex_ret = vm::Object::new_object(ex_class);
        if (ex_ret.is_err())
        {
            return nullptr;
        }
        vm::RtException* ex = reinterpret_cast<vm::RtException*>(ex_ret.unwrap());
        ex->message = vm::String::create_string_from_utf8cstr(arg);
        return ex;
    }

    void il2cpp_format_exception(const Il2CppException* ex, char* message, int message_size)
    {
        utils::StringBuilder sb;
        vm::Exception::format_exception(const_cast<vm::RtException*>(ex), sb);
        size_t copy_size = std::min(sb.length(), static_cast<size_t>(message_size) - 1);
        std::memcpy(message, sb.as_cstr(), copy_size);
        message[copy_size] = '\0';
    }

    void il2cpp_format_stack_trace(const Il2CppException* ex, char* output, int output_size)
    {
        utils::StringBuilder sb;
        if (ex->stack_trace)
        {
            sb.append_utf16_str(vm::String::get_chars_ptr(ex->stack_trace), vm::String::get_length(ex->stack_trace));
        }
        size_t copy_size = std::min(sb.length(), static_cast<size_t>(output_size) - 1);
        std::memcpy(output, sb.as_cstr(), copy_size);
        output[copy_size] = '\0';
    }

    void il2cpp_unhandled_exception(Il2CppException* exc)
    {
        vm::Exception::report_unhandled_exception(exc);
    }

    void il2cpp_native_stack_trace(const Il2CppException* ex, uintptr_t** addresses, int* numFrames, char** imageUUID, char** imageName)
    {
        *numFrames = 0;
        *addresses = nullptr;
        *imageUUID = nullptr;
        *imageName = nullptr;
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
        assert(vm::Field::is_instance(field));
        auto ret = vm::Field::get_instance_value(field, obj, value);
        assert(ret.is_ok());
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
        assert(vm::Field::is_instance(field));
        auto ret = vm::Field::set_instance_value(field, obj, value);
        assert(ret.is_ok());
    }

    void il2cpp_field_set_value_object(Il2CppObject* objectInstance, FieldInfo* field, Il2CppObject* value)
    {
        assert(vm::Field::is_instance(field));
        auto ret = vm::Field::set_value_object(field, objectInstance, value);
        assert(ret.is_ok());
    }

    void il2cpp_field_static_get_value(FieldInfo* field, void* value)
    {
        auto ret = vm::Field::get_static_value(field, value);
        assert(ret.is_ok());
    }

    void il2cpp_field_static_set_value(FieldInfo* field, void* value)
    {
        auto ret = vm::Field::set_static_value(field, value);
        assert(ret.is_ok());
    }

    bool il2cpp_field_is_literal(FieldInfo* field)
    {
        return vm::Field::is_static_literal(field);
    }

    // -- gc -------------------------------------------------------------------

    void il2cpp_gc_collect(int maxGenerations)
    {
        vm::GC::collect(static_cast<int32_t>(maxGenerations));
    }

    int32_t il2cpp_gc_collect_a_little()
    {
        return vm::GC::collect_a_little();
    }

    void il2cpp_gc_start_incremental_collection()
    {
        vm::GC::start_incremental_collection();
    }
    void il2cpp_gc_enable()
    {
        vm::GC::enable();
    }
    void il2cpp_gc_disable()
    {
        vm::GC::disable();
    }

    bool il2cpp_gc_is_disabled()
    {
        return vm::GC::is_disabled();
    }

    void il2cpp_gc_set_mode(Il2CppGCMode mode)
    {
        vm::GC::set_mode(mode);
    }

    bool il2cpp_gc_is_incremental()
    {
        return vm::GC::is_incremental();
    }

    int64_t il2cpp_gc_get_max_time_slice_ns()
    {
        return vm::GC::get_max_time_slice_ns();
    }

    void il2cpp_gc_set_max_time_slice_ns(int64_t maxTimeSlice)
    {
        vm::GC::set_max_time_slice_ns(maxTimeSlice);
    }

    int64_t il2cpp_gc_get_used_size()
    {
        return vm::GC::get_used_size();
    }

    int64_t il2cpp_gc_get_heap_size()
    {
        return vm::GC::get_heap_size();
    }

    void il2cpp_gc_foreach_heap(void (*func)(void* data, void* context), void* userData)
    {
        vm::GC::foreach_heap(func, userData);
    }

    void il2cpp_stop_gc_world()
    {
        vm::GC::stop_gc_world();
    }

    void il2cpp_start_gc_world()
    {
        vm::GC::start_gc_world();
    }

    void* il2cpp_gc_alloc_fixed(size_t size)
    {
        return vm::GC::alloc_fixed(size);
    }

    void il2cpp_gc_free_fixed(void* address)
    {
        vm::GC::free_fixed(address);
    }

    // -- gchandle -------------------------------------------------------------

    uint32_t il2cpp_gchandle_new(Il2CppObject* obj, bool pinned)
    {
        void* handle = vm::GCHandle::new_handle(obj, pinned);
        return vm::GCHandle::get_handle_id(handle);
    }

    uint32_t il2cpp_gchandle_new_weakref(Il2CppObject* obj, bool track_resurrection)
    {
        void* handle = vm::GCHandle::new_weakref_handle(obj, track_resurrection);
        return vm::GCHandle::get_handle_id(handle);
    }

    Il2CppObject* il2cpp_gchandle_get_target(uint32_t gchandle)
    {
        void* handle = vm::GCHandle::get_handle_by_id(gchandle);
        if (handle == nullptr)
        {
            return nullptr;
        }
        return vm::GCHandle::get_target(handle);
    }

    void il2cpp_gchandle_foreach_get_target(void (*func)(void*, void*), void* userData)
    {
        vm::GCHandle::foreach_strong_handles(func, userData);
    }

    void il2cpp_gc_wbarrier_set_field(Il2CppObject* obj, void** targetAddress, void* object)
    {
        (void)obj;
        vm::GC::write_barrier(reinterpret_cast<vm::RtObject**>(targetAddress), reinterpret_cast<vm::RtObject*>(object));
    }

    bool il2cpp_gc_has_strict_wbarriers()
    {
        return vm::GC::has_strict_wbarriers();
    }

    void il2cpp_gc_set_external_allocation_tracker(void (*func)(void*, size_t, int))
    {
        vm::GC::set_external_allocation_tracker(func);
    }

    void il2cpp_gc_set_external_wbarrier_tracker(void (*func)(void**))
    {
        vm::GC::set_external_wbarrier_tracker(func);
    }

    void il2cpp_gchandle_free(uint32_t gchandle)
    {
        void* handle = vm::GCHandle::get_handle_by_id(gchandle);
        if (handle == nullptr)
        {
            return;
        }
        vm::GCHandle::free_handle(handle);
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

    void* il2cpp_unity_liveness_allocate_struct(Il2CppClass* filter, int max_object_count, il2cpp_register_object_callback callback, void* userdata,
                                                il2cpp_liveness_reallocate_callback reallocate)
    {
        // TODO: liveness analysis not implemented
        return nullptr;
    }

    void il2cpp_unity_liveness_calculation_from_root(Il2CppObject* root, void* state)
    { /* TODO */
    }
    void il2cpp_unity_liveness_calculation_from_statics(void* state)
    { /* TODO */
    }
    void il2cpp_unity_liveness_finalize(void* state)
    { /* TODO */
    }
    void il2cpp_unity_liveness_free_struct(void* state)
    { /* TODO */
    }

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
        if (index >= method->parameter_count)
            return nullptr;
        return method->parameters[index];
    }

    Il2CppClass* il2cpp_method_get_class(const MethodInfo* method)
    {
        return const_cast<Il2CppClass*>(method->parent);
    }

    bool il2cpp_method_has_attribute(const MethodInfo* method, Il2CppClass* attr_class)
    {
        auto result = vm::CustomAttribute::has_customattribute_on_method(method, attr_class);
        return result.is_ok() && result.unwrap();
    }

    Il2CppClass* il2cpp_method_get_declaring_type(const MethodInfo* method)
    {
        return const_cast<Il2CppClass*>(method->parent);
    }

    uint32_t il2cpp_method_get_flags(const MethodInfo* method, uint32_t* iflags)
    {
        if (iflags)
            *iflags = method->iflags;
        return method->flags;
    }

    uint32_t il2cpp_method_get_token(const MethodInfo* method)
    {
        return method->token;
    }

    const char* il2cpp_method_get_param_name(const MethodInfo* method, uint32_t index)
    {
        auto tokenResult = vm::Method::get_parameter_token(method, static_cast<int32_t>(index));
        if (tokenResult.is_err())
            return nullptr;
        auto optToken = tokenResult.unwrap();
        if (!optToken.has_value())
            return nullptr;
        auto nameResult = vm::Method::get_parameter_name_by_token(method->parent->image, optToken.value());
        if (nameResult.is_err())
            return nullptr;
        // Single-threaded wasm: a static buffer is safe.
        static char s_nameBuf[256];
        Il2CppString* rtStr = nameResult.unwrap();
        const Utf16Char* chars = vm::String::get_chars_ptr(rtStr);
        int32_t len = vm::String::get_length(rtStr);
        if (len > 255)
            len = 255;
        for (int32_t i = 0; i < len; i++)
            s_nameBuf[i] = static_cast<char>(chars[i]);
        s_nameBuf[len] = '\0';
        return s_nameBuf;
    }

    // -- profiler (no-op stubs) ------------------------------------------------

#if IL2CPP_ENABLE_PROFILER
    void il2cpp_profiler_install(Il2CppProfiler* prof, Il2CppProfileFunc shutdown_callback)
    {
    }
    void il2cpp_profiler_set_events(Il2CppProfileFlags events)
    {
    }
    void il2cpp_profiler_install_enter_leave(Il2CppProfileMethodFunc enter, Il2CppProfileMethodFunc flee)
    {
    }
    void il2cpp_profiler_install_allocation(Il2CppProfileAllocFunc callback)
    {
    }
    void il2cpp_profiler_install_gc(Il2CppProfileGCFunc cb, Il2CppProfileGCResizeFunc resize_cb)
    {
    }
    void il2cpp_profiler_install_fileio(Il2CppProfileFileIOFunc callback)
    {
    }
    void il2cpp_profiler_install_thread(Il2CppProfileThreadFunc start, Il2CppProfileThreadFunc end)
    {
    }
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
        return const_cast<Il2CppClass*>(prop->parent);
    }

    uint32_t il2cpp_property_get_flags(PropertyInfo* prop)
    {
        return prop->flags;
    }

    // -- object ---------------------------------------------------------------

    Il2CppClass* il2cpp_object_get_class(Il2CppObject* obj)
    {
        return const_cast<Il2CppClass*>(obj->klass);
    }

    uint32_t il2cpp_object_get_size(Il2CppObject* obj)
    {
        const Il2CppClass* klass = obj->klass;
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
        auto result = vm::Object::new_object(klass);
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

    Il2CppObject* il2cpp_runtime_invoke_convert_args(const MethodInfo* method, void* obj, Il2CppObject** params, int paramCount, Il2CppException** exc)
    {
        // TODO: convert boxed Il2CppObject** params to raw value pointers before invoking
        return nullptr;
    }

    Il2CppObject* il2cpp_runtime_invoke(const MethodInfo* method, void* obj, void** params, Il2CppException** exc)
    {
        auto result = vm::Runtime::invoke_with_run_cctor(method, static_cast<vm::RtObject*>(obj), reinterpret_cast<const void* const*>(params));
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
        if (!ctor)
            return;
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

    void il2cpp_thread_detach(Il2CppThread* thread)
    { /* TODO */
    }

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

    void il2cpp_current_thread_walk_frame_stack(Il2CppFrameWalkFunc func, void* user_data)
    { /* TODO */
    }
    void il2cpp_thread_walk_frame_stack(Il2CppThread* thread, Il2CppFrameWalkFunc func, void* user_data)
    { /* TODO */
    }

    bool il2cpp_current_thread_get_top_frame(Il2CppStackFrameInfo* frame)
    {
        return false; /* TODO */
    }
    bool il2cpp_thread_get_top_frame(Il2CppThread* thread, Il2CppStackFrameInfo* frame)
    {
        return false; /* TODO */
    }
    bool il2cpp_current_thread_get_frame_at(int32_t offset, Il2CppStackFrameInfo* frame)
    {
        return false; /* TODO */
    }
    bool il2cpp_thread_get_frame_at(Il2CppThread* thread, int32_t offset, Il2CppStackFrameInfo* frame)
    {
        return false; /* TODO */
    }

    int32_t il2cpp_current_thread_get_stack_depth()
    {
        return 0; /* TODO */
    }
    int32_t il2cpp_thread_get_stack_depth(Il2CppThread* thread)
    {
        return 0; /* TODO */
    }

    void il2cpp_set_default_thread_affinity(int64_t affinity_mask)
    { /* TODO */
    }
    void il2cpp_override_stack_backtrace(Il2CppBacktraceFunc stackBacktraceFunc)
    { /* TODO */
    }

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
        if (type == otherType)
            return true;
        if (type->ele_type != otherType->ele_type)
            return false;
        if (type->by_ref != otherType->by_ref)
            return false;
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

    Il2CppManagedMemorySnapshot* il2cpp_capture_memory_snapshot()
    {
        return nullptr; /* TODO */
    }
    void il2cpp_free_captured_memory_snapshot(Il2CppManagedMemorySnapshot* s)
    { /* TODO */
    }

    // -- plugin / log / debugger ----------------------------------------------

    void il2cpp_set_find_plugin_callback(Il2CppSetFindPlugInCallback method)
    { /* TODO */
    }
    void il2cpp_register_log_callback(Il2CppLogCallback method)
    { /* TODO */
    }
    void il2cpp_debugger_set_agent_options(const char* options)
    { /* TODO */
    }

    bool il2cpp_is_debugger_attached()
    {
        return false;
    }

    void il2cpp_register_debugger_agent_transport(Il2CppDebuggerTransport* t)
    { /* TODO */
    }

    bool il2cpp_debug_get_method_info(const MethodInfo* method, Il2CppMethodDebugInfo* info)
    {
        return false; // TODO
    }

    void il2cpp_unity_install_unitytls_interface(const void* unitytlsInterfaceStruct)
    { /* TODO */
    }

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
            auto result = vm::CustomAttribute::has_customattribute_on_class(reinterpret_cast<Il2CppClass*>(ptr), attr_klass);
            return result.is_ok() && result.unwrap();
        }
        else if (tag == 1)
        {
            auto result = vm::CustomAttribute::has_customattribute_on_method(reinterpret_cast<const MethodInfo*>(ptr), attr_klass);
            return result.is_ok() && result.unwrap();
        }
        else if (tag == 2)
        {
            auto result = vm::CustomAttribute::has_customattribute_on_field(reinterpret_cast<const FieldInfo*>(ptr), attr_klass);
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

    void il2cpp_type_get_name_chunked(const Il2CppType* type, void (*chunkReportFunc)(void* data, void* userData), void* userData)
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

    void il2cpp_unity_set_android_network_up_state_func(Il2CppAndroidUpStateFunc func)
    { /* TODO */
    }
}
