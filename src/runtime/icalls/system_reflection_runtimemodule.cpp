#include "system_reflection_runtimemodule.h"

#include <cstring>

#include "icall_base.h"
#include "vm/reflection.h"
#include "vm/assembly.h"
#include "vm/class.h"
#include "vm/assembly.h"
#include "vm/rt_array.h"
#include "metadata/module_def.h"
#include "metadata/metadata_cache.h"
#include "utils/binary_reader.h"

namespace leanclr
{
namespace icalls
{

RtResult<int32_t> SystemReflectionRuntimeModule::get_metadata_token(vm::RtReflectionModule* module) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtModuleDef*, mod, vm::Reflection::get_module_from_reflection_object(module));
    RET_OK(static_cast<int32_t>(mod->get_module_token()));
}

RtResult<intptr_t> get_metadata_import(vm::RtReflectionModule* module) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtModuleDef*, mod, vm::Reflection::get_module_from_reflection_object(module));
    RET_OK(reinterpret_cast<intptr_t>(mod));
}

struct MetadataConstArray
{
    int32_t length;
    intptr_t data;
};

static RtResult<metadata::RtModuleDef*> get_module_handle_param(const interp::RtStackObject* params, int32_t index) noexcept
{
    auto module_arg = EvalStackOp::get_param<const void*>(params, index);
    return vm::Reflection::get_module_from_handle_arg(module_arg);
}

static RtResult<bool> metadata_import_try_get_user_string_data(metadata::RtModuleDef* module, int32_t md_token,
                                                               const uint16_t** string_metadata_encoding,
                                                               int32_t* length) noexcept
{
    if (module == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    metadata::RtToken token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(md_token));
    if (token.table_type != metadata::TableType::String || token.rid == 0)
    {
        RET_OK(false);
    }

    const auto& heap = module->get_cli_image().get_us_heap();
    if (token.rid >= heap.size)
    {
        RET_OK(false);
    }

    const uint8_t* data = heap.data + token.rid;
    uint32_t str_size = 0;
    size_t size_length = 0;
    if (!utils::BinaryReader::try_decode_compressed_uint32(data, heap.size - token.rid, str_size, size_length) ||
        token.rid + size_length + str_size > heap.size)
    {
        RET_OK(false);
    }

    if (str_size != 0 && (str_size % 2) != 1)
    {
        RET_OK(false);
    }

    if (string_metadata_encoding != nullptr)
    {
        *string_metadata_encoding = reinterpret_cast<const uint16_t*>(data + size_length);
    }
    if (length != nullptr)
    {
        *length = static_cast<int32_t>(str_size == 0 ? 0 : (str_size - 1) / 2);
    }
    RET_OK(true);
}

static RtResult<const uint8_t*> metadata_import_get_mvid_bytes(metadata::RtModuleDef* module) noexcept
{
    if (module == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    auto row = module->get_cli_image().read_module(1);
    if (!row)
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    if (row->mvid == 0)
    {
        RET_OK(nullptr);
    }

    const auto& heap = module->get_cli_image().get_guid_heap();
    uint64_t offset = static_cast<uint64_t>(row->mvid - 1) * 16u;
    if (offset + 16u > heap.size)
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    RET_OK(heap.data + offset);
}

RtResult<int32_t> metadata_import_get_property_props(metadata::RtModuleDef* module, int32_t md_token, void** name,
                                                     int32_t* property_attributes, MetadataConstArray* signature) noexcept
{
    if (module == nullptr || name == nullptr || property_attributes == nullptr || signature == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    metadata::RtToken token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(md_token));
    if (token.table_type != metadata::TableType::Property)
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    auto row = module->get_cli_image().read_property(token.rid);
    if (!row)
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const char*, property_name, module->get_string(row->name));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(utils::BinaryReader, blob_reader, module->get_decoded_blob_reader(row->type_));

    *name = const_cast<char*>(property_name);
    *property_attributes = static_cast<int32_t>(row->flags);
    signature->length = static_cast<int32_t>(blob_reader.length());
    signature->data = reinterpret_cast<intptr_t>(blob_reader.data());
    RET_OK(0);
}

RtResult<int32_t> metadata_import_get_event_props(metadata::RtModuleDef* module, int32_t md_token, void** name,
                                                  int32_t* event_attributes) noexcept
{
    if (module == nullptr || name == nullptr || event_attributes == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    metadata::RtToken token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(md_token));
    if (token.table_type != metadata::TableType::Event)
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    auto row = module->get_cli_image().read_event(token.rid);
    if (!row)
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const char*, event_name, module->get_string(row->name));

    *name = const_cast<char*>(event_name);
    *event_attributes = static_cast<int32_t>(row->event_flags);
    RET_OK(0);
}

RtResult<int32_t> metadata_import_get_field_def_props(metadata::RtModuleDef* module, int32_t md_token,
                                                      int32_t* field_attributes) noexcept
{
    if (module == nullptr || field_attributes == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    metadata::RtToken token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(md_token));
    if (token.table_type != metadata::TableType::Field)
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    auto row = module->get_cli_image().read_field(token.rid);
    if (!row)
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    *field_attributes = static_cast<int32_t>(row->flags);
    RET_OK(0);
}

RtResult<int32_t> metadata_import_get_param_def_props(metadata::RtModuleDef* module, int32_t md_token, int32_t* sequence,
                                                      int32_t* attributes) noexcept
{
    if (module == nullptr || sequence == nullptr || attributes == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    metadata::RtToken token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(md_token));
    if (token.table_type != metadata::TableType::Param)
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    auto row = module->get_cli_image().read_param(token.rid);
    if (!row)
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    *sequence = static_cast<int32_t>(row->sequence);
    *attributes = static_cast<int32_t>(row->flags);
    RET_OK(0);
}

RtResult<int32_t> metadata_import_get_field_marshal(metadata::RtModuleDef* module, int32_t md_token,
                                                    MetadataConstArray* field_marshal) noexcept
{
    if (module == nullptr || field_marshal == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    field_marshal->length = 0;
    field_marshal->data = 0;

    metadata::RtToken token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(md_token));
    if (token.table_type != metadata::TableType::Field && token.table_type != metadata::TableType::Param)
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    uint32_t parent = metadata::RtMetadata::encode_has_field_marshal_coded_index(token.table_type, token.rid);
    const metadata::CliImage& image = module->get_cli_image();
    uint32_t field_marshal_count = module->get_table_row_num(metadata::TableType::FieldMarshal);
    for (uint32_t rid = 1; rid <= field_marshal_count; ++rid)
    {
        auto row = image.read_field_marshal(rid);
        if (!row)
        {
            RET_ERR(RtErr::BadImageFormat);
        }

        if (row->parent != parent)
        {
            continue;
        }

        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(utils::BinaryReader, blob_reader,
                                                 module->get_decoded_blob_reader(row->native_type));
        field_marshal->length = static_cast<int32_t>(blob_reader.length());
        field_marshal->data = reinterpret_cast<intptr_t>(blob_reader.data());
        break;
    }

    RET_OK(0);
}

RtResult<int32_t> metadata_import_get_field_offset(metadata::RtModuleDef* module, int32_t type_token_value, int32_t field_token_value,
                                                   int32_t* offset, bool* found) noexcept
{
    if (module == nullptr || offset == nullptr || found == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    *offset = 0;
    *found = false;

    metadata::RtToken type_token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(type_token_value));
    metadata::RtToken field_token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(field_token_value));
    if (type_token.table_type != metadata::TableType::TypeDef || field_token.table_type != metadata::TableType::Field)
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    auto row = module->get_cli_image().read_field(field_token.rid);
    if (!row)
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    auto field_offset = module->get_field_offset(static_cast<metadata::EncodedTokenId>(field_token_value));
    if (field_offset.has_value())
    {
        *offset = static_cast<int32_t>(field_offset.value());
        *found = true;
    }

    RET_OK(0);
}

RtResult<int32_t> metadata_import_get_class_layout(metadata::RtModuleDef* module, int32_t type_token_value,
                                                   int32_t* pack_size, int32_t* class_size) noexcept
{
    if (module == nullptr || pack_size == nullptr || class_size == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    *pack_size = 0;
    *class_size = 0;

    metadata::RtToken type_token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(type_token_value));
    if (type_token.table_type != metadata::TableType::TypeDef)
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    auto row = module->get_cli_image().read_type_def(type_token.rid);
    if (!row)
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    auto layout_data = module->get_class_layout_data(static_cast<metadata::EncodedTokenId>(type_token_value));
    if (layout_data.has_value())
    {
        *pack_size = static_cast<int32_t>(layout_data->packing);
        *class_size = static_cast<int32_t>(layout_data->size);
    }

    RET_OK(0);
}

RtResult<int32_t> metadata_import_get_default_value(metadata::RtModuleDef* module, int32_t md_token, int64_t* value,
                                                    const uint16_t** string_metadata_encoding, int32_t* length,
                                                    int32_t* cor_element_type) noexcept
{
    if (module == nullptr || value == nullptr || string_metadata_encoding == nullptr || length == nullptr ||
        cor_element_type == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    *value = 0;
    *string_metadata_encoding = nullptr;
    *length = 0;
    *cor_element_type = static_cast<int32_t>(metadata::RtElementType::Void);

    metadata::RtToken token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(md_token));
    if (token.table_type != metadata::TableType::Field && token.table_type != metadata::TableType::Param &&
        token.table_type != metadata::TableType::Property)
    {
        RET_ERR(RtErr::BadImageFormat);
    }
    if (token.rid == 0 || token.rid > module->get_table_row_num(token.table_type))
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    const metadata::CliImage& image = module->get_cli_image();
    uint32_t parent = metadata::RtMetadata::encode_has_constant_coded_index(token.table_type, token.rid);
    auto opt_const_rid = image.find_row_of_owner(metadata::TableType::Constant, 2, parent);
    if (!opt_const_rid)
    {
        RET_OK(0);
    }

    auto opt_row = image.read_constant(opt_const_rid.value());
    if (!opt_row)
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(utils::BinaryReader, blob_reader,
                                             module->get_decoded_blob_reader(opt_row->value));
    metadata::RtElementType element_type = static_cast<metadata::RtElementType>(opt_row->type_);
    *cor_element_type = static_cast<int32_t>(element_type);

    if (element_type == metadata::RtElementType::String)
    {
        if ((blob_reader.length() % 2) != 0)
        {
            RET_ERR(RtErr::BadImageFormat);
        }

        *string_metadata_encoding = reinterpret_cast<const uint16_t*>(blob_reader.data());
        *length = static_cast<int32_t>(blob_reader.length() / 2);
        RET_OK(0);
    }

    size_t copy_size = blob_reader.length();
    if (copy_size > sizeof(*value))
    {
        RET_ERR(RtErr::BadImageFormat);
    }
    if (copy_size != 0)
    {
        std::memcpy(value, blob_reader.data(), copy_size);
    }

    RET_OK(0);
}

