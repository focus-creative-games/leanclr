using dnlib.DotNet;
using dnlib.DotNet.Emit;

namespace LeanAOT.ToCpp
{
    partial class MethodWriterBase
    {
        private bool IsNet10Profile => GlobalServices.Inst.RuntimeApiCatalog?.ProfileName == "coreclr-net10";
        private readonly HashSet<string> _rootCapacityExpressions = new(StringComparer.Ordinal);
        private bool _rootSnapshotSeen;

        private static bool NeedsRootType(TypeSig type)
        {
            return type.RemovePinnedAndModifiers().ElementType switch
            {
                ElementType.Class or ElementType.Object or ElementType.String or ElementType.Array or ElementType.SZArray or
                ElementType.ByRef or ElementType.Ptr or ElementType.I or ElementType.U or ElementType.TypedByRef or
                ElementType.ValueType or ElementType.GenericInst => true,
                _ => false,
            };
        }

        private void WriteRootSnapshot(Instruction instruction)
        {
            if (!IsNet10Profile) return;
            switch (instruction.OpCode.Code)
            {
                case Code.Call: case Code.Callvirt: case Code.Newobj:
                case Code.Ldsfld: case Code.Ldsflda: case Code.Stsfld:
                    break;
                default: return;
            }
            _rootSnapshotSeen = true;
            var roots = new List<(string Name, string Type)>();
            roots.AddRange(_curState.runStackDatas.Where(v => v.type is EvalDataType.Ref or EvalDataType.I or EvalDataType.ValueType)
                .Select(v => (GetEvalVariableName(v), GetTypeName(v))));
            roots = roots.DistinctBy(r => r.Name).ToList();
            if (roots.Count != 0)
                _rootCapacityExpressions.Add(string.Join(" + ", roots.Select(r => $"((sizeof({r.Type}) + sizeof(uintptr_t) - 1) / sizeof(uintptr_t))")));
            _bodyWriter.AddLine($"___aot_roots.assign({{{string.Join(", ", roots.Select(r => $"leanclr::codegen::AotRootRange::from({r.Name})"))}}});");
        }

        private void WriteRootScope()
        {
            if (!_rootSnapshotSeen) return;
            var stable = _parameterVariables.Where(p => NeedsRootType(p.Type)).Select(p => (Name: GetParameterName(p), p.Type))
                .Concat(_localVariables.Where(l => NeedsRootType(l.Type)).Select(l => (Name: GetLocalName(l), l.Type))).ToList();
            var ranges = new List<string>();
            foreach (var root in stable)
            {
                ranges.Add($"leanclr::codegen::AotRootRange::from({root.Name})");
                var type = root.Type.RemovePinnedAndModifiers();
                if (type is ByRefSig byref && NeedsRootType(byref.Next))
                {
                    _forwardDeclaration.AddTypeForwardDefine(byref.Next);
                    ranges.Add($"leanclr::codegen::AotRootRange::target<{GetExactTypeName(byref.Next)}>(&{root.Name})");
                }
            }
            if (_rootCapacityExpressions.Count == 0 && ranges.Count == 0)
            {
                _localsWriter.AddLine("leanclr::codegen::AotRootScopeNoop ___aot_roots;");
                return;
            }
            string capacity = _rootCapacityExpressions.Count == 0 ? "1" : "std::max({size_t(1), " + string.Join(", ", _rootCapacityExpressions.OrderBy(s => s, StringComparer.Ordinal)) + "})";
            _localsWriter.AddLine($"uintptr_t ___aot_root_buffer[{capacity}];");
            if (ranges.Count != 0)
                _localsWriter.AddLine($"const leanclr::codegen::AotRootRange ___aot_stable_roots[] = {{{string.Join(", ", ranges)}}};");
            _localsWriter.AddLine($"leanclr::codegen::AotRootScope ___aot_roots(___aot_root_buffer, sizeof(___aot_root_buffer) / sizeof(uintptr_t), {(ranges.Count == 0 ? "nullptr" : "___aot_stable_roots")}, {ranges.Count});");
        }
    }
}
