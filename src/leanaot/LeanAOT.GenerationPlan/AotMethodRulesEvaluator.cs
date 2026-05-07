using System.Linq;
using System.Text;
using dnlib.DotNet;
using LeanAOT.Core;

namespace LeanAOT.GenerationPlan
{
    /// <summary>
    /// Loads multiple <c>aot.xml</c> documents, validates cross-file conflicts for applicable methods,
    /// and answers whether a method should be included in the AOT manifest after attribute / intrinsic checks.
    /// </summary>
    public sealed class AotMethodRulesEvaluator
    {
        private readonly IReadOnlyList<AotRuleDocument> _documents;

        /// <param name="absolutePaths">Rule files in CLI order.</param>
        /// <param name="assemblyCache">Used to enumerate methods in AOT assemblies for conflict detection.</param>
        /// <param name="aotAssemblyNames">Assembly short names passed to LeanAOT (<c>-a</c>).</param>
        public AotMethodRulesEvaluator(IReadOnlyList<string> absolutePaths, AssemblyCache assemblyCache, IReadOnlyList<string> aotAssemblyNames)
        {
            if (absolutePaths == null || absolutePaths.Count == 0)
            {
                _documents = Array.Empty<AotRuleDocument>();
                return;
            }

            var docs = new List<AotRuleDocument>();
            foreach (var p in absolutePaths)
            {
                if (string.IsNullOrWhiteSpace(p))
                    continue;
                docs.Add(AotRuleDocument.Load(p));
            }
            _documents = docs;
            if (_documents.Count == 0)
                return;

            ValidateNoCrossFileConflicts(assemblyCache, aotAssemblyNames);
        }

        internal IReadOnlyList<AotRuleDocument> Documents => _documents;

        /// <summary>
        /// After <see cref="Manifest"/> has excluded <see cref="AotMethodAttribute"/> false and handled intrinsics,
        /// returns whether the method should be AOT-compiled according to merged rules and default (no match = AOT).
        /// </summary>
        public bool ShouldIncludeByRules(string assemblyShortName, MethodDef method)
        {
            if (_documents.Count == 0)
                return true;

            bool? merged = null;
            foreach (var doc in _documents)
            {
                var v = doc.TryEvaluate(assemblyShortName, method);
                if (!v.HasValue)
                    continue;
                if (!merged.HasValue)
                    merged = v;
                else if (merged.Value != v.Value)
                {
                    // Should not happen after ctor validation
                    throw new InvalidOperationException(
                        $"Internal error: AOT rule conflict for {method.FullName} after validation.");
                }
            }

            // 8.4: no XML conclusion => AOT
            return merged ?? true;
        }

        private void ValidateNoCrossFileConflicts(AssemblyCache assemblyCache, IReadOnlyList<string> aotAssemblyNames)
        {
            if (_documents.Count < 2)
                return;

            var conflicts = new List<string>();
            foreach (var assName in aotAssemblyNames)
            {
                ModuleDefMD mod;
                try
                {
                    mod = assemblyCache.LoadModule(assName);
                }
                catch
                {
                    continue;
                }

                foreach (var type in mod.GetTypes())
                {
                    foreach (var method in type.Methods)
                    {
                        if (!IsCandidateForRuleLayer(method))
                            continue;

                        bool?[] perFile = _documents.Select(d => d.TryEvaluate(assName, method)).ToArray();
                        var hasTrue = perFile.Any(x => x == true);
                        var hasFalse = perFile.Any(x => x == false);
                        if (hasTrue && hasFalse)
                        {
                            var sb = new StringBuilder();
                            sb.AppendLine($"Method: {method.FullName} (assembly key: {assName})");
                            for (int i = 0; i < perFile.Length; i++)
                            {
                                if (perFile[i].HasValue)
                                    sb.AppendLine($"  {_documents[i].SourcePath}: aot={(perFile[i].Value ? "1" : "0")}");
                            }
                            conflicts.Add(sb.ToString().TrimEnd());
                        }
                    }
                }
            }

            if (conflicts.Count == 0)
                return;

            var msg = new StringBuilder();
            msg.AppendLine("Conflicting AOT rule files (same method must not be 1 in one file and 0 in another):");
            foreach (var c in conflicts)
                msg.AppendLine(c);
            throw new AotRuleConflictException(msg.ToString().TrimEnd());
        }

        /// <summary>
        /// Same structural filters as <see cref="Manifest"/> for methods that reach the rule layer,
        /// and skip methods governed only by <see cref="AotMethodAttribute"/> or intrinsics (design §7.3).
        /// </summary>
        internal static bool IsCandidateForRuleLayer(MethodDef method)
        {
            if (method.IsRuntime || method.IsAbstract || method.HasGenericParameters || method.DeclaringType.HasGenericParameters)
                return false;
            if (method.HasBody && method.Body.HasExceptionHandlers)
                return false;
            if (!method.HasBody && method.IsConstructor && method.DeclaringType.FullName == "System.String")
                return false;
            if (method.CallingConvention == CallingConvention.VarArg)
                return false;

            var ca = method.CustomAttributes.FirstOrDefault(a => a.TypeFullName == "AotMethodAttribute");
            if (ca != null)
                return false;
            if (method.IsPinvokeImpl || method.IsInternalCall)
                return false;
            return true;
        }
    }
}
