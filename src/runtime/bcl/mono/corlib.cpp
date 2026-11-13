#include "managed_layouts.h"
#include "bcl/adapter.h"
#include "vm/class.h"
#include "vm/assembly.h"
#include "metadata/module_def.h"

namespace leanclr
{
namespace vm
{
extern RtResult<metadata::RtClass*> get_class_must_exist(metadata::RtModuleDef*, const char*);

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
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_file_not_found_exception, get_class_must_exist(corlib, "System.IO.FileNotFoundException"));
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
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_field, get_class_must_exist(corlib, "System.Reflection.RuntimeFieldInfo"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_method, get_class_must_exist(corlib, "System.Reflection.RuntimeMethodInfo"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_constructor, get_class_must_exist(corlib, "System.Reflection.RuntimeConstructorInfo"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_property, get_class_must_exist(corlib, "System.Reflection.RuntimePropertyInfo"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_event, get_class_must_exist(corlib, "System.Reflection.RuntimeEventInfo"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_parameter, get_class_must_exist(corlib, "System.Reflection.RuntimeParameterInfo"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_memberinfo, get_class_must_exist(corlib, "System.Reflection.MemberInfo"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_methodbody, get_class_must_exist(corlib, "System.Reflection.MethodBody"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_exceptionhandlingclause, get_class_must_exist(corlib, "System.Reflection.ExceptionHandlingClause"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_localvariableinfo, get_class_must_exist(corlib, "System.Reflection.LocalVariableInfo"));

    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_appdomain, get_class_must_exist(corlib, "System.AppDomain"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_appdomain_setup, get_class_must_exist(corlib, "System.AppDomainSetup"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_appcontext, get_class_must_exist(corlib, "System.Runtime.Remoting.Contexts.Context"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_thread, get_class_must_exist(corlib, "System.Threading.Thread"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_internal_thread, get_class_must_exist(corlib, "System.Threading.InternalThread"));

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

RtResultVoid Class::verify_integrity_of_corlib_classes()
{
    const CorLibTypes& t = get_corlib_types();
    metadata::RtClass* const corlib_type_arr[] = {
        t.cls_void,
        t.cls_boolean,
        t.cls_char,
        t.cls_byte,
        t.cls_sbyte,
        t.cls_int16,
        t.cls_uint16,
        t.cls_int32,
        t.cls_uint32,
        t.cls_int64,
        t.cls_uint64,
        t.cls_intptr,
        t.cls_uintptr,
        t.cls_single,
        t.cls_double,
        t.cls_object,
        t.cls_valuetype,
        t.cls_string,
        t.cls_enum,
        t.cls_array,
        t.cls_delegate,
        t.cls_multicastdelegate,
        // t.cls_delegatedata,
        t.cls_typedreference,
        t.cls_systemtype,
        t.cls_runtimetype,
        t.cls_nullable,
        t.cls_icollection,
        t.cls_ienumerable,
        t.cls_ilist,
        t.cls_ienumerator,
        t.cls_ilist_generic,
        t.cls_icollection_generic,
        t.cls_ienumerable_generic,
        t.cls_ireadonlylist_generic,
        t.cls_ireadonlycollection_generic,
        t.cls_ienumerator_generic,
        t.cls_exception,
        t.cls_arithmetic_exception,
        t.cls_division_by_zero_exception,
        t.cls_execution_engine_exception,
        t.cls_overflow_exception,
        t.cls_stack_overflow_exception,
        t.cls_argument_exception,
        t.cls_argument_null_exception,
        t.cls_argument_out_of_range_exception,
        t.cls_type_load_exception,
        t.cls_index_out_of_range_exception,
        t.cls_invalid_cast_exception,
        t.cls_missing_field_exception,
        t.cls_missing_method_exception,
        t.cls_null_reference_exception,
        t.cls_array_type_mismatch_exception,
        t.cls_out_of_memory_exception,
        t.cls_bad_image_format_exception,
        t.cls_file_not_found_exception,
        t.cls_entry_point_not_found_exception,
        t.cls_missing_member_exception,
        t.cls_not_supported_exception,
        t.cls_not_implemented_exception,
        // t.cls_type_unloaded_exception,
        t.cls_type_initialization_exception,
        t.cls_target_exception,
        t.cls_target_invocation_exception,
        t.cls_target_parameter_count_exception,
        t.cls_attribute,
        t.cls_customattributedata,
        t.cls_customattribute_typed_argument,
        t.cls_customattribute_named_argument,
        t.cls_intrinsic,
        t.cls_reflection_assembly,
        t.cls_reflection_module,
        t.cls_reflection_field,
        t.cls_reflection_method,
        t.cls_reflection_constructor,
        t.cls_reflection_property,
        t.cls_reflection_event,
        t.cls_reflection_parameter,
        t.cls_reflection_memberinfo,
        t.cls_reflection_methodbody,
        t.cls_reflection_exceptionhandlingclause,
        t.cls_reflection_localvariableinfo,
        t.cls_appdomain,
        t.cls_appdomain_setup,
        t.cls_appcontext,
        t.cls_thread,
        t.cls_internal_thread,
        t.cls_marshal_as,
        t.cls_byreflike,
        t.cls_culturedata,
        t.cls_cultureinfo,
        t.cls_datetimeformatinfo,
        t.cls_numberformatinfo,
        t.cls_regioninfo,
        t.cls_calendardata,
        t.cls_stackframe,
    };

    for (metadata::RtClass* cls : corlib_type_arr)
    {
        RET_ERR_ON_FAIL(initialize_all(cls));
    }

    RET_ERR_ON_FALSE(sizeof(RtObject) == RT_OBJECT_HEADER_SIZE, RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(RT_TYPED_REFERENCE_SIZE == PTR_SIZE * 3, RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(get_instance_size_without_object_header(t.cls_typedreference) == RT_TYPED_REFERENCE_SIZE, RtErr::BadImageFormat);

    // RET_ERR_ON_FALSE(Class::get_instance_size_with_object_header(t.cls_delegatedata) == sizeof(RtDelegateData), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(Class::get_instance_size_with_object_header(t.cls_delegate) == sizeof(RtDelegate), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(Class::get_instance_size_with_object_header(t.cls_multicastdelegate) == sizeof(RtMulticastDelegate), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(Class::get_instance_size_with_object_header(t.cls_exception) == sizeof(RtException), RtErr::BadImageFormat);

    RET_ERR_ON_FALSE(Class::get_instance_size_with_object_header(t.cls_reflection_assembly) == sizeof(RtReflectionAssembly), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(Class::get_instance_size_with_object_header(t.cls_reflection_module) == sizeof(RtReflectionModule), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(Class::get_instance_size_with_object_header(t.cls_reflection_field) == sizeof(RtReflectionField), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(Class::get_instance_size_with_object_header(t.cls_reflection_method) == sizeof(RtReflectionMethod), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(Class::get_instance_size_with_object_header(t.cls_reflection_constructor) == sizeof(RtReflectionConstructor), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(Class::get_instance_size_with_object_header(t.cls_reflection_property) == sizeof(RtReflectionProperty), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(Class::get_instance_size_with_object_header(t.cls_reflection_event) == sizeof(RtReflectionEventInfo), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(Class::get_instance_size_with_object_header(t.cls_reflection_parameter) == sizeof(RtReflectionParameter), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(Class::get_instance_size_with_object_header(t.cls_reflection_methodbody) == sizeof(RtReflectionMethodBody), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(Class::get_instance_size_with_object_header(t.cls_reflection_exceptionhandlingclause) == sizeof(RtReflectionExceptionHandlingClause),
                     RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(Class::get_instance_size_with_object_header(t.cls_reflection_localvariableinfo) == sizeof(RtReflectionLocalVariableInfo),
                     RtErr::BadImageFormat);

    RET_ERR_ON_FALSE(Class::get_instance_size_with_object_header(t.cls_appdomain) == sizeof(RtAppDomain), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(Class::get_instance_size_with_object_header(t.cls_appcontext) == sizeof(RtAppContext), RtErr::BadImageFormat);

    RET_ERR_ON_FALSE(Class::get_instance_size_with_object_header(t.cls_cultureinfo) == sizeof(RtCultureInfo), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(Class::get_instance_size_with_object_header(t.cls_culturedata) == sizeof(RtCultureData), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(Class::get_instance_size_with_object_header(t.cls_datetimeformatinfo) == sizeof(RtDateTimeFormatInfo), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(Class::get_instance_size_with_object_header(t.cls_numberformatinfo) == sizeof(RtNumberFormatInfo), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(Class::get_instance_size_with_object_header(t.cls_regioninfo) == sizeof(RtRegionInfo), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(Class::get_instance_size_with_object_header(t.cls_calendardata) == sizeof(RtCalendarData), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(Class::get_instance_size_with_object_header(t.cls_stackframe) == sizeof(RtStackFrame), RtErr::BadImageFormat);

    RET_VOID_OK();
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
