#include "reflection.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include "core/stl_compat.h"
#include <vector>

#include "rt_array.h"
#include "assembly.h"
#include "class.h"
#include "field.h"
#include "gchandle.h"
#include "method.h"
#include "object.h"
#include "gc/garbage_collector.h"
#include "gc/gc_roots.h"
#include "runtime.h"
#include "type.h"
#include "rt_string.h"
#include "rt_exception.h"
#include "parameter.h"
#include "alloc/general_allocation.h"
#include "metadata/metadata_cache.h"
#include "metadata/metadata_compare.h"
#include "metadata/metadata_hash.h"
#include "metadata/module_def.h"
#include "metadata/metadata_name.h"
#include "utils/hash_util.h"
#include "utils/hashmap.h"
#include "utils/string_builder.h"
#include "alloc/metadata_allocation.h"

namespace leanclr
{
namespace vm
{
namespace
{
class ScopedRtObjectRoot
{
  public:
    explicit ScopedRtObjectRoot(RtObject** slot) noexcept : slot_(slot)
    {
        gc::GcRoots::register_slot(slot_);
    }

    ~ScopedRtObjectRoot()
    {
        gc::GcRoots::unregister_slot(slot_);
    }

    ScopedRtObjectRoot(const ScopedRtObjectRoot&) = delete;
    ScopedRtObjectRoot& operator=(const ScopedRtObjectRoot&) = delete;

  private:
    RtObject** slot_;
};

struct MethodKey
{
    const metadata::RtMethodInfo* method;
    const metadata::RtClass* klass;
};

struct MethodObjectData
{
    const metadata::RtMethodInfo* method;
    const metadata::RtClass* klass;
};

struct RuntimeMethodInfoStubLayout : public RtObject
{
    RtObject* keep_alive;
    RtObject* a;
    RtObject* b;
    RtObject* c;
    RtObject* d;
    RtObject* e;
    RtObject* f;
    RtObject* g;
    RtObject* h;
    const metadata::RtMethodInfo* value;
};

struct FieldKey
{
    const metadata::RtFieldInfo* field;
    const metadata::RtClass* klass;
};

struct FieldObjectData
{
    const metadata::RtFieldInfo* field;
    const metadata::RtClass* klass;
};

struct PropertyKey
{
    const metadata::RtPropertyInfo* property;
    const metadata::RtClass* klass;
};

struct PropertyObjectData
{
    const metadata::RtPropertyInfo* property;
    const metadata::RtClass* klass;
};

struct EventKey
{
    metadata::RtEventInfo* event_info;
    const metadata::RtClass* klass;
};

struct EventObjectData
{
    metadata::RtEventInfo* event_info;
    const metadata::RtClass* klass;
};

struct MethodKeyHash
{
    size_t operator()(const MethodKey& key) const noexcept
    {
        size_t h = std::hash<const void*>()(key.method);
        return utils::HashUtil::combine_hash(h, std::hash<const metadata::RtClass*>()(key.klass));
    }
};

struct MethodKeyEqual
{
    bool operator()(const MethodKey& a, const MethodKey& b) const noexcept
    {
        return a.method == b.method && a.klass == b.klass;
    }
};

struct FieldKeyHash
{
    size_t operator()(const FieldKey& key) const noexcept
    {
        size_t h = std::hash<const void*>()(key.field);
        return utils::HashUtil::combine_hash(h, std::hash<const metadata::RtClass*>()(key.klass));
    }
};

struct FieldKeyEqual
{
    bool operator()(const FieldKey& a, const FieldKey& b) const noexcept
    {
        return a.field == b.field && a.klass == b.klass;
    }
};

struct PropertyKeyHash
{
    size_t operator()(const PropertyKey& key) const noexcept
    {
        size_t h = std::hash<const void*>()(key.property);
        return utils::HashUtil::combine_hash(h, std::hash<const metadata::RtClass*>()(key.klass));
    }
};

struct PropertyKeyEqual
{
    bool operator()(const PropertyKey& a, const PropertyKey& b) const noexcept
    {
        return a.property == b.property && a.klass == b.klass;
    }
};

struct EventKeyHash
{
    size_t operator()(const EventKey& key) const noexcept
    {
        size_t h = std::hash<metadata::RtEventInfo*>()(key.event_info);
        return utils::HashUtil::combine_hash(h, std::hash<const metadata::RtClass*>()(key.klass));
    }
};

struct EventKeyEqual
{
    bool operator()(const EventKey& a, const EventKey& b) const noexcept
    {
        return a.event_info == b.event_info && a.klass == b.klass;
    }
};

static utils::HashMap<const metadata::RtTypeSig*, RtReflectionType*, metadata::TypeSigIgnoreAttrsHasher, metadata::TypeSigIgnoreAttrsEqual>
    s_class_reflection_type_map;
static utils::HashMap<const metadata::RtClass*, RtReflectionType*> s_klass_reflection_type_map;
static utils::HashMap<MethodKey, RtReflectionMethod*, MethodKeyHash, MethodKeyEqual> s_method_reflection_map;
static utils::HashMap<RtReflectionMethod*, MethodObjectData> s_method_object_data_map;
static utils::HashMap<MethodKey, RtArray*, MethodKeyHash, MethodKeyEqual> s_method_params_map;
static utils::HashMap<FieldKey, RtReflectionField*, FieldKeyHash, FieldKeyEqual> s_field_reflection_map;
static utils::HashMap<RtReflectionField*, FieldObjectData> s_field_object_data_map;
static utils::HashMap<PropertyKey, RtReflectionProperty*, PropertyKeyHash, PropertyKeyEqual> s_property_reflection_map;
static utils::HashMap<RtReflectionProperty*, PropertyObjectData> s_property_object_data_map;
static utils::HashMap<EventKey, RtReflectionEventInfo*, EventKeyHash, EventKeyEqual> s_event_reflection_map;
static utils::HashMap<RtReflectionEventInfo*, EventObjectData> s_event_object_data_map;
static utils::HashMap<const metadata::RtAssembly*, RtReflectionAssembly*> s_assembly_reflection_map;
static utils::HashMap<const metadata::RtModuleDef*, RtReflectionModule*> s_module_reflection_map;
static utils::HashMap<const metadata::RtAssembly*, metadata::RtMonoAssemblyName*> s_assembly_name_map;

constexpr int32_t METHOD_ATTRIBUTE_PRIVATE = 0x0001;
constexpr int32_t METHOD_ATTRIBUTE_PUBLIC = 0x0006;
constexpr int32_t METHOD_ATTRIBUTE_MEMBER_ACCESS_MASK = 0x0007;
constexpr int32_t METHOD_ATTRIBUTE_STATIC = 0x0010;
constexpr int32_t BINDING_FLAGS_INSTANCE = 0x0004;
constexpr int32_t BINDING_FLAGS_STATIC = 0x0008;
constexpr int32_t BINDING_FLAGS_PUBLIC = 0x0010;
constexpr int32_t BINDING_FLAGS_NON_PUBLIC = 0x0020;

struct Net10MethodTableFacade
{
    uint32_t flags;
    uint32_t base_size;
    uint32_t flags2;
    uint16_t token_and_flags;
    uint16_t virtuals;
    const Net10MethodTableFacade* parent_method_table;
    void* reserved_module;
    struct Net10MethodTableAuxiliaryData* auxiliary_data;
    void* reserved_writable_data;
    const Net10MethodTableFacade* element_method_table;
    void* reserved_interface_map;
    const metadata::RtClass* klass;
    const metadata::RtTypeSig* type_sig;
};

struct Net10MethodTableAuxiliaryData
{
    uint32_t flags;
    void* loader_module;
    intptr_t exposed_class_object_raw;
};

static_assert(offsetof(Net10MethodTableFacade, parent_method_table) == 0x10,
              "net10 MethodTable facade must expose ParentMethodTable at CoreLib's expected offset");
static_assert(offsetof(Net10MethodTableFacade, auxiliary_data) == 0x20,
              "net10 MethodTable facade must expose AuxiliaryData at CoreLib's expected offset");
static_assert(offsetof(Net10MethodTableFacade, element_method_table) == 0x30,
              "net10 MethodTable facade must expose ElementType at CoreLib's expected offset");
static_assert(offsetof(Net10MethodTableFacade, reserved_interface_map) == 0x38,
              "net10 MethodTable facade must preserve CoreLib's InterfaceMap offset");
static_assert(offsetof(Net10MethodTableAuxiliaryData, exposed_class_object_raw) == 0x10,
              "net10 MethodTable auxiliary data must expose ExposedClassObjectRaw at CoreLib's expected offset");

static utils::HashMap<const metadata::RtTypeSig*, Net10MethodTableFacade*> s_net10_method_table_by_type_sig;
static utils::HashMap<const void*, Net10MethodTableFacade*> s_net10_method_table_by_handle;

static bool is_system_type_named(const metadata::RtClass* klass, const char* name) noexcept
{
    if (klass == nullptr || klass->namespaze == nullptr || klass->name == nullptr)
    {
        return false;
    }

    return std::strcmp(klass->namespaze, "System") == 0 && std::strcmp(klass->name, name) == 0;
}

static bool is_runtime_method_info_stub_class(const metadata::RtClass* klass) noexcept
{
    return is_system_type_named(klass, "RuntimeMethodInfoStub");
}

static bool is_runtime_field_info_stub_class(const metadata::RtClass* klass) noexcept
{
    return is_system_type_named(klass, "RuntimeFieldInfoStub");
}

static bool is_runtime_field_handle_internal_class(const metadata::RtClass* klass) noexcept
{
    return is_system_type_named(klass, "RuntimeFieldHandleInternal");
}

static bool is_method_metadata_pointer(const metadata::RtMethodInfo* method) noexcept
{
    if (method == nullptr || method->parent == nullptr || method->parent->methods == nullptr)
    {
        return false;
    }

    for (uint16_t i = 0; i < method->parent->method_count; ++i)
    {
        if (method->parent->methods[i] == method)
        {
            return true;
        }
    }

    return false;
}

static bool is_field_metadata_pointer(const metadata::RtFieldInfo* field) noexcept
{
    if (field == nullptr || field->parent == nullptr || field->type_sig == nullptr)
    {
        return false;
    }

    metadata::RtToken token = metadata::RtToken::decode(field->token);
    return token.table_type == metadata::TableType::Field && token.rid != 0;
}

static bool is_property_metadata_pointer(const metadata::RtPropertyInfo* property) noexcept
{
    if (property == nullptr || property->parent == nullptr || property->name == nullptr)
    {
        return false;
    }

    metadata::RtToken token = metadata::RtToken::decode(property->token);
    return token.table_type == metadata::TableType::Property && token.rid != 0;
}

static bool is_event_metadata_pointer(const metadata::RtEventInfo* event_info) noexcept
{
    if (event_info == nullptr || event_info->parent == nullptr || event_info->name == nullptr)
    {
        return false;
    }

    metadata::RtToken token = metadata::RtToken::decode(event_info->token);
    return token.table_type == metadata::TableType::Event && token.rid != 0;
}

static RtResultVoid set_runtime_object_field_value(RtObject* obj, const char* field_name, const void* value) noexcept
{
    const metadata::RtFieldInfo* field = Class::get_field_for_name(obj->klass, field_name, true);
    if (field == nullptr)
    {
        RET_ERR(RtErr::MissingField);
    }

    return Field::set_instance_value(field, obj, value);
}

static RtResultVoid set_runtime_object_field_if_exists(const metadata::RtClass* klass, RtObject* obj, const char* field_name, const void* value) noexcept
{
    const metadata::RtFieldInfo* field = Class::get_field_for_name(klass, field_name, true);
    if (field == nullptr)
    {
        RET_VOID_OK();
    }

    return Field::set_instance_value(field, obj, value);
}

static int32_t get_method_binding_flags(const metadata::RtMethodInfo* method) noexcept
{
    if (method == nullptr)
    {
        return BINDING_FLAGS_NON_PUBLIC | BINDING_FLAGS_INSTANCE;
    }

    bool is_public = (method->flags & METHOD_ATTRIBUTE_MEMBER_ACCESS_MASK) == METHOD_ATTRIBUTE_PUBLIC;
    bool is_static = (method->flags & METHOD_ATTRIBUTE_STATIC) != 0;
    return (is_public ? BINDING_FLAGS_PUBLIC : BINDING_FLAGS_NON_PUBLIC) |
           (is_static ? BINDING_FLAGS_STATIC : BINDING_FLAGS_INSTANCE);
}

static int32_t get_property_binding_flags(const metadata::RtPropertyInfo* property, bool* is_private) noexcept
{
    bool any_public = false;
    bool any_static = false;
    bool all_private = true;
    const metadata::RtMethodInfo* methods[] = {property->get_method, property->set_method};
    for (size_t i = 0; i < sizeof(methods) / sizeof(methods[0]); ++i)
    {
        const metadata::RtMethodInfo* method = methods[i];
        if (method == nullptr)
        {
            continue;
        }

        int32_t visibility = method->flags & METHOD_ATTRIBUTE_MEMBER_ACCESS_MASK;
        if (visibility == METHOD_ATTRIBUTE_PUBLIC)
        {
            any_public = true;
            all_private = false;
        }
        else if (visibility != METHOD_ATTRIBUTE_PRIVATE)
        {
            all_private = false;
        }
        if ((method->flags & METHOD_ATTRIBUTE_STATIC) != 0)
        {
            any_static = true;
        }
    }

    if (is_private != nullptr)
    {
        *is_private = all_private;
    }
    return (any_public ? BINDING_FLAGS_PUBLIC : BINDING_FLAGS_NON_PUBLIC) |
           (any_static ? BINDING_FLAGS_STATIC : BINDING_FLAGS_INSTANCE);
}

static bool has_public_key_token(const uint8_t* token, size_t length) noexcept
{
    if (token == nullptr)
    {
        return false;
    }

    for (size_t i = 0; i < length; ++i)
    {
        if (token[i] != 0)
        {
            return true;
        }
    }
    return false;
}

static RtResult<RtArray*> create_public_key_token_array(const metadata::RtAssemblyName& assembly_name) noexcept
{
    constexpr int32_t token_length = 8;
    if (!has_public_key_token(assembly_name.public_key_token, token_length))
    {
        RET_OK(nullptr);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, token_array,
                                            LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERNAL(Class::get_corlib_types().cls_byte,
                                                                                       token_length,
                                                                                       "Reflection::create_public_key_token_array"));
    for (int32_t i = 0; i < token_length; ++i)
    {
        Array::set_array_data_at<uint8_t>(token_array, i, assembly_name.public_key_token[i]);
    }
    RET_OK(token_array);
}

static RtResult<RtObject*> create_runtime_version_object(const metadata::RtAssemblyName& assembly_name) noexcept
{
    metadata::RtModuleDef* corlib = metadata::RtModuleDef::get_corlib_module();
    if (corlib == nullptr)
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, version_klass,
                                            corlib->get_class_by_name("System.Version", false, true));
    RET_ERR_ON_FAIL(Class::initialize_fields(version_klass));

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, version_obj,
                                            LEANCLR_NEWOBJ_INTERNAL(version_klass, "Reflection::create_runtime_version_object"));

