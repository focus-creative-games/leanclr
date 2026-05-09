#include "array_class.h"
#include "class.h"
#include "metadata/metadata_cache.h"
#include "alloc/metadata_allocation.h"
#include "utils/rt_vector.h"
#include "utils/hashmap.h"
#include "utils/string_builder.h"
#include "metadata/rt_metadata.h"
#include "metadata/module_def.h"
#include "const_strs.h"
#include "generic_class.h"
#include "generic_method.h"
#include "shim.h"

namespace leanclr
{
namespace vm
{
using namespace leanclr::utils;
using namespace leanclr::metadata;
using namespace leanclr::core;
using namespace leanclr::alloc;

// Helper structures and constants
struct ArrayTemplateMethod
{
    const char* original_name;
    const char* final_name;
    const metadata::RtMethodInfo* method;
};

// Static maps for array classes and interface methods
static HashMap<const metadata::RtTypeSig*, metadata::RtClass*> g_arrayClassMap;

static Vector<ArrayTemplateMethod> g_icollectionGenericMethods;
static Vector<ArrayTemplateMethod> g_ienumerableGenericMethods;
static Vector<ArrayTemplateMethod> g_ireadonlyCollectionGenericMethods;
static Vector<ArrayTemplateMethod> g_ireadonlyListGenericMethods;
static Vector<ArrayTemplateMethod> g_ilistGenericMethods;

// Static map for caching array generic methods
static HashMap<const RtMethodInfo*, const RtMethodInfo*> g_arrayGenericMethodCache;

// Helper function to create array type name
static const char* make_array_name(const char* ele_class_name, uint8_t rank, bool bound)
{
    StringBuilder sb;
    sb.append_cstr(ele_class_name);
    sb.append_char('[');
    sb.append_chars(',', rank - 1);
    if (bound)
    {
        sb.append_char('*');
    }
    sb.append_char(']');
    return sb.dup_to_zero_end_cstr();
}

static void setup_array_cast_class(RtClass* klass)
{
    if (Class::is_array_or_szarray(klass->element_class))
    {
        klass->cast_class = klass;
    }
    else
    {
        klass->cast_class = klass->element_class->cast_class;
    }
}

// Common setup for array classes
static void setup_array_class_common(RtClass* array_class, const metadata::RtClass* ele_class)
{
    const CorLibTypes& corlib = Class::get_corlib_types();

    array_class->image = ele_class->image;
    array_class->parent = corlib.cls_array;
    array_class->namespaze = ele_class->namespaze;
    array_class->element_class = ele_class;
    // Array classes don't need cast_class setup (it stays nullptr)
    setup_array_cast_class(array_class);

    array_class->flags = (uint32_t)RtTypeAttribute::Public | (uint32_t)RtTypeAttribute::Sealed | (uint32_t)RtTypeAttribute::Serializable;
    array_class->extra_flags = (uint32_t)RtClassExtraAttribute::ArrayOrSZArray | (uint32_t)RtClassExtraAttribute::ReferenceType;
}

// Build array method (helper for setup_methods)
static RtResult<const RtMethodInfo*> build_array_method(RtClass* klass, const char* name, const RtTypeSig* return_type, const RtTypeSig* const* parameters,
                                                        size_t parameter_count)
{
    RtMethodInfo* method = klass->image->get_mem_pool().calloc_any<RtMethodInfo>(1);
    method->parent = klass;
    method->name = name;
    method->token = 0;
    method->flags = (uint16_t)RtMethodAttribute::Public;
    method->iflags = (uint16_t)RtMethodImplAttribute::InternalCall;
    method->return_type = return_type;

    if (parameter_count > 0)
    {
        const RtTypeSig** new_params = klass->image->get_mem_pool().calloc_any<const RtTypeSig*>(parameter_count);
        std::memcpy(new_params, parameters, parameter_count * sizeof(const RtTypeSig*));
        method->parameters = new_params;
    }
    method->parameter_count = static_cast<uint16_t>(parameter_count);

    if (std::strcmp(name, STR_CTOR) == 0)
    {
        method->flags |= (uint16_t)RtMethodAttribute::SpecialName | (uint16_t)RtMethodAttribute::RtSpecialName;
    }
    else
    {
        method->iflags = (uint16_t)RtMethodImplAttribute::Runtime;
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(InvokeTypeAndMethod, invoker_type_and_method, Shim::get_invoker(method));
    method->invoke_method_ptr = invoker_type_and_method.invoker;
    method->invoker_type = invoker_type_and_method.invoker_type;
    MethodAndVirtualMethod method_and_virtual_method = Shim::get_method_pointer(method);
    method->method_ptr = method_and_virtual_method.method_ptr;
    method->virtual_method_ptr = method_and_virtual_method.virtual_method_ptr;

    RET_OK(method);
}

// Build array generic method (helper for setup_vtables)
static RtResult<const RtMethodInfo*> build_array_generic_method(RtClass* klass, const ArrayTemplateMethod& template_method, const RtTypeSig* element_type_sig)
{
    const RtMethodInfo* original_method = template_method.method;
    const RtMethodInfo* inflated_method = original_method;

    // If the method has a generic container, inflate it with the element type
    if (original_method->generic_container != nullptr)
    {
        const RtTypeSig* const generic_args[1] = {element_type_sig};
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtGenericInst*, method_generic_inst, MetadataCache::get_pooled_generic_inst(generic_args, 1));
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtMethodInfo*, infl, GenericMethod::get_method(original_method, nullptr, method_generic_inst));
        inflated_method = infl;
    }