RtResult<int32_t> metadata_import_get_user_string(metadata::RtModuleDef* module, int32_t md_token,
                                                  const uint16_t** string_metadata_encoding, int32_t* length) noexcept
{
    if (string_metadata_encoding == nullptr || length == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    *string_metadata_encoding = nullptr;
    *length = 0;

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, found,
                                            metadata_import_try_get_user_string_data(module, md_token, string_metadata_encoding, length));
    if (!found)
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    RET_OK(0);
}

RtResult<int32_t> metadata_import_get_scope_props(metadata::RtModuleDef* module, uint8_t* mvid) noexcept
{
    if (mvid == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    std::memset(mvid, 0, 16);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const uint8_t*, mvid_bytes, metadata_import_get_mvid_bytes(module));
    if (mvid_bytes != nullptr)
    {
        std::memcpy(mvid, mvid_bytes, 16);
    }

    RET_OK(0);
}

RtResult<int32_t> metadata_import_get_signature_from_token(metadata::RtModuleDef* module, int32_t md_token,
                                                           MetadataConstArray* signature) noexcept
{
    if (module == nullptr || signature == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    metadata::RtToken token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(md_token));
    const metadata::CliImage& image = module->get_cli_image();
    uint32_t blob_index = 0;

    switch (token.table_type)
    {
    case metadata::TableType::Field:
    {
        auto row = image.read_field(token.rid);
        if (!row)
            RET_ERR(RtErr::BadImageFormat);
        blob_index = row->signature;
        break;
    }
    case metadata::TableType::Method:
    {
        auto row = image.read_method(token.rid);
        if (!row)
            RET_ERR(RtErr::BadImageFormat);
        blob_index = row->signature;
        break;
    }
    case metadata::TableType::MemberRef:
    {
        auto row = image.read_member_ref(token.rid);
        if (!row)
            RET_ERR(RtErr::BadImageFormat);
        blob_index = row->signature;
        break;
    }
    case metadata::TableType::StandaloneSig:
    {
        auto row = image.read_stand_alone_sig(token.rid);
        if (!row)
            RET_ERR(RtErr::BadImageFormat);
        blob_index = row->signature;
        break;
    }
    case metadata::TableType::TypeSpec:
    {
        auto row = image.read_type_spec(token.rid);
        if (!row)
            RET_ERR(RtErr::BadImageFormat);
        blob_index = row->signature;
        break;
    }
    default:
        RET_ERR(RtErr::BadImageFormat);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(utils::BinaryReader, blob_reader, module->get_decoded_blob_reader(blob_index));
    signature->length = static_cast<int32_t>(blob_reader.length());
    signature->data = reinterpret_cast<intptr_t>(blob_reader.data());
    RET_OK(0);
}

RtResult<int32_t> metadata_import_get_member_ref_props(metadata::RtModuleDef* module, int32_t md_token,
                                                       MetadataConstArray* signature) noexcept
{
    metadata::RtToken token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(md_token));
    if (token.table_type != metadata::TableType::MemberRef)
    {
        RET_ERR(RtErr::BadImageFormat);
    }
    return metadata_import_get_signature_from_token(module, md_token, signature);
}

RtResult<int32_t> metadata_import_get_sig_of_method_def(metadata::RtModuleDef* module, int32_t md_token,
                                                        MetadataConstArray* signature) noexcept
{
    metadata::RtToken token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(md_token));
    if (token.table_type != metadata::TableType::Method)
    {
        RET_ERR(RtErr::BadImageFormat);
    }
    return metadata_import_get_signature_from_token(module, md_token, signature);
}

RtResult<int32_t> metadata_import_get_sig_of_field_def(metadata::RtModuleDef* module, int32_t md_token,
                                                       MetadataConstArray* signature) noexcept
{
    metadata::RtToken token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(md_token));
    if (token.table_type != metadata::TableType::Field)
    {
        RET_ERR(RtErr::BadImageFormat);
    }
    return metadata_import_get_signature_from_token(module, md_token, signature);
}

RtResult<int32_t> metadata_import_get_custom_attribute_props(metadata::RtModuleDef* module, int32_t md_token,
                                                             int32_t* constructor_token,
                                                             MetadataConstArray* signature) noexcept
{
    if (module == nullptr || constructor_token == nullptr || signature == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    metadata::RtToken token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(md_token));
    if (token.table_type != metadata::TableType::CustomAttribute || token.rid == 0)
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtCustomAttributeRawData, raw_data,
                                            module->get_custom_attribute_raw_data(token.rid));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(utils::BinaryReader, blob_reader,
                                             module->get_decoded_blob_reader(raw_data.dataBlobIndex));

    *constructor_token = static_cast<int32_t>(raw_data.ctor_token);
    signature->length = static_cast<int32_t>(blob_reader.length());
    signature->data = reinterpret_cast<intptr_t>(blob_reader.data());
    RET_OK(0);
}

static int32_t encode_metadata_parent_token(metadata::RtToken token) noexcept
{
    if (token.table_type == metadata::TableType::Invalid || token.rid == 0)
    {
        return 0;
    }
    return static_cast<int32_t>(metadata::RtToken::encode(token.table_type, token.rid));
}

static RtResult<int32_t> find_typedef_owner_by_field(metadata::RtModuleDef* module, uint32_t field_rid) noexcept
{
    uint32_t type_count = module->get_table_row_num(metadata::TableType::TypeDef);
    uint32_t field_count = module->get_table_row_num(metadata::TableType::Field);
    const metadata::CliImage& image = module->get_cli_image();

    for (uint32_t type_rid = 1; type_rid <= type_count; ++type_rid)
    {
        auto row = image.read_type_def(type_rid);
        if (!row)
        {
            RET_ERR(RtErr::BadImageFormat);
        }

        uint32_t end_rid = field_count + 1;
        if (type_rid < type_count)
        {
            auto next_row = image.read_type_def(type_rid + 1);
            if (!next_row)
            {
                RET_ERR(RtErr::BadImageFormat);
            }
            end_rid = next_row->field_list;
        }

        if (field_rid >= row->field_list && field_rid < end_rid)
        {
            RET_OK(static_cast<int32_t>(metadata::RtToken::encode(metadata::TableType::TypeDef, type_rid)));
        }
    }

    RET_OK(0);
}

static RtResult<int32_t> find_typedef_owner_by_method(metadata::RtModuleDef* module, uint32_t method_rid) noexcept
{
    uint32_t type_count = module->get_table_row_num(metadata::TableType::TypeDef);
    uint32_t method_count = module->get_table_row_num(metadata::TableType::Method);
    const metadata::CliImage& image = module->get_cli_image();

    for (uint32_t type_rid = 1; type_rid <= type_count; ++type_rid)
    {
        auto row = image.read_type_def(type_rid);
        if (!row)
        {
            RET_ERR(RtErr::BadImageFormat);
        }

        uint32_t end_rid = method_count + 1;
        if (type_rid < type_count)
        {
            auto next_row = image.read_type_def(type_rid + 1);
            if (!next_row)
            {
                RET_ERR(RtErr::BadImageFormat);
            }
            end_rid = next_row->method_list;
        }

        if (method_rid >= row->method_list && method_rid < end_rid)
        {
            RET_OK(static_cast<int32_t>(metadata::RtToken::encode(metadata::TableType::TypeDef, type_rid)));
        }
    }

    RET_OK(0);
}

static RtResult<int32_t> find_method_owner_by_param(metadata::RtModuleDef* module, uint32_t param_rid) noexcept
{
    uint32_t method_count = module->get_table_row_num(metadata::TableType::Method);
    uint32_t param_count = module->get_table_row_num(metadata::TableType::Param);
    const metadata::CliImage& image = module->get_cli_image();

    for (uint32_t method_rid = 1; method_rid <= method_count; ++method_rid)
    {
        auto row = image.read_method(method_rid);
        if (!row)
        {
            RET_ERR(RtErr::BadImageFormat);
        }

        uint32_t end_rid = param_count + 1;
        if (method_rid < method_count)
        {
            auto next_row = image.read_method(method_rid + 1);
            if (!next_row)
            {
                RET_ERR(RtErr::BadImageFormat);
            }
            end_rid = next_row->param_list;
        }

        if (param_rid >= row->param_list && param_rid < end_rid)
        {
            RET_OK(static_cast<int32_t>(metadata::RtToken::encode(metadata::TableType::Method, method_rid)));
        }
    }

    RET_OK(0);
}

static RtResult<int32_t> find_typedef_owner_by_property(metadata::RtModuleDef* module, uint32_t property_rid) noexcept
{
    uint32_t map_count = module->get_table_row_num(metadata::TableType::PropertyMap);
    uint32_t property_count = module->get_table_row_num(metadata::TableType::Property);
    const metadata::CliImage& image = module->get_cli_image();

    for (uint32_t map_rid = 1; map_rid <= map_count; ++map_rid)
    {
        auto row = image.read_property_map(map_rid);
        if (!row)
        {
            RET_ERR(RtErr::BadImageFormat);
        }

        uint32_t end_rid = property_count + 1;
        if (map_rid < map_count)
        {
            auto next_row = image.read_property_map(map_rid + 1);
            if (!next_row)
            {
                RET_ERR(RtErr::BadImageFormat);
            }
            end_rid = next_row->property_list;
        }

        if (property_rid >= row->property_list && property_rid < end_rid)
        {
            RET_OK(static_cast<int32_t>(metadata::RtToken::encode(metadata::TableType::TypeDef, row->parent)));
        }
    }

    RET_OK(0);
}