    int32_t major = assembly_name.version_major;
    int32_t minor = assembly_name.version_minor;
    int32_t build = assembly_name.version_build;
    int32_t revision = assembly_name.version_revision;
    RET_ERR_ON_FAIL(set_runtime_object_field_if_exists(version_klass, version_obj, "_Major", &major));
    RET_ERR_ON_FAIL(set_runtime_object_field_if_exists(version_klass, version_obj, "_Minor", &minor));
    RET_ERR_ON_FAIL(set_runtime_object_field_if_exists(version_klass, version_obj, "_Build", &build));
    RET_ERR_ON_FAIL(set_runtime_object_field_if_exists(version_klass, version_obj, "_Revision", &revision));
    RET_OK(version_obj);
}

static RtResult<const metadata::RtMethodInfo*> get_method_info_from_runtime_method_info_stub(RtObject* obj)
{
    const metadata::RtFieldInfo* value_field = Class::get_field_for_name(obj->klass, "m_value", true);
    if (value_field == nullptr)
    {
        value_field = Class::get_field_for_name(obj->klass, "value", true);
    }
    if (value_field != nullptr)
    {
        const metadata::RtMethodInfo* method = nullptr;
        RET_ERR_ON_FAIL(Field::get_instance_value(value_field, obj, &method));
        if (method != nullptr)
        {
            RET_OK(method);
        }
    }

    auto stub = reinterpret_cast<RuntimeMethodInfoStubLayout*>(obj);
    if (stub->value != nullptr)
    {
        RET_OK(stub->value);
    }

    RET_ERR(RtErr::ArgumentNull);
}

static RtResult<const metadata::RtFieldInfo*> get_field_info_from_runtime_field_handle_internal(RtObject* obj)
{
    const metadata::RtFieldInfo* handle_field = Class::get_field_for_name(obj->klass, "m_handle", true);
    if (handle_field != nullptr)
    {
        const metadata::RtFieldInfo* field = nullptr;
        RET_ERR_ON_FAIL(Field::get_instance_value(handle_field, obj, &field));
        RET_OK(is_field_metadata_pointer(field) ? field : nullptr);
    }

    const uint8_t* value_ptr = reinterpret_cast<const uint8_t*>(obj) + sizeof(RtObject);
    auto field = *reinterpret_cast<const metadata::RtFieldInfo* const*>(value_ptr);
    RET_OK(is_field_metadata_pointer(field) ? field : nullptr);
}

static RtResult<const metadata::RtFieldInfo*> get_field_info_from_reflection_field_arg(const void* field_arg)
{
    if (field_arg == nullptr)
    {
        RET_OK(nullptr);
    }

    auto raw_obj = reinterpret_cast<RtObject*>(const_cast<void*>(field_arg));
    auto normalized_obj_ret = Reflection::normalize_coreclr_reflection_object(raw_obj);
    if (!normalized_obj_ret.is_ok())
    {
        RET_OK(nullptr);
    }

    RtObject* obj = normalized_obj_ret.unwrap();
    const CorLibTypes& corlib_types = Class::get_corlib_types();
    if (obj->klass != corlib_types.cls_reflection_field && !is_runtime_field_info_stub_class(obj->klass))
    {
        RET_OK(nullptr);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtFieldInfo*, field,
                                            Reflection::get_field_info_from_reflection_object(reinterpret_cast<RtReflectionField*>(obj)));
    RET_OK(is_field_metadata_pointer(field) ? field : nullptr);
}

static RtReflectionRuntimeType* try_get_runtime_type_object(const void* value) noexcept
{
    if (value == nullptr)
    {
        return nullptr;
    }

    auto obj = reinterpret_cast<RtObject*>(const_cast<void*>(value));
    if (!gc::GarbageCollector::is_allocated_object(obj))
    {
        return nullptr;
    }

    if (obj->klass != Class::get_corlib_types().cls_runtimetype)
    {
        return nullptr;
    }

    return reinterpret_cast<RtReflectionRuntimeType*>(obj);
}

static bool is_valid_type_sig_element_type(metadata::RtElementType element_type) noexcept
{
    switch (element_type)
    {
    case metadata::RtElementType::Void:
    case metadata::RtElementType::Boolean:
    case metadata::RtElementType::Char:
    case metadata::RtElementType::I1:
    case metadata::RtElementType::U1:
    case metadata::RtElementType::I2:
    case metadata::RtElementType::U2:
    case metadata::RtElementType::I4:
    case metadata::RtElementType::U4:
    case metadata::RtElementType::I8:
    case metadata::RtElementType::U8:
    case metadata::RtElementType::R4:
    case metadata::RtElementType::R8:
    case metadata::RtElementType::String:
    case metadata::RtElementType::Ptr:
    case metadata::RtElementType::ByRef:
    case metadata::RtElementType::ValueType:
    case metadata::RtElementType::Class:
    case metadata::RtElementType::Var:
    case metadata::RtElementType::Array:
    case metadata::RtElementType::GenericInst:
    case metadata::RtElementType::TypedByRef:
    case metadata::RtElementType::I:
    case metadata::RtElementType::U:
    case metadata::RtElementType::FnPtr:
    case metadata::RtElementType::Object:
    case metadata::RtElementType::SZArray:
    case metadata::RtElementType::MVar:
        return true;
    default:
        return false;
    }
}

static bool looks_like_leanclr_type_sig(const metadata::RtTypeSig* type_sig) noexcept
{
    return type_sig != nullptr && is_valid_type_sig_element_type(type_sig->ele_type) &&
           type_sig->field_or_param_attrs == 0 && !type_sig->pinned && type_sig->num_mods == 0;
}

static RtResult<bool> is_corlib_primitive_or_enum_type(const metadata::RtTypeSig* type_sig,
                                                       const metadata::RtClass* klass) noexcept
{
    if (type_sig == nullptr || type_sig->by_ref)
    {
        RET_OK(false);
    }

    switch (type_sig->ele_type)
    {
    case metadata::RtElementType::Boolean:
    case metadata::RtElementType::Char:
    case metadata::RtElementType::I1:
    case metadata::RtElementType::U1:
    case metadata::RtElementType::I2:
    case metadata::RtElementType::U2:
    case metadata::RtElementType::I4:
    case metadata::RtElementType::U4:
    case metadata::RtElementType::I8:
    case metadata::RtElementType::U8:
    case metadata::RtElementType::R4:
    case metadata::RtElementType::R8:
    case metadata::RtElementType::I:
    case metadata::RtElementType::U:
        RET_OK(true);
    default:
        break;
    }

    RET_OK(klass != nullptr && Class::is_enum_type(klass));
}

static RtResult<uint16_t> get_net10_component_size(const metadata::RtTypeSig* type_sig) noexcept
{
    if (type_sig == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    if (type_sig->ele_type == metadata::RtElementType::String)
    {
        RET_OK(static_cast<uint16_t>(sizeof(char16_t)));
    }

    const metadata::RtTypeSig* element_type_sig = nullptr;
    if (type_sig->ele_type == metadata::RtElementType::SZArray)
    {
        element_type_sig = type_sig->data.element_type;
    }
    else if (type_sig->ele_type == metadata::RtElementType::Array && type_sig->data.array_type != nullptr)
    {
        element_type_sig = type_sig->data.array_type->ele_type;
    }

    if (element_type_sig == nullptr)
    {
        RET_OK(0);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, is_value_type, Type::is_value_type(element_type_sig));
    if (!is_value_type)
    {
        RET_OK(static_cast<uint16_t>(sizeof(void*)));
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(size_t, element_size, Type::get_size_of_type(element_type_sig));
    if (element_size > UINT16_MAX)
    {
        RET_ERR(RtErr::ExecutionEngine);
    }

    RET_OK(static_cast<uint16_t>(element_size));
}

static RtResult<uint32_t> get_net10_method_table_flags(const metadata::RtTypeSig* type_sig,
                                                       const metadata::RtClass* klass) noexcept
{
    constexpr uint32_t enum_flag_GenericsMask_GenericInst = 0x00000010;
    constexpr uint32_t enum_flag_GenericsMask_TypicalInst = 0x00000030;
    constexpr uint32_t enum_flag_HasDefaultCtor = 0x00000200;
    constexpr uint32_t enum_flag_IsByRefLike = 0x00001000;
    constexpr uint32_t enum_flag_ContainsGCPointers = 0x01000000;
    constexpr uint32_t enum_flag_ContainsGenericVariables = 0x20000000;
    constexpr uint32_t enum_flag_HasComponentSize = 0x80000000;
    constexpr uint32_t enum_flag_Category_ValueType = 0x00040000;
    constexpr uint32_t enum_flag_Category_Nullable = 0x00050000;
    constexpr uint32_t enum_flag_Category_PrimitiveValueType = 0x00060000;
    constexpr uint32_t enum_flag_Category_TruePrimitive = 0x00070000;
    constexpr uint32_t enum_flag_Category_Array = 0x00080000;
    constexpr uint32_t enum_flag_Category_Interface = 0x000C0000;

    if (type_sig == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    uint32_t flags = 0;
    const bool has_component_size =
        type_sig->ele_type == metadata::RtElementType::Array || type_sig->ele_type == metadata::RtElementType::SZArray ||
        type_sig->ele_type == metadata::RtElementType::String;
    if (has_component_size)
    {
        flags |= enum_flag_HasComponentSize;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint16_t, component_size, get_net10_component_size(type_sig));
        flags |= component_size;
    }

    if (type_sig->ele_type == metadata::RtElementType::Array || type_sig->ele_type == metadata::RtElementType::SZArray)
    {
        flags |= enum_flag_Category_Array;
    }
    else if (klass != nullptr && Class::is_interface(klass))
    {
        flags |= enum_flag_Category_Interface;
    }
    else if (klass != nullptr && Class::is_nullable_type(klass))
    {
        flags |= enum_flag_Category_Nullable;
    }
    else
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, primitive_or_enum, is_corlib_primitive_or_enum_type(type_sig, klass));
        if (primitive_or_enum)
        {
            flags |= klass != nullptr && Class::is_enum_type(klass) ? enum_flag_Category_PrimitiveValueType : enum_flag_Category_TruePrimitive;
        }
        else if (klass != nullptr && Class::is_value_type(klass))
        {
            flags |= enum_flag_Category_ValueType;
        }
    }

    if (klass != nullptr)
    {
        if (!has_component_size && Class::is_generic_inst(klass))
        {
            flags |= enum_flag_GenericsMask_GenericInst;
        }
        else if (!has_component_size && Class::is_generic(klass))
        {
            flags |= enum_flag_GenericsMask_TypicalInst;
        }

        if (Class::get_has_references(klass))
        {
            flags |= enum_flag_ContainsGCPointers;
        }

        auto by_ref_like_ret = Class::is_by_ref_like(klass);
        if (by_ref_like_ret.is_ok() && by_ref_like_ret.unwrap())
        {
            flags |= enum_flag_IsByRefLike;
        }

        if (!has_component_size && !Class::is_abstract(klass) && !Class::is_interface(klass))
        {
            flags |= enum_flag_HasDefaultCtor;
        }
    }

    if (Type::contains_generic_param(type_sig))
    {
        flags |= enum_flag_ContainsGenericVariables;
    }

    RET_OK(flags);
}

static RtResult<int32_t> unbox_i32(RtObject* obj, const metadata::RtClass* cls_i32)
{
    if (obj == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }
    auto klass = obj->klass;
    if (klass != cls_i32)
    {
        RET_ERR(RtErr::InvalidCast);
    }
    auto data_ptr = reinterpret_cast<const int32_t*>(reinterpret_cast<uint8_t*>(obj) + sizeof(metadata::RtClass*));
    RET_OK(*data_ptr);
}

static RtResult<RtArray*> invoke_new_array(const metadata::RtMethodInfo* method, RtArray* params)
{
    auto klass = method->parent;
    int32_t method_param_count = static_cast<int32_t>(method->parameter_count);
    auto corlib_types = Class::get_corlib_types();
    if (params == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }
    int32_t argument_count = Array::get_array_length(params);
    if (argument_count <= 0)
    {
        RET_ERR(RtErr::Argument);
    }
    if (argument_count == method_param_count)
    {
        if (argument_count == 1)
        {
            auto length_obj = Array::get_array_data_at<RtObject*>(params, 0);
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, length, unbox_i32(length_obj, corlib_types.cls_int32));
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, arr_obj, LEANCLR_NEW_SZARRAY_FROM_ARRAY_KLASS_INTERNAL(klass, length, "invoke_new_array"));
            RET_OK(arr_obj);
        }
        else
        {
            utils::Vector<int32_t> lengths(static_cast<size_t>(method_param_count));
            for (int32_t i = 0; i < method_param_count; ++i)
            {
                auto length_obj = Array::get_array_data_at<RtObject*>(params, i);
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, length, unbox_i32(length_obj, corlib_types.cls_int32));
                lengths[static_cast<size_t>(i)] = length;
            }
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, arr_obj,
                                                    LEANCLR_NEW_MDARRAY_FROM_ARRAY_KLASS_INTERNAL(klass, lengths.data(), nullptr, "invoke_new_array"));
            RET_OK(arr_obj);
        }
    }
    else if (argument_count == method_param_count * 2)
    {
        utils::Vector<int32_t> lengths(static_cast<size_t>(method_param_count));
        utils::Vector<int32_t> lower_bounds(static_cast<size_t>(method_param_count));
        for (int32_t i = 0; i < method_param_count; ++i)
        {
            auto length_obj = Array::get_array_data_at<RtObject*>(params, i);
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, length, unbox_i32(length_obj, corlib_types.cls_int32));
            lengths[static_cast<size_t>(i)] = length;
        }
        for (int32_t i = 0; i < method_param_count; ++i)
        {
            auto lower_bound_obj = Array::get_array_data_at<RtObject*>(params, i + method_param_count);
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, lower_bound, unbox_i32(lower_bound_obj, corlib_types.cls_int32));
            lower_bounds[static_cast<size_t>(i)] = lower_bound;
        }
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, arr_obj,
                                                LEANCLR_NEW_MDARRAY_FROM_ARRAY_KLASS_INTERNAL(klass, lengths.data(), lower_bounds.data(), "invoke_new_array"));
        RET_OK(arr_obj);
    }
    else
    {
        RET_ERR(RtErr::Argument);
    }
}

