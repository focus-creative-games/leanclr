using dnlib.DotNet;

namespace LeanAOT.GenerationPlan
{
    public class GenericMethodPlan
    {
        public IMethod Method { get; set; }
        public string Key { get; set; }
        public int Depth { get; set; }
    }
}