static RtResult<int32_t> find_typedef_owner_by_event(metadata::RtModuleDef* module, uint32_t event_rid) noexcept
{
    uint32_t map_count = module->get_table_row_num(metadata::TableType::EventMap);
    uint32_t event_count = module->get_table_row_num(metadata::TableType::Event);
    const metadata::CliImage& image = module->get_cli_image();

    for (uint32_t map_rid = 1; map_rid <= map_count; ++map_rid)
    {
        auto row = image.read_event_map(map_rid);
        if (!row)
        {
            RET_ERR(RtErr::BadImageFormat);
        }

        uint32_t end_rid = event_count + 1;
        if (map_rid < map_count)
        {
            auto next_row = image.read_event_map(map_rid + 1);
            if (!next_row)
            {
                RET_ERR(RtErr::BadImageFormat);
            }
            end_rid = next_row->event_list;
        }

        if (event_rid >= row->event_list && event_rid < end_rid)
        {
            RET_OK(static_cast<int32_t>(metadata::RtToken::encode(metadata::TableType::TypeDef, row->parent)));
        }
    }

    RET_OK(0);
}

static RtResult<int32_t> find_enclosing_typedef(metadata::RtModuleDef* module, uint32_t type_rid) noexcept
{
    uint32_t nested_count = module->get_table_row_num(metadata::TableType::NestedClass);
    const metadata::CliImage& image = module->get_cli_image();

    for (uint32_t nested_rid = 1; nested_rid <= nested_count; ++nested_rid)
    {
        auto row = image.read_nested_class(nested_rid);
        if (!row)
        {
            RET_ERR(RtErr::BadImageFormat);
        }

        if (row->nested_class == type_rid)
        {
            RET_OK(static_cast<int32_t>(metadata::RtToken::encode(metadata::TableType::TypeDef, row->enclosing_class)));
        }
    }

    RET_OK(0);
}

RtResult<int32_t> metadata_import_get_parent_token(metadata::RtModuleDef* module, int32_t md_token, int32_t* parent_token) noexcept
{
    if (module == nullptr || parent_token == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    metadata::RtToken token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(md_token));
    const metadata::CliImage& image = module->get_cli_image();
    int32_t parent = 0;

    switch (token.table_type)
    {
    case metadata::TableType::TypeDef:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, enclosing, find_enclosing_typedef(module, token.rid));
        parent = enclosing;
        break;
    }
    case metadata::TableType::Field:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, field_owner, find_typedef_owner_by_field(module, token.rid));
        parent = field_owner;
        break;
    }
    case metadata::TableType::Method:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, method_owner, find_typedef_owner_by_method(module, token.rid));
        parent = method_owner;
        break;
    }
    case metadata::TableType::Param:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, param_owner, find_method_owner_by_param(module, token.rid));
        parent = param_owner;
        break;
    }
    case metadata::TableType::MemberRef:
    {
        auto row = image.read_member_ref(token.rid);
        if (!row)
        {
            RET_ERR(RtErr::BadImageFormat);
        }
        parent = encode_metadata_parent_token(metadata::RtMetadata::decode_member_ref_parent_coded_index(row->class_idx));
        break;
    }
    case metadata::TableType::CustomAttribute:
    {
        auto row = image.read_custom_attribute(token.rid);
        if (!row)
        {
            RET_ERR(RtErr::BadImageFormat);
        }
        parent = encode_metadata_parent_token(metadata::RtMetadata::decode_has_customattribute_coded_index(row->parent));
        break;
    }
    case metadata::TableType::Constant:
    {
        auto row = image.read_constant(token.rid);
        if (!row)
        {
            RET_ERR(RtErr::BadImageFormat);
        }
        parent = encode_metadata_parent_token(metadata::RtMetadata::decode_has_constant_coded_index(row->parent));
        break;
    }
    case metadata::TableType::FieldMarshal:
    {
        auto row = image.read_field_marshal(token.rid);
        if (!row)
        {
            RET_ERR(RtErr::BadImageFormat);
        }
        metadata::TableType parent_table = (row->parent & 0x1u) == 0 ? metadata::TableType::Field : metadata::TableType::Param;
        parent = static_cast<int32_t>(metadata::RtToken::encode(parent_table, row->parent >> 1));
        break;
    }
    case metadata::TableType::DeclSecurity:
    {
        auto row = image.read_decl_security(token.rid);
        if (!row)
        {
            RET_ERR(RtErr::BadImageFormat);
        }
        uint32_t tag = row->parent & 0x3u;
        metadata::TableType parent_table = metadata::TableType::Invalid;
        if (tag == 0)
        {
            parent_table = metadata::TableType::TypeDef;
        }
        else if (tag == 1)
        {
            parent_table = metadata::TableType::Method;
        }
        else if (tag == 2)
        {
            parent_table = metadata::TableType::Assembly;
        }
        parent = encode_metadata_parent_token(metadata::RtToken{parent_table, row->parent >> 2});
        break;
    }
    case metadata::TableType::ClassLayout:
    {
        auto row = image.read_class_layout(token.rid);
        if (!row)
        {
            RET_ERR(RtErr::BadImageFormat);
        }
        parent = static_cast<int32_t>(metadata::RtToken::encode(metadata::TableType::TypeDef, row->parent));
        break;
    }
    case metadata::TableType::FieldLayout:
    {
        auto row = image.read_field_layout(token.rid);
        if (!row)
        {
            RET_ERR(RtErr::BadImageFormat);
        }
        parent = static_cast<int32_t>(metadata::RtToken::encode(metadata::TableType::Field, row->field));
        break;
    }
    case metadata::TableType::InterfaceImpl:
    {
        auto row = image.read_interface_impl(token.rid);
        if (!row)
        {
            RET_ERR(RtErr::BadImageFormat);
        }
        parent = static_cast<int32_t>(metadata::RtToken::encode(metadata::TableType::TypeDef, row->class_idx));
        break;
    }
    case metadata::TableType::EventMap:
    {
        auto row = image.read_event_map(token.rid);
        if (!row)
        {
            RET_ERR(RtErr::BadImageFormat);
        }
        parent = static_cast<int32_t>(metadata::RtToken::encode(metadata::TableType::TypeDef, row->parent));
        break;
    }
    case metadata::TableType::Event:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, event_owner, find_typedef_owner_by_event(module, token.rid));
        parent = event_owner;
        break;
    }
    case metadata::TableType::PropertyMap:
    {
        auto row = image.read_property_map(token.rid);
        if (!row)
        {
            RET_ERR(RtErr::BadImageFormat);
        }
        parent = static_cast<int32_t>(metadata::RtToken::encode(metadata::TableType::TypeDef, row->parent));
        break;
    }
    case metadata::TableType::Property:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, property_owner, find_typedef_owner_by_property(module, token.rid));
        parent = property_owner;
        break;
    }
    case metadata::TableType::MethodImpl:
    {
        auto row = image.read_method_impl(token.rid);
        if (!row)
        {
            RET_ERR(RtErr::BadImageFormat);
        }
        parent = static_cast<int32_t>(metadata::RtToken::encode(metadata::TableType::TypeDef, row->class_idx));
        break;
    }
    case metadata::TableType::ImplMap:
    {
        auto row = image.read_impl_map(token.rid);
        if (!row)
        {
            RET_ERR(RtErr::BadImageFormat);
        }
        metadata::TableType parent_table = (row->member_forwarded & 0x1u) == 0 ? metadata::TableType::Field : metadata::TableType::Method;
        parent = static_cast<int32_t>(metadata::RtToken::encode(parent_table, row->member_forwarded >> 1));
        break;
    }
    case metadata::TableType::NestedClass:
    {
        auto row = image.read_nested_class(token.rid);
        if (!row)
        {
            RET_ERR(RtErr::BadImageFormat);
        }
        parent = static_cast<int32_t>(metadata::RtToken::encode(metadata::TableType::TypeDef, row->enclosing_class));
        break;
    }
    case metadata::TableType::GenericParam:
    {
        auto row = image.read_generic_param(token.rid);
        if (!row)
        {
            RET_ERR(RtErr::BadImageFormat);
        }
        parent = encode_metadata_parent_token(metadata::RtMetadata::decode_type_or_method_def_coded_index(row->owner));
        break;
    }
    case metadata::TableType::GenericParamConstraint:
    {
        auto row = image.read_generic_param_constraint(token.rid);
        if (!row)
        {
            RET_ERR(RtErr::BadImageFormat);
        }
        parent = static_cast<int32_t>(metadata::RtToken::encode(metadata::TableType::GenericParam, row->owner));
        break;
    }
    case metadata::TableType::MethodSpec:
    {
        auto row = image.read_method_spec(token.rid);
        if (!row)
        {
            RET_ERR(RtErr::BadImageFormat);
        }
        parent = encode_metadata_parent_token(metadata::RtMetadata::decode_method_def_or_ref_coded_index(row->method));
        break;
    }
    default:
        parent = 0;
        break;
    }

    *parent_token = parent;
    RET_OK(0);
}

