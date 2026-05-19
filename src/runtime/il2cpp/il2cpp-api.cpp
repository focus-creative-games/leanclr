#include "il2cpp-api.h"

#include <clocale>

#if IL2CPP_API_DYNAMIC_NO_DLSYM
typedef utils::HashMap<const char*, void*, utils::CStrHasher, utils::CStrCompare> SymbolTable;
static SymbolTable s_SymbolTable;

void* il2cpp_api_lookup_symbol(const char* name)
{
    SymbolTable::iterator it = s_SymbolTable.find(name);
    if (it != s_SymbolTable.end())
    {
        return it->second;
    }
    return nullptr;
}

static void RegisterAPIFunction(const char* name, void* symbol)
{
    s_SymbolTable.insert(std::make_pair(name, symbol));
}

void il2cpp_api_register_symbols(void)
{
#define DO_API(r, n, p) RegisterAPIFunction(#n, (void*)n);
#define DO_API_NO_RETURN(r, n, p) DO_API(r, n, p)
#include "il2cpp-api-functions.h"
#undef DO_API
#undef DO_API_NO_RETURN
}
#endif

int il2cpp_init(const char* domain_name)
{
    setlocale(LC_ALL, "");
    vm::Settings::set_domain_name(domain_name);
    vm::Settings::set_file_loader(il2cpp::assembly_file_loader);
    vm::Settings::set_aot_modules_data(&g_aot_modules_data);
    auto ret = vm::Runtime::initialize();
    printf("il2cpp_init: %s\n", ret.is_ok() ? "ok" : "error");
    return ret.is_ok();
}

int il2cpp_init_utf16(const Il2CppChar* domain_name)
{
    utils::Utf8StringBuilder sb(domain_name, static_cast<size_t>(utils::StringUtil::get_utf16chars_length(domain_name)));
    return il2cpp_init(sb.get_const_chars());
}

void il2cpp_shutdown()
{
    vm::Runtime::shutdown();
}

