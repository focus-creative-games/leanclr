#include "module_def.h"

#include "const_strs.h"
#include "utils/rt_vector.h"
#include "utils/string_util.h"
#include "utils/hashmap.h"
#include "alloc/general_allocator.h"
#include "metadata/metadata_cache.h"
#include "metadata/metadata_compare.h"
#include "vm/rt_string.h"
#include "vm/assembly.h"
#include "vm/class.h"
#include "vm/method.h"

namespace leanclr
{
namespace metadata
{

constexpr EncodedTokenId RtToken::Invalid;

// Helper constants

static uint32_t allocate_image_id()
{
    static uint32_t next_id = 1;
    if (next_id >= MAX_ASSEMBLY_ID_COUNT)
    {
        // Exhausted all available IDs
        return 0;
    }
    return next_id++;
}

static utils::Vector<RtModuleDef*> g_loadedModuleDefs;
static RtModuleDef* g_corlibModule = nullptr;

void RtModuleDef::register_module_def(RtModuleDef* moduleDef)
{
    if (moduleDef->is_corlib())
    {
        assert(g_corlibModule == nullptr && "Corlib module already registered");
        g_corlibModule = moduleDef;
    }
    g_loadedModuleDefs.push_back(moduleDef);
    assert(g_loadedModuleDefs.size() == moduleDef->get_id());
}

utils::Span<RtModuleDef*> RtModuleDef::get_registered_modules()
{
    return utils::Span<RtModuleDef*>(g_loadedModuleDefs.data(), g_loadedModuleDefs.size());
}

RtModuleDef* RtModuleDef::find_module(const char* name)
{
    for (auto it = g_loadedModuleDefs.begin(); it != g_loadedModuleDefs.end(); ++it)
    {
        if (strcmp((*it)->get_name_no_ext(), name) == 0)
        {
            return *it;
        }
    }
    return nullptr;
}

RtModuleDef* RtModuleDef::get_module_by_id(uint32_t id)
{
    if (id == 0 || id > g_loadedModuleDefs.size())
    {
        return nullptr;
    }
    return g_loadedModuleDefs[id - 1];
}

RtModuleDef* RtModuleDef::get_corlib_module()
{
    return g_corlibModule;
}

RtResult<const char*> RtModuleDef::get_string(uint32_t index) const
{
    const CliHeap& heap = _cliImage.get_string_heap();
    if (index < heap.size)
    {
        RET_OK(reinterpret_cast<const char*>(heap.data + index));
    }
    RET_ASSERT_ERR(RtErr::BadImageFormat);
}

RtResult<const uint8_t*> RtModuleDef::get_blob(uint32_t index) const
{
    const CliHeap& heap = _cliImage.get_blob_heap();
    if (index < heap.size)
    {
        RET_OK(heap.data + index);
    }
    RET_ASSERT_ERR(RtErr::BadImageFormat);
}

RtResult<vm::RtString*> RtModuleDef::get_user_string(uint32_t index)
{
    auto it = _userStringMap.find(index);
    if (it != _userStringMap.end())
    {
        RET_OK(it->second);
    }
    auto& heap = _cliImage.get_us_heap();
    if (index >= heap.size)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    const uint8_t* data = heap.data + index;
    uint32_t str_size = 0;
    size_t size_length = 0;
    if (!utils::BinaryReader::try_decode_compressed_uint32(data, heap.size - index, str_size, size_length) || index + size_length + str_size > heap.size)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    if (str_size == 0)
    {
        RET_OK(vm::String::get_empty_string()); // Empty string
    }
    if (str_size % 2 == 1)
    {
        vm::RtString* newStr = vm::String::create_string_from_utf16chars(reinterpret_cast<const uint16_t*>(data + size_length), (str_size - 1) / 2);
        _userStringMap.insert({index, newStr});
        RET_OK(newStr);
    }
    RET_ASSERT_ERR(RtErr::BadImageFormat);
}

RtResultVoid RtModuleDef::load()
{
    _id = allocate_image_id();
    if (_id == 0)
    {
        return RtErr::ExceedMaxImageCount;
    }

    RET_ERR_ON_FAIL(setup_assembly_name());

    _nameNoExt = _assemblyName.name;
    _name = utils::StringUtil::concat(_assemblyName.name, ".dll");

    _corLib = strcmp(_nameNoExt, STR_CORLIB_NAME) == 0;

    uint32_t assemblyRefCount = _cliImage.get_table_row_num(TableType::AssemblyRef);
    if (assemblyRefCount > 0)
    {
        _referenceAssemblies = _pool.calloc_any<ReferenceAssembly>(assemblyRefCount);
        _referenceAssemblyCount = assemblyRefCount;
    }

    uint32_t typeDefCount = _cliImage.get_table_row_num(TableType::TypeDef);
    if (typeDefCount > 0)
    {
        _classes = _pool.calloc_any<RtClass*>(typeDefCount);
        _classCount = typeDefCount;
    }
    uint32_t methodCount = _cliImage.get_table_row_num(TableType::Method);
    if (methodCount > 0)
    {
        _methods = _pool.calloc_any<const RtMethodInfo*>(methodCount);
        _methodCount = methodCount;
    }
    _typeDefByValTypeSigs = _pool.calloc_any<RtTypeSig>(typeDefCount);
    _typeDefByRefTypeSigs = _pool.calloc_any<RtTypeSig>(typeDefCount);

    RET_ERR_ON_FAIL(setup_generic_params_and_containers());
    // setup nested classes must be before setup_type_fullname_map because we don't keep fullname map for nested types
    RET_ERR_ON_FAIL(setup_nested_classes());
    RET_ERR_ON_FAIL(setup_type_fullname_map());
    setup_field_offsets();
    setup_class_layouts();

    RET_VOID_OK();
}

RtResultVoid RtModuleDef::setup_assembly_name()
{
    const auto opt_row = _cliImage.read_assembly(1);
    if (!opt_row)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }

