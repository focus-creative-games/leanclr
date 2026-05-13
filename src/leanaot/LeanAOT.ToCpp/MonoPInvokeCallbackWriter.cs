using System;
using System.Collections.Generic;
using System.Linq;
using dnlib.DotNet;
using LeanAOT.Core;
using LeanAOT.GenerationPlan;

namespace LeanAOT.ToCpp
{
    /// <summary>
    /// Emits <c>extern "C"</c> reverse P/Invoke thunks for methods marked with <c>[MonoPInvokeCallback]</c>:
    /// marshal native parameters onto the interpreter stack and dispatch via <see cref="VmFunctionNames.InvokeWithRunClassStaticConstructor"/>.
    /// </summary>
    static class MonoPInvokeCallbackWriter
    {
        public static string GetNativeThunkSymbolName(ModuleDef mod, MethodDef method)
        {
            return $"__leanclr_mono_pinvoke_cb_{ModuleGenerationUtil.GetStandardizedModuleNameWithoutExt(mod)}_{method.MDToken.ToInt32():X8}";
        }

        public static void GenerateAll(AssemblyPlan plan, ForwardDeclaration forwardDeclaration, CodeThrunkWriter implWriter)
        {
            foreach (MethodDefPlan mp in plan.MonoPInvokeCallbackMethodPlans)
            {
                MethodDef method = mp.MethodDef;
                MethodDetail methodDetail = GlobalServices.Inst.MetadataService.GetMethodDetail(method);
                GlobalServices.Inst.InvokerService.GetInvoker(method);
                forwardDeclaration.AddMethodForwardDeclaration(method);
                WriteThunk(plan.Module, implWriter, methodDetail);
            }
        }

        private static void WriteThunk(ModuleDef mod, CodeThrunkWriter w, MethodDetail methodDetail)
        {
            MethodDef method = methodDetail.MethodDef;
            string sym = GetNativeThunkSymbolName(mod, method);
            string nativeRet = GetNativeReturnType(methodDetail);
            string returnStmt = methodDetail.IsVoidReturn ? string.Empty : "return __native_ret;";

            string callConv = "LEANCLR_PINVOKE_CALL_CDECL";
            var nativeParams = methodDetail.ParamsIncludeThis.Select((p, i) => $"{GetNativeParamType(p.Type)} __arg{i}").ToList();
            string paramDecl = string.Join(", ", nativeParams);

            w.AddLine();
            w.AddLine($"extern \"C\" {nativeRet} {callConv} {sym}({paramDecl}){ConstStrings.CppFunctionNoexcept}");
            w.AddLine("{");
            w.IncreaseIndent();

            int paramCount = methodDetail.ParamCountIncludeThis;
            if (paramCount > 0)
            {
                w.AddLine("constexpr size_t ARG0_OFFSET = 0;");
                for (int paramIndex = 0, last = paramCount - 1; paramIndex < last; paramIndex++)
                {
                    ParamDetail param = methodDetail.ParamsIncludeThis[paramIndex];
                    w.AddLine($"constexpr size_t ARG{paramIndex + 1}_OFFSET = ARG{paramIndex}_OFFSET + {VmFunctionNames.GetStackObjectSizeForType}<{MethodGenerationUtil.GetExactTypeName(param.Type)}>();");
                }
                w.AddLine($"constexpr size_t ARGS_SIZE = ARG{paramCount - 1}_OFFSET + {VmFunctionNames.GetStackObjectSizeForType}<{MethodGenerationUtil.GetExactTypeName(methodDetail.ParamsIncludeThis.Last().Type)}>();");
                w.AddLine($"{ConstStrings.StackObjectTypeName} __args[ARGS_SIZE];");
                for (int i = 0; i < paramCount;)
                {
                    ParamDetail p = methodDetail.ParamsIncludeThis[i];
                    TypeSig t = p.Type.RemovePinnedAndModifiers();
                    if (t.ElementType == ElementType.SZArray)
                    {
                        if (!TryGetFollowingNativeLengthIndex(methodDetail.ParamsIncludeThis, i, out int lenIdx))
                        {
                            throw new NotSupportedException(
                                $"MonoPInvokeCallback: parameter #{i} is SZArray but is not followed by int32_t/uint32_t length (use e.g. int[] arr, int count). Method: {methodDetail.FullName}.");
                        }
                        EmitMarshalNativeSzArrayFromPointerAndLength(w, method, t, i, lenIdx, methodDetail.ParamsIncludeThis[lenIdx]);
                        i += 2;
                        continue;
                    }
                    EmitMarshalNativeArgToEvalStack(w, p, i);
                    i++;
                }
            }

            string argsPtr = paramCount > 0 ? "__args" : "nullptr";
            string invokeFn = methodDetail.IsStatic ? VmFunctionNames.InvokeWithRunClassStaticConstructor : VmFunctionNames.InvokeWithoutRunClassStaticConstructor;

            string cachedMethodVarName = $"s_mono_pinvoke_resolved_method_{method.MDToken.ToInt32():X8}";
            w.AddLine($"static {ConstStrings.MethodInfoPtrTypeName} {cachedMethodVarName} = nullptr;");
            w.AddLine($"if (LEANCLR_UNLIKELY({cachedMethodVarName} == nullptr))");
            w.BeginBlock();
            w.AddLine($"{cachedMethodVarName} = reinterpret_cast<{ConstStrings.MethodInfoPtrTypeName}>({ConstStrings.CodegenNamespace}::resolve_metadata_token({ModuleGenerationUtil.GetModuleGlobalVariableName(mod)}, 0x{method.MDToken.ToInt32():X8}, nullptr));");
            w.EndBlock();
            w.AddLine($"{ConstStrings.MethodInfoPtrTypeName} __method = {cachedMethodVarName};");
            if (methodDetail.IsVoidReturn)
            {
                w.AddLine($"auto __inv_r = {invokeFn}(__method, {argsPtr}, nullptr);");
                w.AddLine("if (LEANCLR_UNLIKELY(__inv_r.is_err())) { std::abort(); }");
            }
            else
            {
                w.AddLine($"constexpr size_t RET_SIZE = {VmFunctionNames.GetStackObjectSizeForType}<{MethodGenerationUtil.GetExactTypeName(methodDetail.RetType)}>();");
                w.AddLine($"{ConstStrings.StackObjectTypeName} __ret[RET_SIZE];");
                w.AddLine($"auto __inv_r = {invokeFn}(__method, {argsPtr}, __ret);");
                w.AddLine("if (LEANCLR_UNLIKELY(__inv_r.is_err())) { std::abort(); }");
                string retExact = MethodGenerationUtil.GetExactTypeName(methodDetail.RetType);
                w.AddLine($"{nativeRet} __native_ret = {ConstStrings.CodegenNamespace}::get_eval_stack_value_as_type<{retExact}>(__ret);");
                w.AddLine(returnStmt);
            }

            w.DecreaseIndent();
            w.AddLine("}");
        }

