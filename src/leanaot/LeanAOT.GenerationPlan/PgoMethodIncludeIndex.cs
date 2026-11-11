using System.Xml.Linq;
using dnlib.DotNet;

namespace LeanAOT.GenerationPlan
{
    /// <summary>
    /// Exact-method include list from machine-generated PGO rule files.
    /// <see cref="Contains"/> is additive: profiled methods are AOT-compiled even when <c>aot.xml</c> excluded them.
    /// </summary>
    public sealed class PgoMethodIncludeIndex
    {
        public sealed class InterfaceHint
        {
            public uint Offset { get; set; }
            public int Entries { get; set; }
            public ulong Calls { get; set; }
        }
        private sealed class CacheProfile
        {
            public string Hash;
            public uint Bytes;
            public List<InterfaceHint> Hints;
        }
        private readonly Dictionary<string, Dictionary<string, CacheProfile>> _cacheProfiles = new(StringComparer.OrdinalIgnoreCase);

        public IReadOnlyList<InterfaceHint> GetInterfaceHints(MethodDef method)
        {
            if (!_cacheProfiles.TryGetValue(method.Module.Assembly.Name, out var methods) ||
                !methods.TryGetValue(AotRuleDocument.MethodSignatureDisplay(method), out var profile)) return Array.Empty<InterfaceHint>();
            var fingerprint = AotProfileFingerprint.Compute(method);
            return fingerprint.bytes == profile.Bytes && string.Equals(fingerprint.hash, profile.Hash, StringComparison.OrdinalIgnoreCase)
                ? profile.Hints : Array.Empty<InterfaceHint>();
        }
        private readonly Dictionary<string, HashSet<string>> _signaturesByAssembly;
        private readonly HashSet<string> _restrictedAssemblies;

        private PgoMethodIncludeIndex(
            Dictionary<string, HashSet<string>> signaturesByAssembly,
            HashSet<string> restrictedAssemblies)
        {
            _signaturesByAssembly = signaturesByAssembly;
            _restrictedAssemblies = restrictedAssemblies;
        }

        public bool IsEnabled => _restrictedAssemblies.Count > 0;

        public IReadOnlyCollection<string> RestrictedAssemblies => _restrictedAssemblies;

        public int GetIncludeCount(string assemblyShortName)
        {
            return _signaturesByAssembly.TryGetValue(NormalizeAssemblyKey(assemblyShortName), out var set)
                ? set.Count
                : 0;
        }

        public bool IsAssemblyRestricted(string assemblyShortName) =>
            _restrictedAssemblies.Contains(NormalizeAssemblyKey(assemblyShortName));

        public bool Contains(string assemblyShortName, MethodDef method)
        {
            var assKey = NormalizeAssemblyKey(assemblyShortName);
            if (!_signaturesByAssembly.TryGetValue(assKey, out var signatures))
                return false;
            return signatures.Contains(AotRuleDocument.MethodSignatureDisplay(method));
        }

        public static PgoMethodIncludeIndex Load(IReadOnlyList<string> absolutePaths)
        {
            if (absolutePaths == null || absolutePaths.Count == 0)
                return null;

            var signaturesByAssembly = new Dictionary<string, HashSet<string>>(StringComparer.OrdinalIgnoreCase);
            var restrictedAssemblies = new HashSet<string>(StringComparer.OrdinalIgnoreCase);

            foreach (var path in absolutePaths)
            {
                if (string.IsNullOrWhiteSpace(path))
                    continue;
                MergeFile(path.Trim(), signaturesByAssembly, restrictedAssemblies);
            }

            if (restrictedAssemblies.Count == 0)
                return null;

            var index = new PgoMethodIncludeIndex(signaturesByAssembly, restrictedAssemblies);
            foreach (var path in absolutePaths.Where(p => !string.IsNullOrWhiteSpace(p))) index.LoadCacheProfiles(path);
            return index;
        }

