using dnlib.DotNet;
using LeanAOT.Core;

namespace LeanAOT.ToCpp
{

    class MethodInvokerCodeFilePart
    {
        private readonly CodeWriter _totalWriter;

        private readonly CodeThunkZone _headThunkCollection;
        private readonly CodeThrunkWriter _includeWriter;

        private readonly ForwardDeclaration _forwardDeclaration;

        private readonly CodeThunkZone _invokerImplCollection;
        private readonly CodeThrunkWriter _invokerImplWriter;
        private readonly HashSet<MethodInvokerInfo> _addedInvokers = new HashSet<MethodInvokerInfo>();

        public MethodInvokerCodeFilePart(string filePath)
        {
            _totalWriter = new CodeWriter(filePath);

            _headThunkCollection = _totalWriter.CreateThunkCollection("head");
            _includeWriter = _headThunkCollection.CreateThunk("includes");
            _includeWriter.AddLine($"#include \"{ConstStrings.CodegenCommonHeaderFileName}\"");

            _forwardDeclaration = new ForwardDeclaration(_totalWriter.CreateThunkCollection("forward_declarations"));

            _invokerImplCollection = _totalWriter.CreateThunkCollection("method_invoker_impls");
            _invokerImplWriter = _invokerImplCollection.CreateThunk("method_invokers");

        }

        public bool ExceedsMaxSize()
        {
            const int maxSize = 1024 * 1024 * 5; // 5 MB
            return _totalWriter.CodeSize > maxSize;
        }

        public void AddInvokerImpl(MethodInvokerInfo info)
        {
            if (!_addedInvokers.Add(info))
            {
                return;
            }

            IMethod method = info.method;
            _forwardDeclaration.AddMethodForwardDeclaration(method);
            MethodDetail methodDetail = GlobalServices.Inst.MetadataService.GetMethodDetail(method);

            _invokerImplWriter.AddLine();
            //_invokerImplWriter.AddLine($"// Invoker for {method.FullName}");
            _invokerImplWriter.AddLine($"{ConstStrings.RtResultVoidTypeName} {info.name}({ConstStrings.ManagedMethodPointerTypeName} method_ptr, {ConstStrings.MethodInfoPtrTypeName} method, const {ConstStrings.StackObjectTypeName}* args, {ConstStrings.StackObjectTypeName}* ret){ConstStrings.CppFunctionNoexcept}");
            _invokerImplWriter.AddLine("{");
            _invokerImplWriter.IncreaseIndent();
            _invokerImplWriter.AddLine($"{methodDetail.CreateRelaxMethodFunctionTypedefStatement("FuncType")};");
            foreach (var param in methodDetail.ParamsIncludeThis)
            {
                int paramIndex = param.Index;
                _invokerImplWriter.AddLine($"constexpr size_t ARG{paramIndex}_OFFSET = {(paramIndex > 0 ? $"ARG{paramIndex - 1}_OFFSET + leanclr::codegen::get_stack_object_size_for_type<{MethodGenerationUtil.GetExactTypeName(param.Type)}>()" : "0")};");
            }
            string args = string.Join(", ", methodDetail.ParamsIncludeThis.Select((param, index) => index == 0 && info.isVirtual ? "args[0].obj + 1" : $"*({MethodGenerationUtil.GetCppTypeNameAsFieldOrArgOrLoc(param.Type, TypeNameRelaxLevel.AbiRelaxed)}*)(args + ARG{index}_OFFSET)"));
            if (methodDetail.IsVoidReturn)
            {
                _invokerImplWriter.AddLine($"return ((FuncType)method_ptr)({args});");
            }
            else
            {
                _invokerImplWriter.AddLine($"return leanclr::codegen::set_ret_or_return_error(((FuncType)method_ptr)({args}), ret);");
            }
            _invokerImplWriter.DecreaseIndent();
            _invokerImplWriter.AddLine("}");
        }

        public void Save()
        {
            _totalWriter.Save();
        }
    }
}
