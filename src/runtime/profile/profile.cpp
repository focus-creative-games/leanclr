#include "profile.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "metadata/metadata_name.h"
#include "metadata/module_def.h"
#include "utils/string_builder.h"

namespace leanclr
{
namespace profile
{

#if LEANCLR_PGO_PROFILE
namespace
{
struct ExportCandidate
{
    const metadata::RtMethodInfo* method;
    const char* assembly;
    std::string signature;
    uint64_t calls;
    uint64_t cost;
};

bool g_enabled = true;
std::vector<ProfileSlot> g_slots;
std::vector<const metadata::RtMethodInfo*> g_slot_methods;

// Profiling-only, bounded storage. No allocation, locking or type-name work on
// the observed call path. This follows the runtime's single-thread contract.
struct ReceiverHistogram
{
    const metadata::RtClass* types[4];
    uint64_t calls[4];
    uint64_t other;
    uint64_t cache_observations;
    uint64_t one_entry_hits;
    uint64_t two_entry_hits;
};

struct ReceiverSlot
{
    const metadata::RtMethodInfo* method;
    uint32_t il_offset;
    ReceiverHistogram period;
    ReceiverHistogram global;
    uint64_t invocation;
    const metadata::RtClass* previous;
    const metadata::RtClass* recent[2];
    uint32_t next_victim;
};

ReceiverSlot g_receivers[256] = {};
uint64_t g_receiver_dropped_period = 0, g_receiver_dropped_global = 0;
uint64_t g_invocations = 0;
static_assert(sizeof(g_receivers) <= 64 * 1024, "Bound receiver profiling memory independently of the workload");

uint64_t saturated_add(uint64_t a, uint64_t b)
{
    return b > UINT64_MAX - a ? UINT64_MAX : a + b;
}

void observe(ReceiverHistogram& histogram, const metadata::RtClass* receiver, uint64_t calls)
{
    for (size_t i = 0; i < 4; ++i)
    {
        if (histogram.types[i] == receiver || histogram.types[i] == nullptr)
        {
            histogram.types[i] = receiver;
            histogram.calls[i] = saturated_add(histogram.calls[i], calls);
            return;
        }
    }
    histogram.other = saturated_add(histogram.other, calls);
}

ReceiverHistogram combined_histogram(const ReceiverSlot& slot)
{
    auto result = slot.global;
    for (size_t i = 0; i < 4; ++i)
        if (slot.period.types[i])
            observe(result, slot.period.types[i], slot.period.calls[i]);
    result.other = saturated_add(result.other, slot.period.other);
    result.cache_observations = saturated_add(result.cache_observations, slot.period.cache_observations);
    result.one_entry_hits = saturated_add(result.one_entry_hits, slot.period.one_entry_hits);
    result.two_entry_hits = saturated_add(result.two_entry_hits, slot.period.two_entry_hits);
    return result;
}

inline size_t to_slot_index(uint32_t profile_slot)
{
    return static_cast<size_t>(profile_slot - 1);
}

inline const char* get_assembly_name(const metadata::RtMethodInfo* method)
{
    if (method == nullptr || method->parent == nullptr || method->parent->image == nullptr)
    {
        return "Unknown";
    }
    const char* name = method->parent->image->get_name_no_ext();
    return (name && name[0] != '\0') ? name : "Unknown";
}

RtResult<std::string> build_method_signature(const metadata::RtMethodInfo* method)
{
    utils::Utf8StringBuilder sb;
    RET_ERR_ON_FAIL(metadata::MetadataName::append_type_full_name(sb, method->return_type, metadata::TypeNameFormat::DnlibToString, false));
    sb.append_char(' ');
    RET_ERR_ON_FAIL(metadata::MetadataName::append_method_full_name_with_params(sb, method, metadata::TypeNameFormat::DnlibToString));
    RET_OK(std::string(sb.get_const_chars(), sb.length()));
}

void append_json_escaped(std::string& out, const char* text, size_t length)
{
    if (text == nullptr)
    {
        return;
    }
    for (size_t index = 0; index < length; ++index)
    {
        const char c = text[index];
        switch (c)
        {
        case '"':
            out.append("\\\"");
            break;
        case '\\':
            out.append("\\\\");
            break;
        case '\b':
            out.append("\\b");
            break;
        case '\f':
            out.append("\\f");
            break;
        case '\n':
            out.append("\\n");
            break;
        case '\r':
            out.append("\\r");
            break;
        case '\t':
            out.append("\\t");
            break;
        default:
            if (static_cast<unsigned char>(c) < 0x20)
            {
                char escaped[7];
                std::snprintf(escaped, sizeof(escaped), "\\u%04x", static_cast<unsigned>(static_cast<unsigned char>(c)));
                out.append(escaped);
            }
            else
                out.push_back(c);
            break;
        }
    }
}

void append_json_escaped(std::string& out, const char* text)
{
    append_json_escaped(out, text, text ? std::strlen(text) : 0);
}

void append_json_escaped(std::string& out, const std::string& text)
{
    append_json_escaped(out, text.data(), text.size());
}

void append_interface_sites(std::string& json, const metadata::RtMethodInfo* method, bool global)
{
    json.append(",\"interfaceSites\":[");
    bool first_site = true;
    for (const auto& slot : g_receivers)
    {
        if (slot.method != method)
            continue;
        const auto histogram = global ? combined_histogram(slot) : slot.period;
        uint64_t total = histogram.other;
        for (auto count : histogram.calls)
            total = saturated_add(total, count);
        if (!total)
            continue;
        if (!first_site)
            json.push_back(',');
        first_site = false;
        json.append("{\"ilOffset\":").append(std::to_string(slot.il_offset));
        json.append(",\"calls\":").append(std::to_string(total));
        json.append(",\"otherCalls\":").append(std::to_string(histogram.other));
        json.append(",\"types\":[");
        bool first_type = true;
        for (size_t i = 0; i < 4; ++i)
        {
            if (!histogram.types[i] || !histogram.calls[i])
                continue;
            if (!first_type)
                json.push_back(',');
            first_type = false;
            const auto* klass = histogram.types[i];
            json.append("{\"assembly\":\"");
            append_json_escaped(json, klass->image ? klass->image->get_name_no_ext() : "Unknown");
            json.append("\",\"type\":\"");
            utils::Utf8StringBuilder name;
            auto named = metadata::MetadataName::append_type_full_name(name, klass->by_val, metadata::TypeNameFormat::DnlibToString, false);
            if (named.is_ok())
                append_json_escaped(json, name.get_const_chars(), name.length());
            json.append("\",\"calls\":").append(std::to_string(histogram.calls[i])).push_back('}');
        }
        json.append("],\"cacheObservations\":").append(std::to_string(histogram.cache_observations));
        json.append(",\"oneEntryHits\":").append(std::to_string(histogram.one_entry_hits));
        json.append(",\"twoEntryHits\":").append(std::to_string(histogram.two_entry_hits)).push_back('}');
    }
    json.push_back(']');
    json.append(",\"receiverProfileDropped\":").append(std::to_string(Profile::get_dropped_interface_observations(global)));
}

template <typename CallsGetter, typename CostGetter>
std::vector<ExportCandidate> collect_candidates(const ExportOptions& options, CallsGetter calls_getter, CostGetter cost_getter)
{
    std::vector<ExportCandidate> candidates;
    candidates.reserve(g_slots.size());
    for (size_t i = 0; i < g_slots.size(); ++i)
    {
        const ProfileSlot& slot = g_slots[i];
        uint64_t calls = calls_getter(slot);
        uint64_t cost = cost_getter(slot);
        if (calls < options.min_calls || cost < options.min_cost)
        {
            continue;
        }
        const metadata::RtMethodInfo* method = g_slot_methods[i];
        auto signature_result = build_method_signature(method);
        if (signature_result.is_err())
        {
            continue;
        }
        candidates.push_back(ExportCandidate{
            method,
            get_assembly_name(method),
            signature_result.unwrap(),
            calls,
            cost,
        });
    }

    std::sort(candidates.begin(), candidates.end(),
              [](const ExportCandidate& a, const ExportCandidate& b)
              {
                  if (a.cost != b.cost)
                  {
                      return a.cost > b.cost;
                  }
                  if (a.calls != b.calls)
                  {
                      return a.calls > b.calls;
                  }
                  const int assembly_cmp = std::strcmp(a.assembly, b.assembly);
                  if (assembly_cmp != 0)
                  {
                      return assembly_cmp < 0;
                  }
                  return a.signature < b.signature;
              });

    if (options.top_n > 0 && candidates.size() > options.top_n)
    {
        candidates.resize(options.top_n);
    }
    return candidates;
}

std::string build_candidates_json(const std::vector<ExportCandidate>& candidates, bool global)
{
    if (candidates.empty())
    {
        return "[]";
    }

    std::string json;
    json.append("[\n");
    for (size_t i = 0; i < candidates.size(); ++i)
    {
        if (i > 0)
        {
            json.append(",\n");
        }
        const ExportCandidate& candidate = candidates[i];
        json.append("{\"assembly\":\"");
        append_json_escaped(json, candidate.assembly);
        json.append("\",\"signature\":\"");
        append_json_escaped(json, candidate.signature);
        json.append("\",\"calls\":");
        json.append(std::to_string(candidate.calls));
        json.append(",\"cost\":");
        json.append(std::to_string(candidate.cost));
        uint32_t il_bytes = 0;
        uint64_t il_hash = 14695981039346656037ULL;
        auto body = candidate.method->parent->image->read_method_body(candidate.method->token);
        if (body.is_ok() && body.unwrap().has_value())
        {
            il_bytes = body.unwrap()->code_size;
            for (uint32_t byte = 0; byte < il_bytes; ++byte)
                il_hash = (il_hash ^ body.unwrap()->code[byte]) * 1099511628211ULL;
        }
        json.append(",\"ilBytes\":").append(std::to_string(il_bytes));
        char hash[17];
        std::snprintf(hash, sizeof(hash), "%016llX", static_cast<unsigned long long>(il_hash));
        json.append(",\"ilHash\":\"").append(il_bytes ? hash : "").push_back('"');
        append_interface_sites(json, candidate.method, global);
        json.push_back('}');
    }
    json.append("\n]");
    return json;
}

RtResultVoid write_json_to_file(const char* path, const std::string& json)
{
    FILE* fp = std::fopen(path, "wb");
    if (fp == nullptr)
    {
        RET_ERR(RtErr::FileNotFound);
    }
    const bool written = std::fwrite(json.data(), 1, json.size(), fp) == json.size();
    const int closed = std::fclose(fp);
    if (!written || closed != 0)
        return RtErr::InvalidOperation;
    RET_VOID_OK();
}
} // namespace
#endif

bool Profile::is_enabled() noexcept
{
#if LEANCLR_PGO_PROFILE
    return g_enabled;
#else
    return false;
#endif
}

void Profile::set_enabled(bool enabled) noexcept
{
#if LEANCLR_PGO_PROFILE
    g_enabled = enabled;
#else
    (void)enabled;
#endif
}

uint32_t Profile::ensure_slot(metadata::RtMethodInfo* method) noexcept
{
#if LEANCLR_PGO_PROFILE
    if (method == nullptr)
    {
        return 0;
    }

    if (method->profile_slot != 0)
    {
        return static_cast<uint32_t>(to_slot_index(method->profile_slot));
    }

    const uint32_t new_slot = static_cast<uint32_t>(g_slots.size() + 1);
    g_slots.push_back(ProfileSlot{0, 0, 0, 0});
    g_slot_methods.push_back(method);
    method->profile_slot = new_slot;
    return new_slot - 1;
#else
    (void)method;
    return 0;
#endif
}

void Profile::inc_call_count(const metadata::RtMethodInfo* method, uint64_t delta) noexcept
{
#if LEANCLR_PGO_PROFILE
    if (!g_enabled || method == nullptr)
    {
        return;
    }
    metadata::RtMethodInfo* mutable_method = const_cast<metadata::RtMethodInfo*>(method);
    uint32_t slot_idx = ensure_slot(mutable_method);
    g_slots[slot_idx].period_calls = saturated_add(g_slots[slot_idx].period_calls, delta);
#else
    (void)method;
    (void)delta;
#endif
}

void Profile::add_cost(const metadata::RtMethodInfo* method, uint64_t cost) noexcept
{
#if LEANCLR_PGO_PROFILE
    if (!g_enabled || method == nullptr || cost == 0)
    {
        return;
    }
    metadata::RtMethodInfo* mutable_method = const_cast<metadata::RtMethodInfo*>(method);
    uint32_t slot_idx = ensure_slot(mutable_method);
    g_slots[slot_idx].period_cost = saturated_add(g_slots[slot_idx].period_cost, cost);
#else
    (void)method;
    (void)cost;
#endif
}

uint64_t Profile::next_invocation_id() noexcept
{
#if LEANCLR_PGO_PROFILE
    if (g_enabled && g_invocations != UINT64_MAX)
        return ++g_invocations;
#endif
    return 0;
}

void Profile::observe_interface_receiver(const metadata::RtMethodInfo* caller, uint32_t il_offset, const metadata::RtClass* receiver,
                                         uint64_t invocation) noexcept
{
#if LEANCLR_PGO_PROFILE
    if (!g_enabled || !caller || !receiver)
        return;
    const size_t hash = ((reinterpret_cast<uintptr_t>(caller) >> 4) ^ (il_offset * 2654435761u)) & 255;
    for (size_t probe = 0; probe < 8; ++probe)
    {
        auto& slot = g_receivers[(hash + probe) & 255];
        if (!slot.method || (slot.method == caller && slot.il_offset == il_offset))
        {
            slot.method = caller;
            slot.il_offset = il_offset;
            observe(slot.period, receiver, 1);
            if (invocation)
            {
                if (slot.invocation != invocation)
                {
                    slot.invocation = invocation;
                    slot.previous = slot.recent[0] = slot.recent[1] = nullptr;
                    slot.next_victim = 0;
                }
                slot.period.cache_observations = saturated_add(slot.period.cache_observations, 1);
                if (slot.previous == receiver)
                    slot.period.one_entry_hits = saturated_add(slot.period.one_entry_hits, 1);
                slot.previous = receiver;
                if (slot.recent[0] == receiver || slot.recent[1] == receiver)
                    slot.period.two_entry_hits = saturated_add(slot.period.two_entry_hits, 1);
                else
                {
                    slot.recent[slot.next_victim] = receiver;
                    slot.next_victim ^= 1;
                }
            }
            return;
        }
    }
    g_receiver_dropped_period = saturated_add(g_receiver_dropped_period, 1);
#else
    (void)caller;
    (void)il_offset;
    (void)receiver;
    (void)invocation;
#endif
}

size_t Profile::get_interface_entries(InterfaceProfileEntry* out, size_t capacity, bool global) noexcept
{
#if LEANCLR_PGO_PROFILE
    size_t count = 0;
    for (const auto& slot : g_receivers)
    {
        if (!slot.method)
            continue;
        const auto histogram = global ? combined_histogram(slot) : slot.period;
        bool populated = histogram.other != 0;
        for (auto value : histogram.calls)
            populated |= value != 0;
        if (!populated)
            continue;
        if (out && count < capacity)
        {
            out[count].method = slot.method;
            out[count].il_offset = slot.il_offset;
            for (size_t i = 0; i < 4; ++i)
            {
                out[count].receiver_types[i] = histogram.types[i];
                out[count].receiver_calls[i] = histogram.calls[i];
            }
            out[count].other_calls = histogram.other;
            out[count].cache_observations = histogram.cache_observations;
            out[count].one_entry_hits = histogram.one_entry_hits;
            out[count].two_entry_hits = histogram.two_entry_hits;
        }
        ++count;
    }
    return count;
#else
    (void)out;
    (void)capacity;
    (void)global;
    return 0;
#endif
}

uint64_t Profile::get_dropped_interface_observations(bool global) noexcept
{
#if LEANCLR_PGO_PROFILE
    return global ? saturated_add(g_receiver_dropped_global, g_receiver_dropped_period) : g_receiver_dropped_period;
#else
    (void)global;
    return 0;
#endif
}

void Profile::end_period() noexcept
{
#if LEANCLR_PGO_PROFILE
    for (auto& slot : g_receivers)
    {
        slot.global = combined_histogram(slot);
        slot.period = {};
        slot.invocation = 0;
    }
    g_receiver_dropped_global = saturated_add(g_receiver_dropped_global, g_receiver_dropped_period);
    g_receiver_dropped_period = 0;
    for (ProfileSlot& slot : g_slots)
    {
        slot.global_calls = saturated_add(slot.global_calls, slot.period_calls);
        slot.global_cost = saturated_add(slot.global_cost, slot.period_cost);
        slot.period_calls = 0;
        slot.period_cost = 0;
    }
#endif
}

void Profile::reset_period() noexcept
{
#if LEANCLR_PGO_PROFILE
    for (auto& slot : g_receivers)
    {
        slot.period = {};
        slot.invocation = 0;
    }
    g_receiver_dropped_period = 0;
    for (ProfileSlot& slot : g_slots)
    {
        slot.period_calls = 0;
        slot.period_cost = 0;
    }
#endif
}

void Profile::reset_global() noexcept
{
#if LEANCLR_PGO_PROFILE
    for (auto& slot : g_receivers)
        slot = {};
    g_receiver_dropped_period = g_receiver_dropped_global = 0;
    for (ProfileSlot& slot : g_slots)
    {
        slot.period_calls = 0;
        slot.period_cost = 0;
        slot.global_calls = 0;
        slot.global_cost = 0;
    }
#endif
}

size_t Profile::get_period_entries(ProfileEntry* out, size_t capacity) noexcept
{
#if LEANCLR_PGO_PROFILE
    size_t count = 0;
    for (size_t i = 0; i < g_slots.size(); ++i)
    {
        const ProfileSlot& slot = g_slots[i];
        if (slot.period_calls == 0 && slot.period_cost == 0)
        {
            continue;
        }
        if (out != nullptr && count < capacity)
        {
            out[count] = ProfileEntry{g_slot_methods[i], slot.period_calls, slot.period_cost};
        }
        ++count;
    }
    return count;
#else
    (void)out;
    (void)capacity;
    return 0;
#endif
}

size_t Profile::get_global_entries(ProfileEntry* out, size_t capacity) noexcept
{
#if LEANCLR_PGO_PROFILE
    size_t count = 0;
    for (size_t i = 0; i < g_slots.size(); ++i)
    {
        const ProfileSlot& slot = g_slots[i];
        if (slot.global_calls == 0 && slot.global_cost == 0)
        {
            continue;
        }
        if (out != nullptr && count < capacity)
        {
            out[count] = ProfileEntry{g_slot_methods[i], slot.global_calls, slot.global_cost};
        }
        ++count;
    }
    return count;
#else
    (void)out;
    (void)capacity;
    return 0;
#endif
}

RtResult<std::string> Profile::get_period_stats_json(const ExportOptions& options) noexcept
{
#if LEANCLR_PGO_PROFILE
    const std::vector<ExportCandidate> candidates =
        collect_candidates(options, [](const ProfileSlot& slot) { return slot.period_calls; }, [](const ProfileSlot& slot) { return slot.period_cost; });
    RET_OK(build_candidates_json(candidates, false));
#else
    (void)options;
    RET_OK(std::string("[]"));
#endif
}

RtResult<std::string> Profile::get_global_stats_json(const ExportOptions& options) noexcept
{
#if LEANCLR_PGO_PROFILE
    const std::vector<ExportCandidate> candidates = collect_candidates(
        options, [](const ProfileSlot& slot) { return saturated_add(slot.global_calls, slot.period_calls); },
        [](const ProfileSlot& slot) { return saturated_add(slot.global_cost, slot.period_cost); });
    RET_OK(build_candidates_json(candidates, true));
#else
    (void)options;
    RET_OK(std::string("[]"));
#endif
}

RtResultVoid Profile::export_period_stats_json(const char* path, const ExportOptions& options) noexcept
{
#if LEANCLR_PGO_PROFILE
    if (path == nullptr || path[0] == '\0')
    {
        RET_ERR(RtErr::ArgumentNull);
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(std::string, json, get_period_stats_json(options));
    return write_json_to_file(path, json);
#else
    (void)path;
    (void)options;
    RET_VOID_OK();
#endif
}

RtResultVoid Profile::export_global_stats_json(const char* path, const ExportOptions& options) noexcept
{
#if LEANCLR_PGO_PROFILE
    if (path == nullptr || path[0] == '\0')
    {
        RET_ERR(RtErr::ArgumentNull);
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(std::string, json, get_global_stats_json(options));
    return write_json_to_file(path, json);
#else
    (void)path;
    (void)options;
    RET_VOID_OK();
#endif
}

} // namespace profile
} // namespace leanclr