RtResult<int32_t> metadata_import_get_name(metadata::RtModuleDef* module, int32_t md_token, void** name) noexcept
{
    if (module == nullptr || name == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    metadata::RtToken token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(md_token));
    const char* value = nullptr;
    const metadata::CliImage& image = module->get_cli_image();

    switch (token.table_type)
    {
    case metadata::TableType::TypeDef:
    {
        auto row = image.read_type_def(token.rid);
        if (!row)
            RET_ERR(RtErr::BadImageFormat);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const char*, resolved, module->get_string(row->type_name));
        value = resolved;
        break;
    }
    case metadata::TableType::TypeRef:
    {
        auto row = image.read_type_ref(token.rid);
        if (!row)
            RET_ERR(RtErr::BadImageFormat);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const char*, resolved, module->get_string(row->type_name));
        value = resolved;
        break;
    }
    case metadata::TableType::Field:
    {
        auto row = image.read_field(token.rid);
        if (!row)
            RET_ERR(RtErr::BadImageFormat);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const char*, resolved, module->get_string(row->name));
        value = resolved;
        break;
    }
    case metadata::TableType::Method:
    {
        auto row = image.read_method(token.rid);
        if (!row)
            RET_ERR(RtErr::BadImageFormat);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const char*, resolved, module->get_string(row->name));
        value = resolved;
        break;
    }
    case metadata::TableType::Param:
    {
        auto row = image.read_param(token.rid);
        if (!row)
            RET_ERR(RtErr::BadImageFormat);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const char*, resolved, module->get_string(row->name));
        value = resolved;
        break;
    }
    case metadata::TableType::MemberRef:
    {
        auto row = image.read_member_ref(token.rid);
        if (!row)
            RET_ERR(RtErr::BadImageFormat);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const char*, resolved, module->get_string(row->name));
        value = resolved;
        break;
    }
    case metadata::TableType::Event:
    {
        auto row = image.read_event(token.rid);
        if (!row)
            RET_ERR(RtErr::BadImageFormat);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const char*, resolved, module->get_string(row->name));
        value = resolved;
        break;
    }
    case metadata::TableType::Property:
    {
        auto row = image.read_property(token.rid);
        if (!row)
        {
            RET_ERR(RtErr::BadImageFormat);
        }
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const char*, resolved, module->get_string(row->name));
        value = resolved;
        break;
    }
    case metadata::TableType::ModuleRef:
    {
        auto row = image.read_module_ref(token.rid);
        if (!row)
            RET_ERR(RtErr::BadImageFormat);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const char*, resolved, module->get_string(row->name));
        value = resolved;
        break;
    }
    case metadata::TableType::AssemblyRef:
    {
        auto row = image.read_assembly_ref(token.rid);
        if (!row)
            RET_ERR(RtErr::BadImageFormat);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const char*, resolved, module->get_string(row->name));
        value = resolved;
        break;
    }
    case metadata::TableType::ManifestResource:
    {
        auto row = image.read_manifest_resource(token.rid);
        if (!row)
            RET_ERR(RtErr::BadImageFormat);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const char*, resolved, module->get_string(row->name));
        value = resolved;
        break;
    }
    default:
        RET_ERR(RtErr::BadImageFormat);
    }

    *name = const_cast<char*>(value);
    RET_OK(0);
}

RtResult<int32_t> metadata_import_get_namespace(metadata::RtModuleDef* module, int32_t md_token, void** namespaze) noexcept
{
    if (module == nullptr || namespaze == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    metadata::RtToken token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(md_token));
    const char* value = nullptr;
    const metadata::CliImage& image = module->get_cli_image();

    switch (token.table_type)
    {
    case metadata::TableType::TypeDef:
    {
        auto row = image.read_type_def(token.rid);
        if (!row)
            RET_ERR(RtErr::BadImageFormat);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const char*, resolved, module->get_string(row->type_namespace));
        value = resolved;
        break;
    }
    case metadata::TableType::TypeRef:
    {
        auto row = image.read_type_ref(token.rid);
        if (!row)
            RET_ERR(RtErr::BadImageFormat);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const char*, resolved, module->get_string(row->type_namespace));
        value = resolved;
        break;
    }
    default:
        RET_ERR(RtErr::BadImageFormat);
    }

    *namespaze = const_cast<char*>(value);
    RET_OK(0);
}

/// @icall: System.Reflection.RuntimeModule::get_MetadataToken(System.Reflection.Module)
static RtResultVoid get_metadata_token_invoker_system_reflection_runtimemodule(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto module = EvalStackOp::get_param<vm::RtReflectionModule*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, token, SystemReflectionRuntimeModule::get_metadata_token(module));
    EvalStackOp::set_return(ret, token);
    RET_VOID_OK();
}

/// @icall: System.Reflection.MetadataImport::GetMetadataImport(System.Reflection.RuntimeModule)
static RtResultVoid get_metadata_import_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                interp::RtStackObject* ret) noexcept
{
    auto module = EvalStackOp::get_param<vm::RtReflectionModule*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(intptr_t, metadata_import, get_metadata_import(module));
    EvalStackOp::set_return(ret, metadata_import);
    RET_VOID_OK();
}

/// @icall: System.Reflection.MetadataImport::GetName
static RtResultVoid metadata_import_get_name_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                     interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtModuleDef*, module, get_module_handle_param(params, 0));
    auto md_token = EvalStackOp::get_param<int32_t>(params, 1);
    auto name = EvalStackOp::get_param<void**>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, hr, metadata_import_get_name(module, md_token, name));
    EvalStackOp::set_return(ret, hr);
    RET_VOID_OK();
}

/// @icall: System.Reflection.MetadataImport::GetNamespace
static RtResultVoid metadata_import_get_namespace_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                          const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtModuleDef*, module, get_module_handle_param(params, 0));
    auto md_token = EvalStackOp::get_param<int32_t>(params, 1);
    auto namespaze = EvalStackOp::get_param<void**>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, hr, metadata_import_get_namespace(module, md_token, namespaze));
    EvalStackOp::set_return(ret, hr);
    RET_VOID_OK();
}

/// @icall: System.Reflection.MetadataImport::GetPropertyProps
static RtResultVoid metadata_import_get_property_props_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtModuleDef*, module, get_module_handle_param(params, 0));
    auto md_token = EvalStackOp::get_param<int32_t>(params, 1);
    auto name = EvalStackOp::get_param<void**>(params, 2);
    auto property_attributes = EvalStackOp::get_param<int32_t*>(params, 3);
    auto signature = EvalStackOp::get_param<MetadataConstArray*>(params, 4);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, hr,
                                            metadata_import_get_property_props(module, md_token, name, property_attributes, signature));
    EvalStackOp::set_return(ret, hr);
    RET_VOID_OK();
}

/// @icall: System.Reflection.MetadataImport::GetEventProps(System.IntPtr,System.Int32,System.Void*&,System.Int32&)
static RtResultVoid metadata_import_get_event_props_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                            const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtModuleDef*, module, get_module_handle_param(params, 0));
    auto md_token = EvalStackOp::get_param<int32_t>(params, 1);
    auto name = EvalStackOp::get_param<void**>(params, 2);
    auto event_attributes = EvalStackOp::get_param<int32_t*>(params, 3);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, hr, metadata_import_get_event_props(module, md_token, name, event_attributes));
    EvalStackOp::set_return(ret, hr);
    RET_VOID_OK();
}

/// @icall: System.Reflection.MetadataImport::GetFieldDefProps(System.IntPtr,System.Int32,System.Int32&)
static RtResultVoid metadata_import_get_field_def_props_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtModuleDef*, module, get_module_handle_param(params, 0));
    auto md_token = EvalStackOp::get_param<int32_t>(params, 1);
    auto field_attributes = EvalStackOp::get_param<int32_t*>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, hr,
                                            metadata_import_get_field_def_props(module, md_token, field_attributes));
    EvalStackOp::set_return(ret, hr);
    RET_VOID_OK();
}

/// @icall: System.Reflection.MetadataImport::GetParamDefProps(System.IntPtr,System.Int32,System.Int32&,System.Int32&)
static RtResultVoid metadata_import_get_param_def_props_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtModuleDef*, module, get_module_handle_param(params, 0));
    auto md_token = EvalStackOp::get_param<int32_t>(params, 1);
    auto sequence = EvalStackOp::get_param<int32_t*>(params, 2);
    auto attributes = EvalStackOp::get_param<int32_t*>(params, 3);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, hr,
                                            metadata_import_get_param_def_props(module, md_token, sequence, attributes));
    EvalStackOp::set_return(ret, hr);
    RET_VOID_OK();
}

/// @icall: System.Reflection.MetadataImport::GetFieldMarshal(System.IntPtr,System.Int32,System.Reflection.ConstArray&)
static RtResultVoid metadata_import_get_field_marshal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                              const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtModuleDef*, module, get_module_handle_param(params, 0));
    auto md_token = EvalStackOp::get_param<int32_t>(params, 1);
    auto field_marshal = EvalStackOp::get_param<MetadataConstArray*>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, hr, metadata_import_get_field_marshal(module, md_token, field_marshal));
    EvalStackOp::set_return(ret, hr);
    RET_VOID_OK();
}

/// @icall: System.Reflection.MetadataImport::GetFieldOffset(System.IntPtr,System.Int32,System.Int32,System.Int32&,System.Boolean&)
static RtResultVoid metadata_import_get_field_offset_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                             const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtModuleDef*, module, get_module_handle_param(params, 0));
    auto type_token = EvalStackOp::get_param<int32_t>(params, 1);
    auto field_token = EvalStackOp::get_param<int32_t>(params, 2);
    auto offset = EvalStackOp::get_param<int32_t*>(params, 3);
    auto found = EvalStackOp::get_param<bool*>(params, 4);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, hr, metadata_import_get_field_offset(module, type_token, field_token, offset, found));
    EvalStackOp::set_return(ret, hr);
    RET_VOID_OK();
}

/// @icall: System.Reflection.MetadataImport::GetClassLayout(System.IntPtr,System.Int32,System.Int32&,System.Int32&)
static RtResultVoid metadata_import_get_class_layout_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                             const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtModuleDef*, module, get_module_handle_param(params, 0));
    auto type_token = EvalStackOp::get_param<int32_t>(params, 1);
    auto pack_size = EvalStackOp::get_param<int32_t*>(params, 2);
    auto class_size = EvalStackOp::get_param<int32_t*>(params, 3);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, hr, metadata_import_get_class_layout(module, type_token, pack_size, class_size));
    EvalStackOp::set_return(ret, hr);
    RET_VOID_OK();
}

