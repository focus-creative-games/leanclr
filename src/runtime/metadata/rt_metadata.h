#pragma once

#include "cli_metadata.h"
#include "utils/rt_vector.h"

namespace leanclr
{
namespace interp
{
union RtStackObject;
struct RtInterpMethodInfo;
} // namespace interp
} // namespace leanclr

namespace leanclr
{
namespace metadata
{

// Element type enumeration
enum class RtElementType : uint8_t
{
    End = 0x0,
    Void = 0x1,
    Boolean = 0x2,
    Char = 0x3,
    I1 = 0x4,
    U1 = 0x5,
    I2 = 0x6,
    U2 = 0x7,
    I4 = 0x8,
    U4 = 0x9,
    I8 = 0xA,
    U8 = 0xB,
    R4 = 0xC,
    R8 = 0xD,
    String = 0xE,
    Ptr = 0xF,
    ByRef = 0x10,
    ValueType = 0x11,
    Class = 0x12,
    Var = 0x13,
    Array = 0x14,
    GenericInst = 0x15,
    TypedByRef = 0x16,
    Preserved = 0x17,
    I = 0x18,
    U = 0x19,
    FnPtr = 0x1B,
    Object = 0x1C,
    SZArray = 0x1D,
    MVar = 0x1E,
    CModReqd = 0x1F,
    CModOpt = 0x20,
    Internal = 0x21,
    Modifier = 0x40,
    Sentinel = 0x41,
    Pinned = 0x45,
    CASystemType = 0x50,
    CAObject = 0x51,
    CAPreserved = 0x52,
    CAField = 0x53,
    CAProperty = 0x54,
    CAEnum = 0x55,
};

// // Type signature flags
// enum class RtTypeSigFlags : uint8_t
// {
//     None = 0x0,
//     ByRef = 0x1,
//     Pinned = 0x2,
// };

struct RtGenericClass;
struct RtArrayType;
struct RtGenericParam;
struct RtTypeSig;
struct RtClass;
struct RtMethodInfo;
struct RtPropertyInfo;
struct RtEventInfo;
struct RtMethodSig;

// Type signature variant data union
union RtTypeSigVariantData
{
    void* dummy;
    uint32_t type_def_gid;
    const RtTypeSig* element_type;       // Ptr and SZArray -> RtTypeSig*
    const RtArrayType* array_type;       // Array -> RtArrayType*
    const RtGenericClass* generic_class; // GenericInst -> RtGenericClass*
    const RtGenericParam* generic_param; // GenericParam -> RtGenericParam*
    const RtMethodSig* method_sig;       // MethodSpec -> RtMethodSig*
};

static_assert(sizeof(RtTypeSigVariantData) == sizeof(void*), "RtTypeSigVariantData size must be pointer size");

// Type signature structure
struct RtTypeSig
{
    RtElementType ele_type;
    uint8_t field_or_param_attrs;
    union
    {
        uint8_t flags;
        struct
        {
            uint8_t by_ref : 1;
            uint8_t pinned : 1;
            uint8_t num_mods : 6;
        };
    };
    RtTypeSigVariantData data;

    bool is_void() const
    {
        return ele_type == RtElementType::Void;
    }

    bool is_by_ref() const
    {
        return by_ref != 0;
    }

    bool is_canonized() const
    {
        return pinned == 0 && num_mods == 0;
    }

    RtTypeSig to_canonized() const
    {
        RtTypeSig ts = {};
        ts.ele_type = ele_type;
        ts.by_ref = by_ref;
        ts.data = data;
        return ts;
    }

    RtTypeSig to_canonized_without_byref() const
    {
        RtTypeSig ts = {};
        ts.ele_type = ele_type;
        ts.data = data;
        return ts;
    }

    static RtTypeSig new_by_val(RtElementType eleType)
    {
        RtTypeSig ts = {};
        ts.ele_type = eleType;
        ts.by_ref = 0;
        return ts;
    }

    static RtTypeSig new_by_ref(RtElementType eleType)
    {
        RtTypeSig ts = {};
        ts.ele_type = eleType;
        ts.by_ref = 1;
        return ts;
    }

    static RtTypeSig new_byval_with_data(RtElementType eleType, const void* dataPtr)
    {
        RtTypeSig ts = {};
        ts.ele_type = eleType;
        ts.data.dummy = const_cast<void*>(dataPtr);
        return ts;
    }

