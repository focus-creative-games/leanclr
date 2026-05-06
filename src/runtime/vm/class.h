#pragma once

#include "rt_managed_types.h"

namespace leanclr
{
namespace vm
{
struct CorLibTypes
{
    metadata::RtClass* cls_void;
    metadata::RtClass* cls_boolean;
    metadata::RtClass* cls_char;
    metadata::RtClass* cls_byte;
    metadata::RtClass* cls_sbyte;
    metadata::RtClass* cls_int16;
    metadata::RtClass* cls_uint16;
    metadata::RtClass* cls_int32;
    metadata::RtClass* cls_uint32;
    metadata::RtClass* cls_int64;
    metadata::RtClass* cls_uint64;
    metadata::RtClass* cls_single;
    metadata::RtClass* cls_double;
    metadata::RtClass* cls_intptr;
    metadata::RtClass* cls_uintptr;

    metadata::RtClass* cls_object;
    metadata::RtClass* cls_valuetype;
    metadata::RtClass* cls_string;
    metadata::RtClass* cls_enum;
    metadata::RtClass* cls_array;
    metadata::RtClass* cls_delegate;
    metadata::RtClass* cls_multicastdelegate;
    // metadata::RtClass* cls_delegatedata;
    metadata::RtClass* cls_typedreference;
    metadata::RtClass* cls_systemtype;
    metadata::RtClass* cls_runtimetype;
    metadata::RtClass* cls_nullable;
    metadata::RtClass* cls_icollection;
    metadata::RtClass* cls_ienumerable;
    metadata::RtClass* cls_ilist;
    metadata::RtClass* cls_ienumerator;
    metadata::RtClass* cls_ilist_generic;
    metadata::RtClass* cls_icollection_generic;
    metadata::RtClass* cls_ienumerable_generic;
    metadata::RtClass* cls_ireadonlylist_generic;
    metadata::RtClass* cls_ireadonlycollection_generic;
    metadata::RtClass* cls_ienumerator_generic;

    metadata::RtClass* cls_exception;
    metadata::RtClass* cls_arithmetic_exception;
    metadata::RtClass* cls_division_by_zero_exception;
    metadata::RtClass* cls_execution_engine_exception;
    metadata::RtClass* cls_overflow_exception;
    metadata::RtClass* cls_stack_overflow_exception;
    metadata::RtClass* cls_argument_exception;
    metadata::RtClass* cls_argument_null_exception;
    metadata::RtClass* cls_argument_out_of_range_exception;
    metadata::RtClass* cls_type_load_exception;
    metadata::RtClass* cls_index_out_of_range_exception;
    metadata::RtClass* cls_invalid_cast_exception;
    metadata::RtClass* cls_missing_field_exception;
    metadata::RtClass* cls_missing_method_exception;
    metadata::RtClass* cls_null_reference_exception;
    metadata::RtClass* cls_array_type_mismatch_exception;
    metadata::RtClass* cls_out_of_memory_exception;
    metadata::RtClass* cls_bad_image_format_exception;
    metadata::RtClass* cls_entry_point_not_found_exception;
    metadata::RtClass* cls_missing_member_exception;
    metadata::RtClass* cls_not_supported_exception;
    metadata::RtClass* cls_not_implemented_exception;
    // metadata::RtClass* cls_type_unloaded_exception;
    metadata::RtClass* cls_type_initialization_exception;
    metadata::RtClass* cls_target_exception;
    metadata::RtClass* cls_target_invocation_exception;
    metadata::RtClass* cls_target_parameter_count_exception;

    metadata::RtClass* cls_attribute;
    metadata::RtClass* cls_customattributedata;
    metadata::RtClass* cls_customattribute_typed_argument;
    metadata::RtClass* cls_customattribute_named_argument;

    metadata::RtClass* cls_intrinsic;

    metadata::RtClass* cls_reflection_assembly;
    metadata::RtClass* cls_reflection_module;
    metadata::RtClass* cls_reflection_field;
    metadata::RtClass* cls_reflection_method;
    metadata::RtClass* cls_reflection_constructor;
    metadata::RtClass* cls_reflection_property;
    metadata::RtClass* cls_reflection_event;
    metadata::RtClass* cls_reflection_parameter;
    metadata::RtClass* cls_reflection_memberinfo;
    metadata::RtClass* cls_reflection_methodbody;
    metadata::RtClass* cls_reflection_exceptionhandlingclause;
    metadata::RtClass* cls_reflection_localvariableinfo;

    metadata::RtClass* cls_appdomain;
    metadata::RtClass* cls_appdomain_setup;
    metadata::RtClass* cls_appcontext;
    metadata::RtClass* cls_thread;
    metadata::RtClass* cls_internal_thread;

    metadata::RtClass* cls_marshal_as;
    metadata::RtClass* cls_byreflike;

