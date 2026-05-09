namespace LeanAOT.ToCpp
{
    class VirtualMethodWriter : SpecialMethodWriterBase
    {
        public VirtualMethodWriter(MethodDetail method, IMethodBodyCodeFilePart methodBodyCodeFile) : base(method, methodBodyCodeFile, null)
        {
        }

        protected override string GetMethodName()
        {
            return _method.VirtualMethodUniqueName;
        }

        protected override void WriteMethodBody()
        {
            var thisParam = _method.ParamsIncludeThis[0];
            string thisParamVarName = thisParam.Name;
            string thisParamTypeName = MethodGenerationUtil.GetExactTypeName(thisParam.Type);
            _bodyWriter.AddLine($"{thisParamVarName} = ({thisParamTypeName})(({ConstStrings.ObjectPtrTypeName}){thisParamVarName} + 1);");
            _bodyWriter.AddLine($"return {_method.UniqueName}({string.Join(", ", _method.ParamsIncludeThis.Select(p => p.Name))});");
        }
    }
}