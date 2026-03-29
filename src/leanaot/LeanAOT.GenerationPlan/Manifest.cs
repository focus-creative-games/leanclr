using dnlib.DotNet;
using LeanAOT.Core;

namespace LeanAOT.GenerationPlan
{

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
                        if (!method.HasBody || method.IsAbstract || method.HasGenericParameters || method.DeclaringType.HasGenericParameters)
                            continue;
                        if (method.Body.HasExceptionHandlers)
                            continue;
                        if (method.CallingConvention == CallingConvention.VarArg)
                        {
                            s_logger.Warn($"Skip method with vararg calling convention: {method.FullName} token: {method.MDToken}");
                            continue;
                        }
                        string typeName = method.DeclaringType.Name;
                        if (method.CustomAttributes.Any(ca => ca.TypeFullName == "AotMethodAttribute" && ca.ConstructorArguments[0].Value.Equals(false)))
                        {
                            continue;
                        }
                        if (r.NextSingle() > 0.1f)
                        {
                            continue;
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
            if (method is MethodDef methodDef)
            {
                return _assemblyPlans.TryGetValue(methodDef.Module.Assembly.FullName, out var assPlan) &&
                    assPlan.MethodPlans.Any(mp => mp.MethodDef == methodDef);
            }
            return false;
        }
    }

    public class ManifestArgs
    {
        public AssemblyCache assemblyCache;

        public List<string> aotAssemblyNames;
    }

}