    metadata::RtClass* cls_culturedata;
    metadata::RtClass* cls_cultureinfo;
    metadata::RtClass* cls_datetimeformatinfo;
    metadata::RtClass* cls_numberformatinfo;
    metadata::RtClass* cls_regioninfo;
    metadata::RtClass* cls_calendardata;

    metadata::RtClass* cls_stackframe;
};

class Class
{
  public:
    static RtResultVoid initialize();
    static RtResultVoid init_corlib_classes(metadata::RtModuleDef* corlib);
    static const CorLibTypes& get_corlib_types();
    static RtResultVoid verify_integrity_of_corlib_classes();

    static RtResult<metadata::RtClass*> init_class_of_type_def(metadata::RtModuleDef* moduleDef, uint32_t rid);

    static RtResultVoid initialize_all(metadata::RtClass* klass);
    static RtResultVoid initialize_super_types(metadata::RtClass* klass);
    static RtResultVoid initialize_interfaces(metadata::RtClass* klass);
    static RtResultVoid initialize_nested_classes(metadata::RtClass* klass);
    static RtResultVoid initialize_fields(metadata::RtClass* klass);
    static RtResultVoid initialize_methods(metadata::RtClass* klass);
    static RtResultVoid initialize_properties(metadata::RtClass* klass);
    static RtResultVoid initialize_events(metadata::RtClass* klass);
    static RtResultVoid initialize_vtables(metadata::RtClass* klass);

    static bool has_initialized_part(const metadata::RtClass* klass, metadata::RtClassInitPart parts);
    static void set_initialized_part(metadata::RtClass* klass, metadata::RtClassInitPart parts);
    static bool try_set_initialized_part(metadata::RtClass* klass, metadata::RtClassInitPart parts);
    static metadata::RtClassFamily get_family(const metadata::RtClass* klass);

    static uint32_t get_instance_size_without_object_header(const metadata::RtClass* cls)
    {
        assert(has_initialized_part(cls, metadata::RtClassInitPart::Field));
        return cls->instance_size_without_header;
    }

    static uint32_t get_instance_size_with_object_header(const metadata::RtClass* cls)
    {
        assert(has_initialized_part(cls, metadata::RtClassInitPart::Field));
        return cls->instance_size_without_header + vm::RT_OBJECT_HEADER_SIZE;
    }

    static RtResult<metadata::RtClass*> get_class_from_typesig(const metadata::RtTypeSig* type_sig);

    static RtResult<metadata::RtClass*> get_class_by_type_def_gid(uint32_t gid);

    // Transliterated EEClass member functions
    static bool is_value_type(const metadata::RtClass* klass);
    static bool is_reference_type(const metadata::RtClass* klass);
    static bool is_enum_type(const metadata::RtClass* klass);
    static bool is_nullable_type(const metadata::RtClass* klass);
    static bool is_multicastdelegate_subclass(const metadata::RtClass* klass);
    static bool get_has_references(const metadata::RtClass* klass);
    static void set_has_references(metadata::RtClass* klass);
    static bool is_blittable(const metadata::RtClass* klass);
    static bool is_array_or_szarray(const metadata::RtClass* klass);
    static bool is_ptr(const metadata::RtClass* klass);
    static bool has_static_constructor(const metadata::RtClass* klass);
    static bool has_finalizer(const metadata::RtClass* klass);
    static bool is_interface(const metadata::RtClass* klass);
    static bool is_abstract(const metadata::RtClass* klass);
    static bool is_sealed(const metadata::RtClass* klass);
    static bool is_generic(const metadata::RtClass* klass);
    static bool is_generic_inst(const metadata::RtClass* klass);
    static bool is_cctor_not_finished(const metadata::RtClass* klass);
    static void set_cctor_finished(metadata::RtClass* klass);
    static const metadata::RtTypeSig* get_by_val_type_sig(const metadata::RtClass* klass);
    static const metadata::RtTypeSig* get_by_ref_type_sig(const metadata::RtClass* klass);
    static bool is_object_class(const metadata::RtClass* klass);
    static bool is_string_class(const metadata::RtClass* klass);
    static bool is_szarray_class(const metadata::RtClass* klass);
    static uint8_t get_rank(const metadata::RtClass* klass);
    static metadata::RtElementType get_element_type(const metadata::RtClass* klass);
    static metadata::RtElementType get_enum_element_type(const metadata::RtClass* klass);
    static bool is_by_ref(const metadata::RtClass* klass);
    static bool is_public(const metadata::RtClass* klass);
    static bool is_nested_public(const metadata::RtClass* klass);
    static bool is_initialized(const metadata::RtClass* klass);
    static bool is_explicit_layout(const metadata::RtClass* klass);
    static RtResult<bool> is_by_ref_like(const metadata::RtClass* klass);

    // Extended query and state management functions
    static uint32_t get_type_def_gid(const metadata::RtClass* klass);
    static metadata::RtGenericContainerContext get_generic_container_context(const metadata::RtClass* klass);
    static metadata::RtClass* get_generic_base_klass_of_generic_class(const metadata::RtClass* klass);
    static metadata::RtClass* get_generic_base_klass_or_self(const metadata::RtClass* klass);
    static bool has_class_parent_fast(const metadata::RtClass* klass, const metadata::RtClass* parent);

    // Reflection/search functions
    static const metadata::RtFieldInfo* get_field_for_name(const metadata::RtClass* klass, const char* name, bool search_parent);
    static const metadata::RtFieldInfo* get_field_for_name(const metadata::RtClass* klass, const char* name, uint32_t name_len, bool search_parent);
    static const metadata::RtMethodInfo* get_method_for_name(const metadata::RtClass* klass, const char* name, int32_t argument_count, bool search_parent);
    static const metadata::RtPropertyInfo* get_property_for_name(const metadata::RtClass* klass, const char* name, bool search_parent);
    static const metadata::RtPropertyInfo* get_property_for_name(const metadata::RtClass* klass, const char* name, uint32_t name_len, bool search_parent);
    static const metadata::RtEventInfo* get_event_for_name(const metadata::RtClass* klass, const char* name, bool search_parent);
    static const metadata::RtMethodInfo* get_static_constructor(const metadata::RtClass* klass);

    // Utility helper functions
    static metadata::RtClass* get_array_element_class(const metadata::RtClass* array_class);
    static metadata::RtClass* get_nullable_underlying_class(const metadata::RtClass* klass);
    static uint32_t get_stack_location_size(const metadata::RtClass* klass);

    // Type signature resolution
    static RtResult<metadata::RtClass*> get_ptr_class_by_element_typesig(const metadata::RtTypeSig* ele_type_sig);
    static RtResult<metadata::RtClass*> get_generic_param_class_by_typesig(const metadata::RtGenericParam* generic_param);
    static RtResult<metadata::RtClass*> get_fnptr_class_by_method_sig(const metadata::RtMethodSig* method_sig);

    // Nested class lookup
    // static RtResult<uint32_t> find_nested_class_gid_by_name(metadata::RtClass* enclosing_class, const char* nested_class_name);
    static metadata::RtClass* get_enclosing_class(const metadata::RtClass* nestedClass);
    static RtResult<metadata::RtClass*> find_nested_class_by_name(const metadata::RtClass* enclosingClass, const char* nestedClassName, bool ignore_case);

    // Type assignability checking functions
    static bool is_assignable_from_class(const metadata::RtClass* from_class, const metadata::RtClass* to_class);
    static bool is_assignable_from_generic_parameter_convariant(const metadata::RtClass* from_class, const metadata::RtClass* to_class,
                                                                const metadata::RtClass* implement_class);
    static bool is_assignable_from_generic_interface(const metadata::RtClass* from_class, const metadata::RtClass* to_class);
    static bool is_assignable_from_interface(const metadata::RtClass* from_class, const metadata::RtClass* to_class);
    static bool is_assignable_from(const metadata::RtClass* from_class, const metadata::RtClass* to_class);
    static bool is_exception_sub_class(const metadata::RtClass* klass);
    static bool is_subclass_of_initialized(const metadata::RtClass* from_class, const metadata::RtClass* to_class, bool check_interfaces);
    static bool is_pointer_element_compatible_with(const metadata::RtClass* from_class, const metadata::RtClass* to_class);

    static size_t get_gc_bitmap_size(const metadata::RtClass* klass);
    static void get_gc_bitmap(const metadata::RtClass* klass, size_t* bitmaps, size_t& bitmaps_size);

    static void walk_ptr_classes(metadata::ClassWalkCallback callback, void* userData);

  private:
    static RtResultVoid setup_interfaces_typedef(metadata::RtClass* klass);
    static RtResultVoid setup_nested_classes_typedef(metadata::RtClass* klass);
    static RtResultVoid setup_fields_typedef(metadata::RtClass* klass);
    static RtResultVoid setup_field_layout(metadata::RtClass* klass);
    static RtResultVoid setup_static_field_data(metadata::RtClass* klass);
    static RtResultVoid setup_methods_typedef(metadata::RtClass* klass);
    static RtResultVoid build_methods_arg_descs(metadata::RtClass* klass);
    static RtResultVoid setup_properties_typedef(metadata::RtClass* klass);
    static RtResultVoid setup_events_typedef(metadata::RtClass* klass);
    static RtResultVoid setup_vtable_typedef(metadata::RtClass* klass);
    static bool is_assignable_from_generic_parameter_convariant0(const metadata::RtClass* from_class, const metadata::RtClass* to_class,
                                                                 bool implemented_in_array);
};
} // namespace vm
} // namespace leanclr
