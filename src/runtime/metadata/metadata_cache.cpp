#include "metadata_cache.h"
#include "module_def.h"
#include "utils/hashmap.h"
#include "utils/hashset.h"
#include "utils/hash_util.h"
#include "metadata/metadata_compare.h"
#include "metadata/metadata_hash.h"
#include "alloc/metadata_allocation.h"

namespace leanclr
{
namespace metadata
{

// ===== Corlib Type Sigs =====
struct CorlibTypeSigs
{
    RtTypeSig void_by_val;
    RtTypeSig void_by_ref;
    RtTypeSig boolean_by_val;
    RtTypeSig boolean_by_ref;
    RtTypeSig char_by_val;
    RtTypeSig char_by_ref;
    RtTypeSig i8_by_val;
    RtTypeSig i8_by_ref;
    RtTypeSig u8_by_val;
    RtTypeSig u8_by_ref;
    RtTypeSig i16_by_val;
    RtTypeSig i16_by_ref;
    RtTypeSig u16_by_val;
    RtTypeSig u16_by_ref;
    RtTypeSig i32_by_val;
    RtTypeSig i32_by_ref;
    RtTypeSig u32_by_val;
    RtTypeSig u32_by_ref;
    RtTypeSig i64_by_val;
    RtTypeSig i64_by_ref;
    RtTypeSig u64_by_val;
    RtTypeSig u64_by_ref;
    RtTypeSig intptr_by_val;
    RtTypeSig intptr_by_ref;
    RtTypeSig uintptr_by_val;
    RtTypeSig uintptr_by_ref;
    RtTypeSig f32_by_val;
    RtTypeSig f32_by_ref;
    RtTypeSig f64_by_val;
    RtTypeSig f64_by_ref;
    RtTypeSig object_by_val;
    RtTypeSig object_by_ref;
    RtTypeSig string_by_val;
    RtTypeSig string_by_ref;
    RtTypeSig typedreference_by_val;
    RtTypeSig typedreference_by_ref;
    RtTypeSig class_by_val;
    RtTypeSig class_by_ref;
    RtTypeSig valuetype_by_val;
    RtTypeSig valuetype_by_ref;
};

struct ArrayTypeSigKey
{
    const RtTypeSig* element_type;
    uint8_t rank;
};

struct ArrayTypeSigKeyCompare
{
    bool operator()(const ArrayTypeSigKey& a, const ArrayTypeSigKey& b) const
    {
        return a.rank == b.rank && MetadataCompare::is_typesig_equal_ignore_attrs(a.element_type, b.element_type, false);
    }
};

struct ArrayTypeSigKeyHash
{
    std::size_t operator()(const ArrayTypeSigKey& key) const
    {
        std::size_t h = MetadataHash::hash_type_sig_ignore_attrs(key.element_type);
        h = utils::HashUtil::combine_hash(h, std::hash<uint8_t>()(key.rank));
        return h;
    }
};

// ===== MetadataCache implementation =====

// Static corlib type signatures - initialized lazily
static CorlibTypeSigs g_corlib_type_sigs;

static RtGenericContainer g_globalClassGenericContainer;
static RtGenericContainer g_globalMethodGenericContainer;

static RtGenericParam g_classGenericParams[RT_MAX_GENERIC_PARAM_COUNT];
static RtGenericParam g_methodGenericParams[RT_MAX_GENERIC_PARAM_COUNT];

static RtTypeSig g_varByVals[RT_MAX_GENERIC_PARAM_COUNT];
static RtTypeSig g_varByRefs[RT_MAX_GENERIC_PARAM_COUNT];
static RtTypeSig g_mvarByVals[RT_MAX_GENERIC_PARAM_COUNT];
static RtTypeSig g_mvarByRefs[RT_MAX_GENERIC_PARAM_COUNT];

static utils::HashSet<const RtGenericInst*, GenericInstHash, GenericInstCompare> g_genericInstCache;
static utils::HashSet<const RtGenericClass*, GenericClassHash, GenericClassCompare> g_genericClassCache;
static utils::HashMap<const RtTypeSig*, RtTypeSigByValRef, TypeSigIgnoreAttrsHasher, TypeSigIgnoreAttrsEqual> g_ptrTypesigCache;
static utils::HashMap<const RtTypeSig*, RtTypeSigByValRef, TypeSigIgnoreAttrsHasher, TypeSigIgnoreAttrsEqual> g_szarrayTypesigCache;
static utils::HashMap<ArrayTypeSigKey, RtTypeSigByValRef, ArrayTypeSigKeyHash, ArrayTypeSigKeyCompare> g_arrayTypesigCache;
static utils::HashSet<const RtGenericMethod*, GenericMethodHash, GenericMethodCompare> g_genericMethodCache;

static const char* make_generic_name(bool is_method, uint16_t index)
{
    // Example: !0 for method generic param, !!1 for class generic param
    static char buffer[8];
    if (is_method)
    {
        std::snprintf(buffer, sizeof(buffer), "!%u", index);
    }
    else
    {
        std::snprintf(buffer, sizeof(buffer), "!!%u", index);
    }
    return utils::StringUtil::strdup(buffer);
}

void MetadataCache::initialize()
{
    // Initialize global class and method generic containers
    g_globalClassGenericContainer = {};
    g_globalClassGenericContainer.owner_gid = 0;
    g_globalClassGenericContainer.generic_params = g_classGenericParams;
    g_globalClassGenericContainer.generic_param_count = RT_MAX_GENERIC_PARAM_COUNT;
    g_globalClassGenericContainer.is_method = false;
    g_globalClassGenericContainer.inited = true;

    g_globalMethodGenericContainer = {};
    g_globalMethodGenericContainer.owner_gid = 0;
    g_globalMethodGenericContainer.generic_params = g_methodGenericParams;
    g_globalMethodGenericContainer.generic_param_count = RT_MAX_GENERIC_PARAM_COUNT;
    g_globalMethodGenericContainer.is_method = true;
    g_globalMethodGenericContainer.inited = true;

    // Initialize generic parameters and type signatures
    for (uint32_t i = 0; i < RT_MAX_GENERIC_PARAM_COUNT; ++i)
    {
        uint16_t param_index = static_cast<uint16_t>(i);

        // Initialize class generic parameters
        g_classGenericParams[i].gid = RtMetadata::encode_global_metadata_gid_by_rid(i);
        g_classGenericParams[i].name = make_generic_name(false, param_index);
        g_classGenericParams[i].flags = 0;
        g_classGenericParams[i].index = param_index;
        g_classGenericParams[i].constraint_type_sig_count = 0;
        g_classGenericParams[i].constraint_type_sigs = nullptr;
        g_classGenericParams[i].owner = &g_globalClassGenericContainer;

        // Initialize method generic parameters
        g_methodGenericParams[i].gid = RtMetadata::encode_global_metadata_gid_by_rid(i);
        g_methodGenericParams[i].name = make_generic_name(true, param_index);
        g_methodGenericParams[i].flags = 0;
        g_methodGenericParams[i].index = param_index;
        g_methodGenericParams[i].constraint_type_sig_count = 0;
        g_methodGenericParams[i].constraint_type_sigs = nullptr;
        g_methodGenericParams[i].owner = &g_globalMethodGenericContainer;

        // Initialize VAR type signatures (class generic parameters)
        g_varByVals[i] = {};
        g_varByVals[i].ele_type = RtElementType::Var;
        g_varByVals[i].data.generic_param = &g_classGenericParams[i];
        g_varByVals[i].by_ref = 0;

        g_varByRefs[i] = {};
        g_varByRefs[i].ele_type = RtElementType::Var;
        g_varByRefs[i].data.generic_param = &g_classGenericParams[i];
        g_varByRefs[i].by_ref = 1;

        // Initialize MVAR type signatures (method generic parameters)
        g_mvarByVals[i] = {};
        g_mvarByVals[i].ele_type = RtElementType::MVar;
        g_mvarByVals[i].data.generic_param = &g_methodGenericParams[i];
        g_mvarByVals[i].by_ref = 0;

        g_mvarByRefs[i] = {};
        g_mvarByRefs[i].ele_type = RtElementType::MVar;
        g_mvarByRefs[i].data.generic_param = &g_methodGenericParams[i];
        g_mvarByRefs[i].by_ref = 1;
    }

    // Initialize corlib type signatures
    g_corlib_type_sigs.void_by_val = RtTypeSig::new_by_val(RtElementType::Void);
    g_corlib_type_sigs.void_by_ref = RtTypeSig::new_by_ref(RtElementType::Void);
    g_corlib_type_sigs.boolean_by_val = RtTypeSig::new_by_val(RtElementType::Boolean);
    g_corlib_type_sigs.boolean_by_ref = RtTypeSig::new_by_ref(RtElementType::Boolean);
    g_corlib_type_sigs.char_by_val = RtTypeSig::new_by_val(RtElementType::Char);
    g_corlib_type_sigs.char_by_ref = RtTypeSig::new_by_ref(RtElementType::Char);
    g_corlib_type_sigs.i8_by_val = RtTypeSig::new_by_val(RtElementType::I1);
    g_corlib_type_sigs.i8_by_ref = RtTypeSig::new_by_ref(RtElementType::I1);
    g_corlib_type_sigs.u8_by_val = RtTypeSig::new_by_val(RtElementType::U1);
    g_corlib_type_sigs.u8_by_ref = RtTypeSig::new_by_ref(RtElementType::U1);
    g_corlib_type_sigs.i16_by_val = RtTypeSig::new_by_val(RtElementType::I2);
    g_corlib_type_sigs.i16_by_ref = RtTypeSig::new_by_ref(RtElementType::I2);
    g_corlib_type_sigs.u16_by_val = RtTypeSig::new_by_val(RtElementType::U2);
    g_corlib_type_sigs.u16_by_ref = RtTypeSig::new_by_ref(RtElementType::U2);
    g_corlib_type_sigs.i32_by_val = RtTypeSig::new_by_val(RtElementType::I4);
    g_corlib_type_sigs.i32_by_ref = RtTypeSig::new_by_ref(RtElementType::I4);
    g_corlib_type_sigs.u32_by_val = RtTypeSig::new_by_val(RtElementType::U4);
    g_corlib_type_sigs.u32_by_ref = RtTypeSig::new_by_ref(RtElementType::U4);
    g_corlib_type_sigs.i64_by_val = RtTypeSig::new_by_val(RtElementType::I8);
    g_corlib_type_sigs.i64_by_ref = RtTypeSig::new_by_ref(RtElementType::I8);
    g_corlib_type_sigs.u64_by_val = RtTypeSig::new_by_val(RtElementType::U8);
    g_corlib_type_sigs.u64_by_ref = RtTypeSig::new_by_ref(RtElementType::U8);
    g_corlib_type_sigs.intptr_by_val = RtTypeSig::new_by_val(RtElementType::I);
    g_corlib_type_sigs.intptr_by_ref = RtTypeSig::new_by_ref(RtElementType::I);
    g_corlib_type_sigs.uintptr_by_val = RtTypeSig::new_by_val(RtElementType::U);
    g_corlib_type_sigs.uintptr_by_ref = RtTypeSig::new_by_ref(RtElementType::U);
    g_corlib_type_sigs.f32_by_val = RtTypeSig::new_by_val(RtElementType::R4);
    g_corlib_type_sigs.f32_by_ref = RtTypeSig::new_by_ref(RtElementType::R4);
    g_corlib_type_sigs.f64_by_val = RtTypeSig::new_by_val(RtElementType::R8);
    g_corlib_type_sigs.f64_by_ref = RtTypeSig::new_by_ref(RtElementType::R8);
    g_corlib_type_sigs.object_by_val = RtTypeSig::new_by_val(RtElementType::Object);
    g_corlib_type_sigs.object_by_ref = RtTypeSig::new_by_ref(RtElementType::Object);
    g_corlib_type_sigs.string_by_val = RtTypeSig::new_by_val(RtElementType::String);
    g_corlib_type_sigs.string_by_ref = RtTypeSig::new_by_ref(RtElementType::String);
    g_corlib_type_sigs.typedreference_by_val = RtTypeSig::new_by_val(RtElementType::TypedByRef);
    g_corlib_type_sigs.typedreference_by_ref = RtTypeSig::new_by_ref(RtElementType::TypedByRef);
    g_corlib_type_sigs.class_by_val = RtTypeSig::new_by_val(RtElementType::Class);
    g_corlib_type_sigs.class_by_ref = RtTypeSig::new_by_ref(RtElementType::Class);
    g_corlib_type_sigs.valuetype_by_val = RtTypeSig::new_by_val(RtElementType::ValueType);
    g_corlib_type_sigs.valuetype_by_ref = RtTypeSig::new_by_ref(RtElementType::ValueType);
}

const RtGenericParam* MetadataCache::get_unspecific_generic_param(uint32_t index, bool is_method)
{
    assert(index < RT_MAX_GENERIC_PARAM_COUNT);
    if (is_method)
        return &g_methodGenericParams[index];
    else
        return &g_classGenericParams[index];
}

static const RtTypeSig* dup_typesig(const RtTypeSig& typesig)
{
    RtTypeSig* dup = alloc::MetadataAllocation::malloc_any<RtTypeSig>();
    *dup = typesig;
    return dup;
}

RtResult<const RtTypeSig*> MetadataCache::get_pooled_typesig(const RtTypeSig& typesig)
{
    // Canonized paths return shared pooled instances; otherwise duplicate
    if (!typesig.is_canonized())
    {
        RET_OK(dup_typesig(typesig));
    }

    bool byRef = (typesig.by_ref != 0);
    switch (typesig.ele_type)
    {
    case RtElementType::Void:
        RET_OK(byRef ? &g_corlib_type_sigs.void_by_ref : &g_corlib_type_sigs.void_by_val);
    case RtElementType::Boolean:
        RET_OK(byRef ? &g_corlib_type_sigs.boolean_by_ref : &g_corlib_type_sigs.boolean_by_val);
    case RtElementType::Char:
        RET_OK(byRef ? &g_corlib_type_sigs.char_by_ref : &g_corlib_type_sigs.char_by_val);
    case RtElementType::I1:
        RET_OK(byRef ? &g_corlib_type_sigs.i8_by_ref : &g_corlib_type_sigs.i8_by_val);
    case RtElementType::U1:
        RET_OK(byRef ? &g_corlib_type_sigs.u8_by_ref : &g_corlib_type_sigs.u8_by_val);
    case RtElementType::I2:
        RET_OK(byRef ? &g_corlib_type_sigs.i16_by_ref : &g_corlib_type_sigs.i16_by_val);
    case RtElementType::U2:
        RET_OK(byRef ? &g_corlib_type_sigs.u16_by_ref : &g_corlib_type_sigs.u16_by_val);
    case RtElementType::I4:
        RET_OK(byRef ? &g_corlib_type_sigs.i32_by_ref : &g_corlib_type_sigs.i32_by_val);
    case RtElementType::U4:
        RET_OK(byRef ? &g_corlib_type_sigs.u32_by_ref : &g_corlib_type_sigs.u32_by_val);
    case RtElementType::I8:
        RET_OK(byRef ? &g_corlib_type_sigs.i64_by_ref : &g_corlib_type_sigs.i64_by_val);
    case RtElementType::U8:
        RET_OK(byRef ? &g_corlib_type_sigs.u64_by_ref : &g_corlib_type_sigs.u64_by_val);
    case RtElementType::R4:
        RET_OK(byRef ? &g_corlib_type_sigs.f32_by_ref : &g_corlib_type_sigs.f32_by_val);
    case RtElementType::R8:
        RET_OK(byRef ? &g_corlib_type_sigs.f64_by_ref : &g_corlib_type_sigs.f64_by_val);
    case RtElementType::String:
        RET_OK(byRef ? &g_corlib_type_sigs.string_by_ref : &g_corlib_type_sigs.string_by_val);
    case RtElementType::Object:
        RET_OK(byRef ? &g_corlib_type_sigs.object_by_ref : &g_corlib_type_sigs.object_by_val);
    case RtElementType::TypedByRef:
        RET_OK(byRef ? &g_corlib_type_sigs.typedreference_by_ref : &g_corlib_type_sigs.typedreference_by_val);
    case RtElementType::I:
        RET_OK(byRef ? &g_corlib_type_sigs.intptr_by_ref : &g_corlib_type_sigs.intptr_by_val);
    case RtElementType::U:
        RET_OK(byRef ? &g_corlib_type_sigs.uintptr_by_ref : &g_corlib_type_sigs.uintptr_by_val);

    case RtElementType::Ptr:
        return get_pooled_ptr_typesig_by_element_typesig(typesig.data.element_type, byRef);

    case RtElementType::SZArray:
        return get_pooled_szarray_typesig_by_element_typesig(typesig.data.element_type, byRef);

    case RtElementType::Array:
    {
        const RtArrayType* arr = typesig.data.array_type;
        if (arr->num_sizes == 0 && arr->num_bounds == 0)
        {
            return get_pooled_array_typesig_by_element_typesig(arr->ele_type, arr->rank, byRef);
        }
        RET_OK(dup_typesig(typesig));
    }

    case RtElementType::GenericInst:
    {
        const RtGenericClass* gc = typesig.data.generic_class;
        RET_OK(byRef ? &gc->by_ref_type_sig : &gc->by_val_type_sig);
    }

    case RtElementType::Class:
    case RtElementType::ValueType:
    {
        uint32_t gid = typesig.data.type_def_gid;
        uint32_t modId = RtMetadata::decode_module_id_from_gid(gid);
        uint32_t rid = RtMetadata::decode_rid_from_gid(gid);
        RtModuleDef* mod = RtModuleDef::get_module_by_id(modId);
        if (mod)
        {
            return byRef ? mod->get_type_def_by_ref_typesig(rid) : mod->get_type_def_by_val_typesig(rid);
        }

        // in customattributes, type def gid may be 0
        assert(gid == 0);
        if (typesig.ele_type == RtElementType::Class)
            RET_OK(byRef ? &g_corlib_type_sigs.class_by_ref : &g_corlib_type_sigs.class_by_val);
        else
            RET_OK(byRef ? &g_corlib_type_sigs.valuetype_by_ref : &g_corlib_type_sigs.valuetype_by_val);
    }

    case RtElementType::Var:
    case RtElementType::MVar:
    {
        const RtGenericParam* gp = typesig.data.generic_param;
        uint32_t idx = gp->index;
        if (idx >= RT_MAX_GENERIC_PARAM_COUNT)
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        uint32_t gid = gp->gid;
        uint32_t modId = RtMetadata::decode_module_id_from_gid(gid);
        uint32_t rid = RtMetadata::decode_rid_from_gid(gid);
        if (modId == 0)
        {
            if (typesig.ele_type == RtElementType::MVar)
                RET_OK(byRef ? &g_mvarByRefs[idx] : &g_mvarByVals[idx]);
            else
                RET_OK(byRef ? &g_varByRefs[idx] : &g_varByVals[idx]);
        }
        RtModuleDef* mod = RtModuleDef::get_module_by_id(modId);
        if (mod)
        {
            return mod->get_generic_param_typesig_by_rid(rid, byRef);
        }
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }

    case RtElementType::FnPtr:
    {
        RET_OK(dup_typesig(typesig));
    }

    default:
    {
        assert(false && "Unhandled element type in get_pooled_typesig");
        RET_ASSERT_ERR(RtErr::BadImageFormat);
        // RtTypeSig* dup = alloc::MetadataAllocation::malloc_any_zeroed<RtTypeSig>();
        // *dup = typesig;
        // RET_OK(dup);
    }
    }
}

RtResult<const RtGenericInst*> MetadataCache::get_pooled_generic_inst(const RtTypeSig* const* genericArgs, uint8_t genericArgCount)
{
    if (!genericArgs || genericArgCount == 0)
        RET_ASSERT_ERR(RtErr::BadImageFormat);

    RtGenericInst key{genericArgs, genericArgCount};

    auto it = g_genericInstCache.find(&key);
    if (it != g_genericInstCache.end())
        RET_OK(*it);

    // Allocate new generic instance
    const RtTypeSig** new_args = alloc::MetadataAllocation::calloc_any<const RtTypeSig*>(genericArgCount);
    for (uint8_t i = 0; i < genericArgCount; ++i)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(new_args[i], get_pooled_typesig(genericArgs[i]->to_canonized_without_byref()));
    }

