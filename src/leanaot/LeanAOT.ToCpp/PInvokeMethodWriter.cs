using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using dnlib.DotNet;
using LeanAOT.Core;

namespace LeanAOT.ToCpp
{
    class PInvokeMethodWriter : SpecialMethodWriterBase
    {
        private const string PInvokeFnPtrVar = "__leanclr_pinvoke_fn";
        private readonly CharSet _defaultCharSet;

        private readonly List<MarshalParamOrRet> _params;
        private readonly MarshalParamOrRet _ret;
        private readonly MethodDef _methodDef;

        public PInvokeMethodWriter(MethodDetail method, IMethodBodyCodeFilePart methodBodyCodeFile) : base(method, methodBodyCodeFile, null)
        {
            _defaultCharSet = MarshalUtil.GetMethodDefaultCharSet(_method.MethodDef);
            _params = _method.ParamsIncludeThis.Select(p => CreateMarshalParamOrRet(p, false)).ToList();
            _ret = CreateMarshalParamOrRet(_method.RetTypeParam, true);
            _methodDef = _method.MethodDef;
        }

        private MarshalParamOrRet CreateMarshalParamOrRet(ParamDetail param, bool isReturn)
        {
            return new MarshalParamOrRet(param.Type, MarshalUtil.GetParameterOrFieldMarshalInfo(param.ParamDef), _defaultCharSet, isReturn, param.Name, $"{param.Name}_native");
        }

        protected override void WriteMethodBody()
        {
            if (_params.All(p => p.Supported) && _ret.Supported)
            {
                WritePInvokeBody();
            }
            else
            {
                _bodyWriter.AddLine($"printf(\"PInvoke method {_method.FullName} is not supported\");");
                _bodyWriter.AddLine($"LEANCLR_CODEGEN_RETURN_NOT_IMPLEMENTED_ERROR();");
            }
        }

        private void WritePInvokeBody()
        {
            string importName = MarshalUtil.GetPInvokeImportName(_methodDef);
            string callConvMacro = MarshalUtil.GetPInvokeCallConvCppMacro(_methodDef);
            string dllNameNoExt = MarshalUtil.GetPInvokeModuleName(_methodDef);
            string escapedDllName = StringUtil.EscapeCppString(dllNameNoExt);
            string standardedDllName = StringUtil.StandardizedDllName(dllNameNoExt);
            string fnTypedefName = $"__leanclr_pinvoke_fn_{_method.UniqueName}";
            string nativeParamDeclsWithoutVarName = string.Join(", ", _params.Select(p => p.NativeTypeName));
            _bodyWriter.AddLine($"typedef {_ret.NativeTypeName} ({callConvMacro} *{fnTypedefName})({nativeParamDeclsWithoutVarName});");
            if (MarshalUtil.IsInternalDll(dllNameNoExt))
            {
                _forwardDeclaration.AddPInvokeNativeExternDeclaration(
                    dllNameNoExt,
                    standardedDllName,
                    $"extern \"C\" {_ret.NativeTypeName} {callConvMacro} {importName}({nativeParamDeclsWithoutVarName});");
                _bodyWriter.AddLine($"{fnTypedefName} {PInvokeFnPtrVar} = {importName};");
            }
            else
            {
                _bodyWriter.AddLine($"#if FORCE_PINVOKE_INTERNAL || FORCE_PINVOKE_{standardedDllName}_INTERNAL");
                _forwardDeclaration.AddPInvokeNativeExternDeclaration(
                    dllNameNoExt,
                    standardedDllName,
                    $"extern \"C\" {_ret.NativeTypeName} {callConvMacro} {importName}({nativeParamDeclsWithoutVarName});");
                _bodyWriter.AddLine($"{fnTypedefName} {PInvokeFnPtrVar} = {importName};");
                _bodyWriter.AddLine("#else");
                _bodyWriter.AddLine($"static {fnTypedefName} __leanclr_pinvoke_fn_cache = nullptr;");
                _bodyWriter.AddLine("if (__leanclr_pinvoke_fn_cache == nullptr)");
                _bodyWriter.BeginBlock();
                _bodyWriter.AddLine($"__leanclr_pinvoke_fn_cache = reinterpret_cast<{fnTypedefName}>({ConstStrings.CodegenNamespace}::resolve_pinvoke_function(\"{escapedDllName}\", \"{importName}\"));");
                _bodyWriter.EndBlock();
                _bodyWriter.AddLine($"{fnTypedefName} {PInvokeFnPtrVar} = __leanclr_pinvoke_fn_cache;");
                _bodyWriter.AddLine($"if ({PInvokeFnPtrVar} == nullptr)");
                _bodyWriter.BeginBlock();
                _bodyWriter.AddLine($"{VmFunctionNames.RET_ERROR}({ConstStrings.CodegenNamespace}::raise_pinvoke_entry_not_found_error(\"{escapedDllName}\", \"{importName}\"));");
                _bodyWriter.EndBlock();
                _bodyWriter.AddLine("#endif");
            }

            foreach (var param in _params)
            {
                _bodyWriter.AddLines(param.ManagedToNativeSetupCode);
            }

            string nativeParamExprs = string.Join(", ", _params.Select(p => p.NativeVarName));
            if (_method.IsVoidReturn)
            {
                _bodyWriter.AddLine($"{PInvokeFnPtrVar}({nativeParamExprs});");
                foreach (var param in _params)
                {
                    _bodyWriter.AddLines(param.ManagedToNativeCleanupCode);
                }
                _bodyWriter.AddLine($"{VmFunctionNames.RET_VOID}();");
            }
            else
            {
                _bodyWriter.AddLine($"{_ret.NativeTypeName} {_ret.NativeVarName} = {PInvokeFnPtrVar}({nativeParamExprs});");
                foreach (var param in _params)
                {
                    _bodyWriter.AddLines(param.ManagedToNativeCleanupCode);
                }
                _bodyWriter.AddLines(_ret.NativeToManagedSetupCode);
                _bodyWriter.AddLines(_ret.NativeToManagedCleanupCode);
                _bodyWriter.AddLine($"{VmFunctionNames.RET_VALUE}({_ret.ManagedVarName});");
            }
        }
    }
}
