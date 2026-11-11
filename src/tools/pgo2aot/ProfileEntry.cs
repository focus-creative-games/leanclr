namespace Pgo2Aot
{
    public sealed class ReceiverType
    {
        public string Assembly { get; set; } = string.Empty;
        public string Type { get; set; } = string.Empty;
        public ulong Calls { get; set; }
    }
    public sealed class InterfaceSite
    {
        public uint IlOffset { get; set; }
        public ulong Calls { get; set; }
        public ulong OtherCalls { get; set; }
        public ulong CacheObservations { get; set; }
        public ulong OneEntryHits { get; set; }
        public ulong TwoEntryHits { get; set; }
        public ReceiverType[] Types { get; set; } = System.Array.Empty<ReceiverType>();
    }
    public sealed class ProfileEntry
    {
        public string Assembly { get; set; } = string.Empty;

        public string Signature { get; set; } = string.Empty;

        public ulong Calls { get; set; }

        public ulong Cost { get; set; }
        public uint IlBytes { get; set; }
        public string IlHash { get; set; } = string.Empty;
        public InterfaceSite[] InterfaceSites { get; set; } = System.Array.Empty<InterfaceSite>();
        public ulong ReceiverProfileDropped { get; set; }

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