    // Copy inflated method and update parent and name
    RtMethodInfo* new_method = klass->image->get_mem_pool().calloc_any<RtMethodInfo>(1);
    *new_method = *inflated_method;
    new_method->parent = klass;
    new_method->name = template_method.final_name;

    new_method->invoke_method_ptr = inflated_method->invoke_method_ptr;
    new_method->method_ptr = inflated_method->method_ptr;
    new_method->virtual_method_ptr = inflated_method->virtual_method_ptr;
    new_method->invoker_type = inflated_method->invoker_type;

    RET_OK(new_method);
}

// Initialize array interface methods from System.Array
RtResultVoid ArrayClass::initialize_array_interface_methods()
{
    // Scan System.Array methods and map InternalArray__* templates to interface generic methods
    const CorLibTypes& corlib = Class::get_corlib_types();
    RET_ERR_ON_FAIL(Class::initialize_methods(corlib.cls_array));

    StringBuilder sb(128);
    for (size_t i = 0; i < corlib.cls_array->method_count; ++i)
    {
        const metadata::RtMethodInfo* method = corlib.cls_array->methods[i];
        const char* name = method->name;
        if (std::strncmp(name, "InternalArray__", 15) != 0)
            continue;

        sb.clear();

        if (std::strncmp(name, "InternalArray__ICollection_", 27) == 0)
        {
            const char* original_name = name + 27;
            sb.append_cstr("System.Collections.Generic.ICollection`1.");
            sb.append_cstr(original_name);
            const char* new_name = sb.dup_to_zero_end_cstr();
            g_icollectionGenericMethods.push_back({original_name, new_name, method});
        }
        else if (std::strncmp(name, "InternalArray__IEnumerable_", 27) == 0)
        {
            const char* original_name = name + 27;
            sb.append_cstr("System.Collections.Generic.IEnumerable`1.");
            sb.append_cstr(original_name);
            const char* new_name = sb.dup_to_zero_end_cstr();
            g_ienumerableGenericMethods.push_back({original_name, new_name, method});
        }
        else if (std::strncmp(name, "InternalArray__IReadOnlyCollection_", 35) == 0)
        {
            const char* original_name = name + 35;
            sb.append_cstr("System.Collections.Generic.IReadOnlyCollection`1.");
            sb.append_cstr(original_name);
            const char* new_name = sb.dup_to_zero_end_cstr();
            g_ireadonlyCollectionGenericMethods.push_back({original_name, new_name, method});
        }
        else if (std::strncmp(name, "InternalArray__IReadOnlyList_", 29) == 0)
        {
            const char* original_name = name + 29;
            sb.append_cstr("System.Collections.Generic.IReadOnlyList`1.");
            sb.append_cstr(original_name);
            const char* new_name = sb.dup_to_zero_end_cstr();
            g_ireadonlyListGenericMethods.push_back({original_name, new_name, method});
        }
        else
        {
            const char* original_name = name + 15;
            sb.append_cstr("System.Collections.Generic.IList`1.");
            sb.append_cstr(original_name);
            const char* new_name = sb.dup_to_zero_end_cstr();
            g_ilistGenericMethods.push_back({original_name, new_name, method});
        }
    }
    RET_VOID_OK();
}

// Get array class from element type signature
RtResult<RtClass*> ArrayClass::get_array_class_from_element_type(const RtTypeSig* ele_type, uint8_t rank)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, eleKlass, Class::get_class_from_typesig(ele_type));
    return get_array_class_from_element_klass(eleKlass, rank);
}