static bool has_legacy_reflection_field_layout(const metadata::RtClass* runtime_field_klass)
{
    return Class::get_instance_size_with_object_header(runtime_field_klass) == sizeof(RtReflectionField);
}

} // namespace

static RtResultVoid ensure_net10_method_table_facade_initialized(Net10MethodTableFacade* method_table,
                                                                 const metadata::RtTypeSig* pooled_type_sig) noexcept;

RtResult<const metadata::RtTypeSig*> Reflection::get_net10_type_handle(const metadata::RtTypeSig* type_sig)
{
    if (type_sig == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    auto canon_type_sig = type_sig->to_canonized();
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, pooled_type_sig,
                                            metadata::MetadataCache::get_pooled_typesig(canon_type_sig));

    auto found = s_net10_method_table_by_type_sig.find(pooled_type_sig);
    if (found != s_net10_method_table_by_type_sig.end())
    {
        RET_ERR_ON_FAIL(ensure_net10_method_table_facade_initialized(found->second, pooled_type_sig));
        RET_OK(reinterpret_cast<const metadata::RtTypeSig*>(found->second));
    }

    auto method_table = alloc::MetadataAllocation::malloc_any_zeroed<Net10MethodTableFacade>();
    auto auxiliary_data = alloc::MetadataAllocation::malloc_any_zeroed<Net10MethodTableAuxiliaryData>();
    method_table->type_sig = pooled_type_sig;
    method_table->auxiliary_data = auxiliary_data;
    s_net10_method_table_by_type_sig.emplace(pooled_type_sig, method_table);
    s_net10_method_table_by_handle.emplace(method_table, method_table);

    RET_ERR_ON_FAIL(ensure_net10_method_table_facade_initialized(method_table, pooled_type_sig));

    RET_OK(reinterpret_cast<const metadata::RtTypeSig*>(method_table));
}

RtResult<const void*> Reflection::get_net10_method_table(const metadata::RtTypeSig* type_sig)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, type_handle, get_net10_type_handle(type_sig));
    auto found = s_net10_method_table_by_handle.find(type_handle);
    if (found == s_net10_method_table_by_handle.end())
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    RET_OK(found->second);
}

RtResult<const metadata::RtTypeSig*> Reflection::get_type_sig_from_net10_method_table(const void* method_table)
{
    if (method_table == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    auto found = s_net10_method_table_by_handle.find(method_table);
    if (found == s_net10_method_table_by_handle.end())
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    RET_OK(found->second->type_sig);
}

static RtResultVoid ensure_net10_method_table_facade_initialized(Net10MethodTableFacade* method_table,
                                                                 const metadata::RtTypeSig* pooled_type_sig) noexcept
{
    if (method_table == nullptr || pooled_type_sig == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    if (method_table->auxiliary_data == nullptr)
    {
        method_table->auxiliary_data = alloc::MetadataAllocation::malloc_any_zeroed<Net10MethodTableAuxiliaryData>();
    }

    if (method_table->klass == nullptr)
    {
        auto klass_ret = Class::get_class_from_typesig(pooled_type_sig);
        if (klass_ret.is_ok())
        {
            method_table->klass = klass_ret.unwrap();
        }
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint32_t, flags,
                                            get_net10_method_table_flags(pooled_type_sig, method_table->klass));
    method_table->flags = flags;
    method_table->base_size = 0;
    if (pooled_type_sig->ele_type == metadata::RtElementType::SZArray)
    {
        method_table->base_size = static_cast<uint32_t>(3 * sizeof(void*));
    }
    else if (pooled_type_sig->ele_type == metadata::RtElementType::Array && pooled_type_sig->data.array_type != nullptr)
    {
        method_table->base_size = static_cast<uint32_t>(3 * sizeof(void*) + pooled_type_sig->data.array_type->rank * 2 * sizeof(int32_t));
    }

    if (method_table->klass != nullptr)
    {
        if (!method_table->klass->by_val->is_by_ref() &&
            (pooled_type_sig->ele_type == metadata::RtElementType::Array || pooled_type_sig->ele_type == metadata::RtElementType::SZArray))
        {
            const metadata::RtTypeSig* element_type_sig =
                pooled_type_sig->ele_type == metadata::RtElementType::Array ? pooled_type_sig->data.array_type->ele_type
                                                                            : pooled_type_sig->data.element_type;
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const void*, element_method_table,
                                                    Reflection::get_net10_method_table(element_type_sig));
            method_table->element_method_table = reinterpret_cast<const Net10MethodTableFacade*>(element_method_table);
        }

        RET_ERR_ON_FAIL(Class::initialize_super_types(const_cast<metadata::RtClass*>(method_table->klass)));
        if (method_table->klass->parent != nullptr && method_table->parent_method_table == nullptr)
        {
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const void*, parent_method_table,
                                                    Reflection::get_net10_method_table(Class::get_by_val_type_sig(method_table->klass->parent)));
            method_table->parent_method_table = reinterpret_cast<const Net10MethodTableFacade*>(parent_method_table);
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionType*, parent_runtime_type,
                                                    Reflection::get_klass_reflection_object(method_table->klass->parent));
            (void)parent_runtime_type;
        }
    }

    RET_VOID_OK();
}

RtResult<const metadata::RtTypeSig*> Reflection::get_type_sig_from_net10_type_handle(const void* type_handle)
{
    if (type_handle == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    auto found = s_net10_method_table_by_handle.find(type_handle);
    if (found != s_net10_method_table_by_handle.end())
    {
        RET_OK(found->second->type_sig);
    }

    auto type_sig = reinterpret_cast<const metadata::RtTypeSig*>(type_handle);
    if (looks_like_leanclr_type_sig(type_sig))
    {
        RET_OK(type_sig);
    }

    auto klass = reinterpret_cast<const metadata::RtClass*>(type_handle);
    if (klass != nullptr && klass->by_val != nullptr)
    {
        RET_OK(klass->by_val);
    }

    auto slot_value = *reinterpret_cast<void* const*>(type_handle);
    if (slot_value != nullptr && slot_value != type_handle)
    {
        auto slot_type_sig = get_type_sig_from_net10_method_table(slot_value);
        if (slot_type_sig.is_ok())
        {
            return slot_type_sig;
        }

        auto slot_as_type_sig = reinterpret_cast<const metadata::RtTypeSig*>(slot_value);
        if (looks_like_leanclr_type_sig(slot_as_type_sig))
        {
            RET_OK(slot_as_type_sig);
        }

        auto slot_as_klass = reinterpret_cast<const metadata::RtClass*>(slot_value);
        if (slot_as_klass != nullptr && slot_as_klass->by_val != nullptr)
        {
            RET_OK(slot_as_klass->by_val);
        }
    }

    if (std::getenv("LEANCLR_REFLECTION_TRACE") != nullptr)
    {
        std::fprintf(stderr, "leanclr-reflection: bad net10 type handle handle=%p slot=%p\n", type_handle, slot_value);
    }
    RET_ERR(RtErr::BadImageFormat);
}

RtResult<const metadata::RtClass*> Reflection::get_class_from_net10_method_table(const void* method_table)
{
    if (method_table == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    auto found = s_net10_method_table_by_handle.find(method_table);
    if (found != s_net10_method_table_by_handle.end())
    {
        if (found->second->klass == nullptr)
        {
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass,
                                                    Class::get_class_from_typesig(found->second->type_sig));
            found->second->klass = klass;
        }
        RET_OK(found->second->klass);
    }

    auto type_sig = reinterpret_cast<const metadata::RtTypeSig*>(method_table);
    if (looks_like_leanclr_type_sig(type_sig))
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, Class::get_class_from_typesig(type_sig));
        RET_OK(klass);
    }

    RET_OK(reinterpret_cast<const metadata::RtClass*>(method_table));
}

RtResult<RtReflectionRuntimeType*> Reflection::get_runtime_type_from_handle_arg(const void* type_handle)
{
    if (type_handle == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    if (auto runtime_type = try_get_runtime_type_object(type_handle))
    {
        RET_OK(runtime_type);
    }

    auto net10_type_sig = get_type_sig_from_net10_method_table(type_handle);
    if (net10_type_sig.is_ok())
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionType*, ref_type,
                                                get_type_reflection_object(net10_type_sig.unwrap()));
        RET_OK(reinterpret_cast<RtReflectionRuntimeType*>(ref_type));
    }

    auto slot_value = *reinterpret_cast<void* const*>(type_handle);
    if (auto runtime_type = try_get_runtime_type_object(slot_value))
    {
        RET_OK(runtime_type);
    }

    auto slot_net10_type_sig = get_type_sig_from_net10_method_table(slot_value);
    if (slot_net10_type_sig.is_ok())
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionType*, ref_type,
                                                get_type_reflection_object(slot_net10_type_sig.unwrap()));
        RET_OK(reinterpret_cast<RtReflectionRuntimeType*>(ref_type));
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtClass*, klass,
                                            get_class_from_net10_method_table(type_handle));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionType*, ref_type, get_klass_reflection_object(klass));
    RET_OK(reinterpret_cast<RtReflectionRuntimeType*>(ref_type));
}