    static RtTypeSig new_byref_with_data(RtElementType eleType, const void* dataPtr)
    {
        RtTypeSig ts = {};
        ts.ele_type = eleType;
        ts.by_ref = 1;
        ts.data.dummy = const_cast<void*>(dataPtr);
        return ts;
    }
};

struct RtTypeSigWithoutData
{
    RtElementType ele_type;
    uint8_t field_or_param_attrs;
    uint8_t flags;
    uint8_t num_mods;
};

// Field attribute flags
enum class RtFieldAttribute : uint32_t
{
    FieldAccessMask = 0x0007,
    CompilerControlled = 0x0000,
    Private = 0x0001,
    FamAndAssem = 0x0002,
    Assembly = 0x0003,
    Family = 0x0004,
    FamOrAssem = 0x0005,
    Public = 0x0006,
    Static = 0x0010,
    InitOnly = 0x0020,
    Literal = 0x0040,
    NotSerialized = 0x0080,
    SpecialName = 0x0200,
    PinvokeImpl = 0x2000,
    ReservedMask = 0x9500,
    RtSpecialName = 0x0400,
    HasFieldMarshal = 0x1000,
    HasDefault = 0x8000,
    HasFieldRva = 0x0100,
};

// Method attribute flags
enum class RtMethodAttribute : uint16_t
{
    MemberAccessMask = 0x0007,
    CompilerControlled = 0x0000,
    ReuseSlot = 0x0000,
    Private = 0x0001,
    FamAndAssem = 0x0002,
    Assembly = 0x0003,
    Family = 0x0004,
    FamOrAssem = 0x0005,
    Public = 0x0006,
    Static = 0x0010,
    Final = 0x0020,
    Virtual = 0x0040,
    HideBySig = 0x0080,
    NewSlot = 0x0100,
    Abstract = 0x0400,
    SpecialName = 0x0800,
    PinvokeImpl = 0x2000,
    UnmanagedExport = 0x0008,
    ReservedMask = 0xD000,
    RtSpecialName = 0x1000,
    HasSecurity = 0x4000,
    RequireSecObject = 0x8000,
};

// Method implementation attribute flags
enum class RtMethodImplAttribute : uint16_t
{
    IlOrManaged = 0x0000,
    Native = 0x0001,
    Optil = 0x0002,
    Runtime = 0x0003,
    Unmanaged = 0x0004,
    ForwardRef = 0x0010,
    PreserveSig = 0x0080,
    InternalCall = 0x1000,
    Synchronized = 0x0020,
    NoInlining = 0x0008,
    MaxMethodImplVal = 0xFFFF,
};

// Class extra attribute flags
enum class RtClassExtraAttribute : uint32_t
{
    None = 0x0,
    ValueType = 0x1,
    Nullable = 0x2,
    Enum = 0x4,
    HasReferences = 0x8,
    ArrayOrSZArray = 0x10,
    Generic = 0x20,
    HasStaticConstructor = 0x40,
    HasFinalizer = 0x80,
    MethodMask = 0x80 | 0x40,
    ReferenceType = 0x100,
};

enum class RtClassInitPart : uint32_t
{
    Field = 0x1,
    Method = 0x2,
    Property = 0x4,
    Event = 0x8,
    VirtualTable = 0x10,
    SuperTypes = 0x20,
    InterfaceTypes = 0x40,
    NestedClasses = 0x80,
    All = 0x10000,
    RuntimeClassInit = 0x20000,
};

// Class family enumeration
enum class RtClassFamily : uint8_t
{
    TypeDef,
    GenericInst,
    ArrayOrSZArray,
    GenericParam,
    // ByRef,
    TypeOrFnPtr,
};

// Type attribute flags
enum class RtTypeAttribute : uint32_t
{
    NotPublic = 0x00000000,
    AutoLayout = 0x00000000,
    Class = 0x00000000,
    AnsiClass = 0x00000000,
    Public = 0x00000001,
    NestedPublic = 0x00000002,
    NestedPrivate = 0x00000003,
    NestedFamily = 0x00000004,
    NestedAssembly = 0x00000005,
    NestedFamAndAssem = 0x00000006,
    NestedFamOrAssem = 0x00000007,
    VisibilityMask = 0x00000007,
    LayoutMask = 0x00000018,
    SequentialLayout = 0x00000008,
    ExplicitLayout = 0x00000010,
    Interface = 0x00000020,
    ClassSemanticMask = 0x00000020,
    Abstract = 0x00000080,
    Sealed = 0x00000100,
    SpecialName = 0x00000400,
    Import = 0x00001000,
    Serializable = 0x00002000,
    StringFormatMask = 0x00030000,
    UnicodeClass = 0x00010000,
    AutoClass = 0x00020000,
    BeforeFieldInit = 0x00100000,
    Forwarder = 0x00200000,
    ReservedMask = 0x00040800,
    RtSpecialName = 0x00000800,
    HasSecurity = 0x00040000,
};

// Method semantics attributes
enum class RtMethodSemanticsAttributes : uint16_t
{
    Setter = 0x1,
    Getter = 0x2,
    Other = 0x4,
    AddOn = 0x8,
    RemoveOn = 0x10,
    Fire = 0x20,
};

// Parameter attribute flags
enum class RtParamAttribute : uint32_t
{
    In = 0x0001,
    Out = 0x0002,
    Optional = 0x0010,
    ReservedMask = 0xF000,
    HasDefault = 0x1000,
    HasFieldMarshal = 0x2000,
    Unused = 0xCFE0,
};

// Signature type enumeration
enum class RtSigType : uint8_t
{
    Default = 0x0,
    C = 0x1,
    StdCall = 2,
    ThisCall = 3,
    FastCall = 4,
    VarArg = 5,
    Field = 0x6,
    LocalVar = 0x7,
    Property = 0x8,
    MethodSpec = 0xA,
    GenericInst = 0x10,
    HasThis = 0x20,
    Sentinel = 0x41,
    TypeMask = 0x0F,
    FlagMask = 0xF0,
};

// Method signature structure
struct RtMethodSig
{
    uint8_t flags;
    uint8_t generic_param_count;
    const RtTypeSig* return_type;
    utils::Vector<const RtTypeSig*> params;
};

// Property signature structure
struct RtPropertySig
{
    const RtTypeSig* type_sig;
    utils::Vector<const RtTypeSig*> params;
};

// Array type structure
struct RtArrayType
{
    const RtTypeSig* ele_type;
    const uint32_t* sizes;
    const uint32_t* bounds;
    uint8_t rank;
    uint8_t num_sizes;
    uint8_t num_bounds;

