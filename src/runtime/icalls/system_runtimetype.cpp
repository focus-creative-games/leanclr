#include "system_runtimetype.h"
#include "icall_base.h"
#include "vm/type.h"
#include "vm/class.h"
#include "vm/rt_string.h"
#include "vm/rt_array.h"
#include "vm/array_class.h"
#include "vm/reflection.h"
#include "vm/object.h"
#include "vm/method.h"
#include "vm/property.h"
#include "vm/field.h"
#include "metadata/metadata_cache.h"
#include "utils/safegptrarray.h"
#include "utils/string_builder.h"
#include "metadata/module_def.h"
#include <cstring>
#include <algorithm>

namespace leanclr
{
namespace icalls
{

// BindingFlags constants (mirrors System.Reflection.BindingFlags)
constexpr int32_t BINDING_FLAGS_DEFAULT = 0x0;
constexpr int32_t BINDING_FLAGS_IGNORE_CASE = 0x1;
constexpr int32_t BINDING_FLAGS_DECLARED_ONLY = 0x2;
constexpr int32_t BINDING_FLAGS_INSTANCE = 0x4;
constexpr int32_t BINDING_FLAGS_STATIC = 0x8;
constexpr int32_t BINDING_FLAGS_PUBLIC = 0x10;
constexpr int32_t BINDING_FLAGS_NON_PUBLIC = 0x20;
constexpr int32_t BINDING_FLAGS_FLATTEN_HIERARCHY = 0x40;

// MemberListType constants
constexpr int32_t MEMBER_LIST_TYPE_ALL = 0;
constexpr int32_t MEMBER_LIST_TYPE_CASE_SENSITIVE = 1;
constexpr int32_t MEMBER_LIST_TYPE_CASE_INSENSITIVE = 2;

// Helper function: case-insensitive string comparison (ASCII only)
static bool is_str_case_insensitive_equal_ascii(const char* s1, const char* s2)
{
    if (s1 == nullptr && s2 == nullptr)
        return true;
    if (s1 == nullptr || s2 == nullptr)
        return false;

    while (*s1 && *s2)
    {
        char c1 = static_cast<char>(std::tolower(static_cast<unsigned char>(*s1)));
        char c2 = static_cast<char>(std::tolower(static_cast<unsigned char>(*s2)));
        if (c1 != c2)
            return false;
        ++s1;
        ++s2;
    }
    return *s1 == *s2;
}

// Helper function: check if a member name matches (with case sensitivity option)
static bool matches_member_name(const char* member_name, const char* search_name, bool case_insensitive)
{
    if (search_name == nullptr)
        return true; // null means match all names

    if (case_insensitive)
        return is_str_case_insensitive_equal_ascii(member_name, search_name);
    else
        return std::strcmp(member_name, search_name) == 0;
}

RtResult<vm::RtReflectionType*> SystemRuntimeType::make_array_type(vm::RtReflectionRuntimeType* runtime_type, int32_t rank) noexcept
{
    if (rank < 0 || rank > static_cast<int32_t>(metadata::RT_MAX_ARRAY_RANK))
        RET_ERR(RtErr::TypeLoad);

    const metadata::RtTypeSig* ele_type_sig = runtime_type->reflection_type.type_handle;

    // Cannot create array of ByRef types
    if (ele_type_sig->by_ref)
        RET_ERR(RtErr::TypeLoad);

    // Cannot create array of TypedByRef
    if (ele_type_sig->ele_type == metadata::RtElementType::TypedByRef)
        RET_ERR(RtErr::TypeLoad);

    // Create array class
    metadata::RtClass* arr_class = nullptr;
    if (rank <= 1)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(arr_class, vm::ArrayClass::get_szarray_class_from_element_typesig(ele_type_sig));
    }
    else
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(arr_class, vm::ArrayClass::get_array_class_from_element_type(ele_type_sig, static_cast<uint8_t>(rank)));
    }

    return vm::Reflection::get_klass_reflection_object(arr_class);
}

RtResult<vm::RtReflectionType*> SystemRuntimeType::make_byref_type(vm::RtReflectionRuntimeType* runtime_type) noexcept
{
    const metadata::RtTypeSig* ele_type_sig = runtime_type->reflection_type.type_handle;

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(ele_type_sig));
    const metadata::RtTypeSig* byref_type_sig = vm::Class::get_by_ref_type_sig(klass);

    return vm::Reflection::get_type_reflection_object(byref_type_sig);
}

RtResult<vm::RtReflectionType*> SystemRuntimeType::make_pointer_type(vm::RtReflectionRuntimeType* runtime_type) noexcept
{
    const metadata::RtTypeSig* ele_type_sig = runtime_type->reflection_type.type_handle;

    // Cannot create pointer to ByRef type
    if (ele_type_sig->by_ref)
        RET_ERR(RtErr::TypeLoad);

    // Cannot create pointer to TypedByRef
    if (ele_type_sig->ele_type == metadata::RtElementType::TypedByRef)
        RET_ERR(RtErr::TypeLoad);

    // Create pointer type signature
    // Note: Pointer type is represented as a type signature with Ptr element type
    // The data field will hold the element type signature as a pointer
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, ptr_class, vm::Class::get_ptr_class_by_element_typesig(ele_type_sig));

    return vm::Reflection::get_klass_reflection_object(ptr_class);
}