    RtGenericInst* new_gi = alloc::MetadataAllocation::malloc_any_zeroed<RtGenericInst>();
    new_gi->generic_args = new_args;
    new_gi->generic_arg_count = genericArgCount;

    g_genericInstCache.insert(new_gi);

    RET_OK(new_gi);
}

const RtGenericClass* MetadataCache::get_pooled_generic_class(uint32_t baseTypeDefGid, const RtGenericInst* classInst)
{
    RtGenericClass key{baseTypeDefGid, classInst};

    auto it = g_genericClassCache.find(&key);
    if (it != g_genericClassCache.end())
        return *it;

    // Allocate new generic class
    RtGenericClass* new_gc = alloc::MetadataAllocation::malloc_any_zeroed<RtGenericClass>();
    new_gc->base_type_def_gid = baseTypeDefGid;
    new_gc->class_inst = classInst;

    // Initialize by_val and by_ref type signatures
    new_gc->by_val_type_sig.ele_type = RtElementType::GenericInst;
    new_gc->by_val_type_sig.data.generic_class = new_gc;
    new_gc->by_val_type_sig.by_ref = 0;

    new_gc->by_ref_type_sig.ele_type = RtElementType::GenericInst;
    new_gc->by_ref_type_sig.data.generic_class = new_gc;
    new_gc->by_ref_type_sig.by_ref = 1;

    g_genericClassCache.insert(new_gc);

    return new_gc;
}

const RtGenericMethod* MetadataCache::get_pooled_generic_method(uint32_t methodDefGid, const RtGenericInst* classInst, const RtGenericInst* methodInst)
{
    RtGenericMethod key{methodDefGid, {classInst, methodInst}};
    auto it = g_genericMethodCache.find(&key);
    if (it != g_genericMethodCache.end())
        return *it;

    RtGenericMethod* gm = alloc::MetadataAllocation::malloc_any_zeroed<RtGenericMethod>();
    gm->base_method_gid = methodDefGid;
    gm->generic_context = RtGenericContext{classInst, methodInst};
    g_genericMethodCache.insert(gm);
    return gm;
}

RtResult<RtTypeSigByValRef> MetadataCache::get_pooled_ptr_typesigs_by_element_typesig(const RtTypeSig* eleType)
{
    auto key = eleType;
    auto it = g_ptrTypesigCache.find(key);
    if (it != g_ptrTypesigCache.end())
    {
        RtTypeSigByValRef ret{it->second.by_val, it->second.by_ref};
        RET_OK(ret);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtTypeSig*, canonEle, get_pooled_typesig(eleType->to_canonized_without_byref()));

    RtTypeSig* byVal = alloc::MetadataAllocation::malloc_any_zeroed<RtTypeSig>();
    byVal->ele_type = RtElementType::Ptr;
    byVal->data.element_type = canonEle;
    byVal->by_ref = 0;

    RtTypeSig* byRef = alloc::MetadataAllocation::malloc_any_zeroed<RtTypeSig>();
    *byRef = *byVal;
    byRef->by_ref = 1;

    RtTypeSigByValRef ret{byVal, byRef};
    g_ptrTypesigCache.insert({key, ret});
    RET_OK(ret);
}

RtResult<const RtTypeSig*> MetadataCache::get_pooled_ptr_typesig_by_element_typesig(const RtTypeSig* eleType, bool byRef)
{
    auto res = get_pooled_ptr_typesigs_by_element_typesig(eleType);
    if (res.is_err())
        RET_ERR(res.unwrap_err());
    RtTypeSigByValRef pair = res.unwrap();
    RET_OK(byRef ? pair.by_ref : pair.by_val);
}

RtResult<RtTypeSigByValRef> MetadataCache::get_pooled_szarray_typesigs_by_element_typesig(const RtTypeSig* eleType)
{
    const RtTypeSig* key = eleType;
    auto it = g_szarrayTypesigCache.find(key);
    if (it != g_szarrayTypesigCache.end())
    {
        RET_OK(it->second);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtTypeSig*, canonEle, get_pooled_typesig(eleType->to_canonized_without_byref()));

    RtTypeSig* byVal = alloc::MetadataAllocation::malloc_any_zeroed<RtTypeSig>();
    byVal->ele_type = RtElementType::SZArray;
    byVal->data.element_type = canonEle;
    byVal->by_ref = 0;

    RtTypeSig* byRef = alloc::MetadataAllocation::malloc_any_zeroed<RtTypeSig>();
    *byRef = *byVal;
    byRef->by_ref = 1;

    RtTypeSigByValRef ret{byVal, byRef};
    g_szarrayTypesigCache.insert({key, ret});
    RET_OK(ret);
}

RtResult<const RtTypeSig*> MetadataCache::get_pooled_szarray_typesig_by_element_typesig(const RtTypeSig* eleType, bool byRef)
{
    auto res = get_pooled_szarray_typesigs_by_element_typesig(eleType);
    if (res.is_err())
        RET_ERR(res.unwrap_err());
    RtTypeSigByValRef pair = res.unwrap();
    RET_OK(byRef ? pair.by_ref : pair.by_val);
}

static RtResult<RtTypeSigByValRef> get_pooled_array_type_info(const RtTypeSig* eleTypeSig, uint8_t rank)
{
    auto key = ArrayTypeSigKey{eleTypeSig, rank};
    auto it = g_arrayTypesigCache.find(key);
    if (it != g_arrayTypesigCache.end())
        return it->second;

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtTypeSig*, canonEle, MetadataCache::get_pooled_typesig(eleTypeSig->to_canonized_without_byref()));
    RtArrayType* array_type = alloc::MetadataAllocation::malloc_any_zeroed<RtArrayType>();
    array_type->ele_type = canonEle;
    array_type->rank = rank;

