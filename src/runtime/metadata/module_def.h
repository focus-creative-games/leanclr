#pragma once

#include "core/rt_base.h"
#include "cli_image.h"
#include "pdb_image.h"
#include "rt_metadata.h"
#include "vm/rt_managed_types.h"
#include "alloc/mem_pool.h"
#include "utils/rt_vector.h"
#include "utils/binary_reader.h"
#include "utils/hashmap.h"
#include "utils/string_util.h"
#include "utils/rt_span.h"

namespace leanclr
{
namespace metadata
{

struct ClassLayoutData
{
    uint16_t packing;
    uint32_t size;

    ClassLayoutData(uint16_t p, uint32_t s) : packing(p), size(s)
    {
    }
};

enum class RtILMethodFormat : uint8_t
{
    Tiny = 0x2,
    Fat = 0x3,
    MoreSects = 0x8,
    InitLocals = 0x10,
};

// Converted from Rust ILSection/headers; names prefixed with Rt
enum class RtILSection : uint8_t
{
    EHTable = 0x1,
    _OptILTable = 0x2,
    FatFormat = 0x40,
    MoreSects = 0x80,
};

// Fat IL method header (ECMA-335 compliant layout)
struct RtILMethodFatHeader
{
    uint16_t flags : 12;
    uint16_t size : 4;
    uint16_t max_stack;
    uint32_t code_size;
    uint32_t local_var_sig_tok;
};

static_assert(sizeof(RtILMethodFatHeader) == 12, "RtILMethodFatHeader size must be 12 bytes");

// Small exception handling clause (packed per spec)
struct RtILEHSmall
{
    uint16_t flags;
    uint16_t try_offset;
    uint32_t try_length : 8;
    uint32_t handler_offset : 16;
    uint32_t handler_length : 8;
    uint32_t class_token_or_filter_offset;
};

static_assert(sizeof(RtILEHSmall) == 12, "RtILEHSmall size must be 12 bytes");

// Fat exception handling clause
struct RtILEHFat
{
    uint32_t flags;
    uint32_t try_offset;
    uint32_t try_length;
    uint32_t handler_offset;
    uint32_t handler_length;
    uint32_t class_token_or_filter_offset;
};
static_assert(sizeof(RtILEHFat) == 24, "RtILEHFat size must be 24 bytes");

// Small EH section header; followed by a variable-length array of RtILEHSmall
struct RtILEHSectionHeaderSmall
{
    uint8_t kind;
    uint8_t data_size;
    uint16_t _reserved;
    RtILEHSmall clauses[1]; // trailing data
};

// Fat EH section header; followed by a variable-length array of RtILEHFat
struct RtILEHSectionHeaderFat
{
    uint8_t kind;
    uint8_t data_size;
    uint8_t data_size1;
    uint8_t data_size2;
    RtILEHFat clauses[1]; // trailing data
};

class RtModuleDef;

class RtModuleDef
{
  public:
    RtModuleDef(RtAssembly* assembly, const CliImage& cliImage, PdbImage* pdbImage, alloc::MemPool& pool)
        : _assembly(assembly), _aotModuleData(nullptr), _cliImage(cliImage), _pdbImage(pdbImage), _pool(pool), _name(nullptr), _nameNoExt(nullptr),
          _classes(nullptr), _classCount(0), _methods(nullptr), _methodCount(0), _id(0), _refOnly(false), _referenceAssemblies(nullptr),
          _referenceAssemblyCount(0), _corLib(false), _moduleCctorFinished(false)
    {
    }

    static void register_module_def(RtModuleDef* moduleDef);
    static utils::Span<RtModuleDef*> get_registered_modules();
    static RtModuleDef* find_module(const char* name);
    static RtModuleDef* get_module_by_id(uint32_t id);
    static RtModuleDef* get_corlib_module();

    RtAssembly* get_assembly() const
    {
        return _assembly;
    }