    auto& row = opt_row.value();
    UNWRAP_OR_RET_ERR_ON_FAIL(_assemblyName.name, get_string(row.name));
    UNWRAP_OR_RET_ERR_ON_FAIL(_assemblyName.public_key, get_blob(row.public_key));
    UNWRAP_OR_RET_ERR_ON_FAIL(_assemblyName.culture, get_string(row.locale));
    _assemblyName.hash_algorithm = row.hash_alg_id;
    _assemblyName.hash_len = 0;
    _assemblyName.flags = row.flags;
    _assemblyName.version_major = row.major_version;
    _assemblyName.version_minor = row.minor_version;
    _assemblyName.version_build = row.build_number;
    _assemblyName.version_revision = row.revision_number;
    RET_VOID_OK();
}

RtResultVoid RtModuleDef::setup_generic_params_and_containers()
{
    uint32_t genericParamCount = _cliImage.get_table_row_num(TableType::GenericParam);
    if (genericParamCount == 0)
    {
        RET_VOID_OK();
    }
    _genericParams = _pool.calloc_any<RtGenericParam>(genericParamCount);
    _genericContainers.reserve(genericParamCount);

    for (uint32_t i = 0; i < genericParamCount; ++i)
    {
        uint32_t rid = i + 1;
        RtGenericParam& gp = _genericParams[i];
        auto row_gp = _cliImage.read_generic_param(rid).value();
        gp.gid = RtMetadata::encode_gid_by_rid(*this, rid);
        int a = 5;
        UNWRAP_OR_RET_ERR_ON_FAIL(gp.name, get_string(row_gp.name));
        gp.flags = row_gp.flags;
        gp.index = row_gp.number;

        RtToken owner_token = RtMetadata::decode_type_or_method_def_coded_index(row_gp.owner);
        auto owner_token_encoded_id = owner_token.to_encoded_id();
        RtGenericContainer* gc = nullptr;
        auto it = _genericContainers.find(owner_token_encoded_id);
        if (it != _genericContainers.end())
        {
            gc = it->second;
        }
        else
        {
            gc = _pool.malloc_any_zeroed<RtGenericContainer>();
            gc->owner_gid = RtMetadata::encode_gid_by_token(*this, owner_token.rid);
            gc->is_method = (owner_token.table_type == TableType::Method);
            gc->generic_param_count = 0;
            gc->generic_params = &gp;
            _genericContainers.insert({owner_token_encoded_id, gc});
        }
        gp.owner = gc;
        gc->generic_param_count++;
    }

    _genericParamByValTypeSigs = _pool.calloc_any<RtTypeSig>(genericParamCount);
    _genericParamByRefTypeSigs = _pool.calloc_any<RtTypeSig>(genericParamCount);
    for (uint32_t i = 0; i < genericParamCount; ++i)
    {
        RtGenericParam* gp = _genericParams + i;
        RtElementType eleType = gp->owner->is_method ? RtElementType::MVar : RtElementType::Var;
        _genericParamByValTypeSigs[i] = RtTypeSig::new_byval_with_data(eleType, gp);
        _genericParamByRefTypeSigs[i] = RtTypeSig::new_byref_with_data(eleType, gp);
    }
    RET_VOID_OK();
}

RtResultVoid RtModuleDef::setup_nested_classes()
{
    uint32_t nestedClassCount = _cliImage.get_table_row_num(TableType::NestedClass);

    for (uint32_t i = 0; i < nestedClassCount; ++i)
    {
        uint32_t rid = i + 1;
        auto row = _cliImage.read_nested_class(rid).value();
        uint32_t nestedTypeDefRid = row.nested_class;
        uint32_t enclosingTypeDefRid = row.enclosing_class;
        _nestedTypeDefRid2EnclosingTypeDefRidMap.insert({nestedTypeDefRid, enclosingTypeDefRid});
        ++_enclosingTypeDefRid2StartRidMap[enclosingTypeDefRid].count;
    }
    for (utils::HashMap<uint32_t, EnclosingTypeInfo>::iterator it = _enclosingTypeDefRid2StartRidMap.begin(); it != _enclosingTypeDefRid2StartRidMap.end();
         ++it)
    {
        EnclosingTypeInfo& value = it->second;
        value.nested_type_def_rids = _pool.calloc_any<uint32_t>(value.count);
        value.count = 0;
    }
    for (utils::HashMap<uint32_t, uint32_t>::const_iterator map_it = _nestedTypeDefRid2EnclosingTypeDefRidMap.begin();
         map_it != _nestedTypeDefRid2EnclosingTypeDefRidMap.end(); ++map_it)
    {
        uint32_t nestedTypeDefRid = map_it->first;
        uint32_t enclosingTypeDefRid = map_it->second;
        utils::HashMap<uint32_t, EnclosingTypeInfo>::iterator found_it = _enclosingTypeDefRid2StartRidMap.find(enclosingTypeDefRid);
        assert(found_it != _enclosingTypeDefRid2StartRidMap.end());
        EnclosingTypeInfo& eti = found_it->second;
        eti.nested_type_def_rids[eti.count++] = nestedTypeDefRid;
    }
    RET_VOID_OK();
}

RtResultVoid RtModuleDef::setup_type_fullname_map()
{
    uint32_t typeDefCount = _cliImage.get_table_row_num(TableType::TypeDef);
    for (uint32_t i = 0; i < typeDefCount; ++i)
    {
        uint32_t rid = i + 1;
        if (_nestedTypeDefRid2EnclosingTypeDefRidMap.find(rid) != _nestedTypeDefRid2EnclosingTypeDefRidMap.end())
        {
            // Nested type, skip
            continue;
        }
        auto row = _cliImage.read_type_def(rid).value();
        const char* namespace_name;
        const char* name;
        UNWRAP_OR_RET_ERR_ON_FAIL(namespace_name, get_string(row.type_namespace));
        UNWRAP_OR_RET_ERR_ON_FAIL(name, get_string(row.type_name));
        _typeDefFullName2TypeDefRidMap.insert({utils::FullNameStr(namespace_name, name), rid});
    }

    uint32_t exportedTypeCount = _cliImage.get_table_row_num(TableType::ExportedType);
    for (uint32_t i = 0; i < exportedTypeCount; ++i)
    {
        uint32_t rid = i + 1;
        auto opt_row = _cliImage.read_exported_type(rid);
        auto& row = opt_row.value();
        const char* namespace_name;
        const char* name;
        UNWRAP_OR_RET_ERR_ON_FAIL(namespace_name, get_string(row.type_namespace));
        UNWRAP_OR_RET_ERR_ON_FAIL(name, get_string(row.type_name));
        _typeDefFullName2ExportedTypeRidMap.insert(
            {utils::FullNameStr(namespace_name, name), RtMetadata::decode_implementation_coded_index(row.implementation)});
    }
    RET_VOID_OK();
}

void RtModuleDef::setup_field_offsets()
{
    uint32_t fieldLayoutCount = _cliImage.get_table_row_num(TableType::FieldLayout);
    for (uint32_t rid = 1; rid <= fieldLayoutCount; ++rid)
    {
        auto row = _cliImage.read_field_layout(rid).value();
        _fieldRid2OffsetMap.insert({row.field, row.offset});
    }
}

void RtModuleDef::setup_class_layouts()
{
    uint32_t classLayoutCount = _cliImage.get_table_row_num(TableType::ClassLayout);
    for (uint32_t rid = 1; rid <= classLayoutCount; ++rid)
    {
        auto row = _cliImage.read_class_layout(rid).value();
        ClassLayoutData layoutData(row.packing_size, row.class_size);
        _typeDefRid2ClassLayoutMap.insert({row.parent, layoutData});
    }
}

RtResult<RtAssembly*> RtModuleDef::get_reference_assembly(uint32_t rid)
{
    if (rid >= 1 && rid <= _referenceAssemblyCount)
    {
        ReferenceAssembly& ref_asm = _referenceAssemblies[rid - 1];
        switch (ref_asm.status)
        {
        case AssemblyResolveStatus::Success:
            assert(ref_asm.assembly);
            RET_OK(ref_asm.assembly);
        case AssemblyResolveStatus::NotFoundOrFailed:
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        case AssemblyResolveStatus::NotResolvedYet:
        {
            RowAssemblyRef rowRefAss = _cliImage.read_assembly_ref(rid).value();
            const char* ref_name;
            UNWRAP_OR_RET_ERR_ON_FAIL(ref_name, get_string(rowRefAss.name));
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtAssembly*, ref_assembly, vm::Assembly::load_by_name(ref_name));
            ref_asm.assembly = ref_assembly;
            ref_asm.status = AssemblyResolveStatus::Success;
            RET_OK(ref_assembly);
        }
        }
    }
    RET_ASSERT_ERR(RtErr::BadImageFormat);
}

RtResultVoid RtModuleDef::get_reference_assemblies(utils::Vector<RtAssembly*>& ref_assemblies)
{
    for (uint32_t i = 1; i <= _referenceAssemblyCount; ++i)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtAssembly*, ref_assembly, get_reference_assembly(i));
        ref_assemblies.push_back(ref_assembly);
    }
    RET_VOID_OK();
}

static bool is_valuetype_or_enum(const char* namespace_name, const char* name)
{
    return strcmp(namespace_name, STR_SYSTEM) == 0 && (strcmp(name, STR_VALUETYPE) == 0 || strcmp(name, STR_ENUM) == 0);
}

RtResult<bool> RtModuleDef::is_value_type_or_enum(uint32_t encodedParentType)
{
    if (encodedParentType == 0)
    {
        RET_OK(false);
    }
    RtToken token = RtMetadata::decode_type_def_ref_spec_coded_index(encodedParentType);

    uint32_t namespaceIdx;
    uint32_t nameIdx;
    switch (token.table_type)
    {
    case TableType::TypeRef:
    {
        auto opt_row = _cliImage.read_type_ref(token.rid);
        if (!opt_row)
        {
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        }
        auto& row = opt_row.value();
        namespaceIdx = row.type_namespace;
        nameIdx = row.type_name;
        break;
    }
    case TableType::TypeDef:
    {
        auto opt_row = _cliImage.read_type_def(token.rid);
        if (!opt_row)
        {
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        }
        auto& row = opt_row.value();
        namespaceIdx = row.type_namespace;
        nameIdx = row.type_name;
        break;
    }
    default:
        RET_OK(false);
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const char*, namespace_name, get_string(namespaceIdx));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const char*, name, get_string(nameIdx));
    RET_OK(is_valuetype_or_enum(namespace_name, name));
}

static RtElementType get_element_type_of_primitive_type(const char* name)
{
    switch (name[0])
    {
    case 'B':
    {
        if (strcmp(name, "Boolean") == 0)
        {
            return RtElementType::Boolean;
        }
        if (strcmp(name, "Byte") == 0)
        {
            return RtElementType::U1;
        }
        break;
    }
    case 'C':
    {
        if (strcmp(name, "Char") == 0)
        {
            return RtElementType::Char;
        }
        break;
    }
    case 'D':
    {
        if (strcmp(name, "Double") == 0)
        {
            return RtElementType::R8;
        }
        break;
    }
    case 'I':
    {
        if (strcmp(name, "Int16") == 0)
        {
            return RtElementType::I2;
        }
        else if (strcmp(name, "Int32") == 0)
        {
            return RtElementType::I4;
        }
        else if (strcmp(name, "Int64") == 0)
        {
            return RtElementType::I8;
        }
        else if (strcmp(name, "IntPtr") == 0)
        {
            return RtElementType::I;
        }
        break;
    }
    case 'O':
    {
        if (strcmp(name, "Object") == 0)
        {
            return RtElementType::Object;
        }
        break;
    }
    case 'S':
    {
        if (strcmp(name, "Single") == 0)
        {
            return RtElementType::R4;
        }
        else if (strcmp(name, "String") == 0)
        {
            return RtElementType::String;
        }
        else if (strcmp(name, "SByte") == 0)
        {
            return RtElementType::I1;
        }
        break;
    }
    case 'T':
    {
        if (strcmp(name, "TypedReference") == 0)
        {
            return RtElementType::TypedByRef;
        }
        break;
    }
    case 'U':
    {
        if (strcmp(name, "UInt16") == 0)
        {
            return RtElementType::U2;
        }
        else if (strcmp(name, "UInt32") == 0)
        {
            return RtElementType::U4;
        }
        else if (strcmp(name, "UInt64") == 0)
        {
            return RtElementType::U8;
        }
        else if (strcmp(name, "UIntPtr") == 0)
        {
            return RtElementType::U;
        }
        break;
    }
    case 'V':
    {
        if (strcmp(name, "Void") == 0)
        {
            return RtElementType::Void;
        }
        break;
    }
    }
    return RtElementType::End;
}

RtResult<const RtTypeSig*> RtModuleDef::get_type_def_by_val_typesig(uint32_t rid)
{
    assert(rid >= 1 && rid <= _classCount);
    RtTypeSig* sig = _typeDefByValTypeSigs + (rid - 1);
    if (sig->ele_type == RtElementType::End)
    {
        RowTypeDef row = _cliImage.read_type_def(rid).value();
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, isValueType, is_value_type_or_enum(row.extends));
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const char*, name, get_string(row.type_name));
        isValueType = isValueType && (!is_corlib() || std::strcmp(name, STR_ENUM) != 0);

        sig->ele_type = isValueType ? RtElementType::ValueType : RtElementType::Class;
        sig->flags = 0;
        sig->data.dummy = nullptr;
        sig->data.type_def_gid = RtMetadata::encode_gid_by_rid(*this, rid);

        if (is_corlib())
        {
            RtElementType eleType = get_element_type_of_primitive_type(name);
            if (eleType != RtElementType::End)
            {
                sig->ele_type = eleType;
                sig->data.dummy = nullptr;
            }
        }
    }
    RET_OK(sig);
}

