using dnlib.DotNet;
using LeanAOT.Core;
using LeanAOT.GenerationPlan;
using System.Text;

namespace LeanAOT.ToCpp
{
    // All generic bodies/identities belong to the generation, not an assembly.
    // Sorted 64-bit keys are a compact immutable index: no runtime allocation,
    // module loading or reconstruction of generic metadata at registration.
    sealed class GenericRegistrationCppCodeFile
    {
        private readonly CodeWriter _writer;
        private readonly CodeThrunkWriter _body;
        private readonly ForwardDeclaration _declarations;
        private readonly IReadOnlyList<GenericMethodPlan> _plans;
        private readonly Dictionary<string, string> _identityArrays = new Dictionary<string, string>(StringComparer.Ordinal);
        private int _nextIdentity;

        public GenericRegistrationCppCodeFile(string path, IReadOnlyList<GenericMethodPlan> plans)
        {
            _writer = new CodeWriter(path);
            _declarations = new ForwardDeclaration(_writer.CreateThunkCollection("declarations"));
            _declarations.AddInclude(ConstStrings.CodegenCommonHeaderFileName);
            _body = _writer.CreateThunkCollection("registration").CreateThunk("entries");
            _plans = plans;
        }

        private static void Byte(ref ulong hash, byte value) { hash = unchecked((hash ^ value) * 1099511628211UL); }
        private static void UInt(ref ulong hash, uint value)
        {
            for (int i = 0; i < 4; ++i) { Byte(ref hash, (byte)value); value >>= 8; }
        }
        private static void Name(ref ulong hash, string value)
        {
            foreach (byte b in Encoding.UTF8.GetBytes(value)) Byte(ref hash, b);
            Byte(ref hash, 0);
        }
        private static TypeDef Definition(TypeSig type) => type is GenericInstSig generic
            ? generic.GenericType.TypeDefOrRef.ResolveTypeDef() : type is ClassOrValueTypeSig named ? named.TypeDefOrRef.ResolveTypeDef() : null;
        private static IList<TypeSig> Children(TypeSig type) => type is GenericInstSig generic ? generic.GenericArguments :
            type.ElementType == ElementType.SZArray ? new[] { type.Next } : null;
        private static string Utf8Literal(string value)
        {
            var result = new StringBuilder("\"");
            foreach (byte b in Encoding.UTF8.GetBytes(value))
            {
                if (b == (byte)'"' || b == (byte)'\\') result.Append('\\').Append((char)b);
                else if (b >= 32 && b <= 126) result.Append((char)b);
                else result.Append('\\').Append(Convert.ToString(b, 8).PadLeft(3, '0'));
            }
            return result.Append('"').ToString();
        }
        private static void Arguments(ref ulong hash, IList<TypeSig> args)
        {
            UInt(ref hash, (uint)(args?.Count ?? 0));
            if (args != null) foreach (var type in args) TypeHash(ref hash, type);
        }
        private static void TypeHash(ref ulong hash, TypeSig type)
        {
            UInt(ref hash, (uint)type.ElementType);
            var def = Definition(type);
            if (def != null) { Name(ref hash, def.Module.Assembly.Name); UInt(ref hash, def.MDToken.Raw); }
            if (type.ElementType == ElementType.GenericInst) Arguments(ref hash, Children(type));
            else if (type.ElementType == ElementType.SZArray) TypeHash(ref hash, type.Next);
        }
        private static ulong Key(IMethod method)
        {
            ulong hash = 14695981039346656037UL;
            var def = method.ResolveMethodDef();
            var ctx = MetaUtil.GetMethodGenericArgumentContext(method);
            Name(ref hash, def.Module.Assembly.Name);
            UInt(ref hash, def.MDToken.Raw);
            Arguments(ref hash, ctx.typeArgsStack);
            Arguments(ref hash, ctx.methodArgsStack);
            return hash;
        }
        private static string Shape(TypeSig type)
        {
            return Manifest.TypeIdentityKey(type);
        }
        private string Identities(IList<TypeSig> types)
        {
            if (types == null || types.Count == 0) return "nullptr";
            string key = string.Join(";", types.Select(t => { string shape = Shape(t); return shape.Length + ":" + shape; }));
            if (_identityArrays.TryGetValue(key, out var existing)) return existing;
            var values = new List<string>();
            foreach (var type in types)
            {
                var children = Children(type);
                string childName = Identities(children);
                var def = Definition(type);
                string module = def == null ? "nullptr" : Utf8Literal(def.Module.Assembly.Name);
                values.Add($"{{ static_cast<leanclr::metadata::RtElementType>({(int)type.ElementType}), {module}, 0x{(def?.MDToken.Raw ?? 0):X8}, {childName}, {children?.Count ?? 0} }}");
            }
            string name = "s_generic_identity_" + _nextIdentity++;
            _body.AddLine($"static const leanclr::metadata::RtAotTypeIdentity {name}[] = {{ {string.Join(", ", values)} }};");
            _identityArrays.Add(key, name);
            return name;
        }
        public void Generate()
        {
            if (_plans.Count == 0) return;
            var entries = new List<string>();
            foreach (var plan in _plans.OrderBy(p => Key(p.Method)).ThenBy(p => p.Key, StringComparer.Ordinal))
            {
                var method = plan.Method;
                var detail = GlobalServices.Inst.MetadataService.GetMethodDetail(method);
                var ctx = MetaUtil.GetMethodGenericArgumentContext(method);
                string classes = Identities(ctx.typeArgsStack), methods = Identities(ctx.methodArgsStack);
                _declarations.AddMethodForwardDeclaration(method);
                var invoker = GlobalServices.Inst.InvokerService.GetInvoker(method);
                _declarations.AddInvokerForwardDeclaration(invoker);
                string owner = Utf8Literal(detail.MethodDef.Module.Assembly.Name);
                entries.Add($"{{ 0x{Key(method):X16}ULL, {owner}, {{ 0x{detail.MethodDef.MDToken.Raw:X8}, (leanclr::metadata::RtManagedMethodPointer){detail.UniqueName}, (leanclr::metadata::RtManagedMethodPointer){(detail.ShouldGenerateVirtualMethod ? detail.VirtualMethodUniqueName : detail.UniqueName)}, {invoker.name} }}, {classes}, {methods}, {ctx.typeArgsStack?.Count ?? 0}, {ctx.methodArgsStack?.Count ?? 0}, {(GlobalServices.Inst.Config.LightweightAotReturns ? 1 : 0)} }}");
            }
            _body.AddLine("extern const leanclr::metadata::RtAotGenericMethodData g_aot_generic_methods[] = {");
            foreach (string entry in entries) _body.AddLine(entry + ",");
            _body.AddLine("};");
        }
        public void Save() => _writer.Save();
    }
}
