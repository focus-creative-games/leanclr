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
            _bodyWriter.AddLine($"static leanclr::vm::InternalCallFunction __icall_method_pointer = nullptr;");
            _bodyWriter.AddLine("if (__icall_method_pointer == nullptr)");
            _bodyWriter.BeginBlock();
            _bodyWriter.AddLine($"__icall_method_pointer = {ConstStrings.CodegenNamespace}::resolve_internal_call(\"{NameUtil.GetICallFullMethodName(_method.MethodDef)}\");");
            _bodyWriter.AddLine("if (__icall_method_pointer == nullptr)");
            _bodyWriter.BeginBlock();
            _bodyWriter.AddLine($"{VmFunctionNames.RET_ERROR}({ConstStrings.CodegenNamespace}::raise_internal_call_entry_not_found_error(\"{NameUtil.GetICallFullMethodName(_method.MethodDef)}\"));");
            _bodyWriter.EndBlock();
            _bodyWriter.EndBlock();
            if (_method.IsVoidReturn)
            {
                _bodyWriter.AddLine($"(({icallMethodType})__icall_method_pointer)({MethodGenerationUtil.CreateMethodFunctionArgsWithoutCast(_method)});");
                _bodyWriter.AddLine($"{VmFunctionNames.RET_VOID}();");
            }
            else
            {
                _bodyWriter.AddLine($"{VmFunctionNames.RET_VALUE}((({icallMethodType})__icall_method_pointer)({MethodGenerationUtil.CreateMethodFunctionArgsWithoutCast(_method)}));");
            }
        }
    }
}