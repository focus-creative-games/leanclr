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
                WritePInvokeBody(importName);
            }
            catch (NotSupportedException ex)
            {
                _bodyWriter.AddLine($"printf(\"PInvoke method {_method.FullName} is not supported: {EscapeCppString(ex.Message)}\\n\");");
                _bodyWriter.AddLine($"LEANCLR_CODEGEN_RETURN_NOT_IMPLEMENTED_ERROR();");
            }
        }

        private void WritePInvokeBody(string importName)
        {
            string nativeRetType = GetNativeTypeName(_method.RetType, true);
            string callConvMacro = GetPInvokeCallConvCppMacro();
            var nativeParams = _method.ParamsIncludeThis.Select(CreateNativeParam).ToList();
            string nativeParamDecls = string.Join(", ", nativeParams.Select((p, index) => $"{p.TypeName} __arg{index}"));
            string nativeParamExprs = string.Join(", ", nativeParams.Select(p => p.Expr));
            string dllNameNoExt = GetPInvokeDllNameNoExt();
            string escapedDllName = EscapeCppString(dllNameNoExt);
            string standardedDllName = StandardizedDllName(dllNameNoExt);
            string escapedImportLiteral = importName;
            string fnTypedefName = $"__leanclr_pinvoke_fn_{_method.UniqueName}";
            bool isInternalDll = dllNameNoExt == ConstStrings.InternalDllName;

            _bodyWriter.AddLine($"typedef {nativeRetType} ({callConvMacro} *{fnTypedefName})({nativeParamDecls});");
            if (isInternalDll)
            {
                _forwardDeclaration.AddPInvokeNativeExternDeclaration(
                    dllNameNoExt,
                    standardedDllName,
                    $"extern \"C\" {nativeRetType} {callConvMacro} {importName}({nativeParamDecls});");
                _bodyWriter.AddLine($"{fnTypedefName} {PInvokeFnPtrVar} = {importName};");
            }
            else
            {
                _bodyWriter.AddLine($"#if FORCE_PINVOKE_INTERNAL || FORCE_PINVOKE_{standardedDllName}_INTERNAL");
                _forwardDeclaration.AddPInvokeNativeExternDeclaration(
                    dllNameNoExt,
                    standardedDllName,
                    $"extern \"C\" {nativeRetType} {callConvMacro} {importName}({nativeParamDecls});");
                _bodyWriter.AddLine($"{fnTypedefName} {PInvokeFnPtrVar} = {importName};");
                _bodyWriter.AddLine("#else");
                _bodyWriter.AddLine($"static {fnTypedefName} __leanclr_pinvoke_fn_cache = nullptr;");
                _bodyWriter.AddLine("if (__leanclr_pinvoke_fn_cache == nullptr)");
                _bodyWriter.BeginBlock();
                _bodyWriter.AddLine($"__leanclr_pinvoke_fn_cache = reinterpret_cast<{fnTypedefName}>({ConstStrings.CodegenNamespace}::resolve_pinvoke_function(\"{escapedDllName}\", \"{escapedImportLiteral}\"));");
                _bodyWriter.EndBlock();
                _bodyWriter.AddLine($"{fnTypedefName} {PInvokeFnPtrVar} = __leanclr_pinvoke_fn_cache;");
                _bodyWriter.AddLine("#endif");
            }
            _bodyWriter.AddLine($"if ({PInvokeFnPtrVar} == nullptr)");
            _bodyWriter.BeginBlock();
            _bodyWriter.AddLine($"{VmFunctionNames.RET_ERROR}({ConstStrings.CodegenNamespace}::raise_pinvoke_entry_not_found_error(\"{escapedDllName}\", \"{escapedImportLiteral}\"));");
            _bodyWriter.EndBlock();

            foreach (var param in nativeParams)
            {
                foreach (string line in param.SetupLines)
                {
                    _bodyWriter.AddLine(line);
                }
            }

            WritePInvokeInvokeAndReturn(PInvokeFnPtrVar, nativeParamExprs);
        }

        private void WritePInvokeInvokeAndReturn(string calleeExpr, string nativeParamExprs)
        {
            if (_method.IsVoidReturn)
            {
                _bodyWriter.AddLine($"{calleeExpr}({nativeParamExprs});");
                _bodyWriter.AddLine($"{VmFunctionNames.RET_VOID}();");
            }
            else if (IsStringType(_method.RetType))
            {
                _bodyWriter.AddLine($"const char* __pinvoke_utf8_ret = {calleeExpr}({nativeParamExprs});");
                _bodyWriter.AddLine($"auto __pinvoke_managed_str = {ConstStrings.CodegenNamespace}::marshal_utf8_string_to_utf16(__pinvoke_utf8_ret);");
                _bodyWriter.AddLine($"{ConstStrings.CodegenNamespace}::free_pinvoke_returned_utf8_cstr(__pinvoke_utf8_ret);");
                _bodyWriter.AddLine($"{VmFunctionNames.RET_VALUE}(__pinvoke_managed_str);");
            }
            else
            {
                string managedRetType = MethodGenerationUtil.GetCppTypeNameAsFieldOrArgOrLoc(_method.RetType, TypeNameRelaxLevel.AbiRelaxed);
                _bodyWriter.AddLine($"auto __pinvoke_ret = {calleeExpr}({nativeParamExprs});");
                _bodyWriter.AddLine($"{VmFunctionNames.RET_VALUE}(({managedRetType})__pinvoke_ret);");
            }
        }

        private string GetPInvokeCallConvCppMacro()
        {
            ImplMap implMap = _method.MethodDef.ImplMap;
            PInvokeAttributes cc = (implMap?.Attributes ?? 0) & PInvokeAttributes.CallConvMask;
            switch (cc)
            {
            case 0:
            case PInvokeAttributes.CallConvWinapi:
                return "LEANCLR_PINVOKE_CALL_WINAPI";
            case PInvokeAttributes.CallConvCdecl:
                return "LEANCLR_PINVOKE_CALL_CDECL";
            case PInvokeAttributes.CallConvStdCall:
                return "LEANCLR_PINVOKE_CALL_STDCALL";
            case PInvokeAttributes.CallConvThiscall:
                return "LEANCLR_PINVOKE_CALL_THISCALL";
            case PInvokeAttributes.CallConvFastcall:
                return "LEANCLR_PINVOKE_CALL_FASTCALL";
            default:
                throw new NotSupportedException($"PInvoke calling convention mask 0x{(ushort)cc:X4} is not supported for {_method.FullName}.");
            }
        }

        private NativeParam CreateNativeParam(ParamDetail param)
        {
            TypeSig type = param.Type.RemovePinnedAndModifiers();
            if (type.ElementType == ElementType.Class)
            {
                TypeDef typeDef = type.ToTypeDefOrRef().ResolveTypeDefThrow();
                if (MetaUtil.IsInheritFrom(typeDef, "System.Delegate"))
                {
                    return new NativeParam
                    {
                        TypeName = "void*",
                        Expr = $"{ConstStrings.CodegenNamespace}::pinvoke_marshal_delegate_to_void_ptr(reinterpret_cast<leanclr::vm::RtDelegate*>({param.Name}))",
                    };
                }
                if (MetaUtil.IsInheritFrom(typeDef, "System.Runtime.InteropServices.SafeHandle")
                    || MetaUtil.IsInheritFrom(typeDef, "System.Runtime.InteropServices.CriticalHandle"))
                {
                    return new NativeParam
                    {
                        TypeName = "void*",
                        Expr = $"{ConstStrings.CodegenNamespace}::pinvoke_marshal_safe_handle_to_void_ptr(reinterpret_cast<leanclr::vm::RtObject*>({param.Name}))",
                    };
                }
            }
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
            case ElementType.FnPtr:
                return "void*";
            case ElementType.String:
                return isReturnType ? "const char*" : "const char*";
            case ElementType.ValueType:
            case ElementType.GenericInst:
            {
                TypeDef typeDef = ResolveTypeDefForStructTypeSig(type);
                if (typeDef.IsEnum)
                {
                    return GetNativeTypeName(typeDef.GetEnumUnderlyingType(), isReturnType);
                }
                if (GlobalServices.Inst.TypeNameService.IsPtrLikeSystemValueType(typeDef))
                {
                    return typeDef.FullName == "System.UIntPtr" ? "uintptr_t" : "intptr_t";
                }
                EnsureBlittableStructForPInvoke(type, isReturnType);
                return MethodGenerationUtil.GetCppTypeNameAsFieldOrArgOrLoc(type, TypeNameRelaxLevel.Exactly);
            }
            case ElementType.Class:
            {
                TypeDef typeDef = type.ToTypeDefOrRef().ResolveTypeDefThrow();
                if (MetaUtil.IsInheritFrom(typeDef, "System.Runtime.InteropServices.SafeHandle")
                    || MetaUtil.IsInheritFrom(typeDef, "System.Runtime.InteropServices.CriticalHandle"))
                {
                    if (isReturnType)
                    {
                        throw new NotSupportedException(
                            $"PInvoke does not marshal return type `{type.FullName}` from an unmanaged handle; use IntPtr or wrap the value in managed code.");
                    }
                    return "void*";
                }
                if (MetaUtil.IsInheritFrom(typeDef, "System.Delegate"))
                {
                    if (isReturnType)
                    {
                        throw new NotSupportedException(
                            $"PInvoke does not marshal return type `{type.FullName}` from a native function pointer; use IntPtr, void*, or Marshal.GetDelegateForFunctionPointer in managed code.");
                    }
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

        private string GetPInvokeDllNameNoExt()
        {
            string module = GetPInvokeModuleName();
            if (string.IsNullOrEmpty(module))
            {
                return string.Empty;
            }
            string n = module.Trim();
            if (n.EndsWith(".dll", StringComparison.OrdinalIgnoreCase))
            {
                n = n.Substring(0, n.Length - 4);
            }
            return n;
        }

        private bool IsCorlibSystemOrSystemCorePInvoke()
        {
            string asmName = _method.MethodDef.Module.Assembly.Name;
            return asmName.Equals("mscorlib", StringComparison.OrdinalIgnoreCase)
                || asmName.Equals("System", StringComparison.OrdinalIgnoreCase)
                || asmName.Equals("System.Core", StringComparison.OrdinalIgnoreCase);
        }

        private static TypeDef ResolveTypeDefForStructTypeSig(TypeSig type)
        {
            type = type.RemovePinnedAndModifiers();
            if (type.ElementType == ElementType.GenericInst)
            {
                return ((GenericInstSig)type).GenericType.ToTypeDefOrRef().ResolveTypeDefThrow();
            }
            return type.ToTypeDefOrRef().ResolveTypeDefThrow();
        }

        private static void EnsureBlittableStructForPInvoke(TypeSig structType, bool isReturnType)
        {
            structType = structType.RemovePinnedAndModifiers();
            TypeDef typeDef = ResolveTypeDefForStructTypeSig(structType);
            if (typeDef.IsEnum)
            {
                return;
            }
            int? layoutKind = TryGetStructLayoutKindFromStructLayoutAttribute(typeDef);
            if (layoutKind == (int)LayoutKind.Auto)
            {
                string r = isReturnType ? "return" : "parameter";
                throw new NotSupportedException(
                    $"PInvoke {r}: struct `{typeDef.FullName}` uses LayoutKind.Auto and cannot be passed by value with a stable unmanaged layout.");
            }
            GenericArgumentContext ctx = null;
            if (structType.ElementType == ElementType.GenericInst)
            {
                var g = (GenericInstSig)structType;
                ctx = new GenericArgumentContext(g.GenericArguments, null);
            }
            foreach (FieldDef field in typeDef.Fields)
            {
                if (field.IsStatic)
                {
                    continue;
                }
                TypeSig ft = MetaUtil.Inflate(field.FieldType, ctx);
                if (!IsPInvokeBlittableFieldType(ft))
                {
                    string r = isReturnType ? "return" : "parameter";
                    throw new NotSupportedException(
                        $"PInvoke {r}: struct `{typeDef.FullName}` has a non-blittable instance field `{field.Name}` ({ft}).");
                }
            }
        }

        private static int? TryGetStructLayoutKindFromStructLayoutAttribute(TypeDef typeDef)
        {
            foreach (CustomAttribute ca in typeDef.CustomAttributes)
            {
                if (ca.AttributeType.FullName != "System.Runtime.InteropServices.StructLayoutAttribute")
                {
                    continue;
                }
                if (ca.ConstructorArguments.Count == 0)
                {
                    return null;
                }
                object v = ca.ConstructorArguments[0].Value;
                if (v is int i)
                {
                    return i;
                }
                if (v is byte b)
                {
                    return b;
                }
                if (v is sbyte sb)
                {
                    return sb;
                }
                if (v is short s)
                {
                    return s;
                }
                if (v is ushort us)
                {
                    return us;
                }
                if (v is uint ui)
                {
                    return (int)ui;
                }
                return null;
            }
            return null;
        }

        private static bool IsPInvokeBlittableFieldType(TypeSig t)
        {
            t = t.RemovePinnedAndModifiers();
            switch (t.ElementType)
            {
            case ElementType.Void:
                return false;
            case ElementType.Boolean:
            case ElementType.Char:
            case ElementType.I1:
            case ElementType.U1:
            case ElementType.I2:
            case ElementType.U2:
            case ElementType.I4:
            case ElementType.U4:
            case ElementType.I8:
            case ElementType.U8:
            case ElementType.R4:
            case ElementType.R8:
            case ElementType.I:
            case ElementType.U:
            case ElementType.Ptr:
            case ElementType.FnPtr:
                return true;
            case ElementType.TypedByRef:
            case ElementType.String:
            case ElementType.Object:
            case ElementType.Class:
            case ElementType.Array:
            case ElementType.SZArray:
            case ElementType.ByRef:
                return false;
            case ElementType.ValueType:
            {
                TypeDef td = t.ToTypeDefOrRef().ResolveTypeDefThrow();
                if (td.IsEnum)
                {
                    return IsPInvokeBlittableFieldType(td.GetEnumUnderlyingType());
                }
                EnsureBlittableStructForPInvoke(t, false);
                return true;
            }
            case ElementType.GenericInst:
            {
                var gi = (GenericInstSig)t;
                TypeDef td = gi.GenericType.ToTypeDefOrRef().ResolveTypeDefThrow();
                if (td.IsEnum)
                {
                    return IsPInvokeBlittableFieldType(td.GetEnumUnderlyingType());
                }
                if (!td.IsValueType)
                {
                    return false;
                }
                EnsureBlittableStructForPInvoke(t, false);
                return true;
            }
            default:
                return false;
            }
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

        private static string StandardizedDllName(string value)
        {
            return value.Replace('.', '_').Replace('/', '_').Replace('\\', '_').Replace('-', '_');
        }
    }
}
