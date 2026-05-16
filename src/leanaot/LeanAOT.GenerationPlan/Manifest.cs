using System.Diagnostics;
using System.Linq;
using dnlib.DotNet;
using LeanAOT.Core;

namespace LeanAOT.GenerationPlan
{

    public class ManifestArgs
    {
        public AssemblyCache assemblyCache;

        public List<string> aotAssemblyNames;

        /// <summary>
        /// When non-null, method inclusion after attribute/intrinsic checks follows <c>aot.xml</c> rules (see design doc).
        /// </summary>
        public AotMethodRulesEvaluator AotRulesEvaluator;
    }

    public class Manifest
    {

        private static readonly NLog.Logger s_logger = NLog.LogManager.GetCurrentClassLogger();

        private readonly Dictionary<string, AssemblyPlan> _assemblyPlans = new Dictionary<string, AssemblyPlan>();

        public IReadOnlyDictionary<string, AssemblyPlan> AssemblyPlans => _assemblyPlans;

        private readonly List<GenericMethodPlan> _genericMethodPlans = new List<GenericMethodPlan>();

        public IReadOnlyList<GenericMethodPlan> GenericMethodPlans => _genericMethodPlans;

        private void TryAddMonoPInvokeCallbackPlan(List<MethodDefPlan> monoPInvokeCallbackPlans, MethodDef method)
        {
            var ca = method.CustomAttributes.FirstOrDefault(MetaUtil.IsMonoPInvokeCallbackAttribute);
            if (ca == null)
            {
                return;
            }
            if (ca.ConstructorArguments.Count != 1 || ca.ConstructorArguments[0].Type.FullName != "System.Type")
            {
                s_logger.Warn($"Skip method with invalid MonoPInvokeCallbackAttribute: {method.FullName}. Expected exactly one argument of type System.Type.");
                return;
            }
            if (!method.IsStatic)
            {
                s_logger.Warn($"Skip method with non-static MonoPInvokeCallbackAttribute: {method.FullName} token: {method.MDToken}");
                return;
            }
            if (!method.HasBody)
            {
                s_logger.Warn($"Skip method with no body: {method.FullName} token: {method.MDToken}");
                return;
            }
            monoPInvokeCallbackPlans.Add(new MethodDefPlan { MethodDef = method });
        }

        public Manifest(ManifestArgs args)
        {
            foreach (var assName in args.aotAssemblyNames)
            {

                var mod = args.assemblyCache.LoadModule(assName);
                var classPlans = new List<ClassPlan>();
                var methodPlans = new List<MethodDefPlan>();
                var monoPInvokeCallbackPlans = new List<MethodDefPlan>();
                var methodsInAotPlan = new HashSet<IMethod>(MethodEqualityComparer.CompareDeclaringTypes);
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
                        TryAddMonoPInvokeCallbackPlan(monoPInvokeCallbackPlans, method);
                        string typeName = method.DeclaringType.Name;
                        CustomAttribute ca = method.CustomAttributes.FirstOrDefault(ca => ca.TypeFullName == "AotMethodAttribute");
                        if (ca != null)
                        {
                            bool isAotMethod = (bool)ca.ConstructorArguments[0].Value;
                            if (!isAotMethod)
                            {
                                continue;
                            }
                            TryAddMethodPlan(methodPlans, methodsInAotPlan, method);
                            continue;
                        }
                        if (method.IsPinvokeImpl || method.IsInternalCall)
                        {
                            // aot or intrinsic methods must be aot
                            TryAddMethodPlan(methodPlans, methodsInAotPlan, method);
                            continue;
                        }

                        // 8.3–8.4: rule files; no match => AOT (design)
                        if (args.AotRulesEvaluator != null && !args.AotRulesEvaluator.ShouldIncludeByRules(assName, method))
                        {
                            s_logger.Debug($"[Manifest] Skip method (AOT rules): {method.FullName} token: {method.MDToken}");
                            continue;
                        }

                        TryAddMethodPlan(methodPlans, methodsInAotPlan, method);
                    }
                }
                var assPlan = new AssemblyPlan(mod, assName, classPlans, methodPlans, monoPInvokeCallbackPlans);

                _assemblyPlans[assName] = assPlan;
            }
        }

        private static void TryAddMethodPlan(List<MethodDefPlan> methodPlans, HashSet<IMethod> methodsInAotPlan, MethodDef method)
        {
            if (!methodsInAotPlan.Add(method))
            {
                return;
            }
            var methodPlan = new MethodDefPlan()
            {
                MethodDef = method,
            };
            s_logger.Debug($"[Manifest] Add MethodDefPlan: {method.FullName}");
            methodPlans.Add(methodPlan);
        }

        public bool ShouldAOT(IMethod method)
        {
            MethodDef methodDef = method.ResolveMethodDef();
            if (methodDef != null)
            {
                return AssemblyPlans.TryGetValue(methodDef.Module.Assembly.Name, out var assPlan) &&
                    assPlan.ContaisAOTMethod(method);
            }
            return false;
        }
    }

}
