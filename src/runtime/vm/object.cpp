#include "object.h"
#include "class.h"
#include "runtime.h"
#include "gc/garbage_collector.h"
#include "rt_managed_types.h"
#include "rt_array.h"

namespace leanclr
{
namespace vm
{

// Helper: Create a new boxed object for internal use
static RtResult<RtObject*> box_object_internal(const metadata::RtClass* klass, const void* value)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, obj, Object::new_object(klass));
    // Value may be unaligned, so use memcpy for safe copy
    std::memcpy(reinterpret_cast<uint8_t*>(obj) + sizeof(RtObject), value, klass->instance_size_without_header);
    RET_OK(obj);
}

// Create new instance of a class
RtResult<RtObject*> Object::new_object(const metadata::RtClass* klass)
{
    RET_ERR_ON_FAIL(Class::initialize_all(const_cast<metadata::RtClass*>(klass)));
    RET_ERR_ON_FAIL(Runtime::run_class_static_constructor(klass));

    size_t total_size = sizeof(RtObject) + klass->instance_size_without_header;
    RtObject* ptr = gc::GarbageCollector::allocate_object(klass, total_size);

    assert(ptr && ptr->klass == klass);
    RET_OK(ptr);
}

// Box a value type into an object
RtResult<RtObject*> Object::box_object(const metadata::RtClass* klass, const void* value)
{
    if (!Class::is_nullable_type(klass))
    {
        return box_object_internal(klass, value);
    }

    // Handle nullable types
    const uint8_t* value_ptr = reinterpret_cast<const uint8_t*>(value);
    const metadata::RtFieldInfo* fields = klass->fields;

    // First field is the "HasValue" boolean
    uint32_t has_value_field_offset = fields->offset;

    // Check if HasValue is false (null)
    if (value_ptr[has_value_field_offset] == 0)
    {
        RET_OK(nullptr);
    }

    // Second field contains the actual value
    const metadata::RtFieldInfo* value_field = fields + 1;
    uint32_t value_field_offset = value_field->offset;

    metadata::RtClass* data_class = Class::get_nullable_underlying_class(klass);
    return box_object_internal(data_class, value_ptr + value_field_offset);
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
    const metadata::RtClass* obj_ele_klass = obj->klass->element_class;
    if (obj_ele_klass->element_class != unbox_cast_klass)
    {
        RET_ERR(RtErr::InvalidCast);
    }

    // Copy HasValue (true) and the actual value
    uint8_t* dst_ptr = reinterpret_cast<uint8_t*>(dst);
    const uint8_t* src_ptr = reinterpret_cast<const uint8_t*>(obj) + sizeof(RtObject);

    const metadata::RtFieldInfo* fields = klass->fields;
    uint32_t has_value_field_offset = fields->offset;
    uint32_t value_field_offset = (fields + 1)->offset;

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
RtResult<RtObject*> Object::clone(RtObject* obj)
{
    assert(obj);
    const metadata::RtClass* klass = obj->klass;
    assert(!Class::is_string_class(klass));

    RtObject* result = nullptr;

    if (Class::is_szarray_class(klass))
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, array_clone, Array::clone(reinterpret_cast<RtArray*>(obj)));
        result = reinterpret_cast<RtObject*>(array_clone);
    }
    else
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, new_obj, new_object(klass));
        const void* src = reinterpret_cast<const uint8_t*>(obj) + sizeof(RtObject);
        void* dst = reinterpret_cast<uint8_t*>(new_obj) + sizeof(RtObject);
        std::memcpy(dst, src, klass->instance_size_without_header);
        result = new_obj;
    }

    RET_OK(result);
}

} // namespace vm
} // namespace leanclr
