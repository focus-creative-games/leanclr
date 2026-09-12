using dnlib.DotNet;
using LeanAOT.Core;
using System.Text;

namespace LeanAOT.ToCpp
{

    static class MethodGenerationUtil
    {
        public static string GetResultTypeName(TypeSig typeSig)
        {
            if (MetaUtil.IsVoidType(typeSig))
            {
                return "leanclr::RtResultVoid";
            }
            return $"leanclr::RtResult<{GlobalServices.Inst.TypeNameService.GetCppTypeNameAsFieldOrArgOrLoc(typeSig, TypeNameRelaxLevel.AbiRelaxed)}>";
        }

        public static string GetParameterName(Parameter param)
        {
            if (param.IsHiddenThisParameter)
            {
                return "___this";
            }
            // Implement logic to generate a C++ parameter name from the .NET parameter
            return $"___p{param.Index}";
        }

        public static string GetExactTypeName(TypeSig typeSig)
        {
            return GlobalServices.Inst.TypeNameService.GetCppTypeNameAsFieldOrArgOrLoc(typeSig, TypeNameRelaxLevel.Exactly);
        }

        public static string GetAbiRelaxedTypeName(TypeSig typeSig)
        {
            return GlobalServices.Inst.TypeNameService.GetCppTypeNameAsFieldOrArgOrLoc(typeSig, TypeNameRelaxLevel.AbiRelaxed);
        }

        public static string GetCppTypeNameAsFieldOrArgOrLoc(TypeSig typeSig, TypeNameRelaxLevel relaxLevel)
        {
            return GlobalServices.Inst.TypeNameService.GetCppTypeNameAsFieldOrArgOrLoc(typeSig, relaxLevel);
        }

        private static readonly string[] IntPtrBackedRuntimeHandleInternalPrefixes =
        {
            "System_Private_CoreLib_System_RuntimeFieldHandleInternal_",
            "System_Private_CoreLib_System_RuntimeMethodHandleInternal_",
        };

        private static bool HasIntPtrBackedRuntimeHandleInternalPrefix(string typeName)
        {
            return IntPtrBackedRuntimeHandleInternalPrefixes.Any(prefix => typeName.StartsWith(prefix, StringComparison.Ordinal));
        }

        public static bool IsIntPtrBackedRuntimeHandleInternalTypeName(string typeName)
        {
            typeName = typeName?.Trim();
            return typeName != null &&
                   !typeName.EndsWith("*", StringComparison.Ordinal) &&
                   HasIntPtrBackedRuntimeHandleInternalPrefix(typeName);
        }

        public static bool IsIntPtrBackedRuntimeHandleInternalPointerTypeName(string typeName)
        {
            typeName = typeName?.Trim();
            return typeName != null &&
                   typeName.EndsWith("*", StringComparison.Ordinal) &&
                   HasIntPtrBackedRuntimeHandleInternalPrefix(typeName.Substring(0, typeName.Length - 1).TrimEnd());
        }

        public static bool IsTypedReferenceValueTypeName(string typeName)
        {
            typeName = typeName?.Trim();
            return typeName != null &&
                   !typeName.EndsWith("*", StringComparison.Ordinal) &&
                   typeName.StartsWith("System_Private_CoreLib_System_TypedReference_", StringComparison.Ordinal);
        }

        public static string GetResultWrapFunctionName(string retTypeName)
        {
            if (IsIntPtrBackedRuntimeHandleInternalTypeName(retTypeName))
            {
                return $"{ConstStrings.CodegenNamespace}::wrap_intptr_result_to";
            }
            if (IsTypedReferenceValueTypeName(retTypeName))
            {
                return $"{ConstStrings.CodegenNamespace}::wrap_typed_reference_result_to";
            }
            return $"{ConstStrings.CodegenNamespace}::wrap_result_to";
        }

        public static string CreateMethodExactArgs(MethodDetail methodDetail, bool includeArgName)
        {
            return string.Join(", ", methodDetail.ParamsIncludeThis.Select(param => $"{GetExactTypeName(param.Type)}{(includeArgName ? $" {param.Name}" : "")}"));
        }

        public static string CreateMethodRelaxedArgs(MethodDetail methodDetail, bool includeArgName)
        {
            return string.Join(", ", methodDetail.ParamsIncludeThis.Select(param => $"{GetCppTypeNameAsFieldOrArgOrLoc(param.Type, TypeNameRelaxLevel.AbiRelaxed)}{(includeArgName ? $" {param.Name}" : "")}"));
        }

        public static string CreateMethodFunctionArgsWithoutCast(MethodDetail methodDetail)
        {
            return string.Join(", ", methodDetail.ParamsIncludeThis.Select(param => $"{param.Name}"));
        }

        public static string CreateRelaxedMethodFunctionTypeDeclaring(MethodDetail methodDetail)
        {
            return $"{GetResultTypeName(methodDetail.RetType)} (*)({CreateMethodRelaxedArgs(methodDetail, true)}){ConstStrings.CppFunctionNoexcept}";
        }
    }
}
