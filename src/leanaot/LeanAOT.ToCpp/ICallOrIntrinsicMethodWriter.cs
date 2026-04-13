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
            var argsStr = CreateMethodFunctionArgsWithCast();
            string namespaceStr = _entry.MethodKind == MethodKind.ICall || _entry.MethodKind == MethodKind.ICallNewObj ? "leanclr::icalls" : "leanclr::intrinsics";
            _bodyWriter.AddLine($"return (({_method.CreateMethodFunctionTypeDefineWithoutName()}){namespaceStr}::{_entry.Func})({argsStr});");
        }
    }
}