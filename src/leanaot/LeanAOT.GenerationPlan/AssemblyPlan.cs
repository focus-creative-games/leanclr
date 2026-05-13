using dnlib.DotNet;

namespace LeanAOT.GenerationPlan
{
    public class AssemblyPlan
    {
        public ModuleDef Module { get; set; }

        public string AssemblyName { get; set; }

        public List<ClassPlan> ClassPlans { get; }

        public List<MethodDefPlan> MethodPlans { get; }

        /// <summary>
        /// Methods carrying <c>[MonoPInvokeCallback]</c> (reverse P/Invoke entry points), sorted by method token.
        /// Independent of whether the method was selected only via AOT rules; such methods are also added to <see cref="MethodPlans"/> for codegen.
        /// </summary>
        public IReadOnlyList<MethodDefPlan> MonoPInvokeCallbackMethodPlans { get; }

        private readonly HashSet<IMethod> _methodSet;

        public AssemblyPlan(ModuleDef module, string assemblyName, List<ClassPlan> classPlans, List<MethodDefPlan> methodPlans,
            List<MethodDefPlan> monoPInvokeCallbackMethodPlans)
        {
            Module = module;
            AssemblyName = assemblyName;
            ClassPlans = classPlans;
            MethodPlans = new List<MethodDefPlan>(methodPlans);
            MethodPlans.Sort((a, b) => a.MethodDef.MDToken.ToInt32().CompareTo(b.MethodDef.MDToken.ToInt32()));
            var monoSorted = new List<MethodDefPlan>(monoPInvokeCallbackMethodPlans);
            monoSorted.Sort((a, b) => a.MethodDef.MDToken.ToInt32().CompareTo(b.MethodDef.MDToken.ToInt32()));
            MonoPInvokeCallbackMethodPlans = monoSorted;
            _methodSet = new HashSet<IMethod>(methodPlans.Select(mp => mp.MethodDef), MethodEqualityComparer.CompareDeclaringTypes);
        }

        public bool ContainsMethod(IMethod method)
        {
            return _methodSet.Contains(method);
        }

        public bool ContaisAOTMethod(IMethod method)
        {
            return _methodSet.Contains(method);
        }
    }
}