RtResult<vm::RtReflectionType*> SystemRuntimeType::make_generic_type(vm::RtReflectionRuntimeType* generic_base_type, vm::RtArray* generic_args) noexcept
{
    const metadata::RtTypeSig* type_sig = generic_base_type->reflection_type.type_handle;

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, generic_def_klass, vm::Class::get_class_from_typesig(type_sig));

    // Check if base type is generic
    if (!vm::Class::is_generic(generic_def_klass))
        RET_ERR(RtErr::Argument);

    // Get generic parameter count from generic container
    const metadata::RtGenericContainer* gc = generic_def_klass->generic_container;
    // Get generic argument array length
    int32_t arg_count = vm::Array::get_array_length(generic_args);
    if (arg_count != gc->generic_param_count)
        RET_ERR(RtErr::Argument);

    const metadata::RtTypeSig** generic_arg_type_sigs = (const metadata::RtTypeSig**)alloca(sizeof(metadata::RtTypeSig*) * static_cast<size_t>(arg_count));
    for (int32_t i = 0; i < arg_count; ++i)
    {
        vm::RtReflectionType* arg_type_ref = vm::Array::get_array_data_at<vm::RtReflectionType*>(generic_args, i);
        generic_arg_type_sigs[i] = arg_type_ref->type_handle;
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtGenericInst*, klass_inst,
                                            metadata::MetadataCache::get_pooled_generic_inst(generic_arg_type_sigs, static_cast<uint8_t>(arg_count)));
    const metadata::RtGenericClass* generic_class =
        metadata::MetadataCache::get_pooled_generic_class(vm::Class::get_type_def_gid(generic_def_klass), klass_inst);

    return vm::Reflection::get_type_reflection_object(&generic_class->by_val_type_sig);
}

RtResult<utils::SafeGPtrArray<metadata::RtMethodInfo>*>
SystemRuntimeType::get_methods_by_name_native(vm::RtReflectionRuntimeType* runtime_type, const char* name, int32_t bind_flags, int32_t list_type) noexcept
{
    const metadata::RtTypeSig* type_sig = runtime_type->reflection_type.type_handle;

    // Cannot get methods from ByRef types
    if (type_sig->by_ref)
    {
        auto empty = utils::SafeGPtrArray<metadata::RtMethodInfo>::create_empty();
        RET_OK(empty);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));

    // Ensure methods are initialized
    RET_ERR_ON_FAIL(vm::Class::initialize_methods(klass));

    // Build list of matching methods
    utils::Vector<const metadata::RtMethodInfo*> methods;
    methods.reserve(klass->method_count);
    bool case_insensitive = (bind_flags & BINDING_FLAGS_IGNORE_CASE) != 0 || list_type == MEMBER_LIST_TYPE_CASE_INSENSITIVE;

    // Traverse class hierarchy
    const metadata::RtClass* current_klass = klass;
    while (current_klass != nullptr)
    {
        for (uint32_t i = 0; i < current_klass->method_count; ++i)
        {
            const metadata::RtMethodInfo* method = current_klass->methods[i];

            // Skip constructors for method enumeration
            if (vm::Method::is_ctor_or_cctor(method))
                continue;

            // Check name match
            if (!matches_member_name(method->name, name, case_insensitive))
                continue;

            // Check public/private access
            if (vm::Method::is_public(method))
            {
                if ((bind_flags & BINDING_FLAGS_PUBLIC) == 0)
                    continue;
            }
            else
            {
                if ((bind_flags & BINDING_FLAGS_NON_PUBLIC) == 0)
                    continue;

                // Private members can't be accessed from derived classes
                if (vm::Method::is_private(method) && current_klass != klass)
                    continue;
            }

            // Check static/instance
            if (vm::Method::is_static(method))
            {
                if ((bind_flags & BINDING_FLAGS_STATIC) == 0)
                    continue;

                // Static members from base classes are only included with FlattenHierarchy flag
                if (current_klass != klass && (bind_flags & BINDING_FLAGS_FLATTEN_HIERARCHY) == 0)
                    continue;
            }
            else
            {
                if ((bind_flags & BINDING_FLAGS_INSTANCE) == 0)
                    continue;
            }

            methods.push_back(method);
        }

        // Stop if DeclaredOnly flag is set
        if ((bind_flags & BINDING_FLAGS_DECLARED_ONLY) != 0)
            break;

        current_klass = current_klass->parent;
    }

    // Create SafeGPtrArray with collected methods
    auto result = utils::SafeGPtrArray<metadata::RtMethodInfo>::create_from_data(methods.data(), static_cast<int32_t>(methods.size()));
    RET_OK(result);
}

