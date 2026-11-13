using dnlib.DotNet;
using dnlib.DotNet.Emit;
using LeanAOT.Core;

namespace LeanAOT.GenerationPlan
{
    public static class AotExceptionPolicy
    {
        private static bool SnapshotType(TypeSig type)
        {
            while (type.ElementType == ElementType.CModOpt || type.ElementType == ElementType.CModReqd) type = type.Next;
            if (type.ContainsGenericParameter) return false;
            switch (type.ElementType)
            {
                case ElementType.Void: case ElementType.Boolean: case ElementType.Char:
                case ElementType.I1: case ElementType.U1: case ElementType.I2: case ElementType.U2:
                case ElementType.I4: case ElementType.U4: case ElementType.I8: case ElementType.U8:
                case ElementType.R4: case ElementType.R8: case ElementType.String: case ElementType.Object:
                case ElementType.Class: case ElementType.SZArray: case ElementType.Array: return true;
                case ElementType.FnPtr: return true;
                case ElementType.GenericInst: return !((GenericInstSig)type).GenericType.IsValueType;
                default: return false;
            }
        }

        public static string FallbackReason(MethodDef method, GenericArgumentContext context = null)
        {
            if (!method.HasBody || !method.Body.HasExceptionHandlers) return null;
            if ((method.HasGenericParameters || method.DeclaringType.HasGenericParameters) &&
                (context == null || (context.typeArgsStack?.Count ?? 0) != method.DeclaringType.GenericParameters.Count ||
                 (context.methodArgsStack?.Count ?? 0) != method.GenericParameters.Count)) return "eh-generic-context";
            if (method.DeclaringType.IsValueType && !method.IsStatic) return "eh-value-this";
            if (method.Body.Variables.Count > 0 && !method.Body.InitLocals) return "eh-uninitialized-locals";
            if (method.Parameters.Count + method.Body.Variables.Count > 128) return "eh-snapshot-budget";
            bool Safe(TypeSig type) => SnapshotType(MetaUtil.Inflate(type, context));
            if (!Safe(method.ReturnType) || method.MethodSig.Params.Any(p => !Safe(p)) ||
                method.Body.Variables.Any(l => !Safe(l.Type))) return "eh-snapshot-type";
            foreach (var eh in method.Body.ExceptionHandlers)
            {
                if (eh.HandlerType != ExceptionHandlerType.Catch && eh.HandlerType != ExceptionHandlerType.Finally) return "eh-filter-or-fault";
                if (eh.HandlerType == ExceptionHandlerType.Finally)
                    foreach (var other in method.Body.ExceptionHandlers)
                        if (other != eh && other.TryStart.Offset >= eh.HandlerStart.Offset &&
                            (eh.HandlerEnd == null || other.TryStart.Offset < eh.HandlerEnd.Offset)) return "eh-handler-inside-finally";
            }
            foreach (var inst in method.Body.Instructions)
            {
                // Catch handlers execute only after interpreter handoff. Their
                // address-taking instructions do not create aliases on the AOT
                // normal path and must not disqualify an otherwise safe snapshot.
                if (method.Body.ExceptionHandlers.Any(eh => eh.HandlerType == ExceptionHandlerType.Catch &&
                    inst.Offset >= eh.HandlerStart.Offset && (eh.HandlerEnd == null || inst.Offset < eh.HandlerEnd.Offset))) continue;
                switch (inst.OpCode.Code)
                {
                    case Code.Ldloca: case Code.Ldloca_S: case Code.Ldarga: case Code.Ldarga_S:
                    case Code.Ldflda: case Code.Ldsflda: case Code.Ldelema: case Code.Unbox:
                    case Code.Localloc: case Code.Cpblk: case Code.Initblk:
                    case Code.Mkrefany: case Code.Refanyval: case Code.Arglist: case Code.Jmp:
                    case Code.Tailcall: return "eh-address-or-native-control";
                }
            }
            return null;
        }
    }
}
