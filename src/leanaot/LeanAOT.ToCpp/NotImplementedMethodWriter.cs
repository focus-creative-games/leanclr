namespace LeanAOT.ToCpp
{
    class NotImplementedMethodWriter : SpecialMethodWriterBase
    {
        public NotImplementedMethodWriter(MethodDetail method, IMethodBodyCodeFilePart methodBodyCodeFile) : base(method, methodBodyCodeFile, null)
        {

        }

        protected override void WriteMethodBody()
        {
            _bodyWriter.AddLine($"printf(\"{_method.FullName} is not implemented\\n\");");
            _bodyWriter.AddLine($"LEANCLR_CODEGEN_RETURN_NOT_IMPLEMENTED_ERROR();");
        }
    }
}