/// @icall: System.Reflection.MetadataImport::GetSignatureFromToken(System.IntPtr,System.Int32,System.Reflection.ConstArray&)
static RtResultVoid metadata_import_get_signature_from_token_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                                     const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtModuleDef*, module, get_module_handle_param(params, 0));
    auto md_token = EvalStackOp::get_param<int32_t>(params, 1);
    auto signature = EvalStackOp::get_param<MetadataConstArray*>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, hr, metadata_import_get_signature_from_token(module, md_token, signature));
    EvalStackOp::set_return(ret, hr);
    RET_VOID_OK();
}

/// @icall: System.Reflection.MetadataImport::GetDefaultValue(System.IntPtr,System.Int32,System.Int64&,System.Char*&,System.Int32&,System.Int32&)
static RtResultVoid metadata_import_get_default_value_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                              const interp::RtStackObject* params,
                                                              interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtModuleDef*, module, get_module_handle_param(params, 0));
    auto md_token = EvalStackOp::get_param<int32_t>(params, 1);
    auto value = EvalStackOp::get_param<int64_t*>(params, 2);
    auto string_metadata_encoding = EvalStackOp::get_param<const uint16_t**>(params, 3);
    auto length = EvalStackOp::get_param<int32_t*>(params, 4);
    auto cor_element_type = EvalStackOp::get_param<int32_t*>(params, 5);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, hr,
                                            metadata_import_get_default_value(module, md_token, value, string_metadata_encoding,
                                                                              length, cor_element_type));
    EvalStackOp::set_return(ret, hr);
    RET_VOID_OK();
}

/// @icall: System.Reflection.MetadataImport::GetUserString(System.IntPtr,System.Int32,System.Char*&,System.Int32&)
static RtResultVoid metadata_import_get_user_string_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                            const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtModuleDef*, module, get_module_handle_param(params, 0));
    auto md_token = EvalStackOp::get_param<int32_t>(params, 1);
    auto string_metadata_encoding = EvalStackOp::get_param<const uint16_t**>(params, 2);
    auto length = EvalStackOp::get_param<int32_t*>(params, 3);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, hr,
                                            metadata_import_get_user_string(module, md_token, string_metadata_encoding, length));
    EvalStackOp::set_return(ret, hr);
    RET_VOID_OK();
}

/// @icall: System.Reflection.MetadataImport::GetScopeProps(System.IntPtr,System.Guid&)
static RtResultVoid metadata_import_get_scope_props_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                            const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtModuleDef*, module, get_module_handle_param(params, 0));
    auto mvid = EvalStackOp::get_param<uint8_t*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, hr, metadata_import_get_scope_props(module, mvid));
    EvalStackOp::set_return(ret, hr);
    RET_VOID_OK();
}

/// @icall: System.Reflection.MetadataImport::GetMemberRefProps(System.IntPtr,System.Int32,System.Reflection.ConstArray&)
static RtResultVoid metadata_import_get_member_ref_props_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                                 const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtModuleDef*, module, get_module_handle_param(params, 0));
    auto md_token = EvalStackOp::get_param<int32_t>(params, 1);
    auto signature = EvalStackOp::get_param<MetadataConstArray*>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, hr, metadata_import_get_member_ref_props(module, md_token, signature));
    EvalStackOp::set_return(ret, hr);
    RET_VOID_OK();
}

/// @icall: System.Reflection.MetadataImport::GetSigOfMethodDef(System.IntPtr,System.Int32,System.Reflection.ConstArray&)
static RtResultVoid metadata_import_get_sig_of_method_def_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                                  const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtModuleDef*, module, get_module_handle_param(params, 0));
    auto md_token = EvalStackOp::get_param<int32_t>(params, 1);
    auto signature = EvalStackOp::get_param<MetadataConstArray*>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, hr, metadata_import_get_sig_of_method_def(module, md_token, signature));
    EvalStackOp::set_return(ret, hr);
    RET_VOID_OK();
}

/// @icall: System.Reflection.MetadataImport::GetSigOfFieldDef(System.IntPtr,System.Int32,System.Reflection.ConstArray&)
static RtResultVoid metadata_import_get_sig_of_field_def_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                                 const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtModuleDef*, module, get_module_handle_param(params, 0));
    auto md_token = EvalStackOp::get_param<int32_t>(params, 1);
    auto signature = EvalStackOp::get_param<MetadataConstArray*>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, hr, metadata_import_get_sig_of_field_def(module, md_token, signature));
    EvalStackOp::set_return(ret, hr);
    RET_VOID_OK();
}

/// @icall: System.Reflection.MetadataImport::GetCustomAttributeProps(System.IntPtr,System.Int32,System.Int32&,System.Reflection.ConstArray&)
static RtResultVoid metadata_import_get_custom_attribute_props_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                                       const interp::RtStackObject* params,
                                                                       interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtModuleDef*, module, get_module_handle_param(params, 0));
    auto md_token = EvalStackOp::get_param<int32_t>(params, 1);
    auto constructor_token = EvalStackOp::get_param<int32_t*>(params, 2);
    auto signature = EvalStackOp::get_param<MetadataConstArray*>(params, 3);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, hr,
                                            metadata_import_get_custom_attribute_props(module, md_token, constructor_token, signature));
    EvalStackOp::set_return(ret, hr);
    RET_VOID_OK();
}

/// @icall: System.Reflection.MetadataImport::GetParentToken(System.IntPtr,System.Int32,System.Int32&)
static RtResultVoid metadata_import_get_parent_token_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                             const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtModuleDef*, module, get_module_handle_param(params, 0));
    auto md_token = EvalStackOp::get_param<int32_t>(params, 1);
    auto parent_token = EvalStackOp::get_param<int32_t*>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, hr, metadata_import_get_parent_token(module, md_token, parent_token));
    EvalStackOp::set_return(ret, hr);
    RET_VOID_OK();
}

RtResult<bool> metadata_import_is_valid_token(metadata::RtModuleDef* module, int32_t token_value) noexcept
{
    if (module == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    metadata::RtToken token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(token_value));
    if (token.rid == 0)
    {
        RET_OK(false);
    }

    if (token.table_type == metadata::TableType::String)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, valid, metadata_import_try_get_user_string_data(module, token_value, nullptr, nullptr));
        RET_OK(valid);
    }

    uint8_t table_index = static_cast<uint8_t>(token.table_type);
    if (table_index > static_cast<uint8_t>(metadata::TableType::CustomDebugInformation))
    {
        RET_OK(false);
    }

    RET_OK(token.rid <= module->get_table_row_num(token.table_type));
}

/// @icall: System.Reflection.MetadataImport::IsValidToken(System.IntPtr,System.Int32)
static RtResultVoid metadata_import_is_valid_token_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                           const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtModuleDef*, module, get_module_handle_param(params, 0));
    int32_t token = EvalStackOp::get_param<int32_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, metadata_import_is_valid_token(module, token));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<int32_t> SystemReflectionRuntimeModule::get_md_stream_version(intptr_t module) noexcept
{
    (void)module;
    // Metadata stream version is not exposed in this runtime; return 0 as a benign default.
    RET_OK(0);
}

/// @icall: System.Reflection.RuntimeModule::GetMDStreamVersion(System.IntPtr)
static RtResultVoid get_md_stream_version_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                  const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto module = EvalStackOp::get_param<intptr_t>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, version, SystemReflectionRuntimeModule::get_md_stream_version(module));
    EvalStackOp::set_return(ret, version);
    RET_VOID_OK();
}

RtResult<vm::RtArray*> SystemReflectionRuntimeModule::internal_get_types(metadata::RtModuleDef* module) noexcept
{
    return vm::Assembly::get_types(module->get_assembly(), false);
}

/// @icall: System.Reflection.RuntimeModule::InternalGetTypes(System.IntPtr)
static RtResultVoid internal_get_types_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtModuleDef*, module, get_module_handle_param(params, 0));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, types, SystemReflectionRuntimeModule::internal_get_types(module));
    EvalStackOp::set_return(ret, types);
    RET_VOID_OK();
}

RtResult<intptr_t> SystemReflectionRuntimeModule::get_hinstance(metadata::RtModuleDef* module) noexcept
{
    (void)module;
    // Return 0 for now - HINSTANCE is not applicable in this runtime
    RET_OK(0);
}

/// @icall: System.Reflection.RuntimeModule::GetHINSTANCE(System.IntPtr)
static RtResultVoid get_hinstance_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                          interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtModuleDef*, module, get_module_handle_param(params, 0));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(intptr_t, hinstance, SystemReflectionRuntimeModule::get_hinstance(module));
    EvalStackOp::set_return(ret, hinstance);
    RET_VOID_OK();
}

RtResultVoid SystemReflectionRuntimeModule::get_guid_internal(metadata::RtModuleDef* module, vm::RtArray* guid_bytes) noexcept
{
    if (guid_bytes == nullptr || guid_bytes->length < 16)
    {
        RET_ERR(RtErr::Argument);
    }

    auto bytes = vm::Array::get_array_data_start_as<uint8_t>(guid_bytes);
    std::memset(bytes, 0, 16);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const uint8_t*, mvid_bytes, metadata_import_get_mvid_bytes(module));
    if (mvid_bytes != nullptr)
    {
        std::memcpy(bytes, mvid_bytes, 16);
    }

    RET_VOID_OK();
}

/// @icall: System.Reflection.RuntimeModule::GetGuidInternal(System.IntPtr,System.Byte[])
static RtResultVoid get_guid_internal_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                              const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)ret;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtModuleDef*, module, get_module_handle_param(params, 0));
    auto guid_bytes = EvalStackOp::get_param<vm::RtArray*>(params, 1);
    RET_ERR_ON_FAIL(SystemReflectionRuntimeModule::get_guid_internal(module, guid_bytes));
    RET_VOID_OK();
}

