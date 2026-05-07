using System.Text.RegularExpressions;
using System.Xml.Linq;
using dnlib.DotNet;

namespace LeanAOT.GenerationPlan
{
    /// <summary>
    /// One parsed <c>aot.xml</c>: ordered rules; later rules override earlier ones for the same <see cref="MethodDef"/>.
    /// </summary>
    internal sealed class AotRuleDocument
    {
        internal AotRuleDocument(string sourcePath, IReadOnlyList<AotRuleEntry> entriesInOrder)
        {
            SourcePath = sourcePath;
            EntriesInOrder = entriesInOrder;
        }

        internal string SourcePath { get; }

        internal IReadOnlyList<AotRuleEntry> EntriesInOrder { get; }

        /// <summary>
        /// Returns <c>null</c> if no rule in this file applies a conclusion to the method.
        /// Otherwise <c>true</c> = require AOT, <c>false</c> = exclude from AOT.
        /// </summary>
        internal bool? TryEvaluate(string assemblyShortName, MethodDef method)
        {
            bool? verdict = null;
            foreach (var e in EntriesInOrder)
            {
                if (e.Matches(assemblyShortName, method))
                    verdict = e.WantAot;
            }
            return verdict;
        }

        internal static AotRuleDocument Load(string path)
        {
            var doc = XDocument.Load(path, LoadOptions.SetLineInfo);
            if (doc.Root == null || !string.Equals(doc.Root.Name.LocalName, "aot", StringComparison.Ordinal))
                throw new AotRuleFileException(path, "Root element must be <aot>.");

            var list = new List<AotRuleEntry>();
            foreach (var asmEl in doc.Root.Elements())
            {
                if (!string.Equals(asmEl.Name.LocalName, "assembly", StringComparison.Ordinal))
                    continue;

                var assemblyPattern = (string)asmEl.Attribute("fullname");
                if (string.IsNullOrWhiteSpace(assemblyPattern))
                    throw new AotRuleFileException(path, "<assembly> requires attribute fullname.");

                var assemblyAotAttr = (string)asmEl.Attribute("aot");
                bool? assemblyAot = ParseOptionalAot(path, assemblyAotAttr, "assembly", assemblyPattern);
                if (assemblyAot.HasValue)
                    list.Add(new AssemblyDefaultRule(assemblyPattern, assemblyAot.Value));

                foreach (var typeEl in asmEl.Elements())
                {
                    if (!string.Equals(typeEl.Name.LocalName, "type", StringComparison.Ordinal))
                        continue;

                    var typePattern = (string)typeEl.Attribute("fullname");
                    if (string.IsNullOrWhiteSpace(typePattern))
                        throw new AotRuleFileException(path, "<type> requires attribute fullname.");

                    var typeAotAttr = (string)typeEl.Attribute("aot");
                    bool? typeExplicit = ParseOptionalAot(path, typeAotAttr, "type", typePattern);
                    bool? effectiveTypeDefault = typeExplicit ?? assemblyAot;
                    if (effectiveTypeDefault.HasValue)
                        list.Add(new TypeDefaultRule(assemblyPattern, typePattern, effectiveTypeDefault.Value));

                    foreach (var methodEl in typeEl.Elements())
                    {
                        if (!string.Equals(methodEl.Name.LocalName, "method", StringComparison.Ordinal))
                            continue;

                        var namePattern = (string)methodEl.Attribute("name");
                        if (string.IsNullOrWhiteSpace(namePattern))
                            throw new AotRuleFileException(path, "<method> requires attribute name.");

                        var methodAotAttr = (string)methodEl.Attribute("aot");
                        if (string.IsNullOrWhiteSpace(methodAotAttr))
                            throw new AotRuleFileException(path, "<method> requires attribute aot (1 or 0).");
                        bool methodWant = ParseRequiredAot(path, methodAotAttr, methodEl);

                        var sigPattern = (string)methodEl.Attribute("signature");
                        if (string.IsNullOrWhiteSpace(sigPattern))
                            sigPattern = null;

                        list.Add(new MethodExplicitRule(assemblyPattern, typePattern, namePattern, sigPattern, methodWant));
                    }
                }
            }

            return new AotRuleDocument(path, list);
        }