// Get array class from element class
RtResult<RtClass*> ArrayClass::get_array_class_from_element_klass(const metadata::RtClass* ele_klass, uint8_t rank)
{
    if (rank > metadata::RT_MAX_ARRAY_RANK)
        RET_ERR(RtErr::IndexOutOfRange);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(metadata::RtTypeSigByValRef, types,
                                             MetadataCache::get_pooled_array_typesigs_by_element_typesig(ele_klass->by_val, rank));
    const RtTypeSig* key = types.by_val;

    auto cached = g_arrayClassMap.find(key);
    if (cached != g_arrayClassMap.end())
        RET_OK(cached->second);

    // Create new array class
    RtClass* array_class = static_cast<RtClass*>(MetadataAllocation::malloc_any_zeroed<RtClass>());
    if (!array_class)
        RET_ERR(RtErr::OutOfMemory);

    array_class->name = make_array_name(ele_klass->name, rank, true);
    array_class->by_val = types.by_val;
    array_class->by_ref = types.by_ref;

    setup_array_class_common(array_class, ele_klass);

    g_arrayClassMap.insert({key, array_class});
    RET_OK(array_class);
}

// Get single-dimensional zero-lower-bound array class
RtResult<RtClass*> ArrayClass::get_szarray_class_from_element_typesig(const RtTypeSig* ele_type)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, eleKlass, Class::get_class_from_typesig(ele_type));
    return get_szarray_class_from_element_class(eleKlass);
}

// Get single-dimensional zero-lower-bound array class from element class
RtResult<RtClass*> ArrayClass::get_szarray_class_from_element_class(const metadata::RtClass* ele_class)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(metadata::RtTypeSigByValRef, types,
                                             MetadataCache::get_pooled_szarray_typesigs_by_element_typesig(ele_class->by_val));

    const RtTypeSig* key = types.by_val;

    // Check cache
    auto cached = g_arrayClassMap.find(key);
    if (cached != g_arrayClassMap.end())
        RET_OK(cached->second);

    // Create new szarray class
    RtClass* array_class = static_cast<RtClass*>(MetadataAllocation::malloc_any_zeroed<RtClass>());
    if (!array_class)
        RET_ERR(RtErr::OutOfMemory);

    array_class->name = make_array_name(ele_class->name, 1, false);

    array_class->by_val = types.by_val;
    array_class->by_ref = types.by_ref;

    setup_array_class_common(array_class, ele_class);

    g_arrayClassMap.insert({key, array_class});
    RET_OK(array_class);
}

const metadata::RtClass* ArrayClass::get_array_variance_reduce_type(const metadata::RtClass* klass)
{
    if (Class::is_array_or_szarray(klass))
    {
        return klass;
    }
    return klass->cast_class;
}

RtResultVoid ArrayClass::setup_interfaces(RtClass* klass)
{
    // Only SZArray implement collection interfaces
    if (!Class::is_szarray_class(klass))
        RET_VOID_OK();

    const RtTypeSig* ele_type_sig = Class::get_by_val_type_sig(klass->element_class);
    const RtTypeSig* const generic_args[1] = {ele_type_sig};
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtGenericInst*, generic_inst, MetadataCache::get_pooled_generic_inst(generic_args, 1));

    const CorLibTypes& corlib = Class::get_corlib_types();
    RtClass* to_inflate[5] = {
        corlib.cls_ilist_generic,         corlib.cls_icollection_generic,         corlib.cls_ienumerable_generic,
        corlib.cls_ireadonlylist_generic, corlib.cls_ireadonlycollection_generic,
    };

    size_t interface_count = 5;
    const RtClass** interfaces = klass->image->get_mem_pool().calloc_any<const RtClass*>(interface_count);
    for (size_t i = 0; i < interface_count; ++i)
    {
        uint32_t gid = Class::get_type_def_gid(to_inflate[i]);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtClass*, inflated_iface, GenericClass::get_class(gid, generic_inst));
        interfaces[i] = inflated_iface;
    }
    klass->interfaces = interfaces;
    klass->interface_count = static_cast<uint16_t>(interface_count);
    RET_VOID_OK();
}

