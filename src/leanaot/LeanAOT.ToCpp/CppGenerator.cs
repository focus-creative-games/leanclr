using dnlib.DotNet;
using LeanAOT.Core;
using LeanAOT.GenerationPlan;

namespace LeanAOT.ToCpp
{
    class MethodGenerationException : Exception
    {
        public MethodDef MethodDef { get; }
        public MethodGenerationException(MethodDef methodDef, string message, Exception innerException) : base(message, innerException)
        {
            MethodDef = methodDef;
        }
    }

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
        private static readonly NLog.Logger s_logger = NLog.LogManager.GetCurrentClassLogger();

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
            try
            {
                GenerateMethodDefSelf(plan.MethodDef, ctx);
                GenerateVirtualMethodDefSelf(plan.MethodDef, ctx);
            }
            catch (Exception e)
            {
                throw new MethodGenerationException(plan.MethodDef, $"Error generating method {plan.MethodDef.FullName}", e);
            }
        }

        private void GenerateMethodDefSelf(MethodDef methodDef, object ctx)
        {
            var invokerService = GlobalServices.Inst.InvokerService;
            invokerService.GetInvoker(methodDef);
            var methodCtx = (MethodDefGenerationContext)ctx;
            var methodDetail = GlobalServices.Inst.MetadataService.GetMethodDetail(methodDef);
            var runtimeApiCatalog = GlobalServices.Inst.RuntimeApiCatalog;
            if (runtimeApiCatalog.TryGetIcallOrIntrinsic(methodDef, out var entry, out var _methodKind))
            {
                var icallOrIntrinsicMethodWriter = new ICallOrIntrinsicMethodWriter(methodDetail, methodCtx.methodBodyCodeFile, entry);
                icallOrIntrinsicMethodWriter.WriteCode();
                return;
            }
            if (!methodDef.HasBody)
            {
                if (methodDef.IsPinvokeImpl)
                {
                    var pinvokeMethodWriter = new PInvokeMethodWriter(methodDetail, methodCtx.methodBodyCodeFile);
                    pinvokeMethodWriter.WriteCode();
                    return;
                }
                else if (methodDef.IsInternalCall)
                {
                    var runtimeResolvedICallMethodWriter = new RuntimeResolvedICallMethodWriter(methodDetail, methodCtx.methodBodyCodeFile);
                    runtimeResolvedICallMethodWriter.WriteCode();
                    return;
                }
                // s_logger.Warn($"Method {methodDef.FullName} has no body and doesn't have icall or intrinsic entry");
                var notImplementedMethodWriter = new NotImplementedMethodWriter(methodDetail, methodCtx.methodBodyCodeFile);
                notImplementedMethodWriter.WriteCode();
                return;
            }

            var methodWriter = new MethodWriter(methodDetail, methodCtx.methodBodyCodeFile);
            methodWriter.WriteCode();
        }


        private void GenerateVirtualMethodDefSelf(MethodDef methodDef, object ctx)
        {
            var methodCtx = (MethodDefGenerationContext)ctx;
            var methodDetail = GlobalServices.Inst.MetadataService.GetMethodDetail(methodDef);
            if (!methodDetail.ShouldGenerateVirtualMethod)
            {
                return;
            }
            var virtualMethodWriter = new VirtualMethodWriter(methodDetail, methodCtx.methodBodyCodeFile);
            virtualMethodWriter.WriteCode();
        }

        public override void GenerateGlobalInitializationCpp()
        {
            new CalliBridgesCppCodeFile().Generate(_config.outputCodeDir, _config.manifest);
            GenerateGlobalGenericMethods();
            GenerateAllModuleRegistrationCpp();
            GenerateAllMethodInvokerCpp();
            GenerateAllDirectCallBridgeCpp();
        }

        private void GenerateGlobalGenericMethods()
        {
            ModuleMethodBodyCodeFilePart body = null;
            int part = 0;
            var emittedBodies = new HashSet<string>(StringComparer.Ordinal);
            foreach (var instance in _config.manifest.GenericMethodPlans.OrderBy(p => p.Key, StringComparer.Ordinal))
            {
                if (body == null || body.ExceedsMaxSize())
                {
                    body?.Save();
                    body = new ModuleMethodBodyCodeFilePart($"{_config.outputCodeDir}/generic_methods_part{part++}.cpp", null);
                }
                GlobalServices.Inst.InvokerService.GetInvoker(instance.Method);
                var method = GlobalServices.Inst.ReferenceSharing.Canonicalize(instance.Method);
                if (!emittedBodies.Add(Manifest.GenericInstanceKey(method))) continue;
                var detail = GlobalServices.Inst.MetadataService.GetMethodDetail(method);
                new MethodWriter(detail, body).WriteCode();
                if (detail.ShouldGenerateVirtualMethod) new VirtualMethodWriter(detail, body).WriteCode();
            }
            body?.Save();
            var registration = new GenericRegistrationCppCodeFile($"{_config.outputCodeDir}/generic_registration.cpp", _config.manifest.GenericMethodPlans);
            registration.Generate();
            registration.Save();
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

        private void GenerateAllDirectCallBridgeCpp()
        {
            int partIndex = 0;
            var directCallBridgeFiles = new List<MethodDirectCallBridgeCodeFilePart>();
            var directCallBridgeCodeFile = new MethodDirectCallBridgeCodeFilePart($"{_config.outputCodeDir}/{ModuleGenerationUtil.GetMethodDirectCallBridgeCppFileNameWithExt(partIndex++)}");
            directCallBridgeFiles.Add(directCallBridgeCodeFile);
            var directCallBridgeService = GlobalServices.Inst.DirectCallBridgeService;
            var directCallBridgeInfos = directCallBridgeService.GetDirectCallBridgeInfos();
            directCallBridgeInfos.Sort((a, b) => a.name.CompareTo(b.name));
            foreach (var directCallBridgeInfo in directCallBridgeInfos)
            {
                if (directCallBridgeCodeFile.ExceedsMaxSize())
                {
                    directCallBridgeCodeFile = new MethodDirectCallBridgeCodeFilePart($"{_config.outputCodeDir}/{ModuleGenerationUtil.GetMethodDirectCallBridgeCppFileNameWithExt(partIndex++)}");
                    directCallBridgeFiles.Add(directCallBridgeCodeFile);
                }
                directCallBridgeCodeFile.AddDirectCallBridgeImpl(directCallBridgeInfo);
            }
            foreach (var directCallBridgeFile in directCallBridgeFiles)
            {
                directCallBridgeFile.Save();
            }
        }
    }
}