        private static bool? ParseOptionalAot(string path, string raw, string elementKind, string hint)
        {
            if (string.IsNullOrWhiteSpace(raw))
                return null;
            return ParseRequiredAot(path, raw, hint: $"{elementKind} fullname='{hint}'");
        }

        private static bool ParseRequiredAot(string path, string raw, XElement context = null, string hint = null)
        {
            var t = raw.Trim();
            if (t == "1")
                return true;
            if (t == "0")
                return false;
            var where = hint ?? (context != null ? context.ToString(SaveOptions.DisableFormatting) : raw);
            throw new AotRuleFileException(path, $"Invalid aot value (must be 1 or 0): {where}");
        }

        /// <summary>
        /// Signature string for <c>method/@signature</c> glob matching (stable across rebuilds; no metadata tokens).
        /// Uses dnlib <see cref="MethodDef.FullName"/> (return type, declaring type, name, parameters).
        /// </summary>
        internal static string MethodSignatureDisplay(MethodDef method) => method?.FullName ?? string.Empty;
    }

    internal abstract class AotRuleEntry
    {
        protected AotRuleEntry(bool wantAot) => WantAot = wantAot;

        internal bool WantAot { get; }

        internal abstract bool Matches(string assemblyShortName, MethodDef method);
    }

    internal sealed class AssemblyDefaultRule : AotRuleEntry
    {
        private readonly Regex _assemblyRx;

        internal AssemblyDefaultRule(string assemblyPattern, bool wantAot)
            : base(wantAot) =>
            _assemblyRx = AotRuleGlob.CompileGlob(assemblyPattern, ignoreCase: true);

        internal override bool Matches(string assemblyShortName, MethodDef method) =>
            _assemblyRx.IsMatch(assemblyShortName);
    }

    internal sealed class TypeDefaultRule : AotRuleEntry
    {
        private readonly Regex _assemblyRx;
        private readonly Regex _typeRx;

        internal TypeDefaultRule(string assemblyPattern, string typePattern, bool wantAot)
            : base(wantAot)
        {
            _assemblyRx = AotRuleGlob.CompileGlob(assemblyPattern, ignoreCase: true);
            _typeRx = AotRuleGlob.CompileGlob(typePattern, ignoreCase: false);
        }

        internal override bool Matches(string assemblyShortName, MethodDef method)
        {
            if (!_assemblyRx.IsMatch(assemblyShortName))
                return false;
            var type = method.DeclaringType;
            var full = type?.FullName ?? string.Empty;
            return _typeRx.IsMatch(full);
        }
    }

    internal sealed class MethodExplicitRule : AotRuleEntry
    {
        private readonly Regex _assemblyRx;
        private readonly Regex _typeRx;
        private readonly Regex _nameRx;
        private readonly Regex _signatureRx;

        internal MethodExplicitRule(
            string assemblyPattern,
            string typePattern,
            string namePattern,
            string signaturePatternOrNull,
            bool wantAot)
            : base(wantAot)
        {
            _assemblyRx = AotRuleGlob.CompileGlob(assemblyPattern, ignoreCase: true);
            _typeRx = AotRuleGlob.CompileGlob(typePattern, ignoreCase: false);
            _nameRx = AotRuleGlob.CompileGlob(namePattern, ignoreCase: false);
            _signatureRx = signaturePatternOrNull != null
                ? AotRuleGlob.CompileGlob(signaturePatternOrNull, ignoreCase: false)
                : null;
        }

        internal override bool Matches(string assemblyShortName, MethodDef method)
        {
            if (!_assemblyRx.IsMatch(assemblyShortName))
                return false;
            var type = method.DeclaringType;
            var typeFull = type?.FullName ?? string.Empty;
            if (!_typeRx.IsMatch(typeFull))
                return false;
            if (!_nameRx.IsMatch(method.Name))
                return false;
            if (_signatureRx == null)
                return true;
            return _signatureRx.IsMatch(AotRuleDocument.MethodSignatureDisplay(method));
        }
    }

    /// <summary>
    /// Thrown when an AOT rule file is missing, malformed, or violates the schema.
    /// </summary>
    public sealed class AotRuleFileException : Exception
    {
        public AotRuleFileException(string path, string message)
            : base($"{path}: {message}") =>
            RuleFilePath = path;

        public string RuleFilePath { get; }
    }
}
