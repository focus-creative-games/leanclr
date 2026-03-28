using dnlib.DotNet;
using LeanAOT.GenerationPlan;

namespace LeanAOT.ToCpp
{
    // class CodeFileBase
    // {
    //     protected C
    // }

    class ModuleRegistrationCppCodeFile
    {
        private readonly CodeWriter _totalWriter;

        private readonly CodeThunkZone _headCollection;
        private readonly CodeThrunkWriter _includeWriter;

        private readonly ForwardDeclaration _forwardDeclaration;

        private readonly CodeThunkZone _implCollection;
        private readonly CodeThrunkWriter _implWriter;

        private readonly ModuleDef _mod;
        private readonly AssemblyPlan _plan;

        public ModuleRegistrationCppCodeFile(string filePath, AssemblyPlan plan)
        {
            _totalWriter = new CodeWriter(filePath);
            _plan = plan;
            _mod = plan.Module;
            _headCollection = _totalWriter.CreateThunkCollection("head");
            _includeWriter = _headCollection.CreateThunk("includes");
            _forwardDeclaration = new ForwardDeclaration(_totalWriter.CreateThunkCollection("forward_declarations"));
            _implCollection = _totalWriter.CreateThunkCollection("implementations");
            _implWriter = _implCollection.CreateThunk("implementations");
        }

        public void Generate()
        {
            AddIncludes();
            AddGlobalVariableDeclaration();
            AddModuleMethodDefDatasDeclaration();
            AddModuleInitializationMethod();
            AddModuleDataDeclaration();
        }

        private string GetMethodDefDatasVariableName()
        {
            return $"s_method_def_datas_{ModuleGenerationUtil.GetStandardizedModuleNameWithoutExt(_mod)}";
        }

        private void AddIncludes()
        {
            _includeWriter.AddLine($"#include \"{ModuleGenerationUtil.GetModuleRegistrationHeaderFileNameWithExt(_mod)}\"");
        }

        private void AddGlobalVariableDeclaration()
        {
            _implWriter.AddLine();
            _implWriter.AddLine($"{ConstStrings.ModulePtrTypeName} {ModuleGenerationUtil.GetModuleGlobalVariableName(_mod)} = nullptr;");
        }

        private void AddModuleInitializationMethod()
        {
            _implWriter.AddLine();
            _implWriter.AddLine($"void {ModuleGenerationUtil.GetModuleInitializeMethodName(_mod)}({ConstStrings.ModulePtrTypeName} mod){ConstStrings.CppFunctionNoexcept}");
            _implWriter.AddLine("{");
            _implWriter.IncreaseIndent();
            _implWriter.AddLine($"{ModuleGenerationUtil.GetModuleGlobalVariableName(_mod)} = mod;");
            _implWriter.DecreaseIndent();
            _implWriter.AddLine("}");
        }

        private void AddModuleMethodDefDatasDeclaration()
        {
            _implWriter.AddLine();
            _implWriter.AddLine($"static {ConstStrings.MethodDefDataTypeName} {GetMethodDefDatasVariableName()}[] = {{");
            _implWriter.IncreaseIndent();
            var invokerService = GlobalServices.Inst.InvokerService;
            if (_plan.MethodPlans.Count > 0)
            {
                foreach (var methodPlan in _plan.MethodPlans)
                {
                    MethodDef method = methodPlan.MethodDef;
                    _forwardDeclaration.AddMethodForwardDeclaration(method);
                    MethodInvokerInfo notVirtualInvoker = invokerService.GetNotVirtualInvoker(method);
                    _forwardDeclaration.AddInvokerForwardDeclaration(notVirtualInvoker);
                    MethodInvokerInfo virtualInvoker = invokerService.GetVirtualInvoker(method);
                    _forwardDeclaration.AddInvokerForwardDeclaration(virtualInvoker);
                    MethodDetail md = GlobalServices.Inst.MetadataService.GetMethodDetail(method);
                    _implWriter.AddLine($"{{ 0x{method.MDToken.ToInt32():X8}, ({ConstStrings.ManagedMethodPointerTypeName}){md.UniqueName}, {notVirtualInvoker.name}, {virtualInvoker.name} }},");
                }
            }
            else
            {
                _implWriter.AddLine($"{{ 0x0, nullptr, nullptr, nullptr}},");
            }
            _implWriter.DecreaseIndent();
            _implWriter.AddLine("};");
        }

        private void AddModuleDataDeclaration()
        {
            _implWriter.AddLine();
            _implWriter.AddLine($"{ConstStrings.ModuleDataTypeName} {ModuleGenerationUtil.GetModuleGlobalDataVariableName(_mod)} = {{");
            _implWriter.IncreaseIndent();
            _implWriter.AddLine($"\"{ModuleGenerationUtil.GetModuleNameNoExt(_mod)}\",");
            _implWriter.AddLine($"{ModuleGenerationUtil.GetModuleInitializeMethodName(_mod)},");
            _implWriter.AddLine($"{GetMethodDefDatasVariableName()},");
            _implWriter.AddLine($"{_plan.MethodPlans.Count},");
            _implWriter.DecreaseIndent();
            _implWriter.AddLine("};");
        }

        public void Save()
        {
            _totalWriter.Save();
        }
    }
}
