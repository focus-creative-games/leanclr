using System.Linq;
using LeanAOT.GenerationPlan;
using LeanAOT.ToCpp;
using NLog;
using System.Text;
using CommandLine;
using CommandLine.Text;

namespace LeanAOT;

internal class Program
{
    /// <summary>
    /// LeanAOT native CLI: -d searchDir -a AssemblyName -o outDir (assembly names without .dll).
    /// IL2CPP-compatible: --convert-to-cpp --assembly=path/to.dll ... --generatedcppdir=... [--symbols-folder=...] ...
    /// </summary>
    private sealed class CliOptions
    {
        [Option('d', Required = false, HelpText = "DLL search directory. Can be specified multiple times. Not required if every --assembly is a full path to a .dll.")]
        public IEnumerable<string> DllSearchPaths { get; set; }

        [Option('a', "assembly", Required = false, HelpText = "Assembly to AOT: short name (with -d) or full path to .dll (IL2CPP style). Can be specified multiple times.")]
        public IEnumerable<string> Assemblies { get; set; }

        [Option('o', "output", Required = false, HelpText = "Output directory for generated C++ code (LeanAOT short form).")]
        public string OutputCodeDir { get; set; }

        [Option("generatedcppdir", Required = false, HelpText = "Output directory for generated C++ code (IL2CPP name, same as -o).")]
        public string GeneratedCppDir { get; set; }

        [Option("convert-to-cpp", Required = false, HelpText = "IL2CPP compatibility: request managed-to-C++ conversion (informational).")]
        public bool ConvertToCpp { get; set; }

        [Option("symbols-folder", Required = false, HelpText = "IL2CPP: symbols output folder (reserved).")]
        public string SymbolsFolder { get; set; }

        [Option("data-folder", Required = false, HelpText = "IL2CPP: data folder (reserved).")]
        public string DataFolder { get; set; }

        [Option("enable-analytics", Required = false, HelpText = "IL2CPP compatibility flag (reserved).")]
        public bool EnableAnalytics { get; set; }

        [Option("emit-null-checks", Required = false, HelpText = "IL2CPP compatibility flag (reserved).")]
        public bool EmitNullChecks { get; set; }

        [Option("enable-array-bounds-check", Required = false, HelpText = "IL2CPP compatibility flag (reserved).")]
        public bool EnableArrayBoundsCheck { get; set; }

        [Option("emit-method-map", Required = false, HelpText = "IL2CPP compatibility flag (reserved).")]
        public bool EmitMethodMap { get; set; }

        [Option("emit-source-mapping", Required = false, HelpText = "IL2CPP compatibility: emit managed-to-generated-C++ source mapping (reserved for codegen).")]
        public bool EmitSourceMapping { get; set; }

        /// <summary>
        /// tuanjie only.
        /// </summary>
        [Option("use-slim-meta-file-format", Required = false, HelpText = "IL2CPP compatibility flag (reserved).")]
        public bool UseSlimMetaFileFormat { get; set; }

        [Option("generics-option", Required = false, HelpText = "IL2CPP generics option (reserved), e.g. None,EnableFullSharing.")]
        public string GenericsOption { get; set; }

        [Option("dotnetprofile", Required = false, HelpText = "IL2CPP: e.g. unityaot-linux (reserved).")]
        public string DotnetProfile { get; set; }

        [Option("incremental-g-c-time-slice", Required = false, HelpText = "IL2CPP: incremental GC time slice (reserved).")]
        public int? IncrementalGCTimeSlice { get; set; }

        [Option("profiler-report", Required = false, HelpText = "IL2CPP compatibility flag (reserved).")]
        public bool ProfilerReport { get; set; }

        [Option("profiler-output-file", Required = false, HelpText = "IL2CPP: profiler trace output path (reserved).")]
        public string ProfilerOutputFile { get; set; }

        [Option("print-command-line", Required = false, HelpText = "Print the effective command line to stdout.")]
        public bool PrintCommandLine { get; set; }

        [Option("compiler-flags", Required = false, HelpText = "IL2CPP: C++ compiler flags (e.g. -fno-exceptions). Repeatable; stored in config, LeanAOT does not compile C++.")]
        public IEnumerable<string> CompilerFlags { get; set; }

