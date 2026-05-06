#pragma once
#include "rt_managed_types.h"

namespace leanclr
{
namespace vm
{
class Parameter
{
  public:
    static bool has_parameter_attr_in(uint32_t param_attrs);
    static bool has_parameter_attr_out(uint32_t param_attrs);
    static bool has_parameter_attr_optional(uint32_t param_attrs);
    static RtResult<RtObject*> get_parameter_default_value_object(metadata::RtModuleDef* mod, metadata::EncodedTokenId param_token,
                                                                  const metadata::RtTypeSig* type);
};
} // namespace vm
} // namespace leanclr