    const RtAotModuleData* get_aot_module_data() const
    {
        return _aotModuleData;
    }

    void set_aot_module_data(const RtAotModuleData* aotModuleData)
    {
        _aotModuleData = aotModuleData;
    }

    const CliImage& get_cli_image() const
    {
        return _cliImage;
    }

    PdbImage* get_pdb_image() const
    {
        return _pdbImage;
    }

    alloc::MemPool& get_mem_pool() const
    {
        return _pool;
    }

    // Module state
    bool is_module_cctor_finished() const
    {
        return _moduleCctorFinished;
    }

    void set_module_cctor_finished()
    {
        _moduleCctorFinished = true;
    }

    // Assembly info accessors
    const RtAssemblyName& get_assembly_name() const
    {
        return _assemblyName;
    }

    const char* get_name() const
    {
        return _name;
    }

    const char* get_name_no_ext() const
    {
        return _nameNoExt;
    }

    uint32_t get_id() const
    {
        return _id;
    }

    bool get_ref_only() const
    {
        return _refOnly;
    }

    bool is_corlib() const
    {
        return _corLib;
    }

    EncodedTokenId get_assembly_token() const
    {
        return RtToken::encode(TableType::Assembly, 1);
    }

    EncodedTokenId get_module_token() const
    {
        return RtToken::encode(TableType::Module, 1);
    }

    EncodedTokenId get_entrypoint_token() const
    {
        return _cliImage.get_entry_point_token();
    }

    // Class and method accessors
    // RtClass** get_classes()
    // {
    //     return _classes;
    // }
    uint32_t get_class_count() const
    {
        return _classCount;
    }
    uint32_t get_method_count() const
    {
        return _methodCount;
    }

    // Table access
    uint32_t get_table_row_num(TableType table_type) const
    {
        return _cliImage.get_table_row_num(table_type);
    }

    // String and blob heap access (Rust Result => core::Result)
    RtResult<const char*> get_string(uint32_t index) const;
    RtResult<const uint8_t*> get_blob(uint32_t index) const;

    RtResult<utils::BinaryReader> get_decoded_blob_reader(uint32_t index) const
    {
        return _cliImage.get_decoded_blob_reader(index);
    }
    RtResult<vm::RtString*> get_user_string(uint32_t index);

    RtResultVoid load();
    RtResultVoid setup_assembly_name();
    RtResultVoid setup_generic_params_and_containers();
    RtResultVoid setup_nested_classes();
    RtResultVoid setup_type_fullname_map();
    void setup_field_offsets();
    void setup_class_layouts();

    // Type signature access (Rust Result => core::Result)
    RtResult<const RtTypeSig*> get_type_def_by_val_typesig(uint32_t rid);
    RtResult<const RtTypeSig*> get_type_def_by_ref_typesig(uint32_t rid);
    RtResult<const RtTypeSig*> get_generic_param_typesig_by_rid(uint32_t rid, bool by_ref);

    // Class lookup methods (Rust Result => core::Result)
    RtResult<RtClass*> get_class_by_name(const char* full_name, bool ignore_case, bool throw_exception_when_not_found);
    RtResult<RtClass*> get_class_by_name2(const char* namespace_name, const char* name, bool ignore_case, bool throw_exception_when_not_found);
    RtResult<uint32_t> get_type_def_gid_by_name2(const char* namespace_name, const char* name, bool throw_exception_when_not_found);
    RtResult<RtClass*> get_class_by_nested_full_name(const char* full_name, bool ignore_case, bool throw_exception_when_not_found);
    RtResult<RtClass*> get_class_by_type_def_rid(uint32_t rid);
    RtResult<RtClass*> get_exported_class_by_token(const RtToken& token, const char* namespace_name, const char* name, bool ignore_case,
                                                   bool throw_exception_when_not_found);
    RtResult<uint32_t> get_exported_type_def_gid_by_token(const RtToken& token, const char* namespace_name, const char* name,
                                                          bool throw_exception_when_not_found);
    RtClass* try_get_created_class_by_type_def_rid(uint32_t rid) const;

