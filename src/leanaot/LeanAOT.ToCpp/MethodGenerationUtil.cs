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