        [Option("usymtool-path", Required = false, HelpText = "IL2CPP: usym tool path (reserved).")]
        public string UsymToolPath { get; set; }

        [Option("static-lib-il2-cpp", Required = false, HelpText = "IL2CPP: static lib il2cpp path (reserved).")]
        public bool StaticLibIl2CppPath { get; set; }

        [Option("map-file-parser", Required = false, HelpText = "IL2CPP: map file parser executable path (reserved).")]
        public string MapFileParser { get; set; }

        [Option("directory", Required = false, HelpText = "IL2CPP: managed assemblies directory. Used to auto-discover assemblies when --assembly is omitted.")]
        public string Directory { get; set; }

        [Option("baselib-directory", Required = false, HelpText = "IL2CPP: baselib directory path (reserved).")]
        public string BaseLibDirectory { get; set; }

        [Option("avoid-dynamic-library-copy", Required = false, HelpText = "IL2CPP compatibility flag (reserved).")]
        public bool AvoidDynamicLibraryCopy { get; set; }

        [Option("stats-output-dir", Required = false, HelpText = "IL2CPP: stats output directory (reserved).")]
        public string StatsOutputDir { get; set; }
    }

    private static Logger s_logger;

    private static void SetupApp()
    {
        ConsoleUtil.EnableQuickEditMode(false);
        Console.OutputEncoding = Encoding.UTF8;
        Encoding.RegisterProvider(CodePagesEncodingProvider.Instance);

        int processorCount = Environment.ProcessorCount;
        ThreadPool.SetMinThreads(Math.Max(4, processorCount), 0);
        ThreadPool.SetMaxThreads(Math.Max(16, processorCount * 2), 2);

        var nlogConfigPath = Path.Combine(AppContext.BaseDirectory, "nlog.xml");
        LogManager.Setup().LoadConfigurationFromFile(nlogConfigPath);
        s_logger = LogManager.GetCurrentClassLogger();
    }

    static void Main(string[] args)
    {
        SetupApp();
        var runtimeApiCatalog = LoadRuntimeApiCatalogFromCurrentDirectory();

        int exitCode = 0;
        var helpWriter = new StringWriter();
        var parser = new Parser(settings =>
        {
            settings.AllowMultiInstance = true;
            settings.CaseInsensitiveEnumValues = true;
            settings.HelpWriter = helpWriter;
        });
        var normalizedArgs = NormalizeSingleDashLongOptions(args);
        var parseResult = parser.ParseArguments<CliOptions>(normalizedArgs);
        if (parseResult.Tag == ParserResultType.NotParsed)
        {
            Console.Error.WriteLine(helpWriter.ToString());
            Environment.Exit(1);
        }
        parseResult
            .WithParsed(options =>
            {
                if (options.PrintCommandLine)
                {
                    Console.WriteLine(string.Join(" ", args.Select(a => a.Contains(' ') ? $"\"{a}\"" : a)));
                }

                if (!TryNormalizeCli(options, out var dllSearchPaths, out var aotAssemblyNames, out var outputCodeDir, out var errorMessage))
                {
                    s_logger.Error(errorMessage);
                    exitCode = 1;
                    return;
                }

                Run(dllSearchPaths, aotAssemblyNames, outputCodeDir, options, runtimeApiCatalog);
            });
        Environment.ExitCode = exitCode;
    }

    private static RuntimeApiCatalog LoadRuntimeApiCatalogFromCurrentDirectory()
    {
        var baseDir = AppContext.BaseDirectory;
        var catalog = RuntimeApiCatalog.LoadFromDirectory(baseDir);
        s_logger.Info(
            "Loaded runtime API configs from {0}: icalls={1}, intrinsics={2}, icalls_newobj={3}, intrinsics_newobj={4}",
            baseDir,
            catalog.IcallCount,
            catalog.IntrinsicCount,
            catalog.IcallNewobjCount,
            catalog.IntrinsicNewobjCount);
        return catalog;
    }