    RtResult<RtClass*> get_class_by_type_ref_rid(uint32_t rid);
    RtResult<uint32_t> get_type_def_gid_by_type_ref_rid(uint32_t rid);
    RtResult<const RtTypeSig*> get_type_spec_typesig_by_rid(uint32_t rid, const RtGenericContainerContext& gcc, const RtGenericContext* gc);
    RtResult<const RtTypeSig*> get_typesig_by_type_def_ref_spec_token(const RtToken& token, const RtGenericContainerContext& gcc, const RtGenericContext* gc);
    RtResult<RtClass*> get_class_by_type_spec_rid(uint32_t rid, const RtGenericContainerContext& gcc, const RtGenericContext* gc);
    RtResult<RtClass*> get_class_by_type_def_ref_spec_token(const RtToken& token, const RtGenericContainerContext& gcc, const RtGenericContext* gc);

    bool has_nested_types(EncodedTokenId enclosing_type_def_token) const
    {
        uint32_t rid = RtToken::decode_rid(enclosing_type_def_token);
        return _enclosingTypeDefRid2StartRidMap.find(rid) != _enclosingTypeDefRid2StartRidMap.end();
    }

    std::optional<uint32_t> get_enclosing_type_def_rid(EncodedTokenId nested_type_def_token)
    {
        uint32_t rid = RtToken::decode_rid(nested_type_def_token);
        auto it = _nestedTypeDefRid2EnclosingTypeDefRidMap.find(rid);
        if (it != _nestedTypeDefRid2EnclosingTypeDefRidMap.end())
        {
            return std::optional<uint32_t>(it->second);
        }
        return std::nullopt;
    }

    RtResultVoid get_nested_type_def_rid(EncodedTokenId enclosing_type_def_token, utils::Span<uint32_t>& outNestedTypeDefRids);
    RtResultVoid get_nested_classs(EncodedTokenId enclosing_type_def_token, utils::Vector<RtClass*>& outNestedClasses);

    RtResult<const RtGenericContainer*> get_generic_container(EncodedTokenId ownerToken);

    RtResult<const RtMethodInfo*> get_method_by_rid(uint32_t rid);
    RtResult<const RtMethodInfo*> get_method_by_member_ref_rid(uint32_t rid, const RtGenericContainerContext& gcc, const RtGenericContext* gc);
    RtResult<const RtMethodInfo*> get_method_by_method_spec_rid(uint32_t rid, const RtGenericContainerContext& gcc, const RtGenericContext* gc);
    RtResult<const RtMethodInfo*> get_method_by_token(const RtToken& token, const RtGenericContainerContext& gcc, const RtGenericContext* gc);

    RtResult<const RtFieldInfo*> get_field_by_rid(uint32_t rid);
    RtResult<const RtFieldInfo*> get_field_by_member_ref_rid(uint32_t rid, const RtGenericContainerContext& gcc, const RtGenericContext* gc);
    RtResult<const RtFieldInfo*> get_field_by_token(const RtToken& token, const RtGenericContainerContext& gcc, const RtGenericContext* gc);

    RtResult<RtRuntimeHandle> get_member_ref_by_rid(uint32_t memberRefRid, const RtGenericContainerContext& gcc, const RtGenericContext* gc);

    std::optional<uint32_t> get_field_offset(EncodedTokenId fieldToken) const;
    std::optional<ClassLayoutData> get_class_layout_data(EncodedTokenId typeDefToken) const;

    RtResult<const uint8_t*> get_field_rva_data(EncodedTokenId fieldToken) const;
    RtResult<utils::BinaryReader> get_const_or_default_value(EncodedTokenId fieldOrParamToken) const;

