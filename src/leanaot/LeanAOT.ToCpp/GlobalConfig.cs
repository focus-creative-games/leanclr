using System.Collections.Generic;

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

        /// <summary>
        /// verify aot type instance-static size and instance-static field offsets are exactly same as in runtime.
        /// </summary>
        public bool EnableLayoutValidation { get; set; }


        /// <summary>
        /// emit null check before calling instance method. this behavior is same as il2cpp, but it is not in coreclr and mono.
        /// if we check null before calling instance method, we can avoid null check to this pointer in the called method.
        /// </summary>
        public bool EmitNullCheckBeforeCallInstanceMethod { get; set;} = true;

        /// <summary>
        /// LeanAOT-only: paths to <c>aot.xml</c> rule files (CLI order). Empty when unset.
        /// </summary>
        public List<string> AotMethodRuleFiles { get; set; } = new List<string>();

        /// <summary>
        /// LeanAOT-only: assembly short names (same as <c>-a</c>) omitted from <c>global-metadata.dat</c> COPH only; case-insensitive keys, stored as first-seen spelling.
        /// </summary>
        public List<string> AssembliesExcludedFromGlobalMetadata { get; set; } = new List<string>();
    }
}
