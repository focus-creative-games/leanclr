using System;
using System.Collections.Generic;
using System.IO;
using CommandLine;

namespace Pgo2Aot
{
    internal sealed class Program
    {
        private static int Main(string[] args)
        {
            using (var parser = new Parser(settings => settings.AllowMultiInstance = true))
                return parser.ParseArguments<ConvertOptions>(args).MapResult(RunConvert, _ => 1);
        }

        private static int RunConvert(ConvertOptions options)
        {
            try
            {
                IReadOnlyList<ProfileEntry> sourceEntries = ProfileJsonReader.LoadAndMerge(options.Input);
                IReadOnlyList<ProfileEntry> selectedEntries = SelectEntries(sourceEntries, options);
                selectedEntries = ReceiverPolicy.Budget(selectedEntries, options.MaxMethods, options.MaxIlBytes);
                AotXmlWriter.Write(Path.GetFullPath(options.Output), selectedEntries, options.MinSiteCalls);

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
        [Option("max-methods", HelpText = "Cap selected PGO roots after ranking; does not cap explicit roots or generic closure.")]
        public int? MaxMethods { get; set; }
        [Option("max-il-bytes", HelpText = "Cap summed IL bytes of selected PGO roots; unknown IL sizes are skipped.")]
        public ulong? MaxIlBytes { get; set; }
        [Option("min-site-calls", Default = (ulong)64, HelpText = "Minimum receiver observations for an interface-cache hint.")]
        public ulong MinSiteCalls { get; set; }

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