    // Export types
    RtResult<RtClass*> get_global_type_def();
    RtResultVoid get_types(bool exported_only, utils::Vector<RtClass*>& types);
    void fill_assembly_name(RtMonoAssemblyName& out) const;

    // Reference assembly access
    // TODO: Implement reference assembly resolution
    RtResult<RtAssembly*> get_reference_assembly(uint32_t rid);
    RtResultVoid get_reference_assemblies(utils::Vector<RtAssembly*>& ref_assemblies);

    struct RtGenericParamConstraint
    {
        const RtTypeSig** constraints;
        uint32_t count;
    };
    RtResult<RtGenericParamConstraint> read_generic_param_constraints(uint32_t genericParamRid, const RtGenericContainerContext& gcc);

    RtResultVoid read_type_modifier(utils::BinaryReader& reader, bool optional, const RtGenericContainerContext& gcc, const RtGenericContext* gc,
                                    utils::Vector<metadata::RtClass*>& outTypeMods);
    RtResultVoid read_member_modifier(utils::BinaryReader& reader, bool optional, const RtGenericContainerContext& gcc, const RtGenericContext* gc,
                                      utils::Vector<metadata::RtClass*>& outMemberMods);
    RtResultVoid read_property_type_modifier(utils::BinaryReader& reader, bool optional, const RtGenericContainerContext& gcc, const RtGenericContext* gc,
                                             utils::Vector<metadata::RtClass*>& outPropMods);
    RtResultVoid read_parameter_modifier(utils::BinaryReader& reader, int32_t index, bool optional, const RtGenericContainerContext& gcc,
                                         const RtGenericContext* gc, utils::Vector<metadata::RtClass*>& outParamMods);

    RtResult<const RtTypeSig*> read_field_sig(utils::BinaryReader& reader, const RtGenericContainerContext& gcc, const RtGenericContext* gc);
    RtResult<const RtTypeSig*> read_field_sig(uint32_t signature, const RtGenericContainerContext& gcc, const RtGenericContext* gc)
    {
        auto ret = get_decoded_blob_reader(signature);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL2(utils::BinaryReader, reader, ret);
        return read_field_sig(reader, gcc, gc);
    }

    RtResult<RtPropertySig> read_property_sig(utils::BinaryReader& reader, const RtGenericContainerContext& gcc, const RtGenericContext* gc);
    RtResult<RtPropertySig> read_property_sig(uint32_t signature, const RtGenericContainerContext& gcc, const RtGenericContext* gc)
    {
        auto ret = get_decoded_blob_reader(signature);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL2(utils::BinaryReader, reader, ret);
        return read_property_sig(reader, gcc, gc);
    }

    RtResult<RtMethodSig> read_method_sig(utils::BinaryReader& reader, const RtGenericContainerContext& gcc, const RtGenericContext* gc);
    RtResult<RtMethodSig> read_method_sig(uint32_t signature, const RtGenericContainerContext& gcc, const RtGenericContext* gc)
    {
        auto ret = get_decoded_blob_reader(signature);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL2(utils::BinaryReader, reader, ret);
        return read_method_sig(reader, gcc, gc);
    }
    RtResult<RtMethodSig> read_method_sig_skip_prologue(uint8_t sigType, utils::BinaryReader& reader, const RtGenericContainerContext& gcc,
                                                        const RtGenericContext* gc);
    RtResult<RtMethodSig> read_stadalone_method_sig(EncodedTokenId standaloneSigToken, const RtGenericContainerContext& gcc, const RtGenericContext* gc);
    RtResult<const RtGenericInst*> read_method_spec_generic_inst(utils::BinaryReader& reader, const RtGenericContainerContext& gcc, const RtGenericContext* gc);
    RtResultVoid read_local_var_sig(EncodedTokenId localVarSigToken, const RtGenericContainerContext& gcc, const RtGenericContext* gc,
                                    utils::Vector<const RtTypeSig*>& outLocalVarTypeSigs);