RtResult<const metadata::RtTypeSig*> Reflection::get_type_sig_from_runtime_type_handle_arg(const void* type_handle)
{
    if (type_handle == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    auto net10_type_sig = get_type_sig_from_net10_method_table(type_handle);
    if (net10_type_sig.is_ok())
    {
        return net10_type_sig;
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionRuntimeType*, runtime_type,
                                            get_runtime_type_from_handle_arg(type_handle));
    return get_type_sig_from_runtime_type_object(runtime_type);
}

RtResult<const metadata::RtTypeSig*> Reflection::get_type_sig_from_qcall_type_handle(void* qcall_type_handle, void* native_handle)
{
    if (native_handle != nullptr)
    {
        return get_type_sig_from_net10_type_handle(native_handle);
    }

    if (qcall_type_handle == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    if (auto runtime_type = try_get_runtime_type_object(qcall_type_handle))
    {
        return get_type_sig_from_runtime_type_object(runtime_type);
    }

    auto runtime_type = *reinterpret_cast<RtReflectionRuntimeType**>(qcall_type_handle);
    if (auto slot_runtime_type = try_get_runtime_type_object(runtime_type))
    {
        return get_type_sig_from_runtime_type_object(slot_runtime_type);
    }

    RET_ERR(RtErr::BadImageFormat);
}

RtResult<const metadata::RtTypeSig*> Reflection::get_type_sig_from_reflection_type_object(const RtReflectionType* type_obj)
{
    if (type_obj == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    auto raw_obj = reinterpret_cast<RtObject*>(const_cast<RtReflectionType*>(type_obj));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, normalized_obj, normalize_coreclr_reflection_object(raw_obj));
    auto normalized_type = reinterpret_cast<const RtReflectionType*>(normalized_obj);
    if (std::getenv("LEANCLR_REFLECTION_TRACE") != nullptr)
    {
        auto klass_ret = Class::get_class_from_typesig(reinterpret_cast<const metadata::RtTypeSig*>(normalized_type->type_handle));
        const char* ns = "<class-error>";
        const char* name = "";
        if (klass_ret.is_ok())
        {
            metadata::RtClass* klass = klass_ret.unwrap();
            ns = klass->namespaze != nullptr ? klass->namespaze : "";
            name = klass->name != nullptr ? klass->name : "";
        }
        std::fprintf(stderr, "leanclr-reflection: reflection type obj=%p normalized=%p type_handle=%p klass=%p\n",
                     type_obj, normalized_type, normalized_type->type_handle, normalized_obj->klass);
        std::fprintf(stderr, "leanclr-reflection-name: %s.%s\n", ns, name);
    }
    return get_type_sig_from_net10_type_handle(normalized_type->type_handle);
}

RtResult<metadata::RtClass*> Reflection::get_class_from_reflection_type_object(const RtReflectionType* type_obj)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, type_sig, get_type_sig_from_reflection_type_object(type_obj));
    return Class::get_class_from_typesig(type_sig);
}

RtResult<const metadata::RtTypeSig*> Reflection::get_type_sig_from_runtime_type_object(const RtReflectionRuntimeType* runtime_type)
{
    if (runtime_type == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    return get_type_sig_from_reflection_type_object(&runtime_type->reflection_type);
}

RtResult<metadata::RtClass*> Reflection::get_class_from_runtime_type_object(const RtReflectionRuntimeType* runtime_type)
{
    if (runtime_type == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    return get_class_from_reflection_type_object(&runtime_type->reflection_type);
}

RtResult<RtReflectionType*> Reflection::get_type_reflection_object(const metadata::RtTypeSig* type_sig)
{
    auto canon_type_sig = type_sig->to_canonized();

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, pooled_type_sig, metadata::MetadataCache::get_pooled_typesig(canon_type_sig));

    const metadata::RtClass* identity_klass = nullptr;
    if (!pooled_type_sig->is_by_ref())
    {
        auto klass_ret = Class::get_class_from_typesig(pooled_type_sig);
        if (klass_ret.is_ok())
        {
            identity_klass = klass_ret.unwrap();
            auto klass_found = s_klass_reflection_type_map.find(identity_klass);
            if (klass_found != s_klass_reflection_type_map.end())
            {
                RET_OK(klass_found->second);
            }
        }
    }

    auto it2 = s_class_reflection_type_map.find(pooled_type_sig);
    if (it2 != s_class_reflection_type_map.end())
    {
        if (identity_klass != nullptr)
        {
            s_klass_reflection_type_map.emplace(identity_klass, it2->second);
        }
        RET_OK(it2->second);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, net10_type_handle, get_net10_type_handle(pooled_type_sig));
    auto runtime_type_klass = Class::get_corlib_types().cls_runtimetype;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, ref_obj_raw, LEANCLR_NEWOBJ_INTERNAL(runtime_type_klass, "Reflection::get_type_reflection_object"));
    auto ref_obj = reinterpret_cast<RtReflectionType*>(ref_obj_raw);

    ref_obj->type_handle = net10_type_handle;
    ref_obj->cache = nullptr;
    auto inserted = s_class_reflection_type_map.emplace(pooled_type_sig, ref_obj);
    if (!inserted.second)
    {
        ref_obj = inserted.first->second;
    }
    auto method_table = reinterpret_cast<Net10MethodTableFacade*>(const_cast<metadata::RtTypeSig*>(net10_type_handle));
    method_table->auxiliary_data->exposed_class_object_raw = reinterpret_cast<intptr_t>(ref_obj);
    if (identity_klass != nullptr)
    {
        s_klass_reflection_type_map.emplace(identity_klass, inserted.first->second);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, runtime_type_cache,
                                            get_or_create_runtime_type_cache(reinterpret_cast<RtReflectionRuntimeType*>(inserted.first->second)));
    (void)runtime_type_cache;

    RET_OK(inserted.first->second);
}

RtResult<RtReflectionRuntimeType*> Reflection::get_runtime_type_from_type_sig(const metadata::RtTypeSig* type_sig)
{
    if (type_sig == nullptr)
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionType*, type_obj, get_type_reflection_object(type_sig));
    RET_OK(reinterpret_cast<RtReflectionRuntimeType*>(type_obj));
}

RtResult<RtReflectionType*> Reflection::get_klass_reflection_object(const metadata::RtClass* klass)
{
    auto found = s_klass_reflection_type_map.find(klass);
    if (found != s_klass_reflection_type_map.end())
    {
        RET_OK(found->second);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionType*, ref_type, get_type_reflection_object(Class::get_by_val_type_sig(klass)));
    auto inserted = s_klass_reflection_type_map.emplace(klass, ref_type);
    RET_OK(inserted.first->second);
}

RtResult<RtObject*> Reflection::get_or_create_runtime_type_cache(RtReflectionRuntimeType* runtime_type)
{
    if (runtime_type == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }

    auto cache_slot = reinterpret_cast<RtObject**>(runtime_type->reflection_type.cache);
    if (cache_slot != nullptr && *cache_slot != nullptr)
    {
        RET_OK(*cache_slot);
    }

    metadata::RtModuleDef* corlib = metadata::RtModuleDef::get_corlib_module();
    if (corlib == nullptr)
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, cache_klass,
                                            corlib->get_class_by_nested_full_name("System.RuntimeType+RuntimeTypeCache", false, true));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, cache_obj,
                                            LEANCLR_NEWOBJ_INTERNAL(cache_klass, "Reflection::get_or_create_runtime_type_cache"));
    ScopedRtObjectRoot cache_root(&cache_obj);

    const metadata::RtFieldInfo* runtime_type_field = Class::get_field_for_name(cache_klass, "m_runtimeType", true);
    if (runtime_type_field == nullptr)
    {
        RET_ERR(RtErr::MissingField);
    }
    RET_ERR_ON_FAIL(Field::set_instance_value(runtime_type_field, cache_obj, &runtime_type));

    const metadata::RtFieldInfo* type_code_field = Class::get_field_for_name(cache_klass, "m_typeCode", true);
    if (type_code_field != nullptr)
    {
        int32_t type_code_empty = 0;
        RET_ERR_ON_FAIL(Field::set_instance_value(type_code_field, cache_obj, &type_code_empty));
    }

    const metadata::RtFieldInfo* is_global_field = Class::get_field_for_name(cache_klass, "m_isGlobal", true);
    if (is_global_field != nullptr)
    {
        bool is_global = false;
        RET_ERR_ON_FAIL(Field::set_instance_value(is_global_field, cache_obj, &is_global));
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, type_sig,
                                            get_type_sig_from_runtime_type_object(runtime_type));
    auto klass_ret = Class::get_class_from_typesig(type_sig);
    if (klass_ret.is_ok())
    {
        metadata::RtClass* klass = klass_ret.unwrap();

        const metadata::RtFieldInfo* name_field = Class::get_field_for_name(cache_klass, "m_name", true);
        if (name_field != nullptr)
        {
            RtString* name = String::create_string_from_utf8cstr(klass->name != nullptr ? klass->name : "");
            RET_ERR_ON_FAIL(Field::set_instance_value(name_field, cache_obj, &name));
        }

        const metadata::RtFieldInfo* namespace_field = Class::get_field_for_name(cache_klass, "m_namespace", true);
        if (namespace_field != nullptr)
        {
            RtString* namespaze = String::create_string_from_utf8cstr(klass->namespaze != nullptr ? klass->namespaze : "");
            RET_ERR_ON_FAIL(Field::set_instance_value(namespace_field, cache_obj, &namespaze));
        }
    }

    if (cache_slot == nullptr)
    {
        void* handle = GCHandle::get_target_handle(cache_obj, nullptr, 2);
        runtime_type->reflection_type.cache = GCHandle::get_target_slot(handle);
    }
    else
    {
        *cache_slot = cache_obj;
    }

    RET_OK(cache_obj);
}

bool Reflection::is_coreclr_reflection_object_class(const metadata::RtClass* klass)
{
    if (klass == nullptr)
    {
        return false;
    }

    const CorLibTypes& corlib_types = Class::get_corlib_types();
    if (klass == corlib_types.cls_runtimetype || klass == corlib_types.cls_reflection_method ||
        klass == corlib_types.cls_reflection_constructor || klass == corlib_types.cls_reflection_field ||
        klass == corlib_types.cls_reflection_property || klass == corlib_types.cls_reflection_event ||
        klass == corlib_types.cls_reflection_parameter || klass == corlib_types.cls_reflection_assembly ||
        klass == corlib_types.cls_reflection_module)
    {
        return true;
    }

    return is_runtime_field_info_stub_class(klass) || is_runtime_method_info_stub_class(klass);
}

RtResult<RtObject*> Reflection::normalize_coreclr_reflection_object(RtObject* obj)
{
    if (obj == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }

    if (gc::GarbageCollector::is_allocated_object(obj))
    {
        RET_OK(obj);
    }

    auto possible_header = reinterpret_cast<RtObject*>(reinterpret_cast<uint8_t*>(obj) - sizeof(RtObject));
    if (gc::GarbageCollector::is_allocated_object(possible_header) &&
        is_coreclr_reflection_object_class(possible_header->klass))
    {
        RET_OK(possible_header);
    }

    RET_ERR(RtErr::BadImageFormat);
}

RtResult<RtReflectionMethod*> Reflection::get_method_reflection_object(const metadata::RtMethodInfo* method, const metadata::RtClass* reflection_at_klass)
{
    MethodKey key{method, reflection_at_klass};
    auto found = s_method_reflection_map.find(key);
    if (found != s_method_reflection_map.end())
    {
        RET_OK(found->second);
    }

    auto corlib_types = Class::get_corlib_types();
    bool is_constructor = Method::is_ctor_or_cctor(method);
    auto runtime_method_klass = is_constructor ? corlib_types.cls_reflection_constructor : corlib_types.cls_reflection_method;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, ref_obj_raw, LEANCLR_NEWOBJ_INTERNAL(runtime_method_klass, "Reflection::get_method_reflection_object"));
    ScopedRtObjectRoot ref_obj_root(&ref_obj_raw);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionType*, ref_type, get_klass_reflection_object(reflection_at_klass));
    auto runtime_ref_type = reinterpret_cast<RtReflectionRuntimeType*>(ref_type);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, reflected_type_cache, get_or_create_runtime_type_cache(runtime_ref_type));
    auto ref_obj = reinterpret_cast<RtReflectionMethod*>(ref_obj_raw);
    if (is_constructor)
    {
        auto ctor_obj = reinterpret_cast<RtReflectionConstructor*>(ref_obj_raw);
        ctor_obj->declaring_type = runtime_ref_type;
        ctor_obj->reflected_type_cache = reflected_type_cache;
        ctor_obj->method = method;
        ctor_obj->method_attributes = method->flags;
        ctor_obj->binding_flags = 0;
    }
    else
    {
        ref_obj->method = method;
        ref_obj->declaring_type = runtime_ref_type;
        ref_obj->reflected_type_cache = reflected_type_cache;
        ref_obj->method_attributes = method->flags;
        ref_obj->binding_flags = 0;
    }
    s_method_reflection_map.emplace(key, ref_obj);
    s_method_object_data_map.emplace(ref_obj, MethodObjectData{method, reflection_at_klass});
    RET_OK(ref_obj);
}

RtResult<RtReflectionMethod*> Reflection::create_runtime_method_info_object(const metadata::RtMethodInfo* method,
                                                                            RtReflectionRuntimeType* declaring_type,
                                                                            RtObject* reflected_type_cache,
                                                                            int32_t method_attributes,
                                                                            int32_t binding_flags,
                                                                            RtObject* keepalive)
{
    if (method == nullptr || declaring_type == nullptr || reflected_type_cache == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, obj,
                                            LEANCLR_NEWOBJ_INTERNAL(Class::get_corlib_types().cls_reflection_method,
                                                                    "Reflection::create_runtime_method_info_object"));
    auto method_obj = reinterpret_cast<RtReflectionMethod*>(obj);
    method_obj->method = method;
    method_obj->reflected_type_cache = reflected_type_cache;
    method_obj->name = nullptr;
    method_obj->to_string = nullptr;
    method_obj->parameters = nullptr;
    method_obj->return_parameter = nullptr;
    method_obj->binding_flags = binding_flags;
    method_obj->method_attributes = method_attributes;
    method_obj->signature = nullptr;
    method_obj->declaring_type = declaring_type;
    method_obj->keepalive = keepalive;
    method_obj->invoker = nullptr;
    RET_OK(method_obj);
}

