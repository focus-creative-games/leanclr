using dnlib.DotNet;
using dnlib.DotNet.Emit;

namespace LeanAOT.ToCpp
{
    partial class MethodWriterBase
    {
        private bool ExceptionHandoff => _method.MethodDef.Body.HasExceptionHandlers;
        private readonly Dictionary<uint, List<(int id, string next)>> _finallyReturns = new Dictionary<uint, List<(int, string)>>();
        private int _nextLeaveId;

        private bool InterpreterOnlyBlock(uint offset)
        {
            return ExceptionHandoff && _method.MethodDef.Body.ExceptionHandlers.Any(e =>
                e.HandlerType == ExceptionHandlerType.Catch && offset >= e.HandlerStart.Offset &&
                (e.HandlerEnd == null || offset < e.HandlerEnd.Offset));
        }

        private void WriteFinallyDispatchers()
        {
            foreach (var pair in _finallyReturns)
            {
                _tailWriter.AddLine($"__finally_end_{pair.Key}:");
                _tailWriter.AddLine("switch (___eh_leave)");
                _tailWriter.BeginBlock();
                foreach (var continuation in pair.Value)
                    _tailWriter.AddLine($"case {continuation.id}: goto {continuation.next};");
                _tailWriter.AddLine("default: return leanclr::RtErr::InvalidOperation;");
                _tailWriter.EndBlock();
            }
        }

        private void WriteExceptionSnapshot()
        {
            _tailWriter.BeginBlock();
            var values = _parameterVariables.Select(p => (type: p.Type, name: GetParameterName(p)))
                .Concat(_localVariables.Select(l => (type: l.Type, name: GetLocalName(l)))).ToList();
            _tailWriter.AddLine("constexpr size_t __eh_offset_0 = 0;");
            for (int i = 0; i < values.Count; ++i)
                _tailWriter.AddLine($"constexpr size_t __eh_offset_{i + 1} = __eh_offset_{i} + leanclr::codegen::get_stack_object_size_for_type<{GetExactTypeName(values[i].type)}>();");
            _tailWriter.AddLine($"leanclr::interp::RtStackObject __eh_snapshot[__eh_offset_{values.Count} ? __eh_offset_{values.Count} : 1] = {{}};");
            for (int i = 0; i < values.Count; ++i)
                _tailWriter.AddLine($"leanclr::codegen::expand_argument_to_eval_stack({values[i].name}, __eh_snapshot + __eh_offset_{i});");
            _tailWriter.AddLine($"auto __eh_result = leanclr::interp::Interpreter::resume_exception({CurrentMethodReference}, __eh_snapshot, __eh_offset_{values.Count}, ___eh_ip, ___ret_err, ___eh_explicit_throw);");
            _tailWriter.AddLine("if (__eh_result.is_err()) return __eh_result.unwrap_err();");
            if (_method.IsVoidReturn) _tailWriter.AddLine("return leanclr::core::Unit{};");
            else _tailWriter.AddLine($"return leanclr::codegen::get_eval_stack_value_as_type<{MethodGenerationUtil.GetAbiRelaxedTypeName(_method.RetType)}>(__eh_result.unwrap());");
            _tailWriter.EndBlock();
        }

        private void EmitNormalLeave(Instruction inst, Instruction target)
        {
            _curState.runStackDatas.Clear();
            var targetBlock = _instToBbMap[target];
            SetupInOutVars(_curBb, targetBlock);
            var finalizers = _method.MethodDef.Body.ExceptionHandlers.Where(e =>
                e.HandlerType == ExceptionHandlerType.Finally && inst.Offset >= e.TryStart.Offset &&
                (e.TryEnd == null || inst.Offset < e.TryEnd.Offset) &&
                !(target.Offset >= e.TryStart.Offset && (e.TryEnd == null || target.Offset < e.TryEnd.Offset)))
                .OrderByDescending(e => e.TryStart.Offset).ThenBy(e => e.TryEnd?.Offset ?? uint.MaxValue).ToList();
            if (finalizers.Count == 0) { _bodyWriter.AddLine($"goto {GetBasicBlockStartLabelName(targetBlock)};"); return; }
            int id = ++_nextLeaveId;
            for (int i = 0; i < finalizers.Count; ++i)
            {
                uint offset = finalizers[i].HandlerStart.Offset;
                if (!_finallyReturns.TryGetValue(offset, out var destinations)) _finallyReturns[offset] = destinations = new List<(int, string)>();
                destinations.Add((id, i + 1 == finalizers.Count ? GetBasicBlockStartLabelName(targetBlock) :
                    GetBasicBlockStartLabelName(_instToBbMap[finalizers[i + 1].HandlerStart])));
            }
            _bodyWriter.AddLine($"___eh_leave = {id};");
            _bodyWriter.AddLine($"goto {GetBasicBlockStartLabelName(_instToBbMap[finalizers[0].HandlerStart])};");
        }
    }
}
