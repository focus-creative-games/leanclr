#include "system_enum.h"
#include "icall_base.h"
#include "vm/class.h"
#include "vm/enum.h"
#include "vm/object.h"
#include "vm/reflection.h"

namespace leanclr
{
namespace icalls
{

// ========== Enum comparison result enum ==========
enum class EnumComparisonResult : int32_t
{
    Less = -1,
    Equal = 0,
    Greater = 1,
    IncompatibleKlass = 2,
};

template <typename T>
static EnumComparisonResult cmp_any(const void* p1, const void* p2)
{
    T v1 = *static_cast<const T*>(p1);
    T v2 = *static_cast<const T*>(p2);
    if (v1 < v2)
    {
        return EnumComparisonResult::Less;
    }
    else if (v1 > v2)
    {
        return EnumComparisonResult::Greater;
    }
    else
    {
        return EnumComparisonResult::Equal;
    }
}

// ========== Impl functions ==========

RtResult<int32_t> SystemEnum::internal_compare_to(vm::RtObject* obj1, vm::RtObject* obj2) noexcept
{
    const metadata::RtClass* klass1 = obj1->klass;
    const metadata::RtClass* klass2 = obj2->klass;

    EnumComparisonResult result;
    if (klass1 != klass2)
    {
        result = EnumComparisonResult::IncompatibleKlass;
    }
    else
    {
        const void* data1 = vm::Object::get_boxed_enum_data_ptr(obj1);
        const void* data2 = vm::Object::get_boxed_enum_data_ptr(obj2);
        metadata::RtElementType elem_type = vm::Class::get_enum_element_type(klass1);

        switch (elem_type)
        {
        case metadata::RtElementType::Boolean:
            result = cmp_any<uint8_t>(data1, data2);
            break;
        case metadata::RtElementType::Char:
            result = cmp_any<uint16_t>(data1, data2);
            break;
        case metadata::RtElementType::I1:
            result = cmp_any<int8_t>(data1, data2);
            break;
        case metadata::RtElementType::U1:
            result = cmp_any<uint8_t>(data1, data2);
            break;
        case metadata::RtElementType::I2:
            result = cmp_any<int16_t>(data1, data2);
            break;
        case metadata::RtElementType::U2:
            result = cmp_any<uint16_t>(data1, data2);
            break;
        case metadata::RtElementType::I4:
            result = cmp_any<int32_t>(data1, data2);
            break;
        case metadata::RtElementType::U4:
            result = cmp_any<uint32_t>(data1, data2);
            break;
        case metadata::RtElementType::I8:
            result = cmp_any<int64_t>(data1, data2);
            break;
        case metadata::RtElementType::U8:
            result = cmp_any<uint64_t>(data1, data2);
            break;
        default:
            RET_ASSERT_ERR(RtErr::ExecutionEngine);
        }
    }

    RET_OK(static_cast<int32_t>(result));
}

RtResult<vm::RtReflectionRuntimeType*> SystemEnum::internal_get_underlying_type(vm::RtReflectionRuntimeType* enum_klass) noexcept
{
    const metadata::RtTypeSig* type_sig = enum_klass->reflection_type.type_handle;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, reflection_type, vm::Reflection::get_klass_reflection_object(klass->element_class));
    RET_OK(reinterpret_cast<vm::RtReflectionRuntimeType*>(reflection_type));
}

RtResult<bool> SystemEnum::get_enum_values_and_names(vm::RtReflectionRuntimeType* enum_klass, vm::RtArray** values, vm::RtArray** names) noexcept
{
    const metadata::RtTypeSig* type_sig = enum_klass->reflection_type.type_handle;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));
    auto result = vm::Enum::get_enum_values_and_names(klass);
    RET_ERR_ON_FAIL(result);
    auto result_tuple = result.unwrap();

    bool sorted = std::get<0>(result_tuple);
    *values = std::get<1>(result_tuple);
    *names = std::get<2>(result_tuple);
    RET_OK(sorted);
}

RtResult<vm::RtObject*> SystemEnum::internal_box_enum(vm::RtReflectionRuntimeType* runtime_type, uint64_t value) noexcept
{
    const metadata::RtTypeSig* type_sig = runtime_type->reflection_type.type_handle;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));
    assert(vm::Class::is_enum_type(klass));
    return vm::Object::box_object(klass, &value);
}

RtResult<vm::RtObject*> SystemEnum::get_value(vm::RtObject* obj) noexcept
{
    if (obj == nullptr)
    {
        RET_OK(nullptr);
    }

    const void* data_ptr = vm::Object::get_boxed_enum_data_ptr(obj);
    const metadata::RtClass* element_class = obj->klass->element_class;
    return vm::Object::box_object(element_class, data_ptr);
}

