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

            return new PgoMethodIncludeIndex(signaturesByAssembly, restrictedAssemblies);
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
