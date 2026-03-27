#pragma once

#include "rt_managed_types.h"
#include "utils/binary_reader.h"

namespace leanclr
{
namespace vm
{

struct CustomAttributeProvider
{
    metadata::RtModuleDef* mod;
    uint32_t token;

    CustomAttributeProvider(metadata::RtModuleDef* mod, uint32_t token) : mod(mod), token(token)
    {
    }

    CustomAttributeProvider() : mod(nullptr), token(0)
    {
    }
};

// Forward declarations
class CustomAttribute
{
  public:
    static RtResult<RtReflectionType*> parse_assembly_qualified_type(metadata::RtModuleDef* default_mod, const char* assembly_qualified_type_name,
                                                                     size_t name_len, bool ignore_case);

    static RtResult<RtObject*> read_custom_attribute(metadata::RtModuleDef* mod, const metadata::RtCustomAttributeRawData* data);

    static RtResultVoid resolve_customattribute_data_arguments(utils::BinaryReader* reader, metadata::RtModuleDef* mod,
                                                               const metadata::RtMethodInfo* ctor_method, RtArray** typed_arg_arr_ptr,
                                                               RtArray** named_arg_arr_ptr);

    static RtResult<bool> has_customattribute_on_target(metadata::RtModuleDef* mod, uint32_t target_token, metadata::RtClass* attr_klass);
    static RtResult<bool> has_customattribute_on_field(const metadata::RtFieldInfo* field, metadata::RtClass* attr_klass);
    static RtResult<bool> has_customattribute_on_method(const metadata::RtMethodInfo* method, metadata::RtClass* customattribute_klass);
    static RtResult<bool> has_customattribute_on_class(metadata::RtClass* klass, metadata::RtClass* customattribute_klass);
    static RtResult<bool> has_customattribute_on_property(const metadata::RtPropertyInfo* property, metadata::RtClass* customattribute_klass);
    static RtResult<bool> has_customattribute_on_event(const metadata::RtEventInfo* event, metadata::RtClass* customattribute_klass);
    static RtResult<bool> has_customattribute_on_parameter(RtReflectionParameter* parameter, metadata::RtClass* customattribute_klass);
    static RtResult<bool> has_customattribute_on_assembly(metadata::RtModuleDef* mod, metadata::RtClass* customattribute_klass);
    static RtResult<bool> has_attribute(RtObject* obj, metadata::RtClass* attr_klass);

    static RtResult<RtArray*> get_customattribute_on_target_token(metadata::RtModuleDef* mod, uint32_t target_token, metadata::RtClass* attr_klass);
    static RtResult<RtArray*> get_customattributes_on_target_object(RtObject* obj, metadata::RtClass* attr_klass);
    static RtResult<RtArray*> get_customattributes_data_on_target(RtObject* obj);
    static RtResult<RtArray*> get_customattributes_data_on_target_token(metadata::RtModuleDef* mod, uint32_t target_token);
};

} // namespace vm
} // namespace leanclr