RtResult<RtObject*> Reflection::create_runtime_method_info_stub(const metadata::RtMethodInfo* method, RtObject* keepalive)
{
    if (method == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    metadata::RtModuleDef* corlib = metadata::RtModuleDef::get_corlib_module();
    if (corlib == nullptr)
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, stub_klass,
                                            corlib->get_class_by_name("System.RuntimeMethodInfoStub", false, true));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, stub, LEANCLR_NEWOBJ_INTERNAL(stub_klass, "Reflection::create_runtime_method_info_stub"));

    const metadata::RtFieldInfo* value_field = Class::get_field_for_name(stub_klass, "m_value", true);
    if (value_field == nullptr)
    {
        RET_ERR(RtErr::MissingField);
    }
    RET_ERR_ON_FAIL(Field::set_instance_value(value_field, stub, &method));

    const metadata::RtFieldInfo* keepalive_field = Class::get_field_for_name(stub_klass, "m_keepalive", true);
    if (keepalive_field != nullptr)
    {
        RET_ERR_ON_FAIL(Field::set_instance_value(keepalive_field, stub, &keepalive));
    }

    RET_OK(stub);
}

RtResult<RtReflectionConstructor*> Reflection::create_runtime_constructor_info_object(const metadata::RtMethodInfo* method,
                                                                                     RtReflectionRuntimeType* declaring_type,
                                                                                     RtObject* reflected_type_cache,
                                                                                     int32_t method_attributes,
                                                                                     int32_t binding_flags)
{
    if (method == nullptr || declaring_type == nullptr || reflected_type_cache == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, obj,
                                            LEANCLR_NEWOBJ_INTERNAL(Class::get_corlib_types().cls_reflection_constructor,
                                                                    "Reflection::create_runtime_constructor_info_object"));
    auto constructor_obj = reinterpret_cast<RtReflectionConstructor*>(obj);
    constructor_obj->declaring_type = declaring_type;
    constructor_obj->reflected_type_cache = reflected_type_cache;
    constructor_obj->to_string = nullptr;
    constructor_obj->parameters = nullptr;
    constructor_obj->empty1 = nullptr;
    constructor_obj->empty2 = nullptr;
    constructor_obj->empty3 = nullptr;
    constructor_obj->method = method;
    constructor_obj->method_attributes = method_attributes;
    constructor_obj->binding_flags = binding_flags;
    constructor_obj->signature = nullptr;
    constructor_obj->invoker = nullptr;
    RET_OK(constructor_obj);
}

RtResult<const metadata::RtMethodInfo*> Reflection::get_method_info_from_reflection_object(RtReflectionMethod* method_obj)
{
    if (method_obj == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, normalized_obj,
                                            normalize_coreclr_reflection_object(reinterpret_cast<RtObject*>(method_obj)));
    const metadata::RtClass* obj_klass = normalized_obj->klass;
    const CorLibTypes& corlib_types = Class::get_corlib_types();
    if (obj_klass != corlib_types.cls_reflection_method && obj_klass != corlib_types.cls_reflection_constructor)
    {
        RET_ERR(RtErr::Argument);
    }

    method_obj = reinterpret_cast<RtReflectionMethod*>(normalized_obj);
    auto found = s_method_object_data_map.find(method_obj);
    if (found != s_method_object_data_map.end())
    {
        RET_OK(found->second.method);
    }

    if (obj_klass == corlib_types.cls_reflection_constructor)
    {
        auto constructor_obj = reinterpret_cast<RtReflectionConstructor*>(normalized_obj);
        if (constructor_obj->method != nullptr)
        {
            RET_OK(constructor_obj->method);
        }
    }
    else if (method_obj->method != nullptr)
    {
        RET_OK(method_obj->method);
    }

    RET_ERR(RtErr::Argument);
}

RtResult<const metadata::RtMethodInfo*> Reflection::get_method_info_from_handle_arg(const void* method_arg)
{
    if (method_arg == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    auto raw_obj = reinterpret_cast<RtObject*>(const_cast<void*>(method_arg));
    auto normalized_obj_ret = normalize_coreclr_reflection_object(raw_obj);
    if (normalized_obj_ret.is_ok())
    {
        RtObject* obj = normalized_obj_ret.unwrap();
        const CorLibTypes& corlib_types = Class::get_corlib_types();
        if (obj->klass == corlib_types.cls_reflection_method || obj->klass == corlib_types.cls_reflection_constructor)
        {
            return get_method_info_from_reflection_object(reinterpret_cast<RtReflectionMethod*>(obj));
        }
        if (is_runtime_method_info_stub_class(obj->klass))
        {
            return get_method_info_from_runtime_method_info_stub(obj);
        }
    }

    auto direct_method = reinterpret_cast<const metadata::RtMethodInfo*>(method_arg);
    if (is_method_metadata_pointer(direct_method))
    {
        RET_OK(direct_method);
    }

    uintptr_t slot_value = *reinterpret_cast<const uintptr_t*>(method_arg);
    auto slot_method = reinterpret_cast<const metadata::RtMethodInfo*>(slot_value);
    if (is_method_metadata_pointer(slot_method))
    {
        RET_OK(slot_method);
    }

    auto slot_obj = reinterpret_cast<RtObject*>(slot_value);
    auto normalized_slot_obj_ret = normalize_coreclr_reflection_object(slot_obj);
    if (normalized_slot_obj_ret.is_ok())
    {
        RtObject* obj = normalized_slot_obj_ret.unwrap();
        const CorLibTypes& corlib_types = Class::get_corlib_types();
        if (obj->klass == corlib_types.cls_reflection_method || obj->klass == corlib_types.cls_reflection_constructor)
        {
            return get_method_info_from_reflection_object(reinterpret_cast<RtReflectionMethod*>(obj));
        }
        if (is_runtime_method_info_stub_class(obj->klass))
        {
            return get_method_info_from_runtime_method_info_stub(obj);
        }
    }

    RET_OK(direct_method);
}

RtResult<const metadata::RtClass*> Reflection::get_reflection_method_klass(RtReflectionMethod* method_obj)
{
    if (method_obj == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, normalized_obj,
                                            normalize_coreclr_reflection_object(reinterpret_cast<RtObject*>(method_obj)));
    const metadata::RtClass* obj_klass = normalized_obj->klass;
    const CorLibTypes& corlib_types = Class::get_corlib_types();
    if (obj_klass != corlib_types.cls_reflection_method && obj_klass != corlib_types.cls_reflection_constructor)
    {
        RET_ERR(RtErr::Argument);
    }

    method_obj = reinterpret_cast<RtReflectionMethod*>(normalized_obj);
    auto found = s_method_object_data_map.find(method_obj);
    if (found != s_method_object_data_map.end())
    {
        RET_OK(found->second.klass);
    }

    RtReflectionRuntimeType* declaring_type = nullptr;
    if (obj_klass == corlib_types.cls_reflection_constructor)
    {
        auto constructor_obj = reinterpret_cast<RtReflectionConstructor*>(normalized_obj);
        declaring_type = constructor_obj->declaring_type;
    }
    else
    {
        declaring_type = method_obj->declaring_type;
    }

    if (declaring_type != nullptr)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, declaring_type_sig,
                                                get_type_sig_from_runtime_type_object(declaring_type));
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass,
                                                Class::get_class_from_typesig(declaring_type_sig));
        RET_OK(klass);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method, get_method_info_from_reflection_object(method_obj));
    if (method->parent != nullptr)
    {
        RET_OK(method->parent);
    }

    RET_ERR(RtErr::Argument);
}

RtResult<RtArray*> Reflection::get_param_objects(const metadata::RtMethodInfo* method, const metadata::RtClass* reflection_at_klass)
{
    MethodKey key{method, reflection_at_klass};
    auto found = s_method_params_map.find(key);
    if (found != s_method_params_map.end())
    {
        RET_OK(found->second);
    }

    size_t param_count = method->parameter_count;
    auto param_info_klass = Class::get_corlib_types().cls_reflection_parameter;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(
        RtArray*, param_info_array_obj,
        LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERNAL(param_info_klass, static_cast<int32_t>(param_count), "Reflection::get_param_objects"));

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionMethod*, ref_member, get_method_reflection_object(method, reflection_at_klass));
    auto ass = method->parent->image;
    for (size_t i = 0; i < param_count; ++i)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, param_obj_base, LEANCLR_NEWOBJ_INTERNAL(param_info_klass, "Reflection::get_param_objects"));
        auto param_info_obj = reinterpret_cast<RtReflectionParameter*>(param_obj_base);

        const metadata::RtTypeSig* param_type_sig = method->parameters[i];
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionType*, parent_type, get_type_reflection_object(param_type_sig));
        param_info_obj->parent_type = parent_type;
        param_info_obj->member = reinterpret_cast<RtObject*>(ref_member);

        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(std::optional<uint32_t>, param_token_opt, Method::get_parameter_token(method, static_cast<int32_t>(i)));
        if (param_token_opt.has_value())
        {
            metadata::EncodedTokenId param_token = param_token_opt.value();
            auto opt_param = ass->get_cli_image().read_param(metadata::RtToken::decode_rid(param_token));
            param_info_obj->attrs = opt_param->flags;
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtString*, param_name, Method::get_parameter_name_by_token(ass, param_token));
            param_info_obj->name = param_name;
            if (Parameter::has_parameter_attr_optional(param_info_obj->attrs))
            {
                UNWRAP_OR_RET_ERR_ON_FAIL(param_info_obj->default_value, Parameter::get_parameter_default_value_object(ass, param_token, param_type_sig));
            }
        }
        param_info_obj->index = static_cast<int32_t>(i);
        // param_info_obj->attrs = static_cast<uint32_t>(param_type_sig->flags);
        Array::set_array_data_at<RtReflectionParameter*>(param_info_array_obj, static_cast<int32_t>(i), param_info_obj);
    }
    s_method_params_map.emplace(key, param_info_array_obj);
    RET_OK(param_info_array_obj);
}

RtResult<const metadata::RtMethodInfo*> Reflection::get_parameter_method_info_from_reflection_object(RtReflectionParameter* parameter_obj)
{
    if (parameter_obj == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, normalized_obj,
                                            normalize_coreclr_reflection_object(reinterpret_cast<RtObject*>(parameter_obj)));
    if (normalized_obj->klass != Class::get_corlib_types().cls_reflection_parameter)
    {
        RET_ERR(RtErr::Argument);
    }

    parameter_obj = reinterpret_cast<RtReflectionParameter*>(normalized_obj);
    if (parameter_obj->member == nullptr)
    {
        RET_ERR(RtErr::Argument);
    }

    return get_method_info_from_reflection_object(reinterpret_cast<RtReflectionMethod*>(parameter_obj->member));
}

RtResult<std::optional<uint32_t>> Reflection::get_parameter_token_from_reflection_object(RtReflectionParameter* parameter_obj)
{
    if (parameter_obj == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, normalized_obj,
                                            normalize_coreclr_reflection_object(reinterpret_cast<RtObject*>(parameter_obj)));
    if (normalized_obj->klass != Class::get_corlib_types().cls_reflection_parameter)
    {
        RET_ERR(RtErr::Argument);
    }

    parameter_obj = reinterpret_cast<RtReflectionParameter*>(normalized_obj);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method,
                                            get_parameter_method_info_from_reflection_object(parameter_obj));
    return Method::get_parameter_token(method, parameter_obj->index);
}

RtResult<RtReflectionField*> Reflection::get_field_reflection_object(const metadata::RtFieldInfo* field, const metadata::RtClass* reflection_at_klass)
{
    FieldKey key{field, reflection_at_klass};
    auto found = s_field_reflection_map.find(key);
    if (found != s_field_reflection_map.end())
    {
        RET_OK(found->second);
    }

    auto corlib_types = Class::get_corlib_types();
    auto runtime_field_klass = corlib_types.cls_reflection_field;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, ref_obj_raw, LEANCLR_NEWOBJ_INTERNAL(runtime_field_klass, "Reflection::get_field_reflection_object"));
    auto ref_obj = reinterpret_cast<RtReflectionField*>(ref_obj_raw);
    s_field_reflection_map.emplace(key, ref_obj);
    s_field_object_data_map.emplace(ref_obj, FieldObjectData{field, reflection_at_klass});

    if (has_legacy_reflection_field_layout(runtime_field_klass))
    {
        ref_obj->field = field;
        ref_obj->klass = reflection_at_klass;
        ref_obj->name = String::create_string_from_utf8chars(field->name, static_cast<int32_t>(std::strlen(field->name)));
        ref_obj->attrs = field->flags;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionType*, type_obj, get_type_reflection_object(field->type_sig));
        ref_obj->type_ = type_obj;
    }
    else
    {
        const metadata::RtFieldInfo* field_handle_field = Class::get_field_for_name(runtime_field_klass, "m_fieldHandle", true);
        if (field_handle_field == nullptr)
        {
            RET_ERR(RtErr::MissingField);
        }

        RET_ERR_ON_FAIL(Field::set_instance_value(field_handle_field, ref_obj, &field));

        const metadata::RtFieldInfo* attributes_field = Class::get_field_for_name(runtime_field_klass, "m_fieldAttributes", true);
        if (attributes_field != nullptr)
        {
            int32_t attrs = static_cast<int32_t>(field->flags);
            RET_ERR_ON_FAIL(Field::set_instance_value(attributes_field, ref_obj, &attrs));
        }

        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionType*, reflected_type, get_klass_reflection_object(reflection_at_klass));
        auto runtime_reflected_type = reinterpret_cast<RtReflectionRuntimeType*>(reflected_type);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, reflected_type_cache, get_or_create_runtime_type_cache(runtime_reflected_type));

        const metadata::RtFieldInfo* declaring_type_field = Class::get_field_for_name(runtime_field_klass, "m_declaringType", true);
        if (declaring_type_field != nullptr)
        {
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionType*, declaring_type, get_klass_reflection_object(field->parent));
            auto runtime_declaring_type = reinterpret_cast<RtReflectionRuntimeType*>(declaring_type);
            RET_ERR_ON_FAIL(Field::set_instance_value(declaring_type_field, ref_obj, &runtime_declaring_type));
        }

        const metadata::RtFieldInfo* reflected_type_cache_field = Class::get_field_for_name(runtime_field_klass, "m_reflectedTypeCache", true);
        if (reflected_type_cache_field != nullptr)
        {
            RET_ERR_ON_FAIL(Field::set_instance_value(reflected_type_cache_field, ref_obj, &reflected_type_cache));
        }
    }
    RET_OK(ref_obj);
}

