#pragma once

#include "core/rt_base.h"

namespace leanclr
{
namespace metadata
{

enum class TableType : uint8_t
{
    Module,
    TypeRef,
    TypeDef,
    FieldPtr,
    Field,
    MethodPtr,
    Method,
    ParamPtr,
    Param,
    InterfaceImpl,
    MemberRef,
    Constant,
    CustomAttribute,
    FieldMarshal,
    DeclSecurity,
    ClassLayout,
    FieldLayout,
    StandaloneSig,
    EventMap,
    EventPtr,
    Event,
    PropertyMap,
    PropertyPtr,
    Property,
    MethodSemantics,
    MethodImpl,
    ModuleRef,
    TypeSpec,
    ImplMap,
    FieldRva,
    EncLog,
    EncMap,
    Assembly,
    AssemblyProcessor,
    AssemblyOs,
    AssemblyRef,
    AssemblyRefProcessor,
    AssemblyRefOs,
    File,
    ExportedType,
    ManifestResource,
    NestedClass,
    GenericParam,
    MethodSpec,
    GenericParamConstraint,
    Unused8,
    Unused9,
    Unused10,
    Document,
    MethodDebugInformation,
    LocalScope,
    LocalVariable,
    LocalConstant,
    ImportScope,
    StateMachineMethod,
    CustomDebugInformation,
    Invalid = 0xFF,
};

constexpr int32_t USER_STRING_HEAP_FAKE_TABLE_TYPE = 0x70;

// Row structures
struct RowModule
{
    uint16_t generation;
    uint32_t name;
    uint32_t mvid;
    uint32_t encid;
    uint32_t enc_base_id;
};

struct RowTypeRef
{
    uint32_t resolution_scope;
    uint32_t type_name;
    uint32_t type_namespace;
};

struct RowTypeDef
{
    uint32_t flags;
    uint32_t type_name;
    uint32_t type_namespace;
    uint32_t extends;
    uint32_t field_list;
    uint32_t method_list;
};

struct RowField
{
    uint32_t flags;
    uint32_t name;
    uint32_t signature;
};

struct RowMethod
{
    uint32_t rva;
    uint16_t impl_flags;
    uint16_t flags;
    uint32_t name;
    uint32_t signature;
    uint32_t param_list;
};

struct RowParam
{
    uint16_t flags;
    uint16_t sequence;
    uint32_t name;
};

struct RowMemberRef
{
    uint32_t class_idx;
    uint32_t name;
    uint32_t signature;
};

struct RowAssembly
{
    uint32_t hash_alg_id;
    uint16_t major_version;
    uint16_t minor_version;
    uint16_t build_number;
    uint16_t revision_number;
    uint32_t flags;
    uint32_t public_key;
    uint32_t name;
    uint32_t locale;
};

struct RowAssemblyRef
{
    uint16_t major_version;
    uint16_t minor_version;
    uint16_t build_number;
    uint16_t revision_number;
    uint32_t flags;
    uint32_t public_key_or_token;
    uint32_t name;
    uint32_t locale;
    uint32_t hash_value;
};

struct RowInterfaceImpl
{
    uint32_t class_idx;
    uint32_t interface_idx;
};

struct RowConstant
{
    uint8_t type_;
    uint8_t padding;
    uint32_t parent;
    uint32_t value;
};

struct RowCustomAttribute
{
    uint32_t parent;
    uint32_t type_;
    uint32_t value;
};

struct RowFieldMarshal
{
    uint32_t parent;
    uint32_t native_type;
};

struct RowDeclSecurity
{
    uint16_t action;
    uint32_t parent;
    uint32_t permission_set;
};

struct RowClassLayout
{
    uint16_t packing_size;
    uint32_t class_size;
    uint32_t parent;
};

struct RowFieldLayout
{
    uint32_t offset;
    uint32_t field;
};

struct RowStandaloneSig
{
    uint32_t signature;
};

struct RowEventMap
{
    uint32_t parent;
    uint32_t event_list;
};

struct RowEvent
{
    uint16_t event_flags;
    uint32_t name;
    uint32_t event_type;
};

struct RowPropertyMap
{
    uint32_t parent;
    uint32_t property_list;
};

struct RowProperty
{
    uint16_t flags;
    uint32_t name;
    uint32_t type_;
};

struct RowMethodSemantics
{
    uint16_t semantics;
    uint32_t method;
    uint32_t association;
};

struct RowMethodImpl
{
    uint32_t class_idx;
    uint32_t method_body;
    uint32_t method_declaration;
};

struct RowModuleRef
{
    uint32_t name;
};

struct RowTypeSpec
{
    uint32_t signature;
};

struct RowImplMap
{
    uint16_t mapping_flags;
    uint32_t member_forwarded;
    uint32_t import_name;
    uint32_t import_scope;
};

struct RowFieldRva
{
    uint32_t rva;
    uint32_t field;
};

struct RowFile
{
    uint32_t flags;
    uint32_t name;
    uint32_t hash_value;
};

struct RowExportedType
{
    uint32_t flags;
    uint32_t type_def_id;
    uint32_t type_name;
    uint32_t type_namespace;
    uint32_t implementation;
};

struct RowManifestResource
{
    uint32_t offset;
    uint32_t flags;
    uint32_t name;
    uint32_t implementation;
};

struct RowNestedClass
{
    uint32_t nested_class;
    uint32_t enclosing_class;
};

struct RowGenericParam
{
    uint16_t number;
    uint16_t flags;
    uint32_t owner;
    uint32_t name;
};

struct RowMethodSpec
{
    uint32_t method;
    uint32_t instantiation;
};

struct RowGenericParamConstraint
{
    uint32_t owner;
    uint32_t constraint;
};

struct RowDocument
{
    uint32_t name;
    uint32_t hash_algorithm;
    uint32_t hash;
    uint32_t language;
};

struct RowMethodDebugInformation
{
    uint32_t document;
    uint32_t sequence_points;
};

struct RowLocalScope
{
    uint32_t method;
    uint32_t import_scope;
    uint32_t variables;
    uint32_t constants;
    uint32_t start_offset;
    uint32_t length;
};

struct RowLocalVariable
{
    uint16_t attributes;
    uint16_t index;
    uint32_t name;
};

struct RowLocalConstant
{
    uint32_t name;
    uint32_t signature;
};

struct RowImportScope
{
    uint32_t parent;
    uint32_t imports;
};

struct RowStateMachineMethod
{
    uint32_t move_next_method;
    uint32_t kickoff_method;
};

struct RowCustomDebugInformation
{
    uint32_t parent;
    uint32_t kind;
    uint32_t value;
};

typedef uint32_t EncodedTokenId;

// Token structure
struct RtToken
{
    static constexpr EncodedTokenId Invalid = 0;

    const TableType table_type; // TableType enum
    const uint32_t rid;

    RtToken(TableType tt, uint32_t r) : table_type(tt), rid(r)
    {
    }

    EncodedTokenId to_encoded_id() const
    {
        return (static_cast<uint32_t>(table_type) << 24) | (rid & 0x00FFFFFF);
    }

    static EncodedTokenId encode(TableType table_type, uint32_t rid)
    {
        return (static_cast<uint32_t>(table_type) << 24) | (rid & 0x00FFFFFF);
    }

    static RtToken decode(EncodedTokenId token)
    {
        auto table_type = static_cast<TableType>((token >> 24) & 0xFF);
        auto rid = token & 0x00FFFFFF;
        return RtToken(table_type, rid);
    }

    static uint32_t decode_rid(EncodedTokenId token)
    {
        return token & 0x00FFFFFF;
    }

    static TableType decode_table_type(EncodedTokenId token)
    {
        return static_cast<TableType>((token >> 24) & 0xFF);
    }
};

} // namespace metadata
} // namespace leanclr
