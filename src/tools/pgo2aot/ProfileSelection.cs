namespace Pgo2Aot;

public static class ProfileSelection
{
    public static IReadOnlyList<ProfileEntry> ApplyThreshold(
        IReadOnlyList<ProfileEntry> entries,
        ulong minCalls,
        ulong minCost,
        SortMetric sortBy)
    {
        return SortEntries(
                entries.Where(entry => entry.Calls >= minCalls && entry.Cost >= minCost),
                sortBy)
            .ToList();
    }

    public static IReadOnlyList<ProfileEntry> ApplyTop(
        IReadOnlyList<ProfileEntry> entries,
        int? topN,
        double? topPercent,
        SortMetric sortBy)
    {
        if (entries.Count == 0)
        {
            return Array.Empty<ProfileEntry>();
        }

        List<ProfileEntry> sorted = SortEntries(entries, sortBy);
        int takeCount;
        if (topN.HasValue)
        {
            if (topN.Value <= 0)
            {
                throw new ArgumentOutOfRangeException(nameof(topN), "top-n must be greater than 0.");
            }

            takeCount = Math.Min(topN.Value, sorted.Count);
        }
        else if (topPercent.HasValue)
        {
            if (topPercent.Value <= 0 || topPercent.Value > 100)
            {
                throw new ArgumentOutOfRangeException(nameof(topPercent), "top-percent must be in (0, 100].");
            }

            takeCount = (int)Math.Ceiling(sorted.Count * topPercent.Value / 100.0);
            takeCount = Math.Max(1, Math.Min(takeCount, sorted.Count));
        }
        else
        {
            throw new InvalidOperationException("Top strategy requires --top-n or --top-percent.");
        }

        return sorted.Take(takeCount).ToList();
    }

    public static IReadOnlyList<ProfileEntry> ApplyPareto(
        IReadOnlyList<ProfileEntry> entries,
        double paretoRatio,
        SortMetric sortBy)
    {
        if (entries.Count == 0)
        {
            return Array.Empty<ProfileEntry>();
        }

        if (paretoRatio <= 0 || paretoRatio > 1)
        {
            throw new ArgumentOutOfRangeException(nameof(paretoRatio), "pareto-ratio must be in (0, 1].");
        }

        List<ProfileEntry> sorted = SortEntries(entries, sortBy);
        ulong totalMetric = 0;
        foreach (ProfileEntry entry in sorted)
        {
            totalMetric += entry.GetMetric(sortBy);
        }

        if (totalMetric == 0)
        {
            return sorted.Take(1).ToList();
        }

        ulong target = (ulong)Math.Ceiling(totalMetric * paretoRatio);
        ulong accumulated = 0;
        var selected = new List<ProfileEntry>(sorted.Count);
        foreach (ProfileEntry entry in sorted)
        {
            selected.Add(entry);
            accumulated += entry.GetMetric(sortBy);
            if (accumulated >= target)
            {
                break;
            }
        }

        return selected;
    }

    private static List<ProfileEntry> SortEntries(IEnumerable<ProfileEntry> entries, SortMetric sortBy)
    {
        return entries
            .OrderByDescending(entry => entry.GetMetric(sortBy))
            .ThenByDescending(entry => sortBy == SortMetric.Cost ? entry.Calls : entry.Cost)
            .ThenBy(entry => entry.Assembly, StringComparer.OrdinalIgnoreCase)
            .ThenBy(entry => entry.Signature, StringComparer.Ordinal)
            .ToList();
    }
}