RtResult<const RtTypeSig*> RtModuleDef::get_type_def_by_ref_typesig(uint32_t rid)
{
    assert(rid >= 1 && rid <= _classCount);
    RtTypeSig* sig = _typeDefByRefTypeSigs + (rid - 1);
    if (sig->ele_type == RtElementType::End)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtTypeSig*, val_type_sig, get_type_def_by_val_typesig(rid));
        *sig = *val_type_sig;
        sig->by_ref = 1;
    }
    RET_OK(sig);
}

RtResult<const RtTypeSig*> RtModuleDef::get_generic_param_typesig_by_rid(uint32_t rid, bool by_ref)
{
    uint32_t genericParamCount = _cliImage.get_table_row_num(TableType::GenericParam);
    if (rid >= 1 && rid <= genericParamCount)
    {
        RET_OK((by_ref ? _genericParamByRefTypeSigs : _genericParamByValTypeSigs) + rid - 1);
    }
    RET_ASSERT_ERR(RtErr::BadImageFormat);
}

RtResult<RtClass*> RtModuleDef::get_class_by_name(const char* full_name, bool ignore_case, bool throw_exception_when_not_found)
{
    // Split full_name into namespace and name by the last '.'
    const char* last_dot = std::strrchr(full_name, '.');
    const char* namespace_name = nullptr;
    const char* name = nullptr;
    char* allocated_namespace = nullptr;

    if (last_dot == nullptr)
    {
        namespace_name = "";
        name = full_name;
    }
    else
    {
        size_t ns_len = static_cast<size_t>(last_dot - full_name);
        size_t name_len = std::strlen(last_dot + 1);

        char* allocated_namespace = static_cast<char*>(alloca(ns_len + 1));
        std::strncpy(allocated_namespace, full_name, ns_len);
        allocated_namespace[ns_len] = '\0';
        namespace_name = allocated_namespace;
        name = last_dot + 1;
    }

    return get_class_by_name2(namespace_name, name, ignore_case, throw_exception_when_not_found);
}

RtResult<RtClass*> RtModuleDef::get_class_by_name2(const char* namespace_name, const char* name, bool ignore_case, bool throw_exception_when_not_found)
{
    if (!ignore_case)
    {
        utils::FullNameStr key{namespace_name, name};
        auto it = _typeDefFullName2TypeDefRidMap.find(key);
        if (it != _typeDefFullName2TypeDefRidMap.end())
        {
            return get_class_by_type_def_rid(it->second);
        }
        auto it2 = _typeDefFullName2ExportedTypeRidMap.find(key);
        if (it2 != _typeDefFullName2ExportedTypeRidMap.end())
        {
            return get_exported_class_by_token(it2->second, namespace_name, name, false, throw_exception_when_not_found);
        }
    }
    else
    {
        for (utils::HashMap<utils::FullNameStr, uint32_t, utils::FullNameStrHasher, utils::FullNameStrCompare>::const_iterator it =
                 _typeDefFullName2TypeDefRidMap.begin();
             it != _typeDefFullName2TypeDefRidMap.end(); ++it)
        {
            const utils::FullNameStr& key = it->first;
            uint32_t rid = it->second;
            if (utils::StringUtil::equals_ignorecase(key.namespace_name, namespace_name) && utils::StringUtil::equals_ignorecase(key.name, name))
            {
                return get_class_by_type_def_rid(rid);
            }
        }
        for (utils::HashMap<utils::FullNameStr, RtToken, utils::FullNameStrHasher, utils::FullNameStrCompare>::const_iterator it =
                 _typeDefFullName2ExportedTypeRidMap.begin();
             it != _typeDefFullName2ExportedTypeRidMap.end(); ++it)
        {
            const utils::FullNameStr& key = it->first;
            const RtToken& token = it->second;
            if (utils::StringUtil::equals_ignorecase(key.namespace_name, namespace_name) && utils::StringUtil::equals_ignorecase(key.name, name))
            {
                return get_exported_class_by_token(token, namespace_name, name, true, throw_exception_when_not_found);
            }
        }
    }
    if (throw_exception_when_not_found)
    {
        RET_ERR(RtErr::TypeLoad);
    }
    RET_OK(nullptr);
}

RtResult<uint32_t> RtModuleDef::get_type_def_gid_by_name2(const char* namespace_name, const char* name, bool throw_exception_when_not_found)
{
    utils::FullNameStr key{namespace_name, name};
    auto it = _typeDefFullName2TypeDefRidMap.find(key);
    if (it != _typeDefFullName2TypeDefRidMap.end())
    {
        return RtMetadata::encode_gid_by_rid(*this, it->second);
    }
    auto it2 = _typeDefFullName2ExportedTypeRidMap.find(key);
    if (it2 != _typeDefFullName2ExportedTypeRidMap.end())
    {
        return get_exported_type_def_gid_by_token(it2->second, namespace_name, name, throw_exception_when_not_found);
    }
    if (throw_exception_when_not_found)
    {
        RET_ERR(RtErr::TypeLoad);
    }
    RET_OK(0);
}

RtResult<RtClass*> RtModuleDef::get_class_by_nested_full_name(const char* full_name, bool ignore_case, bool throw_exception_when_not_found)
{
    RtClass* enclosing_class = nullptr;
    const char* remaining_name = full_name;
    while (true)
    {
        const char* nested_pos = std::strchr(remaining_name, '+');
        // const char* enclosing_name = remaining_name;
        // remaining_name = nested_pos ? (nested_pos + 1) : nullptr;
        if (!enclosing_class)
        {
            if (!nested_pos)
            {
                // No nested type
                return get_class_by_name(remaining_name, ignore_case, throw_exception_when_not_found);
            }
            else
            {
                DUP_STR_TO_LOCAL_TEMP_ZERO_END_STR(enclosing_name, remaining_name, (nested_pos - remaining_name));
                UNWRAP_OR_RET_ERR_ON_FAIL(enclosing_class, get_class_by_name(enclosing_name, ignore_case, throw_exception_when_not_found));
            }
        }
        else
        {
            if (!nested_pos)
            {
                // Last nested type
                UNWRAP_OR_RET_ERR_ON_FAIL(enclosing_class, vm::Class::find_nested_class_by_name(enclosing_class, remaining_name, ignore_case));
                if (!enclosing_class && throw_exception_when_not_found)
                {
                    RET_ERR(RtErr::TypeLoad);
                }
                RET_OK(enclosing_class);
            }
            else
            {
                DUP_STR_TO_LOCAL_TEMP_ZERO_END_STR(nested_name, remaining_name, (nested_pos - remaining_name));
                UNWRAP_OR_RET_ERR_ON_FAIL(enclosing_class, vm::Class::find_nested_class_by_name(enclosing_class, nested_name, ignore_case));
            }
        }
        if (!enclosing_class)
        {
            if (throw_exception_when_not_found)
            {
                RET_ERR(RtErr::TypeLoad);
            }
            else
            {
                RET_OK((RtClass*)nullptr);
            }
        }
        remaining_name = nested_pos + 1;
    }
}

RtResult<RtClass*> RtModuleDef::get_class_by_type_def_rid(uint32_t rid)
{
    if (rid >= 1 && rid <= _classCount)
    {
        RtClass*& class_ptr = _classes[rid - 1];
        if (!class_ptr)
        {
            UNWRAP_OR_RET_ERR_ON_FAIL(class_ptr, vm::Class::init_class_of_type_def(this, rid));
        }
        RET_OK(class_ptr);
    }
    RET_ASSERT_ERR(RtErr::BadImageFormat);
}

RtResult<RtClass*> RtModuleDef::get_exported_class_by_token(const RtToken& token, const char* namespace_name, const char* name, bool ignore_case,
                                                            bool throw_exception_when_not_found)
{
    switch (token.table_type)
    {
    case TableType::File:
    {
        RETURN_NOT_IMPLEMENTED_ERROR();
    }
    case TableType::AssemblyRef:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtAssembly*, refAssembly, get_reference_assembly(token.rid));
        return refAssembly->mod->get_class_by_name2(namespace_name, name, ignore_case, throw_exception_when_not_found);
    }
    case TableType::ExportedType:
    {
        auto opt_row = _cliImage.read_exported_type(token.rid);
        if (!opt_row)
        {
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        }
        RowExportedType& row = opt_row.value();
        RtToken implementationToken = RtMetadata::decode_implementation_coded_index(row.implementation);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(
            metadata::RtClass*, enclosing_klass,
            get_exported_class_by_token(implementationToken, namespace_name, name, ignore_case, throw_exception_when_not_found));
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, nested_class, vm::Class::find_nested_class_by_name(enclosing_klass, name, ignore_case));
        if (!nested_class && throw_exception_when_not_found)
        {
            RET_ERR(RtErr::TypeLoad);
        }
        RET_OK(nested_class);
    }
    default:
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    }
    RET_OK(nullptr);
}

RtResult<uint32_t> RtModuleDef::get_exported_type_def_gid_by_token(const RtToken& token, const char* namespace_name, const char* name,
                                                                   bool throw_exception_when_not_found)
{
    switch (token.table_type)
    {
    case TableType::File:
    {
        RETURN_NOT_IMPLEMENTED_ERROR();
    }
    case TableType::AssemblyRef:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtAssembly*, refAssembly, get_reference_assembly(token.rid));
        return refAssembly->mod->get_type_def_gid_by_name2(namespace_name, name, throw_exception_when_not_found);
    }
    case TableType::ExportedType:
    {
        auto opt_row = _cliImage.read_exported_type(token.rid);
        if (!opt_row)
        {
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        }
        RowExportedType& row = opt_row.value();
        RtToken implementationToken = RtMetadata::decode_implementation_coded_index(row.implementation);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, enclosing_klass,
                                                get_exported_class_by_token(implementationToken, namespace_name, name, false, throw_exception_when_not_found));
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, nested_class, vm::Class::find_nested_class_by_name(enclosing_klass, name, false));

        if (nested_class)
        {
            RET_OK(vm::Class::get_type_def_gid(nested_class));
        }
        if (throw_exception_when_not_found)
        {
            RET_ERR(RtErr::TypeLoad);
        }
    }
    default:
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    }
    RET_OK(0);
}

