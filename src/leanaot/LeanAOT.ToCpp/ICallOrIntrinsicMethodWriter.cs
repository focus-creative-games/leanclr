using System.Text;

namespace LeanAOT.ToCpp
{

    class ICallOrIntrinsicMethodWriter : SpecialMethodWriterBase
    {

        public ICallOrIntrinsicMethodWriter(MethodDetail method, IMethodBodyCodeFilePart methodBodyCodeFile, RuntimeApiEntry entry) : base(method, methodBodyCodeFile, entry)
        {

        }

        protected override void WriteMethodBody()
        {
            var argsStr = MethodGenerationUtil.CreateMethodFunctionArgsWithoutCast(_method);
            string namespaceStr = _entry.MethodKind == MethodKind.ICall || _entry.MethodKind == MethodKind.ICallNewObj ? "leanclr::icalls" : "leanclr::intrinsics";
            string funcFullName = $"{namespaceStr}::{_entry.Func}";
            if (_method.IsVoidReturn)
            {
                _bodyWriter.AddLine($"return (({_method.CreateMethodFunctionTypeDefineWithoutName()}){funcFullName})({argsStr});");
            }
            else
            {
                string relaxRetTypeName = MethodGenerationUtil.GetCppTypeNameAsFieldOrArgOrLoc(_method.RetType, TypeNameRelaxLevel.AbiRelaxed);
                _bodyWriter.AddLine($"using __RetType = leanclr::core::function_return<decltype({funcFullName})>::type;");
                _bodyWriter.AddLine($"return (({_method.CreateOverrideRetTypeRelaxMethodFunctionTypeDefine("", "__RetType")}){funcFullName})({argsStr}).cast<{relaxRetTypeName}>();");
            }
        }
    }
}