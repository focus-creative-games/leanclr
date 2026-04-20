#pragma once

#include "rt_metadata.h"
#include "utils/rt_vector.h"

namespace leanclr
{
namespace metadata
{
class GenericMetadata
{
  public:
    // Inflate a single type signature with generic context
    static RtResult<const RtTypeSig*> inflate_typesig(const RtTypeSig* typesig, const RtGenericContext* genericContext);
    static RtResultVoid inflate_typesigs(utils::Vector<const RtTypeSig*> typesigs, const RtGenericContext* genericContext);
    static RtResult<const RtGenericInst*> inflate_generic_inst(const RtGenericInst* genericInst, const RtGenericContext* genericContext);
    // Inflate a class with generic context
    static RtResult<RtClass*> inflate_class(const RtClass* klass, const RtGenericContext* genericContext);
    static RtResult<const RtMethodSig*> inflate_method_sig(const RtMethodSig* methodSig, const RtGenericContext* genericContext);
};
} // namespace metadata
} // namespace leanclr