    bool is_canonized() const
    {
        return num_sizes == 0 && num_bounds == 0;
    }
};

struct RtGenericContainer;

enum class RtGenericParamAttribute : uint16_t
{
    None = 0x0,
    VarianceMask = 0x3,
    Covariant = 0x1,
    Contravariant = 0x2,
    SpecialConstraintMask = 0x1C,
    ReferenceTypeConstraint = 0x4,
    NotNullableValueTypeConstraint = 0x8,
    DefaultConstructorConstraint = 0x10,
};

// Generic parameter structure
struct RtGenericParam
{
    uint32_t gid;
    const char* name;
    uint16_t flags;
    uint16_t index;
    uint8_t constraint_type_sig_count;
    const RtTypeSig** constraint_type_sigs;
    const RtGenericContainer* owner;
};

// Generic container structure
struct RtGenericContainer
{
    const RtGenericParam* generic_params;
    uint32_t owner_gid;
    uint8_t generic_param_count;
    bool is_method;
    bool inited;
};

// Generic container context structure
struct RtGenericContainerContext
{
    const RtGenericContainer* klass;
    const RtGenericContainer* method;
};

// Field information structure
struct RtFieldInfo
{
    RtClass* parent;
    const char* name;
    const RtTypeSig* type_sig;
    uint32_t flags;
    uint32_t offset;
    EncodedTokenId token;
};

// Generic instance structure
struct RtGenericInst
{
    const RtTypeSig* const* generic_args;
    uint8_t generic_arg_count;
};

// Generic context structure
struct RtGenericContext
{
    const RtGenericInst* class_inst;
    const RtGenericInst* method_inst;
};

// Generic method structure
struct RtGenericMethod
{
    uint32_t base_method_gid;
    RtGenericContext generic_context;
};

// Method argument description
enum class RtArgOrLocOrFieldReduceType
{
    Unspecific,
    Void,
    I1,
    U1,
    I2,
    U2,
    I4,
    I8,
    I,
    R4,
    R8,
    Ref,
    Other,
};

// Method argument descriptor structure
struct RtMethodArgDesc
{
    RtArgOrLocOrFieldReduceType reduce_type;
    uint16_t stack_object_size;
};

// Managed method pointer type (noexcept not on alias - MSVC C2279; all invokers implement noexcept)
using RtManagedMethodPointer = void (*)();
using RtInvokeMethodPointer = RtResultVoid (*)(RtManagedMethodPointer, const RtMethodInfo*, const interp::RtStackObject*, interp::RtStackObject*);
using RtNativeMethodPointer = void (*)();

#define CAST_AS_NOEXCEP_MANAGED_METHOD_POINTER(p) ((void (*)() noexcept)(p))
#define CAST_AS_NOEXCEP_INVOKE_METHOD_POINTER(p)                                                                         \
    ((::leanclr::RtResultVoid (*)(::leanclr::metadata::RtManagedMethodPointer, const ::leanclr::metadata::RtMethodInfo*, \
                                  const ::leanclr::interp::RtStackObject*, ::leanclr::interp::RtStackObject*) noexcept)(p))

// Interface offset structure
struct RtInterfaceOffset
{
    const RtClass* interface;
    uint16_t offset;
};

// Virtual invoke data structure
struct RtVirtualInvokeData
{
    const RtMethodInfo* method;
    const RtMethodInfo* method_impl;
};

// Method definition or reference
struct RtMethodDefOrRef
{
    const RtClass* parent;
    const RtMethodInfo* method;
};

enum class RtInvokerType : uint8_t
{
    NotImplemented,
    InternalCall,
    Intrinsic,
    CustomInstrinsic,
    PInvoke,
    Interpreter,
    InterpreterVirtualAdjustThunk,
    RuntimeImpl,
    Aot,
    AotVirtualAdjustThunk,
    NewObjInternalCall,
    NewObjIntrinsic,
};

// Method information structure
struct RtMethodInfo
{
    const RtClass* parent;
    const char* name;
    const RtGenericContainer* generic_container;
    const RtGenericMethod* generic_method;
    const RtTypeSig* return_type;
    const RtTypeSig** parameters;
    const RtMethodArgDesc* arg_descs;
    RtManagedMethodPointer method_ptr;
    RtManagedMethodPointer virtual_method_ptr;
    RtInvokeMethodPointer invoke_method_ptr;
    const interp::RtInterpMethodInfo* interp_data;
    EncodedTokenId token;
    uint16_t parameter_count;
    uint16_t flags;
    uint16_t iflags;
    uint16_t slot;
    uint16_t total_arg_stack_object_size;
    uint16_t ret_stack_object_size;
    RtInvokerType invoker_type;
};

// Property information structure
struct RtPropertyInfo
{
    const RtClass* parent;
    const char* name;
    RtPropertySig property_sig;
    uint16_t flags;
    const RtMethodInfo* get_method;
    const RtMethodInfo* set_method;
    EncodedTokenId token;
};

// Event information structure
struct RtEventInfo
{
    const RtClass* parent;
    const char* name;
    const RtTypeSig* type_sig;
    uint16_t flags;
    const RtMethodInfo* add_method;
    const RtMethodInfo* remove_method;
    const RtMethodInfo* raise_method;
    EncodedTokenId token;
};

// Generic class structure
struct RtGenericClass
{
    uint32_t base_type_def_gid;
    const RtGenericInst* class_inst;
    RtTypeSig by_val_type_sig;
    RtTypeSig by_ref_type_sig;
    const RtClass* cache_base_klass;
    const RtClass* cache_klass;
};

struct RtAssemblyName
{
    const char* name;
    const char* culture;
    const uint8_t* public_key;
    uint32_t hash_algorithm;
    uint32_t hash_len;
    uint32_t flags;
    uint16_t version_major;
    uint16_t version_minor;
    uint16_t version_build;
    uint16_t version_revision;
    uint8_t public_key_token[8];
};

struct RtMonoAssemblyName
{
    const char* name;
    const char* culture;
    const char* hash_value;
    const uint8_t* public_key;
    uint8_t public_key_token[17];
    uint32_t hash_algorithm;
    uint32_t hash_len;
    uint32_t flags;
    uint16_t version_major;
    uint16_t version_minor;
    uint16_t version_build;
    uint16_t version_revision;
    uint16_t arch;
};

// Constants
constexpr uint8_t RT_CODE_TYPE_MASK = 0x0003;
constexpr uint8_t RT_MANAGED_MASK = 0x0004;
constexpr uint32_t RT_VISIBILITY_MASK = 0x00000007;
constexpr uint32_t RT_CLASS_SEMANTIC_MASK = 0x00000020;
constexpr size_t RT_MAX_GENERIC_PARAM_COUNT = 32;
constexpr size_t RT_MAX_PARAM_COUNT = 65535;
constexpr size_t RT_MAX_GENERIC_PARAM_CONSTRAINT_COUNT = 32;
constexpr size_t RT_MAX_LOCAL_VAR_COUNT = 0xFFFE;
constexpr uint16_t RT_INVALID_METHOD_SLOT = 0xFFFF;
constexpr uint32_t RT_MAX_METHOD_COUNT = 0xFFFF;
constexpr uint32_t RT_MAX_FIELD_COUNT = 0xFFFF;
constexpr uint32_t RT_MAX_PROPERTY_COUNT = 0xFFFF;
constexpr uint32_t RT_MAX_EVENT_COUNT = 0xFFFF;
constexpr uint32_t RT_MAX_VTABLE_COUNT = 0xFFFF;
constexpr uint16_t RT_MAX_INTERFACE_COUNT = 0xFFFF;
constexpr uint32_t RT_MAX_NESTED_CLASS_COUNT = 0xFFFF;
constexpr uint8_t RT_MAX_ARRAY_RANK = 0x32;

// Forward declarations for assembly
struct RtAssembly;
class RtModuleDef;

// Class structure
struct RtClass
{
    RtModuleDef* image;
    const RtClass* parent;
    const char* namespaze;
    const char* name;
    const RtTypeSig* by_val;
    const RtTypeSig* by_ref;
    const RtClass* element_class;
    const RtClass* cast_class;
    const RtClass** super_types;
    const RtClass** interfaces;
    const RtClass* declaring_class; // TODO, may be we can optimize out it
    const RtClass** nested_classes; // TODO, may be we can optimize out it
    const RtGenericContainer* generic_container;
    const RtFieldInfo* fields;
    const RtMethodInfo** methods;
    const RtEventInfo* events;
    const RtPropertyInfo* properties;
    const RtVirtualInvokeData* vtable;
    const RtInterfaceOffset* interface_vtable_offsets;
    uint8_t* static_fields_data;
    void* unity_user_data;
    EncodedTokenId token;
    uint32_t instance_size_without_header;
    uint32_t static_size;
    uint32_t flags;
    uint32_t extra_flags;
    uint32_t init_flags;
    uint16_t nested_class_count; // TODO, may be we can optimize out it
    uint16_t interface_count;
    uint16_t interface_vtable_offset_count;
    uint16_t field_count;
    uint16_t method_count;
    uint16_t property_count;
    uint16_t event_count;
    uint16_t vtable_count;
    uint8_t hierarchy_depth;
    uint8_t alignment;
};

struct RtCustomAttributeRidRange
{
    const uint32_t start_rid;
    const uint32_t count;
};

struct RtCustomAttributeRawData
{
    const RtMethodInfo* ctor;
    uint32_t dataBlobIndex;
};

enum class RtMarshalNativeType
{
    Boolean = 0x2,
    I1 = 0x3,
    U1 = 0x4,
    I2 = 0x5,
    U2 = 0x6,
    I4 = 0x7,
    U4 = 0x8,
    I8 = 0x9,
    U8 = 0xA,
    R4 = 0xB,
    R8 = 0xC,
    LPSTR = 0x14,
    LPWSTR = 0x15,
    Int = 0X1f,
    Uint = 0X20,
    Func = 0x26,
    Array = 0x2a,
    Max = 0x50, // NOT DEFINED IN ecma-335, the implementation of coreclr is 0x50
};

class RtModuleDef;

struct RtAssembly
{
    RtModuleDef* mod;
};

enum class RtILExceptionClauseType : uint8_t
{
    Exception = 0x0,
    Filter = 0x1,
    Finally = 0x2,
    Fault = 0x4,
};

struct RtExceptionClause
{
    RtILExceptionClauseType flags;
    uint32_t try_offset;
    uint32_t try_length;
    uint32_t handler_offset;
    uint32_t handler_length;
    uint32_t class_token_or_filter_offset;

