using dnlib.DotNet;
using dnlib.DotNet.Emit;

namespace LeanAOT.GenerationPlan
{
    public static class AotExceptionPolicy
    {
        private static bool SnapshotType(TypeSig type)
        {
            switch (type.ElementType)
            {
                case ElementType.Void: case ElementType.Boolean: case ElementType.Char:
                case ElementType.I1: case ElementType.U1: case ElementType.I2: case ElementType.U2:
                case ElementType.I4: case ElementType.U4: case ElementType.I8: case ElementType.U8:
                case ElementType.R4: case ElementType.R8: case ElementType.String: case ElementType.Object:
                case ElementType.Class: case ElementType.SZArray: case ElementType.Array: return true;
                default: return false;
            }
        }

        public static string FallbackReason(MethodDef method)
        {
            if (!method.HasBody || !method.Body.HasExceptionHandlers) return null;
            if (method.HasGenericParameters || method.DeclaringType.HasGenericParameters) return "eh-generic-context";
            if (method.DeclaringType.IsValueType && !method.IsStatic) return "eh-value-this";
            if (method.Body.Variables.Count > 0 && !method.Body.InitLocals) return "eh-uninitialized-locals";
            if (method.Parameters.Count + method.Body.Variables.Count > 128) return "eh-snapshot-budget";
            if (!SnapshotType(method.ReturnType) || method.MethodSig.Params.Any(p => !SnapshotType(p)) ||
                method.Body.Variables.Any(l => !SnapshotType(l.Type))) return "eh-snapshot-type";
            foreach (var eh in method.Body.ExceptionHandlers)
            {
                if (eh.HandlerType != ExceptionHandlerType.Catch && eh.HandlerType != ExceptionHandlerType.Finally) return "eh-filter-or-fault";
                if (eh.HandlerType == ExceptionHandlerType.Finally)
                    foreach (var other in method.Body.ExceptionHandlers)
                        if (other != eh && other.TryStart.Offset >= eh.HandlerStart.Offset &&
                            (eh.HandlerEnd == null || other.TryStart.Offset < eh.HandlerEnd.Offset)) return "eh-handler-inside-finally";
            }
            foreach (var inst in method.Body.Instructions)
                switch (inst.OpCode.Code)
                {
                    case Code.Ldloca: case Code.Ldloca_S: case Code.Ldarga: case Code.Ldarga_S:
                    case Code.Ldflda: case Code.Ldsflda: case Code.Ldelema: case Code.Unbox:
                    case Code.Localloc: case Code.Cpblk: case Code.Initblk: case Code.Calli:
                    case Code.Mkrefany: case Code.Refanyval: case Code.Arglist: case Code.Jmp:
                    case Code.Tailcall: return "eh-address-or-native-control";
                }
            return null;
        }
    }
}
