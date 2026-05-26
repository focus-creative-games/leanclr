#include "object.h"
#include "class.h"
#include "runtime.h"
#include "rt_managed_types.h"
#include "rt_array.h"
#include "type.h"
#include "rt_string.h"
#include "field.h"

namespace leanclr
{
namespace vm
{

RtResult<RtObject*> Object::__new_object(const metadata::RtClass* klass LEANCLR_GC_DECLARE_CALL_SITE_PARAM)
{
    if (LEANCLR_UNLIKELY(Class::is_cctor_not_finished(klass)))
    {
        RET_ERR_ON_FAIL(Runtime::run_class_static_constructor(klass));
    }

    const size_t total_size = sizeof(RtObject) + klass->instance_size_without_header;
    RtObject* ptr = gc::GarbageCollector::allocate_object(klass, total_size LEANCLR_GC_CALL_SITE_PARAM);
    if (ptr == nullptr)
    {
        RET_ERR(RtErr::OutOfMemory);
    }
    assert(ptr->klass == klass);
    RET_OK(ptr);
}

RtResult<RtObject*> Object::internal_create_instance(const metadata::RtTypeSig* type_sig LEANCLR_GC_DECLARE_CALL_SITE_PARAM)
{
    if (type_sig->is_by_ref())
    {
        RET_ERR(RtErr::Argument);
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, Class::get_class_from_typesig(type_sig));
    RET_ERR_ON_FAIL(vm::Class::initialize_all(klass));
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
        case metadata::RtElementType::Object:
        {
            break;
        }
        case metadata::RtElementType::ValueType:
        case metadata::RtElementType::Class:
        {
            if (vm::Class::is_generic(klass))
            {
                RET_ERR(RtErr::Argument);
            }
            break;
        }
        case metadata::RtElementType::GenericInst:
        {
            if (vm::Type::contains_generic_param(type_sig))
            {
                RET_ERR(RtErr::Argument);
            }
            if (Class::is_nullable_type(klass))
            {
                RET_OK(nullptr);
            }
            break;
        }
        case metadata::RtElementType::String:
        case metadata::RtElementType::SZArray:
        case metadata::RtElementType::Array:
        {
            RET_ERR(RtErr::Argument);
        }
        // case metadata::RtElementType::String:
        // {
        //     RET_OK(vm::String::get_empty_string());
        // }
        // case metadata::RtElementType::SZArray:
        // {
        //     metadata::RtClass* ele_klass = vm::Class::get_array_element_class(klass);
        //     auto ret_arr = LEANCLR_NEW_EMPTY_SZARRAY_BY_ELE_KLASS_INTERNAL(ele_klass,
        //                                                   "ActivationServices::AllocateUninitializedClassInstance");
        //     return ret_arr.cast<vm::RtObject*>();
        // }
        default:
        {
            RET_ERR(RtErr::Argument);
        }
    }

    if (vm::Class::is_interface(klass) || vm::Class::is_abstract(klass))
    {
        RET_ERR(RtErr::Argument);
    }
    return __new_object(klass LEANCLR_GC_CALL_SITE_PARAM);
}

// Helper: Create a new boxed object for internal use
static RtResult<RtObject*> box_object_internal(const metadata::RtClass* klass, const void* value LEANCLR_GC_DECLARE_CALL_SITE_PARAM)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, obj, Object::__new_object(klass LEANCLR_GC_CALL_SITE_PARAM));
    // Value may be unaligned, so use memcpy for safe copy
    std::memcpy(reinterpret_cast<uint8_t*>(obj) + sizeof(RtObject), value, klass->instance_size_without_header);
    RET_OK(obj);
}

RtResult<RtObject*> Object::__box_object(const metadata::RtClass* klass, const void* value LEANCLR_GC_DECLARE_CALL_SITE_PARAM)
{
    if (!Class::is_nullable_type(klass))
    {
        return box_object_internal(klass, value LEANCLR_GC_CALL_SITE_PARAM);
    }

    // Handle nullable types
    const uint8_t* value_ptr = reinterpret_cast<const uint8_t*>(value);
    const metadata::RtFieldInfo* has_value_field = Field::get_nullable_has_value_field(klass);
    uint32_t has_value_field_offset = has_value_field->offset;

    // Check if HasValue is false (null)
    if (value_ptr[has_value_field_offset] == 0)
    {
        RET_OK(nullptr);
    }

    // Second field contains the actual value
    const metadata::RtFieldInfo* value_field = Field::get_nullable_value_field(klass);
    uint32_t value_field_offset = value_field->offset;

    metadata::RtClass* data_class = Class::get_nullable_underlying_class(klass);
    return box_object_internal(data_class, value_ptr + value_field_offset LEANCLR_GC_CALL_SITE_PARAM);
}

