#include "registry_checks.h"
#include "codegen/aot_result.h"
#include "metadata/aot_module.h"
#include "metadata/module_def.h"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

using namespace leanclr;
using namespace leanclr::metadata;
extern RtAotModulesData g_aot_modules_data;

// Model the previous method layout to ensure the bound return ABI uses padding
// rather than charging every method for a new cache pointer or extra word.
struct PreviousMethodLayout
{
    const void* prefix[7];
    RtManagedMethodPointer method, virtual_method;
    RtInvokeMethodPointer invoke;
    const void* interpreter;
    EncodedTokenId token;
    uint16_t descriptors[6];
    RtInvokerType invoker_type;
#if LEANCLR_PGO_PROFILE
    uint32_t profile_slot;
#endif
};

static_assert(sizeof(PreviousMethodLayout) == sizeof(RtMethodInfo), "bound ABI must not grow RtMethodInfo");
static_assert(sizeof(RtAotGenericMethodData) <= 72, "generic index entry unexpectedly grew");

static void require(bool condition, const char* name)
{
    if (!condition)
    {
        std::fprintf(stderr, "Registry check failed: %s\n", name);
        std::exit(1);
    }
}

static void install(const RtAotModulesData& data)
{
    auto result = AotModule::register_aot_modules(&data);
    require(result.is_ok(), "install index");
}

struct RestoreRegistry
{
    ~RestoreRegistry()
    {
        install(g_aot_modules_data);
    }
};

// Preserve the old structural linear-scan baseline locally, for scaling tests.
// The synthetic table represents one defining assembly, as the old table did.
static bool match_type(const RtAotTypeIdentity& id, const RtTypeSig* type);

static bool match_args(const RtAotTypeIdentity* ids, unsigned count, const RtGenericInst* args)
{
    if (count != (args ? args->generic_arg_count : 0))
        return false;
    for (unsigned i = 0; i < count; ++i)
        if (!match_type(ids[i], args->generic_args[i]))
            return false;
    return true;
}

static bool match_definition(const RtAotTypeIdentity& id, uint32_t gid)
{
    const auto* module = RtModuleDef::get_module_by_id(RtMetadata::decode_module_id_from_gid(gid));
    return module && id.module_name && std::strcmp(id.module_name, module->get_name_no_ext()) == 0 &&
           RtToken::decode_rid(id.type_token) == RtMetadata::decode_rid_from_gid(gid);
}

static bool match_type(const RtAotTypeIdentity& id, const RtTypeSig* type)
{
    if (!type || type->is_by_ref() || id.element_type != type->ele_type)
        return false;
    switch (type->ele_type)
    {
    case RtElementType::Class:
    case RtElementType::ValueType:
        return match_definition(id, type->data.type_def_gid);
    case RtElementType::GenericInst:
        return match_definition(id, type->data.generic_class->base_type_def_gid) &&
               match_args(id.arguments, id.argument_count, type->data.generic_class->class_inst);
    case RtElementType::SZArray:
        return id.argument_count == 1 && match_type(id.arguments[0], type->data.element_type);
    default:
        return id.argument_count == 0;
    }
}

static LEANCLR_CODEGEN_NOINLINE const RtAotGenericMethodData* linear(const RtAotGenericMethodData* entries, unsigned count, const RtMethodInfo* method)
{
    const auto& ctx = method->generic_method->generic_context;
    for (unsigned i = 0; i < count; ++i)
        if (entries[i].implementation.token == method->token && match_args(entries[i].class_arguments, entries[i].class_argument_count, ctx.class_inst) &&
            match_args(entries[i].method_arguments, entries[i].method_argument_count, ctx.method_inst))
            return entries + i;
    return nullptr;
}

static const RtMethodInfo* volatile s_query;
static volatile unsigned s_checksum;