RtResult<vm::RtReflectionType*> SystemReflectionRuntimeModule::get_global_type(metadata::RtModuleDef* module) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, global_cls, module->get_global_type_def());
    return vm::Reflection::get_klass_reflection_object(global_cls);
}

/// @icall: System.Reflection.RuntimeModule::GetGlobalType(System.IntPtr)
static RtResultVoid get_global_type_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                            const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtModuleDef*, module, get_module_handle_param(params, 0));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, global_type, SystemReflectionRuntimeModule::get_global_type(module));
    EvalStackOp::set_return(ret, global_type);
    RET_VOID_OK();
}

enum class ResolveTokenError
{
    OutOfRange,
    BadTable,
    Other,
};

RtResult<const metadata::RtTypeSig*> SystemReflectionRuntimeModule::resolve_type_token(metadata::RtModuleDef* module, int32_t token, vm::RtArray* type_args,
                                                                                       vm::RtArray* method_args, int32_t* error) noexcept
{
    const metadata::RtTypeSig** typesig_arr = type_args ? vm::Array::get_array_data_start_as<const metadata::RtTypeSig*>(type_args) : nullptr;
    const metadata::RtTypeSig** methodsig_arr = method_args ? vm::Array::get_array_data_start_as<const metadata::RtTypeSig*>(method_args) : nullptr;

    const metadata::RtGenericInst* class_inst;
    if (type_args != nullptr)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(class_inst,
                                  metadata::MetadataCache::get_pooled_generic_inst(typesig_arr, static_cast<uint8_t>(vm::Array::get_array_length(type_args))));
    }
    else
    {
        class_inst = nullptr;
    }
    const metadata::RtGenericInst* method_inst;
    if (method_args != nullptr)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(
            method_inst, metadata::MetadataCache::get_pooled_generic_inst(methodsig_arr, static_cast<uint8_t>(vm::Array::get_array_length(method_args))));
    }
    else
    {
        method_inst = nullptr;
    }
    metadata::RtGenericContainerContext gcc{};
    metadata::RtGenericContext gc{class_inst, method_inst};
    metadata::RtToken rt_token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(token));
    auto ret = module->get_typesig_by_type_def_ref_spec_token(rt_token, gcc, &gc);
    if (ret.is_err())
    {
        *error = (int32_t)ResolveTokenError::Other;
        RET_OK(nullptr);
    }
    else
    {
        return ret;
    }
}

/// @icall: System.Reflection.RuntimeModule::ResolveTypeToken(System.IntPtr,System.Int32,System.IntPtr[],System.IntPtr[],System.Reflection.ResolveTokenError&)
static RtResultVoid resolve_type_token_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtModuleDef*, module, get_module_handle_param(params, 0));
    auto token = EvalStackOp::get_param<int32_t>(params, 1);
    auto type_args = EvalStackOp::get_param<vm::RtArray*>(params, 2);
    auto method_args = EvalStackOp::get_param<vm::RtArray*>(params, 3);
    auto error = EvalStackOp::get_param<int32_t*>(params, 4);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, type_sig,
                                            SystemReflectionRuntimeModule::resolve_type_token(module, token, type_args, method_args, error));
    EvalStackOp::set_return(ret, type_sig);
    RET_VOID_OK();
}

RtResult<const metadata::RtMethodInfo*> SystemReflectionRuntimeModule::resolve_method_token(metadata::RtModuleDef* module, int32_t token,
                                                                                            vm::RtArray* type_args, vm::RtArray* method_args,
                                                                                            int32_t* error) noexcept
{
    const metadata::RtTypeSig** typesig_arr = type_args ? vm::Array::get_array_data_start_as<const metadata::RtTypeSig*>(type_args) : nullptr;
    const metadata::RtTypeSig** methodsig_arr = method_args ? vm::Array::get_array_data_start_as<const metadata::RtTypeSig*>(method_args) : nullptr;

    const metadata::RtGenericInst* class_inst;
    if (type_args != nullptr)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(class_inst,
                                  metadata::MetadataCache::get_pooled_generic_inst(typesig_arr, static_cast<uint8_t>(vm::Array::get_array_length(type_args))));
    }
    else
    {
        class_inst = nullptr;
    }
    const metadata::RtGenericInst* method_inst;
    if (method_args != nullptr)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(
            method_inst, metadata::MetadataCache::get_pooled_generic_inst(methodsig_arr, static_cast<uint8_t>(vm::Array::get_array_length(method_args))));
    }
    else
    {
        method_inst = nullptr;
    }
    metadata::RtGenericContainerContext gcc{};
    metadata::RtGenericContext gc{class_inst, method_inst};
    metadata::RtToken rt_token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(token));
    auto ret = module->get_method_by_token(rt_token, gcc, &gc);
    if (ret.is_err())
    {
        *error = (int32_t)ResolveTokenError::Other;
        RET_OK(nullptr);
    }
    else
    {
        return ret;
    }
}

/// @icall: System.Reflection.RuntimeModule::ResolveMethodToken(System.IntPtr,System.Int32,System.IntPtr[],System.IntPtr[],System.Reflection.ResolveTokenError&)
static RtResultVoid resolve_method_token_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                 const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtModuleDef*, module, get_module_handle_param(params, 0));
    auto token = EvalStackOp::get_param<int32_t>(params, 1);
    auto type_args = EvalStackOp::get_param<vm::RtArray*>(params, 2);
    auto method_args = EvalStackOp::get_param<vm::RtArray*>(params, 3);
    auto error = EvalStackOp::get_param<int32_t*>(params, 4);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method_info,
                                            SystemReflectionRuntimeModule::resolve_method_token(module, token, type_args, method_args, error));
    EvalStackOp::set_return(ret, method_info);
    RET_VOID_OK();
}

RtResult<const metadata::RtFieldInfo*> SystemReflectionRuntimeModule::resolve_field_token(metadata::RtModuleDef* module, int32_t token, vm::RtArray* type_args,
                                                                                          vm::RtArray* method_args, int32_t* error) noexcept
{
    const metadata::RtTypeSig** typesig_arr = type_args ? vm::Array::get_array_data_start_as<const metadata::RtTypeSig*>(type_args) : nullptr;
    const metadata::RtTypeSig** methodsig_arr = method_args ? vm::Array::get_array_data_start_as<const metadata::RtTypeSig*>(method_args) : nullptr;

    const metadata::RtGenericInst* class_inst;
    if (type_args != nullptr)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(class_inst,
                                  metadata::MetadataCache::get_pooled_generic_inst(typesig_arr, static_cast<uint8_t>(vm::Array::get_array_length(type_args))));
    }
    else
    {
        class_inst = nullptr;
    }
    const metadata::RtGenericInst* method_inst;
    if (method_args != nullptr)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(
            method_inst, metadata::MetadataCache::get_pooled_generic_inst(methodsig_arr, static_cast<uint8_t>(vm::Array::get_array_length(method_args))));
    }
    else
    {
        method_inst = nullptr;
    }
    metadata::RtGenericContainerContext gcc{};
    metadata::RtGenericContext gc{class_inst, method_inst};
    metadata::RtToken rt_token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(token));
    auto ret = module->get_field_by_token(rt_token, gcc, &gc);
    if (ret.is_err())
    {
        *error = (int32_t)ResolveTokenError::Other;
        RET_OK(nullptr);
    }
    else
    {
        return ret;
    }
}

/// @icall: System.Reflection.RuntimeModule::ResolveFieldToken(System.IntPtr,System.Int32,System.IntPtr[],System.IntPtr[],System.Reflection.ResolveTokenError&)
static RtResultVoid resolve_field_token_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtModuleDef*, module, get_module_handle_param(params, 0));
    auto token = EvalStackOp::get_param<int32_t>(params, 1);
    auto type_args = EvalStackOp::get_param<vm::RtArray*>(params, 2);
    auto method_args = EvalStackOp::get_param<vm::RtArray*>(params, 3);
    auto error = EvalStackOp::get_param<int32_t*>(params, 4);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtFieldInfo*, field_info,
                                            SystemReflectionRuntimeModule::resolve_field_token(module, token, type_args, method_args, error));
    EvalStackOp::set_return(ret, field_info);
    RET_VOID_OK();
}

RtResult<vm::RtString*> SystemReflectionRuntimeModule::resolve_string_token(metadata::RtModuleDef* module, int32_t token, int32_t* error) noexcept
{
    metadata::RtToken rt_token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(token));
    if (rt_token.table_type != metadata::TableType::String)
    {
        *error = (int32_t)ResolveTokenError::BadTable;
        RET_OK(nullptr);
    }
    auto ret = module->get_user_string(rt_token.rid);
    if (ret.is_err())
    {
        *error = (int32_t)ResolveTokenError::Other;
        RET_OK(nullptr);
    }
    else
    {
        return ret;
    }
}

/// @icall: System.Reflection.RuntimeModule::ResolveStringToken(System.IntPtr,System.Int32,System.Reflection.ResolveTokenError&)
static RtResultVoid resolve_string_token_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                 const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtModuleDef*, module, get_module_handle_param(params, 0));
    auto token = EvalStackOp::get_param<int32_t>(params, 1);
    auto error = EvalStackOp::get_param<int32_t*>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, string, SystemReflectionRuntimeModule::resolve_string_token(module, token, error));
    EvalStackOp::set_return(ret, string);
    RET_VOID_OK();
}