RtClass* RtModuleDef::try_get_created_class_by_type_def_rid(uint32_t rid) const
{
    if (rid >= 1 && rid <= _classCount)
    {
        return _classes[rid - 1];
    }
    return nullptr;
}

RtResult<RtClass*> RtModuleDef::get_class_by_type_ref_rid(uint32_t rid)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint32_t, type_def_gid, get_type_def_gid_by_type_ref_rid(rid));
    return vm::Class::get_class_by_type_def_gid(type_def_gid);
}

RtResult<uint32_t> RtModuleDef::get_type_def_gid_by_type_ref_rid(uint32_t rid)
{
    auto opt_row = _cliImage.read_type_ref(rid);
    if (!opt_row)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    RowTypeRef& row = opt_row.value();
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const char*, namespaceName, get_string(row.type_namespace));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const char*, name, get_string(row.type_name));
    RtToken resolutionScopeToken = RtMetadata::decode_resolution_scope_coded_index(row.resolution_scope);
    switch (resolutionScopeToken.table_type)
    {
    case TableType::Module:
    {
        return get_type_def_gid_by_name2(namespaceName, name, true);
    }
    case TableType::ModuleRef:
    {
        RETURN_NOT_IMPLEMENTED_ERROR();
    }
    case TableType::AssemblyRef:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtAssembly*, refAssembly, get_reference_assembly(resolutionScopeToken.rid));
        return refAssembly->mod->get_type_def_gid_by_name2(namespaceName, name, true);
    }
    case TableType::TypeRef:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtClass*, refClass, get_class_by_type_ref_rid(resolutionScopeToken.rid));
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtClass*, nestedClass, vm::Class::find_nested_class_by_name(refClass, name, false));
        RET_OK((nestedClass ? vm::Class::get_type_def_gid(nestedClass) : 0));
    }
    default:
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    }
}

RtResult<const RtTypeSig*> RtModuleDef::get_type_spec_typesig_by_rid(uint32_t rid, const RtGenericContainerContext& gcc, const RtGenericContext* gc)
{
    auto opt_row = _cliImage.read_type_spec(rid);
    if (!opt_row)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    RowTypeSpec& row = opt_row.value();
    auto result = get_decoded_blob_reader(row.signature);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL2(utils::BinaryReader, reader, result);
    return read_typesig(reader, gcc, gc);
}

RtResult<const RtTypeSig*> RtModuleDef::get_typesig_by_type_def_ref_spec_token(const RtToken& token, const RtGenericContainerContext& gcc,
                                                                               const RtGenericContext* gc)
{
    switch (token.table_type)
    {
    case TableType::TypeDef:
        return get_type_def_by_val_typesig(token.rid);
    case TableType::TypeRef:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint32_t, type_def_gid, get_type_def_gid_by_type_ref_rid(token.rid));
        uint32_t module_id = RtMetadata::decode_module_id_from_gid(type_def_gid);
        RtModuleDef* cls_module = get_module_by_id(module_id);
        RET_OK((cls_module)->get_type_def_by_val_typesig(RtMetadata::decode_rid_from_gid(type_def_gid)));
    }
    case TableType::TypeSpec:
        return get_type_spec_typesig_by_rid(token.rid, gcc, gc);
    default:
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
}

RtResult<RtClass*> RtModuleDef::get_class_by_type_spec_rid(uint32_t rid, const RtGenericContainerContext& gcc, const RtGenericContext* gc)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtTypeSig*, typeSig, get_type_spec_typesig_by_rid(rid, gcc, gc));
    return vm::Class::get_class_from_typesig(typeSig);
}

RtResult<RtClass*> RtModuleDef::get_class_by_type_def_ref_spec_token(const RtToken& token, const RtGenericContainerContext& gcc, const RtGenericContext* gc)
{
    switch (token.table_type)
    {
    case TableType::TypeDef:
        return get_class_by_type_def_rid(token.rid);
    case TableType::TypeRef:
        return get_class_by_type_ref_rid(token.rid);
    case TableType::TypeSpec:
        return get_class_by_type_spec_rid(token.rid, gcc, gc);
    default:
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
}

RtResultVoid RtModuleDef::get_nested_type_def_rid(EncodedTokenId type_def_token, utils::Span<uint32_t>& outNestedTypeDefRids)
{
    auto it = _enclosingTypeDefRid2StartRidMap.find(RtToken::decode_rid(type_def_token));
    if (it != _enclosingTypeDefRid2StartRidMap.end())
    {
        EnclosingTypeInfo& eti = it->second;
        outNestedTypeDefRids = utils::Span<uint32_t>(eti.nested_type_def_rids, eti.count);
    }
    RET_VOID_OK();
}

RtResultVoid RtModuleDef::get_nested_classs(EncodedTokenId enclosing_type_def_token, utils::Vector<RtClass*>& outNestedClasses)
{
    auto it = _enclosingTypeDefRid2StartRidMap.find(RtToken::decode_rid(enclosing_type_def_token));
    if (it != _enclosingTypeDefRid2StartRidMap.end())
    {
        EnclosingTypeInfo& eti = it->second;
        uint32_t count = it->second.count;
        for (uint32_t i = 0; i < count; ++i)
        {
            uint32_t nestedClassRid = eti.nested_type_def_rids[i];
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtClass*, nestedClass, get_class_by_type_def_rid(nestedClassRid));
            outNestedClasses.push_back(nestedClass);
        }
    }
    RET_VOID_OK();
}

RtResult<const RtTypeSig*> RtModuleDef::read_typesig(utils::BinaryReader& reader, const RtGenericContainerContext& gcc, const RtGenericContext* gc)
{
    RtTypeSig tempTypeSig = {};
    RET_ERR_ON_FAIL(read_typesig_impl(reader, gcc, gc, tempTypeSig));
    return metadata::MetadataCache::get_pooled_typesig(tempTypeSig);
}

RtResult<const uint32_t*> RtModuleDef::read_u32_array(utils::BinaryReader& reader, size_t num)
{
    uint32_t* arr = _pool.calloc_any<uint32_t>(num);
    for (size_t i = 0; i < num; ++i)
    {
        if (!reader.try_read_compressed_uint32(arr[i]))
        {
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        }
    }
    RET_OK(arr);
}