// Get pointer to boxed value data
const void* Object::get_box_value_type_data_ptr(const RtObject* obj)
{
    assert(obj);
    const metadata::RtClass* klass = obj->klass;
    assert(Class::is_value_type(klass));
    assert(klass->element_class);

    return reinterpret_cast<const uint8_t*>(obj) + sizeof(RtObject);
}

// Get pointer to boxed enum data
const void* Object::get_boxed_enum_data_ptr(const RtObject* obj)
{
    assert(obj);
    const metadata::RtClass* klass = obj->klass;
    assert(Class::is_enum_type(klass));
    assert(klass->element_class);

    return reinterpret_cast<const uint8_t*>(obj) + sizeof(RtObject);
}

void Object::extends_to_eval_stack(const void* src, interp::RtStackObject* dst, const metadata::RtClass* ele_klass)
{
    assert(src && dst && ele_klass);

    metadata::RtElementType ele_type = ele_klass->by_val->ele_type;

    switch (ele_type)
    {
    case metadata::RtElementType::Boolean:
    case metadata::RtElementType::I1:
    {
        int8_t v = *reinterpret_cast<const int8_t*>(src);
        *reinterpret_cast<int32_t*>(dst) = static_cast<int32_t>(v);
        break;
    }
    case metadata::RtElementType::U1:
    {
        uint8_t v = *reinterpret_cast<const uint8_t*>(src);
        *reinterpret_cast<int32_t*>(dst) = static_cast<int32_t>(v);
        break;
    }
    case metadata::RtElementType::I2:
    {
        int16_t v = *reinterpret_cast<const int16_t*>(src);
        *reinterpret_cast<int32_t*>(dst) = static_cast<int32_t>(v);
        break;
    }
    case metadata::RtElementType::U2:
    case metadata::RtElementType::Char:
    {
        uint16_t v = *reinterpret_cast<const uint16_t*>(src);
        *reinterpret_cast<int32_t*>(dst) = static_cast<int32_t>(v);
        break;
    }
    case metadata::RtElementType::I4:
    case metadata::RtElementType::U4:
    case metadata::RtElementType::R4:
    {
        int32_t v = *reinterpret_cast<const int32_t*>(src);
        *reinterpret_cast<int32_t*>(dst) = v;
        break;
    }
    case metadata::RtElementType::I8:
    case metadata::RtElementType::U8:
    case metadata::RtElementType::R8:
    {
        int64_t v = *reinterpret_cast<const int64_t*>(src);
        *reinterpret_cast<int64_t*>(dst) = v;
        break;
    }
    case metadata::RtElementType::I:
    case metadata::RtElementType::U:
    case metadata::RtElementType::Ptr:
    case metadata::RtElementType::FnPtr:
    {
        intptr_t v = *reinterpret_cast<const intptr_t*>(src);
        *reinterpret_cast<intptr_t*>(dst) = v;
        break;
    }
    case metadata::RtElementType::String:
    case metadata::RtElementType::Class:
    case metadata::RtElementType::Object:
    case metadata::RtElementType::Array:
    case metadata::RtElementType::SZArray:
    {
        const RtObject* v = *reinterpret_cast<const RtObject* const*>(src);
        *reinterpret_cast<const RtObject**>(dst) = v;
        break;
    }
    case metadata::RtElementType::ValueType:
    case metadata::RtElementType::TypedByRef:
    {
        size_t size = ele_klass->instance_size_without_header;
        std::memcpy(dst, src, size);
        break;
    }
    case metadata::RtElementType::GenericInst:
    {
        if (Class::is_value_type(ele_klass))
        {
            size_t size = ele_klass->instance_size_without_header;
            std::memcpy(dst, src, size);
        }
        else
        {
            const RtObject* v = *reinterpret_cast<const RtObject* const*>(src);
            *reinterpret_cast<const RtObject**>(dst) = v;
        }
        break;
    }
    default:
        assert(false && "extends_to_i32_on_stack: unsupported element type");
        break;
    }
}