RtResult<bool> SystemEnum::internal_has_flag(vm::RtObject* obj, vm::RtObject* flag) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint64_t, value1, vm::Enum::get_boxed_enum_data_as_unsigned_and_extended_to_u64(obj));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint64_t, value2, vm::Enum::get_boxed_enum_data_as_unsigned_and_extended_to_u64(flag));
    bool result = (value1 & value2) == value2;
    RET_OK(result);
}

RtResult<int32_t> SystemEnum::get_hash_code(vm::RtObject* obj) noexcept
{
    return vm::Enum::get_hash_code(obj);
}

// ========== Invoker functions ==========

/// @icall: System.Enum::InternalCompareTo
static RtResultVoid internal_compare_to_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                interp::RtStackObject* ret) noexcept
{
    vm::RtObject* enum_obj = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    vm::RtObject* other = EvalStackOp::get_param<vm::RtObject*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, SystemEnum::internal_compare_to(enum_obj, other));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Enum::InternalGetUnderlyingType
static RtResultVoid internal_get_underlying_type_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                         interp::RtStackObject* ret) noexcept
{
    vm::RtReflectionRuntimeType* enum_klass = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionRuntimeType*, underlying_type, SystemEnum::internal_get_underlying_type(enum_klass));
    EvalStackOp::set_return(ret, underlying_type);
    RET_VOID_OK();
}

/// @icall: System.Enum::GetEnumValuesAndNames
static RtResultVoid get_enum_values_and_names_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                      interp::RtStackObject* ret) noexcept
{
    vm::RtReflectionRuntimeType* enum_klass = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    vm::RtArray* values = nullptr;
    vm::RtArray* names = nullptr;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, sorted, SystemEnum::get_enum_values_and_names(enum_klass, &values, &names));
    EvalStackOp::set_return(ret, static_cast<int32_t>(sorted));

    vm::RtArray** ret_values_ptr = EvalStackOp::get_param<vm::RtArray**>(params, 1);
    vm::RtArray** ret_names_ptr = EvalStackOp::get_param<vm::RtArray**>(params, 2);
    *ret_values_ptr = values;
    *ret_names_ptr = names;
    RET_VOID_OK();
}

/// @icall: System.Enum::InternalBoxEnum
static RtResultVoid internal_box_enum_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                              interp::RtStackObject* ret) noexcept
{
    vm::RtReflectionRuntimeType* runtime_type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    uint64_t value = EvalStackOp::get_param<uint64_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, boxed_enum, SystemEnum::internal_box_enum(runtime_type, value));
    EvalStackOp::set_return(ret, boxed_enum);
    RET_VOID_OK();
}

/// @icall: System.Enum::get_value
static RtResultVoid enum_get_value_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                           interp::RtStackObject* ret) noexcept
{
    vm::RtObject* obj = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, value, SystemEnum::get_value(obj));
    EvalStackOp::set_return(ret, value);
    RET_VOID_OK();
}

/// @icall: System.Enum::InternalHasFlag
static RtResultVoid internal_has_flag_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                              interp::RtStackObject* ret) noexcept
{
    vm::RtObject* obj = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    vm::RtObject* flag = EvalStackOp::get_param<vm::RtObject*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemEnum::internal_has_flag(obj, flag));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

/// @icall: System.Enum::get_hashcode
static RtResultVoid enum_get_hash_code_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                               interp::RtStackObject* ret) noexcept
{
    vm::RtObject* obj = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, hash_code, SystemEnum::get_hash_code(obj));
    EvalStackOp::set_return(ret, hash_code);
    RET_VOID_OK();
}

// ========== Registration ==========

static vm::InternalCallEntry s_internal_call_entries_system_enum[] = {
    {"System.Enum::InternalCompareTo", (vm::InternalCallFunction)&SystemEnum::internal_compare_to, internal_compare_to_invoker},
    {"System.Enum::InternalGetUnderlyingType", (vm::InternalCallFunction)&SystemEnum::internal_get_underlying_type, internal_get_underlying_type_invoker},
    {"System.Enum::GetEnumValuesAndNames", (vm::InternalCallFunction)&SystemEnum::get_enum_values_and_names, get_enum_values_and_names_invoker},
    {"System.Enum::InternalBoxEnum", (vm::InternalCallFunction)&SystemEnum::internal_box_enum, internal_box_enum_invoker},
    {"System.Enum::get_value", (vm::InternalCallFunction)&SystemEnum::get_value, enum_get_value_invoker},
    {"System.Enum::InternalHasFlag", (vm::InternalCallFunction)&SystemEnum::internal_has_flag, internal_has_flag_invoker},
    {"System.Enum::get_hashcode", (vm::InternalCallFunction)&SystemEnum::get_hash_code, enum_get_hash_code_invoker},
};

utils::Span<vm::InternalCallEntry> SystemEnum::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_enum,
                                              sizeof(s_internal_call_entries_system_enum) / sizeof(s_internal_call_entries_system_enum[0]));
}

} // namespace icalls
} // namespace leanclr