RtResult<vm::RtObject*> SystemReflectionRuntimeModule::resolve_member_token(metadata::RtModuleDef* module, int32_t token, vm::RtArray* type_args,
                                                                            vm::RtArray* method_args, int32_t* error) noexcept
{
    const metadata::RtTypeSig** typesig_arr = type_args ? vm::Array::get_array_data_start_as<const metadata::RtTypeSig*>(type_args) : nullptr;
    const metadata::RtTypeSig** methodsig_arr = method_args ? vm::Array::get_array_data_start_as<const metadata::RtTypeSig*>(method_args) : nullptr;

    const metadata::RtGenericInst* class_inst;
    if (type_args != nullptr)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(class_inst,
                                  metadata::MetadataCache::get_pooled_generic_inst(typesig_arr, static_cast<uint8_t>(vm::Array::get_array_length(type_args))));
    }
    else
    {
        class_inst = nullptr;
    }
    const metadata::RtGenericInst* method_inst;
    if (method_args != nullptr)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(
            method_inst, metadata::MetadataCache::get_pooled_generic_inst(methodsig_arr, static_cast<uint8_t>(vm::Array::get_array_length(method_args))));
    }
    else
    {
        method_inst = nullptr;
    }
    metadata::RtGenericContainerContext gcc{};
    metadata::RtGenericContext gc{class_inst, method_inst};
    metadata::RtToken rt_token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(token));
    vm::RtObject* ret = nullptr;
    switch (rt_token.table_type)
    {
    case metadata::TableType::TypeDef:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, module->get_class_by_type_def_rid(rt_token.rid));
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, ref_type, vm::Reflection::get_klass_reflection_object(klass));
        ret = (vm::RtObject*)ref_type;
        break;
    }
    case metadata::TableType::Field:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtFieldInfo*, field_info, module->get_field_by_rid(rt_token.rid));
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionField*, field_obj, vm::Reflection::get_field_reflection_object(field_info, field_info->parent));
        ret = (vm::RtObject*)field_obj;
        break;
    }
    case metadata::TableType::Method:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method_info, module->get_method_by_rid(rt_token.rid));
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionMethod*, method_obj,
                                                vm::Reflection::get_method_reflection_object(method_info, method_info->parent));
        ret = (vm::RtObject*)method_obj;
        break;
    }
    case metadata::TableType::MemberRef:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtRuntimeHandle, handle, module->get_member_ref_by_rid(rt_token.rid, gcc, &gc));
        switch (handle.type)
        {
        case metadata::RtRuntimeHandleType::Method:
        {
            const metadata::RtMethodInfo* method_info = handle.method;
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionMethod*, method_obj,
                                                    vm::Reflection::get_method_reflection_object(method_info, method_info->parent));
            ret = (vm::RtObject*)method_obj;
            break;
        }
        case metadata::RtRuntimeHandleType::Field:
        {
            const metadata::RtFieldInfo* field_info = handle.field;
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionField*, field_obj,
                                                    vm::Reflection::get_field_reflection_object(field_info, field_info->parent));
            ret = (vm::RtObject*)field_obj;
            break;
        }
        case metadata::RtRuntimeHandleType::Type:
        {
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(handle.typeSig));
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, ref_type, vm::Reflection::get_klass_reflection_object(klass));
            ret = (vm::RtObject*)ref_type;
            break;
        }
        default:
        {
            assert(false && "Invalid MemberRef handle type");
            *error = (int32_t)ResolveTokenError::Other;
            ret = nullptr;
        }
        }
        break;
    }
    default:
    {
        *error = (int32_t)ResolveTokenError::BadTable;
        ret = nullptr;
    }
    }
    RET_OK(ret);
}

RtResult<vm::RtArray*> SystemReflectionRuntimeModule::resolve_signature(metadata::RtModuleDef* module, int32_t token, int32_t* error) noexcept
{
    metadata::RtToken rt_token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(token));
    const metadata::CliImage& cli_image = module->get_cli_image();

    uint32_t signature = 0;
    switch (rt_token.table_type)
    {
    case metadata::TableType::TypeSpec:
    {
        auto row_type_spec = cli_image.read_type_spec(rt_token.rid);
        if (!row_type_spec)
        {
            *error = (int32_t)ResolveTokenError::OutOfRange;
            RET_OK(nullptr);
        }
        signature = row_type_spec->signature;
        break;
    }
    case metadata::TableType::Field:
    {
        auto row_field = cli_image.read_field(rt_token.rid);
        if (!row_field)
        {
            *error = (int32_t)ResolveTokenError::OutOfRange;
            RET_OK(nullptr);
        }
        signature = row_field->signature;
        break;
    }
    case metadata::TableType::Method:
    {
        auto row_method = cli_image.read_method(rt_token.rid);
        if (!row_method)
        {
            *error = (int32_t)ResolveTokenError::OutOfRange;
            RET_OK(nullptr);
        }
        signature = row_method->signature;
        break;
    }
    case metadata::TableType::MemberRef:
    {
        auto row_member_ref = cli_image.read_member_ref(rt_token.rid);
        if (!row_member_ref)
        {
            *error = (int32_t)ResolveTokenError::OutOfRange;
            RET_OK(nullptr);
        }
        signature = row_member_ref->signature;
        break;
    }
    case metadata::TableType::StandaloneSig:
    {
        auto row_standalone = cli_image.read_stand_alone_sig(rt_token.rid);
        if (!row_standalone)
        {
            *error = (int32_t)ResolveTokenError::OutOfRange;
            RET_OK(nullptr);
        }
        signature = row_standalone->signature;
        break;
    }
    default:
    {
        *error = (int32_t)ResolveTokenError::BadTable;
        RET_OK(nullptr);
    }
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(utils::BinaryReader, blob_reader, module->get_decoded_blob_reader(signature));
    metadata::RtClass* byte_klass = vm::Class::get_corlib_types().cls_byte;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, byte_arr,
                                            LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERNAL(byte_klass, static_cast<int32_t>(blob_reader.length()), "icalls::SystemReflectionRuntimeModule::resolve_signature"));
    std::memcpy(vm::Array::get_array_data_start_as<uint8_t>(byte_arr), blob_reader.data(), blob_reader.length());
    RET_OK(byte_arr);
}

/// @icall: System.Reflection.RuntimeModule::ResolveSignature(System.IntPtr,System.Int32,System.Reflection.ResolveTokenError&)
static RtResultVoid resolve_signature_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                              const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtModuleDef*, module, get_module_handle_param(params, 0));
    auto token = EvalStackOp::get_param<int32_t>(params, 1);
    auto error = EvalStackOp::get_param<int32_t*>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, sig, SystemReflectionRuntimeModule::resolve_signature(module, token, error));
    EvalStackOp::set_return(ret, sig);
    RET_VOID_OK();
}

/// @icall: System.Reflection.RuntimeModule::ResolveMemberToken(System.IntPtr,System.Int32,System.IntPtr[],System.IntPtr[],System.Reflection.ResolveTokenError&)
static RtResultVoid resolve_member_token_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                 const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtModuleDef*, module, get_module_handle_param(params, 0));
    auto token = EvalStackOp::get_param<int32_t>(params, 1);
    auto type_args = EvalStackOp::get_param<vm::RtArray*>(params, 2);
    auto method_args = EvalStackOp::get_param<vm::RtArray*>(params, 3);
    auto error = EvalStackOp::get_param<int32_t*>(params, 4);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, member,
                                            SystemReflectionRuntimeModule::resolve_member_token(module, token, type_args, method_args, error));
    EvalStackOp::set_return(ret, member);
    RET_VOID_OK();
}

