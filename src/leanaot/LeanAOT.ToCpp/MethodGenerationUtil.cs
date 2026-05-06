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

        public static string GetCppTypeNameAsFieldOrArgOrLoc(TypeSig typeSig, TypeNameRelaxLevel relaxLevel)
        {
            return GlobalServices.Inst.TypeNameService.GetCppTypeNameAsFieldOrArgOrLoc(typeSig, relaxLevel);
        }

        public static string CreateMethodForwardDeclaration(IMethod method, string methodName)
        {
            MethodSig methodSig = method.MethodSig;
            StringBuilder sb = new StringBuilder();
            sb.Append(GetResultTypeName(methodSig.RetType));
            sb.Append($" {methodName}(");
            bool first = true;
            if (methodSig.HasThis)
            {
                sb.Append(GetExactTypeName(MetaUtil.GetThisType(method)));
                first = false;
            }
            foreach (var param in methodSig.Params)
            {
                if (first)
                    first = false;
                else
                {
                    sb.Append(", ");
                }
                sb.Append(GetExactTypeName(param));
            }
            sb.Append(')');
            return sb.ToString();
        }

        public static string CreateMethodFunctionArgsWithoutCast(MethodDetail methodDetail)
        {
            var sb = new StringBuilder();
            foreach (var param in methodDetail.ParamsIncludeThis)
            {
                if (param.Index > 0)
                {
                    sb.Append(", ");
                }
                sb.Append($"{param.Name}");
            }
            return sb.ToString();
        }
    }
}