RtResultVoid RtModuleDef::read_typesig_impl(utils::BinaryReader& reader, const RtGenericContainerContext& gcc, const RtGenericContext* gc, RtTypeSig& result)
{
    while (true)
    {
        uint8_t byteEleType = 0;
        if (!reader.try_read_byte(byteEleType))
        {
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        }
        RtElementType ele_type = static_cast<RtElementType>(byteEleType);
        result.ele_type = ele_type;

        switch (ele_type)
        {
        // Primitive types with no additional data
        case RtElementType::Void:
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
        case RtElementType::Object:
        case RtElementType::String:
        case RtElementType::TypedByRef:
            RET_VOID_OK();
        case RtElementType::SZArray:
        case RtElementType::Ptr:
        {
            UNWRAP_OR_RET_ERR_ON_FAIL(result.data.element_type, read_typesig(reader, gcc, gc));
            RET_VOID_OK();
        }

        case RtElementType::FnPtr:
        {
            auto ret_result = read_method_sig(reader, gcc, gc);
            RET_ERR_ON_FAIL(ret_result);
            result.data.method_sig = new (_pool.malloc_any_zeroed<RtMethodSig>()) RtMethodSig{std::move(ret_result.unwrap())};
            RET_VOID_OK();
        }
        case RtElementType::ByRef:
            result.by_ref = 1;
            // Continue loop to read the referenced type
            continue;

        case RtElementType::ValueType:
        case RtElementType::Class:
        {
            uint32_t coded_index = 0;
            if (!reader.try_read_compressed_uint32(coded_index))
            {
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            }
            RtToken token = RtMetadata::decode_type_def_ref_spec_coded_index(coded_index);

            switch (token.table_type)
            {
            case TableType::TypeDef:
                result.data.type_def_gid = RtMetadata::encode_gid_by_rid(*this, token.rid);
                break;
            case TableType::TypeRef:
            {
                UNWRAP_OR_RET_ERR_ON_FAIL(result.data.type_def_gid, get_type_def_gid_by_type_ref_rid(token.rid));
                break;
            }
            case TableType::TypeSpec:
            {
                auto row_opt = _cliImage.read_type_spec(token.rid);
                if (!row_opt.has_value())
                {
                    RET_ASSERT_ERR(RtErr::BadImageFormat);
                }
                auto decoded_blob_result = get_decoded_blob_reader(row_opt->signature);
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL2(utils::BinaryReader, spec_reader, decoded_blob_result);
                return read_typesig_impl(spec_reader, gcc, gc, result);
            }
            default:
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            }
            RET_VOID_OK();
        }
        case RtElementType::Array:
        {
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtTypeSig*, elementTypeSig, read_typesig(reader, gcc, gc));
            uint32_t rank;
            if (!reader.try_read_compressed_uint32(rank))
            {
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            }
            uint32_t numSizes;
            if (!reader.try_read_compressed_uint32(numSizes))
            {
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            }
            const uint32_t* sizes = nullptr;
            if (numSizes > 0)
            {
                UNWRAP_OR_RET_ERR_ON_FAIL(sizes, read_u32_array(reader, numSizes));
            }
            uint32_t numLoBounds;
            if (!reader.try_read_compressed_uint32(numLoBounds))
            {
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            }
            const uint32_t* loBounds = nullptr;
            if (numLoBounds > 0)
            {
                UNWRAP_OR_RET_ERR_ON_FAIL(loBounds, read_u32_array(reader, numLoBounds));
            }
            if (numSizes == 0 && numLoBounds == 0)
            {
                UNWRAP_OR_RET_ERR_ON_FAIL(result.data.array_type, metadata::MetadataCache::get_pooled_array_type(elementTypeSig, static_cast<uint8_t>(rank)));
            }
            else
            {
                RtArrayType* newArrType = _pool.malloc_any_zeroed<RtArrayType>();
                newArrType->ele_type = elementTypeSig;
                newArrType->rank = static_cast<uint8_t>(rank);
                newArrType->num_sizes = static_cast<uint8_t>(numSizes);
                newArrType->sizes = sizes;
                newArrType->num_bounds = static_cast<uint8_t>(numLoBounds);
                newArrType->bounds = loBounds;
                result.data.array_type = newArrType;
            }
            RET_VOID_OK();
        }

        case RtElementType::GenericInst:
        {
            RtTypeSig baseTypeSig{};
            RET_ERR_ON_FAIL(read_typesig_impl(reader, gcc, gc, baseTypeSig));

            if (baseTypeSig.ele_type != RtElementType::Class && baseTypeSig.ele_type != RtElementType::ValueType)
            {
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            }
            uint32_t genericArgCount;
            if (!reader.try_read_compressed_uint32(genericArgCount) || genericArgCount > RT_MAX_GENERIC_PARAM_COUNT)
            {
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            }

            const RtTypeSig* tempGenericArgs[RT_MAX_GENERIC_PARAM_COUNT];
            for (uint32_t i = 0; i < genericArgCount; ++i)
            {
                UNWRAP_OR_RET_ERR_ON_FAIL(tempGenericArgs[i], read_typesig(reader, gcc, gc));
            }
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtGenericInst*, gi,
                                                    metadata::MetadataCache::get_pooled_generic_inst(tempGenericArgs, static_cast<uint8_t>(genericArgCount)));
            result.data.generic_class = metadata::MetadataCache::get_pooled_generic_class(baseTypeSig.data.type_def_gid, gi);
            RET_VOID_OK();
        }

        case RtElementType::Var:
        case RtElementType::MVar:
        {
            uint32_t num = 0;
            if (!reader.try_read_compressed_uint32(num))
            {
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            }
            bool is_mvar = (ele_type == RtElementType::MVar);

            // Check if we have a generic context with instantiation
            if (gc != nullptr)
            {
                const RtGenericInst* used_gi = is_mvar ? gc->method_inst : gc->class_inst;
                if (used_gi != nullptr)
                {
                    if (num >= used_gi->generic_arg_count)
                    {
                        RET_ASSERT_ERR(RtErr::BadImageFormat);
                    }
                    const RtTypeSig* arg = used_gi->generic_args[num];
                    // preserve flags like ByRef
                    result.ele_type = arg->ele_type;
                    result.data = arg->data;
                    RET_VOID_OK();
                }
            }

            // Use container's generic params
            const RtGenericContainer* container = is_mvar ? gcc.method : gcc.klass;
            if (container != nullptr)
            {
                if (num >= container->generic_param_count)
                {
                    RET_ASSERT_ERR(RtErr::BadImageFormat);
                }
                result.data.generic_param = &container->generic_params[num];
            }
            else
            {
                result.data.generic_param = metadata::MetadataCache::get_unspecific_generic_param(num, is_mvar);
            }
            RET_VOID_OK();
        }

        case RtElementType::CModReqd:
        {
            result.num_mods += 1;
            uint32_t encoded_index = 0;
            if (!reader.try_read_compressed_uint32(encoded_index))
            {
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            }
            RtToken token = RtMetadata::decode_type_def_ref_spec_coded_index(encoded_index);
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, mod_class, get_class_by_type_def_ref_spec_token(token, gcc, gc));
            if (std::strcmp(mod_class->namespaze, STR_SYSTEM_RUNTIME_INTEROPSERVICES) == 0)
            {
                if (std::strcmp(mod_class->name, STR_INATTRIBUTE) == 0)
                {
                    result.field_or_param_attrs |= (uint8_t)RtParamAttribute::In;
                }
                else if (std::strcmp(mod_class->name, STR_OUTATTRIBUTE) == 0)
                {
                    result.field_or_param_attrs |= (uint8_t)RtParamAttribute::Out;
                }
                else if (std::strcmp(mod_class->name, STR_OPTIONALATTRIBUTE) == 0)
                {
                    result.field_or_param_attrs |= (uint8_t)RtParamAttribute::Optional;
                }
            }
            continue;
        }
        case RtElementType::CModOpt:
        {
            result.num_mods += 1;
            uint32_t encoded_index = 0;
            if (!reader.try_read_compressed_uint32(encoded_index))
            {
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            }
            continue;
        }

        case RtElementType::Pinned:
            result.pinned = 1;
            // Continue to read the pinned type
            continue;

        default:
            // Unknown element type
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        }
    }

    RET_VOID_OK();
}

RtResultVoid RtModuleDef::read_type_modifier(utils::BinaryReader& reader, bool optional, const RtGenericContainerContext& gcc, const RtGenericContext* gc,
                                             utils::Vector<metadata::RtClass*>& out_type_mods)
{
    while (true)
    {
        uint8_t byteEleType = 0;
        if (!reader.try_read_byte(byteEleType))
        {
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        }
        RtElementType ele_type = static_cast<RtElementType>(byteEleType);

        switch (ele_type)
        {
        case RtElementType::CModReqd:
        case RtElementType::CModOpt:
        {
            uint32_t encoded_index = 0;
            if (!reader.try_read_compressed_uint32(encoded_index))
            {
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            }
            if (optional != (ele_type == RtElementType::CModReqd))
            {
                continue;
            }
            RtToken token = RtMetadata::decode_type_def_ref_spec_coded_index(encoded_index);
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, mod_class, get_class_by_type_def_ref_spec_token(token, gcc, gc));
            out_type_mods.push_back(mod_class);
            continue;
        }
        default:
            RET_VOID_OK();
        }
    }
}

RtResultVoid RtModuleDef::read_member_modifier(utils::BinaryReader& reader, bool optional, const RtGenericContainerContext& gcc, const RtGenericContext* gc,
                                               utils::Vector<metadata::RtClass*>& outMemberMods)
{
    uint8_t byteType;
    if (!reader.try_read_byte(byteType))
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    return read_type_modifier(reader, optional, gcc, gc, outMemberMods);
}

RtResultVoid RtModuleDef::read_property_type_modifier(utils::BinaryReader& reader, bool optional, const RtGenericContainerContext& gcc,
                                                      const RtGenericContext* gc, utils::Vector<metadata::RtClass*>& outPropMods)
{
    return read_member_modifier(reader, optional, gcc, gc, outPropMods);
}

RtResultVoid RtModuleDef::read_parameter_modifier(utils::BinaryReader& reader, int32_t index, bool optional, const RtGenericContainerContext& gcc,
                                                  const RtGenericContext* gc, utils::Vector<metadata::RtClass*>& outParamMods)
{
    uint8_t byteType;
    if (!reader.try_read_byte(byteType))
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    RtSigType sigType = RtMetadata::decode_sig_type(byteType);
    if ((uint8_t)sigType >= (uint8_t)RtSigType::Field)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    uint32_t genericParamCount;
    if (((uint8_t)sigType & (uint8_t)RtSigType::GenericInst) != 0)
    {
        if (!reader.try_read_compressed_uint32(genericParamCount) || genericParamCount > RT_MAX_GENERIC_PARAM_COUNT)
        {
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        }
    }
    else
    {
        genericParamCount = 0;
    }

    uint32_t paramCount;
    if (!reader.try_read_compressed_uint32(paramCount) || paramCount > RT_MAX_PARAM_COUNT)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    if (index == -1)
    {
        return read_type_modifier(reader, optional, gcc, gc, outParamMods);
    }
    if ((uint32_t)index >= paramCount)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    for (int32_t i = 0; i < index; ++i)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtTypeSig*, _skip, read_typesig(reader, gcc, gc));
    }
    return read_type_modifier(reader, optional, gcc, gc, outParamMods);
}

RtResult<const RtTypeSig*> RtModuleDef::read_field_sig(utils::BinaryReader& reader, const RtGenericContainerContext& gcc, const RtGenericContext* gc)
{
    uint8_t byteType;
    if (!reader.try_read_byte(byteType))
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    RtSigType sigType = RtMetadata::decode_sig_type(byteType);
    if (sigType != RtSigType::Field)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    return read_typesig(reader, gcc, gc);
}

RtResult<RtPropertySig> RtModuleDef::read_property_sig(utils::BinaryReader& reader, const RtGenericContainerContext& gcc, const RtGenericContext* gc)
{
    uint8_t byteType;
    if (!reader.try_read_byte(byteType))
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    RtSigType sigType = RtMetadata::decode_sig_type(byteType);
    if (sigType != RtSigType::Property)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }

    uint32_t paramCount;
    if (!reader.try_read_compressed_uint32(paramCount) || paramCount > RT_MAX_PARAM_COUNT)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtTypeSig*, returnTypeSig, read_typesig(reader, gcc, gc));

    RtPropertySig propSig;

    propSig.params.resize(paramCount);
    for (uint32_t i = 0; i < paramCount; i++)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(propSig.params[i], read_typesig(reader, gcc, gc));
    }
    propSig.type_sig = returnTypeSig;
    RET_OK(propSig);
}

