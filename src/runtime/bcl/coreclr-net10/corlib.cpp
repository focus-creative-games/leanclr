#include "managed_layouts.h"
#include "const_strs.h"
#include "bcl/adapter.h"
#include "vm/class.h"
#include "vm/field.h"
#include "host/services.h"
#include "vm/assembly.h"
#include "metadata/module_def.h"

namespace leanclr
{
namespace vm
{
extern RtResult<metadata::RtClass*> get_class_must_exist(metadata::RtModuleDef*, const char*);

RtResult<metadata::RtClass*> get_class_optional(metadata::RtModuleDef* corlib, const char* full_name)
{
    return corlib->get_class_by_name(full_name, false, false);
}

RtResultVoid Class::init_corlib_classes(metadata::RtModuleDef* corlib)
{
    CorLibTypes& t = g_corlibTypes;

    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_object, get_class_must_exist(corlib, "System.Object"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_void, get_class_must_exist(corlib, "System.Void"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_string, get_class_must_exist(corlib, "System.String"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_valuetype, get_class_must_exist(corlib, "System.ValueType"));

    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_boolean, get_class_must_exist(corlib, "System.Boolean"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_char, get_class_must_exist(corlib, "System.Char"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_sbyte, get_class_must_exist(corlib, "System.SByte"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_byte, get_class_must_exist(corlib, "System.Byte"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_int16, get_class_must_exist(corlib, "System.Int16"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_uint16, get_class_must_exist(corlib, "System.UInt16"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_int32, get_class_must_exist(corlib, "System.Int32"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_uint32, get_class_must_exist(corlib, "System.UInt32"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_int64, get_class_must_exist(corlib, "System.Int64"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_uint64, get_class_must_exist(corlib, "System.UInt64"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_intptr, get_class_must_exist(corlib, "System.IntPtr"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_uintptr, get_class_must_exist(corlib, "System.UIntPtr"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_single, get_class_must_exist(corlib, "System.Single"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_double, get_class_must_exist(corlib, "System.Double"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_typedreference, get_class_must_exist(corlib, "System.TypedReference"));

    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_enum, get_class_must_exist(corlib, "System.Enum"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_nullable, get_class_must_exist(corlib, "System.Nullable`1"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_array, get_class_must_exist(corlib, "System.Array"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_delegate, get_class_must_exist(corlib, "System.Delegate"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_multicastdelegate, get_class_must_exist(corlib, "System.MulticastDelegate"));
    // UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_delegatedata, get_class_must_exist(corlib, "System.DelegateData"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_systemtype, get_class_must_exist(corlib, "System.Type"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_runtimetype, get_class_must_exist(corlib, "System.RuntimeType"));

    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_icollection, get_class_must_exist(corlib, "System.Collections.ICollection"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_ienumerable, get_class_must_exist(corlib, "System.Collections.IEnumerable"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_ilist, get_class_must_exist(corlib, "System.Collections.IList"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_ienumerator, get_class_must_exist(corlib, "System.Collections.IEnumerator"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_ilist_generic, get_class_must_exist(corlib, "System.Collections.Generic.IList`1"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_icollection_generic, get_class_must_exist(corlib, "System.Collections.Generic.ICollection`1"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_ienumerable_generic, get_class_must_exist(corlib, "System.Collections.Generic.IEnumerable`1"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_ireadonlylist_generic, get_class_must_exist(corlib, "System.Collections.Generic.IReadOnlyList`1"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_ireadonlycollection_generic, get_class_must_exist(corlib, "System.Collections.Generic.IReadOnlyCollection`1"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_ienumerator_generic, get_class_must_exist(corlib, "System.Collections.Generic.IEnumerator`1"));

    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_exception, get_class_must_exist(corlib, "System.Exception"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_arithmetic_exception, get_class_must_exist(corlib, "System.ArithmeticException"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_division_by_zero_exception, get_class_must_exist(corlib, "System.DivideByZeroException"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_execution_engine_exception, get_class_must_exist(corlib, "System.ExecutionEngineException"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_overflow_exception, get_class_must_exist(corlib, "System.OverflowException"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_stack_overflow_exception, get_class_must_exist(corlib, "System.StackOverflowException"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_argument_exception, get_class_must_exist(corlib, "System.ArgumentException"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_argument_null_exception, get_class_must_exist(corlib, "System.ArgumentNullException"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_argument_out_of_range_exception, get_class_must_exist(corlib, "System.ArgumentOutOfRangeException"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_type_load_exception, get_class_must_exist(corlib, "System.TypeLoadException"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_index_out_of_range_exception, get_class_must_exist(corlib, "System.IndexOutOfRangeException"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_invalid_cast_exception, get_class_must_exist(corlib, "System.InvalidCastException"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_missing_field_exception, get_class_must_exist(corlib, "System.MissingFieldException"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_missing_method_exception, get_class_must_exist(corlib, "System.MissingMethodException"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_null_reference_exception, get_class_must_exist(corlib, "System.NullReferenceException"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_array_type_mismatch_exception, get_class_must_exist(corlib, "System.ArrayTypeMismatchException"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_out_of_memory_exception, get_class_must_exist(corlib, "System.OutOfMemoryException"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_bad_image_format_exception, get_class_must_exist(corlib, "System.BadImageFormatException"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_entry_point_not_found_exception, get_class_must_exist(corlib, "System.EntryPointNotFoundException"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_missing_member_exception, get_class_must_exist(corlib, "System.MissingMemberException"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_not_supported_exception, get_class_must_exist(corlib, "System.NotSupportedException"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_not_implemented_exception, get_class_must_exist(corlib, "System.NotImplementedException"));
    // UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_type_unloaded_exception, get_class_must_exist(corlib, "System.TypeUnloadedException"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_type_initialization_exception, get_class_must_exist(corlib, "System.TypeInitializationException"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_target_exception, get_class_must_exist(corlib, "System.Reflection.TargetException"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_target_invocation_exception, get_class_must_exist(corlib, "System.Reflection.TargetInvocationException"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_target_parameter_count_exception, get_class_must_exist(corlib, "System.Reflection.TargetParameterCountException"));

    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_attribute, get_class_must_exist(corlib, "System.Attribute"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_customattributedata, get_class_must_exist(corlib, "System.Reflection.CustomAttributeData"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_customattribute_typed_argument, get_class_must_exist(corlib, "System.Reflection.CustomAttributeTypedArgument"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_customattribute_named_argument, get_class_must_exist(corlib, "System.Reflection.CustomAttributeNamedArgument"));

    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_intrinsic, get_class_must_exist(corlib, "System.Runtime.CompilerServices.IntrinsicAttribute"));

    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_assembly, get_class_must_exist(corlib, "System.Reflection.RuntimeAssembly"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_module, get_class_must_exist(corlib, "System.Reflection.RuntimeModule"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_field, get_class_optional(corlib, "System.Reflection.RtFieldInfo"));
    if (t.cls_reflection_field == nullptr)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_field, get_class_must_exist(corlib, "System.Reflection.RuntimeFieldInfo"));
    }
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_method, get_class_must_exist(corlib, "System.Reflection.RuntimeMethodInfo"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_constructor, get_class_must_exist(corlib, "System.Reflection.RuntimeConstructorInfo"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_property, get_class_must_exist(corlib, "System.Reflection.RuntimePropertyInfo"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_event, get_class_must_exist(corlib, "System.Reflection.RuntimeEventInfo"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_parameter, get_class_must_exist(corlib, "System.Reflection.RuntimeParameterInfo"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_memberinfo, get_class_must_exist(corlib, "System.Reflection.MemberInfo"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_methodbody, get_class_optional(corlib, "System.Reflection.RuntimeMethodBody"));
    if (t.cls_reflection_methodbody == nullptr)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_methodbody, get_class_must_exist(corlib, "System.Reflection.MethodBody"));
    }
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_exceptionhandlingclause, get_class_optional(corlib, "System.Reflection.RuntimeExceptionHandlingClause"));
    if (t.cls_reflection_exceptionhandlingclause == nullptr)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_exceptionhandlingclause, get_class_must_exist(corlib, "System.Reflection.ExceptionHandlingClause"));
    }
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_localvariableinfo, get_class_optional(corlib, "System.Reflection.RuntimeLocalVariableInfo"));
    if (t.cls_reflection_localvariableinfo == nullptr)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_localvariableinfo, get_class_must_exist(corlib, "System.Reflection.LocalVariableInfo"));
    }

    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_appdomain, get_class_must_exist(corlib, "System.AppDomain"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_appdomain_setup, get_class_must_exist(corlib, "System.AppDomainSetup"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_appcontext, get_class_optional(corlib, "System.Runtime.Remoting.Contexts.Context"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_thread, get_class_must_exist(corlib, "System.Threading.Thread"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_internal_thread, get_class_optional(corlib, "System.Threading.InternalThread"));

    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_marshal_as, get_class_must_exist(corlib, "System.Runtime.InteropServices.MarshalAsAttribute"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_byreflike, get_class_must_exist(corlib, "System.Runtime.CompilerServices.IsByRefLikeAttribute"));

    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_culturedata, get_class_must_exist(corlib, "System.Globalization.CultureData"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_cultureinfo, get_class_must_exist(corlib, "System.Globalization.CultureInfo"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_datetimeformatinfo, get_class_must_exist(corlib, "System.Globalization.DateTimeFormatInfo"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_numberformatinfo, get_class_must_exist(corlib, "System.Globalization.NumberFormatInfo"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_regioninfo, get_class_must_exist(corlib, "System.Globalization.RegionInfo"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_calendardata, get_class_must_exist(corlib, "System.Globalization.CalendarData"));

    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_stackframe, get_class_must_exist(corlib, "System.Diagnostics.StackFrame"));

    RET_VOID_OK();
}

template <typename T, typename F>
static size_t native_field_offset(F T::* member)
{
    T value{};
    return static_cast<size_t>(reinterpret_cast<const char*>(&(value.*member)) - reinterpret_cast<const char*>(&value));
}

static RtResultVoid verify_field_layout(metadata::RtClass* klass, const char* name, size_t expected, const char* expected_type = nullptr, bool by_ref = false)
{
    RET_ERR_ON_FAIL(Class::initialize_fields(klass));
    const auto* field = Class::get_field_for_name(klass, name, true);
    if (!field || Field::get_field_offset_includes_object_header_for_all_type(field) != expected)
    {
        host::Services::logf(host::LogLevel::Error, "CoreLib layout mismatch: %s.%s::%s", klass->namespaze, klass->name, name);
        RET_ERR(RtErr::BadImageFormat);
    }
    if (expected_type)
    {
        auto type_sig = field->type_sig->to_canonized_without_byref();
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, actual, Class::get_class_from_typesig(&type_sig));
        const size_t length = std::strlen(expected_type);
        const bool array = length >= 2 && std::strcmp(expected_type + length - 2, "[]") == 0;
        char element_name[128];
        if (array)
        {
            if (length - 2 >= sizeof(element_name) || !Class::is_array_or_szarray(actual))
                RET_ERR(RtErr::BadImageFormat);
            std::memcpy(element_name, expected_type, length - 2);
            element_name[length - 2] = 0;
            expected_type = element_name;
            actual = const_cast<metadata::RtClass*>(actual->element_class);
        }
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, expected_class,
                                                metadata::RtModuleDef::get_corlib_module()->get_class_by_name(expected_type, false, true));
        RET_ERR_ON_FALSE(actual == expected_class && field->type_sig->by_ref == by_ref, RtErr::BadImageFormat);
    }
    RET_VOID_OK();
}

RtResultVoid Class::verify_integrity_of_corlib_classes()
{
    const CorLibTypes& t = get_corlib_types();
    metadata::RtModuleDef* corlib = metadata::RtModuleDef::get_corlib_module();
    if (corlib != nullptr && std::strcmp(corlib->get_name_no_ext(), STR_SYSTEM_PRIVATE_CORELIB_NAME) == 0)
    {
        RET_ERR_ON_FALSE(sizeof(RtObject) == RT_OBJECT_HEADER_SIZE, RtErr::BadImageFormat);
        // Validate native views before any adapter writes managed fields.
        // Reflection types with dynamic layouts use field metadata at access time.
        RET_ERR_ON_FAIL(
            verify_field_layout(t.cls_typedreference, "_value", RT_OBJECT_HEADER_SIZE + native_field_offset(&RtTypedReference::value), "System.Byte", true));
        RET_ERR_ON_FAIL(
            verify_field_layout(t.cls_typedreference, "_type", RT_OBJECT_HEADER_SIZE + native_field_offset(&RtTypedReference::type_handle), "System.IntPtr"));
        RET_ERR_ON_FALSE(get_instance_size_without_object_header(t.cls_typedreference) == sizeof(RtTypedReference), RtErr::BadImageFormat);
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_exception, "_message", native_field_offset(&RtException::message), "System.String"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_exception, "_innerException", native_field_offset(&RtException::inner_exception), "System.Exception"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_exception, "_stackTrace", native_field_offset(&RtException::trace_ips), "System.Object"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_runtimetype, "m_keepalive", native_field_offset(&RtReflectionType::keepalive), "System.Object"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_runtimetype, "m_cache", native_field_offset(&RtReflectionType::cache), "System.IntPtr"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_runtimetype, "m_handle", native_field_offset(&RtReflectionType::type_handle), "System.IntPtr"));
        RET_ERR_ON_FAIL(initialize_fields(t.cls_runtimetype));
        RET_ERR_ON_FALSE(get_instance_size_with_object_header(t.cls_runtimetype) == sizeof(RtReflectionType), RtErr::BadImageFormat);
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_delegate, "_target", native_field_offset(&RtDelegate::target), "System.Object"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_delegate, "_methodBase", native_field_offset(&RtDelegate::method_base), "System.Object"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_delegate, "_methodPtr", native_field_offset(&RtDelegate::method_ptr), "System.IntPtr"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_delegate, "_methodPtrAux", native_field_offset(&RtDelegate::method_ptr_aux), "System.IntPtr"));
        RET_ERR_ON_FAIL(initialize_fields(t.cls_delegate));
        RET_ERR_ON_FALSE(get_instance_size_with_object_header(t.cls_delegate) == sizeof(RtDelegate), RtErr::BadImageFormat);
        RET_ERR_ON_FAIL(
            verify_field_layout(t.cls_multicastdelegate, "_invocationList", native_field_offset(&RtMulticastDelegate::invocation_list), "System.Object"));
        RET_ERR_ON_FAIL(
            verify_field_layout(t.cls_multicastdelegate, "_invocationCount", native_field_offset(&RtMulticastDelegate::invocation_count), "System.IntPtr"));
        RET_ERR_ON_FAIL(initialize_fields(t.cls_multicastdelegate));
        RET_ERR_ON_FALSE(get_instance_size_with_object_header(t.cls_multicastdelegate) == sizeof(RtMulticastDelegate), RtErr::BadImageFormat);
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_reflection_method, "m_handle", native_field_offset(&RtReflectionMethod::method), "System.IntPtr"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_reflection_method, "m_name", native_field_offset(&RtReflectionMethod::name), "System.String"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_reflection_method, "m_toString", native_field_offset(&RtReflectionMethod::to_string), "System.String"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_reflection_method, "m_bindingFlags", native_field_offset(&RtReflectionMethod::binding_flags),
                                            "System.Reflection.BindingFlags"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_reflection_method, "m_methodAttributes", native_field_offset(&RtReflectionMethod::method_attributes),
                                            "System.Reflection.MethodAttributes"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_reflection_method, "m_signature", native_field_offset(&RtReflectionMethod::signature), "System.Signature"));
        RET_ERR_ON_FAIL(
            verify_field_layout(t.cls_reflection_method, "m_declaringType", native_field_offset(&RtReflectionMethod::declaring_type), "System.RuntimeType"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_reflection_method, "m_keepalive", native_field_offset(&RtReflectionMethod::keepalive), "System.Object"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_reflection_method, "m_invoker", native_field_offset(&RtReflectionMethod::invoker),
                                            "System.Reflection.MethodBaseInvoker"));
        RET_ERR_ON_FAIL(initialize_fields(t.cls_reflection_method));
        RET_ERR_ON_FALSE(get_instance_size_with_object_header(t.cls_reflection_method) == sizeof(RtReflectionMethod), RtErr::BadImageFormat);
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_reflection_constructor, "m_declaringType", native_field_offset(&RtReflectionConstructor::declaring_type),
                                            "System.RuntimeType"));
        RET_ERR_ON_FAIL(
            verify_field_layout(t.cls_reflection_constructor, "m_toString", native_field_offset(&RtReflectionConstructor::to_string), "System.String"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_reflection_constructor, "_empty1", native_field_offset(&RtReflectionConstructor::empty1), "System.Object"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_reflection_constructor, "_empty2", native_field_offset(&RtReflectionConstructor::empty2), "System.Object"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_reflection_constructor, "_empty3", native_field_offset(&RtReflectionConstructor::empty3), "System.Object"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_reflection_constructor, "m_handle", native_field_offset(&RtReflectionConstructor::method), "System.IntPtr"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_reflection_constructor, "m_bindingFlags", native_field_offset(&RtReflectionConstructor::binding_flags),
                                            "System.Reflection.BindingFlags"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_reflection_constructor, "m_methodAttributes",
                                            native_field_offset(&RtReflectionConstructor::method_attributes), "System.Reflection.MethodAttributes"));
        RET_ERR_ON_FAIL(
            verify_field_layout(t.cls_reflection_constructor, "m_signature", native_field_offset(&RtReflectionConstructor::signature), "System.Signature"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_reflection_constructor, "m_invoker", native_field_offset(&RtReflectionConstructor::invoker),
                                            "System.Reflection.MethodBaseInvoker"));
        RET_ERR_ON_FAIL(initialize_fields(t.cls_reflection_constructor));
        RET_ERR_ON_FALSE(get_instance_size_with_object_header(t.cls_reflection_constructor) == sizeof(RtReflectionConstructor), RtErr::BadImageFormat);
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_thread, "_executionContext", native_field_offset(&RtThread::execution_context)));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_thread, "_synchronizationContext", native_field_offset(&RtThread::synchronization_context)));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_thread, "_name", native_field_offset(&RtThread::name), "System.String"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_thread, "_startHelper", native_field_offset(&RtThread::start_helper)));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_thread, "_DONT_USE_InternalThread", native_field_offset(&RtThread::native_thread), "System.IntPtr"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_thread, "_priority", native_field_offset(&RtThread::priority), "System.Int32"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_thread, "_managedThreadId", native_field_offset(&RtThread::managed_thread_id), "System.Int32"));
        RET_ERR_ON_FAIL(
            verify_field_layout(t.cls_thread, "_mayNeedResetForThreadPool", native_field_offset(&RtThread::may_need_reset_for_thread_pool), "System.Boolean"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_thread, "_isDead", native_field_offset(&RtThread::is_dead), "System.Boolean"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_thread, "_isThreadPool", native_field_offset(&RtThread::is_thread_pool), "System.Boolean"));
        RET_ERR_ON_FALSE(get_instance_size_with_object_header(t.cls_thread) == sizeof(RtThread), RtErr::BadImageFormat);
        RET_ERR_ON_FAIL(
            verify_field_layout(t.cls_reflection_module, "m_runtimeType", native_field_offset(&RtReflectionModule::runtime_type), "System.RuntimeType"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_reflection_module, "m_runtimeAssembly", native_field_offset(&RtReflectionModule::assembly),
                                            "System.Reflection.RuntimeAssembly"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_reflection_module, "m_pData", native_field_offset(&RtReflectionModule::native_handle), "System.IntPtr"));
        RET_ERR_ON_FALSE(get_instance_size_with_object_header(t.cls_reflection_module) == sizeof(RtReflectionModule), RtErr::BadImageFormat);
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_reflection_assembly, "_ModuleResolve", native_field_offset(&RtReflectionAssembly::module_resolve),
                                            "System.Reflection.ModuleResolveEventHandler"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_reflection_assembly, "m_fullname", native_field_offset(&RtReflectionAssembly::full_name), "System.String"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_reflection_assembly, "m_syncRoot", native_field_offset(&RtReflectionAssembly::sync_root), "System.Object"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_reflection_assembly, "m_assembly", native_field_offset(&RtReflectionAssembly::assembly), "System.IntPtr"));
        RET_ERR_ON_FALSE(get_instance_size_with_object_header(t.cls_reflection_assembly) == sizeof(RtReflectionAssembly), RtErr::BadImageFormat);
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_reflection_parameter, "AttrsImpl", native_field_offset(&RtReflectionParameter::attrs),
                                            "System.Reflection.ParameterAttributes"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_reflection_parameter, "ClassImpl", native_field_offset(&RtReflectionParameter::parent_type), "System.Type"));
        RET_ERR_ON_FAIL(
            verify_field_layout(t.cls_reflection_parameter, "DefaultValueImpl", native_field_offset(&RtReflectionParameter::default_value), "System.Object"));
        RET_ERR_ON_FAIL(
            verify_field_layout(t.cls_reflection_parameter, "MemberImpl", native_field_offset(&RtReflectionParameter::member), "System.Reflection.MemberInfo"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_reflection_parameter, "NameImpl", native_field_offset(&RtReflectionParameter::name), "System.String"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_reflection_parameter, "PositionImpl", native_field_offset(&RtReflectionParameter::index), "System.Int32"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_reflection_method, "m_reflectedTypeCache", native_field_offset(&RtReflectionMethod::reflected_type_cache)));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_reflection_method, "m_parameters", native_field_offset(&RtReflectionMethod::parameters),
                                            "System.Reflection.ParameterInfo[]"));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_reflection_method, "m_returnParameter", native_field_offset(&RtReflectionMethod::return_parameter),
                                            "System.Reflection.ParameterInfo"));
        RET_ERR_ON_FAIL(
            verify_field_layout(t.cls_reflection_constructor, "m_reflectedTypeCache", native_field_offset(&RtReflectionConstructor::reflected_type_cache)));
        RET_ERR_ON_FAIL(verify_field_layout(t.cls_reflection_constructor, "m_parameters", native_field_offset(&RtReflectionConstructor::parameters),
                                            "System.Reflection.ParameterInfo[]"));
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, signature, get_class_must_exist(corlib, "System.Signature"));
        RET_ERR_ON_FAIL(verify_field_layout(signature, "_arguments", native_field_offset(&RtSignature::arguments), "System.RuntimeType[]"));
        RET_ERR_ON_FAIL(verify_field_layout(signature, "_declaringType", native_field_offset(&RtSignature::declaring_type), "System.RuntimeType"));
        RET_ERR_ON_FAIL(
            verify_field_layout(signature, "_returnTypeORfieldType", native_field_offset(&RtSignature::return_type_or_field_type), "System.RuntimeType"));
        RET_ERR_ON_FAIL(verify_field_layout(signature, "_keepAlive", native_field_offset(&RtSignature::keep_alive), "System.Object"));
        RET_ERR_ON_FAIL(verify_field_layout(signature, "_sig", native_field_offset(&RtSignature::sig)));
        RET_ERR_ON_FAIL(verify_field_layout(signature, "_csig", native_field_offset(&RtSignature::csig), "System.Int32"));
        RET_ERR_ON_FAIL(verify_field_layout(signature, "_managedCallingConventionAndArgIteratorFlags",
                                            native_field_offset(&RtSignature::managed_calling_convention_and_arg_iterator_flags), "System.Int32"));
        RET_ERR_ON_FAIL(verify_field_layout(signature, "_nSizeOfArgStack", native_field_offset(&RtSignature::size_of_arg_stack), "System.Int32"));
        RET_ERR_ON_FAIL(verify_field_layout(signature, "_pMethod", native_field_offset(&RtSignature::method), "System.RuntimeMethodHandleInternal"));
        RET_ERR_ON_FALSE(get_instance_size_with_object_header(signature) == sizeof(RtSignature), RtErr::BadImageFormat);
        RET_VOID_OK();
    }

    RET_ERR(RtErr::BadImageFormat);
}
} // namespace vm

namespace bcl
{
RtResultVoid Adapter::initialize_corlib(metadata::RtModuleDef* corlib)
{
    return vm::Class::init_corlib_classes(corlib);
}

RtResultVoid Adapter::validate_managed_layouts()
{
    return vm::Class::verify_integrity_of_corlib_classes();
}
} // namespace bcl
} // namespace leanclr