    private static bool TryNormalizeCli(
        CliOptions options,
        out List<string> dllSearchPaths,
        out List<string> aotAssemblyNames,
        out string outputCodeDir,
        out string errorMessage)
    {
        dllSearchPaths = new List<string>();
        aotAssemblyNames = new List<string>();
        outputCodeDir = null;
        errorMessage = null;

        var rawAssemblies = options.Assemblies?.ToList() ?? new List<string>();
        if (rawAssemblies.Count == 0 && !string.IsNullOrWhiteSpace(options.Directory))
        {
            var managedDir = Path.GetFullPath(options.Directory.Trim());
            if (!System.IO.Directory.Exists(managedDir))
            {
                errorMessage = $"Managed directory not found: {managedDir}";
                return false;
            }

            AddUniquePath(dllSearchPaths, managedDir);
            rawAssemblies.AddRange(System.IO.Directory.EnumerateFiles(managedDir, "*.dll", SearchOption.TopDirectoryOnly)
                .Select(Path.GetFileName));
        }
        if (rawAssemblies.Count == 0)
        {
            errorMessage = "Missing required input: specify at least one assembly (-a / --assembly), or use --directory to auto-discover DLLs.";
            return false;
        }

        foreach (var d in options.DllSearchPaths ?? Enumerable.Empty<string>())
        {
            if (string.IsNullOrWhiteSpace(d))
                continue;
            AddUniquePath(dllSearchPaths, Path.GetFullPath(d));
        }

        foreach (var raw in rawAssemblies)
        {
            if (string.IsNullOrWhiteSpace(raw))
                continue;
            var trimmed = raw.Trim();
            var name = Path.GetFileNameWithoutExtension(trimmed);
            if (!aotAssemblyNames.Contains(name, StringComparer.OrdinalIgnoreCase))
                aotAssemblyNames.Add(name);
        }

        if (dllSearchPaths.Count == 0)
        {
            errorMessage =
                "No DLL search path: use -d for each search directory, or pass full paths with --assembly=path/to.dll (IL2CPP style).";
            return false;
        }

        outputCodeDir = !string.IsNullOrWhiteSpace(options.GeneratedCppDir)
            ? options.GeneratedCppDir.Trim()
            : options.OutputCodeDir?.Trim();
        if (string.IsNullOrEmpty(outputCodeDir))
        {
            errorMessage = "Missing output directory: use -o / --output or --generatedcppdir (IL2CPP).";
            return false;
        }

        return true;
    }

    /// <summary>
    /// IL2CPP 工具链有时会传入 <c>-convert-to-cpp</c> 这类单横线长参数，这里统一转为双横线格式。
    /// </summary>
    private static string[] NormalizeSingleDashLongOptions(IEnumerable<string> args)
    {
        return args.Select(a =>
        {
            if (string.IsNullOrEmpty(a) || !a.StartsWith("-") || a.StartsWith("--"))
                return a;
            if (a.Length <= 2)
                return a; // 保留短参数，如 -d / -a / -o

            var eqIndex = a.IndexOf('=');
            var optionName = eqIndex >= 0 ? a.Substring(1, eqIndex - 1) : a.Substring(1);
            if (optionName.Length > 1 && optionName.Contains('-'))
                return "--" + a.Substring(1);
            return a;
        }).ToArray();
    }

    private static void AddUniquePath(List<string> list, string path)
    {
        if (string.IsNullOrEmpty(path))
            return;
        if (!list.Contains(path, StringComparer.OrdinalIgnoreCase))
            list.Add(path);
    }

    private static void ApplyIl2CppOptionsToGlobalConfig(GlobalConfig config, CliOptions options)
    {
        config.ConvertToCpp = options.ConvertToCpp;
        config.EnableAnalytics = options.EnableAnalytics;
        config.EmitNullChecks = options.EmitNullChecks;
        config.EnableArrayBoundsCheck = options.EnableArrayBoundsCheck;
        config.EmitMethodMap = options.EmitMethodMap;
        config.EmitSourceMapping = options.EmitSourceMapping;
        config.UseSlimMetaFileFormat = options.UseSlimMetaFileFormat;
        config.GenericsOption = options.GenericsOption;
        config.DotnetProfile = options.DotnetProfile;
        config.IncrementalGCTimeSlice = options.IncrementalGCTimeSlice;
        config.ProfilerReport = options.ProfilerReport;
        config.ProfilerOutputFile = options.ProfilerOutputFile;
        config.PrintCommandLine = options.PrintCommandLine;
        config.SymbolsFolder = options.SymbolsFolder;
        // in unity 2019 and 2021, the data folder is under generated cpp dir.
        config.DataFolder = options.DataFolder ?? (!string.IsNullOrWhiteSpace(options.Directory) ? Path.Combine(options.GeneratedCppDir, "Data") : null);
        config.MapFileParser = options.MapFileParser;
        config.BaseLibDirectory = options.BaseLibDirectory;
        config.AvoidDynamicLibraryCopy = options.AvoidDynamicLibraryCopy;
        config.StatsOutputDir = options.StatsOutputDir;
        config.CompilerFlags = NormalizeCompilerFlags(options.CompilerFlags);
    }

    /// <summary>
    /// 合并多次 <c>--compiler-flags</c>，去掉空白与 CR，单条内 trim。
    /// </summary>
    private static string NormalizeCompilerFlags(IEnumerable<string> rawParts)
    {
        if (rawParts == null)
            return null;
        var tokens = new List<string>();
        foreach (var raw in rawParts)
        {
            if (string.IsNullOrWhiteSpace(raw))
                continue;
            var t = raw.Trim().Replace("\r", string.Empty).Trim();
            if (t.Length > 0)
                tokens.Add(t);
        }
        return tokens.Count == 0 ? null : string.Join(" ", tokens);
    }

    /// <summary>
    /// IL2CPP 兼容：在 <see cref="GlobalConfig.ProfilerOutputFile"/> 中给出路径时，创建空文件（含父目录）。
    /// </summary>
    private static void EnsureEmptyProfilerOutputFile(GlobalConfig config)
    {
        var path = config.ProfilerOutputFile?.Trim();
        if (string.IsNullOrEmpty(path))
            return;

        var full = Path.GetFullPath(path);
        var dir = Path.GetDirectoryName(full);
        if (!string.IsNullOrEmpty(dir))
            Directory.CreateDirectory(dir);
        File.WriteAllText(full, string.Empty);
        s_logger.Info("Created empty profiler output file: {0}", full);
    }

    private static void Run(List<string> dllSearchPaths, List<string> aotAssemblyNames, string outputCodeDir, CliOptions il2CppOptions,
                            RuntimeApiCatalog runtimeApiCatalog)
    {
        var generator = new CppGenerator();
        var assemblyCache = new Core.AssemblyCache(new Core.MultiDirectoryAssemblyResolver(dllSearchPaths));
        var manifestArgs = new ManifestArgs()
        {
            assemblyCache = assemblyCache,
            aotAssemblyNames = aotAssemblyNames,
        };
        var manifest = new Manifest(manifestArgs);

        var metaService = new MetadataService();
        var globalServices = new GlobalServices()
        {
            Config = new GlobalConfig(),
            ManifestService = new ManifestService(manifest),
            TypeNameService = new TypeNameService(metaService),
            InvokerService = new InvokerService(metaService),
            MetadataService = metaService,
            RuntimeApiCatalog = runtimeApiCatalog,
        };
        ApplyIl2CppOptionsToGlobalConfig(globalServices.Config, il2CppOptions);
        GlobalServices.Inst = globalServices;

        EnsureEmptyProfilerOutputFile(globalServices.Config);

        var confBuilder = new GenerationConfigBuilder()
        {
            outputCodeDir = outputCodeDir,
            manifest = manifest,
            dllSearchPaths = dllSearchPaths,
            assemblyCache = assemblyCache,
        };
        if (Directory.Exists(confBuilder.outputCodeDir))
        {
            Directory.Delete(confBuilder.outputCodeDir, true);
        }
        Directory.CreateDirectory(confBuilder.outputCodeDir);
        var conf = confBuilder.Build();
        generator.Generate(conf);

        Il2CppCompatibilityOutputs.EmitIfRequested(globalServices.Config, manifest, dllSearchPaths, aotAssemblyNames, metaService);
    }
}