RtResult<utils::SafeGPtrArray<metadata::RtPropertyInfo>*>
SystemRuntimeType::get_properties_by_name_native(vm::RtReflectionRuntimeType* runtime_type, const char* name, int32_t bind_flags, int32_t list_type) noexcept
{
    const metadata::RtTypeSig* type_sig = runtime_type->reflection_type.type_handle;

    // Cannot get properties from ByRef types
    if (type_sig->by_ref)
    {
        auto empty = utils::SafeGPtrArray<metadata::RtPropertyInfo>::create_empty();
        RET_OK(empty);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));

    // Ensure properties are initialized
    RET_ERR_ON_FAIL(vm::Class::initialize_properties(klass));

    // Build list of matching properties
    utils::Vector<const metadata::RtPropertyInfo*> properties;
    properties.reserve(klass->property_count);
    bool case_insensitive = (bind_flags & BINDING_FLAGS_IGNORE_CASE) != 0 || list_type == MEMBER_LIST_TYPE_CASE_INSENSITIVE;

    const metadata::RtClass* current_klass = klass;
    while (current_klass != nullptr)
    {
        for (uint32_t i = 0; i < current_klass->property_count; ++i)
        {
            const metadata::RtPropertyInfo* property = current_klass->properties + i;

            // Check name match
            if (!matches_member_name(property->name, name, case_insensitive))
                continue;

            // Check public/private (properties are typically public unless they have private accessors)
            bool prop_is_public = vm::Property::is_public(property);
            if (prop_is_public)
            {
                if ((bind_flags & BINDING_FLAGS_PUBLIC) == 0)
                    continue;
            }
            else
            {
                if ((bind_flags & BINDING_FLAGS_NON_PUBLIC) == 0)
                    continue;
            }

            // Check static/instance
            if (vm::Property::is_static(property))
            {
                if ((bind_flags & BINDING_FLAGS_STATIC) == 0)
                    continue;

                if (current_klass != klass && (bind_flags & BINDING_FLAGS_FLATTEN_HIERARCHY) == 0)
                    continue;
            }
            else
            {
                if ((bind_flags & BINDING_FLAGS_INSTANCE) == 0)
                    continue;
            }

            properties.push_back(property);
        }

        if ((bind_flags & BINDING_FLAGS_DECLARED_ONLY) != 0)
            break;

        current_klass = current_klass->parent;
    }

    auto result = utils::SafeGPtrArray<metadata::RtPropertyInfo>::create_from_data(properties.data(), static_cast<int32_t>(properties.size()));
    RET_OK(result);
}

RtResult<utils::SafeGPtrArray<metadata::RtMethodInfo>*> SystemRuntimeType::get_constructors_native(vm::RtReflectionRuntimeType* runtime_type,
                                                                                                   int32_t bind_flags) noexcept
{
    const metadata::RtTypeSig* type_sig = runtime_type->reflection_type.type_handle;

    // Cannot get constructors from ByRef types
    if (type_sig->by_ref)
    {
        auto empty = utils::SafeGPtrArray<metadata::RtMethodInfo>::create_empty();
        RET_OK(empty);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));

    // Ensure methods are initialized
    RET_ERR_ON_FAIL(vm::Class::initialize_methods(klass));

    // Build list of constructors (only from current class, not base classes)
    utils::Vector<const metadata::RtMethodInfo*> constructors;
    constructors.reserve(klass->method_count);

    for (uint32_t i = 0; i < klass->method_count; ++i)
    {
        const metadata::RtMethodInfo* method = klass->methods[i];

        // Only include constructors
        if (!vm::Method::is_ctor_or_cctor(method))
            continue;

        // Check public/private
        if (vm::Method::is_public(method))
        {
            if ((bind_flags & BINDING_FLAGS_PUBLIC) == 0)
                continue;
        }
        else
        {
            if ((bind_flags & BINDING_FLAGS_NON_PUBLIC) == 0)
                continue;
        }

        // Constructors are instance methods
        if ((bind_flags & BINDING_FLAGS_INSTANCE) == 0)
            continue;

        constructors.push_back(method);
    }

    auto result = utils::SafeGPtrArray<metadata::RtMethodInfo>::create_from_data(constructors.data(), static_cast<int32_t>(constructors.size()));
    RET_OK(result);
}

RtResult<utils::SafeGPtrArray<metadata::RtEventInfo>*> SystemRuntimeType::get_events_native(vm::RtReflectionRuntimeType* runtime_type, const char* name,
                                                                                            int32_t list_type) noexcept
{
    if (runtime_type == nullptr)
        RET_ERR(RtErr::ArgumentNull);

    const metadata::RtTypeSig* type_sig = runtime_type->reflection_type.type_handle;

    // Cannot get events from ByRef types
    if (type_sig->by_ref)
    {
        auto empty = utils::SafeGPtrArray<metadata::RtEventInfo>::create_empty();
        RET_OK(empty);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));

    // Ensure events are initialized
    RET_ERR_ON_FAIL(vm::Class::initialize_events(klass));

    // Build list of matching events
    utils::Vector<const metadata::RtEventInfo*> events;
    events.reserve(klass->event_count);
    bool case_insensitive = list_type == MEMBER_LIST_TYPE_CASE_INSENSITIVE;

    // For events, we typically search the entire hierarchy
    const metadata::RtClass* current_klass = klass;
    while (current_klass != nullptr)
    {
        for (uint32_t i = 0; i < current_klass->event_count; ++i)
        {
            const metadata::RtEventInfo* event = current_klass->events + i;

            // Check name match
            if (!matches_member_name(event->name, name, case_insensitive))
                continue;

            events.push_back(event);
        }

        current_klass = current_klass->parent;
    }

    auto result = utils::SafeGPtrArray<metadata::RtEventInfo>::create_from_data(events.data(), static_cast<int32_t>(events.size()));
    RET_OK(result);
}