RtResult<const metadata::RtFieldInfo*> Reflection::get_field_info_from_reflection_object(RtReflectionField* field_obj)
{
    if (field_obj == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, normalized_obj,
                                            normalize_coreclr_reflection_object(reinterpret_cast<RtObject*>(field_obj)));
    field_obj = reinterpret_cast<RtReflectionField*>(normalized_obj);
    auto found = s_field_object_data_map.find(field_obj);
    if (found != s_field_object_data_map.end())
    {
        RET_OK(found->second.field);
    }

    if (has_legacy_reflection_field_layout(normalized_obj->klass) && field_obj->field != nullptr)
    {
        RET_OK(field_obj->field);
    }

    const metadata::RtFieldInfo* handle_field = Class::get_field_for_name(normalized_obj->klass, "m_fieldHandle", true);
    if (handle_field != nullptr)
    {
        const metadata::RtFieldInfo* handle = nullptr;
        RET_ERR_ON_FAIL(Field::get_instance_value(handle_field, field_obj, &handle));
        if (handle != nullptr)
        {
            RET_OK(handle);
        }
    }

    RET_ERR(RtErr::Argument);
}

RtResult<const metadata::RtFieldInfo*> Reflection::get_field_info_from_handle_arg(const void* field_arg)
{
    if (field_arg == nullptr)
    {
        RET_OK(nullptr);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtFieldInfo*, reflection_field,
                                            get_field_info_from_reflection_field_arg(field_arg));
    if (reflection_field != nullptr)
    {
        RET_OK(reflection_field);
    }

    auto raw_obj = reinterpret_cast<RtObject*>(const_cast<void*>(field_arg));
    if (gc::GarbageCollector::is_allocated_object(raw_obj) && is_runtime_field_handle_internal_class(raw_obj->klass))
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtFieldInfo*, boxed_field,
                                                get_field_info_from_runtime_field_handle_internal(raw_obj));
        if (boxed_field != nullptr)
        {
            RET_OK(boxed_field);
        }
    }

    auto direct_field = reinterpret_cast<const metadata::RtFieldInfo*>(field_arg);
    if (is_field_metadata_pointer(direct_field))
    {
        RET_OK(direct_field);
    }

    uintptr_t slot_value = *reinterpret_cast<const uintptr_t*>(field_arg);
    auto slot_field = reinterpret_cast<const metadata::RtFieldInfo*>(slot_value);
    if (is_field_metadata_pointer(slot_field))
    {
        RET_OK(slot_field);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtFieldInfo*, slot_object_field,
                                            get_field_info_from_reflection_field_arg(reinterpret_cast<const void*>(slot_value)));
    if (slot_object_field != nullptr)
    {
        RET_OK(slot_object_field);
    }

    RET_ERR(RtErr::BadImageFormat);
}

RtResult<RtObject*> Reflection::create_runtime_field_info_object(const metadata::RtFieldInfo* field,
                                                                 RtReflectionRuntimeType* declaring_type,
                                                                 RtObject* reflected_type_cache,
                                                                 int32_t binding_flags)
{
    if (field == nullptr || declaring_type == nullptr || reflected_type_cache == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, obj,
                                            LEANCLR_NEWOBJ_INTERNAL(Class::get_corlib_types().cls_reflection_field,
                                                                    "Reflection::create_runtime_field_info_object"));

    int32_t field_attributes = static_cast<int32_t>(field->flags);
    RET_ERR_ON_FAIL(set_runtime_object_field_value(obj, "m_bindingFlags", &binding_flags));
    RET_ERR_ON_FAIL(set_runtime_object_field_value(obj, "m_reflectedTypeCache", &reflected_type_cache));
    RET_ERR_ON_FAIL(set_runtime_object_field_value(obj, "m_declaringType", &declaring_type));
    RET_ERR_ON_FAIL(set_runtime_object_field_value(obj, "m_fieldHandle", &field));
    RET_ERR_ON_FAIL(set_runtime_object_field_value(obj, "m_fieldAttributes", &field_attributes));

    RET_OK(obj);
}

RtResult<RtObject*> Reflection::create_runtime_field_info_stub(const metadata::RtFieldInfo* field)
{
    if (field == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    metadata::RtModuleDef* corlib = metadata::RtModuleDef::get_corlib_module();
    if (corlib == nullptr)
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, stub_klass,
                                            corlib->get_class_by_name("System.RuntimeFieldInfoStub", false, true));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, stub, LEANCLR_NEWOBJ_INTERNAL(stub_klass, "Reflection::create_runtime_field_info_stub"));

    const metadata::RtFieldInfo* field_handle_field = Class::get_field_for_name(stub_klass, "m_fieldHandle", true);
    if (field_handle_field == nullptr)
    {
        RET_ERR(RtErr::MissingField);
    }

    RET_ERR_ON_FAIL(Field::set_instance_value(field_handle_field, stub, &field));
    RET_OK(stub);
}

RtResult<const metadata::RtClass*> Reflection::get_reflection_field_klass(RtReflectionField* field_obj)
{
    if (field_obj == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, normalized_obj,
                                            normalize_coreclr_reflection_object(reinterpret_cast<RtObject*>(field_obj)));
    field_obj = reinterpret_cast<RtReflectionField*>(normalized_obj);
    auto found = s_field_object_data_map.find(field_obj);
    if (found != s_field_object_data_map.end())
    {
        RET_OK(found->second.klass);
    }

    if (has_legacy_reflection_field_layout(normalized_obj->klass) && field_obj->klass != nullptr)
    {
        RET_OK(field_obj->klass);
    }

    const metadata::RtFieldInfo* declaring_type_field = Class::get_field_for_name(normalized_obj->klass, "m_declaringType", true);
    if (declaring_type_field != nullptr)
    {
        RtReflectionRuntimeType* declaring_type = nullptr;
        RET_ERR_ON_FAIL(Field::get_instance_value(declaring_type_field, field_obj, &declaring_type));
        if (declaring_type != nullptr)
        {
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, declaring_type_sig,
                                                    get_type_sig_from_runtime_type_object(declaring_type));
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass,
                                                    Class::get_class_from_typesig(declaring_type_sig));
            RET_OK(klass);
        }
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtFieldInfo*, field, get_field_info_from_reflection_object(field_obj));
    if (field->parent != nullptr)
    {
        RET_OK(field->parent);
    }

    RET_ERR(RtErr::Argument);
}

RtResult<RtReflectionProperty*> Reflection::get_property_reflection_object(const metadata::RtPropertyInfo* prop, const metadata::RtClass* reflection_at_klass)
{
    PropertyKey key{prop, reflection_at_klass};
    auto found = s_property_reflection_map.find(key);
    if (found != s_property_reflection_map.end())
    {
        RET_OK(found->second);
    }

    auto corlib_types = Class::get_corlib_types();
    auto runtime_prop_klass = corlib_types.cls_reflection_property;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, ref_obj_raw, LEANCLR_NEWOBJ_INTERNAL(runtime_prop_klass, "Reflection::get_property_reflection_object"));
    auto ref_obj = reinterpret_cast<RtReflectionProperty*>(ref_obj_raw);
    ref_obj->property = prop;
    ref_obj->klass = reflection_at_klass;
    s_property_reflection_map.emplace(key, ref_obj);
    s_property_object_data_map.emplace(ref_obj, PropertyObjectData{prop, reflection_at_klass});
    RET_OK(ref_obj);
}

RtResult<const metadata::RtPropertyInfo*> Reflection::get_property_info_from_runtime_type(RtReflectionRuntimeType* declaring_type,
                                                                                         int32_t property_token)
{
    if (declaring_type == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    metadata::RtToken token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(property_token));
    if (token.table_type != metadata::TableType::Property || token.rid == 0)
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, declaring_type_sig,
                                            get_type_sig_from_runtime_type_object(declaring_type));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, Class::get_class_from_typesig(declaring_type_sig));
    RET_ERR_ON_FAIL(Class::initialize_properties(klass));
    for (uint16_t i = 0; i < klass->property_count; ++i)
    {
        const metadata::RtPropertyInfo* property = klass->properties + i;
        if (property->token == static_cast<metadata::EncodedTokenId>(property_token))
        {
            RET_OK(property);
        }
    }

    RET_ERR(RtErr::MissingField);
}

RtResult<RtObject*> Reflection::create_runtime_property_info_object(const metadata::RtPropertyInfo* property,
                                                                    RtReflectionRuntimeType* declaring_type,
                                                                    RtObject* reflected_type_cache,
                                                                    bool* is_private)
{
    if (property == nullptr || declaring_type == nullptr || reflected_type_cache == nullptr || is_private == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    RtReflectionMethod* getter = nullptr;
    if (property->get_method != nullptr)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(
            RtReflectionMethod*, getter_obj,
            create_runtime_method_info_object(property->get_method,
                                              declaring_type,
                                              reflected_type_cache,
                                              static_cast<int32_t>(property->get_method->flags),
                                              get_method_binding_flags(property->get_method),
                                              nullptr));
        getter = getter_obj;
    }

    RtReflectionMethod* setter = nullptr;
    if (property->set_method != nullptr)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(
            RtReflectionMethod*, setter_obj,
            create_runtime_method_info_object(property->set_method,
                                              declaring_type,
                                              reflected_type_cache,
                                              static_cast<int32_t>(property->set_method->flags),
                                              get_method_binding_flags(property->set_method),
                                              nullptr));
        setter = setter_obj;
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, obj,
                                            LEANCLR_NEWOBJ_INTERNAL(Class::get_corlib_types().cls_reflection_property,
                                                                    "Reflection::create_runtime_property_info_object"));

    int32_t token = static_cast<int32_t>(property->token);
    RtString* name = nullptr;
    void* utf8_name = const_cast<char*>(property->name);
    int32_t flags = static_cast<int32_t>(property->flags);
    RtArray* other_methods = nullptr;
    int32_t binding_flags = get_property_binding_flags(property, is_private);
    RtObject* signature = nullptr;
    RtArray* parameters = nullptr;

    RET_ERR_ON_FAIL(set_runtime_object_field_value(obj, "m_token", &token));
    RET_ERR_ON_FAIL(set_runtime_object_field_value(obj, "m_name", &name));
    RET_ERR_ON_FAIL(set_runtime_object_field_value(obj, "m_utf8name", &utf8_name));
    RET_ERR_ON_FAIL(set_runtime_object_field_value(obj, "m_flags", &flags));
    RET_ERR_ON_FAIL(set_runtime_object_field_value(obj, "m_reflectedTypeCache", &reflected_type_cache));
    RET_ERR_ON_FAIL(set_runtime_object_field_value(obj, "m_getterMethod", &getter));
    RET_ERR_ON_FAIL(set_runtime_object_field_value(obj, "m_setterMethod", &setter));
    RET_ERR_ON_FAIL(set_runtime_object_field_value(obj, "m_otherMethod", &other_methods));
    RET_ERR_ON_FAIL(set_runtime_object_field_value(obj, "m_declaringType", &declaring_type));
    RET_ERR_ON_FAIL(set_runtime_object_field_value(obj, "m_bindingFlags", &binding_flags));
    RET_ERR_ON_FAIL(set_runtime_object_field_value(obj, "m_signature", &signature));
    RET_ERR_ON_FAIL(set_runtime_object_field_value(obj, "m_parameters", &parameters));

    RET_OK(obj);
}

