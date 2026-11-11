#include "aot_module.h"
#include "utils/hashmap.h"
#include "utils/string_util.h"

namespace leanclr
{
namespace metadata
{

static utils::HashMap<const char*, const RtAotModuleData*, utils::CStrHasher, utils::CStrCompare>& get_s_aotModuleMap()
{
    static utils::HashMap<const char*, const RtAotModuleData*, utils::CStrHasher, utils::CStrCompare> value;
    return value;
}

static const RtAotGenericMethodData* s_generic_entries = nullptr;
static uint32_t s_generic_count = 0;

RtResultVoid AotModule::register_aot_modules(const RtAotModulesData* initData)
{
    if (!initData || (initData->module_count && !initData->modules) || (initData->generic_method_entry_count && !initData->generic_method_entries))
        return RtErr::BadImageFormat;
    // Validate before changing registry state. The index itself is static data;
    // registration never loads argument assemblies or allocates a generic map.
    for (uint32_t i = 0; i < initData->module_count; ++i)
        if (!initData->modules[i] || !initData->modules[i]->module_name || initData->modules[i]->return_protocol > 3)
            return RtErr::BadImageFormat;
    for (uint32_t i = 0; i < initData->generic_method_entry_count; ++i)
    {
        const auto& entry = initData->generic_method_entries[i];
        if (!entry.definition_module || !*entry.definition_module || entry.return_protocol > 3 ||
            (!entry.class_argument_count && !entry.method_argument_count) || (entry.class_argument_count && !entry.class_arguments) ||
            (entry.method_argument_count && !entry.method_arguments) || (entry.implementation.token >> 24) != 6 ||
            !RtToken::decode_rid(entry.implementation.token) || !entry.implementation.method_ptr || !entry.implementation.virtual_method_ptr ||
            !entry.implementation.invoke_method_ptr || (i && initData->generic_method_entries[i - 1].key_hash > entry.key_hash))
            return RtErr::BadImageFormat;
    }
    for (uint32_t i = 0; i < initData->module_count; i++)
    {
        const RtAotModuleData* moduleData = initData->modules[i];
        get_s_aotModuleMap().insert({moduleData->module_name, moduleData});
    }
    s_generic_entries = initData->generic_method_entries;
    s_generic_count = initData->generic_method_entry_count;
    RET_VOID_OK();
}

const RtAotModuleData* AotModule::find_aot_module_by_name(const char* module_name)
{
    auto it = get_s_aotModuleMap().find(module_name);
    return it != get_s_aotModuleMap().end() ? it->second : nullptr;
}

namespace
{
bool matches_type(const RtAotTypeIdentity& identity, const RtTypeSig* type, unsigned depth);

bool matches_arguments(const RtAotTypeIdentity* identities, uint32_t count, const RtGenericInst* inst, unsigned depth = 0)
{
    if (depth > 16 || (count && !identities) || count != (inst ? inst->generic_arg_count : 0))
        return false;
    for (uint32_t i = 0; i < count; ++i)
        if (!matches_type(identities[i], inst->generic_args[i], depth))
            return false;
    return true;
}

bool matches_definition(const RtAotTypeIdentity& identity, uint32_t gid)
{
    const auto* module = RtModuleDef::get_module_by_id(RtMetadata::decode_module_id_from_gid(gid));
    return module && identity.module_name && std::strcmp(module->get_name_no_ext(), identity.module_name) == 0 &&
           RtMetadata::decode_rid_from_gid(gid) == RtToken::decode_rid(identity.type_token);
}

bool matches_type(const RtAotTypeIdentity& identity, const RtTypeSig* type, unsigned depth)
{
    if (depth > 16 || !type || type->is_by_ref() || type->ele_type != identity.element_type)
        return false;
    switch (type->ele_type)
    {
    case RtElementType::Class:
    case RtElementType::ValueType:
        return matches_definition(identity, type->data.type_def_gid);
    case RtElementType::GenericInst:
        return matches_definition(identity, type->data.generic_class->base_type_def_gid) &&
               matches_arguments(identity.arguments, identity.argument_count, type->data.generic_class->class_inst, depth + 1);
    case RtElementType::SZArray:
        return identity.arguments && identity.argument_count == 1 && matches_type(identity.arguments[0], type->data.element_type, depth + 1);
    default:
        return identity.argument_count == 0;
    }
}

struct GenericKeyHash
{
    uint64_t value = UINT64_C(14695981039346656037);

    void byte(uint8_t b)
    {
        value = (value ^ b) * UINT64_C(1099511628211);
    }

    void integer(uint32_t n)
    {
        for (unsigned i = 0; i < 4; ++i)
        {
            byte(static_cast<uint8_t>(n));
            n >>= 8;
        }
    }

    void name(const char* text)
    {
        while (*text)
            byte(static_cast<uint8_t>(*text++));
        byte(0);
    }

    bool definition(uint32_t gid)
    {
        const auto* module = RtModuleDef::get_module_by_id(RtMetadata::decode_module_id_from_gid(gid));
        if (!module)
            return false;
        name(module->get_name_no_ext());
        integer(0x02000000u | RtMetadata::decode_rid_from_gid(gid));
        return true;
    }

    bool arguments(const RtGenericInst* inst, unsigned depth = 0)
    {
        if (depth > 16)
            return false;
        integer(inst ? inst->generic_arg_count : 0);
        if (inst)
            for (uint32_t i = 0; i < inst->generic_arg_count; ++i)
                if (!type(inst->generic_args[i], depth))
                    return false;
        return true;
    }