    bool is_in_try_block(uint32_t il_offset) const
    {
        return il_offset >= try_offset && il_offset < (try_offset + try_length);
    }

    bool is_in_handler_block(uint32_t il_offset) const
    {
        return il_offset >= handler_offset && il_offset < (handler_offset + handler_length);
    }

    bool is_finally_or_fault() const
    {
        return flags == RtILExceptionClauseType::Finally || flags == RtILExceptionClauseType::Fault;
    }
};

struct RtMethodBody
{
    uint16_t flags;
    uint16_t max_stack;
    const uint8_t* code;
    uint32_t code_size;
    EncodedTokenId local_var_sig_token;
    utils::Vector<RtExceptionClause> exception_clauses;
};

enum class RtRuntimeHandleType : uint8_t
{
    Type = 0,
    Field = 1,
    Method = 2,
};

struct RtRuntimeHandle
{
    RtRuntimeHandleType type;
    union
    {
        const void* value;
        const RtTypeSig* typeSig;
        const RtFieldInfo* field;
        const RtMethodInfo* method;
    };

    RtRuntimeHandle() : type(RtRuntimeHandleType::Type), value(nullptr)
    {
    }

    RtRuntimeHandle(const RtTypeSig* t) : RtRuntimeHandle(RtRuntimeHandleType::Type, t)
    {
    }