// Setup methods for array (Get, Set, Address constructors)
RtResultVoid ArrayClass::setup_methods(RtClass* klass)
{
    uint8_t rank = Class::get_rank(klass);
    if (rank == 0)
        RET_ASSERT_ERR(RtErr::BadImageFormat);

    uint16_t method_count = 3 + (rank == 1 ? 1 : 2);

    const CorLibTypes& corlib = Class::get_corlib_types();
    const RtTypeSig* void_type_sig = corlib.cls_void->by_val;
    const RtTypeSig* int32_type_sig = corlib.cls_int32->by_val;
    const RtTypeSig* element_type_sig = klass->element_class->by_val;

    const RtTypeSig* params_buf[32] = {};
    size_t cur_method_index = 0;
    const RtMethodInfo** methods = klass->image->get_mem_pool().calloc_any<const RtMethodInfo*>(method_count);

    // .ctor(lengths)
    {
        size_t parameter_count = static_cast<size_t>(rank);
        for (size_t i = 0; i < parameter_count; ++i)
            params_buf[i] = int32_type_sig;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtMethodInfo*, ctor_m, build_array_method(klass, STR_CTOR, void_type_sig, params_buf, parameter_count));
        methods[cur_method_index++] = ctor_m;
        if (rank > 1)
        {
            for (size_t i = parameter_count; i < parameter_count * 2; ++i)
                params_buf[i] = int32_type_sig;
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtMethodInfo*, cctor_m,
                                                    build_array_method(klass, STR_CCTOR, void_type_sig, params_buf, parameter_count * 2));
            methods[cur_method_index++] = cctor_m;
        }
    }

    // Set(indices..., value)
    {
        size_t idx_count = static_cast<size_t>(rank);
        for (size_t i = 0; i < idx_count; ++i)
            params_buf[i] = int32_type_sig;
        params_buf[idx_count] = element_type_sig;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtMethodInfo*, set_m,
                                                build_array_method(klass, STR_ARRAY_SET_NZ, void_type_sig, params_buf, idx_count + 1));
        methods[cur_method_index++] = set_m;
    }

    // Get(indices...) -> element
    {
        size_t idx_count = static_cast<size_t>(rank);
        for (size_t i = 0; i < idx_count; ++i)
            params_buf[i] = int32_type_sig;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtMethodInfo*, get_m,
                                                build_array_method(klass, STR_ARRAY_GET_NZ, element_type_sig, params_buf, idx_count));
        methods[cur_method_index++] = get_m;
    }

    // Address(indices...) -> byref element
    {
        size_t idx_count = static_cast<size_t>(rank);
        for (size_t i = 0; i < idx_count; ++i)
            params_buf[i] = int32_type_sig;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtMethodInfo*, addr_m,
                                                build_array_method(klass, STR_ARRAY_ADDRESS_NZ, klass->element_class->by_ref, params_buf, idx_count));
        methods[cur_method_index++] = addr_m;
    }

    assert(cur_method_index == method_count);
    klass->methods = methods;
    klass->method_count = method_count;
    RET_VOID_OK();
}

