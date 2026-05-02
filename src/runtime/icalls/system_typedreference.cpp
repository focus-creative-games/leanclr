#include "system_typedreference.h"

#include "vm/rt_array.h"
#include "vm/class.h"
#include "vm/field.h"
#include "vm/object.h"
#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace icalls
{

// TypedReference operations
RtResultVoid SystemTypedReference::internal_make_typed_reference(vm::RtTypedReference* result, vm::RtObject* target, vm::RtArray* fields,
                                                                 vm::RtReflectionType* last_field_type) noexcept
{
    (void)last_field_type;

    auto field_data_ptr = vm::Array::get_array_data_start_as<const metadata::RtFieldInfo*>(fields);
    uint8_t* value = nullptr;
    const metadata::RtTypeSig* field_type = nullptr;

    for (int32_t i = 0; i < fields->length; ++i)
    {
        const metadata::RtFieldInfo* field_info = field_data_ptr[i];
        if (field_info == nullptr)
        {
            RET_ERR(RtErr::ArgumentNull);
        }

        if (i == 0)
        {
            value = reinterpret_cast<uint8_t*>(target) + vm::Field::get_instance_field_offset_includes_object_header_for_all_type(field_info);
        }
        else
        {
            value = value + vm::Field::get_field_offset_excludes_object_header_for_all_type(field_info);
        }
        field_type = field_info->type_sig;
    }

    result->type_handle = field_type;
    UNWRAP_OR_RET_ERR_ON_FAIL(result->klass, vm::Class::get_class_from_typesig(field_type));
    result->value = value;

    RET_VOID_OK();
}

RtResult<vm::RtObject*> SystemTypedReference::internal_to_object(const vm::RtTypedReference* typed_ref) noexcept
{
    if (vm::Class::is_reference_type(typed_ref->klass))
    {
        // For reference types, the value is already a pointer to the object
        RET_OK(*reinterpret_cast<vm::RtObject* const*>(typed_ref->value));
    }
    else
    {
        // For value types, we need to box the value
        return vm::Object::box_object(typed_ref->klass, typed_ref->value);
    }
}

/// @icall: System.TypedReference::InternalMakeTypedReference
static RtResultVoid invoker_internal_make_typed_reference(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                                          const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)method_pointer;
    (void)method;
    (void)ret;

    auto result = EvalStackOp::get_param<vm::RtTypedReference*>(params, 0);
    auto target = EvalStackOp::get_param<vm::RtObject*>(params, 1);
    auto fields = EvalStackOp::get_param<vm::RtArray*>(params, 2);
    auto last_field_type = EvalStackOp::get_param<vm::RtReflectionType*>(params, 3);

    return SystemTypedReference::internal_make_typed_reference(result, target, fields, last_field_type);
}

/// @icall: System.TypedReference::InternalToObject
static RtResultVoid invoker_internal_to_object(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)method_pointer;
    (void)method;

    auto typed_ref = EvalStackOp::get_param<const vm::RtTypedReference*>(params, 0);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, obj, SystemTypedReference::internal_to_object(typed_ref));
    EvalStackOp::set_return(ret, obj);

    RET_VOID_OK();
}

// Internal call entries
static vm::InternalCallEntry s_internal_call_entries_system_typedreference[] = {
    {"System.TypedReference::InternalMakeTypedReference", (vm::InternalCallFunction)&SystemTypedReference::internal_make_typed_reference,
     invoker_internal_make_typed_reference},
    {"System.TypedReference::InternalToObject", (vm::InternalCallFunction)&SystemTypedReference::internal_to_object, invoker_internal_to_object},
};

utils::Span<vm::InternalCallEntry> SystemTypedReference::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_typedreference,
                                              sizeof(s_internal_call_entries_system_typedreference) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr
