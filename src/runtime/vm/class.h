#pragma once

#include <cassert>

#include "rt_managed_types.h"
#include "utils/mem_op.h"

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

extern CorLibTypes g_corlibTypes;

class Class
{
  public:
    static RtResultVoid initialize();
    static RtResultVoid init_corlib_classes(metadata::RtModuleDef* corlib);

    static const CorLibTypes& get_corlib_types()
    {
        return g_corlibTypes;
    }

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

    static bool has_initialized_part(const metadata::RtClass* klass, metadata::RtClassInitPart parts)
    {
        return (klass->init_flags & (uint32_t)parts) != 0;
    }

    static void set_initialized_part(metadata::RtClass* klass, metadata::RtClassInitPart parts)
    {
        klass->init_flags |= (uint32_t)parts;
    }

    static bool try_set_initialized_part(metadata::RtClass* klass, metadata::RtClassInitPart parts)
    {
        if ((klass->init_flags & (uint32_t)parts) == 0)
        {
            klass->init_flags |= (uint32_t)parts;
            return true;
        }
        return false;
    }

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
    static bool is_value_typedef_or_generic_inst(const metadata::RtClass* klass)
    {
        return (klass->extra_flags & (uint32_t)metadata::RtClassExtraAttribute::ValueType) != 0;
    }

    static bool is_value_type(const metadata::RtClass* klass)
    {
        // fnptr, pointer are tried to be value type, but they are not.
        return (klass->extra_flags & (uint32_t)metadata::RtClassExtraAttribute::ReferenceType) == 0;
    }

    static bool is_reference_type(const metadata::RtClass* klass)
    {
        return (klass->extra_flags & (uint32_t)metadata::RtClassExtraAttribute::ReferenceType) != 0;
    }

    static bool is_enum_type(const metadata::RtClass* klass)
    {
        return (klass->extra_flags & (uint32_t)metadata::RtClassExtraAttribute::Enum) != 0;
    }

    static bool is_nullable_type(const metadata::RtClass* klass)
    {
        return (klass->extra_flags & (uint32_t)metadata::RtClassExtraAttribute::Nullable) != 0;
    }

    static bool is_multicastdelegate_subclass(const metadata::RtClass* klass)
    {
        return klass->parent == g_corlibTypes.cls_multicastdelegate;
    }

    static bool get_has_references(const metadata::RtClass* klass)
    {
        return (klass->extra_flags & (uint32_t)metadata::RtClassExtraAttribute::HasReferences) != 0;
    }

    static void set_has_references(metadata::RtClass* klass)
    {
        klass->extra_flags |= (uint32_t)metadata::RtClassExtraAttribute::HasReferences;
    }

    static bool is_blittable(const metadata::RtClass* klass)
    {
        return (klass->extra_flags & (uint32_t)metadata::RtClassExtraAttribute::HasReferences) == 0;
    }

    static bool is_array_or_szarray(const metadata::RtClass* klass)
    {
        return (klass->extra_flags & (uint32_t)metadata::RtClassExtraAttribute::ArrayOrSZArray) != 0;
    }

    static bool is_ptr(const metadata::RtClass* klass)
    {
        return klass->by_val->ele_type == metadata::RtElementType::Ptr;
    }

    static bool has_static_constructor(const metadata::RtClass* klass)
    {
        return (klass->extra_flags & (uint32_t)metadata::RtClassExtraAttribute::HasStaticConstructor) != 0;
    }

    static bool has_finalizer(const metadata::RtClass* klass)
    {
        assert(has_initialized_part(klass, metadata::RtClassInitPart::VirtualTable));
        assert(s_finalizer_vtable_index != -1);
        if (klass->vtable_count > s_finalizer_vtable_index)
        {
            const metadata::RtMethodInfo* finalizer = klass->vtable[s_finalizer_vtable_index].method_impl;
            return !is_object_class(finalizer->parent);
        }
        return false;
    }

    static const metadata::RtMethodInfo* get_finalizer(const metadata::RtClass* klass)
    {
        assert(has_initialized_part(klass, metadata::RtClassInitPart::VirtualTable));
        assert(s_finalizer_vtable_index != -1);
        if (klass->vtable_count > s_finalizer_vtable_index)
        {
            const metadata::RtMethodInfo* finalizer = klass->vtable[s_finalizer_vtable_index].method_impl;
            return finalizer;
        }
        return nullptr;
    }

    static bool is_interface(const metadata::RtClass* klass)
    {
        return (klass->flags & (uint32_t)metadata::RtTypeAttribute::Interface) != 0;
    }

    static bool is_abstract(const metadata::RtClass* klass)
    {
        return (klass->flags & (uint32_t)metadata::RtTypeAttribute::Abstract) != 0;
    }

    static bool is_sealed(const metadata::RtClass* klass)
    {
        return (klass->flags & (uint32_t)metadata::RtTypeAttribute::Sealed) != 0;
    }

    static bool is_generic(const metadata::RtClass* klass)
    {
        return klass->generic_container != nullptr;
    }

    static bool is_generic_inst(const metadata::RtClass* klass)
    {
        return klass->by_val->ele_type == metadata::RtElementType::GenericInst;
    }

    static bool is_cctor_not_finished(const metadata::RtClass* klass)
    {
        return klass->cctor_status == metadata::RtCCtorStatus::NotInited;
    }

    static bool is_cctor_not_finished_hierarchy(const metadata::RtClass* klass)
    {
        return klass->cctor_status < metadata::RtCCtorStatus::InitHierarchy;
    }

    static void set_cctor_initing(metadata::RtClass* klass)
    {
        assert(klass->cctor_status == metadata::RtCCtorStatus::NotInited);
        klass->cctor_status = metadata::RtCCtorStatus::Initing;
    }

    static void set_cctor_finished(metadata::RtClass* klass)
    {
        assert(klass->cctor_status < metadata::RtCCtorStatus::InitSelf);
        klass->cctor_status = metadata::RtCCtorStatus::InitSelf;
    }

    static void set_cctor_finished_hierarchy(metadata::RtClass* klass)
    {
        assert(klass->cctor_status < metadata::RtCCtorStatus::InitHierarchy);
        klass->cctor_status = metadata::RtCCtorStatus::InitHierarchy;
    }

    static const metadata::RtTypeSig* get_by_val_type_sig(const metadata::RtClass* klass)
    {
        return klass->by_val;
    }

    static const metadata::RtTypeSig* get_by_ref_type_sig(const metadata::RtClass* klass)
    {
        return klass->by_ref;
    }

    static bool is_object_class(const metadata::RtClass* klass)
    {
        return klass->by_val->ele_type == metadata::RtElementType::Object;
    }

    static bool is_string_class(const metadata::RtClass* klass)
    {
        return klass->by_val->ele_type == metadata::RtElementType::String;
    }

    static bool is_szarray_class(const metadata::RtClass* klass)
    {
        return klass->by_val->ele_type == metadata::RtElementType::SZArray;
    }

    static uint8_t get_rank(const metadata::RtClass* klass)
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

    static metadata::RtElementType get_element_type(const metadata::RtClass* klass)
    {
        return klass->by_val->ele_type;
    }

    static metadata::RtElementType get_enum_element_type(const metadata::RtClass* klass)
    {
        assert(is_enum_type(klass));
        return get_element_type(klass->element_class);
    }

    static bool is_by_ref(const metadata::RtClass* klass)
    {
        return klass->by_val->by_ref;
    }

    static bool is_not_public(const metadata::RtClass* klass)
    {
        uint32_t visibility = klass->flags & (uint32_t)metadata::RtTypeAttribute::VisibilityMask;
        return visibility == (uint32_t)metadata::RtTypeAttribute::NotPublic;
    }

    static bool is_public(const metadata::RtClass* klass)
    {
        uint32_t visibility = klass->flags & (uint32_t)metadata::RtTypeAttribute::VisibilityMask;
        return visibility == (uint32_t)metadata::RtTypeAttribute::Public;
    }

    static bool is_nested_public(const metadata::RtClass* klass)
    {
        uint32_t visibility = klass->flags & (uint32_t)metadata::RtTypeAttribute::VisibilityMask;
        return visibility == (uint32_t)metadata::RtTypeAttribute::NestedPublic;
    }

    static bool is_nested_private(const metadata::RtClass* klass)
    {
        uint32_t visibility = klass->flags & (uint32_t)metadata::RtTypeAttribute::VisibilityMask;
        return visibility == (uint32_t)metadata::RtTypeAttribute::NestedPrivate;
    }

    static bool is_nested_family(const metadata::RtClass* klass)
    {
        uint32_t visibility = klass->flags & (uint32_t)metadata::RtTypeAttribute::VisibilityMask;
        return visibility == (uint32_t)metadata::RtTypeAttribute::NestedFamily;
    }

    static bool is_nested_assembly(const metadata::RtClass* klass)
    {
        uint32_t visibility = klass->flags & (uint32_t)metadata::RtTypeAttribute::VisibilityMask;
        return visibility == (uint32_t)metadata::RtTypeAttribute::NestedAssembly;
    }

    static bool is_nested_fam_and_assem(const metadata::RtClass* klass)
    {
        uint32_t visibility = klass->flags & (uint32_t)metadata::RtTypeAttribute::VisibilityMask;
        return visibility == (uint32_t)metadata::RtTypeAttribute::NestedFamAndAssem;
    }

    static bool is_nested_fam_or_assem(const metadata::RtClass* klass)
    {
        uint32_t visibility = klass->flags & (uint32_t)metadata::RtTypeAttribute::VisibilityMask;
        return visibility == (uint32_t)metadata::RtTypeAttribute::NestedFamOrAssem;
    }

    static bool is_initialized(const metadata::RtClass* klass)
    {
        return (klass->init_flags & (uint32_t)metadata::RtClassInitPart::All) != 0;
    }

    static bool is_explicit_layout(const metadata::RtClass* klass)
    {
        return (klass->flags & (uint32_t)metadata::RtTypeAttribute::ExplicitLayout) != 0;
    }

    static RtResult<bool> is_by_ref_like(const metadata::RtClass* klass);

    // Extended query and state management functions
    static uint32_t get_type_def_gid(const metadata::RtClass* klass)
    {
        return klass->by_val->data.type_def_gid;
    }

    static metadata::RtGenericContainerContext get_generic_container_context(const metadata::RtClass* klass)
    {
        return metadata::RtGenericContainerContext{klass->generic_container, nullptr};
    }

    static metadata::RtClass* get_generic_base_klass_of_generic_class(const metadata::RtClass* klass);

    static metadata::RtClass* get_generic_base_klass_or_self(const metadata::RtClass* klass)
    {
        if (is_generic_inst(klass))
        {
            return get_generic_base_klass_of_generic_class(klass);
        }
        return const_cast<metadata::RtClass*>(klass);
    }

    static bool has_class_parent_fast(const metadata::RtClass* klass, const metadata::RtClass* parent)
    {
        assert(has_initialized_part(klass, metadata::RtClassInitPart::SuperTypes));
        return parent->hierarchy_depth <= klass->hierarchy_depth && klass->super_types[parent->hierarchy_depth] == parent;
    }

    static void collect_instance_fields(const metadata::RtClass* klass, utils::Vector<const metadata::RtFieldInfo*>& instanceFields, bool inherit);

    // Reflection/search functions
    static const metadata::RtFieldInfo* get_field_for_name(const metadata::RtClass* klass, const char* name, bool search_parent);
    static const metadata::RtFieldInfo* get_field_for_name(const metadata::RtClass* klass, const char* name, uint32_t name_len, bool search_parent);
    static const metadata::RtMethodInfo* get_method_for_name(const metadata::RtClass* klass, const char* name, int32_t argument_count, bool search_parent);
    static const metadata::RtPropertyInfo* get_property_for_name(const metadata::RtClass* klass, const char* name, bool search_parent);
    static const metadata::RtPropertyInfo* get_property_for_name(const metadata::RtClass* klass, const char* name, uint32_t name_len, bool search_parent);
    static const metadata::RtEventInfo* get_event_for_name(const metadata::RtClass* klass, const char* name, bool search_parent);
    static const metadata::RtMethodInfo* get_static_constructor(const metadata::RtClass* klass);

    // Utility helper functions
    static metadata::RtClass* get_array_element_class(const metadata::RtClass* array_class)
    {
        return const_cast<metadata::RtClass*>(array_class->element_class);
    }

    static metadata::RtClass* get_nullable_underlying_class(const metadata::RtClass* klass)
    {
        if (is_nullable_type(klass))
        {
            return const_cast<metadata::RtClass*>(klass->element_class);
        }
        return const_cast<metadata::RtClass*>(klass);
    }

    static uint32_t get_stack_location_size(const metadata::RtClass* klass)
    {
        if (is_value_type(klass))
        {
            return get_instance_size_without_object_header(klass);
        }
        return static_cast<uint32_t>(sizeof(void*));
    }

    // Type signature resolution
    static RtResult<metadata::RtClass*> get_ptr_class_by_element_typesig(const metadata::RtTypeSig* ele_type_sig);
    static RtResult<metadata::RtClass*> get_generic_param_class_by_typesig(const metadata::RtGenericParam* generic_param);
    static RtResult<metadata::RtClass*> get_fnptr_class_by_method_sig(const metadata::RtMethodSig* method_sig);

    // Nested class lookup
    // static RtResult<uint32_t> find_nested_class_gid_by_name(metadata::RtClass* enclosing_class, const char* nested_class_name);
    static metadata::RtClass* get_enclosing_class(const metadata::RtClass* nestedClass)
    {
        return const_cast<metadata::RtClass*>(nestedClass->declaring_class);
    }

    static RtResult<metadata::RtClass*> find_nested_class_by_name(const metadata::RtClass* enclosingClass, const char* nestedClassName, bool ignore_case);

    // Type assignability checking functions
    static bool is_assignable_from_class(const metadata::RtClass* from_class, const metadata::RtClass* to_class);
    static bool is_assignable_from_generic_parameter_convariant(const metadata::RtClass* from_class, const metadata::RtClass* to_class,
                                                                const metadata::RtClass* implement_class);
    static bool is_assignable_from_generic_interface(const metadata::RtClass* from_class, const metadata::RtClass* to_class);
    static bool is_assignable_from_interface(const metadata::RtClass* from_class, const metadata::RtClass* to_class);
    static bool is_assignable_from(const metadata::RtClass* from_class, const metadata::RtClass* to_class);

    static bool is_exception_sub_class(const metadata::RtClass* klass)
    {
        return has_class_parent_fast(klass, get_corlib_types().cls_exception);
    }

    static bool is_subclass_of_initialized(const metadata::RtClass* from_class, const metadata::RtClass* to_class, bool check_interfaces);

    static bool is_pointer_element_compatible_with(const metadata::RtClass* from_class, const metadata::RtClass* to_class)
    {
        return from_class->cast_class == to_class->cast_class;
    }

    static constexpr size_t kFirstGCBitmapBitIndex = RT_OBJECT_HEADER_SIZE / sizeof(void*);
    static constexpr size_t kBitsPerWord = sizeof(size_t) * 8;

    static size_t get_gc_bitmap_size(const metadata::RtClass* klass)
    {
        // bitmap_size is byte count aligned with size_t
        return static_cast<size_t>(klass->gc_bitmap_word_count) * sizeof(size_t);
    }

    static void get_gc_bitmap(const metadata::RtClass* klass, size_t* bitmaps, size_t& bitmaps_size)
    {
        bitmaps_size = get_gc_bitmap_size(klass);
        std::memcpy(bitmaps, klass->gc_bitmap, bitmaps_size);
    }

    static bool has_reference_at_offset_includes_object_header(const metadata::RtClass* klass, size_t offset_includes_object_header)
    {
        assert(offset_includes_object_header % sizeof(void*) == 0);
        size_t bit_index = offset_includes_object_header / sizeof(void*);
        return get_gc_bitmap_bit(klass->gc_bitmap, bit_index);
    }

    static bool has_reference_at_bitmap_bit_index(const metadata::RtClass* klass, size_t bitmap_bit_index)
    {
        return get_gc_bitmap_bit(klass->gc_bitmap, bitmap_bit_index);
    }

    static void walk_ptr_classes(metadata::ClassWalkCallback callback, void* userData);

    static const utils::Vector<const metadata::RtClass*>& get_all_classes_with_static_data();

  private:
    static int32_t s_finalizer_vtable_index;
    static RtResultVoid setup_finalizer_vtable_index();

    static bool get_gc_bitmap_bit(const size_t* bitmap, size_t bit_index)
    {
        return bitmap[bit_index / Class::kBitsPerWord] & (static_cast<size_t>(1) << (bit_index % Class::kBitsPerWord));
    }

    static void set_gc_bitmap_bit(size_t* bitmap, size_t bit_index)
    {
        bitmap[bit_index / Class::kBitsPerWord] |= static_cast<size_t>(1) << (bit_index % Class::kBitsPerWord);
    }

    static RtResultVoid setup_interfaces_typedef(metadata::RtClass* klass);
    static RtResultVoid setup_nested_classes_typedef(metadata::RtClass* klass);
    static RtResultVoid setup_fields_typedef(metadata::RtClass* klass);
    static RtResultVoid setup_field_layout(metadata::RtClass* klass);
    static RtResultVoid setup_gc_bitmap(metadata::RtClass* klass);
    static RtResultVoid setup_instance_gc_bitmap(metadata::RtClass* klass);
    static RtResultVoid setup_static_gc_bitmap(metadata::RtClass* klass);
    static RtResultVoid setup_instance_gc_bitmap_impl(metadata::RtClass* klass, size_t* bitmap, size_t& max_bitmap_index);
    static RtResultVoid setup_static_gc_bitmap_impl(metadata::RtClass* klass, size_t* bitmap, size_t& max_bitmap_index);
    static RtResultVoid setup_gc_bitmap_for_field(const metadata::RtFieldInfo* field, bool for_static, size_t* bitmap, size_t& max_bitmap_index);
    static RtResultVoid finalize_gc_bitmap(size_t** dest_bitmap, uint16_t* dest_word_count, size_t* scratch_bitmap, size_t max_bitmap_index,
                                           size_t max_bitmap_bit_count, bool require_nonempty);
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
