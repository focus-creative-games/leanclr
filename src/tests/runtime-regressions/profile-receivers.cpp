#include "profile/profile.h"
#include <cstdio>
#include <cstdlib>

using namespace leanclr;

static void check(bool value, const char* message)
{
    if (!value)
    {
        std::fprintf(stderr, "%s\n", message);
        std::exit(1);
    }
}

int main()
{
    static metadata::RtMethodInfo methods[512] = {};
    static metadata::RtClass types[5] = {};
    profile::Profile::reset_global();
    profile::Profile::set_enabled(true);
    for (unsigned i = 0; i < 1000; ++i)
        profile::Profile::observe_interface_receiver(methods, 7, types);
    profile::InterfaceProfileEntry entry = {};
    check(profile::Profile::get_interface_entries(&entry, 1, false) == 1 && entry.receiver_calls[0] == 1000 && entry.il_offset == 7,
          "receiver hits must all be counted");
    profile::Profile::end_period();
    check(profile::Profile::get_interface_entries(nullptr, 0, false) == 0, "end period clears active counts");
    check(profile::Profile::get_interface_entries(&entry, 1, true) == 1 && entry.receiver_calls[0] == 1000, "end period retains global counts");
    for (auto& type : types)
        profile::Profile::observe_interface_receiver(methods, 7, &type);
    check(profile::Profile::get_interface_entries(&entry, 1, false) == 1 && entry.other_calls == 1 && entry.receiver_calls[0] == 1,
          "fifth receiver enters conservative overflow count");
    profile::Profile::reset_period();
    profile::Profile::observe_interface_receiver(methods, 7, types + 4);
    check(profile::Profile::get_interface_entries(&entry, 1, false) == 1 && entry.receiver_types[0] == types + 4 && entry.receiver_calls[0] == 1,
          "new period captures fresh receiver identities");
    profile::Profile::reset_global();
    check(profile::Profile::get_interface_entries(nullptr, 0, true) == 0, "global reset clears keys and counts");
    profile::Profile::observe_interface_receiver(methods, 7, types, 1);
    profile::Profile::observe_interface_receiver(methods, 7, types, 1);
    profile::Profile::observe_interface_receiver(methods, 7, types + 1, 1);
    profile::Profile::observe_interface_receiver(methods, 7, types, 1);
    check(profile::Profile::get_interface_entries(&entry, 1, false) == 1 && entry.cache_observations == 4 && entry.one_entry_hits == 1 &&
              entry.two_entry_hits == 2,
          "shadow policies match single and round-robin double entry reuse");
    profile::Profile::observe_interface_receiver(methods, 7, types, 2);
    profile::Profile::observe_interface_receiver(methods, 7, types, 1);
    check(profile::Profile::get_interface_entries(&entry, 1, false) == 1 && entry.one_entry_hits == 1 && entry.two_entry_hits == 2,
          "new and reentered invocations conservatively reset history");
    profile::Profile::reset_global();
    profile::Profile::set_enabled(false);
    profile::Profile::observe_interface_receiver(methods, 7, types);
    check(profile::Profile::get_interface_entries(nullptr, 0, true) == 0, "disabled observer is inert");
    profile::Profile::set_enabled(true);
    profile::Profile::observe_interface_receiver(nullptr, 7, types);
    profile::Profile::observe_interface_receiver(methods, 7, nullptr);
    for (unsigned i = 0; i < 512; ++i)
        profile::Profile::observe_interface_receiver(methods + i, i, types);
    check(profile::Profile::get_interface_entries(nullptr, 0, true) <= 256 && profile::Profile::get_dropped_interface_observations(true) > 0,
          "site table and probe work remain bounded");
    profile::Profile::reset_global();
    profile::Profile::inc_call_count(methods, UINT64_MAX);
    profile::Profile::inc_call_count(methods, 1);
    profile::Profile::add_cost(methods, UINT64_MAX);
    profile::Profile::add_cost(methods, 1);
    profile::ProfileEntry method = {};
    check(profile::Profile::get_period_entries(&method, 1) == 1 && method.calls == UINT64_MAX && method.cost == UINT64_MAX,
          "profile counters saturate instead of wrapping");
    std::puts("Bounded receiver profile checks passed");
}