RtResult<RtMethodSig> RtModuleDef::read_method_sig(utils::BinaryReader& reader, const RtGenericContainerContext& gcc, const RtGenericContext* gc)
{
    uint8_t byteType;
    if (!reader.try_read_byte(byteType))
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    RtSigType sigType = RtMetadata::decode_sig_type(byteType);
    if (sigType >= RtSigType::Field)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    return read_method_sig_skip_prologue(byteType, reader, gcc, gc);
}

RtResult<RtMethodSig> RtModuleDef::read_method_sig_skip_prologue(uint8_t sigType, utils::BinaryReader& reader, const RtGenericContainerContext& gcc,
                                                                 const RtGenericContext* gc)
{
    uint32_t genericParamCount;
    if (((uint8_t)sigType & (uint8_t)RtSigType::GenericInst) != 0)
    {
        if (!reader.try_read_compressed_uint32(genericParamCount) || genericParamCount > RT_MAX_GENERIC_PARAM_COUNT)
        {
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        }
    }
    else
    {
        genericParamCount = 0;
    }
    uint32_t paramCount;
    if (!reader.try_read_compressed_uint32(paramCount) || paramCount > RT_MAX_PARAM_COUNT)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtTypeSig*, returnTypeSig, read_typesig(reader, gcc, gc));

    RtMethodSig methodSig;

    methodSig.params.resize(paramCount);
    for (uint32_t i = 0; i < paramCount; i++)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(methodSig.params[i], read_typesig(reader, gcc, gc));
    }
    methodSig.flags = sigType;
    methodSig.generic_param_count = static_cast<uint8_t>(genericParamCount);
    methodSig.return_type = returnTypeSig;
    RET_OK(methodSig);
}

RtResult<RtMethodSig> RtModuleDef::read_stadalone_method_sig(EncodedTokenId standaloneSigToken, const RtGenericContainerContext& gcc,
                                                             const RtGenericContext* gc)
{
    auto optStandaloneSigRow = _cliImage.read_stand_alone_sig(RtToken::decode_rid(standaloneSigToken));
    if (!optStandaloneSigRow)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    RowStandaloneSig& row = optStandaloneSigRow.value();
    auto result = get_decoded_blob_reader(row.signature);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL2(utils::BinaryReader, reader, result);
    return read_method_sig(reader, gcc, gc);
}

RtResult<const RtGenericInst*> RtModuleDef::read_method_spec_generic_inst(utils::BinaryReader& reader, const RtGenericContainerContext& gcc,
                                                                          const RtGenericContext* gc)
{
    uint8_t byteType;
    if (!reader.try_read_byte(byteType))
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    RtSigType sigType = RtMetadata::decode_sig_type(byteType);
    if (sigType != RtSigType::MethodSpec)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    uint32_t genericArgCount;
    if (!reader.try_read_compressed_uint32(genericArgCount) || genericArgCount > RT_MAX_GENERIC_PARAM_COUNT)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    const RtTypeSig* tempGenericArgs[RT_MAX_GENERIC_PARAM_COUNT];
    for (uint32_t i = 0; i < genericArgCount; ++i)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(tempGenericArgs[i], read_typesig(reader, gcc, gc));
    }
    return metadata::MetadataCache::get_pooled_generic_inst(tempGenericArgs, static_cast<uint8_t>(genericArgCount));
}

RtResult<const RtTypeSig*> RtModuleDef::read_typesig_from_member_parent(const RtToken& token, const RtGenericContainerContext& gcc, const RtGenericContext* gc)
{
    switch (token.table_type)
    {
    case TableType::TypeDef:
    case TableType::TypeRef:
    case TableType::TypeSpec:
        return get_typesig_by_type_def_ref_spec_token(token, gcc, gc);
    default:
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
}

RtResultVoid RtModuleDef::read_local_var_sig(EncodedTokenId localVarSigToken, const RtGenericContainerContext& gcc, const RtGenericContext* gc,
                                             utils::Vector<const RtTypeSig*>& outLocalVarTypeSigs)
{
    auto optStandaloneSigRow = _cliImage.read_stand_alone_sig(RtToken::decode_rid(localVarSigToken));
    if (!optStandaloneSigRow)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    RowStandaloneSig& row = optStandaloneSigRow.value();
    auto result = get_decoded_blob_reader(row.signature);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL2(utils::BinaryReader, reader, result);
    uint8_t byteType;
    if (!reader.try_read_byte(byteType))
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    RtSigType sigType = RtMetadata::decode_sig_type(byteType);
    if (sigType != RtSigType::LocalVar)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    uint32_t localVarCount;
    if (!reader.try_read_compressed_uint32(localVarCount) || localVarCount > RT_MAX_LOCAL_VAR_COUNT)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    outLocalVarTypeSigs.reserve(localVarCount);
    for (uint32_t i = 0; i < localVarCount; ++i)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtTypeSig*, localVarTypeSig, read_typesig(reader, gcc, gc));
        outLocalVarTypeSigs.push_back(localVarTypeSig);
    }
    RET_VOID_OK();
}

RtResult<std::optional<RowImplMap>> RtModuleDef::read_method_impl_map(EncodedTokenId methodDefToken)
{
    uint32_t rid = RtToken::decode_rid(methodDefToken);
    uint32_t encodedIndex = RtMetadata::encode_member_forwarded(TableType::Method, rid);
    auto optRid = _cliImage.find_row_of_owner(TableType::ImplMap, 1, encodedIndex);
    if (!optRid)
    {
        return RtResult<std::optional<RowImplMap>>(std::nullopt);
    }
    auto optRow = _cliImage.read_impl_map(optRid.value());
    if (!optRow)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    RET_OK(optRow);
}

RtResult<std::optional<RtMethodBody>> RtModuleDef::read_method_body(EncodedTokenId methodDefToken)
{
    auto optMethodRow = _cliImage.read_method(RtToken::decode_rid(methodDefToken));
    if (!optMethodRow)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    RowMethod& methodRow = optMethodRow.value();
    if (methodRow.rva == 0)
    {
        return RtResult<std::optional<RtMethodBody>>(std::nullopt);
    }
    auto result = read_method_body_from_rva(methodRow.rva);
    if (result.is_err())
    {
        RET_ERR(result.unwrap_err());
    }
    RET_OK(std::optional<RtMethodBody>(result.unwrap()));
}

RtResult<RtMethodBody> RtModuleDef::read_method_body_from_rva(uint32_t rva)
{
    assert(rva != 0);
    auto optBodyDataOffset = _cliImage.rva_to_image_offset(rva);
    if (!optBodyDataOffset)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    uint32_t bodyDataOffset = optBodyDataOffset.value();
    const uint8_t* bodyDataPtr = _cliImage.get_image_data_at(bodyDataOffset);
    utils::BinaryReader reader(bodyDataPtr, _cliImage.get_image_length() - bodyDataOffset);

    uint8_t bodyFlags;
    if (!reader.try_peek_byte(bodyFlags))
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    RtILMethodFormat smallFat = (RtILMethodFormat)(bodyFlags & 0x3);
    RtMethodBody body;
    if (smallFat == RtILMethodFormat::Tiny)
    {
        body.max_stack = 8;
        body.code_size = (bodyFlags >> 2);
        body.code = reader.get_current_ptr() + 1;
        body.flags = (uint8_t)smallFat;
        body.local_var_sig_token = 0;
    }
    else if (smallFat == RtILMethodFormat::Fat)
    {
        if (!reader.try_align_up_position(4))
        {
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        }
        auto fatHeader = (const RtILMethodFatHeader*)reader.get_current_ptr();
        uint16_t flags = fatHeader->flags;
        uint16_t headerSize = fatHeader->size;
        if (headerSize != 3)
        {
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        }
        if (!reader.try_advance(sizeof(RtILMethodFatHeader)))
        {
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        }
        const uint8_t* codes = reader.get_current_ptr();
        if (!reader.try_advance(fatHeader->code_size))
        {
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        }
        if ((flags & (uint16_t)RtILMethodFormat::MoreSects) != 0)
        {
            if (!reader.try_align_up_position(4))
            {
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            }
            while (true)
            {
                uint8_t sectKind;
                if (!reader.try_peek_byte(sectKind))
                {
                    RET_ASSERT_ERR(RtErr::BadImageFormat);
                }
                if ((sectKind & (uint8_t)RtILSection::EHTable) == 0)
                {
                    RETURN_NOT_IMPLEMENTED_ERROR();
                }
                if ((sectKind & (uint8_t)RtILSection::FatFormat) == 0)
                {
                    auto ehHeader = (const RtILEHSectionHeaderSmall*)reader.get_current_ptr();
                    if (ehHeader->data_size % 12 != 4)
                    {
                        RET_ASSERT_ERR(RtErr::BadImageFormat);
                    }
                    if (!reader.try_advance(ehHeader->data_size))
                    {
                        RET_ASSERT_ERR(RtErr::BadImageFormat);
                    }

                    size_t clauseCount = (ehHeader->data_size - 4) / 12;
                    body.exception_clauses.reserve(clauseCount);
                    for (size_t i = 0; i < clauseCount; ++i)
                    {
                        const RtILEHSmall& clause = ehHeader->clauses[i];
                        RtExceptionClause exClause;
                        exClause.flags = (RtILExceptionClauseType)clause.flags;
                        exClause.try_offset = clause.try_offset;
                        exClause.try_length = clause.try_length;
                        exClause.handler_offset = clause.handler_offset;
                        exClause.handler_length = clause.handler_length;
                        exClause.class_token_or_filter_offset = clause.class_token_or_filter_offset;
                        body.exception_clauses.push_back(exClause);
                    }
                }
                else
                {
                    auto ehHeader = (const RtILEHSectionHeaderFat*)reader.get_current_ptr();
                    if (ehHeader->data_size % 24 != 4)
                    {
                        RET_ASSERT_ERR(RtErr::BadImageFormat);
                    }
                    if (!reader.try_advance(ehHeader->data_size))
                    {
                        RET_ASSERT_ERR(RtErr::BadImageFormat);
                    }
                    size_t clauseCount = (ehHeader->data_size - 4) / 24;
                    body.exception_clauses.reserve(clauseCount);
                    for (size_t i = 0; i < clauseCount; ++i)
                    {
                        const RtILEHFat& clause = ehHeader->clauses[i];
                        RtExceptionClause exClause;
                        exClause.flags = (RtILExceptionClauseType)clause.flags;
                        exClause.try_offset = clause.try_offset;
                        exClause.try_length = clause.try_length;
                        exClause.handler_offset = clause.handler_offset;
                        exClause.handler_length = clause.handler_length;
                        exClause.class_token_or_filter_offset = clause.class_token_or_filter_offset;
                        body.exception_clauses.push_back(exClause);
                    }
                }
                if ((sectKind & (uint8_t)RtILSection::MoreSects) == 0)
                {
                    break;
                }
            }
        }
        body.flags = flags;
        body.max_stack = fatHeader->max_stack;
        body.code_size = fatHeader->code_size;
        body.code = codes;
        body.local_var_sig_token = fatHeader->local_var_sig_tok;
    }
    else
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    RET_OK(body);
}