    RtRuntimeHandle(const RtFieldInfo* f) : RtRuntimeHandle(RtRuntimeHandleType::Field, f)
    {
    }

    RtRuntimeHandle(const RtMethodInfo* m) : RtRuntimeHandle(RtRuntimeHandleType::Method, m)
    {
    }

    RtRuntimeHandle(RtRuntimeHandleType t, const void* v) : type(t), value(v)
    {
        assert(((size_t)v & 0x3) == 0 && "Pointer's lower 2 bits must be zero for RtRuntimeHandle encoding");
    }

    // RtRuntimeHandle(const RtRuntimeHandle& other) : type(other.type), value(other.value)
    // {
    // }

    bool is_type() const
    {
        return type == RtRuntimeHandleType::Type;
    }

    bool is_method() const
    {
        return type == RtRuntimeHandleType::Method;
    }

    bool is_field() const
    {
        return type == RtRuntimeHandleType::Field;
    }
};

class RtEncodedRuntimeHandle
{
    static_assert(sizeof(size_t) == sizeof(void*), "size_t must be equal to pointer size for RtEncodedRuntimeHandle");
    const size_t value;

  public:
    explicit RtEncodedRuntimeHandle(size_t v) : value(v)
    {
    }

    explicit RtEncodedRuntimeHandle(const void* v) : value((size_t)v)
    {
    }

