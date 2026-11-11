using System.Text;
using System.Text.Json;
using System.Text.Json.Serialization;
using dnlib.DotNet;

namespace Net10ApiScan;

internal sealed class ApiWhitelist
{
    public List<string> AssemblyNames { get; set; } = [];
    public List<string> AssemblyPrefixes { get; set; } = [];
    public List<string> TypeNames { get; set; } = [];
    public List<string> TypePrefixes { get; set; } = [];
    public List<string> MemberNames { get; set; } = [];
    public List<string> MemberPrefixes { get; set; } = [];
}

internal sealed class ScanOptions
{
    public List<string> Inputs { get; } = [];
    public string? WhitelistPath { get; set; }
    public string? ReportPath { get; set; }
    public bool WarnOnly { get; set; }
}

internal sealed record ScanItem(string Kind, string Scope, string Name, string Source);

internal static class Program
{
    private static int Main(string[] args)
    {
        try
        {
            var options = ParseArgs(args);
            var whitelist = LoadWhitelist(options.WhitelistPath);
            var assemblies = ResolveInputs(options.Inputs);
            var skipped = new List<string>();
            var items = assemblies.SelectMany(path => ReadScanItems(path, skipped)).Distinct().OrderBy(i => i.Kind).ThenBy(i => i.Name).ToList();
            var unsupported = items.Where(i => !IsAllowed(i, whitelist)).ToList();

            WriteReport(options.ReportPath, assemblies, skipped, items, unsupported);

            Console.WriteLine($"Scanned {assemblies.Count} assembly file(s).");
            if (skipped.Count > 0)
            {
                Console.WriteLine($"Skipped {skipped.Count} file(s) without readable .NET metadata.");
            }
            Console.WriteLine($"Metadata refs: {items.Count}; unsupported: {unsupported.Count}.");
            if (unsupported.Count > 0)
            {
                foreach (var item in unsupported.Take(50))
                {
                    Console.WriteLine($"unsupported_api {item.Kind} {item.Name} [{item.Scope}] from {Path.GetFileName(item.Source)}");
                }
                if (unsupported.Count > 50)
                {
                    Console.WriteLine($"... {unsupported.Count - 50} more unsupported refs. See report for details.");
                }
            }

            return unsupported.Count == 0 || options.WarnOnly ? 0 : 2;
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine(ex.Message);
            return 1;
        }
    }

    private static ScanOptions ParseArgs(string[] args)
    {
        var options = new ScanOptions();
        for (int i = 0; i < args.Length; i++)
        {
            string arg = args[i];
            switch (arg)
            {
            case "--input":
                options.Inputs.Add(RequireValue(args, ref i, arg));
                break;
            case "--whitelist":
                options.WhitelistPath = RequireValue(args, ref i, arg);
                break;
            case "--report":
                options.ReportPath = RequireValue(args, ref i, arg);
                break;
            case "--warn-only":
                options.WarnOnly = true;
                break;
            case "-h":
            case "--help":
                PrintUsage();
                Environment.Exit(0);
                break;
            default:
                if (arg.StartsWith("-", StringComparison.Ordinal))
                {
                    throw new ArgumentException($"Unknown option: {arg}");
                }
                options.Inputs.Add(arg);
                break;
            }
        }

        if (options.Inputs.Count == 0)
        {
            throw new ArgumentException("At least one --input path is required.");
        }

        return options;
    }

    private static string RequireValue(string[] args, ref int index, string option)
    {
        if (index + 1 >= args.Length)
        {
            throw new ArgumentException($"{option} requires a value.");
        }
        index++;
        return args[index];
    }

    private static void PrintUsage()
    {
        Console.WriteLine("Usage: dotnet run --project src/tools/net10apiscan -- --input <dll-or-dir> [--whitelist file] [--report file] [--warn-only]");
    }

    private static ApiWhitelist LoadWhitelist(string? path)
    {
        if (string.IsNullOrWhiteSpace(path))
        {
            return new ApiWhitelist();
        }

        using var stream = File.OpenRead(path);
        var whitelist = JsonSerializer.Deserialize<ApiWhitelist>(stream, new JsonSerializerOptions
        {
            PropertyNameCaseInsensitive = true,
            ReadCommentHandling = JsonCommentHandling.Skip,
            AllowTrailingCommas = true,
        });
        return whitelist ?? new ApiWhitelist();
    }

    private static List<string> ResolveInputs(IEnumerable<string> inputs)
    {
        var files = new List<string>();
        foreach (string input in inputs)
        {
            string fullPath = Path.GetFullPath(input);
            if (Directory.Exists(fullPath))
            {
                files.AddRange(Directory.EnumerateFiles(fullPath, "*.dll", SearchOption.TopDirectoryOnly));
                files.AddRange(Directory.EnumerateFiles(fullPath, "*.exe", SearchOption.TopDirectoryOnly));
            }
            else if (File.Exists(fullPath))
            {
                files.Add(fullPath);
            }
            else
            {
                throw new FileNotFoundException($"Input path was not found: {input}");
            }
        }

        return files.Distinct(StringComparer.OrdinalIgnoreCase).OrderBy(p => p, StringComparer.OrdinalIgnoreCase).ToList();
    }

    private static IEnumerable<ScanItem> ReadScanItems(string assemblyPath, ICollection<string> skipped)
    {
        ModuleDefMD module;
        try
        {
            module = ModuleDefMD.Load(assemblyPath);
        }
        catch (Exception)
        {
            skipped.Add(assemblyPath);
            yield break;
        }

        foreach (var assemblyRef in module.GetAssemblyRefs())
        {
            yield return new ScanItem("AssemblyRef", assemblyRef.Name, assemblyRef.Name, assemblyPath);
        }

        foreach (var typeRef in module.GetTypeRefs())
        {
            string scope = typeRef.DefinitionAssembly?.Name ?? GetResolutionScopeName(typeRef.ResolutionScope);
            yield return new ScanItem("TypeRef", scope, typeRef.FullName, assemblyPath);
        }

        foreach (var memberRef in module.GetMemberRefs())
        {
            string scope = memberRef.DeclaringType?.DefinitionAssembly?.Name ?? GetTypeScopeName(memberRef.DeclaringType);
            yield return new ScanItem("MemberRef", scope, memberRef.FullName, assemblyPath);
        }
    }

    private static string GetResolutionScopeName(IResolutionScope? scope)
    {
        return scope switch
        {
            null => "",
            AssemblyRef assemblyRef => assemblyRef.Name,
            ModuleRef moduleRef => moduleRef.Name,
            TypeRef typeRef => typeRef.DefinitionAssembly?.Name ?? typeRef.FullName,
            ModuleDef moduleDef => moduleDef.Assembly?.Name ?? moduleDef.Name,
            _ => scope.FullName,
        };
    }

    private static string GetTypeScopeName(ITypeDefOrRef? type)
    {
        return type switch
        {
            null => "",
            TypeRef typeRef => typeRef.DefinitionAssembly?.Name ?? GetResolutionScopeName(typeRef.ResolutionScope),
            TypeDef typeDef => typeDef.Module.Assembly?.Name ?? typeDef.Module.Name,
            TypeSpec typeSpec => GetTypeScopeName(typeSpec.TypeSig?.ToTypeDefOrRef()),
            _ => type.FullName,
        };
    }

    private static bool IsAllowed(ScanItem item, ApiWhitelist whitelist)
    {
        if (item.Kind == "AssemblyRef")
        {
            return Matches(item.Name, whitelist.AssemblyNames, whitelist.AssemblyPrefixes);
        }

        if (Matches(item.Scope, whitelist.AssemblyNames, whitelist.AssemblyPrefixes))
        {
            return true;
        }

        if (item.Kind == "TypeRef")
        {
            return Matches(item.Name, whitelist.TypeNames, whitelist.TypePrefixes);
        }

        if (item.Kind == "MemberRef")
        {
            return Matches(item.Name, whitelist.MemberNames, whitelist.MemberPrefixes) ||
                   Matches(GetDeclaringTypeName(item.Name), whitelist.TypeNames, whitelist.TypePrefixes);
        }

        return false;
    }

    private static bool Matches(string value, IEnumerable<string> exactValues, IEnumerable<string> prefixes)
    {
        return exactValues.Any(v => string.Equals(value, v, StringComparison.Ordinal)) ||
               prefixes.Any(v => value.StartsWith(v, StringComparison.Ordinal));
    }

    private static string GetDeclaringTypeName(string memberFullName)
    {
        int marker = memberFullName.IndexOf("::", StringComparison.Ordinal);
        if (marker < 0)
        {
            return memberFullName;
        }

        string beforeMember = memberFullName[..marker];
        int lastSpace = beforeMember.LastIndexOf(' ');
        return lastSpace < 0 ? beforeMember : beforeMember[(lastSpace + 1)..];
    }

    private static void WriteReport(string? reportPath, IReadOnlyList<string> assemblies, IReadOnlyList<string> skippedAssemblies,
                                    IReadOnlyList<ScanItem> items, IReadOnlyList<ScanItem> unsupported)
    {
        if (string.IsNullOrWhiteSpace(reportPath))
        {
            return;
        }

        string fullPath = Path.GetFullPath(reportPath);
        string? directory = Path.GetDirectoryName(fullPath);
        if (!string.IsNullOrEmpty(directory))
        {
            Directory.CreateDirectory(directory);
        }

        var report = new
        {
            Assemblies = assemblies,
            SkippedAssemblies = skippedAssemblies,
            TotalReferenceCount = items.Count,
            UnsupportedReferenceCount = unsupported.Count,
            UnsupportedReferences = unsupported,
            References = items,
        };
        File.WriteAllText(fullPath, JsonSerializer.Serialize(report, new JsonSerializerOptions
        {
            WriteIndented = true,
            DefaultIgnoreCondition = JsonIgnoreCondition.WhenWritingNull,
        }), Encoding.UTF8);
    }
}
