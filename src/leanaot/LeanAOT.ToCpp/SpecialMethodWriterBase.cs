using dnlib.DotNet;
using System.Text;

namespace LeanAOT.ToCpp
{
    abstract class SpecialMethodWriterBase
    {
        protected readonly MethodDetail _method;
        protected readonly RuntimeApiEntry _entry;
        protected readonly ICorLibTypes _corlibTypes;
        protected readonly MetadataService _metadataService;
        protected readonly ManifestService _manifestService;

        protected readonly IMethodBodyCodeFilePart _methodBodyCodeFile;
        protected readonly ForwardDeclaration _forwardDeclaration;
        protected readonly CodeThunkZone _writer;

        protected readonly CodeThrunkWriter _bodyWriter;
        protected SpecialMethodWriterBase(MethodDetail method, IMethodBodyCodeFilePart methodBodyCodeFile, RuntimeApiEntry entry)
        {
            _method = method;
            _entry = entry;
            _corlibTypes = method.ModuleOfMethodDef.CorLibTypes;
            _metadataService = GlobalServices.Inst.MetadataService;
            _manifestService = GlobalServices.Inst.ManifestService;
            _methodBodyCodeFile = methodBodyCodeFile;
            _forwardDeclaration = methodBodyCodeFile.ForwardDeclaration;
            _writer = _methodBodyCodeFile.MethodWriter.CreateThunkCollection(_method.UniqueName);

            _bodyWriter = _writer.CreateThunk("method_body");
            _bodyWriter.AddLine();

            InitMethodVariables();
        }

        protected virtual string GetMethodName()
        {
            return _method.UniqueName;
        }

        protected virtual void InitMethodVariables()
        {
        }

        public void WriteCode()
        {
            if (_entry != null)
            {
                _forwardDeclaration.AddInclude(_entry.Header);
            }
            AddParameterAndReturnTypeForwardDeclarations();
            WriteMethodHeader();
            WriteMethodBody();
            WriteMethodEnd();
            _writer.MarkAsArchived();
        }


        protected void AddParameterAndReturnTypeForwardDeclarations()
        {
            foreach (var param in _method.ParamsIncludeThis)
            {
                _forwardDeclaration.AddTypeForwardDefine(param.Type);
            }
            _forwardDeclaration.AddTypeForwardDefine(_method.RetType);
        }

        void WriteMethodHeader()
        {
            _bodyWriter.AddLine($"// Method: {_method.FullName}");
            _bodyWriter.AddLine(_method.GenerateMethodDeclaring(GetMethodName()));
            _bodyWriter.BeginBlock();
        }

        void WriteMethodEnd()
        {
            _bodyWriter.EndBlock();
        }

        protected abstract void WriteMethodBody();
    }
}