    RtEncodedRuntimeHandle() : value(0)
    {
    }

    const void* get_handle_without_type() const
    {
        return (void*)(value & ~static_cast<size_t>(3));
    }

    size_t get_encoded_value() const
    {
        return value;
    }

    static RtEncodedRuntimeHandle encode(RtRuntimeHandle handle)
    {
        assert(((size_t)handle.value & 0x3) == 0 && "Pointer's lower 2 bits must be zero for encoding");
        size_t encoded_value = (size_t)handle.value | static_cast<size_t>(handle.type);
        return RtEncodedRuntimeHandle{encoded_value};
    }

    static RtRuntimeHandle decode(RtEncodedRuntimeHandle encodedHandle)
    {
        return decode(encodedHandle.value);
    }

    static RtRuntimeHandle decode(size_t encodedValue)
    {
        RtRuntimeHandleType type = static_cast<RtRuntimeHandleType>(encodedValue & 0x3);
        void* value = (void*)(encodedValue & ~static_cast<size_t>(3));
        return RtRuntimeHandle{type, value};
    }
};

constexpr size_t ASSEMBLY_ID_BITS = 12;
constexpr size_t MODULE_ID_SHIFT_AMOUNT = 32 - ASSEMBLY_ID_BITS;
constexpr size_t MAX_METADATA_RID = (1 << MODULE_ID_SHIFT_AMOUNT) - 1;
constexpr size_t MAX_ASSEMBLY_ID_COUNT = 1 << ASSEMBLY_ID_BITS;

typedef void (*RtAotModuleInitializer)(RtModuleDef* mod);

struct RtAotMethodDefData
{
    EncodedTokenId token;
    RtManagedMethodPointer method_ptr;
    RtManagedMethodPointer virtual_method_ptr;
    RtInvokeMethodPointer invoke_method_ptr;
};

struct RtAotMethodImplData
{
    RtManagedMethodPointer method_ptr;
    RtManagedMethodPointer virtual_method_ptr;
    RtInvokeMethodPointer invoke_method_ptr;
};

struct RtAotModuleData
{
    const char* module_name;
    RtAotModuleInitializer initializer;
    RtAotModuleInitializer deferred_initializer;
    const RtAotMethodDefData* method_def_entries;
    uint32_t method_def_entry_count;
};

struct RtAotModulesData
{
    const RtAotModuleData** modules;
    uint32_t module_count;
};

typedef void (*ClassWalkCallback)(RtClass* klass, void* userData);

class RtMetadata
{
  public:
    static uint32_t encode_gid_by_rid(RtModuleDef& module, uint32_t rid);

    // Encode a global generic parameter RID (module ID = 0)
    static uint32_t encode_global_metadata_gid_by_rid(uint32_t rid)
    {
        assert(rid <= MAX_METADATA_RID);
        return rid; // Global parameters use module ID 0
    }

    static uint32_t encode_gid_by_token(RtModuleDef& module, EncodedTokenId token)
    {
        return encode_gid_by_rid(module, RtToken::decode_rid(token));
    }

    static uint32_t decode_module_id_from_gid(uint32_t gid)
    {
        return gid >> MODULE_ID_SHIFT_AMOUNT;
    }

    static uint32_t decode_rid_from_gid(uint32_t gid)
    {
        return gid & MAX_METADATA_RID;
    }

    // Decode TypeDefOrRef (TypeDef | TypeRef | TypeSpec), tag bits = 2
    static RtToken decode_type_def_ref_spec_coded_index(uint32_t index)
    {
        uint32_t tag = index & 0x3u;
        TableType tt;
        switch (tag)
        {
        case 0:
            tt = TableType::TypeDef;
            break;
        case 1:
            tt = TableType::TypeRef;
            break;
        case 2:
            tt = TableType::TypeSpec;
            break;
        default:
            assert(false && "Invalid tag for TypeDefOrRef coded index");
            tt = TableType::Invalid;
            break;
        }
        return RtToken{tt, index >> 2};
    }

    // Decode ResolutionScope (Module | ModuleRef | AssemblyRef | TypeRef), tag bits = 2
    static RtToken decode_resolution_scope_coded_index(uint32_t index)
    {
        uint32_t tag = index & 0x3u;
        TableType tt;
        switch (tag)
        {
        case 0:
            tt = TableType::Module;
            break;
        case 1:
            tt = TableType::ModuleRef;
            break;
        case 2:
            tt = TableType::AssemblyRef;
            break;
        default:
            tt = TableType::TypeRef;
            break;
        }
        return RtToken{tt, index >> 2};
    }

