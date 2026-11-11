using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace Pgo2Aot
{
    public static class ReceiverPolicy
    {
        public static void Validate(ProfileEntry entry)
        {
            entry.InterfaceSites ??= Array.Empty<InterfaceSite>();
            if (!string.IsNullOrEmpty(entry.IlHash) && (entry.IlHash.Length != 16 || !entry.IlHash.All(Uri.IsHexDigit)))
                throw new InvalidDataException("IL hash must be a 16-digit FNV1a64 fingerprint.");
            if (entry.InterfaceSites.Select(s => s.IlOffset).Distinct().Count() != entry.InterfaceSites.Length)
                throw new InvalidDataException("Duplicate interface callsite offset.");
            foreach (var site in entry.InterfaceSites)
            {
                site.Types ??= Array.Empty<ReceiverType>();
                decimal total = site.OtherCalls;
                var keys = new HashSet<string>(StringComparer.Ordinal);
                foreach (var type in site.Types)
                {
                    if (string.IsNullOrEmpty(type.Assembly) || string.IsNullOrEmpty(type.Type) || !keys.Add(type.Assembly + "\0" + type.Type))
                        throw new InvalidDataException("Receiver identity is missing or duplicated.");
                    total += type.Calls;
                }
                if (site.Types.Length > 4 || total != site.Calls || site.CacheObservations > site.Calls ||
                    site.OneEntryHits > site.TwoEntryHits || site.TwoEntryHits > site.CacheObservations ||
                    (entry.IlBytes != 0 && site.IlOffset >= entry.IlBytes))
                    throw new InvalidDataException("Invalid receiver counters, capacity or IL offset.");
            }
        }

        private static ulong Saturate(decimal value) => value >= ulong.MaxValue ? ulong.MaxValue : (ulong)value;

        // Preserve cumulative snapshot semantics: max per receiver, union of
        // identities, and conservatively retain only four receiver identities.
        public static InterfaceSite[] Merge(ProfileEntry first, ProfileEntry second)
        {
            if (string.IsNullOrEmpty(first.IlHash) || !string.Equals(first.IlHash, second.IlHash, StringComparison.OrdinalIgnoreCase))
                return Array.Empty<InterfaceSite>();
            return first.InterfaceSites.Concat(second.InterfaceSites).GroupBy(s => s.IlOffset).Select(group =>
            {
                var types = group.SelectMany(s => s.Types).GroupBy(t => t.Assembly + "\0" + t.Type, StringComparer.Ordinal)
                    .Select(g => new ReceiverType { Assembly = g.First().Assembly, Type = g.First().Type, Calls = g.Max(t => t.Calls) })
                    .OrderByDescending(t => t.Calls).ThenBy(t => t.Assembly, StringComparer.Ordinal).ThenBy(t => t.Type, StringComparer.Ordinal).ToArray();
                decimal other = group.Max(s => s.OtherCalls);
                foreach (var type in types.Skip(4)) other += type.Calls;
                var known = types.Take(4).ToArray();
                decimal calls = other;
                foreach (var type in known) calls += type.Calls;
                if (calls > ulong.MaxValue)
                    return new InterfaceSite { IlOffset = group.Key }; // Saturated captures cannot justify a capacity.
                var merged = new InterfaceSite { IlOffset = group.Key, Calls = Saturate(calls), OtherCalls = Saturate(other), Types = known };
                if (group.All(s => s.Calls > 0 && s.CacheObservations == s.Calls))
                {
                    // Use the worst captured reuse rate, avoiding optimistic
                    // hints when merging different workloads or snapshots.
                    merged.CacheObservations = merged.Calls;
                    merged.OneEntryHits = (ulong)decimal.Floor(calls * group.Min(s => (decimal)s.OneEntryHits / s.Calls));
                    merged.TwoEntryHits = (ulong)decimal.Floor(calls * group.Min(s => (decimal)s.TwoEntryHits / s.Calls));
                }
                return merged;
            }).OrderBy(s => s.IlOffset).ToArray();
        }

        public static int Capacity(InterfaceSite site, ulong minCalls = 64)
        {
            if (site.Calls < minCalls || site.Calls == 0) return 0;
            if (site.CacheObservations == site.Calls)
            {
                if ((decimal)site.OneEntryHits / site.Calls >= 0.90m) return 1;
                if ((decimal)site.TwoEntryHits / site.Calls >= 0.95m) return 2;
                return 0;
            }
            if (site.CacheObservations != 0) return 0; // Partial invocation evidence must not imply cache reuse.
            var types = site.Types.OrderByDescending(t => t.Calls).ToArray();
            if (types.Length > 0 && (decimal)types[0].Calls / site.Calls >= 0.90m) return 1;
            if (types.Length > 1 && ((decimal)types[0].Calls + types[1].Calls) / site.Calls >= 0.95m) return 2;
            return 0;
        }

        public static IReadOnlyList<ProfileEntry> Budget(IReadOnlyList<ProfileEntry> entries, int? maxMethods, ulong? maxIlBytes)
        {
            if (maxMethods <= 0 || maxIlBytes == 0) throw new ArgumentOutOfRangeException("Budgets must be positive.");
            var result = new List<ProfileEntry>();
            ulong used = 0;
            foreach (var entry in entries)
            {
                if (result.Count >= (maxMethods ?? int.MaxValue)) break;
                if (maxIlBytes.HasValue && (entry.IlBytes == 0 || entry.IlBytes > maxIlBytes.Value - used)) continue;
                result.Add(entry);
                if (maxIlBytes.HasValue) used += entry.IlBytes;
            }
            return result;
        }
    }
}