    RtTypeSig* byVal = alloc::MetadataAllocation::malloc_any_zeroed<RtTypeSig>();
    byVal->ele_type = RtElementType::Array;
    byVal->data.array_type = array_type;

    RtTypeSig* byRef = alloc::MetadataAllocation::malloc_any_zeroed<RtTypeSig>();
    *byRef = *byVal;
    byRef->by_ref = 1;

    auto value = RtTypeSigByValRef{byVal, byRef};
    g_arrayTypesigCache.insert({key, value});
    return value;
}

RtResult<const RtArrayType*> MetadataCache::get_pooled_array_type(const RtTypeSig* eleTypeSig, uint8_t rank)
{
    auto res = get_pooled_array_type_info(eleTypeSig, rank);
    if (res.is_err())
        RET_ERR(res.unwrap_err());
    RtTypeSigByValRef pair = res.unwrap();
    RET_OK(pair.by_val->data.array_type);
}

RtResult<RtTypeSigByValRef> MetadataCache::get_pooled_array_typesigs_by_element_typesig(const RtTypeSig* eleType, uint8_t rank)
{
    return get_pooled_array_type_info(eleType, rank);
}

RtResult<const RtTypeSig*> MetadataCache::get_pooled_array_typesig_by_element_typesig(const RtTypeSig* eleType, uint8_t rank, bool byRef)
{
    auto res = get_pooled_array_typesigs_by_element_typesig(eleType, rank);
    if (res.is_err())
        RET_ERR(res.unwrap_err());
    RtTypeSigByValRef pair = res.unwrap();
    RET_OK(byRef ? pair.by_ref : pair.by_val);
}

void MetadataCache::walk_generic_classes(ClassWalkCallback callback, void* userData)
{
    for (auto& entry : g_genericClassCache)
    {
        callback(const_cast<RtClass*>(entry->cache_klass), userData);
    }
}

} // namespace metadata
} // namespace leanclr
