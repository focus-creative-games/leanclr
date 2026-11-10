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
        Check(full.GenericMethodPlans.Any(p => p.Method.ResolveMethodDef().DeclaringType.Name == "GenericHolder`1" &&
            MetaUtil.GetMethodGenericArgumentContext(p.Method).typeArgsStack[0].ElementType == ElementType.I4), "generic declaring type missing");
        Check(full.GenericFallbacks.Any(f => f.Reason == "exception-handlers"), "EH fallback not reported");
        Check(full.GenericFallbacks.Any(f => f.Reason == "attribute-excluded"), "attribute exclusion ignored");
        Check(full.GenericFallbacks.Any(f => f.Reason == "assembly-not-selected"), "unselected assembly must not be compiled implicitly");
        var bounded = Build(4);
        Check(bounded.GenericMethodPlans.Count == 4 && bounded.GenericFallbacks.Any(f => f.Reason == "instance-budget"), "instance budget ineffective");
        var shallow = Build(2048, 2);
        Check(shallow.GenericFallbacks.Any(f => f.Reason == "depth-budget"), "expanding recursion was not bounded");
        Check(Build(0).GenericMethodPlans.Count == 0, "discovery cannot be disabled");
        Console.WriteLine("Generic planner checks passed: " + full.GenericMethodPlans.Count + " instances");
    }
}
