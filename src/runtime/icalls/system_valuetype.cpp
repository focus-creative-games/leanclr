#include "system_valuetype.h"
#include "icall_base.h"

#include "vm/object.h"
#include "vm/class.h"
#include "vm/field.h"
#include "vm/rt_string.h"
#include "vm/rt_array.h"
#include "utils/rt_vector.h"

namespace leanclr
{
namespace icalls
{

template <typename T>
static bool eq_any(const void* ptr1, const void* ptr2)
{
    const T& val1 = *reinterpret_cast<const T*>(ptr1);
    const T& val2 = *reinterpret_cast<const T*>(ptr2);
    return val1 == val2;
}

RtResult<bool> SystemValueType::internal_equals(vm::RtObject* obj1, vm::RtObject* obj2, vm::RtArray** uncompared_field_objs) noexcept
{
    *uncompared_field_objs = nullptr;

    const metadata::RtClass* klass1 = obj1->klass;
    const metadata::RtClass* klass2 = obj2->klass;

    if (klass1 != klass2)
        RET_OK(false);

    // Handle enum types
    if (vm::Class::is_enum_type(klass1))
    {
        const void* data_ptr1 = vm::Object::get_box_value_type_data_ptr(obj1);
        const void* data_ptr2 = vm::Object::get_box_value_type_data_ptr(obj2);

        metadata::RtElementType enum_ele_type = vm::Class::get_enum_element_type(klass1);
        bool equal = false;

        switch (enum_ele_type)
        {
        case metadata::RtElementType::Boolean:
        case metadata::RtElementType::I1:
        case metadata::RtElementType::U1:
            equal = eq_any<uint8_t>(data_ptr1, data_ptr2);
            break;
        case metadata::RtElementType::I2:
        case metadata::RtElementType::Char:
        case metadata::RtElementType::U2:
            equal = eq_any<uint16_t>(data_ptr1, data_ptr2);
            break;
        case metadata::RtElementType::I4:
        case metadata::RtElementType::U4:
            equal = eq_any<uint32_t>(data_ptr1, data_ptr2);
            break;
        case metadata::RtElementType::I8:
        case metadata::RtElementType::U8:
            equal = eq_any<uint64_t>(data_ptr1, data_ptr2);
            break;
        default:
            RET_ASSERT_ERR(RtErr::ExecutionEngine);
        }

        RET_OK(equal);
    }

    // Iterate through all fields
    utils::Vector<vm::RtObject*> uncompared_fields;
    const metadata::RtFieldInfo* fields = klass1->fields;
    uint32_t field_count = klass1->field_count;

    for (uint32_t i = 0; i < field_count; ++i)
    {
        const metadata::RtFieldInfo* field = &fields[i];

        // Skip static fields
        if (!vm::Field::is_instance(field))
            continue;

        metadata::RtElementType field_ele_type = field->type_sig->ele_type;
        size_t offset = vm::Field::get_instance_field_offset_includes_object_header_for_all_type(field);

        uint8_t* field_data_ptr1 = reinterpret_cast<uint8_t*>(obj1) + offset;
        uint8_t* field_data_ptr2 = reinterpret_cast<uint8_t*>(obj2) + offset;

        bool field_equal = false;

        switch (field_ele_type)
        {
        case metadata::RtElementType::Boolean:
        case metadata::RtElementType::I1:
        case metadata::RtElementType::U1:
            if (!eq_any<uint8_t>(field_data_ptr1, field_data_ptr2))
                RET_OK(false);
            break;

        case metadata::RtElementType::I2:
        case metadata::RtElementType::Char:
        case metadata::RtElementType::U2:
            if (!eq_any<uint16_t>(field_data_ptr1, field_data_ptr2))
                RET_OK(false);
            break;

        case metadata::RtElementType::I4:
        case metadata::RtElementType::U4:
            if (!eq_any<uint32_t>(field_data_ptr1, field_data_ptr2))
                RET_OK(false);
            break;

        case metadata::RtElementType::I8:
        case metadata::RtElementType::U8:
            if (!eq_any<uint64_t>(field_data_ptr1, field_data_ptr2))
                RET_OK(false);
            break;

        case metadata::RtElementType::I:
        case metadata::RtElementType::U:
            if (!eq_any<uintptr_t>(field_data_ptr1, field_data_ptr2))
                RET_OK(false);
            break;

        case metadata::RtElementType::R4:
            if (!eq_any<float>(field_data_ptr1, field_data_ptr2))
                RET_OK(false);
            break;

        case metadata::RtElementType::R8:
            if (!eq_any<double>(field_data_ptr1, field_data_ptr2))
                RET_OK(false);
            break;

        case metadata::RtElementType::Ptr:
        case metadata::RtElementType::FnPtr:
        {
            if (!eq_any<void*>(field_data_ptr1, field_data_ptr2))
                RET_OK(false);
            break;
        }
        case metadata::RtElementType::String:
        {
            vm::RtString* str1 = *reinterpret_cast<vm::RtString**>(field_data_ptr1);
            vm::RtString* str2 = *reinterpret_cast<vm::RtString**>(field_data_ptr2);
            if (str1 != str2)
            {
                if (str1 == nullptr || str2 == nullptr)
                    RET_OK(false);
                if (vm::String::get_length(str1) != vm::String::get_length(str2))
                    RET_OK(false);
                if (std::memcmp(vm::String::get_chars_ptr(str1), vm::String::get_chars_ptr(str2),
                                static_cast<size_t>(vm::String::get_length(str1)) * sizeof(Utf16Char)) != 0)
                {
                    RET_OK(false);
                }
            }
            break;
        }

        case metadata::RtElementType::Object:
        case metadata::RtElementType::Array:
        case metadata::RtElementType::SZArray:
        {
            vm::RtObject* obj_ref1 = *reinterpret_cast<vm::RtObject**>(field_data_ptr1);
            vm::RtObject* obj_ref2 = *reinterpret_cast<vm::RtObject**>(field_data_ptr2);
            if (obj_ref1 != obj_ref2)
            {
                uncompared_fields.push_back(obj_ref1);
                uncompared_fields.push_back(obj_ref2);
            }
            break;
        }

        default:
        {
            // Handle class types
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, field_klass, vm::Class::get_class_from_typesig(field->type_sig));

            if (vm::Class::is_value_type(field_klass))
            {
                if (vm::Class::is_enum_type(field_klass))
                {
                    metadata::RtElementType enum_ele_type = vm::Class::get_enum_element_type(field_klass);
                    bool enum_equal = false;

                    switch (enum_ele_type)
                    {
                    case metadata::RtElementType::Boolean:
                    case metadata::RtElementType::I1:
                    case metadata::RtElementType::U1:
                        enum_equal = eq_any<uint8_t>(field_data_ptr1, field_data_ptr2);
                        break;
                    case metadata::RtElementType::Char:
                    case metadata::RtElementType::I2:
                    case metadata::RtElementType::U2:
                        enum_equal = eq_any<uint16_t>(field_data_ptr1, field_data_ptr2);
                        break;
                    case metadata::RtElementType::I4:
                    case metadata::RtElementType::U4:
                        enum_equal = eq_any<uint32_t>(field_data_ptr1, field_data_ptr2);
                        break;
                    case metadata::RtElementType::I8:
                    case metadata::RtElementType::U8:
                        enum_equal = eq_any<uint64_t>(field_data_ptr1, field_data_ptr2);
                        break;
                    default:
                        RET_ASSERT_ERR(RtErr::ExecutionEngine);
                    }

                    if (!enum_equal)
                        RET_OK(false);
                }
                else
                {
                    // Box value types and compare
                    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, boxed1, vm::Object::box_object(field_klass, field_data_ptr1));
                    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, boxed2, vm::Object::box_object(field_klass, field_data_ptr2));
                    uncompared_fields.push_back(boxed1);
                    uncompared_fields.push_back(boxed2);
                }
            }
            else
            {
                vm::RtObject* obj_ref1 = *reinterpret_cast<vm::RtObject**>(field_data_ptr1);
                vm::RtObject* obj_ref2 = *reinterpret_cast<vm::RtObject**>(field_data_ptr2);
                if (obj_ref1 != obj_ref2)
                {
                    uncompared_fields.push_back(obj_ref1);
                    uncompared_fields.push_back(obj_ref2);
                }
            }
            break;
        }
        }
    }

    if (uncompared_fields.empty())
    {
        RET_OK(true);
    }
    else
    {
        // Create array from uncompared fields
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(
            vm::RtArray*, arr, vm::Array::new_szarray_from_ele_klass(vm::Class::get_corlib_types().cls_object, static_cast<int32_t>(uncompared_fields.size())));

        vm::RtObject** arr_data = vm::Array::get_array_data_start_as<vm::RtObject*>(arr);
        for (size_t i = 0; i < uncompared_fields.size(); ++i)
        {
            arr_data[i] = uncompared_fields[i];
        }

        *uncompared_field_objs = arr;
        RET_OK(false);
    }
}

