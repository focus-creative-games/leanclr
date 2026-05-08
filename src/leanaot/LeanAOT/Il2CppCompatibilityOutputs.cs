using System.Linq;
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
                WriteDataFolder(config.DataFolder.Trim(), dllSearchPaths, aotAssemblyNames, config);
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
    private static void WriteDataFolder(string dataFolder, List<string> dllSearchPaths, List<string> aotAssemblyNames, GlobalConfig config)
    {
        Directory.CreateDirectory(dataFolder);

        var metadataDir = Path.Combine(dataFolder, "Metadata");
        Directory.CreateDirectory(metadataDir);
        var datPath = Path.Combine(metadataDir, "global-metadata.dat");
        WriteGlobalMetadataDat(datPath, dllSearchPaths, aotAssemblyNames, config);

        // Doc spelling "Resouces" (Unity compatibility)
        var resourcesDir = Path.Combine(dataFolder, "Resources");
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
    private static void WriteGlobalMetadataDat(string outputPath, List<string> dllSearchPaths, List<string> aotAssemblyNames, GlobalConfig config)
    {
        var exclude = config.AssembliesExcludedFromGlobalMetadata ?? new List<string>();
        var excludeSet = new HashSet<string>(exclude, StringComparer.OrdinalIgnoreCase);
        var metadataAssemblyNames = aotAssemblyNames.Where(n => !excludeSet.Contains(n)).ToList();
        if (excludeSet.Count > 0)
        {
            var omitted = aotAssemblyNames.Where(n => excludeSet.Contains(n)).ToList();
            s_logger.Info("Excluding from global-metadata.dat ({0}): {1}", omitted.Count, string.Join(", ", omitted));
        }

        var assemblies = new List<(string ShortName, byte[] Bytes)>();
        foreach (var name in metadataAssemblyNames)
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
    /// IL2CPP-style: return type, declaring type (with type generic args), method name (with method generic args), parameters.
    /// Example: <c>T System.Array::InternalArray__get_Item&lt;T&gt;(System.Int32)</c> — Var/MVar use metadata generic parameter names;
    /// <c>GenericInst</c> uses concrete type arguments (e.g. <c>UnityEngine.Plane</c> inside parameter lists).
    /// </summary>
    private static string FormatManagedMethodMapName(MethodDef method)
    {
        try
        {
            return MethodMapSignatureFormatter.Format(method);
        }
        catch (Exception ex)
        {
            s_logger.Warn(ex, "Falling back to dnlib FullName for method {0}", method.FullName);
            return method.FullName;
        }
    }

    /// <summary>
    /// C#-like managed signatures for MethodMap.tsv (generic type + generic method + Var/MVar/GenericInst).
    /// </summary>
    private static class MethodMapSignatureFormatter
    {
        public static string Format(MethodDef method)
        {
            var ret = FormatTypeSigCSharp(method.MethodSig.RetType, method);
            var decl = FormatTypeDefCSharp(method.DeclaringType);
            var methodName = FormatMethodNameWithMethodGenericParams(method);
            var sb = new StringBuilder();
            sb.Append(ret);
            sb.Append(' ');
            sb.Append(decl);
            sb.Append("::");
            sb.Append(methodName);
            sb.Append('(');
            var ps = method.MethodSig.Params;
            for (int i = 0; i < ps.Count; i++)
            {
                if (i > 0)
                    sb.Append(',');
                sb.Append(FormatTypeSigCSharp(ps[i], method));
            }
            sb.Append(')');
            return sb.ToString();
        }

        private static string FormatMethodNameWithMethodGenericParams(MethodDef method)
        {
            var name = method.Name;
            if (method.GenericParameters.Count == 0)
                return name;
            var args = string.Join(",", method.GenericParameters.Select(p => p.Name));
            return name + "<" + args + ">";
        }

        /// <summary>
        /// Declaring type with nested path and type generic parameters (e.g. <c>System.Collections.Generic.List&lt;T&gt;</c>).
        /// </summary>
        private static string FormatTypeDefCSharp(TypeDef td)
        {
            if (td.DeclaringType != null)
                return FormatTypeDefCSharp(td.DeclaringType) + "." + FormatTypeNameWithTypeGenericParams(td);

            var ns = td.Namespace;
            var name = FormatTypeNameWithTypeGenericParams(td);
            return string.IsNullOrEmpty(ns) ? name : ns + "." + name;
        }

        private static string FormatTypeNameWithTypeGenericParams(TypeDef td)
        {
            var name = StripGenericAritySuffix(td.Name);
            if (td.GenericParameters.Count == 0)
                return name;
            var args = string.Join(",", td.GenericParameters.Select(p => p.Name));
            return name + "<" + args + ">";
        }

        private static string StripGenericAritySuffix(string name)
        {
            var tick = name.IndexOf('`');
            return tick >= 0 ? name.Substring(0, tick) : name;
        }

        private static string FormatTypeDefOrRefAsCSharp(ITypeDefOrRef type, MethodDef method)
        {
            if (type is TypeDef td)
                return FormatTypeDefCSharp(td);
            if (type is TypeRef tr)
            {
                var resolved = tr.Resolve();
                if (resolved != null)
                    return FormatTypeDefCSharp(resolved);
                return tr.FullName;
            }
            return type.FullName;
        }

        private static string FormatTypeDefNamespaceAndNestedNameForGenericInst(TypeDef td)
        {
            if (td.DeclaringType != null)
                return FormatTypeDefNamespaceAndNestedNameForGenericInst(td.DeclaringType) + "." + StripGenericAritySuffix(td.Name);

            var ns = td.Namespace;
            var name = StripGenericAritySuffix(td.Name);
            return string.IsNullOrEmpty(ns) ? name : ns + "." + name;
        }

        private static string FormatTypeSigCSharp(TypeSig sig, MethodDef method)
        {
            sig = sig.RemovePinnedAndModifiers();

            switch (sig.ElementType)
            {
            case ElementType.Void:
                return "System.Void";
            case ElementType.Var:
            {
                var gv = (GenericVar)sig;
                var td = method.DeclaringType;
                if ((int)gv.Number < td.GenericParameters.Count)
                    return td.GenericParameters[(int)gv.Number].Name;
                return "!" + gv.Number;
            }
            case ElementType.MVar:
            {
                var gmv = (GenericMVar)sig;
                if ((int)gmv.Number < method.GenericParameters.Count)
                    return method.GenericParameters[(int)gmv.Number].Name;
                return "!!" + gmv.Number;
            }
            case ElementType.GenericInst:
            {
                var gi = (GenericInstSig)sig;
                var typeDef = gi.GenericType.TypeDefOrRef.ResolveTypeDef();
                if (typeDef == null)
                    return gi.ToString();
                var head = FormatTypeDefNamespaceAndNestedNameForGenericInst(typeDef);
                var args = string.Join(",", gi.GenericArguments.Select(a => FormatTypeSigCSharp(a, method)));
                return head + "<" + args + ">";
            }
            case ElementType.SZArray:
                return FormatTypeSigCSharp(((SZArraySig)sig).Next, method) + "[]";
            case ElementType.Array:
            {
                var arr = (ArraySig)sig;
                var inner = FormatTypeSigCSharp(arr.Next, method);
                var commas = arr.Rank <= 1 ? "" : new string(',', (int)arr.Rank - 1);
                return inner + "[" + commas + "]";
            }
            case ElementType.ByRef:
                return FormatTypeSigCSharp(((ByRefSig)sig).Next, method) + "&";
            case ElementType.Ptr:
                return FormatTypeSigCSharp(((PtrSig)sig).Next, method) + "*";
            case ElementType.Class:
            case ElementType.ValueType:
            {
                var cv = (ClassOrValueTypeSig)sig;
                return FormatTypeDefOrRefAsCSharp(cv.TypeDefOrRef, method);
            }
            case ElementType.Object:
                return "System.Object";
            case ElementType.String:
                return "System.String";
            case ElementType.TypedByRef:
                return "System.TypedReference";
            case ElementType.I:
                return "System.IntPtr";
            case ElementType.U:
                return "System.UIntPtr";
            default:
                if (sig is CorLibTypeSig cor)
                    return cor.FullName;
                return sig.ToString();
            }
        }
    }

    private static string EscapeTsvField(string s)
    {
        if (s == null)
            return string.Empty;
        return s.Replace("\t", " ").Replace("\r", " ").Replace("\n", " ");
    }
}