    // Encode TypeOrMethodDef (TypeDef | Method), tag bits = 1
    static uint32_t encode_type_or_method_def_coded_index(const RtToken& tk)
    {
        uint32_t tag = (tk.table_type == TableType::Method) ? 1u : 0u;
        return (tk.rid << 1) | tag;
    }

    // Decode TypeOrMethodDef (TypeDef | Method), tag bits = 1
    static RtToken decode_type_or_method_def_coded_index(uint32_t index)
    {
        uint32_t tag = index & 0x1u;
        TableType tt = (tag == 0) ? TableType::TypeDef : TableType::Method;
        return RtToken{tt, index >> 1};
    }

    // Decode MethodDefOrRef (Method | MemberRef), tag bits = 1
    static RtToken decode_method_def_or_ref_coded_index(uint32_t index)
    {
        uint32_t tag = index & 0x1u;
        TableType tt = (tag == 0) ? TableType::Method : TableType::MemberRef;
        return RtToken{tt, index >> 1};
    }

    // Decode MemberRefParent (TypeDef | TypeRef | ModuleRef | Method | TypeSpec), tag bits = 3
    static RtToken decode_member_ref_parent_coded_index(uint32_t index)
    {
        uint32_t tag = index & 0x7u;
        TableType tt;
        switch (tag)
        {
        case 0:
            tt = TableType::TypeDef;
            break;
        case 1:
            tt = TableType::TypeRef;
            break;
        case 2:
            tt = TableType::ModuleRef;
            break;
        case 3:
            tt = TableType::Method;
            break;
        case 4:
            tt = TableType::TypeSpec;
            break;
        default:
            assert(false && "Invalid tag for MemberRefParent coded index");
            tt = TableType::Invalid;
            break;
        }
        return RtToken{tt, index >> 3};
    }

    // Decode CustomAttributeType (Method | MemberRef), tag bits = 3 (only values 2, 3 valid)
    static RtToken decode_custom_attribute_type_coded_index(uint32_t index)
    {
        uint32_t tag = index & 0x7u;
        TableType tt;
        switch (tag)
        {
        case 2:
            tt = TableType::Method;
            break;
        case 3:
            tt = TableType::MemberRef;
            break;
        default:
            assert(false && "Invalid tag for CustomAttributeType coded index");
            tt = TableType::Invalid;
            break;
        }
        return RtToken{tt, index >> 3};
    }

    // Decode HasCustomAttribute (wide set of 22 types), tag bits = 5
    // Maps to: Method, Field, TypeRef, TypeDef, Param, InterfaceImpl, MemberRef, Module,
    //          DeclSecurity, Property, Event, StandaloneSig, ModuleRef, TypeSpec, Assembly,
    //          AssemblyRef, File, ExportedType, ManifestResource, GenericParam,
    //          GenericParamConstraint, MethodSpec
    static RtToken decode_has_customattribute_coded_index(uint32_t index)
    {
        uint32_t tag = index & 0x1Fu;
        TableType tt;
        switch (tag)
        {
        case 0:
            tt = TableType::Method;
            break;
        case 1:
            tt = TableType::Field;
            break;
        case 2:
            tt = TableType::TypeRef;
            break;
        case 3:
            tt = TableType::TypeDef;
            break;
        case 4:
            tt = TableType::Param;
            break;
        case 5:
            tt = TableType::InterfaceImpl;
            break;
        case 6:
            tt = TableType::MemberRef;
            break;
        case 7:
            tt = TableType::Module;
            break;
        case 8:
            tt = TableType::DeclSecurity;
            break;
        case 9:
            tt = TableType::Property;
            break;
        case 10:
            tt = TableType::Event;
            break;
        case 11:
            tt = TableType::StandaloneSig;
            break;
        case 12:
            tt = TableType::ModuleRef;
            break;
        case 13:
            tt = TableType::TypeSpec;
            break;
        case 14:
            tt = TableType::Assembly;
            break;
        case 15:
            tt = TableType::AssemblyRef;
            break;
        case 16:
            tt = TableType::File;
            break;
        case 17:
            tt = TableType::ExportedType;
            break;
        case 18:
            tt = TableType::ManifestResource;
            break;
        case 19:
            tt = TableType::GenericParam;
            break;
        case 20:
            tt = TableType::GenericParamConstraint;
            break;
        case 21:
            tt = TableType::MethodSpec;
            break;
        default:
            assert(false && "Invalid tag for HasCustomAttribute coded index");
            tt = TableType::Invalid;
            break;
        }
        return RtToken{tt, index >> 5};
    }