    bool type(const RtTypeSig* sig, unsigned depth)
    {
        if (depth > 16 || !sig || sig->is_by_ref())
            return false;
        integer(static_cast<uint32_t>(sig->ele_type));
        switch (sig->ele_type)
        {
        case RtElementType::Class:
        case RtElementType::ValueType:
            return definition(sig->data.type_def_gid);
        case RtElementType::GenericInst:
            return definition(sig->data.generic_class->base_type_def_gid) && arguments(sig->data.generic_class->class_inst, depth + 1);
        case RtElementType::SZArray:
            return type(sig->data.element_type, depth + 1);
        case RtElementType::Boolean:
        case RtElementType::Char:
        case RtElementType::I1:
        case RtElementType::U1:
        case RtElementType::I2:
        case RtElementType::U2:
        case RtElementType::I4:
        case RtElementType::U4:
        case RtElementType::I8:
        case RtElementType::U8:
        case RtElementType::R4:
        case RtElementType::R8:
        case RtElementType::I:
        case RtElementType::U:
        case RtElementType::String:
        case RtElementType::Object:
            return true;
        default:
            return false;
        }
    }
};
} // namespace

const RtAotGenericMethodData* AotModule::find_aot_generic_method_impl(const RtMethodInfo* method)
{
    if (!s_generic_count || !method->generic_method || method->generic_container)
        return nullptr;
    GenericKeyHash key;
    key.name(method->parent->image->get_name_no_ext());
    key.integer(method->token);
    const auto& context = method->generic_method->generic_context;
    if (!key.arguments(context.class_inst) || !key.arguments(context.method_inst))
        return nullptr;
    uint32_t left = 0, right = s_generic_count;
    while (left < right)
    {
        const uint32_t mid = left + (right - left) / 2;
        if (s_generic_entries[mid].key_hash < key.value)
            left = mid + 1;
        else
            right = mid;
    }
    // Equal hashes are not identities: retain full structural checks, including
    // the defining assembly and every argument's defining assembly.
    for (; left < s_generic_count && s_generic_entries[left].key_hash == key.value; ++left)
    {
        const auto& entry = s_generic_entries[left];
        if (entry.implementation.token == method->token && std::strcmp(entry.definition_module, method->parent->image->get_name_no_ext()) == 0 &&
            matches_arguments(entry.class_arguments, entry.class_argument_count, context.class_inst) &&
            matches_arguments(entry.method_arguments, entry.method_argument_count, context.method_inst))
            return &entry;
    }
    return nullptr;
}

RtAotMethodBinding AotModule::find_aot_method_binding(const RtMethodInfo* method)
{
    if (method->generic_method && !method->generic_container)
    {
        const auto* entry = find_aot_generic_method_impl(method);
        return {entry ? &entry->implementation : nullptr, entry ? entry->return_protocol : uint8_t(0)};
    }
    if (method->generic_method != nullptr || method->generic_container != nullptr || method->parent->generic_container != nullptr)
    {
        return {nullptr, 0};
    }
    const auto* entry = find_aot_method_def_impl(method->parent->image, method->token);
    return {entry, entry ? static_cast<uint8_t>(method->parent->image->get_aot_module_data()->return_protocol) : uint8_t(0)};
}

const RtAotMethodDefData* AotModule::find_aot_method_impl(const RtMethodInfo* method)
{
    return find_aot_method_binding(method).implementation;
}

const RtAotMethodDefData* AotModule::find_aot_method_def_impl(const RtModuleDef* module, EncodedTokenId token)
{
    const RtAotModuleData* aotModuleData = module->get_aot_module_data();
    if (aotModuleData == nullptr || aotModuleData->method_def_entry_count == 0)
    {
        return nullptr;
    }
    const RtAotMethodDefData* entries = aotModuleData->method_def_entries;
    uint32_t count = aotModuleData->method_def_entry_count;
    uint32_t left = 0;
    uint32_t right = count;
    while (left < right)
    {
        uint32_t mid = left + (right - left) / 2;
        EncodedTokenId cur_token = entries[mid].token;
        if (cur_token < token)
        {
            left = mid + 1;
        }
        else if (cur_token > token)
        {
            right = mid;
        }
        else
        {
            return &entries[mid];
        }
    }
    return nullptr;
}

const RtAotMethodMonoPInvokeCallbackData* AotModule::find_mono_pinvoke_callback_method(const RtModuleDef* module, EncodedTokenId token)
{
    const RtAotModuleData* aotModuleData = module->get_aot_module_data();
    if (aotModuleData == nullptr || aotModuleData->mono_pinvoke_callback_entry_count == 0)
    {
        return nullptr;
    }
    const RtAotMethodMonoPInvokeCallbackData* entries = aotModuleData->mono_pinvoke_callback_entries;
    uint32_t count = aotModuleData->mono_pinvoke_callback_entry_count;
    uint32_t left = 0;
    uint32_t right = count;
    while (left < right)
    {
        uint32_t mid = left + (right - left) / 2;
        EncodedTokenId cur_token = entries[mid].token;
        if (cur_token < token)
        {
            left = mid + 1;
        }
        else if (cur_token > token)
        {
            right = mid;
        }
        else
        {
            return &entries[mid];
        }
    }
    return nullptr;
}

} // namespace metadata
} // namespace leanclr
