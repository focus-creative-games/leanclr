#include "parameter.h"
#include "metadata/metadata_const.h"

namespace leanclr
{
namespace vm
{

bool Parameter::has_parameter_attr_in(uint32_t param_attrs)
{
    return (param_attrs & (uint32_t)metadata::RtParamAttribute::In) != 0;
}

bool Parameter::has_parameter_attr_out(uint32_t param_attrs)
{
    return (param_attrs & (uint32_t)metadata::RtParamAttribute::Out) != 0;
}

bool Parameter::has_parameter_attr_optional(uint32_t param_attrs)
{
    return (param_attrs & (uint32_t)metadata::RtParamAttribute::Optional) != 0;
}

RtResult<RtObject*> Parameter::get_parameter_default_value_object(metadata::RtModuleDef* mod, metadata::EncodedTokenId param_token,
                                                                  const metadata::RtTypeSig* type)
{
    return metadata::MetadataConst::decode_const_object(mod, param_token, type);
}
} // namespace vm
} // namespace leanclr