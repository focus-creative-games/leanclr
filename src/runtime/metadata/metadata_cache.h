#pragma once

#include "rt_metadata.h"

namespace leanclr
{
namespace metadata
{

struct RtTypeSigByValRef
{
    const RtTypeSig* by_val;
    const RtTypeSig* by_ref;
};

class MetadataCache
{
  public:
    static void initialize();

    // Corlib and pooled helpers
    static const RtGenericParam* get_unspecific_generic_param(uint32_t index, bool is_method);
    static RtResult<const RtTypeSig*> get_pooled_typesig(const RtTypeSig& typesig);
    static RtResult<const RtGenericInst*> get_pooled_generic_inst(const RtTypeSig* const* genericArgs, uint8_t genericArgCount);
    static const RtGenericClass* get_pooled_generic_class(uint32_t baseTypeDefGid, const RtGenericInst* classInst);
    static const RtGenericMethod* get_pooled_generic_method(uint32_t methodDefGid, const RtGenericInst* classInst, const RtGenericInst* methodInst);

    // Pointer types
    static RtResult<RtTypeSigByValRef> get_pooled_ptr_typesigs_by_element_typesig(const RtTypeSig* elementType);
    static RtResult<const RtTypeSig*> get_pooled_ptr_typesig_by_element_typesig(const RtTypeSig* elementType, bool byRef);

    // SZArray types
    static RtResult<const RtArrayType*> get_pooled_array_type(const RtTypeSig* eleTypeSig, uint8_t rank);
    static RtResult<RtTypeSigByValRef> get_pooled_szarray_typesigs_by_element_typesig(const RtTypeSig* elementType);
    static RtResult<const RtTypeSig*> get_pooled_szarray_typesig_by_element_typesig(const RtTypeSig* elementType, bool byRef);

    // Multidimensional Array types
    static RtResult<RtTypeSigByValRef> get_pooled_array_typesigs_by_element_typesig(const RtTypeSig* elementType, uint8_t rank);
    static RtResult<const RtTypeSig*> get_pooled_array_typesig_by_element_typesig(const RtTypeSig* elementType, uint8_t rank, bool byRef);

    static void walk_generic_classes(ClassWalkCallback callback, void* userData);
};
} // namespace metadata
} // namespace leanclr