RtResult<const RtGenericContainer*> RtModuleDef::get_generic_container(EncodedTokenId ownerToken)
{
    auto it = _genericContainers.find(ownerToken);
    if (it == _genericContainers.end())
    {
        RET_OK((const RtGenericContainer*)nullptr);
    }
    const RtGenericContainer* gc = it->second;
    if (!gc->inited)
    {
        RtGenericContainerContext gcc;
        if (gc->is_method)
        {
            uint32_t methodDefRid = RtMetadata::decode_rid_from_gid(gc->owner_gid);
            auto opt_typeDefRid = _cliImage.find_last_row_less_equal(TableType::TypeDef, 5, methodDefRid);
            if (!opt_typeDefRid)
            {
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            }
            EncodedTokenId typeDefToken = RtToken::encode(TableType::TypeDef, opt_typeDefRid.value());
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtGenericContainer*, classOwner, get_generic_container(typeDefToken));
            gcc.klass = classOwner;
            gcc.method = gc;
        }
        else
        {
            gcc.klass = gc;
            gcc.method = nullptr;
        }
        for (uint8_t i = 0; i < gc->generic_param_count; ++i)
        {
            RtGenericParam* gpInfo = const_cast<RtGenericParam*>(gc->generic_params + i);
            uint32_t genericParamRid = RtMetadata::decode_rid_from_gid(gpInfo->gid);
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtGenericParamConstraint, constraintInfo, read_generic_param_constraints(genericParamRid, gcc));
            gpInfo->constraint_type_sigs = constraintInfo.constraints;
            gpInfo->constraint_type_sig_count = static_cast<uint8_t>(constraintInfo.count);
        }
        const_cast<RtGenericContainer*>(gc)->inited = true;
    }
    RET_OK(gc);
}

RtResult<RtCustomAttributeRidRange> RtModuleDef::get_custom_attribute_rid_range(EncodedTokenId parentToken)
{
    RtToken token = RtToken::decode(parentToken);
    uint32_t encodedIndex = RtMetadata::encode_has_customattribute_coded_index(token.table_type, token.rid);
    auto optRange = _cliImage.find_row_range_of_owner_at_sorted_table(TableType::CustomAttribute, 0, encodedIndex);
    if (!optRange)
    {
        RET_OK(RtCustomAttributeRidRange{});
    }
    auto& range = optRange.value();
    auto ret = RtCustomAttributeRidRange{range.ridBegin, range.ridEnd - range.ridBegin};
    RET_OK(ret);
}

RtResult<RtCustomAttributeRawData> RtModuleDef::get_custom_attribute_raw_data(uint32_t customAttributeRid)
{
    auto optRow = _cliImage.read_custom_attribute(customAttributeRid);
    if (!optRow)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    RowCustomAttribute& row = optRow.value();
    RtToken constructorToken = RtMetadata::decode_custom_attribute_type_coded_index(row.type_);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtMethodInfo*, constructorMethod,
                                            get_method_by_token(constructorToken, RtGenericContainerContext{}, nullptr));
    auto ret = RtCustomAttributeRawData{constructorMethod, row.value};
    RET_OK(ret);
}

RtResult<RtModuleDef::RtGenericParamConstraint> RtModuleDef::read_generic_param_constraints(uint32_t genericParamRid, const RtGenericContainerContext& gcc)
{
    auto opt_range = _cliImage.find_row_range_of_owner_at_sorted_table(TableType::GenericParamConstraint, 0, genericParamRid);
    if (!opt_range)
    {
        RET_OK(RtGenericParamConstraint{});
    }
    auto& range = opt_range.value();
    uint32_t ridStart = range.ridBegin;
    uint32_t count = range.ridEnd - ridStart;
    if (count > RT_MAX_GENERIC_PARAM_CONSTRAINT_COUNT)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    const RtTypeSig** constraints = _pool.calloc_any<const RtTypeSig*>(count);
    for (uint32_t i = 0; i < count; ++i)
    {
        uint32_t constraintRid = ridStart + i;
        auto opt_row = _cliImage.read_generic_param_constraint(constraintRid);
        if (!opt_row)
        {
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        }
        RowGenericParamConstraint& row = opt_row.value();
        RtToken token = RtMetadata::decode_type_def_ref_spec_coded_index(row.constraint);
        UNWRAP_OR_RET_ERR_ON_FAIL(constraints[i], get_typesig_by_type_def_ref_spec_token(token, gcc, nullptr));
    }
    return RtGenericParamConstraint{constraints, count};
}

RtResult<const RtMethodInfo*> RtModuleDef::get_method_by_rid(uint32_t rid)
{
    if (rid == 0 || rid > _methodCount)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    const RtMethodInfo* method = _methods[rid - 1];
    if (method)
    {
        RET_OK(method);
    }

    auto opt_typeDefRid = _cliImage.find_last_row_less_equal(TableType::TypeDef, 5, rid);
    if (!opt_typeDefRid)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    uint32_t typeDefRid = opt_typeDefRid.value();
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtClass*, klass, get_class_by_type_def_rid(typeDefRid));
    RET_ERR_ON_FAIL(vm::Class::initialize_methods(klass));
    assert(klass->method_count > 0);
    uint32_t firstMethodRid = RtToken::decode_rid(klass->methods[0]->token);
    assert(rid >= firstMethodRid && rid < firstMethodRid + klass->method_count);
    method = klass->methods[rid - firstMethodRid];
    assert(method != nullptr);
    assert(RtToken::decode_rid(method->token) == rid);
    RET_OK(method);
}

RtResult<const RtMethodInfo*> RtModuleDef::get_method_by_member_ref_rid(uint32_t rid, const RtGenericContainerContext& gcc, const RtGenericContext* gc)
{
    auto result = get_member_ref_by_rid(rid, gcc, gc);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL2(RtRuntimeHandle, memberHandle, result);
    if (memberHandle.type != RtRuntimeHandleType::Method)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    RET_OK(memberHandle.method);
}

RtResult<const RtMethodInfo*> RtModuleDef::get_method_by_method_spec_rid(uint32_t rid, const RtGenericContainerContext& gcc, const RtGenericContext* gc)
{
    auto opt_row = _cliImage.read_method_spec(rid);
    if (!opt_row)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    RowMethodSpec& row = opt_row.value();
    RtToken methodToken = RtMetadata::decode_method_def_or_ref_coded_index(row.method);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtMethodInfo*, baseMethod, get_method_by_token(methodToken, gcc, gc));
    auto result = get_decoded_blob_reader(row.instantiation);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL2(utils::BinaryReader, reader, result);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtGenericInst*, gi, read_method_spec_generic_inst(reader, gcc, gc));

    auto newGc = RtGenericContext{nullptr, gi};
    RET_OK(vm::Method::inflate(baseMethod, &newGc));
}

RtResult<const RtMethodInfo*> RtModuleDef::get_method_by_token(const RtToken& token, const RtGenericContainerContext& gcc, const RtGenericContext* gc)
{
    switch (token.table_type)
    {
    case TableType::Method:
        return get_method_by_rid(token.rid);
    case TableType::MemberRef:
        return get_method_by_member_ref_rid(token.rid, gcc, gc);
    case TableType::MethodSpec:
        return get_method_by_method_spec_rid(token.rid, gcc, gc);
    default:
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
}

RtResult<const RtFieldInfo*> RtModuleDef::get_field_by_rid(uint32_t rid)
{
    if (rid == 0 || rid > _cliImage.get_table_row_num(TableType::Field))
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    auto opt_typeDefRid = _cliImage.find_last_row_less_equal(TableType::TypeDef, 4, rid);
    if (!opt_typeDefRid)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    uint32_t typeDefRid = opt_typeDefRid.value();
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtClass*, klass, get_class_by_type_def_rid(typeDefRid));
    RET_ERR_ON_FAIL(vm::Class::initialize_fields(klass));
    assert(klass->field_count > 0);
    uint32_t firstFieldRid = RtToken::decode_rid(klass->fields[0].token);
    assert(rid >= firstFieldRid && rid < firstFieldRid + klass->field_count);
    const RtFieldInfo* field = &klass->fields[rid - firstFieldRid];
    assert(field != nullptr);
    assert(RtToken::decode_rid(field->token) == rid);
    RET_OK(field);
}

RtResult<const RtFieldInfo*> RtModuleDef::get_field_by_member_ref_rid(uint32_t rid, const RtGenericContainerContext& gcc, const RtGenericContext* gc)
{
    auto result = get_member_ref_by_rid(rid, gcc, gc);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL2(RtRuntimeHandle, memberHandle, result);
    if (memberHandle.type != RtRuntimeHandleType::Field)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    RET_OK(memberHandle.field);
}