        private static void EmitMarshalNativeArgToEvalStack(CodeThrunkWriter w, ParamDetail param, int index)
        {
            TypeSig type = param.Type.RemovePinnedAndModifiers();
            string dst = $"__args + ARG{index}_OFFSET";
            if (type.ElementType == ElementType.ByRef)
            {
                TypeSig inner = ((ByRefSig)type).Next.RemovePinnedAndModifiers();
                string innerExact = MethodGenerationUtil.GetExactTypeName(inner);
                w.AddLine($"{VmFunctionNames.ExpandArgumentToEvalStack}(*reinterpret_cast<{innerExact}*>(__arg{index}), {dst});");
                return;
            }
            if (type.ElementType == ElementType.String)
            {
                w.AddLine($"{{ auto __s = {ConstStrings.CodegenNamespace}::marshal_utf8_string_to_utf16(__arg{index}); {VmFunctionNames.ExpandArgumentToEvalStack}(__s, {dst}); }}");
                return;
            }
            if (type.ElementType == ElementType.SZArray)
            {
                throw new InvalidOperationException("SZArray marshalling must use EmitMarshalNativeSzArrayFromPointerAndLength (paired with length parameter).");
            }
            string expandType = GetExpandTypeForNativeEdge(type, index);
            w.AddLine($"{VmFunctionNames.ExpandArgumentToEvalStack}(static_cast<{expandType}>(__arg{index}), {dst});");
        }

        private static bool TryGetFollowingNativeLengthIndex(IReadOnlyList<ParamDetail> @params, int arrayParamIndex, out int lengthParamIndex)
        {
            lengthParamIndex = arrayParamIndex + 1;
            if (arrayParamIndex + 1 >= @params.Count)
            {
                return false;
            }
            ElementType et = @params[arrayParamIndex + 1].Type.RemovePinnedAndModifiers().ElementType;
            return et == ElementType.I4 || et == ElementType.U4;
        }