    // Encode HasCustomAttribute with precise mapping to tag index
    static uint32_t encode_has_customattribute_coded_index(TableType table_type, uint32_t rid)
    {
        uint32_t tag;
        switch (table_type)
        {
        case TableType::Method:
            tag = 0;
            break;
        case TableType::Field:
            tag = 1;
            break;
        case TableType::TypeRef:
            tag = 2;
            break;
        case TableType::TypeDef:
            tag = 3;
            break;
        case TableType::Param:
            tag = 4;
            break;
        case TableType::InterfaceImpl:
            tag = 5;
            break;
        case TableType::MemberRef:
            tag = 6;
            break;
        case TableType::Module:
            tag = 7;
            break;
        case TableType::DeclSecurity:
            tag = 8;
            break;
        case TableType::Property:
            tag = 9;
            break;
        case TableType::Event:
            tag = 10;
            break;
        case TableType::StandaloneSig:
            tag = 11;
            break;
        case TableType::ModuleRef:
            tag = 12;
            break;
        case TableType::TypeSpec:
            tag = 13;
            break;
        case TableType::Assembly:
            tag = 14;
            break;
        case TableType::AssemblyRef:
            tag = 15;
            break;
        case TableType::File:
            tag = 16;
            break;
        case TableType::ExportedType:
            tag = 17;
            break;
        case TableType::ManifestResource:
            tag = 18;
            break;
        case TableType::GenericParam:
            tag = 19;
            break;
        case TableType::GenericParamConstraint:
            tag = 20;
            break;
        case TableType::MethodSpec:
            tag = 21;
            break;
        default:
            assert(false && "Invalid tag for HasCustomAttribute coded index");
            return 0;
        }
        return (rid << 5) | tag;
    }

    // Decode CustomAttributeType (Method | MemberRef), tag bits = 2 (but stored as 3-bit value with 2,3)
    static RtToken decode_customattributetype_coded_index(uint32_t index)
    {
        uint32_t tag = index & 0x3u;
        TableType tt;
        switch (tag)
        {
        case 0:
            tt = TableType::Method;
            break;
        case 1:
            tt = TableType::MemberRef;
            break;
        default:
            assert(false && "Invalid tag for CustomAttributeType coded index");
            tt = TableType::Invalid;
            break;
        }
        return RtToken{tt, index >> 2};
    }

    // Encode HasSemantics (Event | Property), tag bits = 1
    static uint32_t encode_has_semantics_coded_index(TableType table_type, uint32_t rid)
    {
        uint32_t tag = (table_type == TableType::Property) ? 1u : 0u;
        return (rid << 1) | tag;
    }

    // Encode MemberForwarded (Field | Method), tag bits = 1
    static uint32_t encode_member_forwarded(TableType table_type, uint32_t rid)
    {
        uint32_t tag = (table_type == TableType::Method) ? 1u : 0u;
        return (rid << 1) | tag;
    }

    // Decode HasConstant (Field | Param | Property), tag bits = 2
    static RtToken decode_has_constant_coded_index(uint32_t index)
    {
        uint32_t tag = index & 0x3u;
        TableType tt;
        switch (tag)
        {
        case 0:
            tt = TableType::Field;
            break;
        case 1:
            tt = TableType::Param;
            break;
        case 2:
            tt = TableType::Property;
            break;
        default:
            assert(false && "Invalid tag for HasConstant coded index");
            tt = TableType::Invalid;
            break;
        }
        return RtToken{tt, index >> 2};
    }

    // Encode HasConstant with precise mapping to tag index
    static uint32_t encode_has_constant_coded_index(TableType table_type, uint32_t rid)
    {
        uint32_t tag;
        switch (table_type)
        {
        case TableType::Field:
            tag = 0;
            break;
        case TableType::Param:
            tag = 1;
            break;
        case TableType::Property:
            tag = 2;
            break;
        default:
            assert(false && "Invalid tag for HasConstant coded index");
            return 0;
        }
        return (rid << 2) | tag;
    }

    static uint32_t encode_has_field_marshal_coded_index(TableType table_type, uint32_t rid)
    {
        assert(table_type == TableType::Field || table_type == TableType::Param);
        uint32_t tag = (table_type == TableType::Field) ? 0u : 1u;
        return (rid << 1) | tag;
    }

    static RtToken decode_implementation_coded_index(uint32_t index)
    {
        uint32_t tag = index & 0x3u;
        TableType tt;
        switch (tag)
        {
        case 0:
            tt = TableType::File;
            break;
        case 1:
            tt = TableType::AssemblyRef;
            break;
        case 2:
            tt = TableType::ExportedType;
            break;
        default:
            return RtToken{(TableType)0, 0};
        }
        return RtToken{tt, index >> 2};
    }

    static RtSigType decode_sig_type(uint8_t flags)
    {
        return static_cast<RtSigType>(flags & (uint8_t)RtSigType::TypeMask);
    }

    static uint8_t decode_sig_flag(uint8_t flags)
    {
        return flags & (uint8_t)RtSigType::FlagMask;
    }
};

} // namespace metadata
} // namespace leanclr