RtResult<utils::SafeGPtrArray<metadata::RtFieldInfo>*> SystemRuntimeType::get_fields_native(vm::RtReflectionRuntimeType* runtime_type, const char* name,
                                                                                            int32_t bind_flags, int32_t list_type) noexcept
{
    if (runtime_type == nullptr)
        RET_ERR(RtErr::ArgumentNull);

    const metadata::RtTypeSig* type_sig = runtime_type->reflection_type.type_handle;

    // Cannot get fields from ByRef types
    if (type_sig->by_ref)
    {
        auto empty = utils::SafeGPtrArray<metadata::RtFieldInfo>::create_empty();
        RET_OK(empty);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));

    // Ensure fields are initialized
    RET_ERR_ON_FAIL(vm::Class::initialize_fields(klass));

    // Build list of matching fields
    utils::Vector<const metadata::RtFieldInfo*> fields;
    fields.reserve(klass->field_count);
    bool case_insensitive = (bind_flags & BINDING_FLAGS_IGNORE_CASE) != 0 || list_type == MEMBER_LIST_TYPE_CASE_INSENSITIVE;

    const metadata::RtClass* current_klass = klass;
    while (current_klass != nullptr)
    {
        for (uint32_t i = 0; i < current_klass->field_count; ++i)
        {
            const metadata::RtFieldInfo* field = current_klass->fields + i;

            // Check name match
            if (!matches_member_name(field->name, name, case_insensitive))
                continue;

            // Check public/private
            if (vm::Field::is_public(field))
            {
                if ((bind_flags & BINDING_FLAGS_PUBLIC) == 0)
                    continue;
            }
            else
            {
                if ((bind_flags & BINDING_FLAGS_NON_PUBLIC) == 0)
                    continue;

                if (vm::Field::is_private(field) && current_klass != klass)
                    continue;
            }

            // Check static/instance
            if (vm::Field::is_static_included_literal_and_rva(field))
            {
                if ((bind_flags & BINDING_FLAGS_STATIC) == 0)
                    continue;

                if (current_klass != klass && (bind_flags & BINDING_FLAGS_FLATTEN_HIERARCHY) == 0)
                    continue;
            }
            else
            {
                if ((bind_flags & BINDING_FLAGS_INSTANCE) == 0)
                    continue;
            }

            fields.push_back(field);
        }

        if ((bind_flags & BINDING_FLAGS_DECLARED_ONLY) != 0)
            break;

        current_klass = current_klass->parent;
    }

    auto result = utils::SafeGPtrArray<metadata::RtFieldInfo>::create_from_data(fields.data(), static_cast<int32_t>(fields.size()));
    RET_OK(result);
}

RtResultVoid SystemRuntimeType::get_interface_map_data(vm::RtReflectionRuntimeType* runtime_type, vm::RtReflectionRuntimeType* interface_type,
                                                       vm::RtArray** targets, vm::RtArray** methods) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(runtime_type->reflection_type.type_handle));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, interface_klass,
                                            vm::Class::get_class_from_typesig(interface_type->reflection_type.type_handle));
    int32_t interface_vir_method_count = 0;
    for (int32_t i = 0; i < interface_klass->method_count; ++i)
    {
        const metadata::RtMethodInfo* method = interface_klass->methods[i];
        if (vm::Method::is_virtual(method))
            interface_vir_method_count++;
    }
    metadata::RtClass* cls_method_info = vm::Class::get_corlib_types().cls_reflection_method;

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, target_array, vm::Array::new_szarray_from_ele_klass(cls_method_info, interface_vir_method_count));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, method_array, vm::Array::new_szarray_from_ele_klass(cls_method_info, interface_vir_method_count));

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtVirtualInvokeData*, vtable_start,
                                            vm::Method::get_interface_method_invoke_data(klass, interface_klass, 0));
    for (int32_t i = 0; i < interface_vir_method_count; ++i)
    {
        const metadata::RtVirtualInvokeData& vid = vtable_start[i];

        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionMethod*, target_method_ref,
                                                vm::Reflection::get_method_reflection_object(vid.method, interface_klass));
        vm::RtReflectionMethod* impl_method_ref;
        if (vid.method_impl != nullptr)
        {
            UNWRAP_OR_RET_ERR_ON_FAIL(impl_method_ref, vm::Reflection::get_method_reflection_object(vid.method_impl, klass));
        }
        else
        {
            impl_method_ref = nullptr;
        }

        vm::Array::set_array_data_at<vm::RtReflectionMethod*>(target_array, i, target_method_ref);
        vm::Array::set_array_data_at<vm::RtReflectionMethod*>(method_array, i, impl_method_ref);
    }
    *targets = target_array;
    *methods = method_array;
    RET_VOID_OK();
}

RtResultVoid SystemRuntimeType::get_guid(vm::RtReflectionRuntimeType* runtime_type, vm::RtArray* guid) noexcept
{
    // GUID extraction is rarely used and requires MarshalAs/CustomAttribute metadata
    RETURN_NOT_IMPLEMENTED_ERROR();
}

RtResultVoid SystemRuntimeType::get_packing(vm::RtReflectionRuntimeType* runtime_type, int32_t* packing, int32_t* size) noexcept
{
    const metadata::RtTypeSig* type_sig = runtime_type->reflection_type.type_handle;

    if (type_sig->by_ref)
    {
        *packing = 0;
        *size = 0;
        RET_VOID_OK();
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));
    auto layout = klass->image->get_class_layout_data(klass->token);
    if (layout)
    {
        *packing = static_cast<int32_t>(layout->packing);
        *size = static_cast<int32_t>(layout->size);
    }
    else
    {
        *packing = 0;
        *size = 0;
    }

    RET_VOID_OK();
}

enum class TypeCode : int32_t
{
    Empty = 0,
    Object = 1,
    DBNull = 2,
    Boolean = 3,
    Char = 4,
    SByte = 5,
    Byte = 6,
    Int16 = 7,
    UInt16 = 8,
    Int32 = 9,
    UInt32 = 10,
    Int64 = 11,
    UInt64 = 12,
    Single = 13,
    Double = 14,
    Decimal = 15,
    DateTime = 16,
    String = 18,
};

