using dnlib.DotNet;
using LeanAOT.GenerationPlan;

namespace LeanAOT.ToCpp
{
    public class ManifestService
    {
        public Manifest Manifest { get; }

        public ManifestService(Manifest manifest)
        {
            Manifest = manifest;
        }

        public bool ShouldAOT(IMethod method)
        {
            return Manifest.ShouldAOT(method);
        }
    }
}
