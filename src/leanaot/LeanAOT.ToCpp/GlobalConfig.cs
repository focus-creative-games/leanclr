namespace LeanAOT.ToCpp
{
    public class GlobalConfig
    {
        public int MaxCodeSizeOfCppFile { get; set; } = 1024 * 1024 * 5; // 5 MB

        /// <summary>
        /// Options parsed from IL2CPP-compatible command line (reserved for future codegen / diagnostics).
        /// </summary>
        public bool ConvertToCpp { get; set; }

        public bool EnableAnalytics { get; set; }

        public bool EmitNullChecks { get; set; }

        public bool EnableArrayBoundsCheck { get; set; }

        public bool EmitMethodMap { get; set; }

        public bool EmitSourceMapping { get; set; }

        public bool UseSlimMetaFileFormat { get; set; }

        public string GenericsOption { get; set; }

        public string DotnetProfile { get; set; }

        public int? IncrementalGCTimeSlice { get; set; }

        public bool ProfilerReport { get; set; }

        public string ProfilerOutputFile { get; set; }

        public bool PrintCommandLine { get; set; }

        public string SymbolsFolder { get; set; }

        public string DataFolder { get; set; }

        public string MapFileParser { get; set; }

        public string BaseLibDirectory { get; set; }

        public bool AvoidDynamicLibraryCopy { get; set; }

        public string StatsOutputDir { get; set; }

        public string CompilerFlags { get; set; }

        public bool EnableLayoutValidation { get; set; }

        /// <summary>
        /// LeanAOT-only (from <c>--leanaot-aot-percent</c> or <c>LEANAOT_EXTRA_ARGS</c>): 0–100, reserved for future AOT method sampling.
        /// </summary>
        public int? AotSamplingPercent { get; set; }

        /// <summary>
        /// LeanAOT-only: path to a rule file listing which methods should be AOT-compiled (reserved; manifest does not load it yet).
        /// </summary>
        public string AotMethodRuleFile { get; set; }
    }
}
