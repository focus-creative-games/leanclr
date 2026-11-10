using dnlib.DotNet;
using LeanAOT.Core;

namespace LeanAOT.ToCpp
{
    // The single policy for generated declarations, calls, bridges and invokers.
    // Runtime APIs (icalls/intrinsics included) always retain their Result ABI.
    static class ReturnProtocol
    {
        public static bool IsLightweight(TypeSig type, bool enabled)
        {
            if (!enabled) return false;
            switch (type.ElementType)
            {
                case ElementType.Void: case ElementType.Object: case ElementType.String:
                case ElementType.Class: case ElementType.Array: case ElementType.SZArray: return true;
                case ElementType.GenericInst: return !((GenericInstSig)type).GenericType.IsValueType;
                default: return false; // In particular, never tag arbitrary managed byrefs/native pointers.
            }
        }

        public static string ResultType(TypeSig type, bool enabled)
        {
            if (MetaUtil.IsVoidType(type))
                return enabled ? "leanclr::codegen::AotVoidResult" : "leanclr::RtResultVoid";
            string value = MethodGenerationUtil.GetAbiRelaxedTypeName(type);
            return IsLightweight(type, enabled)
                ? $"leanclr::codegen::AotPointerResult<{value}>" : $"leanclr::RtResult<{value}>";
        }
    }
}
