using dnlib.DotNet;
using LeanAOT.Core;

namespace LeanAOT.ToCpp
{
    // The single policy for generated declarations, calls, bridges and invokers.
    // Runtime APIs (icalls/intrinsics included) always retain their Result ABI.
    static class ReturnProtocol
    {
        public static int Flags => (GlobalServices.Inst.Config.LightweightAotReturns ? 1 : 0) |
            (GlobalServices.Inst.Config.PackedInt32AotReturns ? 2 : 0);

        public static bool IsInt32(TypeSig type)
        {
            // Match the C++ type canonicalization; modifiers do not turn an
            // int32 into an enum or a byref, and cannot change its return ABI.
            type = type.RemovePinnedAndModifiers();
            return type.ElementType == ElementType.I4 || type.ElementType == ElementType.U4;
        }

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

        public static string ResultType(TypeSig type, bool enabled, bool packedInt32)
        {
            if (MetaUtil.IsVoidType(type))
                return enabled ? "leanclr::codegen::AotVoidResult" : "leanclr::RtResultVoid";
            string value = MethodGenerationUtil.GetAbiRelaxedTypeName(type);
            if (packedInt32 && IsInt32(type)) return $"leanclr::codegen::AotInt32Result<{value}>";
            return IsLightweight(type, enabled)
                ? $"leanclr::codegen::AotPointerResult<{value}>" : $"leanclr::RtResult<{value}>";
        }
    }
}