RtResult<const metadata::RtPropertyInfo*> Reflection::get_property_info_from_reflection_object(RtReflectionProperty* property_obj)
{
    if (property_obj == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, normalized_obj,
                                            normalize_coreclr_reflection_object(reinterpret_cast<RtObject*>(property_obj)));
    if (normalized_obj->klass != Class::get_corlib_types().cls_reflection_property)
    {
        RET_ERR(RtErr::Argument);
    }

    property_obj = reinterpret_cast<RtReflectionProperty*>(normalized_obj);
    auto found = s_property_object_data_map.find(property_obj);
    if (found != s_property_object_data_map.end())
    {
        RET_OK(found->second.property);
    }

    if (property_obj->property != nullptr)
    {
        RET_OK(property_obj->property);
    }

    const metadata::RtFieldInfo* token_field = Class::get_field_for_name(normalized_obj->klass, "m_token", true);
    const metadata::RtFieldInfo* declaring_type_field = Class::get_field_for_name(normalized_obj->klass, "m_declaringType", true);
    if (token_field != nullptr && declaring_type_field != nullptr)
    {
        int32_t property_token = 0;
        RtReflectionRuntimeType* declaring_type = nullptr;
        RET_ERR_ON_FAIL(Field::get_instance_value(token_field, property_obj, &property_token));
        RET_ERR_ON_FAIL(Field::get_instance_value(declaring_type_field, property_obj, &declaring_type));
        return get_property_info_from_runtime_type(declaring_type, property_token);
    }

    RET_ERR(RtErr::Argument);
}

RtResult<const metadata::RtPropertyInfo*> Reflection::get_property_info_from_handle_arg(const void* property_arg)
{
    if (property_arg == nullptr)
    {
        RET_OK(nullptr);
    }

    auto raw_obj = reinterpret_cast<RtObject*>(const_cast<void*>(property_arg));
    auto normalized_obj_ret = normalize_coreclr_reflection_object(raw_obj);
    if (normalized_obj_ret.is_ok())
    {
        RtObject* obj = normalized_obj_ret.unwrap();
        if (obj->klass == Class::get_corlib_types().cls_reflection_property)
        {
            return get_property_info_from_reflection_object(reinterpret_cast<RtReflectionProperty*>(obj));
        }
    }

    auto direct_property = reinterpret_cast<const metadata::RtPropertyInfo*>(property_arg);
    if (is_property_metadata_pointer(direct_property))
    {
        RET_OK(direct_property);
    }

    uintptr_t slot_value = *reinterpret_cast<const uintptr_t*>(property_arg);
    auto slot_property = reinterpret_cast<const metadata::RtPropertyInfo*>(slot_value);
    if (is_property_metadata_pointer(slot_property))
    {
        RET_OK(slot_property);
    }

    auto slot_obj = reinterpret_cast<RtObject*>(slot_value);
    auto normalized_slot_obj_ret = normalize_coreclr_reflection_object(slot_obj);
    if (normalized_slot_obj_ret.is_ok())
    {
        RtObject* obj = normalized_slot_obj_ret.unwrap();
        if (obj->klass == Class::get_corlib_types().cls_reflection_property)
        {
            return get_property_info_from_reflection_object(reinterpret_cast<RtReflectionProperty*>(obj));
        }
    }

    RET_ERR(RtErr::BadImageFormat);
}

RtResult<const metadata::RtClass*> Reflection::get_reflection_property_klass(RtReflectionProperty* property_obj)
{
    if (property_obj == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, normalized_obj,
                                            normalize_coreclr_reflection_object(reinterpret_cast<RtObject*>(property_obj)));
    if (normalized_obj->klass != Class::get_corlib_types().cls_reflection_property)
    {
        RET_ERR(RtErr::Argument);
    }

    property_obj = reinterpret_cast<RtReflectionProperty*>(normalized_obj);
    auto found = s_property_object_data_map.find(property_obj);
    if (found != s_property_object_data_map.end())
    {
        RET_OK(found->second.klass);
    }

    if (property_obj->klass != nullptr)
    {
        RET_OK(property_obj->klass);
    }

    const metadata::RtFieldInfo* declaring_type_field = Class::get_field_for_name(normalized_obj->klass, "m_declaringType", true);
    if (declaring_type_field != nullptr)
    {
        RtReflectionRuntimeType* declaring_type = nullptr;
        RET_ERR_ON_FAIL(Field::get_instance_value(declaring_type_field, property_obj, &declaring_type));
        if (declaring_type != nullptr)
        {
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, declaring_type_sig,
                                                    get_type_sig_from_runtime_type_object(declaring_type));
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, Class::get_class_from_typesig(declaring_type_sig));
            RET_OK(klass);
        }
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtPropertyInfo*, property,
                                            get_property_info_from_reflection_object(property_obj));
    if (property->parent != nullptr)
    {
        RET_OK(property->parent);
    }

    RET_ERR(RtErr::Argument);
}

RtResult<RtReflectionEventInfo*> Reflection::get_event_reflection_object(metadata::RtEventInfo* event_info, const metadata::RtClass* reflection_at_klass)
{
    EventKey key{event_info, reflection_at_klass};
    auto found = s_event_reflection_map.find(key);
    if (found != s_event_reflection_map.end())
    {
        RET_OK(found->second);
    }

    auto corlib_types = Class::get_corlib_types();
    auto runtime_event_klass = corlib_types.cls_reflection_event;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, ref_obj_raw, LEANCLR_NEWOBJ_INTERNAL(runtime_event_klass, "Reflection::get_event_reflection_object"));
    auto ref_obj = reinterpret_cast<RtReflectionEventInfo*>(ref_obj_raw);
    ref_obj->event = event_info;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionType*, ref_type, get_klass_reflection_object(reflection_at_klass));
    ref_obj->ref_type = ref_type;
    s_event_reflection_map.emplace(key, ref_obj);
    s_event_object_data_map.emplace(ref_obj, EventObjectData{event_info, reflection_at_klass});
    RET_OK(ref_obj);
}

RtResult<metadata::RtEventInfo*> Reflection::get_event_info_from_runtime_type(RtReflectionRuntimeType* declaring_type,
                                                                              int32_t event_token)
{
    if (declaring_type == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    metadata::RtToken token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(event_token));
    if (token.table_type != metadata::TableType::Event || token.rid == 0)
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, declaring_type_sig,
                                            get_type_sig_from_runtime_type_object(declaring_type));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, Class::get_class_from_typesig(declaring_type_sig));
    RET_ERR_ON_FAIL(Class::initialize_events(klass));
    for (uint16_t i = 0; i < klass->event_count; ++i)
    {
        metadata::RtEventInfo* event_info = const_cast<metadata::RtEventInfo*>(klass->events + i);
        if (event_info->token == static_cast<metadata::EncodedTokenId>(event_token))
        {
            RET_OK(event_info);
        }
    }

    RET_ERR(RtErr::MissingField);
}

RtResult<metadata::RtEventInfo*> Reflection::get_event_info_from_reflection_object(RtReflectionEventInfo* event_obj)
{
    if (event_obj == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, normalized_obj,
                                            normalize_coreclr_reflection_object(reinterpret_cast<RtObject*>(event_obj)));
    if (normalized_obj->klass != Class::get_corlib_types().cls_reflection_event)
    {
        RET_ERR(RtErr::Argument);
    }

    event_obj = reinterpret_cast<RtReflectionEventInfo*>(normalized_obj);
    auto found = s_event_object_data_map.find(event_obj);
    if (found != s_event_object_data_map.end())
    {
        RET_OK(found->second.event_info);
    }

    const metadata::RtFieldInfo* token_field = Class::get_field_for_name(normalized_obj->klass, "m_token", true);
    const metadata::RtFieldInfo* declaring_type_field = Class::get_field_for_name(normalized_obj->klass, "m_declaringType", true);
    if (token_field != nullptr && declaring_type_field != nullptr)
    {
        int32_t event_token = 0;
        RtReflectionRuntimeType* declaring_type = nullptr;
        RET_ERR_ON_FAIL(Field::get_instance_value(token_field, event_obj, &event_token));
        RET_ERR_ON_FAIL(Field::get_instance_value(declaring_type_field, event_obj, &declaring_type));
        return get_event_info_from_runtime_type(declaring_type, event_token);
    }

    if (event_obj->event != nullptr)
    {
        RET_OK(event_obj->event);
    }

    RET_ERR(RtErr::Argument);
}

RtResult<metadata::RtEventInfo*> Reflection::get_event_info_from_handle_arg(const void* event_arg)
{
    if (event_arg == nullptr)
    {
        RET_OK(nullptr);
    }

    auto raw_obj = reinterpret_cast<RtObject*>(const_cast<void*>(event_arg));
    auto normalized_obj_ret = normalize_coreclr_reflection_object(raw_obj);
    if (normalized_obj_ret.is_ok())
    {
        RtObject* obj = normalized_obj_ret.unwrap();
        if (obj->klass == Class::get_corlib_types().cls_reflection_event)
        {
            return get_event_info_from_reflection_object(reinterpret_cast<RtReflectionEventInfo*>(obj));
        }
    }

    auto direct_event = reinterpret_cast<metadata::RtEventInfo*>(const_cast<void*>(event_arg));
    if (is_event_metadata_pointer(direct_event))
    {
        RET_OK(direct_event);
    }

    uintptr_t slot_value = *reinterpret_cast<const uintptr_t*>(event_arg);
    auto slot_event = reinterpret_cast<metadata::RtEventInfo*>(slot_value);
    if (is_event_metadata_pointer(slot_event))
    {
        RET_OK(slot_event);
    }

    auto slot_obj = reinterpret_cast<RtObject*>(slot_value);
    auto normalized_slot_obj_ret = normalize_coreclr_reflection_object(slot_obj);
    if (normalized_slot_obj_ret.is_ok())
    {
        RtObject* obj = normalized_slot_obj_ret.unwrap();
        if (obj->klass == Class::get_corlib_types().cls_reflection_event)
        {
            return get_event_info_from_reflection_object(reinterpret_cast<RtReflectionEventInfo*>(obj));
        }
    }

    RET_ERR(RtErr::BadImageFormat);
}

RtResult<const metadata::RtClass*> Reflection::get_reflection_event_klass(RtReflectionEventInfo* event_obj)
{
    if (event_obj == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, normalized_obj,
                                            normalize_coreclr_reflection_object(reinterpret_cast<RtObject*>(event_obj)));
    if (normalized_obj->klass != Class::get_corlib_types().cls_reflection_event)
    {
        RET_ERR(RtErr::Argument);
    }

    event_obj = reinterpret_cast<RtReflectionEventInfo*>(normalized_obj);
    auto found = s_event_object_data_map.find(event_obj);
    if (found != s_event_object_data_map.end())
    {
        RET_OK(found->second.klass);
    }

    const metadata::RtFieldInfo* declaring_type_field = Class::get_field_for_name(normalized_obj->klass, "m_declaringType", true);
    if (declaring_type_field != nullptr)
    {
        RtReflectionRuntimeType* declaring_type = nullptr;
        RET_ERR_ON_FAIL(Field::get_instance_value(declaring_type_field, event_obj, &declaring_type));
        if (declaring_type != nullptr)
        {
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, declaring_type_sig,
                                                    get_type_sig_from_runtime_type_object(declaring_type));
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, Class::get_class_from_typesig(declaring_type_sig));
            RET_OK(klass);
        }
    }

    if (event_obj->ref_type != nullptr)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, reflected_type_sig,
                                                get_type_sig_from_reflection_type_object(event_obj->ref_type));
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, reflected_klass,
                                                Class::get_class_from_typesig(reflected_type_sig));
        RET_OK(reflected_klass);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtEventInfo*, event_info, get_event_info_from_reflection_object(event_obj));
    if (event_info->parent != nullptr)
    {
        RET_OK(event_info->parent);
    }

    RET_ERR(RtErr::Argument);
}

RtResult<RtReflectionAssembly*> Reflection::get_assembly_reflection_object(metadata::RtAssembly* assembly)
{
    auto found = s_assembly_reflection_map.find(assembly);
    if (found != s_assembly_reflection_map.end())
    {
        RET_OK(found->second);
    }

    auto corlib_types = Class::get_corlib_types();
    auto runtime_assembly_klass = corlib_types.cls_reflection_assembly;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, ref_obj_raw,
                                            LEANCLR_NEWOBJ_INTERNAL(runtime_assembly_klass, "Reflection::get_assembly_reflection_object"));
    auto ref_obj = reinterpret_cast<RtReflectionAssembly*>(ref_obj_raw);
    ref_obj->assembly = assembly;
    s_assembly_reflection_map.emplace(assembly, ref_obj);
    RET_OK(ref_obj);
}

