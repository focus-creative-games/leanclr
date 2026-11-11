using dnlib.DotNet;
using dnlib.DotNet.Emit;
using LeanAOT.Core;
using LeanAOT.GenerationPlan;

namespace LeanAOT.ToCpp
{
    // Canonical code is not a runtime instance. Only registered closed methods
    // can bind it, and every call must still pass its actual RtMethodInfo.
    public sealed class ReferenceGenericSharing
    {
        private readonly Dictionary<string, IMethod> _canonical = new(StringComparer.Ordinal);
        public List<(string Instance, string Body)> Bindings { get; } = new();

        public ReferenceGenericSharing(IEnumerable<GenericMethodPlan> plans, bool enabled)
        {
            if (!enabled) return;
            var independentBodies = new Dictionary<MethodDef, bool>();
            foreach (var plan in plans.OrderBy(p => p.Key, StringComparer.Ordinal))
            {
                var detail = GlobalServices.Inst.MetadataService.GetMethodDetail(plan.Method);
                var def = detail.MethodDef;
                var context = detail.GAC;
                var arguments = (context.typeArgsStack ?? new()).Concat(context.methodArgsStack ?? new()).ToList();
                if (arguments.Count == 0 || !arguments.All(IsReference) || def.DeclaringType.IsValueType ||
                    def.Body.HasExceptionHandlers || !Plain(detail.RetType) ||
                    detail.ParamsIncludeThis.Any(p => !Plain(p.Type)) || def.Body.Variables.Any(v => !Plain(detail.InflateType(v.Type)))) continue;
                // No token-dependent operations: typeof/box/newobj/fields/calls,
                // constrained calls and typed memory operations stay specialized.
                // This also prevents sharing a static metadata cache across contexts.
                if (!independentBodies.TryGetValue(def, out bool independent))
                    independentBodies.Add(def, independent = def.Body.Instructions.All(Independent));
                if (!independent) continue;
                var objectType = def.Module.CorLibTypes.Object;
                IMethodDefOrRef basis = def;
                if (def.DeclaringType.HasGenericParameters)
                {
                    var parent = new GenericInstSig((ClassOrValueTypeSig)def.DeclaringType.ToTypeSig(),
                        Enumerable.Repeat<TypeSig>(objectType, def.DeclaringType.GenericParameters.Count).ToList());
                    basis = new MemberRefUser(def.Module, def.Name, def.MethodSig, parent.ToTypeDefOrRef());
                }
                IMethod canonical = def.HasGenericParameters
                    ? new MethodSpecUser(basis, new GenericInstMethodSig(Enumerable.Repeat<TypeSig>(objectType, def.GenericParameters.Count).ToList())) : basis;
                _canonical.Add(plan.Key, canonical);
                Bindings.Add((plan.Key, Manifest.GenericInstanceKey(canonical)));
            }
        }

        private static bool IsReference(TypeSig type)
        {
            switch (type.ElementType)
            {
                case ElementType.Object: case ElementType.String: case ElementType.Class:
                case ElementType.Array: case ElementType.SZArray: return true;
                case ElementType.GenericInst: return !((GenericInstSig)type).GenericType.IsValueType;
                default: return false;
            }
        }
        private static bool Plain(TypeSig type) => IsReference(type) || type.ElementType == ElementType.Void ||
            (type.ElementType >= ElementType.Boolean && type.ElementType <= ElementType.R8);

        private static bool Independent(Instruction instruction)
        {
            switch (instruction.OpCode.OperandType)
            {
                case OperandType.InlineField: case OperandType.InlineMethod: case OperandType.InlineSig:
                case OperandType.InlineString: case OperandType.InlineTok: case OperandType.InlineType: return false;
            }
            switch (instruction.OpCode.Code)
            {
                case Code.Ldarga: case Code.Ldarga_S: case Code.Ldloca: case Code.Ldloca_S:
                case Code.Localloc: case Code.Cpblk: case Code.Initblk: case Code.Arglist: case Code.Jmp: return false;
                // Array element helpers can acquire metadata even when the IL
                // opcode itself has no token. Keep array manipulation specialized.
                case Code.Ldlen: case Code.Ldelem_I1: case Code.Ldelem_U1: case Code.Ldelem_I2: case Code.Ldelem_U2:
                case Code.Ldelem_I4: case Code.Ldelem_U4: case Code.Ldelem_I8: case Code.Ldelem_I:
                case Code.Ldelem_R4: case Code.Ldelem_R8: case Code.Ldelem_Ref:
                case Code.Stelem_I: case Code.Stelem_I1: case Code.Stelem_I2: case Code.Stelem_I4: case Code.Stelem_I8:
                case Code.Stelem_R4: case Code.Stelem_R8: case Code.Stelem_Ref: return false;
                default: return true;
            }
        }

        public IMethod Canonicalize(IMethod method) => _canonical.Count != 0 && _canonical.TryGetValue(Manifest.GenericInstanceKey(method), out var canonical) ? canonical : method;
    }
}
