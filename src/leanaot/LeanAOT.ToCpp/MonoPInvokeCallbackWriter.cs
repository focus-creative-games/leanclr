using System.Diagnostics;
using System.Runtime.InteropServices;
using dnlib.DotNet;

namespace LeanAOT.ToCpp
{
    class MonoPInvokeCallbackWriter
    {
        private readonly MonoPInvokeCallbackInfo _info;
        private readonly ForwardDeclaration _forwardDeclaration;
        private readonly CodeThrunkWriter _writer;

        private readonly List<MarshalParamOrRet> _params;
        private readonly MarshalParamOrRet _ret;

        public MonoPInvokeCallbackWriter(MonoPInvokeCallbackInfo info, ForwardDeclaration forwardDeclaration, CodeThrunkWriter writer)
        {
            _info = info;
            _forwardDeclaration = forwardDeclaration;
            _writer = writer;
            _params = info.methodDetail.ParamsIncludeThis.Select(p => CreateMarshalParamOrRet(p, false)).ToList();
            _ret = CreateMarshalParamOrRet(info.methodDetail.RetTypeParam, true);
        }

        private MarshalParamOrRet CreateMarshalParamOrRet(ParamDetail param, bool isReturn)
        {
            return new MarshalParamOrRet(param.Type, MarshalUtil.GetParameterOrFieldMarshalInfo(param.ParamDef), CharSet.None, isReturn, param.Name, $"{param.Name}_native");
        }

        public void WriteCode()
        {
            MethodDef method = _info.method;
            Debug.Assert(method.IsStatic);
            _forwardDeclaration.AddMethodForwardDeclaration(method);
            MethodDetail methodDetail = _info.methodDetail;
            CodeThrunkWriter writer = _writer;
            string methodName = _info.name;

            string callConv = MarshalUtil.GetMonoPInvokeCallbackCallingConventionCppMacro(method);
            string paramDecl = string.Join(", ", _params.Select(p => $"{p.NativeTypeName} {p.NativeVarName}"));

            writer.AddLine();
            writer.AddLine($"{_ret.NativeTypeName} {callConv} {methodName}({paramDecl}){ConstStrings.CppFunctionNoexcept}");
            writer.BeginBlock();

            if (!(_params.All(p => p.Supported) && _ret.Supported))
            {
                writer.AddLine($"printf(\"MonoPInvokeCallback method {method.FullName} is not supported\");");
                writer.AddLine($"{VmFunctionNames.ABORT_WHEN_RAISE_EXCEPTION_IN_MONO_PINVOKE_CALLBACK}();");
                if (methodDetail.HasNotVoidReturn)
                {
                    writer.AddLine($"return {{}};");
                }
                writer.EndBlock();
                return;
            }

            string cachedMethodVarName = $"s_mono_pinvoke_resolved_method_{method.MDToken.ToInt32():X8}";
            writer.AddLine($"static {ConstStrings.MethodInfoPtrTypeName} {cachedMethodVarName} = nullptr;");
            writer.AddLine($"if (LEANCLR_UNLIKELY({cachedMethodVarName} == nullptr))");
            writer.BeginBlock();
            writer.AddLine($"{cachedMethodVarName} = reinterpret_cast<{ConstStrings.MethodInfoPtrTypeName}>({ConstStrings.CodegenNamespace}::resolve_metadata_token({ModuleGenerationUtil.GetModuleGlobalVariableName(method.Module)}, 0x{method.MDToken.ToInt32():X8}, nullptr));");
            writer.EndBlock();
            writer.AddLine($"{ConstStrings.MethodInfoPtrTypeName} __method = {cachedMethodVarName};");

            foreach (var param in _params)
            {
                writer.AddLines(param.NativeToManagedSetupCode);
            }

            int paramCount = methodDetail.ParamCountIncludeThis;
            string argsPtr;
            if (paramCount > 0)
            {
                argsPtr = "__args_buffer";
                writer.AddLine("constexpr size_t ARG0_OFFSET = 0;");
                for (int paramIndex = 0, last = paramCount - 1; paramIndex < last; paramIndex++)
                {
                    MarshalParamOrRet param = _params[paramIndex];
                    writer.AddLine($"constexpr size_t ARG{paramIndex + 1}_OFFSET = ARG{paramIndex}_OFFSET + {VmFunctionNames.GetStackObjectSizeForType}<{param.ManagedTypeName}>();");
                }
                writer.AddLine($"constexpr size_t ARGS_SIZE = ARG{paramCount - 1}_OFFSET + {VmFunctionNames.GetStackObjectSizeForType}<{_params.Last().ManagedTypeName}>();");
                writer.AddLine($"{ConstStrings.StackObjectTypeName} {argsPtr}[ARGS_SIZE];");
                for (int i = 0; i < paramCount; i++)
                {
                    MarshalParamOrRet param = _params[i];
                    writer.AddLine($"{VmFunctionNames.ExpandArgumentToEvalStack}({param.ManagedVarName}, {argsPtr} + ARG{i}_OFFSET);");
                    writer.AddLines(param.NativeToManagedCleanupCode);
                }
            }
            else
            {
                argsPtr = "nullptr";
            }
            string retPtr;
            if (methodDetail.HasNotVoidReturn)
            {
                writer.AddLine($"constexpr size_t RET_SIZE = {VmFunctionNames.GetStackObjectSizeForType}<{_ret.ManagedTypeName}>();");
                retPtr = "__ret_buffer";
                writer.AddLine($"{ConstStrings.StackObjectTypeName} {retPtr}[RET_SIZE];");
            }
            else
            {
                retPtr = "nullptr";
            }
            string invokeFn = VmFunctionNames.InvokeWithRunClassStaticConstructor;
            writer.AddLine($"auto __inv_result = {invokeFn}(__method, {argsPtr}, {retPtr});");
            foreach (var param in _params)
            {
                writer.AddLines(param.NativeToManagedCleanupCode);
            }
            writer.AddLine($"if (LEANCLR_UNLIKELY(__inv_result.is_err())) {{ {VmFunctionNames.ABORT_WHEN_RAISE_EXCEPTION_IN_MONO_PINVOKE_CALLBACK}(); }}");

            if (methodDetail.HasNotVoidReturn)
            {
                string retExact = MethodGenerationUtil.GetExactTypeName(methodDetail.RetType);
                writer.AddLine($"{_ret.ManagedTypeName} {_ret.ManagedVarName} = {ConstStrings.CodegenNamespace}::get_eval_stack_value_as_type<{_ret.ManagedTypeName}>({retPtr});");
                writer.AddLines(_ret.ManagedToNativeSetupCode);
                writer.AddLines(_ret.ManagedToNativeCleanupCode);
                writer.AddLine($"return {_ret.NativeVarName};");
            }
            writer.EndBlock();
        }
    }
}
