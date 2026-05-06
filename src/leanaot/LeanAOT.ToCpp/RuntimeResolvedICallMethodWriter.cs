using LeanAOT.Core;
using System.Text;

namespace LeanAOT.ToCpp
{
    class RuntimeResolvedICallMethodWriter : SpecialMethodWriterBase
    {
        public RuntimeResolvedICallMethodWriter(MethodDetail method, IMethodBodyCodeFilePart methodBodyCodeFile) : base(method, methodBodyCodeFile, null)
        {

        }

        protected override void WriteMethodBody()
        {
            string icallMethodType = _method.CreateNativeMethodFunctionTypeDefine("");
            _bodyWriter.AddLine($"static auto __icall_method_pointer = {ConstStrings.CodegenNamespace}::resolve_internal_call(\"{NameUtil.GetICallFullMethodName(_method.MethodDef)}\");");
            _bodyWriter.AddLine($"assert(__icall_method_pointer != nullptr);");
            if (_method.IsVoidReturn)
            {
                _bodyWriter.AddLine($"(({icallMethodType})__icall_method_pointer)({MethodGenerationUtil.CreateMethodFunctionArgsWithoutCast(_method)});");
                _bodyWriter.AddLine($"{ConstStrings.CodegenReturnVoid}();");
            }
            else
            {
                _bodyWriter.AddLine($"{ConstStrings.CodegenReturn}((({icallMethodType})__icall_method_pointer)({MethodGenerationUtil.CreateMethodFunctionArgsWithoutCast(_method)}));");
            }
        }
    }
}