RtResult<TypeCode> get_type_code(const metadata::RtTypeSig* type_sig) noexcept
{
    if (type_sig->by_ref)
    {
        RET_OK(TypeCode::Object);
    }

    switch (type_sig->ele_type)
    {
    case metadata::RtElementType::Void:
        RET_OK(TypeCode::Object); // Void is mapped to Object in .NET
    case metadata::RtElementType::Boolean:
        RET_OK(TypeCode::Boolean);
    case metadata::RtElementType::Char:
        RET_OK(TypeCode::Char);
    case metadata::RtElementType::I1:
        RET_OK(TypeCode::SByte);
    case metadata::RtElementType::U1:
        RET_OK(TypeCode::Byte);
    case metadata::RtElementType::I2:
        RET_OK(TypeCode::Int16);
    case metadata::RtElementType::U2:
        RET_OK(TypeCode::UInt16);
    case metadata::RtElementType::I4:
        RET_OK(TypeCode::Int32);
    case metadata::RtElementType::U4:
        RET_OK(TypeCode::UInt32);
    case metadata::RtElementType::I8:
        RET_OK(TypeCode::Int64);
    case metadata::RtElementType::U8:
        RET_OK(TypeCode::UInt64);
    case metadata::RtElementType::R4:
        RET_OK(TypeCode::Single);
    case metadata::RtElementType::R8:
        RET_OK(TypeCode::Double);
    case metadata::RtElementType::String:
        RET_OK(TypeCode::String);
    case metadata::RtElementType::Array:
    case metadata::RtElementType::SZArray:
    case metadata::RtElementType::Var:
    case metadata::RtElementType::MVar:
    case metadata::RtElementType::TypedByRef:
    case metadata::RtElementType::Object:
    case metadata::RtElementType::GenericInst:
    case metadata::RtElementType::I:
    case metadata::RtElementType::U:
    case metadata::RtElementType::Ptr:
    case metadata::RtElementType::FnPtr:
        RET_OK(TypeCode::Object);
    case metadata::RtElementType::ValueType:
    {
        // For value types, check if it's an enum or a known type like Decimal/DateTime
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));

        if (vm::Class::is_enum_type(klass))
        {
            const metadata::RtTypeSig* underlying_type_sig = klass->element_class->by_val;
            return get_type_code(underlying_type_sig);
        }

        // Check for System.Decimal
        if (klass->image->is_corlib() && std::strcmp(klass->name, "Decimal") == 0)
        {
            RET_OK(TypeCode::Decimal);
        }

        // Check for System.DateTime
        if (klass->image->is_corlib() && std::strcmp(klass->name, "DateTime") == 0)
        {
            RET_OK(TypeCode::DateTime);
        }

        RET_OK(TypeCode::Object);
    }
    case metadata::RtElementType::Class:
    {
        // Check for System.DBNull
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));

        if (klass->image->is_corlib() && std::strcmp(klass->name, "DBNull") == 0)
        {
            RET_OK(TypeCode::DBNull);
        }

        RET_OK(TypeCode::Object);
    }
    default:
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    }
}

RtResult<int32_t> SystemRuntimeType::get_type_code_impl_internal(vm::RtReflectionRuntimeType* runtime_type) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(TypeCode, type_code, get_type_code(runtime_type->reflection_type.type_handle));
    RET_OK((int32_t)type_code);
}

RtResult<vm::RtObject*> SystemRuntimeType::create_instance_internal(vm::RtReflectionRuntimeType* runtime_type) noexcept
{
    if (runtime_type == nullptr)
        RET_OK(nullptr);

    const metadata::RtTypeSig* type_sig = runtime_type->reflection_type.type_handle;

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));

    // Check if type is nullable - return null
    if (vm::Class::is_nullable_type(klass))
        RET_OK(nullptr);

    // Check if type is array or SZArray - cannot create without specifying length
    if (vm::Class::is_array_or_szarray(klass))
        RET_ERR(RtErr::MissingMethod);

    // Create default instance
    return vm::Object::new_object(klass);
}

RtResult<vm::RtReflectionMethod*> SystemRuntimeType::get_declaring_method(vm::RtReflectionRuntimeType* runtime_type) noexcept
{
    if (runtime_type == nullptr)
        RET_OK(nullptr);

    const metadata::RtTypeSig* type_sig = runtime_type->reflection_type.type_handle;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method, vm::Type::get_declaring_method_of_mvar(type_sig));
    if (method)
    {
        return vm::Reflection::get_method_reflection_object(method, method->parent);
    }
    RET_OK(nullptr);
}

RtResult<vm::RtString*> SystemRuntimeType::get_full_name(vm::RtReflectionRuntimeType* runtime_type, bool full_name, bool assembly_qualified) noexcept
{
    return vm::Type::get_full_name(runtime_type->reflection_type.type_handle, full_name, assembly_qualified);
}