        /// <summary>
        /// Native ABI: element pointer + length (e.g. <c>int32_t*</c> and <c>int32_t</c>); allocates a managed SZArray and copies blittable elements.
        /// </summary>
        private static void EmitMarshalNativeSzArrayFromPointerAndLength(
            CodeThrunkWriter w,
            MethodDef method,
            TypeSig arrayParamType,
            int arrayArgIndex,
            int lengthArgIndex,
            ParamDetail lengthParam)
        {
            TypeSig elem = ((SZArraySig)arrayParamType.RemovePinnedAndModifiers()).Next.RemovePinnedAndModifiers();
            TypeDef elemTd = elem.ToTypeDefOrRef().ResolveTypeDefThrow();
            string elemCache = $"s_mono_pinvoke_szarray_ele_{method.MDToken.ToInt32():X8}_{arrayArgIndex}";
            string modGv = ModuleGenerationUtil.GetModuleGlobalVariableName(elemTd.Module);
            string tok = $"0x{elemTd.MDToken.ToInt32():X8}";
            string dstArr = $"__args + ARG{arrayArgIndex}_OFFSET";
            string dstLen = $"__args + ARG{lengthArgIndex}_OFFSET";
            string lenExact = MethodGenerationUtil.GetExactTypeName(lengthParam.Type.RemovePinnedAndModifiers());

            w.AddLine($"if (__arg{arrayArgIndex} == nullptr)");
            w.BeginBlock();
            w.AddLine($"{VmFunctionNames.ExpandArgumentToEvalStack}(static_cast<{ConstStrings.ArrayPtrTypeName}>(nullptr), {dstArr});");
            w.EndBlock();
            w.AddLine("else");
            w.BeginBlock();
            w.AddLine($"static {ConstStrings.ClassPtrTypeName} {elemCache} = nullptr;");
            w.AddLine($"if (LEANCLR_UNLIKELY({elemCache} == nullptr))");
            w.BeginBlock();
            w.AddLine($"{elemCache} = reinterpret_cast<{ConstStrings.ClassPtrTypeName}>({ConstStrings.CodegenNamespace}::resolve_metadata_token({modGv}, {tok}, nullptr));");
            w.EndBlock();
            w.AddLine($"auto __arr_r{arrayArgIndex} = {ConstStrings.CodegenNamespace}::mono_pinvoke_reverse_marshal_szarray_blittable_copy({elemCache}, __arg{arrayArgIndex}, static_cast<int32_t>(__arg{lengthArgIndex}));");
            w.AddLine($"if (LEANCLR_UNLIKELY(__arr_r{arrayArgIndex}.is_err())) {{ std::abort(); }}");
            w.AddLine($"auto* __arr{arrayArgIndex} = __arr_r{arrayArgIndex}.unwrap();");
            w.AddLine($"{VmFunctionNames.ExpandArgumentToEvalStack}(__arr{arrayArgIndex}, {dstArr});");
            w.EndBlock();
            w.AddLine($"{VmFunctionNames.ExpandArgumentToEvalStack}(static_cast<{lenExact}>(__arg{lengthArgIndex}), {dstLen});");
        }

        /// <summary>Expression type for <c>static_cast</c> from native thunk parameter into overload used by expand_argument_to_eval_stack.</summary>
        private static string GetExpandTypeForNativeEdge(TypeSig type, int argIndex)
        {
            type = type.RemovePinnedAndModifiers();
            switch (type.ElementType)
            {
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
            case ElementType.Class:
            case ElementType.ValueType:
            case ElementType.GenericInst:
            case ElementType.FnPtr:
                return MethodGenerationUtil.GetExactTypeName(type);
            default:
                throw new NotSupportedException($"MonoPInvokeCallback parameter #{argIndex} type {type} is not supported yet.");
            }
        }

        private static string GetNativeReturnType(MethodDetail methodDetail)
        {
            return GetNativeParamType(methodDetail.RetType);
        }

        private static string GetNativeParamType(TypeSig type)
        {
            type = type.RemovePinnedAndModifiers();
            switch (type.ElementType)
            {
            case ElementType.Void:
                return "void";
            case ElementType.Boolean:
                return "bool";
            case ElementType.Char:
                return "leanclr::Utf16Char";
            case ElementType.I1:
                return "int8_t";
            case ElementType.U1:
                return "uint8_t";
            case ElementType.I2:
                return "int16_t";
            case ElementType.U2:
                return "uint16_t";
            case ElementType.I4:
                return "int32_t";
            case ElementType.U4:
                return "uint32_t";
            case ElementType.I8:
                return "int64_t";
            case ElementType.U8:
                return "uint64_t";
            case ElementType.R4:
                return ConstStrings.Float32TypeName;
            case ElementType.R8:
            case ElementType.R:
                return ConstStrings.Float64TypeName;
            case ElementType.I:
                return "intptr_t";
            case ElementType.U:
                return "uintptr_t";
            case ElementType.Ptr:
                return $"{GetNativeParamType(((PtrSig)type).Next)}*";
            case ElementType.ByRef:
                return $"{GetNativeParamType(((ByRefSig)type).Next)}*";
            case ElementType.FnPtr:
                return "void*";
            case ElementType.SZArray:
            {
                TypeSig elem = ((SZArraySig)type).Next.RemovePinnedAndModifiers();
                return $"{GetNativeParamType(elem)}*";
            }
            case ElementType.String:
                return "const char*";
            case ElementType.ValueType:
            case ElementType.GenericInst:
            {
                TypeDef typeDef = type.ToTypeDefOrRef().ResolveTypeDefThrow();
                if (typeDef.IsEnum)
                {
                    return GetNativeParamType(typeDef.GetEnumUnderlyingType());
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
                if (MetaUtil.IsInheritFrom(typeDef, "System.Delegate")
                    || MetaUtil.IsInheritFrom(typeDef, "System.Runtime.InteropServices.SafeHandle")
                    || MetaUtil.IsInheritFrom(typeDef, "System.Runtime.InteropServices.CriticalHandle"))
                {
                    return "void*";
                }
                return ConstStrings.ObjectPtrTypeName;
            }
            default:
                throw new NotSupportedException($"MonoPInvokeCallback native ABI does not support type {type.FullName}.");
            }
        }
    }
}
