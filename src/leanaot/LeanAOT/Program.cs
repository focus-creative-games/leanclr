using LeanAOT.GenerationPlan;
using LeanAOT.ToCpp;
using NLog;
using System.Text;
using CommandLine;

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

        [Option("dotnetprofile", Required = false, HelpText = "IL2CPP: e.g. unityaot-linux (reserved).")]
        public string DotnetProfile { get; set; }

        [Option("profiler-report", Required = false, HelpText = "IL2CPP compatibility flag (reserved).")]
        public bool ProfilerReport { get; set; }

        [Option("profiler-output-file", Required = false, HelpText = "IL2CPP: profiler trace output path (reserved).")]
        public string ProfilerOutputFile { get; set; }

        [Option("print-command-line", Required = false, HelpText = "Print the effective command line to stdout.")]
        public bool PrintCommandLine { get; set; }
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

        LogManager.Setup().LoadConfigurationFromFile("nlog.xml");
        s_logger = LogManager.GetCurrentClassLogger();
    }

    static void Main(string[] args)
    {
        SetupApp();

        int exitCode = 0;
        var parser = new Parser(settings =>
        {
            settings.AllowMultiInstance = true;
            settings.CaseInsensitiveEnumValues = true;
        });
        parser.ParseArguments<CliOptions>(args)
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

                Run(dllSearchPaths, aotAssemblyNames, outputCodeDir, options);
            })
            .WithNotParsed(_ => exitCode = 1);
        Environment.ExitCode = exitCode;
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
        if (rawAssemblies.Count == 0)
        {
            errorMessage = "Missing required input: specify at least one assembly (-a / --assembly).";
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
            if (LooksLikeAssemblyPath(trimmed))
            {
                var full = Path.GetFullPath(trimmed);
                if (!File.Exists(full))
                {
                    errorMessage = $"Assembly file not found: {full}";
                    return false;
                }
                var dir = Path.GetDirectoryName(full);
                if (!string.IsNullOrEmpty(dir))
                    AddUniquePath(dllSearchPaths, dir);
                if (!aotAssemblyNames.Contains(Path.GetFileNameWithoutExtension(full), StringComparer.OrdinalIgnoreCase))
                    aotAssemblyNames.Add(Path.GetFileNameWithoutExtension(full));
            }
            else
            {
                var name = Path.GetFileNameWithoutExtension(trimmed);
                if (!aotAssemblyNames.Contains(name, StringComparer.OrdinalIgnoreCase))
                    aotAssemblyNames.Add(name);
            }
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

    private static bool LooksLikeAssemblyPath(string s)
    {
        if (string.IsNullOrEmpty(s))
            return false;
        if (Path.IsPathRooted(s))
            return true;
        if (s.IndexOf(Path.DirectorySeparatorChar) >= 0 || s.IndexOf(Path.AltDirectorySeparatorChar) >= 0)
            return true;
        if (s.EndsWith(".dll", StringComparison.OrdinalIgnoreCase) || s.EndsWith(".exe", StringComparison.OrdinalIgnoreCase))
            return true;
        return false;
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
        config.DotnetProfile = options.DotnetProfile;
        config.ProfilerReport = options.ProfilerReport;
        config.ProfilerOutputFile = options.ProfilerOutputFile;
        config.PrintCommandLine = options.PrintCommandLine;
        config.SymbolsFolder = options.SymbolsFolder;
        config.DataFolder = options.DataFolder;
    }

    private static void Run(List<string> dllSearchPaths, List<string> aotAssemblyNames, string outputCodeDir, CliOptions il2CppOptions)
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
        };
        ApplyIl2CppOptionsToGlobalConfig(globalServices.Config, il2CppOptions);
        GlobalServices.Inst = globalServices;

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
    }
}
