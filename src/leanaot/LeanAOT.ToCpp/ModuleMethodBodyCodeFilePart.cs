using dnlib.DotNet;

namespace LeanAOT.ToCpp
{


    class ModuleMethodBodyCodeFilePart : IMethodBodyCodeFilePart
    {
        private string _filePath;

        private CodeWriter _totalWriter;

        private ForwardDeclaration _forwardDeclaration;

        public ForwardDeclaration ForwardDeclaration => _forwardDeclaration;

        public CodeWriter MethodWriter => _totalWriter;



        public ModuleMethodBodyCodeFilePart(string filePath, ModuleDef mod)
        {
            _filePath = filePath;
            _totalWriter = new CodeWriter(filePath);
            _forwardDeclaration = new ForwardDeclaration(_totalWriter.CreateThunkCollection("forward_declarations"));
            if (mod == null) _forwardDeclaration.AddInclude(ConstStrings.CodegenCommonHeaderFileName);
            else _forwardDeclaration.AddCommonIncludes(mod);
        }

        public bool ExceedsMaxSize()
        {
            return _totalWriter.CodeSize > GlobalServices.Inst.Config.MaxCodeSizeOfCppFile;
        }


        public void Save()
        {
            _totalWriter.Save();
        }
    }
}