// Initialize virtual table for array with interface methods
RtResultVoid ArrayClass::setup_vtables(metadata::RtClass* klass)
{
    uint8_t rank = Class::get_rank(klass);

    // Multi-dimensional arrays don't implement collection interfaces
    // Just inherit parent vtable
    if (rank > 1)
    {
        if (klass->parent)
        {
            klass->vtable_count = klass->parent->vtable_count;
            klass->vtable = klass->parent->vtable;
            klass->interface_vtable_offsets = klass->parent->interface_vtable_offsets;
            klass->interface_vtable_offset_count = klass->parent->interface_vtable_offset_count;
        }
        RET_VOID_OK();
    }

    // Single-dimensional arrays: extend vtable with collection interfaces
    const metadata::RtClass* parent = klass->parent;
    size_t total_vtable_count = parent->vtable_count;

    // Build interface vtable offsets (parent offsets + new ones)
    utils::Vector<metadata::RtInterfaceOffset> new_offsets;
    new_offsets.reserve(static_cast<size_t>(klass->interface_count + parent->interface_vtable_offset_count));
    for (size_t i = 0; i < parent->interface_vtable_offset_count; ++i)
        new_offsets.push_back(parent->interface_vtable_offsets[i]);

    for (size_t i = 0; i < klass->interface_count; ++i)
    {
        const metadata::RtClass* iface = klass->interfaces[i];
        new_offsets.push_back(metadata::RtInterfaceOffset{iface, static_cast<uint16_t>(total_vtable_count)});
        total_vtable_count += iface->vtable_count;
    }

    alloc::MemPool& mem_pool = klass->image->get_mem_pool();

    metadata::RtInterfaceOffset* interface_vtable_offsets = mem_pool.calloc_any<metadata::RtInterfaceOffset>(new_offsets.size());
    std::memcpy(interface_vtable_offsets, new_offsets.data(), new_offsets.size() * sizeof(metadata::RtInterfaceOffset));
    klass->interface_vtable_offsets = interface_vtable_offsets;
    klass->interface_vtable_offset_count = static_cast<uint16_t>(new_offsets.size());

    // Build new vtable: copy parent, then interfaces
    metadata::RtVirtualInvokeData* new_vtables = mem_pool.calloc_any<metadata::RtVirtualInvokeData>(total_vtable_count);
    std::memcpy(new_vtables, parent->vtable, parent->vtable_count * sizeof(metadata::RtVirtualInvokeData));

    const char* iface_ilist = "IList`1";
    const char* iface_icollection = "ICollection`1";
    const char* iface_ienumerable = "IEnumerable`1";
    const char* iface_ireadonlycollection = "IReadOnlyCollection`1";
    const char* iface_ireadonlylist = "IReadOnlyList`1";

    const metadata::RtTypeSig* element_type_sig = klass->element_class->by_val;
    size_t current_slot = parent->vtable_count;
    for (size_t i = 0; i < klass->interface_count; ++i)
    {
        const metadata::RtClass* iface = klass->interfaces[i];
        std::memcpy(new_vtables + current_slot, iface->vtable, iface->vtable_count * sizeof(metadata::RtVirtualInvokeData));

        const char* iface_name = iface->name;
        const Vector<ArrayTemplateMethod>* method_list = nullptr;
        if (std::strcmp(iface_name, iface_ilist) == 0)
            method_list = &g_ilistGenericMethods;
        else if (std::strcmp(iface_name, iface_icollection) == 0)
            method_list = &g_icollectionGenericMethods;
        else if (std::strcmp(iface_name, iface_ienumerable) == 0)
            method_list = &g_ienumerableGenericMethods;
        else if (std::strcmp(iface_name, iface_ireadonlycollection) == 0)
            method_list = &g_ireadonlyCollectionGenericMethods;
        else if (std::strcmp(iface_name, iface_ireadonlylist) == 0)
            method_list = &g_ireadonlyListGenericMethods;
        else
            RET_ASSERT_ERR(RtErr::BadImageFormat);

        if (iface->vtable_count != method_list->size())
            RET_ASSERT_ERR(RtErr::BadImageFormat);

        for (size_t j = 0; j < iface->vtable_count; ++j)
        {
            metadata::RtVirtualInvokeData* entry = new_vtables + current_slot + j;
            const metadata::RtMethodInfo* method = entry->method;
            const char* method_name = method->name;
            bool found = false;
            for (const ArrayTemplateMethod& gm : *method_list)
            {
                if (std::strcmp(method_name, gm.original_name) == 0)
                {
                    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, final_m, build_array_generic_method(klass, gm, element_type_sig));
                    entry->method_impl = final_m;
                    found = true;
                    break;
                }
            }
            if (!found)
                RET_ASSERT_ERR(RtErr::BadImageFormat);
        }
        current_slot += iface->vtable_count;
    }

    klass->vtable_count = static_cast<uint16_t>(total_vtable_count);
    klass->vtable = new_vtables;
    RET_VOID_OK();
}

RtResultVoid ArrayClass::initialize()
{
    RET_ERR_ON_FAIL(initialize_array_interface_methods());
    RET_VOID_OK();
}

void ArrayClass::walk_array_classes(metadata::ClassWalkCallback callback, void* userData)
{
    for (auto& entry : g_arrayClassMap)
    {
        callback(entry.second, userData);
    }
}

} // namespace vm
} // namespace leanclr
