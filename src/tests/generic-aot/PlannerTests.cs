using dnlib.DotNet;
using LeanAOT.Core;
using LeanAOT.GenerationPlan;

static class PlannerTests
{
    static void Check(bool value, string message) { if (!value) throw new Exception(message); }
    public static void Main(string[] args)
    {
        var cache = new AssemblyCache(new MultiDirectoryAssemblyResolver(args.ToList()));
        Manifest Build(int count = 2048, int depth = 32) => new Manifest(new ManifestArgs
        {
            assemblyCache = cache, aotAssemblyNames = new List<string> { "GenericFixtures" },
            MaxGenericInstances = count, MaxGenericDepth = depth,
        });
        var full = Build();
        Check(full.GenericMethodPlans.Count > 20, "closed generic graph not discovered");
        Check(full.GenericMethodPlans.Select(p => p.Key).Distinct().Count() == full.GenericMethodPlans.Count, "duplicate instances");
        var noArgs = full.GenericMethodPlans.Where(p => p.Method.Name == "NoArguments").ToList();
        Check(noArgs.Count == 2 && noArgs[0].Key != noArgs[1].Key, "same signature, different generic arguments collided");
        Check(full.GenericMethodPlans.Count(p => p.Method.Name == "Recurse") == 1, "recursive instance not deduplicated");
        Check(full.GenericMethodPlans.Any(p => p.Method.Name == ".cctor" && p.Method.ResolveMethodDef().DeclaringType.Name == "FieldOnly`1"),
            "field-only generic static constructor was not discovered");
        Check(full.GenericMethodPlans.Any(p => p.Method.Name == "Read" && p.Method.ResolveMethodDef().DeclaringType.Name == "VirtualOnly`1"),
            "constructed generic class virtual implementation was not discovered");
        Check(full.GenericMethodPlans.Any(p => p.Method.ResolveMethodDef().DeclaringType.Name == "GenericHolder`1" &&
            MetaUtil.GetMethodGenericArgumentContext(p.Method).typeArgsStack[0].ElementType == ElementType.I4), "generic declaring type missing");
        Check(full.GenericMethodPlans.Any(p => p.Method.Name == "WithCatch"), "safe generic EH was not compiled");
        Check(full.GenericFallbacks.Any(f => f.Reason == "eh-snapshot-type"), "unsafe generic EH fallback not reported");
        Check(full.GenericFallbacks.Any(f => f.Reason == "attribute-excluded"), "attribute exclusion ignored");
        Check(!full.AssemblyPlans.ContainsKey("GenericLibrary") &&
            full.GenericMethodPlans.Any(p => p.Method.ResolveMethodDef().Module.Assembly.Name == "GenericLibrary"),
            "closed dependencies must not require ordinary AOT selection");
        var bounded = Build(4);
        Check(bounded.GenericMethodPlans.Count == 4 && bounded.GenericFallbacks.Any(f => f.Reason == "instance-budget"), "instance budget ineffective");
        var shallow = Build(2048, 2);
        Check(shallow.GenericFallbacks.Any(f => f.Reason == "depth-budget"), "expanding recursion was not bounded");
        Check(Build(0).GenericMethodPlans.Count == 0, "discovery cannot be disabled");
        var ehPlan = full.GenericMethodPlans.Single(p => p.Method.Name == "WithCatch" &&
            MetaUtil.GetMethodGenericArgumentContext(p.Method).methodArgsStack[0].ElementType == ElementType.I4);
        var ehDefinition = ehPlan.Method.ResolveMethodDef();
        var context = MetaUtil.GetMethodGenericArgumentContext(ehPlan.Method);
        Check(AotExceptionPolicy.FallbackReason(ehDefinition) == "eh-generic-context", "open EH definition accepted");
        Check(AotExceptionPolicy.FallbackReason(ehDefinition, context) == null, "closed scalar EH rejected");
        bool init = ehDefinition.Body.InitLocals;
        int locals = ehDefinition.Body.Variables.Count;
        try
        {
            if (locals == 0) ehDefinition.Body.Variables.Add(new dnlib.DotNet.Emit.Local(ehDefinition.Module.CorLibTypes.Int32));
            ehDefinition.Body.InitLocals = false;
            Check(AotExceptionPolicy.FallbackReason(ehDefinition, context) == "eh-uninitialized-locals", "uninitialized snapshot accepted");
            ehDefinition.Body.InitLocals = true;
            while (ehDefinition.Body.Variables.Count < 129) ehDefinition.Body.Variables.Add(new dnlib.DotNet.Emit.Local(ehDefinition.Module.CorLibTypes.Int32));
            Check(AotExceptionPolicy.FallbackReason(ehDefinition, context) == "eh-snapshot-budget", "snapshot budget exceeded");
        }
        finally
        {
            ehDefinition.Body.InitLocals = init;
            while (ehDefinition.Body.Variables.Count > locals) ehDefinition.Body.Variables.RemoveAt(ehDefinition.Body.Variables.Count - 1);
        }
        Console.WriteLine("Generic planner checks passed: " + full.GenericMethodPlans.Count + " instances");
    }
}