        private void LoadCacheProfiles(string path)
        {
            foreach (var assembly in XDocument.Load(path).Root.Elements("assembly"))
            {
                string name = NormalizeAssemblyKey((string)assembly.Attribute("name"));
                if (!_cacheProfiles.TryGetValue(name, out var methods)) _cacheProfiles[name] = methods = new(StringComparer.Ordinal);
                foreach (var method in assembly.Elements("method"))
                {
                    var elements = method.Elements("interface-cache").ToList();
                    if (elements.Count == 0) continue;
                    string hash = (string)method.Attribute("il-hash");
                    if (hash == null || hash.Length != 16 || !hash.All(Uri.IsHexDigit) || !uint.TryParse((string)method.Attribute("il-bytes"), out uint bytes) || bytes == 0)
                        throw new AotRuleFileException(path, "Interface cache hints require a valid il-hash and il-bytes.");
                    var hints = new List<InterfaceHint>();
                    int words = 0;
                    foreach (var element in elements)
                    {
                        if (!uint.TryParse((string)element.Attribute("il-offset"), out uint offset) || offset >= bytes ||
                            !int.TryParse((string)element.Attribute("entries"), out int entries) || entries < 0 || entries > 2 ||
                            !ulong.TryParse((string)element.Attribute("calls"), out ulong calls) || hints.Any(h => h.Offset == offset))
                            throw new AotRuleFileException(path, "Invalid or duplicated interface cache hint.");
                        words += entries * 3;
                        hints.Add(new InterfaceHint { Offset = offset, Entries = entries, Calls = calls });
                    }
                    if (words > 12 || hints.Count > 4) throw new AotRuleFileException(path, "Interface cache hints exceed the per-method storage budget.");
                    string signature = ((string)method.Attribute("signature")).Trim();
                    if (methods.TryGetValue(signature, out var old))
                    {
                        // Conflicting captures cannot safely suggest one capacity.
                        if (!string.Equals(old.Hash, hash, StringComparison.OrdinalIgnoreCase) || old.Bytes != bytes)
                        { old.Hash = ""; old.Hints.Clear(); continue; }
                        foreach (var hint in hints)
                        {
                            var existing = old.Hints.FirstOrDefault(h => h.Offset == hint.Offset);
                            if (existing == null) old.Hints.Add(hint);
                            else { if (existing.Entries != hint.Entries) existing.Entries = 0; existing.Calls = Math.Max(existing.Calls, hint.Calls); }
                        }
                    }
                    else methods.Add(signature, new CacheProfile { Hash = hash, Bytes = bytes, Hints = hints });
                }
            }
        }

        private static void MergeFile(
            string path,
            Dictionary<string, HashSet<string>> signaturesByAssembly,
            HashSet<string> restrictedAssemblies)
        {
            var fullPath = Path.GetFullPath(path);
            var doc = XDocument.Load(fullPath, LoadOptions.SetLineInfo);
            if (doc.Root == null || !string.Equals(doc.Root.Name.LocalName, "aot", StringComparison.Ordinal))
                throw new AotRuleFileException(fullPath, "Root element must be <aot>.");

            foreach (var asmEl in doc.Root.Elements())
            {
                if (!string.Equals(asmEl.Name.LocalName, "assembly", StringComparison.Ordinal))
                    continue;

                var assemblyName = (string)asmEl.Attribute("name");
                if (string.IsNullOrWhiteSpace(assemblyName))
                    throw new AotRuleFileException(fullPath, "<assembly> requires attribute name.");

                var assKey = NormalizeAssemblyKey(assemblyName);
                restrictedAssemblies.Add(assKey);
                if (!signaturesByAssembly.TryGetValue(assKey, out var signatures))
                {
                    signatures = new HashSet<string>(StringComparer.Ordinal);
                    signaturesByAssembly[assKey] = signatures;
                }

                foreach (var methodEl in asmEl.Elements())
                {
                    if (!string.Equals(methodEl.Name.LocalName, "method", StringComparison.Ordinal))
                        continue;

                    var signature = (string)methodEl.Attribute("signature");
                    if (string.IsNullOrWhiteSpace(signature))
                        throw new AotRuleFileException(fullPath, "<method> requires attribute signature.");

                    signature = signature.Trim();
                    signatures.Add(signature);
                }
            }
        }

        private static string NormalizeAssemblyKey(string assemblyShortName) =>
            assemblyShortName.Trim();
    }
}
