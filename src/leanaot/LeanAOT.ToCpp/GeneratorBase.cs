using LeanAOT.GenerationPlan;

namespace LeanAOT.ToCpp
{

    public abstract class GeneratorBase : IGenerator
    {
        protected GenerationConfig _config;

        public void Generate(GenerationConfig conf)
        {
            _config = conf;
            GlobalServices.Inst.ReferenceSharing = new ReferenceGenericSharing(conf.manifest.GenericMethodPlans, GlobalServices.Inst.Config.ShareReferenceGenerics);
            foreach (var assPlan in conf.manifest.AssemblyPlans.Values)
            {
                GenerateAssembly(assPlan);
            }
            GenerateGlobalInitializationCpp();
        }

        public abstract void GenerateAssembly(AssemblyPlan plan);

        public abstract void GenerateMethodDef(MethodDefPlan plan, object ctx);

        public abstract void GenerateGlobalInitializationCpp();
    }
}
