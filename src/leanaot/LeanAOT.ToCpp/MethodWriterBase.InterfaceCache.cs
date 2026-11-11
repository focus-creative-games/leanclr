using dnlib.DotNet.Emit;
using dnlib.DotNet;
using LeanAOT.GenerationPlan;

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
        private Dictionary<uint, int> _profiledInterfaceSites;

        private int ProfiledCapacity(Instruction inst)
        {
            if (_profiledInterfaceSites == null)
            {
                _profiledInterfaceSites = new Dictionary<uint, int>();
                var hints = _globalConfig.PgoIncludeIndex?.GetInterfaceHints(_method.MethodDef) ?? Array.Empty<PgoMethodIncludeIndex.InterfaceHint>();
                int words = 0;
                foreach (var hint in hints.OrderByDescending(h => h.Calls).ThenBy(h => h.Offset))
                {
                    var instruction = _method.MethodDef.Body.Instructions.FirstOrDefault(i => i.Offset == hint.Offset);
                    if (hint.Entries == 0 || words + hint.Entries * 3 > 12 || instruction == null ||
                        instruction.OpCode.Code != Code.Callvirt || instruction.Operand is not IMethod target ||
                        target.ResolveMethodDef()?.DeclaringType.IsInterface != true || !IsInterfaceLoopCandidate(instruction)) continue;
                    _profiledInterfaceSites.Add(hint.Offset, hint.Entries);
                    words += hint.Entries * 3;
                }
            }
            return _profiledInterfaceSites.TryGetValue(inst.Offset, out int entries) ? entries : 0;
        }

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
            if (_globalConfig.EnableProfileInstrumentation && method.MethodDef.DeclaringType.IsInterface)
                _bodyWriter.AddLine($"LEANCLR_CODEGEN_PROFILE_INTERFACE_RECEIVER({GetEvalVariableName(receiver)}, {CurrentMethodReference}, {inst.Offset}u);");
            string args = $"{GetEvalVariableName(receiver)}, {declaration.GetFullReferenceVariableName()}";
            int capacity = _globalConfig.ProfiledInterfaceCaches ? ProfiledCapacity(inst) : _globalConfig.InterfaceCallCacheEntries;
            bool twoEntries = capacity == 2;
            int siteBudget = twoEntries ? 2 : 4;
            bool eligible = _globalConfig.InterfaceCallCache && capacity > 0 && method.MethodDef.DeclaringType.IsInterface &&
                (_globalConfig.ProfiledInterfaceCaches || _interfaceCacheCount < siteBudget) && IsInterfaceLoopCandidate(inst);
            if (_globalConfig.ProfiledInterfaceCaches && method.MethodDef.DeclaringType.IsInterface)
                _globalConfig.InterfaceCacheDecisions.Add(new { method = _method.FullName, il_offset = inst.Offset, entries = eligible ? capacity : 0 });
            if (!eligible)
                return $"{VmFunctionNames.GetVirtualMethodOnObj}({args})";
            string cache = $"__interface_cache_{_interfaceCacheCount++}";
            _localsWriter.AddLine($"leanclr::codegen::InterfaceCallCache{(twoEntries ? "2" : "")} {cache} = {{}};");
            return $"leanclr::codegen::get_interface_method_impl_cached({args}, {cache})";
        }
    }
}
