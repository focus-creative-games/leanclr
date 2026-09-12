namespace LeanAOT.ToCpp
{
    class MethodDirectCallBridgeCodeFilePart : IMethodBodyCodeFilePart
    {
        public ForwardDeclaration ForwardDeclaration => throw new NotImplementedException();
        public CodeWriter MethodWriter => throw new NotImplementedException();


        private readonly CodeWriter _totalWriter;

        private readonly CodeThunkZone _headThunkCollection;
        private readonly CodeThrunkWriter _includeWriter;

        private readonly ForwardDeclaration _forwardDeclaration;

        private readonly CodeThunkZone _directCallBridgeImplCollection;
        private readonly CodeThrunkWriter _directCallBridgeImplWriter;
        private readonly HashSet<DirectCallBridgeInfo> _addedDirectCallBridgeInfos = new HashSet<DirectCallBridgeInfo>();

        public MethodDirectCallBridgeCodeFilePart(string filePath)
        {
            _totalWriter = new CodeWriter(filePath);

            _headThunkCollection = _totalWriter.CreateThunkCollection("head");
            _includeWriter = _headThunkCollection.CreateThunk("includes");
            _includeWriter.AddLine($"#include \"{ConstStrings.CodegenCommonHeaderFileName}\"");

            _forwardDeclaration = new ForwardDeclaration(_totalWriter.CreateThunkCollection("forward_declarations"));

            _directCallBridgeImplCollection = _totalWriter.CreateThunkCollection("method_direct_call_bridge_impls");
            _directCallBridgeImplWriter = _directCallBridgeImplCollection.CreateThunk("method_direct_call_bridges");

        }

        public bool ExceedsMaxSize()
        {
            const int maxSize = 1024 * 1024 * 5; // 5 MB
            return _totalWriter.CodeSize > maxSize;
        }

        public void AddDirectCallBridgeImpl(DirectCallBridgeInfo directCallBridgeInfo)
        {
            if (!_addedDirectCallBridgeInfos.Add(directCallBridgeInfo))
            {
                return;
            }

            MethodDetail methodDetail = directCallBridgeInfo.method;
            _forwardDeclaration.AddMethodForwardDeclaration(methodDetail.Method);

            _directCallBridgeImplWriter.AddLine();
            _directCallBridgeImplWriter.AddLine($"{directCallBridgeInfo.GenerateMethodDeclaring()}");
            _directCallBridgeImplWriter.BeginBlock();

            var parametersIncludeThis = methodDetail.ParamsIncludeThis;
            string methodVarName = "__method";
            string callvirVarName = "__callvir";
            var castFnPtrType = methodDetail.CreateRelaxMethodFunctionPointerTypeForCast();

            _directCallBridgeImplWriter.AddLine($"if (__vararg_count == 0 && {VmFunctionNames.IsAotMethod}({methodVarName}))");
            _directCallBridgeImplWriter.BeginBlock();
            _directCallBridgeImplWriter.AddLine($"if (!{callvirVarName} && leanclr::vm::Method::is_static({methodVarName}) && std::strcmp({methodVarName}->name, \"{VmFunctionNames.CCtor}\") != 0 && {VmFunctionNames.IsCctorNotFinishied}({methodVarName}->parent))");
            _directCallBridgeImplWriter.BeginBlock();
            _directCallBridgeImplWriter.AddLine($"auto __cctor_result = {VmFunctionNames.RunClassStaticConstructor}({methodVarName}->parent);");
            _directCallBridgeImplWriter.AddLine("if (__cctor_result.is_err())");
            _directCallBridgeImplWriter.BeginBlock();
            _directCallBridgeImplWriter.AddLine($"{VmFunctionNames.RET_ERROR}(__cctor_result.unwrap_err());");
            _directCallBridgeImplWriter.EndBlock();
            _directCallBridgeImplWriter.EndBlock();
            _directCallBridgeImplWriter.AddLine($"{ConstStrings.ManagedMethodPointerTypeName} __method_pointer = {callvirVarName} ? {methodVarName}->{ConstStrings.VirtualMethodPointerFieldName} : {methodVarName}->{ConstStrings.MethodPointerFieldName};");
            _directCallBridgeImplWriter.AddLine($"return (({castFnPtrType})__method_pointer)({MethodGenerationUtil.CreateMethodFunctionArgsWithoutCast(methodDetail)});");
            _directCallBridgeImplWriter.EndBlock();
            _directCallBridgeImplWriter.AddLine($"else");
            _directCallBridgeImplWriter.BeginBlock();

            int paramCount = parametersIncludeThis.Length;
            bool hasReturnValue = methodDetail.HasNotVoidReturn;

            string argsStr;
            if (paramCount == 0)
            {
                argsStr = "const_cast<leanclr::interp::RtStackObject*>(__varargs)";
            }
            else
            {
                _directCallBridgeImplWriter.AddLine("constexpr size_t ARG0_OFFSET = 0;");
                for (int paramIndex = 0, last = paramCount - 1; paramIndex < last; paramIndex++)
                {
                    ParamDetail param = methodDetail.ParamsIncludeThis[paramIndex];
                    _directCallBridgeImplWriter.AddLine($"constexpr size_t ARG{paramIndex + 1}_OFFSET = ARG{paramIndex}_OFFSET + {VmFunctionNames.GetStackObjectSizeForType}<{MethodGenerationUtil.GetAbiRelaxedTypeName(param.Type)}>();");
                }
                _directCallBridgeImplWriter.AddLine($"constexpr size_t ARGS_SIZE = ARG{paramCount - 1}_OFFSET + {VmFunctionNames.GetStackObjectSizeForType}<{MethodGenerationUtil.GetAbiRelaxedTypeName(methodDetail.ParamsIncludeThis.Last().Type)}>();");
                argsStr = "__argsBuf";
                _directCallBridgeImplWriter.AddLine($"{ConstStrings.StackObjectTypeName} {argsStr}[ARGS_SIZE];");
                if (!methodDetail.IsStatic)
                {
                    _directCallBridgeImplWriter.AddLine($"void* __this_for_interp = {parametersIncludeThis[0].Name};");
                    _directCallBridgeImplWriter.AddLine($"if ({callvirVarName} && leanclr::vm::Method::is_instance({methodVarName}) && leanclr::vm::Class::is_value_type({methodVarName}->parent))");
                    _directCallBridgeImplWriter.BeginBlock();
                    _directCallBridgeImplWriter.AddLine($"__this_for_interp = const_cast<void*>(leanclr::vm::Object::get_box_value_type_data_ptr((leanclr::vm::RtObject*){parametersIncludeThis[0].Name}));");
                    _directCallBridgeImplWriter.EndBlock();
                }
                foreach (var param in parametersIncludeThis)
                {
                    int paramIndex = param.Index;
                    string argName = !methodDetail.IsStatic && paramIndex == 0 ? "__this_for_interp" : param.Name;
                    _directCallBridgeImplWriter.AddLine($"{VmFunctionNames.ExpandArgumentToEvalStack}({argName}, {argsStr} + ARG{paramIndex}_OFFSET);");
                }
            }
            string retStr;
            if (hasReturnValue)
            {
                _directCallBridgeImplWriter.AddLine($"constexpr size_t RET_SIZE = {VmFunctionNames.GetStackObjectSizeForType}<{MethodGenerationUtil.GetAbiRelaxedTypeName(methodDetail.RetType)}>();");
                retStr = "__retBuf";
                _directCallBridgeImplWriter.AddLine($"{ConstStrings.StackObjectTypeName} {retStr}[RET_SIZE];");
            }
            else
            {
                retStr = "nullptr";
            }
            _directCallBridgeImplWriter.AddLine($"auto __invoker = {callvirVarName} ? {VmFunctionNames.VirtualInvokeWithoutRunClassStaticConstructorWithVarArgs} : {VmFunctionNames.InvokeWithRunClassStaticConstructorWithVarArgs};");
            if (hasReturnValue)
            {
                string retTypeName = MethodGenerationUtil.GetAbiRelaxedTypeName(methodDetail.RetType);
                _directCallBridgeImplWriter.AddLine($"auto __result = __invoker({methodVarName}, {argsStr}, {retStr}, __vararg_count);");
                _directCallBridgeImplWriter.AddLine($"if (__result.is_ok())");
                _directCallBridgeImplWriter.BeginBlock();
                _directCallBridgeImplWriter.AddLine($"{VmFunctionNames.RET_VALUE}(*({retTypeName}*){retStr});");
                _directCallBridgeImplWriter.EndBlock();
                _directCallBridgeImplWriter.AddLine($"else");
                _directCallBridgeImplWriter.BeginBlock();
                _directCallBridgeImplWriter.AddLine($"{VmFunctionNames.RET_ERROR}(__result.unwrap_err());");
                _directCallBridgeImplWriter.EndBlock();
            }
            else
            {
                _directCallBridgeImplWriter.AddLine($"return __invoker({methodVarName}, {argsStr}, {retStr}, __vararg_count);");
            }
            // end of if (is_aot_method)
            _directCallBridgeImplWriter.EndBlock();
            // end of method
            _directCallBridgeImplWriter.EndBlock();
        }

        public void Save()
        {
            _totalWriter.Save();
        }
    }
}