    RtResult<const RtTypeSig*> read_typesig_from_member_parent(const RtToken& token, const RtGenericContainerContext& gcc, const RtGenericContext* gc);

    RtResult<std::optional<RowImplMap>> read_method_impl_map(EncodedTokenId methodDefToken);

    RtResult<std::optional<RtMethodBody>> read_method_body(EncodedTokenId methodDefToken);
    RtResult<RtMethodBody> read_method_body_from_rva(uint32_t rva);

    RtResult<RtCustomAttributeRidRange> get_custom_attribute_rid_range(EncodedTokenId parentToken);
    RtResult<RtCustomAttributeRawData> get_custom_attribute_raw_data(uint32_t customAttributeRid);

    /// @brief
    /// @param methodDefToken
    /// @param paramIndex  -1 for return type, 0..n-1 for parameters
    /// @return
    RtResult<EncodedTokenId> get_parameter_token(EncodedTokenId methodDefToken, int32_t paramIndex);

    RtResult<bool> is_exported_klass(RtClass* klass);
    RtResult<bool> is_value_type_or_enum(uint32_t encodedParentType);

  private:
    RtResult<const uint32_t*> read_u32_array(utils::BinaryReader& reader, size_t num);
    RtResult<const RtTypeSig*> read_typesig(utils::BinaryReader& reader, const RtGenericContainerContext& gcc, const RtGenericContext* gc);
    RtResultVoid read_typesig_impl(utils::BinaryReader& reader, const RtGenericContainerContext& gcc, const RtGenericContext* gc, RtTypeSig& result);

    enum class AssemblyResolveStatus
    {
        NotResolvedYet,
        Success,
        NotFoundOrFailed,
    };

    struct ReferenceAssembly
    {
        RtAssembly* assembly;
        AssemblyResolveStatus status;
    };

    const CliImage& _cliImage;
    PdbImage* _pdbImage;
    alloc::MemPool& _pool;

    const RtAotModuleData* _aotModuleData;
    RtAssembly* const _assembly;

    const char* _name;
    const char* _nameNoExt;

    ReferenceAssembly* _referenceAssemblies;
    uint32_t _referenceAssemblyCount;
    RtClass** _classes;
    uint32_t _classCount;
    const RtMethodInfo** _methods;
    uint32_t _methodCount;
    RtTypeSig* _typeDefByValTypeSigs{};
    RtTypeSig* _typeDefByRefTypeSigs{};
    RtGenericParam* _genericParams{};
    utils::HashMap<uint32_t, RtGenericContainer*> _genericContainers;
    RtTypeSig* _genericParamByValTypeSigs{};
    RtTypeSig* _genericParamByRefTypeSigs{};

    struct EnclosingTypeInfo
    {
        uint32_t* nested_type_def_rids;
        uint32_t count;
    };
    utils::HashMap<uint32_t, EnclosingTypeInfo> _enclosingTypeDefRid2StartRidMap;
    utils::HashMap<uint32_t, uint32_t> _nestedTypeDefRid2EnclosingTypeDefRidMap;
    utils::HashMap<uint32_t, uint32_t> _fieldRid2OffsetMap;
    utils::HashMap<uint32_t, ClassLayoutData> _typeDefRid2ClassLayoutMap;
    utils::HashMap<utils::FullNameStr, uint32_t, utils::FullNameStrHasher, utils::FullNameStrCompare> _typeDefFullName2TypeDefRidMap;
    utils::HashMap<utils::FullNameStr, RtToken, utils::FullNameStrHasher, utils::FullNameStrCompare> _typeDefFullName2ExportedTypeRidMap;

    RtAssemblyName _assemblyName{};
    uint32_t _id;
    bool _refOnly;
    bool _corLib;
    bool _moduleCctorFinished;

    utils::HashMap<uint32_t, vm::RtString*> _userStringMap;
};
} // namespace metadata
} // namespace leanclr