static void measure(const RtAotModulesData& data, const RtMethodInfo* query, bool hit, bool indexed, unsigned iterations)
{
    s_query = query;
    unsigned found = 0;
    const auto start = std::chrono::steady_clock::now();
    for (unsigned i = 0; i < iterations; ++i)
    {
        const auto* result =
            indexed ? AotModule::find_aot_generic_method_impl(s_query) : linear(data.generic_method_entries, data.generic_method_entry_count, s_query);
        found += result != nullptr;
    }
    const auto end = std::chrono::steady_clock::now();
    s_checksum = found;
    require(found == (hit ? iterations : 0), "benchmark hit/miss checksum");
    const double elapsed = std::chrono::duration<double, std::nano>(end - start).count() / iterations;
    std::printf("{\"workload\":\"lookup-%s\",\"index\":\"%s\",\"entries\":%u,\"iterations\":%u,\"checksum\":%u,\"ns_per_lookup\":%.6f,\"entry_bytes\":%zu,"
                "\"extra_index_heap_bytes\":0}\n",
                hit ? "hit" : "miss", indexed ? "sorted-hash" : "linear", data.generic_method_entry_count, iterations, found, elapsed,
                sizeof(RtAotGenericMethodData) * data.generic_method_entry_count);
}

void verify_generic_registry(const RtMethodInfo* primitive, const RtMethodInfo* external, const RtMethodInfo* missing, bool benchmark, unsigned iterations)
{
    RestoreRegistry restore;
    auto data = g_aot_modules_data;
    data.generic_method_entries = nullptr;
    data.generic_method_entry_count = 1;
    auto invalid = AotModule::register_aot_modules(&data);
    require(invalid.is_err() && invalid.unwrap_err() == RtErr::BadImageFormat, "reject count without storage");
    data.generic_method_entry_count = 0;
    install(data);
    require(AotModule::find_aot_method_impl(primitive) == nullptr, "empty registry");
    install(g_aot_modules_data);
    const auto* prototype = AotModule::find_aot_generic_method_impl(primitive);
    if (!prototype)
    {
        require(!benchmark, "benchmark needs enabled AOT");
        return;
    }
    const auto* external_entry = AotModule::find_aot_generic_method_impl(external);
    require(external_entry && external_entry->method_argument_count == 1, "external collision prototype");
    RtAotGenericMethodData collision[2] = {*external_entry, *external_entry};
    RtAotTypeIdentity wrong_assembly = external_entry->method_arguments[0];
    wrong_assembly.module_name = "Common"; // Same token and hash, wrong argument assembly.
    collision[0].method_arguments = &wrong_assembly;
    data.generic_method_entries = collision;
    data.generic_method_entry_count = 2;
    install(data);
    require(AotModule::find_aot_generic_method_impl(external) == collision + 1, "hash collision must compare argument assembly");
    data.generic_method_entry_count = 1;
    install(data);
    require(AotModule::find_aot_generic_method_impl(external) == nullptr, "collision-only miss must not borrow implementation");
    install(g_aot_modules_data);
    collision[0] = collision[1] = *prototype;
    ++collision[0].key_hash;
    data.generic_method_entry_count = 2;
    auto unordered = AotModule::register_aot_modules(&data);
    require(unordered.is_err() && unordered.unwrap_err() == RtErr::BadImageFormat, "reject unordered index");
    require(AotModule::find_aot_generic_method_impl(primitive) == prototype, "failed registration must preserve registry");
    collision[0] = *prototype;
    collision[0].return_protocol = 2;
    data.generic_method_entry_count = 1;
    auto protocol = AotModule::register_aot_modules(&data);
    require(protocol.is_err() && protocol.unwrap_err() == RtErr::BadImageFormat, "reject unknown generic return ABI");
    require(missing->invoker_type == RtInvokerType::Interpreter, "unregistered method must stay interpreted");
    if (!benchmark)
        return;
    require(iterations > 0, "positive benchmark iterations");
    for (unsigned size : {64u, 1024u, 16384u})
    {
        std::vector<RtAotGenericMethodData> table(size, *prototype);
        for (unsigned i = 0; i < size; ++i)
        {
            table[i].key_hash = prototype->key_hash - size / 2 + i;
            ++table[i].implementation.token;
        }
        table[size / 2] = *prototype;
        data.generic_method_entries = table.data();
        data.generic_method_entry_count = size;
        install(data);
        // Hit and miss use opposite timing orders; the external runner
        // interleaves process samples. Volatile query loads prevent LICM hoisting.
        measure(data, primitive, true, false, iterations);
        measure(data, primitive, true, true, iterations);
        measure(data, missing, false, true, iterations);
        measure(data, missing, false, false, iterations);
        install(g_aot_modules_data); // Never leave the registry pointing at freed test data.
    }
}
