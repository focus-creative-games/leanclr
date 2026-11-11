using dnlib.DotNet.Emit;

namespace LeanAOT.ToCpp
{
    partial class MethodWriterBase
    {
        // Bound nominal cache storage to twelve pointer-sized words per frame:
        // four single-entry sites or two double-entry sites. A backward
        // branch interval is a conservative loop candidate, not a claim about
        // hotness. Never allocate caches for straight-line interface calls.
        private int _interfaceCacheCount;
        private List<(uint start, uint end)> _interfaceLoopRanges;

        private bool IsInterfaceLoopCandidate(Instruction inst)
        {
            if (_interfaceLoopRanges == null)
            {
                _interfaceLoopRanges = new List<(uint, uint)>();
                foreach (var branch in _method.MethodDef.Body.Instructions)
                {
                    if (branch.Operand is Instruction target && target.Offset <= branch.Offset)
                        _interfaceLoopRanges.Add((target.Offset, branch.Offset));
                    else if (branch.Operand is IList<Instruction> targets)
                        foreach (var destination in targets)
                            if (destination.Offset <= branch.Offset)
                                _interfaceLoopRanges.Add((destination.Offset, branch.Offset));
                }
            }
            return _interfaceLoopRanges.Any(range => inst.Offset >= range.start && inst.Offset <= range.end);
        }

        private string InterfaceResolution(Instruction inst, MethodDetail method, EvalVariable receiver, RuntimeResolvedVariable declaration)
        {
            string args = $"{GetEvalVariableName(receiver)}, {declaration.GetFullReferenceVariableName()}";
            bool twoEntries = _globalConfig.InterfaceCallCacheEntries == 2;
            int siteBudget = twoEntries ? 2 : 4;
            if (!_globalConfig.InterfaceCallCache || !method.MethodDef.DeclaringType.IsInterface ||
                _interfaceCacheCount >= siteBudget || !IsInterfaceLoopCandidate(inst))
                return $"{VmFunctionNames.GetVirtualMethodOnObj}({args})";
            string cache = $"__interface_cache_{_interfaceCacheCount++}";
            _localsWriter.AddLine($"leanclr::codegen::InterfaceCallCache{(twoEntries ? "2" : "")} {cache} = {{}};");
            return $"leanclr::codegen::get_interface_method_impl_cached({args}, {cache})";
        }
    }
}
