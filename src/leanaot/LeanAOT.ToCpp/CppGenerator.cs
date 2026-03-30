using dnlib.DotNet;
using LeanAOT.Core;
using LeanAOT.GenerationPlan;

namespace LeanAOT.ToCpp
{

    class AssemblyGenerationContext
    {
        public GenerationConfig config;

        public AssemblyPlan plan;

        private int fileIndex = 0;

        private readonly List<ModuleMethodBodyCodeFilePart> _methodBodyCodeFiles = new List<ModuleMethodBodyCodeFilePart>();

        public ModuleMethodBodyCodeFilePart CreateMethodBodyCodeFileForNextFile()
        {
            fileIndex++;
            var methodBodyCodeFile = new ModuleMethodBodyCodeFilePart($"{config.outputCodeDir}/{ModuleGenerationUtil.GetModuleMethodBodyFileNameWithExt(plan.Module, fileIndex)}", plan.Module);
            _methodBodyCodeFiles.Add(methodBodyCodeFile);
            return methodBodyCodeFile;
        }

        public void SaveAll()
        {
            foreach (var methodBodyCodeFile in _methodBodyCodeFiles)
            {
                methodBodyCodeFile.Save();
            }
        }
    }

    class MethodDefGenerationContext
    {
        public ModuleMethodBodyCodeFilePart methodBodyCodeFile;

        public AssemblyGenerationContext assemblyContext;
    }

    public class CppGenerator : GeneratorBase
    {

        private readonly Dictionary<string, AssemblyGenerationContext> _assemblyContexts = new Dictionary<string, AssemblyGenerationContext>();

        private AssemblyGenerationContext GetAssemblyContext(AssemblyPlan plan)
        {
            if (!_assemblyContexts.TryGetValue(plan.AssemblyName, out var context))
            {
                context = new AssemblyGenerationContext
                {
                    config = _config,
                    plan = plan,
                };
                _assemblyContexts[plan.AssemblyName] = context;
            }
            return context;
        }

        public override void GenerateAssembly(AssemblyPlan plan)
        {
            GenerateModuleRegistrationHeader(plan);
            GenerateModuleRegistrationCpp(plan);
            GenerateModuleMethodBodyRegistrationCpp(plan);
        }

        private void GenerateModuleRegistrationHeader(AssemblyPlan plan)
        {
            var context = GetAssemblyContext(plan);
            var registrationHeaderCodeFile = new ModuleRegistrationHeaderCodeFile($"{context.config.outputCodeDir}/{ModuleGenerationUtil.GetModuleRegistrationHeaderFileNameWithExt(plan.Module)}", plan.Module);
            registrationHeaderCodeFile.Generate();
            registrationHeaderCodeFile.Save();
        }

        private void GenerateModuleRegistrationCpp(AssemblyPlan plan)
        {
            var context = GetAssemblyContext(plan);
            var moduleRegistrationCppCodeFile = new ModuleRegistrationCppCodeFile($"{context.config.outputCodeDir}/{ModuleGenerationUtil.GetModuleRegistrationCppFileNameWithExt(plan.Module)}", plan);
            moduleRegistrationCppCodeFile.Generate();
            moduleRegistrationCppCodeFile.Save();
        }

        private void GenerateModuleMethodBodyRegistrationCpp(AssemblyPlan plan)
        {
            var context = GetAssemblyContext(plan);
            var methodBodyCodeFile = context.CreateMethodBodyCodeFileForNextFile();
            foreach (var methodPlan in plan.MethodPlans)
            {
                if (methodBodyCodeFile.ExceedsMaxSize())
                {
                    methodBodyCodeFile = context.CreateMethodBodyCodeFileForNextFile();
                }
                var methodCtx = new MethodDefGenerationContext
                {
                    methodBodyCodeFile = methodBodyCodeFile,
                    assemblyContext = context,
                };
                GenerateMethodDef(methodPlan, methodCtx);
            }
            context.SaveAll();
        }

        public override void GenerateMethodDef(MethodDefPlan plan, object ctx)
        {
            var invokerService = GlobalServices.Inst.InvokerService;
            invokerService.GetNotVirtualInvoker(plan.MethodDef);
            invokerService.GetVirtualInvoker(plan.MethodDef);
            var methodCtx = (MethodDefGenerationContext)ctx;
            var methodDetail = GlobalServices.Inst.MetadataService.GetMethodDetail(plan.MethodDef);
            var methodWriter = new MethodWriter(methodDetail, methodCtx.methodBodyCodeFile);
            methodWriter.WriteCode();
        }

        public override void GenerateGlobalInitializationCpp()
        {
            GenerateAllModuleRegistrationCpp();
            GenerateAllMethodInvokerCpp();
        }

        private void GenerateAllModuleRegistrationCpp()
        {
            var allModuleRegistrationCppCodeFile = new ModulesRegistrationCppCodeFile($"{_config.outputCodeDir}/{ModuleGenerationUtil.GetAllModuleRegistrationCppFileNameWithExt()}", _config.manifest.AssemblyPlans.Values.Select(plan => plan.Module).ToList());
            allModuleRegistrationCppCodeFile.Generate();
            allModuleRegistrationCppCodeFile.Save();
        }

        private void GenerateAllMethodInvokerCpp()
        {
            int partIndex = 0;
            var invokerFiles = new List<MethodInvokerCodeFilePart>();
            var methodInvokerCodeFile = new MethodInvokerCodeFilePart($"{_config.outputCodeDir}/{ModuleGenerationUtil.GetMethodInvokerCppFileNameWithExt(partIndex++)}");
            invokerFiles.Add(methodInvokerCodeFile);
            var invokerService = GlobalServices.Inst.InvokerService;
            var methodInvokerInfos = invokerService.GetNotVirtualInvokers();
            methodInvokerInfos.Sort((a, b) => a.name.CompareTo(b.name));

            var virtualMethodInvokerInfos = invokerService.GetVirtualInvokers();
            virtualMethodInvokerInfos.Sort((a, b) => a.name.CompareTo(b.name));

            var totalMethodInvokerInfos = new List<MethodInvokerInfo>(methodInvokerInfos);
            totalMethodInvokerInfos.AddRange(virtualMethodInvokerInfos);

            foreach (var invokerInfo in totalMethodInvokerInfos)
            {
                if (methodInvokerCodeFile.ExceedsMaxSize())
                {
                    methodInvokerCodeFile = new MethodInvokerCodeFilePart($"{_config.outputCodeDir}/{ModuleGenerationUtil.GetMethodInvokerCppFileNameWithExt(partIndex++)}");
                    invokerFiles.Add(methodInvokerCodeFile);
                }
                methodInvokerCodeFile.AddInvokerImpl(invokerInfo);
            }
            foreach (var invokerFile in invokerFiles)
            {
                invokerFile.Save();
            }
        }
    }
}