RtResult<vm::RtArray*> SystemRuntimeType::get_generic_arguments_internal(vm::RtReflectionRuntimeType* runtime_type, bool runtime_array) noexcept
{
    if (runtime_type == nullptr)
        RET_ERR(RtErr::ArgumentNull);

    const metadata::RtTypeSig* type_sig = runtime_type->reflection_type.type_handle;

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));

    const auto& corlib_types = vm::Class::get_corlib_types();
    metadata::RtClass* result_ele_klass = runtime_array ? corlib_types.cls_runtimetype : corlib_types.cls_systemtype;

    // Check if this class is a generic type definition
    if (vm::Class::is_generic(klass))
    {
        // Return generic parameters
        const metadata::RtGenericContainer* gc = klass->generic_container;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, new_array, vm::Array::new_szarray_from_ele_klass(result_ele_klass, gc->generic_param_count));

        for (uint32_t i = 0; i < gc->generic_param_count; ++i)
        {
            // Create type signature for generic parameter
            const metadata::RtGenericParam* param = &gc->generic_params[i];
            metadata::RtTypeSig generic_param_type_sig = metadata::RtTypeSig::new_byval_with_data(metadata::RtElementType::Var, param);
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, generic_param_type_ref,
                                                    vm::Reflection::get_type_reflection_object(&generic_param_type_sig));
            vm::Array::set_array_data_at<vm::RtReflectionType*>(new_array, static_cast<int32_t>(i), generic_param_type_ref);
        }
        RET_OK(new_array);
    }
    else if (type_sig->ele_type == metadata::RtElementType::GenericInst)
    {
        // Return generic arguments from instantiated generic type
        const metadata::RtGenericInst* generic_inst = type_sig->data.generic_class->class_inst;
        uint8_t arg_count = generic_inst->generic_arg_count;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, new_array, vm::Array::new_szarray_from_ele_klass(result_ele_klass, arg_count));

        for (uint8_t i = 0; i < arg_count; ++i)
        {
            const metadata::RtTypeSig* arg_type_sig = generic_inst->generic_args[i];
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, arg_type_ref, vm::Reflection::get_type_reflection_object(arg_type_sig));
            vm::Array::set_array_data_at<vm::RtReflectionType*>(new_array, i, arg_type_ref);
        }
        RET_OK(new_array);
    }
    else
    {
        // Return empty array if not generic
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, empty_array, vm::Array::new_empty_szarray_by_ele_klass(result_ele_klass));
        RET_OK(empty_array);
    }
}

RtResult<int32_t> SystemRuntimeType::get_generic_parameter_position(vm::RtReflectionRuntimeType* runtime_type) noexcept
{
    const metadata::RtTypeSig* type_sig = runtime_type->reflection_type.type_handle;

    // Check if this is a generic variable or method variable
    int32_t pos = vm::Type::is_generic_param(type_sig) ? static_cast<int32_t>(type_sig->data.generic_param->index) : -1;
    RET_OK(pos);
}

RtResult<vm::RtReflectionType*> SystemRuntimeType::get_declaring_type(vm::RtReflectionRuntimeType* runtime_type) noexcept
{
    const metadata::RtTypeSig* type_sig = runtime_type->reflection_type.type_handle;
    // Get the enclosing/declaring class
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, enclosing_klass, vm::Type::get_declaring_type(type_sig));

    if (enclosing_klass != nullptr)
        return vm::Reflection::get_klass_reflection_object(enclosing_klass);

    RET_OK(nullptr);
}

RtResult<utils::SafeGPtrArray<metadata::RtClass>*> SystemRuntimeType::get_nested_types_native(vm::RtReflectionRuntimeType* runtime_type, const char* name,
                                                                                              int32_t bind_flags) noexcept
{
    if (runtime_type == nullptr)
        RET_ERR(RtErr::ArgumentNull);

    const metadata::RtTypeSig* type_sig = runtime_type->reflection_type.type_handle;

    // Only classes and value types can have nested types
    if (type_sig->by_ref || (type_sig->ele_type != metadata::RtElementType::Class && type_sig->ele_type != metadata::RtElementType::ValueType))
    {
        auto empty = utils::SafeGPtrArray<metadata::RtClass>::create_empty();
        RET_OK(empty);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));

    // Ensure nested classes are initialized
    RET_ERR_ON_FAIL(vm::Class::initialize_nested_classes(klass));

    // Build list of matching nested types
    utils::Vector<const metadata::RtClass*> nested_types;
    nested_types.reserve(klass->nested_class_count);
    bool case_insensitive = (bind_flags & BINDING_FLAGS_IGNORE_CASE) != 0;

    for (uint32_t i = 0; i < klass->nested_class_count; ++i)
    {
        const metadata::RtClass* nested_klass = klass->nested_classes[i];

        // Check name match
        if (!matches_member_name(nested_klass->name, name, case_insensitive))
            continue;

        // Check public/private access
        if (vm::Class::is_nested_public(nested_klass))
        {
            if ((bind_flags & BINDING_FLAGS_PUBLIC) == 0)
                continue;
        }
        else
        {
            if ((bind_flags & BINDING_FLAGS_NON_PUBLIC) == 0)
                continue;
        }

        nested_types.push_back(nested_klass);
    }

    auto result = utils::SafeGPtrArray<metadata::RtClass>::create_from_data(nested_types.data(), static_cast<int32_t>(nested_types.size()));
    RET_OK(result);
}

RtResult<vm::RtString*> SystemRuntimeType::get_name(vm::RtReflectionRuntimeType* runtime_type) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(runtime_type->reflection_type.type_handle));
    return vm::String::create_string_from_utf8cstr(klass->name);
}

RtResult<vm::RtString*> SystemRuntimeType::get_namespace(vm::RtReflectionRuntimeType* runtime_type) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(runtime_type->reflection_type.type_handle));
    return vm::String::create_string_from_utf8cstr(klass->namespaze);
}

RtResult<vm::RtArray*> SystemRuntimeType::get_interfaces(vm::RtReflectionRuntimeType* runtime_type) noexcept
{
    const metadata::RtTypeSig* type_sig = runtime_type->reflection_type.type_handle;

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));

    // Ensure interfaces are initialized
    RET_ERR_ON_FAIL(vm::Class::initialize_interfaces(klass));

    const auto cls_systemtype = vm::Class::get_corlib_types().cls_systemtype;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, interface_array, vm::Array::new_szarray_from_ele_klass(cls_systemtype, klass->interface_count));

    // Fill array with interface type references
    for (uint32_t i = 0; i < klass->interface_count; ++i)
    {
        const metadata::RtClass* interface_klass = klass->interfaces[i];
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, interface_reflection, vm::Reflection::get_klass_reflection_object(interface_klass));
        vm::Array::set_array_data_at<vm::RtReflectionType*>(interface_array, static_cast<int32_t>(i), interface_reflection);
    }

    RET_OK(interface_array);
}