void il2cpp_set_config_dir(const char* config_path)
{
    vm::Settings::set_config_dir(config_path);
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

void il2cpp_set_memory_callbacks(Il2CppMemoryCallbacks* callbacks)
{
    alloc::GeneralAllocation::set_memory_callbacks(*callbacks);
}

void il2cpp_set_config_utf16(const Il2CppChar* executablePath)
{
    utils::Utf8StringBuilder sb(executablePath, static_cast<size_t>(utils::StringUtil::get_utf16chars_length(executablePath)));
    vm::Settings::set_config(sb.get_const_chars());
}

void il2cpp_set_config(const char* executablePath)
{
    vm::Settings::set_config(executablePath);
}

void il2cpp_memory_pool_set_region_size(size_t size)
{
    alloc::MemPool::set_default_region_size(size);
}

size_t il2cpp_memory_pool_get_region_size()
{
    return alloc::MemPool::get_default_region_size();
}

const Il2CppImage* il2cpp_get_corlib()
{
    return vm::Assembly::get_corlib()->mod;
}

// -- internal calls -------------------------------------------------------

void il2cpp_add_internal_call(const char* name, Il2CppMethodPointer method)
{
    vm::InternalCalls::register_lite_internal_call(name, method);
}

Il2CppMethodPointer il2cpp_resolve_icall(const char* name)
{
    return vm::InternalCalls::get_lite_internal_call(name);
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
    auto result = LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERNAL(elementTypeInfo, static_cast<int32_t>(length), "il2cpp_array_new");
    assert(result.is_ok());
    return result.is_ok() ? result.unwrap() : nullptr;
}

Il2CppArray* il2cpp_array_new_specific(Il2CppClass* arrayTypeInfo, il2cpp_array_size_t length)
{
    auto result = LEANCLR_NEW_SZARRAY_FROM_ARRAY_KLASS_INTERNAL(arrayTypeInfo, static_cast<int32_t>(length), "il2cpp_array_new_specific");
    return result.is_ok() ? result.unwrap() : nullptr;
}

Il2CppArray* il2cpp_array_new_full(Il2CppClass* array_class, il2cpp_array_size_t* lengths, il2cpp_array_size_t* lower_bounds)
{
    switch (array_class->by_val->ele_type)
    {
    case metadata::RtElementType::SZArray:
    {
        auto result = LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERNAL(array_class, static_cast<int32_t>(lengths[0]), "il2cpp_array_new_full");
        assert(result.is_ok());
        return result.is_ok() ? result.unwrap() : nullptr;
    }
    case metadata::RtElementType::Array:
    {
        auto result =
            LEANCLR_NEW_MDARRAY_FROM_ARRAY_KLASS_INTERNAL(array_class, reinterpret_cast<const int32_t*>(lengths), reinterpret_cast<const int32_t*>(lower_bounds), "il2cpp_array_new_full");
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
    auto ret = vm::Class::initialize_super_types(klass);
    if (ret.is_err())
    {
        return false;
    }
    return vm::Class::is_subclass_of_initialized(klass, klassc, check_interfaces);
}

bool il2cpp_class_has_parent(Il2CppClass* klass, Il2CppClass* klassc)
{
    auto ret = vm::Class::initialize_super_types(klass);
    if (ret.is_err())
    {
        return false;
    }
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
    if (result.is_ok())
    {
        return result.unwrap();
    }
    return nullptr;
}

Il2CppClass* il2cpp_class_get_element_class(Il2CppClass* klass)
{
    return const_cast<Il2CppClass*>(klass->element_class);
}

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
        if (klass->event_count == 0)
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
        if (klass->field_count == 0)
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
    auto ret = vm::Class::initialize_properties(klass);
    if (ret.is_err())
    {
        return nullptr;
    }
    return const_cast<PropertyInfo*>(vm::Class::get_property_for_name(klass, name, true));
}

FieldInfo* il2cpp_class_get_field_from_name(Il2CppClass* klass, const char* name)
{
    auto ret = vm::Class::initialize_fields(klass);
    if (ret.is_err())
    {
        return nullptr;
    }
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
    auto ret = vm::Class::initialize_methods(klass);
    if (ret.is_err())
    {
        return nullptr;
    }
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
    return static_cast<int32_t>(vm::Class::get_instance_size_without_object_header(klass));
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
    if (!klass)
    {
        return nullptr;
    }
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
    printf("open assembly:%s\n", name);
    std::string dll_name(name);
    if (dll_name.length() > 4 && dll_name.substr(dll_name.length() - 4) == ".dll")
    {
        dll_name = dll_name.substr(0, dll_name.length() - 4);
    }
    auto result = vm::Assembly::load_by_name(dll_name.c_str());
    return result.is_ok() ? result.unwrap() : nullptr;
}

const Il2CppAssembly** s_cached_assemblies = nullptr;
size_t s_cached_assemblies_size = 0;

const Il2CppAssembly** il2cpp_domain_get_assemblies(const Il2CppDomain* domain, size_t* size)
{
    utils::Vector<metadata::RtModuleDef*> modules;
    metadata::RtModuleDef::get_registered_modules(modules);
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
    auto ex_ret = LEANCLR_NEWOBJ_INTERNAL(klass, "il2cpp::il2cpp_exception_from_name_msg");
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
    auto ex_ret = LEANCLR_NEWOBJ_INTERNAL(ex_class, "il2cpp::il2cpp_get_exception_argument_null");
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
    utils::Utf8StringBuilder sb;
    vm::Exception::format_exception(const_cast<vm::RtException*>(ex), sb);
    size_t copy_size = std::min(sb.length(), static_cast<size_t>(message_size) - 1);
    std::memcpy(message, sb.get_const_chars(), copy_size);
    message[copy_size] = '\0';
}

void il2cpp_format_stack_trace(const Il2CppException* ex, char* output, int output_size)
{
    utils::Utf8StringBuilder sb;
    if (ex->stack_trace)
    {
        sb.append_utf16_str(vm::String::get_chars_ptr(ex->stack_trace), static_cast<size_t>(vm::String::get_length(ex->stack_trace)));
    }
    size_t copy_size = std::min(sb.length(), static_cast<size_t>(output_size) - 1);
    std::memcpy(output, sb.get_const_chars(), copy_size);
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

// since Unity 6000.x.y
const FieldInfo* il2cpp_field_get_from_reflection(const Il2CppReflectionField* field)
{
    return field->field;
}

Il2CppClass* il2cpp_field_get_parent(FieldInfo* field)
{
    return field->parent;
}

Il2CppReflectionField* il2cpp_field_get_object(FieldInfo* field, Il2CppClass* refclass)
{
    auto result = vm::Reflection::get_field_reflection_object(field, refclass);
    if (result.is_err())
    {
        return nullptr;
    }
    return reinterpret_cast<Il2CppReflectionField*>(result.unwrap());
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

Il2CppGCHandle il2cpp_gchandle_new(Il2CppObject* obj, bool pinned)
{
    void* handle = vm::GCHandle::new_handle(obj, pinned);
#if LEANCLR_USE_VOID_PTR_GCHANDLE
    return reinterpret_cast<Il2CppGCHandle>(handle);
#else
    return static_cast<Il2CppGCHandle>(vm::GCHandle::get_handle_id(handle));
#endif
}

Il2CppGCHandle il2cpp_gchandle_new_weakref(Il2CppObject* obj, bool track_resurrection)
{
    void* handle = vm::GCHandle::new_weakref_handle(obj, track_resurrection);
#if LEANCLR_USE_VOID_PTR_GCHANDLE
    return reinterpret_cast<Il2CppGCHandle>(handle);
#else
    return static_cast<Il2CppGCHandle>(vm::GCHandle::get_handle_id(handle));
#endif
}

Il2CppObject* il2cpp_gchandle_get_target(Il2CppGCHandle gchandle)
{
#if LEANCLR_USE_VOID_PTR_GCHANDLE
    void* handle = reinterpret_cast<void*>(gchandle);
#else
    void* handle = vm::GCHandle::get_handle_by_id(static_cast<uint32_t>(gchandle));
#endif
    if (handle == nullptr)
    {
        return nullptr;
    }
    return vm::GCHandle::get_target(handle);
}

void il2cpp_gchandle_free(Il2CppGCHandle gchandle)
{
#if LEANCLR_USE_VOID_PTR_GCHANDLE
    void* handle = reinterpret_cast<void*>(gchandle);
#else
    void* handle = vm::GCHandle::get_handle_by_id(static_cast<uint32_t>(gchandle));
#endif
    if (handle == nullptr)
    {
        return;
    }
    vm::GCHandle::free_handle(handle);
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

// -- vm runtime info ------------------------------------------------------

uint32_t il2cpp_object_header_size()
{
    return leanclr::vm::RT_OBJECT_HEADER_SIZE;
}

uint32_t il2cpp_array_object_header_size()
{
    return leanclr::vm::RT_ARRAY_HEADER_SIZE;
}

uint32_t il2cpp_offset_of_array_length_in_array_object_header()
{
    return leanclr::vm::RT_ARRAY_LENGTH_OFFSET;
}

uint32_t il2cpp_offset_of_array_bounds_in_array_object_header()
{
    return leanclr::vm::RT_ARRAY_BOUNDS_OFFSET;
}

uint32_t il2cpp_allocation_granularity()
{
    return static_cast<uint32_t>(2 * sizeof(void*));
}

// -- liveness -------------------------------------------------------------

// only used in unity 2019 and 2020.
void* il2cpp_unity_liveness_calculation_begin(Il2CppClass* filter, int max_object_count, il2cpp_register_object_callback callback, void* userdata,
                                              il2cpp_WorldChangedCallback onWorldStarted, il2cpp_WorldChangedCallback onWorldStopped)
{
    il2cpp_liveness_reallocate_callback reallocate = nullptr;
    return leanclr::il2cpp::Liveness::allocate_struct(filter, static_cast<uint32_t>(max_object_count), callback, userdata, reallocate);
    // FIXME: should call onWorldStarted.
}

// only used in unity 2019 and 2020.
void il2cpp_unity_liveness_calculation_end(void* state)
{
    leanclr::il2cpp::Liveness::finalize(state);
    leanclr::il2cpp::Liveness::free_struct(state);
    // FIXME: should call onWorldStopped.
}

void* il2cpp_unity_liveness_allocate_struct(Il2CppClass* filter, int max_object_count, il2cpp_register_object_callback callback, void* userdata,
                                            il2cpp_liveness_reallocate_callback reallocate)
{
    return leanclr::il2cpp::Liveness::allocate_struct(filter, static_cast<uint32_t>(max_object_count), callback, userdata, reallocate);
}

void il2cpp_unity_liveness_calculation_from_root(Il2CppObject* root, void* state)
{
    leanclr::il2cpp::Liveness::calculation_from_root(root, state);
}

void il2cpp_unity_liveness_calculation_from_statics(void* state)
{
    leanclr::il2cpp::Liveness::calculation_from_statics(state);
}

void il2cpp_unity_liveness_finalize(void* state)
{
    leanclr::il2cpp::Liveness::finalize(state);
}

void il2cpp_unity_liveness_free_struct(void* state)
{
    leanclr::il2cpp::Liveness::free_struct(state);
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
    return method ? static_cast<uint32_t>(method->parameter_count) : 0;
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
    auto nameResult = vm::Method::get_parameter_c_name_by_token(method->parent->image, optToken.value());
    return nameResult.is_ok() ? nameResult.unwrap() : nullptr;
}

// -- profiler (no-op stubs) ------------------------------------------------

#if IL2CPP_ENABLE_PROFILER
void il2cpp_profiler_install(Il2CppProfiler* prof, Il2CppProfileFunc shutdown_callback)
{
    Profiler::install(prof, shutdown_callback);
}
void il2cpp_profiler_set_events(Il2CppProfileFlags events)
{
    Profiler::set_events(events);
}
void il2cpp_profiler_install_enter_leave(Il2CppProfileMethodFunc enter, Il2CppProfileMethodFunc flee)
{
    Profiler::install_enter_leave(enter, flee);
}
void il2cpp_profiler_install_allocation(Il2CppProfileAllocFunc callback)
{
    Profiler::install_allocation(callback);
}
void il2cpp_profiler_install_gc(Il2CppProfileGCFunc cb, Il2CppProfileGCResizeFunc resize_cb)
{
    Profiler::install_gc(cb, resize_cb);
}
void il2cpp_profiler_install_fileio(Il2CppProfileFileIOFunc callback)
{
    Profiler::install_fileio(callback);
}
void il2cpp_profiler_install_thread(Il2CppProfileThreadFunc start, Il2CppProfileThreadFunc end)
{
    Profiler::install_thread(start, end);
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
    if (vm::Class::is_string_class(klass))
    {
        return static_cast<uint32_t>(vm::String::get_string_allocation_size(((Il2CppString*)obj)->length));
    }
    if (vm::Class::is_array_or_szarray(klass))
    {
        auto* arr = static_cast<vm::RtArray*>(obj);
        return static_cast<uint32_t>(vm::Array::get_array_allocation_size(arr->klass, vm::Array::get_array_length(arr)));
    }
    return static_cast<uint32_t>(vm::Class::get_instance_size_with_object_header(klass));
}

const MethodInfo* il2cpp_object_get_virtual_method(Il2CppObject* obj, const MethodInfo* method)
{
    auto result = vm::Method::get_virtual_method_impl(obj, method);
    return result.is_ok() ? result.unwrap() : nullptr;
}

Il2CppObject* il2cpp_object_new(const Il2CppClass* klass)
{
    auto result = LEANCLR_NEWOBJ_INTERNAL(klass, "il2cpp::object_new");
    return result.is_ok() ? result.unwrap() : nullptr;
}

void* il2cpp_object_unbox(Il2CppObject* obj)
{
    return const_cast<void*>(vm::Object::get_box_value_type_data_ptr(obj));
}

Il2CppObject* il2cpp_value_box(Il2CppClass* klass, void* data)
{
    auto result = LEANCLR_BOX_OBJECT_INTERNAL(klass, data, "il2cpp::value_box");
    return result.is_ok() ? result.unwrap() : nullptr;
}

// -- monitor --------------------------------------------------------------

void il2cpp_monitor_enter(Il2CppObject* obj)
{
    vm::Monitor::enter(obj);
}

bool il2cpp_monitor_try_enter(Il2CppObject* obj, uint32_t timeout)
{
    return vm::Monitor::monitor_try_enter(obj, static_cast<int32_t>(timeout));
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
    auto ret = vm::Runtime::invoke_object_arguments_with_run_cctor(method, static_cast<vm::RtObject*>(obj), params, paramCount);
    if (ret.is_err())
    {
        Il2CppException* ex = vm::Exception::raise_error_as_exception(ret.unwrap_err(), nullptr, nullptr);
        if (exc)
            *exc = ex;
        return nullptr;
    }
    return ret.unwrap();
}

Il2CppObject* il2cpp_runtime_invoke(const MethodInfo* method, void* obj, void** params, Il2CppException** exc)
{
    if (!method)
    {
        return nullptr;
    }
    auto result = vm::Runtime::invoke_with_run_cctor(method, static_cast<vm::RtObject*>(obj), reinterpret_cast<const void* const*>(params));
    if (result.is_err())
    {
        Il2CppException* ex = vm::Exception::raise_error_as_exception(result.unwrap_err(), nullptr, nullptr);
        if (exc)
            *exc = ex;
        return nullptr;
    }
    return result.unwrap();
}

void il2cpp_runtime_class_init(Il2CppClass* klass)
{
    auto ret = vm::Runtime::run_class_static_constructor(klass);
    if (ret.is_err())
    {
        il2cpp_raise_exception(vm::Exception::raise_error_as_exception(ret.unwrap_err(), nullptr, nullptr));
    }
}

void il2cpp_runtime_object_init(Il2CppObject* obj)
{
    const MethodInfo* ctor = vm::Class::get_method_for_name(obj->klass, ".ctor", 0, true);
    assert(ctor);
    auto ret = vm::Runtime::invoke_with_run_cctor(ctor, obj, nullptr);
    if (ret.is_err())
    {
        il2cpp_raise_exception(vm::Exception::raise_error_as_exception(ret.unwrap_err(), nullptr, nullptr));
    }
}

void il2cpp_runtime_object_init_exception(Il2CppObject* obj, Il2CppException** exc)
{
    const MethodInfo* ctor = vm::Class::get_method_for_name(obj->klass, ".ctor", 0, true);
    assert(ctor);
    if (!ctor)
    {
        Il2CppException* ex = vm::Exception::raise_error_as_exception(RtErr::MissingMethod, nullptr, nullptr);
        if (exc)
            *exc = ex;
        return;
    }

    auto ret = vm::Runtime::invoke_object_arguments_with_run_cctor(ctor, obj, nullptr, 0);
    if (ret.is_err())
    {
        Il2CppException* ex = vm::Exception::raise_error_as_exception(ret.unwrap_err(), nullptr, nullptr);
        if (exc)
            *exc = ex;
    }
}

void il2cpp_runtime_unhandled_exception_policy_set(Il2CppRuntimeUnhandledExceptionPolicy value)
{
    il2cpp::Runtime::set_unhandled_exception_policy(value);
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
{
    vm::Thread::detach(thread);
}

Il2CppThread** il2cpp_thread_get_all_attached_threads(size_t* size)
{
    return vm::Thread::get_all_attached_threads(size);
}

bool il2cpp_is_vm_thread(Il2CppThread* thread)
{
    return vm::Thread::is_vm_thread(thread);
}

// -- stacktrace -----------------------------------------------------------

void il2cpp_current_thread_walk_frame_stack(Il2CppFrameWalkFunc func, void* user_data)
{
    auto& ms = interp::MachineState::get_global_machine_state();
    auto frames = ms.get_active_frames();
    for (auto& frame : frames)
    {
        func(&frame, user_data);
    }
}

void il2cpp_thread_walk_frame_stack(Il2CppThread* thread, Il2CppFrameWalkFunc func, void* user_data)
{
    assert(thread == vm::Thread::get_current_thread());
    il2cpp_current_thread_walk_frame_stack(func, user_data);
}

bool il2cpp_current_thread_get_top_frame(Il2CppStackFrameInfo* frame)
{
    auto& ms = interp::MachineState::get_global_machine_state();
    auto frames = ms.get_active_frames();
    if (frames.size() == 0)
    {
        return false;
    }
    *frame = frames[frames.size() - 1];
    return true;
}

bool il2cpp_thread_get_top_frame(Il2CppThread* thread, Il2CppStackFrameInfo* frame)
{
    assert(thread == vm::Thread::get_current_thread());
    return il2cpp_current_thread_get_top_frame(frame);
}

bool il2cpp_current_thread_get_frame_at(int32_t offset, Il2CppStackFrameInfo* frame)
{
    auto& ms = interp::MachineState::get_global_machine_state();
    auto frames = ms.get_active_frames();
    if (offset < 0 || frames.size() == 0 || static_cast<size_t>(offset) >= frames.size())
    {
        return false;
    }
    *frame = frames[frames.size() - 1U - static_cast<size_t>(offset)];
    return true;
}

bool il2cpp_thread_get_frame_at(Il2CppThread* thread, int32_t offset, Il2CppStackFrameInfo* frame)
{
    assert(thread == vm::Thread::get_current_thread());
    return il2cpp_current_thread_get_frame_at(offset, frame);
}

int32_t il2cpp_current_thread_get_stack_depth()
{
    auto& ms = interp::MachineState::get_global_machine_state();
    return static_cast<int32_t>(ms.get_active_frames().size());
}

int32_t il2cpp_thread_get_stack_depth(Il2CppThread* thread)
{
    assert(thread == vm::Thread::get_current_thread());
    return il2cpp_current_thread_get_stack_depth();
}

void il2cpp_set_default_thread_affinity(int64_t affinity_mask)
{
    vm::Thread::set_default_affinity_mask(affinity_mask);
}

void il2cpp_override_stack_backtrace(Il2CppBacktraceFunc stackBacktraceFunc)
{
    il2cpp::StackTrace::override_stack_backtrace(stackBacktraceFunc);
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
    switch (type->ele_type)
    {
    case metadata::RtElementType::Array:
    {
        auto result = vm::Class::get_class_from_typesig(type->data.array_type->ele_type);
        return result.is_ok() ? result.unwrap() : nullptr;
    }
    case metadata::RtElementType::SZArray:
    {
        auto result = vm::Class::get_class_from_typesig(type->data.element_type);
        return result.is_ok() ? result.unwrap() : nullptr;
    }
    default:
    {
        auto result = vm::Class::get_class_from_typesig(type);
        return result.is_ok() ? result.unwrap() : nullptr;
    }
    }
}

char* il2cpp_type_get_name(const Il2CppType* type)
{
    utils::Utf8StringBuilder sb;
    vm::TypeNameFormat format = vm::TypeNameFormat::IL;
    auto ret = vm::Type::append_type_full_name(sb, type, format, false);
    if (ret.is_err())
    {
        return nullptr;
    }
    return sb.dup_zero_terminated_chars();
}

char* il2cpp_type_get_assembly_qualified_name(const Il2CppType* type)
{
    utils::Utf8StringBuilder sb;
    vm::TypeNameFormat format = vm::TypeNameFormat::AssemblyQualified;
    auto ret = vm::Type::append_type_full_name(sb, type, format, false);
    if (ret.is_err())
    {
        return nullptr;
    }
    return sb.dup_zero_terminated_chars();
}

char* il2cpp_type_get_reflection_name(const Il2CppType* type)
{
    utils::Utf8StringBuilder sb;
    vm::TypeNameFormat format = vm::TypeNameFormat::Reflection;
    auto ret = vm::Type::append_type_full_name(sb, type, format, false);
    if (ret.is_err())
    {
        return nullptr;
    }
    return sb.dup_zero_terminated_chars();
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
    return metadata::MetadataCompare::is_typesig_equal_ignore_attrs(type, otherType, false);
}

bool il2cpp_type_is_static(const Il2CppType* type)
{
    return (type->field_or_param_attrs & static_cast<uint32_t>(metadata::RtFieldAttribute::Static)) != 0;
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
    metadata::EncodedTokenId entrypoint_token = image->get_entrypoint_token();
    if (entrypoint_token == 0)
    {
        return nullptr;
    }
    uint32_t entrypoint_rid = metadata::RtToken::decode_rid(entrypoint_token);
    auto result = const_cast<Il2CppImage*>(image)->get_method_by_rid(entrypoint_rid);
    if (result.is_err())
    {
        return nullptr;
    }
    return result.unwrap();
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
    return il2cpp::Runtime::capture_memory_snapshot();
}
void il2cpp_free_captured_memory_snapshot(Il2CppManagedMemorySnapshot* s)
{
    il2cpp::Runtime::free_captured_memory_snapshot(s);
}

// -- plugin / log / debugger ----------------------------------------------

void il2cpp_set_find_plugin_callback(Il2CppSetFindPlugInCallback method)
{
    il2cpp::Runtime::set_find_plugin_callback(method);
}
void il2cpp_register_log_callback(Il2CppLogCallback method)
{
    il2cpp::Runtime::set_log_callback(method);
}
void il2cpp_debugger_set_agent_options(const char* options)
{
    il2cpp::Debugger::set_agent_options(options);
}

bool il2cpp_is_debugger_attached()
{
    return il2cpp::Debugger::is_debugger_attached();
}

void il2cpp_register_debugger_agent_transport(Il2CppDebuggerTransport* t)
{
    il2cpp::Debugger::register_debugger_agent_transport(t);
}

// add since Unity 6000.x.y
void il2cpp_debug_foreach_method(void (*func)(const MethodInfo* method, Il2CppMethodDebugInfo* methodDebugInfo, void* userData), void* userData)
{
    // TODO: Implement this
}

bool il2cpp_debug_get_method_info(const MethodInfo* method, Il2CppMethodDebugInfo* info)
{
    return il2cpp::Debugger::get_method_info(method, info);
}

void il2cpp_unity_install_unitytls_interface(const void* unitytlsInterfaceStruct)
{
    il2cpp::UnityEngine::set_unitytls_interface(unitytlsInterfaceStruct);
}

// -- custom attributes ----------------------------------------------------
// Il2CppCustomAttrInfo* is an opaque tagged pointer encoding the provider:
//   bits[1:0] == 0 => Il2CppClass*
//   bits[1:0] == 1 => const MethodInfo*
//   bits[1:0] == 2 => const FieldInfo*

Il2CppCustomAttrInfo* il2cpp_custom_attrs_from_class(Il2CppClass* klass)
{
    metadata::RtRuntimeHandle handle(klass->by_val);
    return (Il2CppCustomAttrInfo*)metadata::RtEncodedRuntimeHandle::encode(handle).get_encoded_value();
}

Il2CppCustomAttrInfo* il2cpp_custom_attrs_from_method(const MethodInfo* method)
{
    metadata::RtRuntimeHandle handle(method);
    return (Il2CppCustomAttrInfo*)metadata::RtEncodedRuntimeHandle::encode(handle).get_encoded_value();
}

Il2CppCustomAttrInfo* il2cpp_custom_attrs_from_field(const FieldInfo* field)
{
    metadata::RtRuntimeHandle handle(field);
    return (Il2CppCustomAttrInfo*)metadata::RtEncodedRuntimeHandle::encode(handle).get_encoded_value();
}

bool il2cpp_custom_attrs_has_attr(Il2CppCustomAttrInfo* ainfo, Il2CppClass* attr_klass)
{
    metadata::RtEncodedRuntimeHandle encodedHandle(ainfo);
    metadata::RtRuntimeHandle handle = metadata::RtEncodedRuntimeHandle::decode(encodedHandle);
    if (handle.is_type())
    {
        auto ret_klass = vm::Class::get_class_from_typesig(handle.typeSig);
        if (ret_klass.is_err())
        {
            return false;
        }
        auto result = vm::CustomAttribute::has_customattribute_on_class(ret_klass.unwrap(), attr_klass);
        return result.is_ok() && result.unwrap();
    }
    else if (handle.is_method())
    {
        auto result = vm::CustomAttribute::has_customattribute_on_method(handle.method, attr_klass);
        return result.is_ok() && result.unwrap();
    }
    else if (handle.is_field())
    {
        auto result = vm::CustomAttribute::has_customattribute_on_field(handle.field, attr_klass);
        return result.is_ok() && result.unwrap();
    }
    return false;
}

Il2CppObject* il2cpp_custom_attrs_get_attr(Il2CppCustomAttrInfo* ainfo, Il2CppClass* attr_klass)
{
    metadata::RtEncodedRuntimeHandle encodedHandle(ainfo);
    metadata::RtRuntimeHandle handle = metadata::RtEncodedRuntimeHandle::decode(encodedHandle);
    Il2CppImage* image = nullptr;
    metadata::EncodedTokenId token;
    if (handle.is_type())
    {
        auto ret_klass = vm::Class::get_class_from_typesig(handle.typeSig);
        if (ret_klass.is_err())
        {
            return nullptr;
        }
        token = ret_klass.unwrap()->token;
        image = ret_klass.unwrap()->image;
    }
    else if (handle.is_method())
    {
        token = handle.method->token;
        image = handle.method->parent->image;
    }
    else if (handle.is_field())
    {
        token = handle.field->token;
        image = handle.field->parent->image;
    }
    else
    {
        return nullptr;
    }
    auto result = vm::CustomAttribute::get_customattributes_on_target_token(image, token, attr_klass);
    if (result.is_err())
    {
        return nullptr;
    }

    Il2CppArray* attrs = result.unwrap();
    if (attrs && attrs->length > 0)
    {
        return vm::Array::get_array_data_at<Il2CppObject*>(attrs, 0);
    }
    return nullptr;
}

Il2CppArray* il2cpp_custom_attrs_construct(Il2CppCustomAttrInfo* ainfo)
{
    metadata::RtEncodedRuntimeHandle encodedHandle(ainfo);
    metadata::RtRuntimeHandle handle = metadata::RtEncodedRuntimeHandle::decode(encodedHandle);
    Il2CppImage* image = nullptr;
    metadata::EncodedTokenId token;
    if (handle.is_type())
    {
        auto ret_klass = vm::Class::get_class_from_typesig(handle.typeSig);
        if (ret_klass.is_err())
        {
            return nullptr;
        }
        token = ret_klass.unwrap()->token;
        image = ret_klass.unwrap()->image;
    }
    else if (handle.is_method())
    {
        token = handle.method->token;
        image = handle.method->parent->image;
    }
    else if (handle.is_field())
    {
        token = handle.field->token;
        image = handle.field->parent->image;
    }
    else
    {
        return nullptr;
    }
    auto result = vm::CustomAttribute::get_customattributes_on_target_token(image, token, nullptr);
    if (result.is_err())
    {
        return nullptr;
    }
    return result.unwrap();
}

void il2cpp_custom_attrs_free(Il2CppCustomAttrInfo* ainfo)
{
    // nothing to free - handles are encoded pointers
}

// -- type name chunked ----------------------------------------------------

void il2cpp_type_get_name_chunked(const Il2CppType* type, void (*chunkReportFunc)(void* data, void* userData), void* userData)
{
    utils::Utf8StringBuilder sb;
    vm::TypeNameFormat format = vm::TypeNameFormat::IL;
    auto ret = vm::Type::append_type_full_name(sb, type, format, false);
    if (ret.is_err())
    {
        assert(false);
        return;
    }
    sb.sure_null_terminator_but_not_append();
    chunkReportFunc((void*)sb.get_const_chars(), userData);
}

// -- class user data ------------------------------------------------------

void il2cpp_class_set_userdata(Il2CppClass* klass, void* userdata)
{
    klass->unity_user_data = userdata;
}

int il2cpp_class_get_userdata_offset()
{
    return offsetof(Il2CppClass, unity_user_data);
}

// -- class enumeration ----------------------------------------------------

void il2cpp_class_for_each(void (*klassReportFunc)(Il2CppClass* klass, void* userData), void* userData)
{
    utils::Vector<metadata::RtModuleDef*> modules;
    metadata::RtModuleDef::get_registered_modules(modules);
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
    metadata::MetadataCache::walk_generic_classes(klassReportFunc, userData);
    vm::ArrayClass::walk_array_classes(klassReportFunc, userData);
    vm::Class::walk_ptr_classes(klassReportFunc, userData);
    // FIXME: il2cpp doesn't walk generic param classes, is it a mistake?
}

// -- android --------------------------------------------------------------

void il2cpp_unity_set_android_network_up_state_func(Il2CppAndroidUpStateFunc func)
{
    il2cpp::UnityEngine::set_android_network_up_state_func(func);
}
