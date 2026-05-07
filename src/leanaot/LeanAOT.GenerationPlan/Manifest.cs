using dnlib.DotNet;
using LeanAOT.Core;

namespace LeanAOT.GenerationPlan
{

    public class ManifestArgs
    {
        public AssemblyCache assemblyCache;

        public List<string> aotAssemblyNames;

        public int? aotSamplingPercent;
    }

    public class Manifest
    {

        private static readonly NLog.Logger s_logger = NLog.LogManager.GetCurrentClassLogger();

        private readonly Dictionary<string, AssemblyPlan> _assemblyPlans = new Dictionary<string, AssemblyPlan>();

        public IReadOnlyDictionary<string, AssemblyPlan> AssemblyPlans => _assemblyPlans;

        private readonly List<GenericMethodPlan> _genericMethodPlans = new List<GenericMethodPlan>();

        public IReadOnlyList<GenericMethodPlan> GenericMethodPlans => _genericMethodPlans;


        public Manifest(ManifestArgs args)
        {
            var r = new Random(0);
            int aotSamplingPercent = args.aotSamplingPercent ?? 100;
            foreach (var assName in args.aotAssemblyNames)
            {

                var mod = args.assemblyCache.LoadModule(assName);
                var classPlans = new List<ClassPlan>();
                var methodPlans = new List<MethodDefPlan>();
                foreach (TypeDef type in mod.GetTypes())
                {
                    var classPlan = new ClassPlan()
                    {
                        TypeDef = type,
                    };
                    classPlans.Add(classPlan);
                    foreach (var method in type.Methods)
                    {
                        if (method.IsRuntime || method.IsAbstract || method.HasGenericParameters || method.DeclaringType.HasGenericParameters)
                            continue;
                        if (method.HasBody && method.Body.HasExceptionHandlers)
                            continue;
                        // don't aot System.String's constructor, because we will handle it in a special way
                        if (!method.HasBody && method.IsConstructor && type.FullName == "System.String")
                        {
                            continue;
                        }
                        if (method.CallingConvention == CallingConvention.VarArg)
                        {
                            s_logger.Warn($"Skip method with vararg calling convention: {method.FullName} token: {method.MDToken}");
                            continue;
                        }
                        string typeName = method.DeclaringType.Name;
                        CustomAttribute ca = method.CustomAttributes.FirstOrDefault(ca => ca.TypeFullName == "AotMethodAttribute" );
                        if (ca != null)
                        {
                            bool isAotMethod = (bool)ca.ConstructorArguments[0].Value;
                            if (!isAotMethod)
                            {
                                continue;
                            }
                        }
                        else if (method.IsPinvokeImpl || method.IsInternalCall)
                        {
                            // aot or intrinsic methods must be aot
                        }
                        else
                        {
                            if (r.Next(0, 100) >= aotSamplingPercent)
                            {
                                s_logger.Debug($"[Manifest] Skip method: {method.FullName} token: {method.MDToken} because of aot sampling percent: {aotSamplingPercent}");
                                continue;
                            }
                        }
                        var methodPlan = new MethodDefPlan()
                        {
                            MethodDef = method,
                        };
                        s_logger.Debug($"[Manifest] Add MethodDefPlan: {method.FullName}");
                        methodPlans.Add(methodPlan);
                    }
                }
                var assPlan = new AssemblyPlan(mod, assName, classPlans, methodPlans);

                _assemblyPlans[assName] = assPlan;
            }
        }

        public bool ShouldAOT(IMethod method)
        {
            MethodDef methodDef = method.ResolveMethodDef();
            if (methodDef != null)
            {
                return AssemblyPlans.TryGetValue(methodDef.Module.Assembly.Name, out var assPlan) &&
                    assPlan.ContainsMethod(method);
            }
            return false;
        }
    }

}