// Invoker wrappers
/// @icall: System.RuntimeType::make_array_type
static RtResultVoid make_array_type_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                            const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    int32_t rank = EvalStackOp::get_param<int32_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, result, SystemRuntimeType::make_array_type(runtime_type, rank));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.RuntimeType::make_byref_type
static RtResultVoid make_byref_type_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                            interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, result, SystemRuntimeType::make_byref_type(runtime_type));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.RuntimeType::MakePointerType
static RtResultVoid make_pointer_type_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                              interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, result, SystemRuntimeType::make_pointer_type(runtime_type));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.RuntimeType::MakeGenericType
static RtResultVoid make_generic_type_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                              interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    auto generic_args = EvalStackOp::get_param<vm::RtArray*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, result, SystemRuntimeType::make_generic_type(runtime_type, generic_args));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.RuntimeType::GetMethodsByName_native
static RtResultVoid get_methods_by_name_native_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                       interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    auto name = EvalStackOp::get_param<const char*>(params, 1);
    int32_t bind_flags = EvalStackOp::get_param<int32_t>(params, 2);
    int32_t list_type = EvalStackOp::get_param<int32_t>(params, 3);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(utils::SafeGPtrArray<metadata::RtMethodInfo>*, result,
                                            SystemRuntimeType::get_methods_by_name_native(runtime_type, name, bind_flags, list_type));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.RuntimeType::GetPropertiesByName_native
static RtResultVoid get_properties_by_name_native_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                          interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    auto name = EvalStackOp::get_param<const char*>(params, 1);
    int32_t bind_flags = EvalStackOp::get_param<int32_t>(params, 2);
    int32_t list_type = EvalStackOp::get_param<int32_t>(params, 3);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(utils::SafeGPtrArray<metadata::RtPropertyInfo>*, result,
                                            SystemRuntimeType::get_properties_by_name_native(runtime_type, name, bind_flags, list_type));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.RuntimeType::GetConstructors_native
static RtResultVoid get_constructors_native_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                    interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    int32_t bind_flags = EvalStackOp::get_param<int32_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(utils::SafeGPtrArray<metadata::RtMethodInfo>*, result,
                                            SystemRuntimeType::get_constructors_native(runtime_type, bind_flags));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.RuntimeType::GetEvents_native
static RtResultVoid get_events_native_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                              interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    auto name = EvalStackOp::get_param<const char*>(params, 1);
    int32_t list_type = EvalStackOp::get_param<int32_t>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(utils::SafeGPtrArray<metadata::RtEventInfo>*, result,
                                            SystemRuntimeType::get_events_native(runtime_type, name, list_type));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.RuntimeType::GetFields_native
static RtResultVoid get_fields_native_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                              interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    auto name = EvalStackOp::get_param<const char*>(params, 1);
    int32_t bind_flags = EvalStackOp::get_param<int32_t>(params, 2);
    int32_t list_type = EvalStackOp::get_param<int32_t>(params, 3);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(utils::SafeGPtrArray<metadata::RtFieldInfo>*, result,
                                            SystemRuntimeType::get_fields_native(runtime_type, name, bind_flags, list_type));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.RuntimeType::GetInterfaceMapData
static RtResultVoid get_interface_map_data_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                   interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    auto interface_type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 1);
    auto targets_ptr = EvalStackOp::get_param<vm::RtArray**>(params, 2);
    auto methods_ptr = EvalStackOp::get_param<vm::RtArray**>(params, 3);
    RET_ERR_ON_FAIL(SystemRuntimeType::get_interface_map_data(runtime_type, interface_type, targets_ptr, methods_ptr));
    RET_VOID_OK();
}

/// @icall: System.RuntimeType::GetGUID
static RtResultVoid get_guid_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                     interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    auto guid_data = EvalStackOp::get_param<vm::RtArray*>(params, 1);
    return SystemRuntimeType::get_guid(runtime_type, guid_data);
}

/// @icall: System.RuntimeType::GetPacking
static RtResultVoid get_packing_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                        interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    auto packing = EvalStackOp::get_param<int32_t*>(params, 1);
    auto size = EvalStackOp::get_param<int32_t*>(params, 2);
    RET_ERR_ON_FAIL(SystemRuntimeType::get_packing(runtime_type, packing, size));
    RET_VOID_OK();
}

/// @icall: System.RuntimeType::GetTypeCodeImplInternal
static RtResultVoid get_type_code_impl_internal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                        interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, SystemRuntimeType::get_type_code_impl_internal(runtime_type));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.RuntimeType::CreateInstanceInternal
static RtResultVoid create_instance_internal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                     interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, result, SystemRuntimeType::create_instance_internal(runtime_type));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.RuntimeType::get_DeclaringMethod
static RtResultVoid get_declaring_method_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                 interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionMethod*, result, SystemRuntimeType::get_declaring_method(runtime_type));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.RuntimeType::getFullName
static RtResultVoid get_full_name_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                          interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    bool full_name = EvalStackOp::get_param<bool>(params, 1);
    bool assembly_qualified = EvalStackOp::get_param<bool>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, result, SystemRuntimeType::get_full_name(runtime_type, full_name, assembly_qualified));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.RuntimeType::GetGenericArgumentsInternal
