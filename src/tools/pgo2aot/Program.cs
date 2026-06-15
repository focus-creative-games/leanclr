using System;
using System.Collections.Generic;
using System.IO;
using CommandLine;

namespace Pgo2Aot
{
    internal sealed class Program
    {
        private static int Main(string[] args) =>
            Parser.Default.ParseArguments<ConvertOptions>(args)
                .MapResult(RunConvert, _ => 1);

        private static int RunConvert(ConvertOptions options)
        {
            try
            {
                IReadOnlyList<ProfileEntry> sourceEntries = ProfileJsonReader.LoadAndMerge(options.Input);
                IReadOnlyList<ProfileEntry> selectedEntries = SelectEntries(sourceEntries, options);
                AotXmlWriter.Write(Path.GetFullPath(options.Output), selectedEntries);

                Console.WriteLine(
                    $"Selected {selectedEntries.Count} method(s) from {sourceEntries.Count} profile record(s) -> {options.Output}");
                return 0;
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine($"Error: {ex.Message}");
                return 1;
            }
        }

        private static IReadOnlyList<ProfileEntry> SelectEntries(IReadOnlyList<ProfileEntry> entries, ConvertOptions options)
        {
            SelectionStrategyKind strategy = ParseStrategy(options.Strategy);
            SortMetric sortBy = ParseSortMetric(options.SortBy);

            return strategy switch
            {
                SelectionStrategyKind.Threshold => ProfileSelection.ApplyThreshold(
                    entries,
                    options.MinCalls,
                    options.MinCost,
                    sortBy),
                SelectionStrategyKind.Top => ProfileSelection.ApplyTop(
                    entries,
                    options.TopN,
                    options.TopPercent,
                    sortBy),
                SelectionStrategyKind.Pareto => ProfileSelection.ApplyPareto(
                    entries,
                    options.ParetoRatio,
                    sortBy),
                _ => throw new InvalidOperationException($"Unsupported strategy: {options.Strategy}"),
            };
        }

        private static SelectionStrategyKind ParseStrategy(string strategy)
        {
            return strategy.Trim().ToLowerInvariant() switch
            {
                "threshold" => SelectionStrategyKind.Threshold,
                "top" => SelectionStrategyKind.Top,
                "pareto" => SelectionStrategyKind.Pareto,
                _ => throw new ArgumentException("strategy must be one of: threshold, top, pareto.", nameof(strategy)),
            };
        }

        private static SortMetric ParseSortMetric(string sortBy)
        {
            return sortBy.Trim().ToLowerInvariant() switch
            {
                "cost" => SortMetric.Cost,
                "calls" => SortMetric.Calls,
                _ => throw new ArgumentException("sort-by must be one of: cost, calls.", nameof(sortBy)),
            };
        }
    }

    [Verb("convert", isDefault: true, HelpText = "Convert LeanCLR profile JSON to LeanAOT PGO rule XML.")]
    internal sealed class ConvertOptions
    {
        [Option('i', "input", Required = true, HelpText = "Input profile JSON file. Repeat for multiple files to merge.")]
        public IEnumerable<string> Input { get; set; } = Array.Empty<string>();

        [Option('o', "output", Required = true, HelpText = "Output pgo-aot.xml path.")]
        public string Output { get; set; } = string.Empty;

        [Option("strategy", Required = true, HelpText = "Selection strategy: threshold | top | pareto.")]
        public string Strategy { get; set; } = string.Empty;

        [Option("min-calls", Default = (ulong)1, HelpText = "threshold: minimum call count.")]
        public ulong MinCalls { get; set; }

        [Option("min-cost", Default = (ulong)0, HelpText = "threshold: minimum weighted cost.")]
        public ulong MinCost { get; set; }

        [Option("top-n", HelpText = "top: keep the first N methods after sorting.")]
        public int? TopN { get; set; }

        [Option("top-percent", HelpText = "top: keep the first N percent of methods after sorting, e.g. 10 for top 10%.")]
        public double? TopPercent { get; set; }

        [Option("pareto-ratio", Default = 0.8, HelpText = "pareto: cumulative metric ratio to keep, e.g. 0.8 for 80%.")]
        public double ParetoRatio { get; set; }

        [Option("sort-by", Default = "cost", HelpText = "top/pareto ranking metric: cost | calls.")]
        public string SortBy { get; set; } = "cost";
    }
}
