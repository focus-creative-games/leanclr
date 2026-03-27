#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemRuntimeTypeHandle
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries();

    // Get type attributes
    static RtResult<uint32_t> get_attributes(const vm::RtReflectionRuntimeType* runtime_type);

    // Get metadata token
    static RtResult<int32_t> get_metadata_token(const vm::RtReflectionRuntimeType* runtime_type);

    // Get COR element type
    static RtResult<metadata::RtElementType> get_cor_element_type(const vm::RtReflectionRuntimeType* runtime_type);

    // Check if type has instantiation
    static RtResult<bool> has_instantiation(const vm::RtReflectionRuntimeType* runtime_type);

    // Check if type is COM object
    static RtResult<bool> is_com_object(const vm::RtReflectionRuntimeType* runtime_type);

    // Check if type has references
    static RtResult<bool> has_references(metadata::RtClass* klass);

    // Get array rank
    static RtResult<int32_t> get_array_rank(const vm::RtReflectionRuntimeType* runtime_type);

    // Get element type
    static RtResult<vm::RtReflectionRuntimeType*> get_element_type(const vm::RtReflectionRuntimeType* runtime_type);

    // Check if type is generic variable
    static RtResult<bool> is_generic_variable(const vm::RtReflectionRuntimeType* runtime_type);

    // Get base type
    static RtResult<vm::RtReflectionRuntimeType*> get_base_type(const vm::RtReflectionRuntimeType* runtime_type);

    // Check if type is generic type definition
    static RtResult<bool> is_generic_type_definition(const vm::RtReflectionRuntimeType* runtime_type);

    // Get generic parameter info
    static RtResult<const metadata::RtGenericParam*> get_generic_parameter_info(const vm::RtReflectionRuntimeType* runtime_type);

    // Check if child type is subclass of parent type
    static RtResult<bool> is_subclass_of(const metadata::RtTypeSig* child_type, const metadata::RtTypeSig* parent_type);

    // Check if type is ByRefLike
    static RtResult<bool> is_by_ref_like(const vm::RtReflectionRuntimeType* runtime_type);

    // Check if type is assignable from another type
    static RtResult<bool> type_is_assignable_from(vm::RtReflectionRuntimeType* to_type, vm::RtReflectionRuntimeType* from_type);

    // Get assembly
    static RtResult<vm::RtReflectionAssembly*> get_assembly(const vm::RtReflectionRuntimeType* runtime_type);

    // Check if object is instance of type
    static RtResult<bool> is_instance_of_type(const vm::RtReflectionRuntimeType* runtime_type, vm::RtObject* obj);

    // Get generic type definition implementation
    static RtResult<vm::RtReflectionRuntimeType*> get_generic_type_definition_impl(vm::RtReflectionRuntimeType* runtime_type);

    // Get module
    static RtResult<vm::RtReflectionModule*> get_module(const vm::RtReflectionRuntimeType* runtime_type);

    // Resolve type from name
    static RtResult<vm::RtReflectionType*> internal_from_name(vm::RtString* name, int32_t* stack_crawl_mark, vm::RtReflectionAssembly* assembly,
                                                              bool throw_on_error, bool ignore_case, bool reflection_only);
};

} // namespace icalls
} // namespace leanclr