static RtResultVoid get_generic_arguments_internal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                           interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    bool runtime_array = EvalStackOp::get_param<bool>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, result, SystemRuntimeType::get_generic_arguments_internal(runtime_type, runtime_array));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.RuntimeType::GetGenericParameterPosition
static RtResultVoid get_generic_parameter_position_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                           interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, SystemRuntimeType::get_generic_parameter_position(runtime_type));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.RuntimeType::get_DeclaringType
static RtResultVoid get_declaring_type_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                               interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, result, SystemRuntimeType::get_declaring_type(runtime_type));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.RuntimeType::GetNestedTypes_native
static RtResultVoid get_nested_types_native_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                    interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    auto name = EvalStackOp::get_param<const char*>(params, 1);
    int32_t bind_flags = EvalStackOp::get_param<int32_t>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(utils::SafeGPtrArray<metadata::RtClass>*, result,
                                            SystemRuntimeType::get_nested_types_native(runtime_type, name, bind_flags));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.RuntimeType::get_Name
static RtResultVoid runtimetype_get_name_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                 interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, result, SystemRuntimeType::get_name(runtime_type));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.RuntimeType::get_Namespace
static RtResultVoid get_namespace_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                          interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, result, SystemRuntimeType::get_namespace(runtime_type));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.RuntimeType::GetInterfaces
static RtResultVoid get_interfaces_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                           interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, result, SystemRuntimeType::get_interfaces(runtime_type));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

// Internal call registry
static vm::InternalCallEntry s_internal_call_entries_system_runtimetype[] = {
    {"System.RuntimeType::make_array_type", (vm::InternalCallFunction)&SystemRuntimeType::make_array_type, make_array_type_invoker},
    {"System.RuntimeType::make_byref_type", (vm::InternalCallFunction)&SystemRuntimeType::make_byref_type, make_byref_type_invoker},
    {"System.RuntimeType::MakePointerType", (vm::InternalCallFunction)&SystemRuntimeType::make_pointer_type, make_pointer_type_invoker},
    {"System.RuntimeType::MakeGenericType", (vm::InternalCallFunction)&SystemRuntimeType::make_generic_type, make_generic_type_invoker},
    {"System.RuntimeType::GetMethodsByName_native", (vm::InternalCallFunction)&SystemRuntimeType::get_methods_by_name_native,
     get_methods_by_name_native_invoker},
    {"System.RuntimeType::GetPropertiesByName_native", (vm::InternalCallFunction)&SystemRuntimeType::get_properties_by_name_native,
     get_properties_by_name_native_invoker},
    {"System.RuntimeType::GetConstructors_native", (vm::InternalCallFunction)&SystemRuntimeType::get_constructors_native, get_constructors_native_invoker},
    {"System.RuntimeType::GetEvents_native", (vm::InternalCallFunction)&SystemRuntimeType::get_events_native, get_events_native_invoker},
    {"System.RuntimeType::GetFields_native", (vm::InternalCallFunction)&SystemRuntimeType::get_fields_native, get_fields_native_invoker},
    {"System.RuntimeType::GetInterfaceMapData", (vm::InternalCallFunction)&SystemRuntimeType::get_interface_map_data, get_interface_map_data_invoker},
    {"System.RuntimeType::GetGUID", (vm::InternalCallFunction)&SystemRuntimeType::get_guid, get_guid_invoker},
    {"System.RuntimeType::GetPacking", (vm::InternalCallFunction)&SystemRuntimeType::get_packing, get_packing_invoker},
    {"System.RuntimeType::GetTypeCodeImplInternal", (vm::InternalCallFunction)&SystemRuntimeType::get_type_code_impl_internal,
     get_type_code_impl_internal_invoker},
    {"System.RuntimeType::CreateInstanceInternal", (vm::InternalCallFunction)&SystemRuntimeType::create_instance_internal, create_instance_internal_invoker},
    {"System.RuntimeType::get_DeclaringMethod", (vm::InternalCallFunction)&SystemRuntimeType::get_declaring_method, get_declaring_method_invoker},
    {"System.RuntimeType::getFullName", (vm::InternalCallFunction)&SystemRuntimeType::get_full_name, get_full_name_invoker},
    {"System.RuntimeType::GetGenericArgumentsInternal", (vm::InternalCallFunction)&SystemRuntimeType::get_generic_arguments_internal,
     get_generic_arguments_internal_invoker},
    {"System.RuntimeType::GetGenericParameterPosition", (vm::InternalCallFunction)&SystemRuntimeType::get_generic_parameter_position,
     get_generic_parameter_position_invoker},
    {"System.RuntimeType::get_DeclaringType", (vm::InternalCallFunction)&SystemRuntimeType::get_declaring_type, get_declaring_type_invoker},
    {"System.RuntimeType::GetNestedTypes_native", (vm::InternalCallFunction)&SystemRuntimeType::get_nested_types_native, get_nested_types_native_invoker},
    {"System.RuntimeType::get_Name", (vm::InternalCallFunction)&SystemRuntimeType::get_name, runtimetype_get_name_invoker},
    {"System.RuntimeType::get_Namespace", (vm::InternalCallFunction)&SystemRuntimeType::get_namespace, get_namespace_invoker},
    {"System.RuntimeType::GetInterfaces", (vm::InternalCallFunction)&SystemRuntimeType::get_interfaces, get_interfaces_invoker},
};

utils::Span<vm::InternalCallEntry> SystemRuntimeType::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_runtimetype,
                                              sizeof(s_internal_call_entries_system_runtimetype) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr
