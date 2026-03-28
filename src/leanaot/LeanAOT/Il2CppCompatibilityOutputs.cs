using System.Text;
using dnlib.DotNet;
using LeanAOT.GenerationPlan;
using LeanAOT.ToCpp;
using NLog;

namespace LeanAOT;

/// <summary>
/// IL2CPP-compatible outputs described in docs/unity.md: global-metadata.dat, mscorlib resources stub, MethodMap.tsv.
/// </summary>
internal static class Il2CppCompatibilityOutputs
{
    private static readonly Logger s_logger = LogManager.GetCurrentClassLogger();

    public static void EmitIfRequested(
        GlobalConfig config,
        Manifest manifest,
        List<string> dllSearchPaths,
        List<string> aotAssemblyNames,
        MetadataService metadataService)
    {
        if (!string.IsNullOrWhiteSpace(config.DataFolder))
        {
            try
            {
                WriteDataFolder(config.DataFolder.Trim(), dllSearchPaths, aotAssemblyNames);
            }
            catch (Exception ex)
            {
                s_logger.Error(ex, "Failed to write IL2CPP data folder.");
                throw;
            }
        }

        if (!string.IsNullOrWhiteSpace(config.SymbolsFolder))
        {
            try
            {
                WriteMethodMapTsv(config.SymbolsFolder.Trim(), manifest, metadataService);
            }
            catch (Exception ex)
            {
                s_logger.Error(ex, "Failed to write MethodMap.tsv.");
                throw;
            }
        }
    }

    /// <summary>
    /// docs/unity.md: Metadata/global-metadata.dat (COPH bundle) and Resouces/mscorlib.dll-resources.dat (empty).
    /// </summary>
    private static void WriteDataFolder(string dataFolder, List<string> dllSearchPaths, List<string> aotAssemblyNames)
    {
        Directory.CreateDirectory(dataFolder);

        var metadataDir = Path.Combine(dataFolder, "Metadata");
        Directory.CreateDirectory(metadataDir);
        var datPath = Path.Combine(metadataDir, "global-metadata.dat");
        WriteGlobalMetadataDat(datPath, dllSearchPaths, aotAssemblyNames);

        // Doc spelling "Resouces" (Unity compatibility)
        var resourcesDir = Path.Combine(dataFolder, "Resouces");
        Directory.CreateDirectory(resourcesDir);
        var emptyResources = Path.Combine(resourcesDir, "mscorlib.dll-resources.dat");
        if (File.Exists(emptyResources))
            File.Delete(emptyResources);
        File.WriteAllBytes(emptyResources, Array.Empty<byte>());
        s_logger.Info($"Wrote empty resources file: {emptyResources}");
    }

    /// <summary>
    /// Format: Signature | AssemblyCount | AssemblyInfos | AssemblyBytes (see docs/unity.md).
    /// </summary>
    private static void WriteGlobalMetadataDat(string outputPath, List<string> dllSearchPaths, List<string> aotAssemblyNames)
    {
        var assemblies = new List<(string ShortName, byte[] Bytes)>();
        foreach (var name in aotAssemblyNames)
        {
            var path = ResolveAssemblyDllPath(name, dllSearchPaths);
            assemblies.Add((name, File.ReadAllBytes(path)));
        }

        using var ms = new MemoryStream();
        using var w = new BinaryWriter(ms, Encoding.UTF8, leaveOpen: true);

        w.Write(Encoding.ASCII.GetBytes("COPH"));
        w.Write(assemblies.Count);

        var nameBlocks = new List<byte[]>();
        foreach (var (shortName, _) in assemblies)
        {
            var raw = Encoding.UTF8.GetBytes(shortName);
            var withNull = new byte[raw.Length + 1];
            Array.Copy(raw, withNull, raw.Length);
            withNull[^1] = 0;
            var paddedLen = Align4(withNull.Length);
            var padded = new byte[paddedLen];
            Array.Copy(withNull, padded, withNull.Length);
            nameBlocks.Add(padded);
        }

        var offsets = new uint[assemblies.Count];
        uint assemblyBytesCursor = 0;
        for (int i = 0; i < assemblies.Count; i++)
        {
            offsets[i] = assemblyBytesCursor;
            assemblyBytesCursor += (uint)Align4(assemblies[i].Bytes.Length);
        }

        for (int i = 0; i < assemblies.Count; i++)
        {
            w.Write(nameBlocks[i]);
            w.Write((uint)assemblies[i].Bytes.Length);
            w.Write(offsets[i]);
        }

        for (int i = 0; i < assemblies.Count; i++)
        {
            w.Write(assemblies[i].Bytes);
            var pad = Padding4(assemblies[i].Bytes.Length);
            for (int p = 0; p < pad; p++)
                w.Write((byte)0);
        }

        File.WriteAllBytes(outputPath, ms.ToArray());
        s_logger.Info($"Wrote global-metadata.dat ({assemblies.Count} assemblies): {outputPath}");
    }

    private static int Align4(int length)
    {
        return (length + 3) & ~3;
    }

    private static int Padding4(int length)
    {
        var pad = (4 - (length % 4)) % 4;
        return pad;
    }

    private static string ResolveAssemblyDllPath(string assemblyName, List<string> dllSearchPaths)
    {
        foreach (var dir in dllSearchPaths)
        {
            var candidate = Path.Combine(dir, assemblyName + ".dll");
            if (File.Exists(candidate))
                return candidate;
        }
        throw new FileNotFoundException($"Cannot resolve DLL for assembly '{assemblyName}' in search paths.");
    }

    /// <summary>
    /// docs/unity.md: MethodMap.tsv — cpp name, managed signature, assembly name; NULL when no AOT body was generated.
    /// </summary>
    private static void WriteMethodMapTsv(string symbolsFolder, Manifest manifest, MetadataService metadataService)
    {
        Directory.CreateDirectory(symbolsFolder);
        var path = Path.Combine(symbolsFolder, "MethodMap.tsv");
        var lines = new List<string>();

        foreach (var assPlan in manifest.AssemblyPlans.Values.OrderBy(p => p.AssemblyName, StringComparer.Ordinal))
        {
            var module = assPlan.Module;
            var assemblyDisplayName = module.Assembly.Name;

            foreach (var type in module.GetTypes().OrderBy(t => t.FullName, StringComparer.Ordinal))
            {
                foreach (var method in type.Methods.OrderBy(m => m.MDToken.ToInt32()))
                {
                    var managedName = FormatManagedMethodMapName(method);
                    string cppName;
                    if (assPlan.ContainsMethod(method))
                    {
                        var detail = metadataService.GetMethodDetail(method);
                        cppName = detail.UniqueName;
                    }
                    else
                    {
                        cppName = "NULL";
                    }

                    lines.Add($"{EscapeTsvField(cppName)}\t{EscapeTsvField(managedName)}\t{EscapeTsvField(assemblyDisplayName)}");
                }
            }
        }

        File.WriteAllLines(path, lines, new UTF8Encoding(encoderShouldEmitUTF8Identifier: false));
        s_logger.Info($"Wrote MethodMap.tsv ({lines.Count} rows): {path}");
    }

    /// <summary>
    /// Matches docs/unity.md examples (return type + declaring type + :: + name + params).
    /// </summary>
    private static string FormatManagedMethodMapName(MethodDef method)
    {
        if (method.IsConstructor)
        {
            return method.FullName;
        }

        var ret = method.MethodSig.RetType.ToString();
        var decl = method.DeclaringType.FullName;
        var sb = new StringBuilder();
        sb.Append(ret);
        sb.Append(' ');
        sb.Append(decl);
        sb.Append("::");
        sb.Append(method.Name);
        sb.Append('(');
        var ps = method.MethodSig.Params;
        for (int i = 0; i < ps.Count; i++)
        {
            if (i > 0)
                sb.Append(',');
            sb.Append(ps[i].ToString());
        }
        sb.Append(')');
        return sb.ToString();
    }

    private static string EscapeTsvField(string s)
    {
        if (s == null)
            return string.Empty;
        return s.Replace("\t", " ").Replace("\r", " ").Replace("\n", " ");
    }
}