// Unbox value from boxed object
RtResultVoid Object::unbox_any(const RtObject* obj, const metadata::RtClass* klass, void* dst, bool extend_to_stack)
{
    assert(dst && klass);

    const metadata::RtClass* element_class = klass->element_class;
    assert(element_class);

    const metadata::RtClass* unbox_cast_klass = element_class->cast_class;

    if (!Class::is_nullable_type(klass))
    {
        // Non-nullable type
        if (!obj)
        {
            RET_ERR(RtErr::NullReference);
        }

        if (obj->klass->cast_class != unbox_cast_klass)
        {
            RET_ERR(RtErr::InvalidCast);
        }

        const void* src = reinterpret_cast<const uint8_t*>(obj) + sizeof(RtObject);

        if (extend_to_stack)
        {
            extends_to_eval_stack(src, reinterpret_cast<interp::RtStackObject*>(dst), unbox_cast_klass);
        }
        else
        {
            std::memcpy(dst, src, klass->instance_size_without_header);
        }

        RET_VOID_OK();
    }

    // Handle nullable type
    if (!obj)
    {
        // Null value - zero initialize destination
        std::memset(dst, 0, klass->instance_size_without_header);
        RET_VOID_OK();
    }

    // Get the underlying value type
    if (obj->klass->cast_class != unbox_cast_klass)
    {
        RET_ERR(RtErr::InvalidCast);
    }

    // Copy HasValue (true) and the actual value
    uint8_t* dst_ptr = reinterpret_cast<uint8_t*>(dst);
    const uint8_t* src_ptr = reinterpret_cast<const uint8_t*>(obj) + sizeof(RtObject);

    uint32_t has_value_field_offset = Field::get_nullable_has_value_field(klass)->offset;
    uint32_t value_field_offset = Field::get_nullable_value_field(klass)->offset;

    // Set HasValue to 1
    dst_ptr[has_value_field_offset] = 1;

    // Copy actual value
    std::memcpy(dst_ptr + value_field_offset, src_ptr, unbox_cast_klass->instance_size_without_header);

    RET_VOID_OK();
}

// Unbox with exact type checking
RtResult<const void*> Object::unbox_ex(const RtObject* obj, const metadata::RtClass* unbox_class)
{
    assert(unbox_class);

    if (!Class::is_nullable_type(unbox_class))
    {
        // Non-nullable type
        if (!obj)
        {
            RET_ERR(RtErr::NullReference);
        }

        if (obj->klass->element_class != unbox_class->element_class)
        {
            RET_ERR(RtErr::InvalidCast);
        }

        const void* result = reinterpret_cast<const uint8_t*>(obj) + sizeof(RtObject);
        RET_OK(result);
    }

    // Handle nullable type
    if (!obj)
    {
        RET_OK(nullptr);
    }

    const metadata::RtClass* result_class = unbox_class->element_class;
    if (obj->klass->element_class->element_class != result_class)
    {
        RET_ERR(RtErr::InvalidCast);
    }

    const void* result = reinterpret_cast<const uint8_t*>(obj) + sizeof(RtObject);
    RET_OK(result);
}

// Type checking: is obj an instance of class?
RtObject* Object::is_inst(RtObject* obj, const metadata::RtClass* klass)
{
    assert(obj && klass);

    const metadata::RtClass* obj_class = obj->klass;
    if (Class::is_assignable_from(obj_class, klass))
    {
        return obj;
    }
    return nullptr;
}

// Type casting: cast obj to class (or null if incompatible)
RtObject* Object::cast_class(RtObject* obj, const metadata::RtClass* klass)
{
    assert(obj && klass);

    const metadata::RtClass* obj_class = obj->klass;
    if (Class::is_assignable_from(obj_class, klass))
    {
        return obj;
    }
    return nullptr;
}

// Clone an object
RtResult<RtObject*> Object::__clone(RtObject* obj LEANCLR_GC_DECLARE_CALL_SITE_PARAM)
{
    assert(obj);
    const metadata::RtClass* klass = obj->klass;
    assert(!Class::is_string_class(klass));

    RtObject* result = nullptr;

    if (Class::is_szarray_class(klass))
    {
        auto ret_clone = Array::__clone(reinterpret_cast<RtArray*>(obj) LEANCLR_GC_CALL_SITE_PARAM);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, array_clone, ret_clone);
        result = reinterpret_cast<RtObject*>(array_clone);
    }
    else
    {
        auto ret_new_obj = Object::__new_object(klass LEANCLR_GC_CALL_SITE_PARAM);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, new_obj, ret_new_obj);
        const void* src = reinterpret_cast<const uint8_t*>(obj) + sizeof(RtObject);
        void* dst = reinterpret_cast<uint8_t*>(new_obj) + sizeof(RtObject);
        std::memcpy(dst, src, klass->instance_size_without_header);
        result = new_obj;
    }

    RET_OK(result);
}

} // namespace vm
} // namespace leanclr
