using dnlib.DotNet;
using LeanAOT.Core;

namespace LeanAOT.GenerationPlan
{
    public sealed class GenericFallback
    {
        public string Caller { get; set; }
        public uint Token { get; set; }
        public string Target { get; set; }
        public string Reason { get; set; }
    }

    public partial class Manifest
    {
        private readonly HashSet<string> _genericKeys = new HashSet<string>(StringComparer.Ordinal);
        public List<GenericFallback> GenericFallbacks { get; } = new List<GenericFallback>();

        public static string GenericInstanceKey(IMethod method)
        {
            var def = method.ResolveMethodDef();
            if (def == null) return "";
            var ctx = MetaUtil.GetMethodGenericArgumentContext(method);
            return Atom(def.Module.Assembly.FullName) + def.MDToken.Raw.ToString("X8") + ":" +
                string.Join(";", (ctx.typeArgsStack ?? new List<TypeSig>()).Select(t => Atom(TypeKey(t)))) + ":" +
                string.Join(";", (ctx.methodArgsStack ?? new List<TypeSig>()).Select(t => Atom(TypeKey(t))));
        }

        private static string Atom(string value) => value.Length + ":" + value;
        public static string TypeIdentityKey(TypeSig type) => TypeKey(type);
        private static string TypeKey(TypeSig type)
        {
            if (type is GenericInstSig gi)
                return TypeKey(gi.GenericType) + "<" + string.Join(",", gi.GenericArguments.Select(t => Atom(TypeKey(t)))) + ">";
            if (type.Next != null) return type.ElementType + "(" + TypeKey(type.Next) + ")" + (type is ArraySig a ? a.Rank.ToString() : "");
            var def = type.ToTypeDefOrRef()?.ResolveTypeDef();
            return def == null ? type.FullName : ((int)type.ElementType) + ":" + Atom(def.Module.Assembly.FullName) + def.MDToken.Raw.ToString("X8");
        }

        public static bool SupportedGenericType(TypeSig type, int depth = 0)
        {
            if (type == null || depth > 16 || type.ContainsGenericParameter) return false;
            switch (type.ElementType)
            {
                case ElementType.Boolean: case ElementType.Char:
                case ElementType.I1: case ElementType.U1: case ElementType.I2: case ElementType.U2:
                case ElementType.I4: case ElementType.U4: case ElementType.I8: case ElementType.U8:
                case ElementType.R4: case ElementType.R8: case ElementType.I: case ElementType.U:
                case ElementType.String: case ElementType.Object: return true;
                case ElementType.Class: case ElementType.ValueType: return type.ToTypeDefOrRef().ResolveTypeDef() != null;
                case ElementType.SZArray: return SupportedGenericType(type.Next, depth + 1);
                case ElementType.GenericInst:
                    var gi = (GenericInstSig)type;
                    return gi.GenericType.TypeDefOrRef.ResolveTypeDef() != null && gi.GenericArguments.Count <= 255 &&
                        gi.GenericArguments.All(t => SupportedGenericType(t, depth + 1));
                default: return false;
            }
        }

