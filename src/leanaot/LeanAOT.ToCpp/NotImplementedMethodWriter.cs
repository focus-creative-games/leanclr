namespace LeanAOT.ToCpp
{
    class NotImplementedMethodWriter : SpecialMethodWriterBase
    {
        public NotImplementedMethodWriter(MethodDetail method, IMethodBodyCodeFilePart methodBodyCodeFile) : base(method, methodBodyCodeFile, null)
        {

        }

        protected override void WriteMethodBody()
        {
            _bodyWriter.AddLine("assert(false);");
            _bodyWriter.AddLine($"return leanclr::RtErr::NotImplemented;");
        }
    }
}