utils::Span<vm::InternalCallEntry> SystemReflectionRuntimeModule::get_net10_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.Reflection.MetadataImport::GetMetadataImport(System.Reflection.RuntimeModule)", nullptr, get_metadata_import_invoker},
        {"System.Reflection.MetadataImport::GetName(System.IntPtr,System.Int32,System.Byte*&)", nullptr, metadata_import_get_name_invoker},
        {"System.Reflection.MetadataImport::GetNamespace(System.IntPtr,System.Int32,System.Byte*&)", nullptr,
         metadata_import_get_namespace_invoker},
        {"System.Reflection.MetadataImport::GetPropertyProps(System.IntPtr,System.Int32,System.Void*&,System.Int32&,System.Reflection.ConstArray&)", nullptr,
         metadata_import_get_property_props_invoker},
        {"System.Reflection.MetadataImport::GetEventProps(System.IntPtr,System.Int32,System.Void*&,System.Int32&)", nullptr,
         metadata_import_get_event_props_invoker},
        {"System.Reflection.MetadataImport::GetFieldDefProps(System.IntPtr,System.Int32,System.Int32&)", nullptr,
         metadata_import_get_field_def_props_invoker},
        {"System.Reflection.MetadataImport::GetParamDefProps(System.IntPtr,System.Int32,System.Int32&,System.Int32&)", nullptr,
         metadata_import_get_param_def_props_invoker},
        {"System.Reflection.MetadataImport::GetFieldMarshal(System.IntPtr,System.Int32,System.Reflection.ConstArray&)", nullptr,
         metadata_import_get_field_marshal_invoker},
        {"System.Reflection.MetadataImport::GetFieldOffset(System.IntPtr,System.Int32,System.Int32,System.Int32&,System.Boolean&)", nullptr,
         metadata_import_get_field_offset_invoker},
        {"System.Reflection.MetadataImport::GetClassLayout(System.IntPtr,System.Int32,System.Int32&,System.Int32&)", nullptr,
         metadata_import_get_class_layout_invoker},
        {"System.Reflection.MetadataImport::GetDefaultValue(System.IntPtr,System.Int32,System.Int64&,System.Char*&,System.Int32&,System.Int32&)", nullptr,
         metadata_import_get_default_value_invoker},
        {"System.Reflection.MetadataImport::GetSignatureFromToken(System.IntPtr,System.Int32,System.Reflection.ConstArray&)", nullptr,
         metadata_import_get_signature_from_token_invoker},
        {"System.Reflection.MetadataImport::GetScopeProps(System.IntPtr,System.Guid&)", nullptr, metadata_import_get_scope_props_invoker},
        {"System.Reflection.MetadataImport::GetUserString(System.IntPtr,System.Int32,System.Char*&,System.Int32&)", nullptr,
         metadata_import_get_user_string_invoker},
        {"System.Reflection.MetadataImport::GetMemberRefProps(System.IntPtr,System.Int32,System.Reflection.ConstArray&)", nullptr,
         metadata_import_get_member_ref_props_invoker},
        {"System.Reflection.MetadataImport::GetSigOfMethodDef(System.IntPtr,System.Int32,System.Reflection.ConstArray&)", nullptr,
         metadata_import_get_sig_of_method_def_invoker},
        {"System.Reflection.MetadataImport::GetSigOfFieldDef(System.IntPtr,System.Int32,System.Reflection.ConstArray&)", nullptr,
         metadata_import_get_sig_of_field_def_invoker},
        {"System.Reflection.MetadataImport::GetCustomAttributeProps(System.IntPtr,System.Int32,System.Int32&,System.Reflection.ConstArray&)", nullptr,
         metadata_import_get_custom_attribute_props_invoker},
        {"System.Reflection.MetadataImport::GetParentToken(System.IntPtr,System.Int32,System.Int32&)", nullptr,
         metadata_import_get_parent_token_invoker},
        {"System.Reflection.MetadataImport::IsValidToken(System.IntPtr,System.Int32)", nullptr, metadata_import_is_valid_token_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

utils::Span<vm::InternalCallEntry> SystemReflectionRuntimeModule::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.Reflection.RuntimeModule::get_MetadataToken(System.Reflection.Module)",
         (vm::InternalCallFunction)&SystemReflectionRuntimeModule::get_metadata_token, get_metadata_token_invoker_system_reflection_runtimemodule},
        {"System.Reflection.MetadataImport::GetMetadataImport(System.Reflection.RuntimeModule)", nullptr, get_metadata_import_invoker},
        {"System.Reflection.MetadataImport::GetMetadataImport", nullptr, get_metadata_import_invoker},
        {"System.Reflection.MetadataImport::GetName(System.IntPtr,System.Int32,System.Byte*&)", nullptr, metadata_import_get_name_invoker},
        {"System.Reflection.MetadataImport::GetNamespace(System.IntPtr,System.Int32,System.Byte*&)", nullptr,
         metadata_import_get_namespace_invoker},
        {"System.Reflection.MetadataImport::GetPropertyProps(System.IntPtr,System.Int32,System.Void*&,System.Int32&,System.Reflection.ConstArray&)", nullptr,
         metadata_import_get_property_props_invoker},
        {"System.Reflection.MetadataImport::GetEventProps(System.IntPtr,System.Int32,System.Void*&,System.Int32&)", nullptr,
         metadata_import_get_event_props_invoker},
        {"System.Reflection.MetadataImport::GetEventProps", nullptr, metadata_import_get_event_props_invoker},
        {"System.Reflection.MetadataImport::GetFieldDefProps(System.IntPtr,System.Int32,System.Int32&)", nullptr,
         metadata_import_get_field_def_props_invoker},
        {"System.Reflection.MetadataImport::GetFieldDefProps", nullptr, metadata_import_get_field_def_props_invoker},
        {"System.Reflection.MetadataImport::GetParamDefProps(System.IntPtr,System.Int32,System.Int32&,System.Int32&)", nullptr,
         metadata_import_get_param_def_props_invoker},
        {"System.Reflection.MetadataImport::GetParamDefProps", nullptr, metadata_import_get_param_def_props_invoker},
        {"System.Reflection.MetadataImport::GetFieldMarshal(System.IntPtr,System.Int32,System.Reflection.ConstArray&)", nullptr,
         metadata_import_get_field_marshal_invoker},
        {"System.Reflection.MetadataImport::GetFieldMarshal", nullptr, metadata_import_get_field_marshal_invoker},
        {"System.Reflection.MetadataImport::GetFieldOffset(System.IntPtr,System.Int32,System.Int32,System.Int32&,System.Boolean&)", nullptr,
         metadata_import_get_field_offset_invoker},
        {"System.Reflection.MetadataImport::GetFieldOffset", nullptr, metadata_import_get_field_offset_invoker},
        {"System.Reflection.MetadataImport::GetClassLayout(System.IntPtr,System.Int32,System.Int32&,System.Int32&)", nullptr,
         metadata_import_get_class_layout_invoker},
        {"System.Reflection.MetadataImport::GetClassLayout", nullptr, metadata_import_get_class_layout_invoker},
        {"System.Reflection.MetadataImport::GetDefaultValue(System.IntPtr,System.Int32,System.Int64&,System.Char*&,System.Int32&,System.Int32&)", nullptr,
         metadata_import_get_default_value_invoker},
        {"System.Reflection.MetadataImport::GetDefaultValue", nullptr, metadata_import_get_default_value_invoker},
        {"System.Reflection.MetadataImport::GetSignatureFromToken(System.IntPtr,System.Int32,System.Reflection.ConstArray&)", nullptr,
         metadata_import_get_signature_from_token_invoker},
        {"System.Reflection.MetadataImport::GetScopeProps(System.IntPtr,System.Guid&)", nullptr, metadata_import_get_scope_props_invoker},
        {"System.Reflection.MetadataImport::GetScopeProps", nullptr, metadata_import_get_scope_props_invoker},
        {"System.Reflection.MetadataImport::GetUserString(System.IntPtr,System.Int32,System.Char*&,System.Int32&)", nullptr,
         metadata_import_get_user_string_invoker},
        {"System.Reflection.MetadataImport::GetUserString", nullptr, metadata_import_get_user_string_invoker},
        {"System.Reflection.MetadataImport::GetMemberRefProps(System.IntPtr,System.Int32,System.Reflection.ConstArray&)", nullptr,
         metadata_import_get_member_ref_props_invoker},
        {"System.Reflection.MetadataImport::GetMemberRefProps", nullptr, metadata_import_get_member_ref_props_invoker},
        {"System.Reflection.MetadataImport::GetSigOfMethodDef(System.IntPtr,System.Int32,System.Reflection.ConstArray&)", nullptr,
         metadata_import_get_sig_of_method_def_invoker},
        {"System.Reflection.MetadataImport::GetSigOfMethodDef", nullptr, metadata_import_get_sig_of_method_def_invoker},
        {"System.Reflection.MetadataImport::GetSigOfFieldDef(System.IntPtr,System.Int32,System.Reflection.ConstArray&)", nullptr,
         metadata_import_get_sig_of_field_def_invoker},
        {"System.Reflection.MetadataImport::GetSigOfFieldDef", nullptr, metadata_import_get_sig_of_field_def_invoker},
        {"System.Reflection.MetadataImport::GetCustomAttributeProps(System.IntPtr,System.Int32,System.Int32&,System.Reflection.ConstArray&)", nullptr,
         metadata_import_get_custom_attribute_props_invoker},
        {"System.Reflection.MetadataImport::GetCustomAttributeProps", nullptr, metadata_import_get_custom_attribute_props_invoker},
        {"System.Reflection.MetadataImport::GetParentToken(System.IntPtr,System.Int32,System.Int32&)", nullptr,
         metadata_import_get_parent_token_invoker},
        {"System.Reflection.MetadataImport::GetParentToken", nullptr, metadata_import_get_parent_token_invoker},
        {"System.Reflection.MetadataImport::IsValidToken(System.IntPtr,System.Int32)", nullptr, metadata_import_is_valid_token_invoker},
        {"System.Reflection.RuntimeModule::GetMDStreamVersion(System.IntPtr)", (vm::InternalCallFunction)&SystemReflectionRuntimeModule::get_md_stream_version,
         get_md_stream_version_invoker},
        {"System.Reflection.RuntimeModule::InternalGetTypes(System.IntPtr)", (vm::InternalCallFunction)&SystemReflectionRuntimeModule::internal_get_types,
         internal_get_types_invoker},
        {"System.Reflection.RuntimeModule::GetHINSTANCE(System.IntPtr)", (vm::InternalCallFunction)&SystemReflectionRuntimeModule::get_hinstance,
         get_hinstance_invoker},
        {"System.Reflection.RuntimeModule::GetGuidInternal(System.IntPtr,System.Byte[])",
         (vm::InternalCallFunction)&SystemReflectionRuntimeModule::get_guid_internal, get_guid_internal_invoker},
        {"System.Reflection.RuntimeModule::GetGlobalType(System.IntPtr)", (vm::InternalCallFunction)&SystemReflectionRuntimeModule::get_global_type,
         get_global_type_invoker},
        {"System.Reflection.RuntimeModule::ResolveTypeToken(System.IntPtr,System.Int32,System.IntPtr[],System.IntPtr[],System.Reflection.ResolveTokenError&)",
         (vm::InternalCallFunction)&SystemReflectionRuntimeModule::resolve_type_token, resolve_type_token_invoker},
        {"System.Reflection.RuntimeModule::ResolveMethodToken(System.IntPtr,System.Int32,System.IntPtr[],System.IntPtr[],System.Reflection.ResolveTokenError&)",
         (vm::InternalCallFunction)&SystemReflectionRuntimeModule::resolve_method_token, resolve_method_token_invoker},
        {"System.Reflection.RuntimeModule::ResolveFieldToken(System.IntPtr,System.Int32,System.IntPtr[],System.IntPtr[],System.Reflection.ResolveTokenError&)",
         (vm::InternalCallFunction)&SystemReflectionRuntimeModule::resolve_field_token, resolve_field_token_invoker},
        {"System.Reflection.RuntimeModule::ResolveStringToken(System.IntPtr,System.Int32,System.Reflection.ResolveTokenError&)",
         (vm::InternalCallFunction)&SystemReflectionRuntimeModule::resolve_string_token, resolve_string_token_invoker},
        {"System.Reflection.RuntimeModule::ResolveMemberToken(System.IntPtr,System.Int32,System.IntPtr[],System.IntPtr[],System.Reflection.ResolveTokenError&)",
         (vm::InternalCallFunction)&SystemReflectionRuntimeModule::resolve_member_token, resolve_member_token_invoker},
        {"System.Reflection.RuntimeModule::ResolveSignature(System.IntPtr,System.Int32,System.Reflection.ResolveTokenError&)",
         (vm::InternalCallFunction)&SystemReflectionRuntimeModule::resolve_signature, resolve_signature_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr
