namespace Pgo2Aot
{
    public sealed class ProfileEntry
    {
        public string Assembly { get; set; } = string.Empty;

        public string Signature { get; set; } = string.Empty;

        public ulong Calls { get; set; }

        public ulong Cost { get; set; }

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
}
