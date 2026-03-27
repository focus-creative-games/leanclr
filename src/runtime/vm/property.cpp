#include "property.h"
#include "method.h"
#include "core/rt_result.h"
#include "metadata/metadata_const.h"
#include "metadata/module_def.h"

namespace leanclr
{
namespace vm
{

bool Property::is_static(const metadata::RtPropertyInfo* property)
{
    auto getter = property->get_method;
    if (getter != nullptr && Method::is_static(getter))
    {
        return true;
    }
    auto setter = property->set_method;
    if (setter != nullptr && Method::is_static(setter))
    {
        return true;
    }
    return false;
}

bool Property::is_public(const metadata::RtPropertyInfo* property)
{
    auto getter = property->get_method;
    if (getter != nullptr && Method::is_public(getter))
    {
        return true;
    }
    auto setter = property->set_method;
    if (setter != nullptr && Method::is_public(setter))
    {
        return true;
    }
    return false;
}

bool Property::is_private(const metadata::RtPropertyInfo* property)
{
    auto getter = property->get_method;
    if (getter != nullptr && !Method::is_private(getter))
    {
        return false;
    }
    auto setter = property->set_method;
    if (setter != nullptr && !Method::is_private(setter))
    {
        return false;
    }
    return true;
}

RtResult<RtObject*> Property::get_const_object(const metadata::RtPropertyInfo* property)
{
    return metadata::MetadataConst::decode_const_object(property->parent->image, property->token, property->property_sig.type_sig);
}

RtResultVoid Property::get_property_modifiers(const metadata::RtPropertyInfo* property, bool optional, utils::Vector<metadata::RtClass*>& modifiers)
{
    metadata::RtModuleDef* mod = property->parent->image;
    auto optPropertyRow = mod->get_cli_image().read_property(metadata::RtToken::decode_rid(property->token));
    assert(optPropertyRow.has_value());

    auto retBlobReader = mod->get_decoded_blob_reader(optPropertyRow->type_);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL2(utils::BinaryReader, blobReader, retBlobReader);

    metadata::RtGenericContainerContext gcc{};
    return mod->read_property_type_modifier(blobReader, optional, gcc, nullptr, modifiers);
}

} // namespace vm
} // namespace leanclr