/// @icall: System.ValueType::InternalEquals(System.Object,System.Object,System.Object[]&)
static RtResultVoid internal_equals_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                            const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto obj1 = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    auto obj2 = EvalStackOp::get_param<vm::RtObject*>(params, 1);
    auto uncompared_field_objs_ptr = EvalStackOp::get_param<vm::RtArray**>(params, 2);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemValueType::internal_equals(obj1, obj2, uncompared_field_objs_ptr));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

RtResult<int32_t> SystemValueType::internal_get_hash_code(vm::RtObject* obj, vm::RtArray** uncompared_field_objs) noexcept
{
    *uncompared_field_objs = nullptr;

    const metadata::RtClass* klass = obj->klass;
    int32_t hash = static_cast<int32_t>(reinterpret_cast<uintptr_t>(klass));

    utils::Vector<vm::RtObject*> uncomputed_fields;
    const metadata::RtFieldInfo* fields = klass->fields;
    uint32_t field_count = klass->field_count;

    for (uint32_t i = 0; i < field_count; ++i)
    {
        const metadata::RtFieldInfo* field = &fields[i];

        // Skip static fields
        if (!vm::Field::is_instance(field))
            continue;

        metadata::RtElementType field_ele_type = field->type_sig->ele_type;
        size_t offset = vm::Field::get_instance_field_offset_includes_object_header_for_all_type(field);
        uint8_t* field_data_ptr = reinterpret_cast<uint8_t*>(obj) + offset;

        int32_t field_hash = 0;

        switch (field_ele_type)
        {
        case metadata::RtElementType::Boolean:
        case metadata::RtElementType::I1:
        case metadata::RtElementType::U1:
            field_hash = *reinterpret_cast<uint8_t*>(field_data_ptr);
            break;

        case metadata::RtElementType::I2:
        case metadata::RtElementType::Char:
        case metadata::RtElementType::U2:
            field_hash = *reinterpret_cast<uint16_t*>(field_data_ptr);
            break;

        case metadata::RtElementType::I4:
        case metadata::RtElementType::U4:
            field_hash = *reinterpret_cast<int32_t*>(field_data_ptr);
            break;

        case metadata::RtElementType::I8:
        case metadata::RtElementType::U8:
        {
            int32_t* two_part = reinterpret_cast<int32_t*>(field_data_ptr);
            field_hash = two_part[0] ^ two_part[1];
            break;
        }

        case metadata::RtElementType::I:
        case metadata::RtElementType::U:
        case metadata::RtElementType::Ptr:
        case metadata::RtElementType::FnPtr:
        {
#if LEANCLR_ARCH_64BIT
            uintptr_t ptr_value = *reinterpret_cast<uintptr_t*>(field_data_ptr);
            uint32_t lo = static_cast<uint32_t>(ptr_value);
            uint32_t hi = static_cast<uint32_t>(ptr_value >> 32);
            field_hash = static_cast<int32_t>(lo ^ hi);
#else

            field_hash = *reinterpret_cast<int32_t*>(field_data_ptr);
#endif
            break;
        }

        case metadata::RtElementType::String:
        {
            vm::RtString* str_ref = *reinterpret_cast<vm::RtString**>(field_data_ptr);
            if (str_ref == nullptr)
            {
                field_hash = 0;
            }
            else
            {
                field_hash = vm::String::get_hash_code(str_ref);
            }
            break;
        }

        case metadata::RtElementType::Object:
        case metadata::RtElementType::Array:
        case metadata::RtElementType::SZArray:
        {
            vm::RtObject* obj_ref = *reinterpret_cast<vm::RtObject**>(field_data_ptr);
            if (obj_ref == nullptr)
            {
                field_hash = 0;
            }
            else
            {
                uncomputed_fields.push_back(obj_ref);
                continue;
            }
            break;
        }

        default:
        {
            // Handle class types
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, field_klass, vm::Class::get_class_from_typesig(field->type_sig));

            if (vm::Class::is_value_type(field_klass))
            {
                if (vm::Class::is_enum_type(field_klass))
                {
                    metadata::RtElementType enum_ele_type = vm::Class::get_enum_element_type(field_klass);

                    switch (enum_ele_type)
                    {
                    case metadata::RtElementType::Boolean:
                    case metadata::RtElementType::I1:
                    case metadata::RtElementType::U1:
                        field_hash = *reinterpret_cast<uint8_t*>(field_data_ptr);
                        break;
                    case metadata::RtElementType::I2:
                    case metadata::RtElementType::Char:
                    case metadata::RtElementType::U2:
                        field_hash = *reinterpret_cast<uint16_t*>(field_data_ptr);
                        break;
                    case metadata::RtElementType::I4:
                    case metadata::RtElementType::U4:
                        field_hash = *reinterpret_cast<int32_t*>(field_data_ptr);
                        break;
                    case metadata::RtElementType::I8:
                    case metadata::RtElementType::U8:
                    {
                        int32_t* two_part = reinterpret_cast<int32_t*>(field_data_ptr);
                        field_hash = two_part[0] ^ two_part[1];
                        break;
                    }
                    default:
                        RET_ASSERT_ERR(RtErr::ExecutionEngine);
                    }
                }
                else
                {
                    // Box value types and include in uncomputed
                    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, boxed, vm::Object::box_object(field_klass, field_data_ptr));
                    uncomputed_fields.push_back(boxed);
                    continue;
                }
            }
            else
            {
                vm::RtObject* obj_ref = *reinterpret_cast<vm::RtObject**>(field_data_ptr);
                if (obj_ref == nullptr)
                {
                    field_hash = 0;
                }
                else
                {
                    uncomputed_fields.push_back(obj_ref);
                    continue;
                }
            }
            break;
        }
        }

        hash = hash * 31 + field_hash;
    }

    if (!uncomputed_fields.empty())
    {
        // Create array from uncomputed fields
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(
            vm::RtArray*, arr, vm::Array::new_szarray_from_ele_klass(vm::Class::get_corlib_types().cls_object, static_cast<int32_t>(uncomputed_fields.size())));

        vm::RtObject** arr_data = vm::Array::get_array_data_start_as<vm::RtObject*>(arr);
        for (size_t i = 0; i < uncomputed_fields.size(); ++i)
        {
            arr_data[i] = uncomputed_fields[i];
        }

        *uncompared_field_objs = arr;
    }

    RET_OK(hash);
}

/// @icall: System.ValueType::InternalGetHashCode(System.Object,System.Object[]&)
static RtResultVoid internal_get_hash_code_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                   const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto obj = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    auto uncomputed_field_objs_ptr = EvalStackOp::get_param<vm::RtArray**>(params, 1);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, hash_code, SystemValueType::internal_get_hash_code(obj, uncomputed_field_objs_ptr));
    EvalStackOp::set_return(ret, hash_code);
    RET_VOID_OK();
}

static vm::InternalCallEntry s_internal_call_entries_system_valuetype[] = {
    {"System.ValueType::InternalEquals(System.Object,System.Object,System.Object[]&)", (vm::InternalCallFunction)&SystemValueType::internal_equals,
     internal_equals_invoker},
    {"System.ValueType::InternalGetHashCode(System.Object,System.Object[]&)", (vm::InternalCallFunction)&SystemValueType::internal_get_hash_code,
     internal_get_hash_code_invoker},
};

utils::Span<vm::InternalCallEntry> SystemValueType::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_valuetype,
                                              sizeof(s_internal_call_entries_system_valuetype) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr
