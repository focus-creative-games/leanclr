#pragma once

#include "core/stl_compat.h"

#include "rt_managed_types.h"
#include "runtime.h"
#include "core/rt_result.h"
#include "utils/rt_vector.h"

namespace leanclr
{
namespace vm
{
class Method
{
  public:
    // Generic inflation helpers
    static RtResult<const metadata::RtMethodInfo*> inflate(const metadata::RtMethodInfo* method_info, const metadata::RtGenericContext* gc);

    // Metadata lookups
    static RtResult<const metadata::RtMethodInfo*> get_method_by_method_def_gid(uint32_t method_def_gid);
    static uint32_t get_method_def_gid(const metadata::RtMethodInfo* method);

    // VTable helpers
    static const metadata::RtVirtualInvokeData* get_vtable_method_invoke_data(const metadata::RtClass* klass, size_t method_index);
    static metadata::RtManagedMethodPointer get_vtable_method_ptr(const metadata::RtClass* klass, size_t method_index);
    static const metadata::RtMethodInfo* get_vtable_method(const metadata::RtClass* klass, size_t method_index);
    static RtResult<const metadata::RtVirtualInvokeData*> get_interface_method_invoke_data(const metadata::RtClass* klass,
                                                                                           const metadata::RtClass* interface_klass, size_t slot);
    static RtResult<const metadata::RtMethodInfo*> get_virtual_method_impl(RtObject* obj, const metadata::RtMethodInfo* virtual_method);
    static RtResult<const metadata::RtMethodInfo*> get_virtual_method_impl_on_klass(const metadata::RtClass* klass,
                                                                                    const metadata::RtMethodInfo* virtual_method);

    // Method queries/search
    static const metadata::RtMethodInfo* find_matched_method_in_class(const metadata::RtClass* klass, const metadata::RtMethodInfo* to_match_method);
    static const metadata::RtMethodInfo* find_matched_method_in_class_by_name_and_signature(const metadata::RtClass* klass, const char* name,
                                                                                            const metadata::RtTypeSig* const* param_type_sigs,
                                                                                            size_t param_count);
    static const metadata::RtMethodInfo* find_matched_method_in_class_by_name(const metadata::RtClass* klass, const char* name);
    static const metadata::RtMethodInfo* find_matched_method_in_class_by_name_and_param_count(const metadata::RtClass* klass, const char* name,
                                                                                              size_t parameter_count);

    // Attribute/flag helpers
    static bool is_virtual(const metadata::RtMethodInfo* method);
    static bool is_devirtualed(const metadata::RtMethodInfo* method);
    static bool is_abstract(const metadata::RtMethodInfo* method);
    static bool is_instance(const metadata::RtMethodInfo* method);
    static bool is_sealed(const metadata::RtMethodInfo* method);
    static bool is_new_slot(const metadata::RtMethodInfo* method);
    static bool is_static(const metadata::RtMethodInfo* method);
    static bool is_void_return(const metadata::RtMethodInfo* method);
    static bool is_pinvoke(const metadata::RtMethodInfo* method);
    static RtResult<bool> is_intrinsic(const metadata::RtMethodInfo* method);
    static bool is_internal_call(const metadata::RtMethodInfo* method);
    static bool is_runtime_implemented(const metadata::RtMethodInfo* method);
    static bool is_public(const metadata::RtMethodInfo* method);
    static bool is_private(const metadata::RtMethodInfo* method);
    static bool is_runtime_special_method(const metadata::RtMethodInfo* method);
    static bool is_ctor_or_cctor(const metadata::RtMethodInfo* method);
    static bool is_ctor(const metadata::RtMethodInfo* method);
    static metadata::RtMethodImplAttribute get_code_type(const metadata::RtMethodInfo* method);
    static bool has_method_body(const metadata::RtMethodInfo* method);
    static bool has_this(const metadata::RtMethodSig* method_sig);

    // Parameter helpers
    static size_t get_param_count_include_this(const metadata::RtMethodInfo* method);
    static size_t get_param_count_exclude_this(const metadata::RtMethodInfo* method);
    static uint8_t get_generic_param_count(const metadata::RtMethodInfo* method);
    static bool contains_not_instantiated_generic_param(const metadata::RtMethodInfo* method);
    static size_t get_total_arg_stack_object_size(const metadata::RtMethodInfo* method);
    static size_t get_return_value_stack_object_size(const metadata::RtMethodInfo* method);
    static RtResultVoid build_method_arg_descs(metadata::RtMethodInfo* method);
    static size_t get_method_index_in_class(const metadata::RtMethodInfo* method);

    // Metadata accessors
    static RtResult<const metadata::RtMethodInfo*> inflate_method(const metadata::RtMethodInfo* method, const metadata::RtGenericContext* gc);
    static RtResult<std::optional<uint32_t>> get_parameter_token(const metadata::RtMethodInfo* method, int32_t index);
    static RtResult<vm::RtString*> get_parameter_name_by_token(metadata::RtModuleDef* ass, metadata::EncodedTokenId param_token);
    static RtResult<const char*> get_parameter_c_name_by_token(metadata::RtModuleDef* ass, metadata::EncodedTokenId param_token);
    static RtResult<std::optional<metadata::RowImplMap>> get_imp_map_info(const metadata::RtMethodInfo* method);
    static RtResult<std::optional<metadata::RtMethodBody>> get_method_body(const metadata::RtMethodInfo* method);
    static RtResult<vm::RtReflectionMethodBody*> create_reflection_method_body(const metadata::RtMethodInfo* method);
    static RtResultVoid get_parameter_modifiers(const metadata::RtMethodInfo* method, int32_t index, bool optional,
                                                utils::Vector<metadata::RtClass*>& modifiers);
};
} // namespace vm
} // namespace leanclr
