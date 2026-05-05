using System;
using dnlib.DotNet;
using LeanAOT.Core;
using System.Text;

namespace LeanAOT.ToCpp
{
    class PInvokeMethodWriter : SpecialMethodWriterBase
    {
        private class NativeParam
        {
            public string TypeName;
            public string Expr;
            public List<string> SetupLines = new List<string>();
        }

        public PInvokeMethodWriter(MethodDetail method, IMethodBodyCodeFilePart methodBodyCodeFile) : base(method, methodBodyCodeFile, null)
        {

        }

        protected override void WriteMethodBody()
        {
            string importName = GetImportName();
            try
            {
                WriteWasmPInvokeBody(importName);
            }
            catch (NotSupportedException ex)
            {
                _bodyWriter.AddLine($"printf(\"PInvoke method {_method.FullName} is not supported: {EscapeCppString(ex.Message)}\\n\");");
                _bodyWriter.AddLine($"LEANCLR_CODEGEN_RETURN_NOT_IMPLEMENTED_ERROR();");
            }
        }

        private void WriteWasmPInvokeBody(string importName)
        {
            string nativeRetType = GetNativeTypeName(_method.RetType, true);
            string pinvokeSymbol = $"__leanclr_pinvoke_{_method.UniqueName}";
            var nativeParams = _method.ParamsIncludeThis.Select(CreateNativeParam).ToList();
            string nativeParamDecls = string.Join(", ", nativeParams.Select((p, index) => $"{p.TypeName} __arg{index}"));
            string nativeParamExprs = string.Join(", ", nativeParams.Select(p => p.Expr));

            bool kernel32Guard = IsWinOnlyDll(GetPInvokeModuleName());
            if (kernel32Guard)
            {
                _bodyWriter.AddLine("#if !LEANCLR_PLATFORM_WIN");
                _bodyWriter.AddLine($"printf(\"PInvoke method {_method.FullName} is not supported on non-Windows platforms for symbol {EscapeCppString(importName)}\\n\");");
                _bodyWriter.AddLine($"LEANCLR_CODEGEN_RETURN_NOT_IMPLEMENTED_ERROR();");
                _bodyWriter.AddLine("#else");
            }

            _bodyWriter.AddLine("#if LEANCLR_PLATFORM_WASM");
            _bodyWriter.AddLine($"extern {nativeRetType} {pinvokeSymbol}({nativeParamDecls}) __asm__(\"{EscapeCppString(importName)}\");");
            foreach (var param in nativeParams)
            {
                foreach (string line in param.SetupLines)
                {
                    _bodyWriter.AddLine(line);
                }
            }

            if (_method.IsVoidReturn)
            {
                _bodyWriter.AddLine($"{pinvokeSymbol}({nativeParamExprs});");
                _bodyWriter.AddLine($"{ConstStrings.CodegenReturnVoid}();");
            }
            else if (IsStringType(_method.RetType))
            {
                _bodyWriter.AddLine($"const char* __pinvoke_utf8_ret = {pinvokeSymbol}({nativeParamExprs});");
                _bodyWriter.AddLine($"auto __pinvoke_managed_str = {ConstStrings.CodegenNamespace}::marshal_utf8_string_to_utf16(__pinvoke_utf8_ret);");
                _bodyWriter.AddLine($"{ConstStrings.CodegenNamespace}::free_pinvoke_returned_utf8_cstr(__pinvoke_utf8_ret);");
                _bodyWriter.AddLine($"{ConstStrings.CodegenReturn}(__pinvoke_managed_str);");
            }
            else
            {
                string managedRetType = MethodGenerationUtil.GetCppTypeNameAsFieldOrArgOrLoc(_method.RetType, TypeNameRelaxLevel.AbiRelaxed);
                _bodyWriter.AddLine($"auto __pinvoke_ret = {pinvokeSymbol}({nativeParamExprs});");
                _bodyWriter.AddLine($"{ConstStrings.CodegenReturn}(({managedRetType})__pinvoke_ret);");
            }

            _bodyWriter.AddLine("#else");
            _bodyWriter.AddLine($"printf(\"PInvoke method {_method.FullName} requires wasm static linking for symbol {EscapeCppString(importName)}\\n\");");
            _bodyWriter.AddLine($"LEANCLR_CODEGEN_RETURN_NOT_IMPLEMENTED_ERROR();");
            _bodyWriter.AddLine("#endif");

            if (kernel32Guard)
            {
                _bodyWriter.AddLine("#endif");
            }
        }