RtResult<const RtFieldInfo*> RtModuleDef::get_field_by_token(const RtToken& token, const RtGenericContainerContext& gcc, const RtGenericContext* gc)
{
    switch (token.table_type)
    {
    case TableType::Field:
        return get_field_by_rid(token.rid);
    case TableType::MemberRef:
        return get_field_by_member_ref_rid(token.rid, gcc, gc);
    default:
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
}

RtResult<RtRuntimeHandle> RtModuleDef::get_member_ref_by_rid(uint32_t memberRefRid, const RtGenericContainerContext& gcc, const RtGenericContext* gc)
{
    auto opt_row = _cliImage.read_member_ref(memberRefRid);
    if (!opt_row)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    RowMemberRef& row = opt_row.value();
    RtToken parentToken = RtMetadata::decode_member_ref_parent_coded_index(row.class_idx);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtTypeSig*, parentTypeSig, read_typesig_from_member_parent(parentToken, gcc, gc));
    auto opt_decoded_blob = get_decoded_blob_reader(row.signature);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL2(utils::BinaryReader, reader, opt_decoded_blob);
    uint8_t byteType;
    if (!reader.try_read_byte(byteType))
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    RtSigType sigType = RtMetadata::decode_sig_type(byteType);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const char*, name, get_string(row.name));

    RtClass* baseClass;
    RtElementType eleType = parentTypeSig->ele_type;
    switch (eleType)
    {
    case RtElementType::GenericInst:
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(baseClass, vm::Class::get_class_by_type_def_gid(parentTypeSig->data.generic_class->base_type_def_gid));
        break;
    }
    default:
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(baseClass, vm::Class::get_class_from_typesig(parentTypeSig));
        break;
    }
    }

    RtGenericContainerContext declaring_gcc = vm::Class::get_generic_container_context(baseClass);

    if (sigType == RtSigType::Field)
    {
        // Find field in baseClass by name
        RET_ERR_ON_FAIL(vm::Class::initialize_fields(baseClass));
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtTypeSig*, fieldTypeSig, read_typesig(reader, declaring_gcc, nullptr));
        for (uint32_t i = 0; i < baseClass->field_count; ++i)
        {
            const RtFieldInfo* field = baseClass->fields + i;
            if (std::strcmp(field->name, name) != 0)
            {
                continue;
            }
            if (!MetadataCompare::is_typesig_equal_ignore_attrs(field->type_sig, fieldTypeSig, true))
            {
                continue;
            }
            if (eleType == RtElementType::GenericInst)
            {
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtClass*, fieldDeclClass, vm::Class::get_class_from_typesig(parentTypeSig));
                RET_ERR_ON_FAIL(vm::Class::initialize_fields(fieldDeclClass));
                RET_OK(RtRuntimeHandle{fieldDeclClass->fields + i});
            }
            else
            {
                RET_OK(RtRuntimeHandle{field});
            }
        }
        RET_ERR(RtErr::MissingField);
    }
    else if (sigType < RtSigType::Field)
    {
        // Find method in baseClass by name
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtMethodSig, methodSig, read_method_sig_skip_prologue(byteType, reader, declaring_gcc, nullptr));
        RET_ERR_ON_FAIL(vm::Class::initialize_methods(baseClass));
        for (uint32_t i = 0; i < baseClass->method_count; ++i)
        {
            const RtMethodInfo* method = baseClass->methods[i];
            if (std::strcmp(method->name, name) != 0)
            {
                continue;
            }
            if (method->parameter_count != methodSig.params.size())
            {
                continue;
            }
            uint8_t genericParamCount = method->generic_container ? method->generic_container->generic_param_count : 0;
            if (genericParamCount != methodSig.generic_param_count)
            {
                continue;
            }
            if (!MetadataCompare::is_typesig_equal_ignore_attrs(methodSig.return_type, method->return_type, true))
            {
                continue;
            }
            if (!MetadataCompare::is_typesigs_equal_ignore_attrs(methodSig.params.begin(), method->parameters, methodSig.params.size(), true))
            {
                continue;
            }
            if (eleType == RtElementType::GenericInst)
            {
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtClass*, methodDeclClass, vm::Class::get_class_from_typesig(parentTypeSig));
                RET_ERR_ON_FAIL(vm::Class::initialize_methods(methodDeclClass));
                RET_OK(RtRuntimeHandle{methodDeclClass->methods[i]});
            }
            else
            {
                RET_OK(RtRuntimeHandle{method});
            }
        }
        RET_ERR(RtErr::MissingMethod);
    }
    else
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
}

std::optional<uint32_t> RtModuleDef::get_field_offset(EncodedTokenId fieldToken) const
{
    uint32_t fieldRid = RtToken::decode_rid(fieldToken);
    auto it = _fieldRid2OffsetMap.find(fieldRid);
    if (it != _fieldRid2OffsetMap.end())
    {
        return it->second;
    }
    return std::nullopt;
}

std::optional<ClassLayoutData> RtModuleDef::get_class_layout_data(EncodedTokenId typeDefToken) const
{
    auto it = _typeDefRid2ClassLayoutMap.find(RtToken::decode_rid(typeDefToken));
    if (it != _typeDefRid2ClassLayoutMap.end())
    {
        return it->second;
    }
    return std::nullopt;
}

RtResult<const uint8_t*> RtModuleDef::get_field_rva_data(EncodedTokenId fieldToken) const
{
    uint32_t rid = RtToken::decode_rid(fieldToken);
    auto optFieldRavRid = _cliImage.find_row_of_owner(TableType::FieldRva, 1, rid);
    if (!optFieldRavRid)
    {
        RET_OK((const uint8_t*)nullptr);
    }
    uint32_t fieldRvaRid = optFieldRavRid.value();

    auto optRow = _cliImage.read_field_rva(fieldRvaRid);
    if (!optRow)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    RowFieldRva& row = optRow.value();
    auto optOffset = _cliImage.rva_to_image_offset(row.rva);
    if (!optOffset)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    uint32_t offset = optOffset.value();
    RET_OK(_cliImage.get_image_data() + offset);
}

RtResult<utils::BinaryReader> RtModuleDef::get_const_or_default_value(EncodedTokenId fieldOrParamToken) const
{
    RtToken token = RtToken::decode(fieldOrParamToken);
    uint32_t encodedIndex = RtMetadata::encode_has_constant_coded_index(token.table_type, token.rid);
    auto optConstRid = _cliImage.find_row_of_owner(TableType::Constant, 2, encodedIndex);
    if (!optConstRid)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    auto optRow = _cliImage.read_constant(optConstRid.value());
    if (!optRow)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    RowConstant& row = optRow.value();
    return get_decoded_blob_reader(row.value);
}

RtResult<EncodedTokenId> RtModuleDef::get_parameter_token(EncodedTokenId methodDefToken, int32_t paramIndex)
{
    uint32_t methodRid = RtToken::decode_rid(methodDefToken);
    auto optMethodRow = _cliImage.read_method(methodRid);
    if (!optMethodRow)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    RowMethod& methodRow = optMethodRow.value();
    uint32_t paramRidStart = methodRow.param_list;
    uint32_t paramRidEnd;
    auto optNextMethodRow = _cliImage.read_method(methodRid + 1);
    if (optNextMethodRow)
    {
        RowMethod& nextMethodRow = optNextMethodRow.value();
        paramRidEnd = nextMethodRow.param_list;
    }
    else
    {
        paramRidEnd = _cliImage.get_table_row_num(TableType::Param) + 1;
    }
    for (uint32_t rid = paramRidStart; rid < paramRidEnd; ++rid)
    {
        auto optParamRow = _cliImage.read_param(rid);
        if (!optParamRow)
        {
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        }
        RowParam& paramRow = optParamRow.value();
        // sequence is 1-based index, 0 means return value
        if (paramRow.sequence == paramIndex + 1)
        {
            RET_OK(RtToken::encode(TableType::Param, rid));
        }
    }
    RET_OK(RtToken::Invalid);
}

RtResult<RtClass*> RtModuleDef::get_global_type_def()
{
    return get_class_by_name2("", STR_MODULE, false, true);
}

RtResult<bool> RtModuleDef::is_exported_klass(RtClass* klass)
{
    uint32_t flags = klass->flags;
    RtTypeAttribute vis = (RtTypeAttribute)(flags & RT_VISIBILITY_MASK);
    switch (vis)
    {
    case RtTypeAttribute::Public:
    case RtTypeAttribute::NestedPublic:
    case RtTypeAttribute::NestedFamily:
    case RtTypeAttribute::NestedAssembly:
    case RtTypeAttribute::NestedFamAndAssem:
    case RtTypeAttribute::NestedFamOrAssem:
        RET_OK(true);
    default:
        RET_OK(false);
    }
}

RtResultVoid RtModuleDef::get_types(bool exported_only, utils::Vector<RtClass*>& types)
{
    uint32_t rows = get_table_row_num(TableType::TypeDef);
    for (uint32_t i = 1; i <= rows; ++i)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtClass*, klass, get_class_by_type_def_rid(i));
        if (exported_only)
        {
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, isExported, is_exported_klass(klass));
            if (!isExported)
            {
                continue;
            }
        }
        types.push_back(klass);
    }
    RET_VOID_OK();
}

void RtModuleDef::fill_assembly_name(RtMonoAssemblyName& out) const
{
    out.name = utils::StringUtil::strdup(_assemblyName.name);
    out.culture = utils::StringUtil::strdup(_assemblyName.culture);
    out.public_key = _assemblyName.public_key;
    out.hash_algorithm = _assemblyName.hash_algorithm;
    out.version_major = _assemblyName.version_major;
    out.version_minor = _assemblyName.version_minor;
    out.version_build = _assemblyName.version_build;
    out.version_revision = _assemblyName.version_revision;
    out.flags = _assemblyName.flags;
}

} // namespace metadata
} // namespace leanclr
