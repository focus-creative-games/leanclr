#include <functional>

#include "class.h"
#include "const_strs.h"
#include "rt_managed_types.h"
#include "metadata/module_def.h"
#include "metadata/metadata_compare.h"
#include "metadata/metadata_hash.h"
#include "metadata/metadata_cache.h"
#include "alloc/metadata_allocation.h"
#include "utils/hashmap.h"
#include "utils/hashset.h"
#include "gc/garbage_collector.h"
#include "array_class.h"
#include "generic_class.h"
#include "field.h"
#include "metadata/layout.h"
#include "method.h"
#include "shim.h"
#include "customattribute.h"
#include "assembly.h"

namespace leanclr
{
namespace vm
{

RtResult<metadata::RtClass*> get_class_must_exist(metadata::RtModuleDef* corlib, const char* full_name)
{
    return corlib->get_class_by_name(full_name, false, true);
}

static CorLibTypes g_corlibTypes{};

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
#if UNITY_VERSION >= 20210000
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_field, get_class_must_exist(corlib, "System.Reflection.RuntimeFieldInfo"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_method, get_class_must_exist(corlib, "System.Reflection.RuntimeMethodInfo"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_constructor, get_class_must_exist(corlib, "System.Reflection.RuntimeConstructorInfo"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_property, get_class_must_exist(corlib, "System.Reflection.RuntimePropertyInfo"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_event, get_class_must_exist(corlib, "System.Reflection.RuntimeEventInfo"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_parameter, get_class_must_exist(corlib, "System.Reflection.RuntimeParameterInfo"));
#else
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_field, get_class_must_exist(corlib, "System.Reflection.MonoField"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_method, get_class_must_exist(corlib, "System.Reflection.MonoMethod"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_constructor, get_class_must_exist(corlib, "System.Reflection.MonoCMethod"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_property, get_class_must_exist(corlib, "System.Reflection.MonoProperty"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_event, get_class_must_exist(corlib, "System.Reflection.MonoEvent"));
    UNWRAP_OR_RET_ERR_ON_FAIL(t.cls_reflection_parameter, get_class_must_exist(corlib, "System.Reflection.MonoParameterInfo"));
#endif
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

const CorLibTypes& Class::get_corlib_types()
{
    return g_corlibTypes;
}

RtResult<metadata::RtClass*> Class::get_class_by_type_def_gid(uint32_t gid)
{
    metadata::RtModuleDef* def_mod = metadata::RtModuleDef::get_module_by_id(metadata::RtMetadata::decode_module_id_from_gid(gid));
    return def_mod->get_class_by_type_def_rid(metadata::RtMetadata::decode_rid_from_gid(gid));
}

RtResultVoid Class::initialize()
{
    RET_ERR_ON_FAIL(init_corlib_classes(metadata::RtModuleDef::get_corlib_module()));
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

    // RET_ERR_ON_FALSE(get_instance_size_with_object_header(t.cls_delegatedata) == sizeof(RtDelegateData), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(get_instance_size_with_object_header(t.cls_delegate) == sizeof(RtDelegate), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(get_instance_size_with_object_header(t.cls_multicastdelegate) == sizeof(RtMulticastDelegate), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(get_instance_size_with_object_header(t.cls_exception) == sizeof(RtException), RtErr::BadImageFormat);

    RET_ERR_ON_FALSE(get_instance_size_with_object_header(t.cls_reflection_assembly) == sizeof(RtReflectionAssembly), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(get_instance_size_with_object_header(t.cls_reflection_module) == sizeof(RtReflectionModule), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(get_instance_size_with_object_header(t.cls_reflection_field) == sizeof(RtReflectionField), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(get_instance_size_with_object_header(t.cls_reflection_method) == sizeof(RtReflectionMethod), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(get_instance_size_with_object_header(t.cls_reflection_constructor) == sizeof(RtReflectionConstructor), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(get_instance_size_with_object_header(t.cls_reflection_property) == sizeof(RtReflectionProperty), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(get_instance_size_with_object_header(t.cls_reflection_event) == sizeof(RtReflectionEventInfo), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(get_instance_size_with_object_header(t.cls_reflection_parameter) == sizeof(RtReflectionParameter), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(get_instance_size_with_object_header(t.cls_reflection_methodbody) == sizeof(RtReflectionMethodBody), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(get_instance_size_with_object_header(t.cls_reflection_exceptionhandlingclause) == sizeof(RtReflectionExceptionHandlingClause),
                     RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(get_instance_size_with_object_header(t.cls_reflection_localvariableinfo) == sizeof(RtReflectionLocalVariableInfo), RtErr::BadImageFormat);

    RET_ERR_ON_FALSE(get_instance_size_with_object_header(t.cls_appdomain) == sizeof(RtAppDomain), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(get_instance_size_with_object_header(t.cls_appcontext) == sizeof(RtAppContext), RtErr::BadImageFormat);

    RET_ERR_ON_FALSE(get_instance_size_with_object_header(t.cls_cultureinfo) == sizeof(RtCultureInfo), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(get_instance_size_with_object_header(t.cls_culturedata) == sizeof(RtCultureData), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(get_instance_size_with_object_header(t.cls_datetimeformatinfo) == sizeof(RtDateTimeFormatInfo), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(get_instance_size_with_object_header(t.cls_numberformatinfo) == sizeof(RtNumberFormatInfo), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(get_instance_size_with_object_header(t.cls_regioninfo) == sizeof(RtRegionInfo), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(get_instance_size_with_object_header(t.cls_calendardata) == sizeof(RtCalendarData), RtErr::BadImageFormat);
    RET_ERR_ON_FALSE(get_instance_size_with_object_header(t.cls_stackframe) == sizeof(RtStackFrame), RtErr::BadImageFormat);

    RET_VOID_OK();
}

static RtResultVoid setup_class_typsig(metadata::RtModuleDef* mod, metadata::RtClass* klass)
{
    auto rid = metadata::RtToken::decode_rid(klass->token);
    UNWRAP_OR_RET_ERR_ON_FAIL(klass->by_val, mod->get_type_def_by_val_typesig(rid));
    UNWRAP_OR_RET_ERR_ON_FAIL(klass->by_ref, mod->get_type_def_by_ref_typesig(rid));
    RET_VOID_OK();
}

static void setup_cast_class(metadata::RtClass* klass)
{
    const CorLibTypes& ct = g_corlibTypes;
    switch (klass->by_val->ele_type)
    {
    case metadata::RtElementType::Boolean:
        if (ct.cls_sbyte)
            klass->cast_class = ct.cls_sbyte;
        break;
    case metadata::RtElementType::Char:
        if (ct.cls_int16)
            klass->cast_class = ct.cls_int16;
        break;
    case metadata::RtElementType::I1:
        if (ct.cls_byte)
            ct.cls_byte->cast_class = klass;
        if (ct.cls_boolean)
            ct.cls_boolean->cast_class = klass;
        break;
    case metadata::RtElementType::U1:
        if (ct.cls_sbyte)
            klass->cast_class = ct.cls_sbyte;
        break;
    case metadata::RtElementType::I2:
        if (ct.cls_uint16)
            ct.cls_uint16->cast_class = klass;
        if (ct.cls_char)
            ct.cls_char->cast_class = klass;
        break;
    case metadata::RtElementType::U2:
        if (ct.cls_int16)
            klass->cast_class = ct.cls_int16;
        break;
    case metadata::RtElementType::I4:
        if (ct.cls_uint32)
            ct.cls_uint32->cast_class = klass;
        break;
    case metadata::RtElementType::U4:
        if (ct.cls_int32)
            klass->cast_class = ct.cls_int32;
        break;
    case metadata::RtElementType::I8:
        if (ct.cls_uint64)
            ct.cls_uint64->cast_class = klass;
        break;
    case metadata::RtElementType::U8:
        if (ct.cls_int64)
            klass->cast_class = ct.cls_int64;
        break;
    case metadata::RtElementType::I:
        if (ct.cls_uintptr)
            ct.cls_uintptr->cast_class = klass;
        break;
    case metadata::RtElementType::U:
        if (ct.cls_intptr)
            klass->cast_class = ct.cls_intptr;
        break;
    default:
        break;
    }
}

// Public static member functions translating from EEClass methods
bool Class::is_value_type(const metadata::RtClass* klass)
{
    return (klass->extra_flags & (uint32_t)metadata::RtClassExtraAttribute::ValueType) != 0;
}

bool Class::is_reference_type(const metadata::RtClass* klass)
{
    return (klass->extra_flags & (uint32_t)metadata::RtClassExtraAttribute::ReferenceType) != 0;
}

bool Class::is_enum_type(const metadata::RtClass* klass)
{
    return (klass->extra_flags & (uint32_t)metadata::RtClassExtraAttribute::Enum) != 0;
}

bool Class::is_nullable_type(const metadata::RtClass* klass)
{
    return (klass->extra_flags & (uint32_t)metadata::RtClassExtraAttribute::Nullable) != 0;
}

bool Class::is_multicastdelegate_subclass(const metadata::RtClass* klass)
{
    return klass->parent == g_corlibTypes.cls_multicastdelegate;
}

bool Class::get_has_references(const metadata::RtClass* klass)
{
    return (klass->extra_flags & (uint32_t)metadata::RtClassExtraAttribute::HasReferences) != 0;
}

void Class::set_has_references(metadata::RtClass* klass)
{
    klass->extra_flags |= (uint32_t)metadata::RtClassExtraAttribute::HasReferences;
}

bool Class::is_blittable(const metadata::RtClass* klass)
{
    return (klass->extra_flags & (uint32_t)metadata::RtClassExtraAttribute::HasReferences) == 0;
}

bool Class::is_array_or_szarray(const metadata::RtClass* klass)
{
    return (klass->extra_flags & (uint32_t)metadata::RtClassExtraAttribute::ArrayOrSZArray) != 0;
}

bool Class::is_ptr(const metadata::RtClass* klass)
{
    return klass->by_val->ele_type == metadata::RtElementType::Ptr;
}

bool Class::has_static_constructor(const metadata::RtClass* klass)
{
    return (klass->extra_flags & (uint32_t)metadata::RtClassExtraAttribute::HasStaticConstructor) != 0;
}

bool Class::has_finalizer(const metadata::RtClass* klass)
{
    return (klass->extra_flags & (uint32_t)metadata::RtClassExtraAttribute::HasFinalizer) != 0;
}

bool Class::is_interface(const metadata::RtClass* klass)
{
    return (klass->flags & (uint32_t)metadata::RtTypeAttribute::Interface) != 0;
}

bool Class::is_abstract(const metadata::RtClass* klass)
{
    return (klass->flags & (uint32_t)metadata::RtTypeAttribute::Abstract) != 0;
}

bool Class::is_sealed(const metadata::RtClass* klass)
{
    return (klass->flags & (uint32_t)metadata::RtTypeAttribute::Sealed) != 0;
}

bool Class::is_generic(const metadata::RtClass* klass)
{
    return klass->generic_container != nullptr;
}

bool Class::is_generic_inst(const metadata::RtClass* klass)
{
    return klass->by_val->ele_type == metadata::RtElementType::GenericInst;
}

bool Class::is_cctor_not_finished(const metadata::RtClass* klass)
{
    return (klass->init_flags & (uint32_t)metadata::RtClassInitPart::RuntimeClassInit) == 0;
}

void Class::set_cctor_finished(metadata::RtClass* klass)
{
    klass->init_flags |= (uint32_t)metadata::RtClassInitPart::RuntimeClassInit;
}

const metadata::RtTypeSig* Class::get_by_val_type_sig(const metadata::RtClass* klass)
{
    return klass->by_val;
}

const metadata::RtTypeSig* Class::get_by_ref_type_sig(const metadata::RtClass* klass)
{
    return klass->by_ref;
}

bool Class::is_object_class(const metadata::RtClass* klass)
{
    return klass->by_val->ele_type == metadata::RtElementType::Object;
}

bool Class::is_string_class(const metadata::RtClass* klass)
{
    return klass->by_val->ele_type == metadata::RtElementType::String;
}

bool Class::is_szarray_class(const metadata::RtClass* klass)
{
    return klass->by_val->ele_type == metadata::RtElementType::SZArray;
}

uint8_t Class::get_rank(const metadata::RtClass* klass)
{
    switch (klass->by_val->ele_type)
    {
    case metadata::RtElementType::Array:
        return klass->by_val->data.array_type->rank;
    case metadata::RtElementType::SZArray:
        return 1;
    default:
        return 0;
    }
}

metadata::RtElementType Class::get_element_type(const metadata::RtClass* klass)
{
    return klass->by_val->ele_type;
}

metadata::RtElementType Class::get_enum_element_type(const metadata::RtClass* klass)
{
    assert(is_enum_type(klass));
    return get_element_type(klass->element_class);
}

bool Class::is_by_ref(const metadata::RtClass* klass)
{
    return klass->by_val->by_ref;
}

bool Class::is_public(const metadata::RtClass* klass)
{
    return (klass->flags & (uint32_t)metadata::RtTypeAttribute::Public) != 0;
}

bool Class::is_nested_public(const metadata::RtClass* klass)
{
    return (klass->flags & (uint32_t)metadata::RtTypeAttribute::NestedPublic) != 0;
}

bool Class::is_initialized(const metadata::RtClass* klass)
{
    return (klass->init_flags & (uint32_t)metadata::RtClassInitPart::All) != 0;
}

bool Class::is_explicit_layout(const metadata::RtClass* klass)
{
    return (klass->flags & (uint32_t)metadata::RtTypeAttribute::ExplicitLayout) != 0;
}

RtResult<bool> Class::is_by_ref_like(const metadata::RtClass* klass)
{
    return CustomAttribute::has_customattribute_on_class(klass, g_corlibTypes.cls_byreflike);
}

static bool is_enum_type_internal(const metadata::RtClass* klass)
{
    auto parent = klass->parent;
    return parent && strcmp(parent->name, STR_ENUM) == 0 && parent->image->is_corlib();
}

static bool is_value_typedef(const metadata::RtElementType eleType)
{
    switch (eleType)
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
    case metadata::RtElementType::ValueType:
    case metadata::RtElementType::TypedByRef:
        return true;
    default:
        return false;
    }
}

RtResult<metadata::RtClass*> Class::init_class_of_type_def(metadata::RtModuleDef* mod, uint32_t rid)
{
    metadata::RtClass* klass = mod->get_mem_pool().malloc_any_zeroed<metadata::RtClass>();
    klass->image = mod;
    klass->token = metadata::RtToken::encode(metadata::TableType::TypeDef, rid);

    const metadata::CliImage& cliImage = mod->get_cli_image();
    metadata::RowTypeDef typeDefRow = cliImage.read_type_def(rid).value();
    UNWRAP_OR_RET_ERR_ON_FAIL(klass->name, mod->get_string(typeDefRow.type_name));
    UNWRAP_OR_RET_ERR_ON_FAIL(klass->namespaze, mod->get_string(typeDefRow.type_namespace));
    klass->flags = typeDefRow.flags;
    UNWRAP_OR_RET_ERR_ON_FAIL(klass->generic_container, mod->get_generic_container(klass->token));
    if (klass->generic_container)
    {
        klass->extra_flags |= (uint32_t)metadata::RtClassExtraAttribute::Generic;
    }
    metadata::RtGenericContainerContext gcc{klass->generic_container, nullptr};
    if (typeDefRow.extends != 0)
    {
        metadata::RtToken baseTypeToken = metadata::RtMetadata::decode_type_def_ref_spec_coded_index(typeDefRow.extends);
        UNWRAP_OR_RET_ERR_ON_FAIL(klass->parent, mod->get_class_by_type_def_ref_spec_token(baseTypeToken, gcc, nullptr));
    }

    auto optEnclosingTypeDefRid = mod->get_enclosing_type_def_rid(klass->token);
    if (optEnclosingTypeDefRid)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(klass->declaring_class, mod->get_class_by_type_def_rid(optEnclosingTypeDefRid.value()));
    }

    RET_ERR_ON_FAIL(setup_class_typsig(mod, klass));
    klass->element_class = klass->cast_class = klass;

    klass->extra_flags |= is_value_typedef(klass->by_val->ele_type) ? (uint32_t)metadata::RtClassExtraAttribute::ValueType
                                                                    : (uint32_t)metadata::RtClassExtraAttribute::ReferenceType;
    if (is_enum_type_internal(klass))
    {
        klass->extra_flags |= (uint32_t)metadata::RtClassExtraAttribute::Enum;
        auto optFieldRow = cliImage.read_field(typeDefRow.field_list);
        if (!optFieldRow)
        {
            RET_ERR(RtErr::BadImageFormat);
        }
        metadata::RowField fieldRow = optFieldRow.value();
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, fieldTypeSig, mod->read_field_sig(fieldRow.signature, gcc, nullptr));
        UNWRAP_OR_RET_ERR_ON_FAIL(klass->element_class, get_class_from_typesig(fieldTypeSig));
        klass->cast_class = klass->element_class->cast_class;
    }
    else if (mod->is_corlib())
    {
        setup_cast_class(klass);
    }
    RET_OK(klass);
}

// Transliterated query and state management functions
uint32_t Class::get_type_def_gid(const metadata::RtClass* klass)
{
    return klass->by_val->data.type_def_gid;
}

metadata::RtGenericContainerContext Class::get_generic_container_context(const metadata::RtClass* klass)
{
    return metadata::RtGenericContainerContext{klass->generic_container, nullptr};
}

metadata::RtClass* Class::get_generic_base_klass_of_generic_class(const metadata::RtClass* klass)
{
    assert(is_generic_inst(klass));
    const metadata::RtGenericClass* gc = klass->by_val->data.generic_class;
    auto res = get_class_by_type_def_gid(gc->base_type_def_gid);
    return res.is_ok() ? res.unwrap() : nullptr;
}

metadata::RtClass* Class::get_generic_base_klass_or_self(const metadata::RtClass* klass)
{
    if (vm::Class::is_generic_inst(klass))
    {
        return vm::Class::get_generic_base_klass_of_generic_class(klass);
    }
    return const_cast<metadata::RtClass*>(klass);
}

bool Class::has_class_parent_fast(const metadata::RtClass* klass, const metadata::RtClass* parent)
{
    assert(has_initialized_part(klass, metadata::RtClassInitPart::SuperTypes));
    return parent->hierarchy_depth <= klass->hierarchy_depth && klass->super_types[parent->hierarchy_depth] == parent;
}

bool Class::has_initialized_part(const metadata::RtClass* klass, metadata::RtClassInitPart parts)
{
    return (klass->init_flags & (uint32_t)parts) != 0;
}

void Class::set_initialized_part(metadata::RtClass* klass, metadata::RtClassInitPart parts)
{
    klass->init_flags |= (uint32_t)parts;
}

bool Class::try_set_initialized_part(metadata::RtClass* klass, metadata::RtClassInitPart parts)
{
    if ((klass->init_flags & (uint32_t)parts) == 0)
    {
        klass->init_flags |= (uint32_t)parts;
        return true;
    }
    return false;
}

// Class family determination - transliterated from get_family()
metadata::RtClassFamily Class::get_family(const metadata::RtClass* klass)
{
    switch (klass->by_val->ele_type)
    {
    case metadata::RtElementType::Void:
    case metadata::RtElementType::Object:
    case metadata::RtElementType::String:
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
    case metadata::RtElementType::TypedByRef:
    case metadata::RtElementType::ValueType:
    case metadata::RtElementType::Class:
        return metadata::RtClassFamily::TypeDef;
    case metadata::RtElementType::GenericInst:
        return metadata::RtClassFamily::GenericInst;
    case metadata::RtElementType::Array:
    case metadata::RtElementType::SZArray:
        return metadata::RtClassFamily::ArrayOrSZArray;
    case metadata::RtElementType::Var:
    case metadata::RtElementType::MVar:
        return metadata::RtClassFamily::GenericParam;
    case metadata::RtElementType::Ptr:
    case metadata::RtElementType::FnPtr:
        return metadata::RtClassFamily::TypeOrFnPtr;
    default:
        assert(false && "Unreachable");
        return metadata::RtClassFamily::TypeDef;
    }
}

// Reflection/search functions
const metadata::RtFieldInfo* Class::get_field_for_name(const metadata::RtClass* klass, const char* name, bool search_parent)
{
    assert(Class::has_initialized_part(klass, metadata::RtClassInitPart::Field));
    const metadata::RtClass* cur = klass;
    while (cur)
    {
        for (uint16_t i = 0; i < cur->field_count; ++i)
        {
            const metadata::RtFieldInfo* field = cur->fields + i;
            if (strcmp(field->name, name) == 0)
                return field;
        }
        if (!search_parent || !cur->parent)
            break;
        cur = cur->parent;
    }
    return nullptr;
}

const metadata::RtFieldInfo* Class::get_field_for_name(const metadata::RtClass* klass, const char* name, uint32_t name_len, bool search_parent)
{
    assert(Class::has_initialized_part(klass, metadata::RtClassInitPart::Field));
    const metadata::RtClass* cur = klass;
    while (cur)
    {
        for (uint16_t i = 0; i < cur->field_count; ++i)
        {
            const metadata::RtFieldInfo* field = cur->fields + i;
            if (utils::StringUtil::equals(field->name, name, name_len))
                return field;
        }
        if (!search_parent || !cur->parent)
            break;
        cur = cur->parent;
    }
    return nullptr;
}

const metadata::RtMethodInfo* Class::get_method_for_name(const metadata::RtClass* klass, const char* name, bool search_parent)
{
    assert(Class::has_initialized_part(klass, metadata::RtClassInitPart::Method));
    const metadata::RtClass* cur = klass;
    while (cur)
    {
        const metadata::RtMethodInfo** methods = cur->methods;
        for (size_t i = 0; i < cur->method_count; ++i)
        {
            const metadata::RtMethodInfo* method = methods[i];
            if (std::strcmp(method->name, name) == 0)
            {
                return method;
            }
        }
        if (!search_parent || !cur->parent)
            break;
        cur = cur->parent;
    }
    return nullptr;
}

const metadata::RtPropertyInfo* Class::get_property_for_name(const metadata::RtClass* klass, const char* name, bool search_parent)
{
    assert(Class::has_initialized_part(klass, metadata::RtClassInitPart::Property));
    const metadata::RtClass* cur = klass;
    while (cur)
    {
        for (uint16_t i = 0; i < cur->property_count; ++i)
        {
            const metadata::RtPropertyInfo* prop = cur->properties + i;
            if (strcmp(prop->name, name) == 0)
                return prop;
        }
        if (!search_parent || !cur->parent)
            break;
        cur = cur->parent;
    }
    return nullptr;
}

const metadata::RtPropertyInfo* Class::get_property_for_name(const metadata::RtClass* klass, const char* name, uint32_t name_len, bool search_parent)
{
    assert(Class::has_initialized_part(klass, metadata::RtClassInitPart::Property));
    const metadata::RtClass* cur = klass;
    while (cur)
    {
        for (uint16_t i = 0; i < cur->property_count; ++i)
        {
            const metadata::RtPropertyInfo* prop = cur->properties + i;
            if (utils::StringUtil::equals(prop->name, name, name_len))
                return prop;
        }
        if (!search_parent || !cur->parent)
            break;
        cur = cur->parent;
    }
    return nullptr;
}

const metadata::RtEventInfo* Class::get_event_for_name(const metadata::RtClass* klass, const char* name, bool search_parent)
{
    assert(Class::has_initialized_part(klass, metadata::RtClassInitPart::Event));
    const metadata::RtClass* cur = klass;
    while (cur)
    {
        for (uint16_t i = 0; i < cur->event_count; ++i)
        {
            const metadata::RtEventInfo* evt = cur->events + i;
            if (strcmp(evt->name, name) == 0)
                return evt;
        }
        if (!search_parent || !cur->parent)
            break;
        cur = cur->parent;
    }
    return nullptr;
}

const metadata::RtMethodInfo* Class::get_static_constructor(const metadata::RtClass* klass)
{
    assert(has_initialized_part(klass, metadata::RtClassInitPart::Method));
    if (!has_static_constructor(klass))
        return nullptr;

    for (uint16_t i = 0; i < klass->method_count; ++i)
    {
        const metadata::RtMethodInfo* method = klass->methods[i];
        if (strcmp(method->name, STR_CCTOR) == 0)
            return method;
    }
    assert(false && "Static constructor flag is set, but no static constructor found");
    return nullptr;
}

// Class initialization functions - transliterated from class.rs

RtResultVoid Class::initialize_all(metadata::RtClass* klass)
{
    if (!try_set_initialized_part(klass, metadata::RtClassInitPart::All))
        RET_VOID_OK();

    RET_ERR_ON_FAIL(initialize_super_types(klass));
    RET_ERR_ON_FAIL(initialize_interfaces(klass));
    RET_ERR_ON_FAIL(initialize_nested_classes(klass));
    RET_ERR_ON_FAIL(initialize_fields(klass));
    RET_ERR_ON_FAIL(initialize_methods(klass));
    RET_ERR_ON_FAIL(initialize_properties(klass));
    RET_ERR_ON_FAIL(initialize_events(klass));
    RET_ERR_ON_FAIL(initialize_vtables(klass));

    RET_VOID_OK();
}

RtResultVoid Class::initialize_super_types(metadata::RtClass* klass)
{
    if (!try_set_initialized_part(klass, metadata::RtClassInitPart::SuperTypes))
        RET_VOID_OK();

    // Initialize parent class hierarchy
    if (klass->parent)
    {
        RET_ERR_ON_FAIL(initialize_super_types(const_cast<metadata::RtClass*>(klass->parent)));
        klass->hierarchy_depth = klass->parent->hierarchy_depth + 1;
    }
    else
    {
        klass->hierarchy_depth = 0;
    }

    // Allocate and copy super_types array
    uint32_t super_types_count = klass->hierarchy_depth + 1;
    klass->super_types = klass->image->get_mem_pool().calloc_any<const metadata::RtClass*>(super_types_count);

    if (klass->parent)
    {
        std::memcpy(klass->super_types, klass->parent->super_types, sizeof(metadata::RtClass*) * klass->hierarchy_depth);
    }
    klass->super_types[klass->hierarchy_depth] = klass;

    RET_VOID_OK();
}

RtResultVoid Class::initialize_interfaces(metadata::RtClass* klass)
{
    if (!try_set_initialized_part(klass, metadata::RtClassInitPart::InterfaceTypes))
        RET_VOID_OK();

    // Initialize parent interfaces
    if (klass->parent)
    {
        RET_ERR_ON_FAIL(initialize_interfaces(const_cast<metadata::RtClass*>(klass->parent)));
    }

    switch (get_family(klass))
    {
    case metadata::RtClassFamily::TypeDef:
    {
        return setup_interfaces_typedef(klass);
    }
    case metadata::RtClassFamily::GenericInst:
    {
        return GenericClass::setup_interfaces(klass);
    }
    case metadata::RtClassFamily::ArrayOrSZArray:
    {
        return ArrayClass::setup_interfaces(klass);
    }
    case metadata::RtClassFamily::TypeOrFnPtr:
    case metadata::RtClassFamily::GenericParam:
    {
        break;
    }
    };

    RET_VOID_OK();
}

RtResultVoid Class::setup_interfaces_typedef(metadata::RtClass* klass)
{
    if (!klass->parent)
    {
        // No parent, so no interfaces to inherit
        RET_VOID_OK();
    }
    metadata::RtModuleDef* mod = klass->image;
    const metadata::CliImage& cliImage = mod->get_cli_image();
    auto rid = metadata::RtToken::decode_rid(klass->token);
    auto optTypeDefRidRange = cliImage.find_row_range_of_owner_at_sorted_table(metadata::TableType::InterfaceImpl, 0, rid);
    if (!optTypeDefRidRange)
    {
        RET_VOID_OK();
    }
    metadata::RidRange& typeDefRidRange = optTypeDefRidRange.value();
    uint32_t interfaceCount = typeDefRidRange.ridEnd - typeDefRidRange.ridBegin;
    if (interfaceCount > metadata::RT_MAX_INTERFACE_COUNT)
    {
        RET_ERR(RtErr::BadImageFormat);
    }
    const metadata::RtClass** interfaces = mod->get_mem_pool().calloc_any<const metadata::RtClass*>(interfaceCount);
    for (uint32_t i = 0; i < interfaceCount; ++i)
    {
        uint32_t interfaceImplRid = typeDefRidRange.ridBegin + i;
        metadata::RowInterfaceImpl interfaceImplRow = cliImage.read_interface_impl(interfaceImplRid).value();
        metadata::RtToken interfaceTypeToken = metadata::RtMetadata::decode_type_def_ref_spec_coded_index(interfaceImplRow.interface_idx);
        metadata::RtGenericContainerContext gcc = get_generic_container_context(klass);
        UNWRAP_OR_RET_ERR_ON_FAIL(interfaces[i], mod->get_class_by_type_def_ref_spec_token(interfaceTypeToken, gcc, nullptr));
        RET_ERR_ON_FAIL(initialize_all(const_cast<metadata::RtClass*>(interfaces[i])));
    }
    klass->interfaces = interfaces;
    klass->interface_count = static_cast<uint16_t>(interfaceCount);
    RET_VOID_OK();
}

RtResultVoid Class::initialize_nested_classes(metadata::RtClass* klass)
{
    if (!try_set_initialized_part(klass, metadata::RtClassInitPart::NestedClasses))
        RET_VOID_OK();

    // Initialize parent nested classes
    if (klass->parent)
    {
        RET_ERR_ON_FAIL(initialize_nested_classes(const_cast<metadata::RtClass*>(klass->parent)));
    }
    switch (get_family(klass))
    {
    case metadata::RtClassFamily::TypeDef:
    {
        return setup_nested_classes_typedef(klass);
    }
    case metadata::RtClassFamily::GenericInst:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, baseClass, GenericClass::get_base_class(klass->by_val->data.generic_class));
        RET_ERR_ON_FAIL(initialize_nested_classes(baseClass));
        klass->nested_classes = baseClass->nested_classes;
        klass->nested_class_count = baseClass->nested_class_count;
        break;
    }
    case metadata::RtClassFamily::ArrayOrSZArray:
    case metadata::RtClassFamily::TypeOrFnPtr:
    case metadata::RtClassFamily::GenericParam:
    {
        break;
    }
    }
    RET_VOID_OK();
}

RtResultVoid Class::setup_nested_classes_typedef(metadata::RtClass* klass)
{
    metadata::RtModuleDef* mod = klass->image;
    utils::Vector<metadata::RtClass*> nestedClasses;
    RET_ERR_ON_FAIL(mod->get_nested_classs(klass->token, nestedClasses));
    if (nestedClasses.size() > 0)
    {
        size_t nestedClassCount = nestedClasses.size();
        if (nestedClassCount > metadata::RT_MAX_NESTED_CLASS_COUNT)
        {
            RET_ERR(RtErr::BadImageFormat);
        }
        klass->nested_classes = mod->get_mem_pool().calloc_any<const metadata::RtClass*>(nestedClassCount);
        for (size_t i = 0; i < nestedClassCount; ++i)
        {
            klass->nested_classes[i] = nestedClasses[i];
        }
        klass->nested_class_count = static_cast<uint16_t>(nestedClassCount);
    }
    RET_VOID_OK();
}

static RtResult<bool> is_reference_type_or_contains_reference_type_in_typesig(const metadata::RtTypeSig* typeSig)
{
    if (typeSig->by_ref)
        RET_OK(false);
    switch (typeSig->ele_type)
    {
    case metadata::RtElementType::Object:
    case metadata::RtElementType::String:
    case metadata::RtElementType::Class:
    case metadata::RtElementType::Array:
    case metadata::RtElementType::SZArray:
        RET_OK(true);
    case metadata::RtElementType::ValueType:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, cls, Class::get_class_from_typesig(typeSig));
        RET_ERR_ON_FAIL(Class::initialize_fields(cls));
        RET_OK(Class::get_has_references(cls));
    }
    case metadata::RtElementType::GenericInst:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, baseClass, GenericClass::get_base_class(typeSig->data.generic_class));
        if (Class::is_reference_type(baseClass))
        {
            RET_OK(true);
        }
        else
        {
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, inflatedClass, Class::get_class_from_typesig(typeSig));
            RET_ERR_ON_FAIL(Class::initialize_fields(inflatedClass));
            RET_OK(Class::get_has_references(inflatedClass));
        }
    }
    default:
        RET_OK(false);
    }
}

RtResultVoid Class::initialize_fields(metadata::RtClass* klass)
{
    if (!try_set_initialized_part(klass, metadata::RtClassInitPart::Field))
        RET_VOID_OK();

    // Initialize parent fields
    if (klass->parent)
    {
        RET_ERR_ON_FAIL(initialize_fields(const_cast<metadata::RtClass*>(klass->parent)));
    }

    switch (get_family(klass))
    {
    case metadata::RtClassFamily::TypeDef:
    {
        RET_ERR_ON_FAIL(setup_fields_typedef(klass));
        RET_ERR_ON_FAIL(setup_field_layout(klass));
        RET_ERR_ON_FAIL(setup_static_field_data(klass));
        break;
    }
    case metadata::RtClassFamily::GenericInst:
    {
        RET_ERR_ON_FAIL(GenericClass::setup_fields(klass));
        RET_ERR_ON_FAIL(setup_field_layout(klass));
        RET_ERR_ON_FAIL(setup_static_field_data(klass));
        break;
    }
    case metadata::RtClassFamily::ArrayOrSZArray:
    {
        klass->instance_size_without_header = 0;
        klass->alignment = PTR_ALIGN;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, isRefType, is_reference_type_or_contains_reference_type_in_typesig(klass->element_class->by_val));
        if (isRefType)
        {
            set_has_references(klass);
        }
        break;
    }
    case metadata::RtClassFamily::TypeOrFnPtr:
    case metadata::RtClassFamily::GenericParam:
    {
        klass->instance_size_without_header = PTR_SIZE;
        klass->alignment = PTR_ALIGN;
        break;
    }
    }
    RET_VOID_OK();
}

RtResultVoid Class::setup_fields_typedef(metadata::RtClass* klass)
{
    uint32_t rid = metadata::RtToken::decode_rid(klass->token);
    metadata::RtModuleDef* mod = klass->image;
    const metadata::CliImage& cliImage = mod->get_cli_image();
    auto optTypeDefRowCurr = cliImage.read_type_def(rid);
    if (!optTypeDefRowCurr)
    {
        RET_ERR(RtErr::BadImageFormat);
    }
    uint32_t fieldRidBegin = optTypeDefRowCurr.value().field_list;
    auto optTypeDefRowNext = cliImage.read_type_def(rid + 1);
    uint32_t fieldRidEnd = optTypeDefRowNext ? optTypeDefRowNext.value().field_list : cliImage.get_table_row_num(metadata::TableType::Field) + 1;
    if (fieldRidBegin >= fieldRidEnd)
    {
        // No fields
        RET_VOID_OK();
    }
    uint32_t fieldCount = fieldRidEnd - fieldRidBegin;
    if (fieldCount > metadata::RT_MAX_FIELD_COUNT)
    {
        RET_ERR(RtErr::BadImageFormat);
    }
    metadata::RtFieldInfo* fields = mod->get_mem_pool().calloc_any<metadata::RtFieldInfo>(fieldCount);
    for (uint32_t i = 0; i < fieldCount; ++i)
    {
        uint32_t fieldRid = fieldRidBegin + i;
        auto optFieldRow = cliImage.read_field(fieldRid);
        assert(optFieldRow && "Field row should exist");
        const metadata::RowField& fieldRow = optFieldRow.value();
        metadata::RtFieldInfo* field = fields + i;
        field->parent = klass;
        UNWRAP_OR_RET_ERR_ON_FAIL(field->name, mod->get_string(fieldRow.name));
        field->token = metadata::RtToken::encode(metadata::TableType::Field, fieldRid);
        field->offset = 0; // To be set up in layout step
        field->flags = fieldRow.flags;
        UNWRAP_OR_RET_ERR_ON_FAIL(field->type_sig, mod->read_field_sig(fieldRow.signature, get_generic_container_context(klass), nullptr));
    }

    klass->fields = fields;
    klass->field_count = static_cast<uint16_t>(fieldCount);

    RET_VOID_OK();
}

RtResultVoid Class::setup_field_layout(metadata::RtClass* klass)
{
    assert(has_initialized_part(klass, metadata::RtClassInitPart::Field));
    utils::Vector<const metadata::RtFieldInfo*> instanceFields;
    utils::Vector<const metadata::RtFieldInfo*> staticFields;

    bool has_references = klass->parent ? get_has_references(klass->parent) : false;
    for (uint16_t i = 0; i < klass->field_count; ++i)
    {
        const metadata::RtFieldInfo* field = klass->fields + i;
        if (Field::is_instance(field))
        {
            instanceFields.push_back(field);
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, isRefType, is_reference_type_or_contains_reference_type_in_typesig(field->type_sig));
            if (isRefType)
            {
                has_references = true;
            }
        }
        else if (Field::is_static_excluded_literal_and_rva(field))
        {
            staticFields.push_back(field);
        }
    }
    if (has_references)
    {
        set_has_references(klass);
    }

    metadata::RtModuleDef* mod = klass->image;
    auto optLayout = mod->get_class_layout_data(klass->token);
    uint32_t classSize;
    uint32_t packingSize;
    if (optLayout)
    {
        classSize = optLayout->size;
        packingSize = optLayout->packing;
    }
    else
    {
        classSize = 0;
        packingSize = 0;
    }

    metadata::SizeAndAlignment instanceSizeAndAlignment;
    if (is_explicit_layout(klass))
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(instanceSizeAndAlignment, metadata::Layout::compute_explicit_layout(mod, instanceFields, static_cast<uint8_t>(packingSize)));
    }
    else
    {
        uint32_t parentSize;
        uint32_t parentAlignment;
        if (klass->parent)
        {
            parentSize = get_instance_size_without_object_header(klass->parent);
            parentAlignment = klass->parent->alignment;
        }
        else
        {
            parentSize = 0;
            parentAlignment = 1;
        }
        UNWRAP_OR_RET_ERR_ON_FAIL(instanceSizeAndAlignment, metadata::Layout::compute_layout(instanceFields, parentSize, static_cast<uint8_t>(parentAlignment),
                                                                                             static_cast<uint8_t>(packingSize)));
    }
    klass->instance_size_without_header = std::max(instanceSizeAndAlignment.size, classSize);
    if (Class::is_value_type(klass))
    {
        klass->instance_size_without_header = std::max(klass->instance_size_without_header, (uint32_t)1);
    }
    klass->alignment = static_cast<uint8_t>(instanceSizeAndAlignment.alignment);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::SizeAndAlignment, staticSizeAndAlignment, metadata::Layout::compute_layout(staticFields, 0, 1, 0));
    klass->static_size = staticSizeAndAlignment.size;
    RET_VOID_OK();
}

RtResultVoid Class::setup_static_field_data(metadata::RtClass* klass)
{
    if (klass->static_size > 0)
    {
        klass->static_fields_data = (uint8_t*)gc::GarbageCollector::allocate_fixed(klass->static_size);
    }
    RET_VOID_OK();
}

RtResultVoid Class::initialize_methods(metadata::RtClass* klass)
{
    if (!try_set_initialized_part(klass, metadata::RtClassInitPart::Method))
        RET_VOID_OK();

    if (klass->parent)
    {
        RET_ERR_ON_FAIL(initialize_methods(const_cast<metadata::RtClass*>(klass->parent)));
    }
    switch (get_family(klass))
    {
    case metadata::RtClassFamily::TypeDef:
    {
        RET_ERR_ON_FAIL(setup_methods_typedef(klass));
        break;
    }
    case metadata::RtClassFamily::GenericInst:
    {
        RET_ERR_ON_FAIL(GenericClass::setup_methods(klass));
        break;
    }
    case metadata::RtClassFamily::ArrayOrSZArray:
    {
        RET_ERR_ON_FAIL(ArrayClass::setup_methods(klass));
        break;
    }
    default:
        break;
    };
    RET_ERR_ON_FAIL(build_methods_arg_descs(klass));
    RET_VOID_OK();
}

RtResultVoid Class::setup_methods_typedef(metadata::RtClass* klass)
{
    uint32_t rid = metadata::RtToken::decode_rid(klass->token);
    metadata::RtModuleDef* mod = klass->image;
    const metadata::CliImage& cliImage = mod->get_cli_image();
    auto optTypeDefRowCur = cliImage.read_type_def(rid);
    if (!optTypeDefRowCur)
    {
        RET_ERR(RtErr::BadImageFormat);
    }
    uint32_t methodRidBegin = optTypeDefRowCur->method_list;
    auto optTypeDefRowNext = cliImage.read_type_def(rid + 1);
    uint32_t methodRidEnd = optTypeDefRowNext ? optTypeDefRowNext->method_list : mod->get_method_count() + 1;
    if (methodRidBegin >= methodRidEnd)
    {
        // No methods
        RET_VOID_OK();
    }
    uint32_t methodCount = methodRidEnd - methodRidBegin;
    if (methodCount > metadata::RT_MAX_METHOD_COUNT)
    {
        RET_ERR(RtErr::BadImageFormat);
    }
    alloc::MemPool& pool = mod->get_mem_pool();
    const metadata::RtMethodInfo** methods = pool.calloc_any<const metadata::RtMethodInfo*>(methodCount);
    for (uint32_t i = 0; i < methodCount; ++i)
    {
        uint32_t methodRid = methodRidBegin + i;
        auto optMethodRow = cliImage.read_method(methodRid);
        assert(optMethodRow && "Method row should exist");
        const metadata::RowMethod& methodRow = optMethodRow.value();
        metadata::RtMethodInfo* method = pool.malloc_any_zeroed<metadata::RtMethodInfo>();
        method->parent = klass;
        UNWRAP_OR_RET_ERR_ON_FAIL(method->name, mod->get_string(methodRow.name));
        method->token = metadata::RtToken::encode(metadata::TableType::Method, methodRid);
        method->flags = methodRow.flags;
        method->iflags = methodRow.impl_flags;
        if (std::strcmp(method->name, STR_CCTOR) == 0)
        {
            klass->extra_flags |= (uint32_t)metadata::RtClassExtraAttribute::HasStaticConstructor;
        }
        else if (std::strcmp(method->name, STR_FINALIZE) == 0)
        {
            klass->extra_flags |= (uint32_t)metadata::RtClassExtraAttribute::HasFinalizer;
        }
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtGenericContainer*, genericContainer, mod->get_generic_container(method->token));
        metadata::RtGenericContainerContext gcc{klass->generic_container, genericContainer};

        auto retMethodSig = mod->read_method_sig(methodRow.signature, gcc, nullptr);
        RET_ERR_ON_FAIL(retMethodSig);
        const metadata::RtMethodSig& methodSig = retMethodSig.unwrap();
        method->return_type = methodSig.return_type;
        size_t paramCount = methodSig.params.size();
        method->parameter_count = static_cast<uint16_t>(paramCount);
        method->parameters = pool.calloc_any<const metadata::RtTypeSig*>(paramCount);
        std::memcpy(method->parameters, methodSig.params.data(), sizeof(metadata::RtTypeSig*) * paramCount);

        if (genericContainer)
        {
            if (methodSig.generic_param_count != genericContainer->generic_param_count)
            {
                RET_ERR(RtErr::BadImageFormat);
            }
            method->generic_container = genericContainer;
        }
        else
        {
            if (methodSig.generic_param_count != 0)
            {
                RET_ERR(RtErr::BadImageFormat);
            }
        }

        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(InvokeTypeAndMethod, invoker_type_and_method, Shim::get_invoker(method));
        method->invoke_method_ptr = invoker_type_and_method.invoker;
        method->invoker_type = invoker_type_and_method.invoker_type;
        method->virtual_invoke_method_ptr = invoker_type_and_method.virtual_invoker;
        method->method_ptr = Shim::get_method_pointer(method);
        methods[i] = method;
    }

    klass->methods = methods;
    klass->method_count = static_cast<uint16_t>(methodCount);

    RET_VOID_OK();
}

RtResultVoid Class::build_methods_arg_descs(metadata::RtClass* klass)
{
    for (uint16_t i = 0; i < klass->method_count; ++i)
    {
        const metadata::RtMethodInfo* method = klass->methods[i];
        if (method->generic_method)
        {
            // Generic methods arg descs are built on demand
            continue;
        }
        RET_ERR_ON_FAIL(Method::build_method_arg_descs(const_cast<metadata::RtMethodInfo*>(method)));
    }
    RET_VOID_OK();
}

RtResultVoid Class::initialize_properties(metadata::RtClass* klass)
{
    if (!try_set_initialized_part(klass, metadata::RtClassInitPart::Property))
        RET_VOID_OK();

    // Properties initialization requires methods first
    RET_ERR_ON_FAIL(initialize_methods(klass));

    // Initialize parent properties
    if (klass->parent)
    {
        RET_ERR_ON_FAIL(initialize_properties(const_cast<metadata::RtClass*>(klass->parent)));
    }
    switch (get_family(klass))
    {
    case metadata::RtClassFamily::TypeDef:
    {
        RET_ERR_ON_FAIL(setup_properties_typedef(klass));
        break;
    }
    case metadata::RtClassFamily::GenericInst:
    {
        RET_ERR_ON_FAIL(GenericClass::setup_properties(klass));
        break;
    }
    default:
        break;
    }
    RET_VOID_OK();
}

RtResultVoid Class::setup_properties_typedef(metadata::RtClass* klass)
{
    metadata::RtModuleDef* mod = klass->image;
    uint32_t rid = metadata::RtToken::decode_rid(klass->token);
    const metadata::CliImage& cliImage = mod->get_cli_image();
    auto optPropertyMapRid = cliImage.find_row_of_owner(metadata::TableType::PropertyMap, 0, rid);
    if (!optPropertyMapRid)
    {
        RET_VOID_OK();
    }
    auto optCurPropertyMap = cliImage.read_property_map(optPropertyMapRid.value());
    auto optNextCurPropertyMap = cliImage.read_property_map(optPropertyMapRid.value() + 1);
    uint32_t propertyRidStart = optCurPropertyMap->property_list;
    uint32_t propertyRidEnd = optNextCurPropertyMap ? optNextCurPropertyMap->property_list : cliImage.get_table_row_num(metadata::TableType::Property) + 1;

    uint32_t propertyCount = propertyRidEnd - propertyRidStart;
    assert(propertyCount > 0);
    if (propertyCount > metadata::RT_MAX_PROPERTY_COUNT)
    {
        RET_ERR(RtErr::BadImageFormat);
    }
    metadata::RtPropertyInfo* properties = mod->get_mem_pool().calloc_any<metadata::RtPropertyInfo>(propertyCount);
    for (uint32_t i = 0; i < propertyCount; ++i)
    {
        metadata::RtPropertyInfo* property = properties + i;
        uint32_t propertyRid = propertyRidStart + i;
        auto optPropertyRow = cliImage.read_property(propertyRid);
        assert(optPropertyRow && "Property row should exist");
        const metadata::RowProperty& propertyRow = optPropertyRow.value();
        property->parent = klass;
        UNWRAP_OR_RET_ERR_ON_FAIL(property->name, mod->get_string(propertyRow.name));
        property->token = metadata::RtToken::encode(metadata::TableType::Property, propertyRid);
        property->flags = propertyRow.flags;
        UNWRAP_OR_RET_ERR_ON_FAIL(property->property_sig, mod->read_property_sig(propertyRow.type_, get_generic_container_context(klass), nullptr));

        uint32_t associationEncodedIdx = metadata::RtMetadata::encode_has_semantics_coded_index(metadata::TableType::Property, propertyRid);
        auto optMethodSemanticsRange = cliImage.find_row_range_of_owner_at_sorted_table(metadata::TableType::MethodSemantics, 2, associationEncodedIdx);
        if (optMethodSemanticsRange)
        {
            metadata::RidRange& methodSemanticsRange = optMethodSemanticsRange.value();
            for (uint32_t methodSemanticsRid = methodSemanticsRange.ridBegin; methodSemanticsRid < methodSemanticsRange.ridEnd; ++methodSemanticsRid)
            {
                auto optMethodSemanticsRow = cliImage.read_method_semantics(methodSemanticsRid);
                assert(optMethodSemanticsRow && "MethodSemantics row should exist");
                const metadata::RowMethodSemantics& methodSemanticsRow = optMethodSemanticsRow.value();
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method, mod->get_method_by_rid(methodSemanticsRow.method));
                if ((methodSemanticsRow.semantics & (uint16_t)metadata::RtMethodSemanticsAttributes::Getter) != 0)
                {
                    property->get_method = method;
                }
                else if ((methodSemanticsRow.semantics & (uint16_t)metadata::RtMethodSemanticsAttributes::Setter) != 0)
                {
                    property->set_method = method;
                }
                else if ((methodSemanticsRow.semantics & (uint16_t)metadata::RtMethodSemanticsAttributes::Other) != 0)
                {
                    RET_ERR(RtErr::NotSupported);
                }
            }
        }
    }
    klass->properties = properties;
    klass->property_count = static_cast<uint16_t>(propertyCount);
    RET_VOID_OK();
}

RtResultVoid Class::initialize_events(metadata::RtClass* klass)
{
    if (!try_set_initialized_part(klass, metadata::RtClassInitPart::Event))
        RET_VOID_OK();

    // Initialize parent events
    if (klass->parent)
    {
        RET_ERR_ON_FAIL(initialize_events(const_cast<metadata::RtClass*>(klass->parent)));
    }
    switch (get_family(klass))
    {
    case metadata::RtClassFamily::TypeDef:
    {
        RET_ERR_ON_FAIL(setup_events_typedef(klass));
        break;
    }
    case metadata::RtClassFamily::GenericInst:
    {
        RET_ERR_ON_FAIL(GenericClass::setup_events(klass));
        break;
    }
    default:
        break;
    }
    RET_VOID_OK();
}

RtResultVoid Class::setup_events_typedef(metadata::RtClass* klass)
{
    metadata::RtModuleDef* mod = klass->image;
    uint32_t rid = metadata::RtToken::decode_rid(klass->token);
    const metadata::CliImage& cliImage = mod->get_cli_image();
    auto optTypeDefRidRange = cliImage.find_row_of_owner(metadata::TableType::EventMap, 0, rid);
    if (!optTypeDefRidRange)
    {
        RET_VOID_OK();
    }
    uint32_t eventMapRid = optTypeDefRidRange.value();
    auto optEventMapRow = cliImage.read_event_map(eventMapRid);
    assert(optEventMapRow && "EventMap row should exist");
    uint32_t eventRidBegin = optEventMapRow.value().event_list;
    auto optEventMapRowNext = cliImage.read_event_map(eventMapRid + 1);
    uint32_t eventRidEnd = optEventMapRowNext ? optEventMapRowNext->event_list : cliImage.get_table_row_num(metadata::TableType::Event) + 1;
    uint32_t eventCount = eventRidEnd - eventRidBegin;
    if (eventCount > metadata::RT_MAX_EVENT_COUNT)
    {
        RET_ERR(RtErr::BadImageFormat);
    }
    metadata::RtEventInfo* events = mod->get_mem_pool().calloc_any<metadata::RtEventInfo>(eventCount);
    for (uint32_t i = 0; i < eventCount; ++i)
    {
        metadata::RtEventInfo* event = events + i;
        uint32_t eventRid = eventRidBegin + i;
        auto optEventRow = cliImage.read_event(eventRid);
        if (!optEventRow)
        {
            RET_ERR(RtErr::BadImageFormat);
        }
        const metadata::RowEvent& eventRow = optEventRow.value();
        event->parent = klass;
        UNWRAP_OR_RET_ERR_ON_FAIL(event->name, mod->get_string(eventRow.name));
        event->token = metadata::RtToken::encode(metadata::TableType::Event, eventRid);
        event->flags = eventRow.event_flags;
        UNWRAP_OR_RET_ERR_ON_FAIL(event->type_sig,
                                  mod->get_typesig_by_type_def_ref_spec_token(metadata::RtMetadata::decode_type_def_ref_spec_coded_index(eventRow.event_type),
                                                                              get_generic_container_context(klass), nullptr));

        uint32_t associationEncodedIdx = metadata::RtMetadata::encode_has_semantics_coded_index(metadata::TableType::Event, eventRid);
        auto optMethodSemanticsRange = cliImage.find_row_range_of_owner_at_sorted_table(metadata::TableType::MethodSemantics, 2, associationEncodedIdx);
        if (optMethodSemanticsRange)
        {
            metadata::RidRange& methodSemanticsRange = optMethodSemanticsRange.value();
            for (uint32_t methodSemanticsRid = methodSemanticsRange.ridBegin; methodSemanticsRid < methodSemanticsRange.ridEnd; ++methodSemanticsRid)
            {
                auto optMethodSemanticsRow = cliImage.read_method_semantics(methodSemanticsRid);
                assert(optMethodSemanticsRow && "MethodSemantics row should exist");
                const metadata::RowMethodSemantics& methodSemanticsRow = optMethodSemanticsRow.value();
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method, mod->get_method_by_rid(methodSemanticsRow.method));
                if ((methodSemanticsRow.semantics & (uint16_t)metadata::RtMethodSemanticsAttributes::AddOn) != 0)
                {
                    event->add_method = method;
                }
                else if ((methodSemanticsRow.semantics & (uint16_t)metadata::RtMethodSemanticsAttributes::RemoveOn) != 0)
                {
                    event->remove_method = method;
                }
                else if ((methodSemanticsRow.semantics & (uint16_t)metadata::RtMethodSemanticsAttributes::Fire) != 0)
                {
                    event->raise_method = method;
                }
                else if ((methodSemanticsRow.semantics & (uint16_t)metadata::RtMethodSemanticsAttributes::Other) != 0)
                {
                    RET_ERR(RtErr::NotSupported);
                }
            }
        }
    }
    klass->events = events;
    klass->event_count = static_cast<uint16_t>(eventCount);
    RET_VOID_OK();
}

RtResultVoid Class::initialize_vtables(metadata::RtClass* klass)
{
    if (!try_set_initialized_part(klass, metadata::RtClassInitPart::VirtualTable))
        RET_VOID_OK();

    if (klass->parent)
    {
        RET_ERR_ON_FAIL(initialize_vtables(const_cast<metadata::RtClass*>(klass->parent)));
    }

    RET_ERR_ON_FAIL(initialize_super_types(klass));
    RET_ERR_ON_FAIL(initialize_interfaces(klass));
    RET_ERR_ON_FAIL(initialize_methods(klass));

    for (uint16_t i = 0; i < klass->interface_count; ++i)
    {
        const metadata::RtClass* interfaceClass = klass->interfaces[i];
        RET_ERR_ON_FAIL(initialize_vtables(const_cast<metadata::RtClass*>(interfaceClass)));
    }

    switch (get_family(klass))
    {
    case metadata::RtClassFamily::TypeDef:
    {
        RET_ERR_ON_FAIL(setup_vtable_typedef(klass));
        break;
    }
    case metadata::RtClassFamily::GenericInst:
    {
        RET_ERR_ON_FAIL(GenericClass::setup_vtables(klass));
        break;
    }
    case metadata::RtClassFamily::ArrayOrSZArray:
    {
        RET_ERR_ON_FAIL(ArrayClass::setup_vtables(klass));
        break;
    }
    case metadata::RtClassFamily::TypeOrFnPtr:
    case metadata::RtClassFamily::GenericParam:
    {
        if (klass->parent)
        {
            klass->vtable = klass->parent->vtable;
            klass->vtable_count = klass->parent->vtable_count;
            // klass->interface_vtable_offsets = klass->parent->interface_vtable_offsets;
            // klass->interface_vtable_offset_count = klass->parent->interface_vtable_offset_count;
        }
        break;
    }
    }

    RET_VOID_OK();
}

static void collect_virtual_methods(const metadata::RtClass* klass, utils::Vector<const metadata::RtMethodInfo*>& virtualMethods)
{
    if (klass->parent)
    {
        collect_virtual_methods(klass->parent, virtualMethods);
    }
    for (uint16_t i = 0; i < klass->method_count; ++i)
    {
        const metadata::RtMethodInfo* method = klass->methods[i];
        if (Method::is_virtual(method))
        {
            virtualMethods.push_back(method);
        }
    }
}

RtResultVoid Class::setup_vtable_typedef(metadata::RtClass* klass)
{
    // Collect all virtual methods in the hierarchy (parent first)
    utils::Vector<const metadata::RtMethodInfo*> total_hierarchy_virtual_methods;

    if (klass->parent)
        collect_virtual_methods(klass->parent, total_hierarchy_virtual_methods);
    size_t self_virtual_method_start_index = total_hierarchy_virtual_methods.size();

    utils::Vector<const metadata::RtMethodInfo*> self_new_slot_virtual_methods;
    utils::Vector<const metadata::RtMethodInfo*> self_override_virtual_methods;
    for (uint16_t i = 0; i < klass->method_count; ++i)
    {
        const metadata::RtMethodInfo* method = klass->methods[i];
        if (Method::is_virtual(method))
        {
            total_hierarchy_virtual_methods.push_back(method);
            if (Method::is_new_slot(method))
                self_new_slot_virtual_methods.push_back(method);
            else
                self_override_virtual_methods.push_back(method);
        }
    }

    alloc::MemPool& pool = klass->image->get_mem_pool();
    // No parent: only build vtable for interfaces or corlib Object
    if (!klass->parent)
    {
        if (Class::is_interface(klass) || (std::strcmp(klass->name, "Object") == 0 && klass->image->is_corlib()))
        {
            metadata::RtVirtualInvokeData* new_vtable = pool.calloc_any<metadata::RtVirtualInvokeData>(self_new_slot_virtual_methods.size());
            uint16_t slot = 0;
            for (size_t i = 0; i < self_new_slot_virtual_methods.size(); ++i)
            {
                const metadata::RtMethodInfo* vmethod = self_new_slot_virtual_methods[i];
                const_cast<metadata::RtMethodInfo*>(vmethod)->slot = slot;
                new_vtable[i] = metadata::RtVirtualInvokeData{vmethod, Method::is_abstract(vmethod) ? nullptr : vmethod};
                ++slot;
            }
            klass->vtable = new_vtable;
            klass->vtable_count = static_cast<uint16_t>(self_new_slot_virtual_methods.size());
        }
        RET_VOID_OK();
    }

    const metadata::RtClass* parent = klass->parent;
    if (self_new_slot_virtual_methods.size() == 0 && self_override_virtual_methods.size() == 0 && klass->interface_count == 0)
    {
        klass->vtable = parent->vtable;
        klass->vtable_count = parent->vtable_count;
        klass->interface_vtable_offsets = parent->interface_vtable_offsets;
        klass->interface_vtable_offset_count = parent->interface_vtable_offset_count;
        RET_VOID_OK();
    }

    // Build interface vtable offsets (inherit parent, add new)
    utils::Vector<metadata::RtInterfaceOffset> new_interface_vtable_offsets;
    new_interface_vtable_offsets.reserve(static_cast<size_t>(parent->interface_vtable_offset_count + klass->interface_count));
    for (uint16_t i = 0; i < parent->interface_vtable_offset_count; ++i)
        new_interface_vtable_offsets.push_back(parent->interface_vtable_offsets[i]);

    utils::Vector<size_t> self_interface_vtable_offset_indexes;
    self_interface_vtable_offset_indexes.reserve(klass->interface_count);

    size_t total_slot_count = parent->vtable_count;
    for (uint16_t i = 0; i < klass->interface_count; ++i)
    {
        const metadata::RtClass* interface_class = klass->interfaces[i];
        bool found = false;
        for (uint16_t j = 0; j < parent->interface_vtable_offset_count; ++j)
        {
            const metadata::RtInterfaceOffset& interface_offset = parent->interface_vtable_offsets[j];
            if (interface_offset.interface == interface_class)
            {
                found = true;
                self_interface_vtable_offset_indexes.push_back(j);
                break;
            }
        }
        if (!found)
        {
            self_interface_vtable_offset_indexes.push_back(new_interface_vtable_offsets.size());
            new_interface_vtable_offsets.push_back(metadata::RtInterfaceOffset{interface_class, static_cast<uint16_t>(total_slot_count)});
            total_slot_count += interface_class->vtable_count;
        }
    }

    metadata::RtInterfaceOffset* interface_vtable_offsets = pool.calloc_any<metadata::RtInterfaceOffset>(new_interface_vtable_offsets.size());
    std::memcpy(interface_vtable_offsets, new_interface_vtable_offsets.data(), new_interface_vtable_offsets.size() * sizeof(metadata::RtInterfaceOffset));
    klass->interface_vtable_offsets = interface_vtable_offsets;
    klass->interface_vtable_offset_count = static_cast<uint16_t>(new_interface_vtable_offsets.size());

    // Build new vtable: parent + newly added interfaces
    utils::Vector<metadata::RtVirtualInvokeData> new_vtable;
    new_vtable.reserve(total_slot_count + self_new_slot_virtual_methods.size());

    for (uint16_t i = 0; i < parent->vtable_count; ++i)
        new_vtable.push_back(parent->vtable[i]);

    for (size_t i = 0; i < new_interface_vtable_offsets.size(); ++i)
    {
        const metadata::RtInterfaceOffset& offset_info = new_interface_vtable_offsets[i];
        if (offset_info.offset < parent->vtable_count)
            continue;
        const metadata::RtClass* interface_class = offset_info.interface;
        for (uint16_t j = 0; j < interface_class->vtable_count; ++j)
            new_vtable.push_back(interface_class->vtable[j]);
    }

    assert(new_vtable.size() == total_slot_count);

    // Allocate slots for new virtual methods (new slot)
    for (size_t i = 0; i < self_new_slot_virtual_methods.size(); ++i)
    {
        const metadata::RtMethodInfo* vmethod = self_new_slot_virtual_methods[i];
        const_cast<metadata::RtMethodInfo*>(vmethod)->slot = static_cast<uint16_t>(total_slot_count);
        new_vtable.push_back(metadata::RtVirtualInvokeData{vmethod, Method::is_abstract(vmethod) ? nullptr : vmethod});
        ++total_slot_count;
    }

    assert(new_vtable.size() == total_slot_count);

    // Track initialized entries
    utils::HashSet<size_t> initialized_vtable_index_set;

    const metadata::CliImage& cli_image = klass->image->get_cli_image();
    metadata::RtGenericContainerContext gcc = Class::get_generic_container_context(klass);

    auto opt_method_impl_range =
        cli_image.find_row_range_of_owner_at_sorted_table(metadata::TableType::MethodImpl, 0, metadata::RtToken::decode_rid(klass->token));
    if (opt_method_impl_range)
    {
        metadata::RidRange& range = opt_method_impl_range.value();
        for (uint32_t method_impl_rid = range.ridBegin; method_impl_rid < range.ridEnd; ++method_impl_rid)
        {
            auto opt_row = cli_image.read_method_impl(method_impl_rid);
            if (!opt_row)
                RET_ERR(RtErr::BadImageFormat);
            metadata::RowMethodImpl row = opt_row.value();

            metadata::RtToken body_token = metadata::RtMetadata::decode_method_def_or_ref_coded_index(row.method_body);
            metadata::RtToken decl_token = metadata::RtMetadata::decode_method_def_or_ref_coded_index(row.method_declaration);

            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, body_method, klass->image->get_method_by_token(body_token, gcc, nullptr));
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, declaration_method,
                                                    klass->image->get_method_by_token(decl_token, gcc, nullptr));

            if (!Method::is_virtual(declaration_method) || !Method::is_virtual(body_method))
                RET_ERR(RtErr::BadImageFormat);

            const metadata::RtClass* declaration_klass = declaration_method->parent;
            uint16_t declaration_slot = declaration_method->slot;
            if (declaration_slot == metadata::RT_INVALID_METHOD_SLOT)
                RET_ERR(RtErr::BadImageFormat);

            size_t slot = 0;
            if (Class::is_interface(declaration_klass))
            {
                uint16_t interface_offset = 0;
                bool found = false;
                for (uint16_t i = 0; i < klass->interface_vtable_offset_count; ++i)
                {
                    const metadata::RtInterfaceOffset& off = klass->interface_vtable_offsets[i];
                    if (off.interface == declaration_klass)
                    {
                        interface_offset = off.offset;
                        found = true;
                        break;
                    }
                }
                if (!found)
                    RET_ERR(RtErr::BadImageFormat);

                size_t vtable_index = static_cast<size_t>(interface_offset) + declaration_slot;
                if (vtable_index >= new_vtable.size())
                    RET_ERR(RtErr::BadImageFormat);
                if (!initialized_vtable_index_set.insert(vtable_index).second)
                    RET_ERR(RtErr::BadImageFormat);
                slot = vtable_index;
            }
            else
            {
                if (!Class::has_class_parent_fast(klass, declaration_klass))
                    RET_ERR(RtErr::BadImageFormat);
                slot = declaration_slot;
            }

            metadata::RtVirtualInvokeData& entry = new_vtable[slot];
            if (entry.method != declaration_method)
                RET_ERR(RtErr::BadImageFormat);
            entry.method_impl = body_method;
        }
    }

    // Handle override virtual methods
    for (size_t idx = 0; idx < self_override_virtual_methods.size(); ++idx)
    {
        const metadata::RtMethodInfo* vmethod = self_override_virtual_methods[idx];
        bool find_impl = false;
        for (size_t i = self_virtual_method_start_index; i-- > 0;)
        {
            const metadata::RtMethodInfo* to_match_method = total_hierarchy_virtual_methods[i];
            if (to_match_method->slot == metadata::RT_INVALID_METHOD_SLOT)
                RET_ERR(RtErr::BadImageFormat);
            if (metadata::MetadataCompare::is_method_signature_equal(vmethod, to_match_method, true, true))
            {
                size_t match_slot = to_match_method->slot;
                if (initialized_vtable_index_set.find(match_slot) != initialized_vtable_index_set.end())
                {
                    find_impl = true;
                    break;
                }
                metadata::RtVirtualInvokeData& entry = new_vtable[match_slot];
                entry.method_impl = vmethod;
                const_cast<metadata::RtMethodInfo*>(vmethod)->slot = static_cast<uint16_t>(match_slot);
                find_impl = true;

                for (uint16_t j = 0; j < parent->vtable_count; ++j)
                {
                    metadata::RtVirtualInvokeData& later_entry = new_vtable[j];
                    if (later_entry.method_impl == to_match_method)
                        later_entry.method_impl = vmethod;
                }
                break;
            }
        }
        if (!find_impl)
            RET_ERR(RtErr::ExecutionEngine);
    }

    // Initialize interface implementations for new slot virtuals
    if (self_interface_vtable_offset_indexes.size() > 0)
    {
        for (size_t vi = 0; vi < self_new_slot_virtual_methods.size(); ++vi)
        {
            const metadata::RtMethodInfo* vmethod = self_new_slot_virtual_methods[vi];
            for (size_t idx : self_interface_vtable_offset_indexes)
            {
                const metadata::RtInterfaceOffset& offset_info = klass->interface_vtable_offsets[idx];
                const metadata::RtClass* iface = offset_info.interface;
                for (uint16_t i = 0; i < iface->vtable_count; ++i)
                {
                    size_t final_slot = static_cast<size_t>(offset_info.offset) + i;
                    metadata::RtVirtualInvokeData& entry = new_vtable[final_slot];
                    if (metadata::MetadataCompare::is_method_signature_equal(vmethod, entry.method, true, true))
                    {
                        entry.method_impl = vmethod;
                    }
                }
            }
        }
    }

    // Initialize default implementations for new interface slots if still null
    for (size_t i = parent->vtable_count; i < new_vtable.size(); ++i)
    {
        metadata::RtVirtualInvokeData& entry = new_vtable[i];
        if (entry.method_impl)
            continue;
        bool find_impl = false;
        for (size_t j = total_hierarchy_virtual_methods.size(); j-- > 0;)
        {
            const metadata::RtMethodInfo* to_match_method = total_hierarchy_virtual_methods[j];
            if (to_match_method->slot == metadata::RT_INVALID_METHOD_SLOT)
                RET_ERR(RtErr::BadImageFormat);
            if (metadata::MetadataCompare::is_method_signature_equal(entry.method, to_match_method, true, true))
            {
                entry.method_impl = to_match_method;
                find_impl = true;
                break;
            }
        }
        if (!find_impl)
            RET_ERR(RtErr::ExecutionEngine);
    }

    if (!Class::is_abstract(klass))
    {
        for (size_t i = 0; i < new_vtable.size(); ++i)
        {
            const metadata::RtVirtualInvokeData& entry = new_vtable[i];
            if (!entry.method_impl && i != 1)
                RET_ERR(RtErr::ExecutionEngine);
        }
    }

    metadata::RtVirtualInvokeData* vtable = pool.calloc_any<metadata::RtVirtualInvokeData>(new_vtable.size());
    std::memcpy(vtable, new_vtable.data(), new_vtable.size() * sizeof(metadata::RtVirtualInvokeData));
    klass->vtable = vtable;
    klass->vtable_count = static_cast<uint16_t>(new_vtable.size());

    RET_VOID_OK();
}

metadata::RtClass* Class::get_array_element_class(const metadata::RtClass* array_class)
{
    return const_cast<metadata::RtClass*>(array_class->element_class);
}

metadata::RtClass* Class::get_nullable_underlying_class(const metadata::RtClass* klass)
{
    if (is_nullable_type(klass))
    {
        return const_cast<metadata::RtClass*>(klass->element_class);
    }
    return const_cast<metadata::RtClass*>(klass);
}

uint32_t Class::get_stack_location_size(const metadata::RtClass* klass)
{
    if (is_value_type(klass))
    {
        return get_instance_size_without_object_header(klass);
    }
    else
    {
        return PTR_SIZE;
    }
}

// Type signature resolution functions

RtResult<metadata::RtClass*> Class::get_class_from_typesig(const metadata::RtTypeSig* typeSig)
{
    auto ele_type = typeSig->ele_type;

    // Primitive types mapped to corlib classes
    switch (ele_type)
    {
    case metadata::RtElementType::Void:
        RET_OK(g_corlibTypes.cls_void);
    case metadata::RtElementType::Boolean:
        RET_OK(g_corlibTypes.cls_boolean);
    case metadata::RtElementType::Char:
        RET_OK(g_corlibTypes.cls_char);
    case metadata::RtElementType::I1:
        RET_OK(g_corlibTypes.cls_sbyte);
    case metadata::RtElementType::U1:
        RET_OK(g_corlibTypes.cls_byte);
    case metadata::RtElementType::I2:
        RET_OK(g_corlibTypes.cls_int16);
    case metadata::RtElementType::U2:
        RET_OK(g_corlibTypes.cls_uint16);
    case metadata::RtElementType::I4:
        RET_OK(g_corlibTypes.cls_int32);
    case metadata::RtElementType::U4:
        RET_OK(g_corlibTypes.cls_uint32);
    case metadata::RtElementType::I8:
        RET_OK(g_corlibTypes.cls_int64);
    case metadata::RtElementType::U8:
        RET_OK(g_corlibTypes.cls_uint64);
    case metadata::RtElementType::R4:
        RET_OK(g_corlibTypes.cls_single);
    case metadata::RtElementType::R8:
        RET_OK(g_corlibTypes.cls_double);
    case metadata::RtElementType::String:
        RET_OK(g_corlibTypes.cls_string);
    case metadata::RtElementType::TypedByRef:
        RET_OK(g_corlibTypes.cls_typedreference);
    case metadata::RtElementType::I:
        RET_OK(g_corlibTypes.cls_intptr);
    case metadata::RtElementType::U:
        RET_OK(g_corlibTypes.cls_uintptr);
    case metadata::RtElementType::Object:
        RET_OK(g_corlibTypes.cls_object);
    case metadata::RtElementType::ValueType:
    case metadata::RtElementType::Class:
        return get_class_by_type_def_gid(typeSig->data.type_def_gid);
    case metadata::RtElementType::Ptr:
        return get_ptr_class_by_element_typesig(typeSig->data.element_type);
    case metadata::RtElementType::ByRef:
        RET_ERR(RtErr::ExecutionEngine);
    case metadata::RtElementType::SZArray:
        return ArrayClass::get_szarray_class_from_element_typesig(typeSig->data.element_type);
    case metadata::RtElementType::Array:
    {
        const metadata::RtArrayType* arrType = typeSig->data.array_type;
        return ArrayClass::get_array_class_from_element_type(arrType->ele_type, arrType->rank);
    }
    case metadata::RtElementType::GenericInst:
    {
        const metadata::RtGenericClass* generic_class = typeSig->data.generic_class;
        return GenericClass::get_class(generic_class->base_type_def_gid, generic_class->class_inst);
    }
    case metadata::RtElementType::Var:
    case metadata::RtElementType::MVar:
        return get_generic_param_class_by_typesig(typeSig->data.generic_param);
    case metadata::RtElementType::FnPtr:
    {
        return get_fnptr_class_by_method_sig(typeSig->data.method_sig);
    }
    default:
        RET_ERR(RtErr::BadImageFormat);
    }
}

static utils::HashMap<const metadata::RtTypeSig*, metadata::RtClass*, metadata::TypeSigIgnoreAttrsHasher, metadata::TypeSigIgnoreAttrsEqual> g_ptrClassCache;

static const char* make_ptr_name(const char* eleName)
{
    return utils::StringUtil::concat(eleName, "*");
}

RtResult<metadata::RtClass*> Class::get_ptr_class_by_element_typesig(const metadata::RtTypeSig* eleTypeSig)
{
    auto it = g_ptrClassCache.find(eleTypeSig);
    if (it != g_ptrClassCache.end())
    {
        RET_OK(it->second);
    }

    auto retByvalByRefTypeSig = metadata::MetadataCache::get_pooled_ptr_typesigs_by_element_typesig(eleTypeSig);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL2(metadata::RtTypeSigByValRef, ptrTypeSigs, retByvalByRefTypeSig);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, eleClass, get_class_from_typesig(eleTypeSig));

    auto* ptrClass = alloc::MetadataAllocation::malloc_any_zeroed<metadata::RtClass>();
    ptrClass->image = eleClass->image;
    ptrClass->token = 0;
    ptrClass->parent = nullptr;
    ptrClass->namespaze = "";
    ptrClass->name = make_ptr_name(eleClass->name);
    ptrClass->element_class = eleClass;
    // in il2cpp, ptrClass->cast_class = eleClass, we think it is a mistake
    ptrClass->cast_class = ptrClass;
    ptrClass->flags = (uint32_t)metadata::RtTypeAttribute::Class | (eleClass->flags & (uint32_t)metadata::RtTypeAttribute::VisibilityMask);
    ptrClass->by_val = ptrTypeSigs.by_val;
    ptrClass->by_ref = ptrTypeSigs.by_ref;

    g_ptrClassCache.insert({eleTypeSig, ptrClass});
    RET_OK(ptrClass);
}

static utils::HashMap<const metadata::RtGenericParam*, metadata::RtClass*> g_genericParamClassCache;

RtResult<metadata::RtClass*> Class::get_generic_param_class_by_typesig(const metadata::RtGenericParam* genericParam)
{
    auto it = g_genericParamClassCache.find(genericParam);
    if (it != g_genericParamClassCache.end())
    {
        RET_OK(it->second);
    }
    uint32_t moduleId = metadata::RtMetadata::decode_module_id_from_gid(genericParam->gid);
    uint32_t rid = metadata::RtMetadata::decode_rid_from_gid(genericParam->gid);
    metadata::RtModuleDef* mod = metadata::RtModuleDef::get_module_by_id(moduleId);
    if (!mod)
    {
        RET_ERR(RtErr::ExecutionEngine);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, byValTypeSig, mod->get_generic_param_typesig_by_rid(rid, false));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, byRefTypeSig, mod->get_generic_param_typesig_by_rid(rid, true));

    auto* genericParamClass = alloc::MetadataAllocation::malloc_any_zeroed<metadata::RtClass>();
    genericParamClass->image = mod;
    genericParamClass->token = 0;
    genericParamClass->parent = nullptr;
    genericParamClass->namespaze = "";
    genericParamClass->name = genericParam->name;
    genericParamClass->element_class = genericParamClass;
    genericParamClass->cast_class = genericParamClass;
    genericParamClass->flags = (uint32_t)metadata::RtTypeAttribute::Public;
    genericParamClass->by_val = byValTypeSig;
    genericParamClass->by_ref = byRefTypeSig;

    g_genericParamClassCache.insert({genericParam, genericParamClass});
    RET_OK(genericParamClass);
}

static utils::HashMap<const metadata::RtMethodSig*, metadata::RtClass*, metadata::MethodSigHash, metadata::MethodSigCompare> g_fnPtrClassCache;

RtResult<metadata::RtClass*> Class::get_fnptr_class_by_method_sig(const metadata::RtMethodSig* method_sig)
{
    auto it = g_fnPtrClassCache.find(method_sig);
    if (it != g_fnPtrClassCache.end())
    {
        RET_OK(it->second);
    }
    metadata::RtClass* new_klass = alloc::MetadataAllocation::malloc_any_zeroed<metadata::RtClass>();
    new_klass->image = Assembly::get_corlib()->mod;
    new_klass->token = 0;
    new_klass->parent = nullptr;
    new_klass->namespaze = "System";
    new_klass->name = "FakeFnPtrClass";
    new_klass->element_class = new_klass;
    new_klass->cast_class = new_klass;
    new_klass->flags = (uint32_t)metadata::RtTypeAttribute::Public;

    metadata::RtMethodSig* new_method_sig = new (alloc::MetadataAllocation::malloc_any_zeroed<metadata::RtMethodSig>()) metadata::RtMethodSig(*method_sig);
    metadata::RtTypeSig* by_val = alloc::MetadataAllocation::malloc_any_zeroed<metadata::RtTypeSig>();
    by_val->ele_type = metadata::RtElementType::FnPtr;
    by_val->data.method_sig = new_method_sig;

    metadata::RtTypeSig* by_ref = alloc::MetadataAllocation::malloc_any_zeroed<metadata::RtTypeSig>();
    by_ref->ele_type = metadata::RtElementType::FnPtr;
    by_ref->data.method_sig = new_method_sig;
    by_ref->by_ref = 1;

    new_klass->by_val = by_val;
    new_klass->by_ref = by_ref;

    g_fnPtrClassCache.insert({method_sig, new_klass});
    RET_OK(new_klass);
}

metadata::RtClass* Class::get_enclosing_class(const metadata::RtClass* nestedClass)
{
    return const_cast<metadata::RtClass*>(nestedClass->declaring_class);
}

RtResult<metadata::RtClass*> Class::find_nested_class_by_name(const metadata::RtClass* enclosingClass, const char* nestedClassName, bool ignore_case)
{
    RET_ERR_ON_FAIL(initialize_nested_classes(const_cast<metadata::RtClass*>(enclosingClass)));
    for (uint16_t i = 0; i < enclosingClass->nested_class_count; ++i)
    {
        const metadata::RtClass* nestedClass = enclosingClass->nested_classes[i];
        if (ignore_case)
        {
            if (utils::StringUtil::equals_ignorecase(nestedClass->name, nestedClassName))
            {
                RET_OK(const_cast<metadata::RtClass*>(nestedClass));
            }
        }
        else
        {
            if (strcmp(nestedClass->name, nestedClassName) == 0)
            {
                RET_OK(const_cast<metadata::RtClass*>(nestedClass));
            }
        }
    }
    RET_OK(nullptr);
}

bool Class::is_assignable_from_class(const metadata::RtClass* fromClass, const metadata::RtClass* toClass)
{
    assert(has_initialized_part(fromClass, metadata::RtClassInitPart::SuperTypes));
    if (fromClass == toClass)
    {
        return true;
    }

    const metadata::RtTypeSig* fromTypeSig = get_by_val_type_sig(fromClass);
    const metadata::RtTypeSig* toTypeSig = get_by_val_type_sig(toClass);

    switch (toTypeSig->ele_type)
    {
    case metadata::RtElementType::Void:
        return false;
    case metadata::RtElementType::Object:
        return true;
    case metadata::RtElementType::String:
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
    case metadata::RtElementType::TypedByRef:
    case metadata::RtElementType::ValueType:
        return fromClass == toClass;
    case metadata::RtElementType::Class:
        // both are reference type
        return has_class_parent_fast(fromClass, toClass);
    case metadata::RtElementType::Var:
    case metadata::RtElementType::MVar:
        return false;
    case metadata::RtElementType::SZArray:
    {
        // array type
        if (fromTypeSig->ele_type != metadata::RtElementType::SZArray)
        {
            return false;
        }
        metadata::RtClass* fromEleClass = get_array_element_class(fromClass);
        metadata::RtClass* toEleClass = get_array_element_class(toClass);
        if (is_value_type(fromEleClass))
        {
            return fromEleClass == toEleClass;
        }
        return is_assignable_from(fromEleClass, toEleClass);
    }
    case metadata::RtElementType::Array:
    {
        // array type
        if (fromTypeSig->ele_type != metadata::RtElementType::Array)
        {
            return false;
        }
        if (fromTypeSig->data.array_type->rank != toTypeSig->data.array_type->rank)
        {
            return false;
        }
        metadata::RtClass* fromEleClass = get_array_element_class(fromClass);
        metadata::RtClass* toEleClass = get_array_element_class(toClass);
        if (is_value_type(fromEleClass))
        {
            return fromEleClass == toEleClass;
        }
        return is_assignable_from(fromEleClass, toEleClass);
    }
    case metadata::RtElementType::GenericInst:
    {
        if (is_value_type(toClass))
        {
            return toClass->cast_class == fromClass->cast_class;
        }
        else
        {
            return has_class_parent_fast(fromClass, toClass);
        }
    }
    case metadata::RtElementType::ByRef:
    default:
        assert(false && "Invalid element type");
        return false;
    }
}

bool Class::is_assignable_from_interface(const metadata::RtClass* fromClass, const metadata::RtClass* toClass)
{
    assert(has_initialized_part(fromClass, metadata::RtClassInitPart::SuperTypes));
    const metadata::RtClass* currentClass = fromClass;
    while (currentClass != nullptr)
    {
        for (uint16_t i = 0; i < currentClass->interface_count; ++i)
        {
            if (currentClass->interfaces[i] == toClass)
            {
                return true;
            }
        }
        currentClass = currentClass->parent;
    }
    return false;
}

bool Class::is_assignable_from(const metadata::RtClass* fromClass, const metadata::RtClass* toClass)
{
    assert(has_initialized_part(fromClass, metadata::RtClassInitPart::SuperTypes));
    if (fromClass == toClass)
    {
        return true;
    }
    else if (!is_interface(toClass))
    {
        return is_assignable_from_class(fromClass, toClass);
    }
    else
    {
        return is_assignable_from_interface(fromClass, toClass);
    }
}

bool Class::is_exception_sub_class(const metadata::RtClass* klass)
{
    return has_class_parent_fast(klass, get_corlib_types().cls_exception);
}

bool Class::is_subclass_of_initialized(const metadata::RtClass* fromClass, const metadata::RtClass* toClass, bool checkInterfaces)
{
    if (fromClass == toClass)
    {
        return true;
    }
    if (checkInterfaces)
    {
        if (is_interface(toClass))
        {
            const metadata::RtClass* currentClass = fromClass;
            while (currentClass != nullptr)
            {
                for (uint16_t i = 0; i < currentClass->interface_count; ++i)
                {
                    if (currentClass->interfaces[i] == toClass)
                    {
                        return true;
                    }
                }
                currentClass = currentClass->parent;
            }
            return false;
        }
        else
        {
            return has_class_parent_fast(fromClass, toClass);
        }
    }
    else
    {
        return has_class_parent_fast(fromClass, toClass);
    }
}

bool Class::is_pointer_element_compatible_with(const metadata::RtClass* fromClass, const metadata::RtClass* toClass)
{
    return fromClass->cast_class == toClass->cast_class;
}

size_t Class::get_gc_bitmap_size(const metadata::RtClass* klass)
{
    return 0;
}

void Class::get_gc_bitmap(const metadata::RtClass* klass, size_t* bitmaps, size_t& bitmaps_size)
{
    bitmaps_size = 0;
    return;
}

void Class::walk_ptr_classes(metadata::ClassWalkCallback callback, void* userData)
{
    for (auto& entry : g_ptrClassCache)
    {
        callback(entry.second, userData);
    }
}
} // namespace vm
} // namespace leanclr
