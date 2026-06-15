namespace Pgo2Aot;

public sealed class ProfileEntry
{
    public required string Assembly { get; init; }

    public required string Signature { get; init; }

    public ulong Calls { get; init; }

    public ulong Cost { get; init; }

    public ulong GetMetric(SortMetric sortBy) => sortBy == SortMetric.Calls ? Calls : Cost;
}

public enum SortMetric
{
    Cost,
    Calls,
}

public enum SelectionStrategyKind
{
    Threshold,
    Top,
    Pareto,
}