RtResult<metadata::RtAssembly*> Reflection::get_assembly_from_qcall_assembly(void* qcall_assembly, void* native_handle)
{
    if (native_handle != nullptr)
    {
        RET_OK(reinterpret_cast<metadata::RtAssembly*>(native_handle));
    }

    if (qcall_assembly == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    const metadata::RtClass* runtime_assembly_klass = Class::get_corlib_types().cls_reflection_assembly;
    auto direct_obj = reinterpret_cast<RtObject*>(qcall_assembly);
    if (gc::GarbageCollector::is_allocated_object(direct_obj) && direct_obj->klass == runtime_assembly_klass)
    {
        return get_assembly_from_reflection_object(reinterpret_cast<RtReflectionAssembly*>(direct_obj));
    }

    auto runtime_assembly = *reinterpret_cast<RtReflectionAssembly**>(qcall_assembly);
    auto runtime_assembly_obj = reinterpret_cast<RtObject*>(runtime_assembly);
    if (runtime_assembly == nullptr ||
        !gc::GarbageCollector::is_allocated_object(runtime_assembly_obj) ||
        runtime_assembly_obj->klass != runtime_assembly_klass)
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    return get_assembly_from_reflection_object(runtime_assembly);
}

RtResult<metadata::RtAssembly*> Reflection::get_assembly_from_handle_arg(const void* assembly_arg)
{
    if (assembly_arg == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    const metadata::RtClass* runtime_assembly_klass = Class::get_corlib_types().cls_reflection_assembly;
    auto direct_obj = reinterpret_cast<RtObject*>(const_cast<void*>(assembly_arg));
    if (gc::GarbageCollector::is_allocated_object(direct_obj) && direct_obj->klass == runtime_assembly_klass)
    {
        return get_assembly_from_reflection_object(reinterpret_cast<RtReflectionAssembly*>(direct_obj));
    }

    auto slot_assembly = *reinterpret_cast<RtReflectionAssembly* const*>(assembly_arg);
    auto slot_obj = reinterpret_cast<RtObject*>(slot_assembly);
    if (slot_assembly != nullptr &&
        gc::GarbageCollector::is_allocated_object(slot_obj) &&
        slot_obj->klass == runtime_assembly_klass)
    {
        return get_assembly_from_reflection_object(slot_assembly);
    }

    RET_OK(reinterpret_cast<metadata::RtAssembly*>(const_cast<void*>(assembly_arg)));
}

RtResult<metadata::RtAssembly*> Reflection::get_assembly_from_reflection_object(RtReflectionAssembly* assembly_obj)
{
    if (assembly_obj == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, normalized_obj,
                                            normalize_coreclr_reflection_object(reinterpret_cast<RtObject*>(assembly_obj)));
    assembly_obj = reinterpret_cast<RtReflectionAssembly*>(normalized_obj);
    if (assembly_obj->assembly != nullptr)
    {
        RET_OK(assembly_obj->assembly);
    }

    const metadata::RtFieldInfo* assembly_field = Class::get_field_for_name(normalized_obj->klass, "m_assembly", true);
    if (assembly_field == nullptr)
    {
        assembly_field = Class::get_field_for_name(normalized_obj->klass, "assembly", true);
    }
    if (assembly_field != nullptr)
    {
        metadata::RtAssembly* assembly = nullptr;
        RET_ERR_ON_FAIL(Field::get_instance_value(assembly_field, assembly_obj, &assembly));
        if (assembly != nullptr)
        {
            RET_OK(assembly);
        }
    }

    RET_ERR(RtErr::Argument);
}

RtResult<metadata::RtMonoAssemblyName*> Reflection::get_assembly_name_object(metadata::RtAssembly* ass)
{
    auto found = s_assembly_name_map.find(ass);
    if (found != s_assembly_name_map.end())
    {
        RET_OK(found->second);
    }

    auto name_obj = alloc::GeneralAllocation::malloc_any_zeroed<metadata::RtMonoAssemblyName>();
    ass->mod->fill_assembly_name(*name_obj);
    s_assembly_name_map.emplace(ass, name_obj);
    RET_OK(name_obj);
}

RtResult<RtObject*> Reflection::create_runtime_assembly_name_object(const metadata::RtAssemblyName& assembly_name)
{
    metadata::RtModuleDef* corlib = metadata::RtModuleDef::get_corlib_module();
    if (corlib == nullptr)
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, assembly_name_klass,
                                            corlib->get_class_by_name("System.Reflection.AssemblyName", false, true));
    RET_ERR_ON_FAIL(Class::initialize_fields(assembly_name_klass));

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, assembly_name_obj,
                                            LEANCLR_NEWOBJ_INTERNAL(assembly_name_klass,
                                                                    "Reflection::create_runtime_assembly_name_object"));

    RtString* name = String::create_string_from_utf8cstr(assembly_name.name);
    RET_ERR_ON_FAIL(set_runtime_object_field_if_exists(assembly_name_klass, assembly_name_obj, "_name", &name));

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, version, create_runtime_version_object(assembly_name));
    RET_ERR_ON_FAIL(set_runtime_object_field_if_exists(assembly_name_klass, assembly_name_obj, "_version", &version));

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, public_key_token, create_public_key_token_array(assembly_name));
    RET_ERR_ON_FAIL(set_runtime_object_field_if_exists(assembly_name_klass, assembly_name_obj, "_publicKeyToken", &public_key_token));

    int32_t hash_algorithm = static_cast<int32_t>(assembly_name.hash_algorithm);
    int32_t flags = static_cast<int32_t>(assembly_name.flags);
    RET_ERR_ON_FAIL(set_runtime_object_field_if_exists(assembly_name_klass, assembly_name_obj, "_hashAlgorithm", &hash_algorithm));
    RET_ERR_ON_FAIL(set_runtime_object_field_if_exists(assembly_name_klass, assembly_name_obj, "_flags", &flags));

    RET_OK(assembly_name_obj);
}

RtResult<RtReflectionModule*> Reflection::get_module_reflection_object(metadata::RtModuleDef* mod)
{
    auto found = s_module_reflection_map.find(mod);
    if (found != s_module_reflection_map.end())
    {
        RET_OK(found->second);
    }

    auto corlib_types = Class::get_corlib_types();
    auto runtime_module_klass = corlib_types.cls_reflection_module;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, ref_obj_raw, LEANCLR_NEWOBJ_INTERNAL(runtime_module_klass, "Reflection::get_module_reflection_object"));
    auto ref_obj = reinterpret_cast<RtReflectionModule*>(ref_obj_raw);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, global_cls, mod->get_global_type_def());
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionType*, global_type, get_klass_reflection_object(global_cls));
    ref_obj->runtime_type = reinterpret_cast<RtReflectionRuntimeType*>(global_type);
    UNWRAP_OR_RET_ERR_ON_FAIL(ref_obj->assembly, get_assembly_reflection_object(mod->get_assembly()));
    ref_obj->native_handle = mod;
    s_module_reflection_map.insert({mod, ref_obj});
    RET_OK(ref_obj);
}

RtResult<metadata::RtModuleDef*> Reflection::get_module_from_handle_arg(const void* module_arg)
{
    if (module_arg == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    const metadata::RtClass* runtime_module_klass = Class::get_corlib_types().cls_reflection_module;
    auto direct_obj = reinterpret_cast<RtObject*>(const_cast<void*>(module_arg));
    if (gc::GarbageCollector::is_allocated_object(direct_obj) && direct_obj->klass == runtime_module_klass)
    {
        return get_module_from_reflection_object(reinterpret_cast<RtReflectionModule*>(direct_obj));
    }

    auto slot_module = *reinterpret_cast<RtReflectionModule* const*>(module_arg);
    auto slot_obj = reinterpret_cast<RtObject*>(slot_module);
    if (slot_module != nullptr &&
        gc::GarbageCollector::is_allocated_object(slot_obj) &&
        slot_obj->klass == runtime_module_klass)
    {
        return get_module_from_reflection_object(slot_module);
    }

    RET_OK(reinterpret_cast<metadata::RtModuleDef*>(const_cast<void*>(module_arg)));
}

RtResult<metadata::RtModuleDef*> Reflection::get_module_from_qcall_module(void* qcall_module, void* native_handle)
{
    if (native_handle != nullptr)
    {
        RET_OK(reinterpret_cast<metadata::RtModuleDef*>(native_handle));
    }

    if (qcall_module == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    const metadata::RtClass* runtime_module_klass = Class::get_corlib_types().cls_reflection_module;
    auto direct_obj = reinterpret_cast<RtObject*>(qcall_module);
    if (gc::GarbageCollector::is_allocated_object(direct_obj) && direct_obj->klass == runtime_module_klass)
    {
        return get_module_from_reflection_object(reinterpret_cast<RtReflectionModule*>(direct_obj));
    }

    auto runtime_module = *reinterpret_cast<RtReflectionModule**>(qcall_module);
    auto runtime_module_obj = reinterpret_cast<RtObject*>(runtime_module);
    if (runtime_module == nullptr ||
        !gc::GarbageCollector::is_allocated_object(runtime_module_obj) ||
        runtime_module_obj->klass != runtime_module_klass)
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    return get_module_from_reflection_object(runtime_module);
}

RtResult<metadata::RtModuleDef*> Reflection::get_module_from_reflection_object(RtReflectionModule* module_obj)
{
    if (module_obj == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, normalized_obj,
                                            normalize_coreclr_reflection_object(reinterpret_cast<RtObject*>(module_obj)));
    module_obj = reinterpret_cast<RtReflectionModule*>(normalized_obj);
    if (module_obj->native_handle != nullptr)
    {
        RET_OK(module_obj->native_handle);
    }

    const metadata::RtFieldInfo* native_handle_field = Class::get_field_for_name(normalized_obj->klass, "m_pData", true);
    if (native_handle_field == nullptr)
    {
        native_handle_field = Class::get_field_for_name(normalized_obj->klass, "native_handle", true);
    }
    if (native_handle_field != nullptr)
    {
        metadata::RtModuleDef* mod = nullptr;
        RET_ERR_ON_FAIL(Field::get_instance_value(native_handle_field, module_obj, &mod));
        if (mod != nullptr)
        {
            RET_OK(mod);
        }
    }

    if (module_obj->assembly != nullptr)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtAssembly*, assembly,
                                                get_assembly_from_reflection_object(module_obj->assembly));
        if (assembly != nullptr && assembly->mod != nullptr)
        {
            RET_OK(assembly->mod);
        }
    }

    RET_ERR(RtErr::Argument);
}

RtResult<RtObject*> Reflection::invoke_method(const metadata::RtMethodInfo* method, RtObject* obj, RtArray* params, RtObject** out_ex)
{
    const metadata::RtClass* klass = method->parent;
    size_t method_param_count = method->parameter_count;
    size_t params_count = params == nullptr ? 0 : static_cast<size_t>(Array::get_array_length(params));
    auto& corlib_types = Class::get_corlib_types();
    if (method_param_count != params_count)
    {
        if (out_ex)
        {
            *out_ex = Exception::raise_internal_runtime_exception(corlib_types.cls_target_parameter_count_exception, "Parameter count mismatch");
        }
        RET_OK(nullptr);
    }

    if (Method::is_instance(method))
    {
        if (Method::is_ctor(method))
        {
            if (Class::is_array_or_szarray(klass))
            {
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, arr_obj, invoke_new_array(method, params));
                RET_OK(reinterpret_cast<RtObject*>(arr_obj));
            }
            if (obj == nullptr)
            {
                if (Class::is_abstract(klass) || Class::is_interface(klass))
                {
                    *out_ex =
                        Exception::raise_internal_runtime_exception(corlib_types.cls_target_exception, "Cannot create instance of abstract class or interface");
                    RET_OK(nullptr);
                }
                else
                {
                    if (Class::is_nullable_type(klass))
                    {
                        assert(params_count == 1);
                        RtObject* param_obj = Array::get_array_data_at<RtObject*>(params, 0);
                        metadata::RtClass* ele_klass = Class::get_array_element_class(klass);
                        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const void*, ele_data_ptr, Object::unbox_ex(param_obj, ele_klass));
                        return LEANCLR_BOX_OBJECT_INTERNAL(ele_klass, ele_data_ptr, "Reflection::invoke_method");
                    }
                }
            }
            else
            {
                if (!Object::is_inst(obj, klass))
                {
                    RET_ERR(RtErr::InvalidCast);
                }
            }
        }
        else
        {
            if (obj == nullptr)
            {
                if (out_ex)
                {
                    *out_ex = Exception::raise_internal_runtime_exception(corlib_types.cls_target_exception, "Non-static method requires a target.");
                }
                RET_OK(nullptr);
            }
            if (!Object::is_inst(obj, klass))
            {
                RET_ERR(RtErr::InvalidCast);
            }
        }

        if (Method::is_virtual(method))
        {
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, virt_method, Method::get_virtual_method_impl(obj, method));
            method = virt_method;
        }
    }

    return Runtime::invoke_array_arguments_with_run_cctor(method, obj, params);
}

template <typename TKey, typename TValue, typename THash, typename TEqual>
static void visit_object_hashmap(const utils::HashMap<TKey, TValue, THash, TEqual>& map, gc::GcVisitObjectRoot visit, void* userdata)
{
    for (typename utils::HashMap<TKey, TValue, THash, TEqual>::const_iterator it = map.begin(); it != map.end(); ++it)
    {
        if (it->second != nullptr)
        {
            visit(reinterpret_cast<RtObject*>(it->second), userdata);
        }
    }
}

static void visit_reflection_object_roots(gc::GcVisitObjectRoot visit, void* userdata)
{
    visit_object_hashmap(s_class_reflection_type_map, visit, userdata);
    visit_object_hashmap(s_klass_reflection_type_map, visit, userdata);
    visit_object_hashmap(s_method_reflection_map, visit, userdata);
    visit_object_hashmap(s_method_params_map, visit, userdata);
    visit_object_hashmap(s_field_reflection_map, visit, userdata);
    visit_object_hashmap(s_property_reflection_map, visit, userdata);
    visit_object_hashmap(s_event_reflection_map, visit, userdata);
    visit_object_hashmap(s_assembly_reflection_map, visit, userdata);
    visit_object_hashmap(s_module_reflection_map, visit, userdata);
}

void register_reflection_gc_roots()
{
    gc::GcRoots::register_visit_object_roots(visit_reflection_object_roots);
}

} // namespace vm
} // namespace leanclr