        private NativeParam CreateNativeParam(ParamDetail param)
        {
            TypeSig type = param.Type.RemovePinnedAndModifiers();
            if (IsStringType(type))
            {
                string converterName = $"__temp_utf8_converter_{param.Name}";
                return new NativeParam
                {
                        TypeName = "const char*",
                        Expr = $"{converterName}.get_utf8_str()",
                    SetupLines = new List<string>
                    {
                        $"{ConstStrings.CodegenNamespace}::TempUtf16StringToUtf8Converter {converterName}({param.Name});",
                    },
                };
            }
            if (type.ElementType == ElementType.SZArray)
            {
                var elemType = ((SZArraySig)type).Next.RemovePinnedAndModifiers();
                string elemNativeType = GetNativeTypeName(elemType, false);
                return new NativeParam
                {
                    TypeName = $"{elemNativeType}*",
                    Expr = $"{param.Name} ? {ConstStrings.CodegenNamespace}::get_array_element_data_start_as<{elemNativeType}>((leanclr::vm::RtArray*){param.Name}) : nullptr",
                };
            }
            return new NativeParam
            {
                TypeName = GetNativeTypeName(type, false),
                Expr = $"({GetNativeTypeName(type, false)}){param.Name}",
            };
        }

        private string GetNativeTypeName(TypeSig type, bool isReturnType)
        {
            type = type.RemovePinnedAndModifiers();
            switch (type.ElementType)
            {
            case ElementType.Void: return "void";
            case ElementType.Boolean: return "bool";
            case ElementType.Char: return "leanclr::Utf16Char";
            case ElementType.I1: return "int8_t";
            case ElementType.U1: return "uint8_t";
            case ElementType.I2: return "int16_t";
            case ElementType.U2: return "uint16_t";
            case ElementType.I4: return "int32_t";
            case ElementType.U4: return "uint32_t";
            case ElementType.I8: return "int64_t";
            case ElementType.U8: return "uint64_t";
            case ElementType.R4: return ConstStrings.Float32TypeName;
            case ElementType.R8:
            case ElementType.R: return ConstStrings.Float64TypeName;
            case ElementType.I: return "intptr_t";
            case ElementType.U: return "uintptr_t";
            case ElementType.Ptr:
                return $"{GetNativeTypeName(((PtrSig)type).Next, false)}*";
            case ElementType.ByRef:
                return $"{GetNativeTypeName(((ByRefSig)type).Next, false)}*";
            case ElementType.String:
                return isReturnType ? "const char*" : "const char*";
            case ElementType.ValueType:
            case ElementType.GenericInst:
            {
                TypeDef typeDef = type.ToTypeDefOrRef().ResolveTypeDefThrow();
                if (typeDef.IsEnum)
                {
                    return GetNativeTypeName(typeDef.GetEnumUnderlyingType(), isReturnType);
                }
                if (GlobalServices.Inst.TypeNameService.IsPtrLikeSystemValueType(typeDef))
                {
                    return typeDef.FullName == "System.UIntPtr" ? "uintptr_t" : "intptr_t";
                }
                return MethodGenerationUtil.GetCppTypeNameAsFieldOrArgOrLoc(type, TypeNameRelaxLevel.Exactly);
            }
            case ElementType.Class:
            {
                TypeDef typeDef = type.ToTypeDefOrRef().ResolveTypeDefThrow();
                // if type is sub class of SafeHandle, return void*
                if (MetaUtil.IsInheritFrom(typeDef, "System.Runtime.InteropServices.SafeHandle"))
                {
                    return "void*";
                }
                throw new NotSupportedException($"PInvoke native ABI does not support parameter or return type {type.FullName}.");
            }
            default:
                throw new NotSupportedException($"PInvoke native ABI does not support parameter or return type {type.FullName}.");
            }
        }

        private string GetImportName()
        {
            ImplMap implMap = _method.MethodDef.ImplMap;
            if (implMap == null)
            {
                return _method.MethodDef.Name;
            }
            string name = implMap.Name?.String;
            return string.IsNullOrEmpty(name) ? _method.MethodDef.Name : name;
        }

        private string GetPInvokeModuleName()
        {
            ImplMap implMap = _method.MethodDef.ImplMap;
            return implMap?.Module?.Name?.String;
        }

        private readonly static string[] s_winOnlyDlls = { "kernel32", "user32", "gdi32", "ole32", "shell32", "advapi32", "msvcrt", "ntdll" };

        private static bool IsWinOnlyDll(string moduleName)
        {
            if (string.IsNullOrEmpty(moduleName))
            {
                return false;
            }
            string n = moduleName.Trim();
            if (n.EndsWith(".dll", StringComparison.OrdinalIgnoreCase))
            {
                n = n.Substring(0, n.Length - 4);
            }
            if (s_winOnlyDlls.Contains(n, StringComparer.OrdinalIgnoreCase))
            {
                return true;
            }
            return false;
        }

        private static bool IsStringType(TypeSig type)
        {
            return type.RemovePinnedAndModifiers().ElementType == ElementType.String;
        }

        private static string EscapeCppString(string value)
        {
            var sb = new StringBuilder();
            foreach (char ch in value)
            {
                switch (ch)
                {
                case '\\': sb.Append("\\\\"); break;
                case '"': sb.Append("\\\""); break;
                case '\n': sb.Append("\\n"); break;
                case '\r': sb.Append("\\r"); break;
                case '\t': sb.Append("\\t"); break;
                default: sb.Append(ch); break;
                }
            }
            return sb.ToString();
        }
    }
}
