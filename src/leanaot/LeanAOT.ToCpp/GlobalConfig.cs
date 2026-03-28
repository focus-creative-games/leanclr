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

        public string DotnetProfile { get; set; }

        public bool ProfilerReport { get; set; }

        public string ProfilerOutputFile { get; set; }

        public bool PrintCommandLine { get; set; }

        public string SymbolsFolder { get; set; }

        public string DataFolder { get; set; }
    }
}