        private void DiscoverGenericInstances(ManifestArgs args)
        {
            if (args.MaxGenericInstances < 0 || args.MaxGenericDepth < 0) throw new ArgumentOutOfRangeException(nameof(args));
            var queue = new Queue<(IMethod method, int depth)>();
            foreach (var plan in _assemblyPlans.Values)
                foreach (var root in plan.MethodPlans) queue.Enqueue((root.MethodDef, 0));
            var rejected = new HashSet<string>(StringComparer.Ordinal);
            var virtualClasses = new HashSet<string>(StringComparer.Ordinal);
            void Consider(IMethod target, IMethod caller, uint token, int depth)
            {
                var def = target.ResolveMethodDef();
                if (def == null) return;
                var ctx = MetaUtil.GetMethodGenericArgumentContext(target);
                if (!ctx.ContainsGenericArguments) return;
                string key = GenericInstanceKey(target);
                if (_genericKeys.Contains(key) || rejected.Contains(key)) return;
                string reason = null;
                if (!def.HasBody || def.IsAbstract || def.IsRuntime || def.IsPinvokeImpl || def.IsInternalCall) reason = "unsupported-body";
                else if (CalliPolicy.Fallback(def, ctx) is string calliReason) reason = calliReason;
                else if (def.CallingConvention == CallingConvention.VarArg) reason = "vararg-calling-convention";
                else if ((ctx.typeArgsStack?.Count ?? 0) != def.DeclaringType.GenericParameters.Count ||
                         (ctx.methodArgsStack?.Count ?? 0) != def.GenericParameters.Count ||
                         (ctx.typeArgsStack?.Count ?? 0) > 255 || (ctx.methodArgsStack?.Count ?? 0) > 255 ||
                         !(ctx.typeArgsStack ?? new List<TypeSig>()).Concat(ctx.methodArgsStack ?? new List<TypeSig>()).All(t => SupportedGenericType(t))) reason = "open-or-unsupported-arguments";
                else
                {
                    var attribute = def.CustomAttributes.FirstOrDefault(a => a.TypeFullName == "AotMethodAttribute");
                    if (attribute != null && !(bool)attribute.ConstructorArguments[0].Value) reason = "attribute-excluded";
                    else if (attribute == null && args.AotRulesEvaluator != null &&
                        !args.AotRulesEvaluator.ShouldIncludeByRules(def.Module.Assembly.Name, def) &&
                        !(args.PgoIncludeIndex?.Contains(def.Module.Assembly.Name, def) ?? false)) reason = "rule-excluded";
                }
                if (reason == null) reason = AotExceptionPolicy.FallbackReason(def, ctx);
                if (reason == null && depth > args.MaxGenericDepth) reason = "depth-budget";
                if (reason == null && _genericMethodPlans.Count >= args.MaxGenericInstances) reason = "instance-budget";
                if (reason != null)
                {
                    rejected.Add(key);
                    GenericFallbacks.Add(new GenericFallback { Caller = caller.FullName, Token = token, Target = target.FullName, Reason = reason });
                    return;
                }
                _genericKeys.Add(key);
                var instance = new GenericMethodPlan { Method = target, Key = key, Depth = depth };
                _genericMethodPlans.Add(instance);
                queue.Enqueue((target, depth));
                var cctor = def.DeclaringType.FindStaticConstructor();
                if (cctor != null && ctx.typeArgsStack != null && cctor != def)
                    Consider(MetaUtil.BindMethod(cctor, ctx), target, cctor.MDToken.Raw, depth + 1);
                // A constructed generic class needs its concrete vtable methods,
                // even when C# emits the call operand as the base declaration.
                // Open generic virtual methods still need concrete method args.
                if (def.IsInstanceConstructor && ctx.typeArgsStack != null &&
                    virtualClasses.Add(TypeIdentityKey(target.DeclaringType.ToTypeSig())))
                    foreach (var member in def.DeclaringType.Methods)
                        if (member.IsVirtual && !member.IsAbstract && !member.HasGenericParameters)
                            Consider(MetaUtil.BindMethod(member, new GenericArgumentContext(ctx.typeArgsStack, null)), target, member.MDToken.Raw, depth + 1);
            }
            while (queue.Count > 0)
            {
                var (caller, depth) = queue.Dequeue();
                var def = caller.ResolveMethodDef();
                if (def?.Body == null) continue;
                var ctx = MetaUtil.GetMethodGenericArgumentContext(caller);
                foreach (var instruction in def.Body.Instructions)
                {
                    if (instruction.Operand is IMethod target && target.IsMethod)
                        Consider(MetaUtil.BindMethod(target, ctx), caller, target.MDToken.Raw, depth + 1);
                    else if (instruction.Operand is IField field && field.IsField)
                    {
                        // A generic type can be used only through a static field;
                        // its cctor still needs its own closed AOT instance.
                        var bound = MetaUtil.InflateField(field, ctx);
                        if (bound.DeclaringType.ToTypeSig() is GenericInstSig type && !type.ContainsGenericParameter)
                        {
                            var cctor = type.GenericType.TypeDefOrRef.ResolveTypeDef()?.FindStaticConstructor();
                            if (cctor != null)
                                Consider(MetaUtil.BindMethod(cctor, new GenericArgumentContext(type.GenericArguments, null)), caller, field.MDToken.Raw, depth + 1);
                        }
                    }
                }
            }
        }
    }
}
