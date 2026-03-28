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

        /// <summary>
        /// IL2CPP <c>--compiler-flags</c>：可多次出现，此处为规范化后空格拼接（如 <c>-fno-exceptions</c>）。
        /// 当前 LeanAOT 不调用 C++ 编译器，仅解析并保存以供代理/诊断或与 Unity 管线对齐。
        /// </summary>
        public string CompilerFlags { get; set; }
    }
}
