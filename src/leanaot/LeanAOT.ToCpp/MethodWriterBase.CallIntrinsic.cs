using dnlib.DotNet;
using dnlib.DotNet.Emit;
using LeanAOT.Core;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LeanAOT.ToCpp
{
    partial class MethodWriterBase
    {
        private bool TryEmitCallInstrinsic(Instruction inst, MethodDetail methodDetail, Func<string> methodVarNameProvider, List<EvalVariable> args, EvalVariable retVar)
        {
            MethodDef methodDef = methodDetail.MethodDef;
            if (methodDef == null)
            {
                TypeSig declaringTypeSig = methodDetail.Method.DeclaringType.ToTypeSig();
                if (declaringTypeSig.ElementType != ElementType.Array)
                {
                    throw new Exception($"impossible: methodDef is null but the declaring type is not an array. method:{methodDetail.Method}");
                }
                switch (methodDetail.Method.Name)
                {
                case "Get":
                {
                    EmitCallMdArrayGetIntrinsic(inst, methodDetail, methodVarNameProvider(), args, retVar);
                    return true;
                }
                case "Set":
                {
                    EmitCallMdArraySetIntrinsic(inst, methodDetail, methodVarNameProvider(), args, retVar);
                    return true;
                }
                case "Address":
                {
                    EmitCallMdArrayAddressIntrinsic(inst, methodDetail, methodVarNameProvider(), args, retVar);
                    return true;
                }
                }
                return false;
            }

            if (!MetaUtil.IsCorlibOrSystemOrSystemCore(methodDef.Module))
            {
                return false;
            }

            UTF8String methodName = methodDef.Name;
            TypeDef declaringType = methodDef.DeclaringType;
            switch (declaringType.Name.ToString())
            {
            case "Object":
            {
                if (methodName == VmFunctionNames.Ctor)
                {
                    // System.Object's .ctor is an empty method, we can just ignore the call to it.
                    return true;
                }
                break;
            }
            case "String":
            {
                break;
            }
            case "Assembly":
            {
                if (methodName == "GetExecutingAssembly")
                {
                    EmitAssignOrThrow(inst, retVar, $"{ConstStrings.CodegenNamespace}::get_assembly_reflection_object({CurMethodVar.GetFullReferenceVariableName()}->parent->image)");
                    return true;
                }
                break;
            }
            case "MethodBase":
            {
                if (methodName == "GetCurrentMethod")
                {
                    EmitAssignOrThrow(inst, retVar, $"{ConstStrings.CodegenNamespace}::get_method_reflection_object({CurMethodVar.GetFullReferenceVariableName()})");
                    return true;
                }
                break;
            }
            default:
                break;
            }
            
            
            var runtimeApiCatalog = GlobalServices.Inst.RuntimeApiCatalog;
            if (runtimeApiCatalog.TryGetIcallOrIntrinsic(methodDef, out var entry, out var _methodKind))
            {
                EmitCallICallOrIntrinsic(inst, methodDetail, entry, args, retVar);
                return true;
            }

            return false;
        }


        private bool TryEmitCallvirIntrinsic(Instruction inst, MethodDetail methodDetail, string methodVarName, List<EvalVariable> args, EvalVariable retVar)
        {

            return false;
        }


        private MethodDef GetRedirectedCtorMethod(TypeDef stringTypeDef, MethodDef ctorMethodDef)
        {
            foreach (var redirectedMethod in stringTypeDef.Methods)
            {
                if ((redirectedMethod.Name == "CreateString" || redirectedMethod.Name == "Ctor") && redirectedMethod.ParamDefs.Count == ctorMethodDef.ParamDefs.Count)
                {
                    for (int i = 0; i < redirectedMethod.ParamDefs.Count; i++)
                    {
                        TypeSig redirectedMethodParamType = redirectedMethod.GetParam(i);
                        TypeSig ctorMethodParamType = ctorMethodDef.GetParam(i + 1); // the first parameter is the this pointer
                        if (!TypeEqualityComparer.Instance.Equals(redirectedMethodParamType, ctorMethodParamType))
                        {
                            break;
                        }
                    }
                    return redirectedMethod;
                }
            }
            throw new Exception($"impossible: the redirected ctor method of System.String not found. ctor:{ctorMethodDef.FullName}");
        }


        private bool TryRedirectNewObjIntrinsic(Instruction inst, MethodDetail methodDetail, out MethodDef redirectedMethod)
        {
            redirectedMethod = null;
            MethodDef methodDef = methodDetail.MethodDef;
            if (methodDef == null)
            {
                return false;
            }
            if (!MetaUtil.IsCorlibOrSystemOrSystemCore(methodDef.Module))
            {
                return false;
            }
            switch (methodDef.FullName)
            {
            case "System.Void System.String::.ctor(System.SByte*,System.Int32,System.Int32,System.Text.Encoding)":
            {
                redirectedMethod = GetRedirectedCtorMethod(methodDef.DeclaringType, methodDef);
                return true;
            }
            }
            return false;
        }

        private bool TryEmitNewobjIntrinsic(Instruction inst, MethodDetail methodDetail, Func<string> methodVarNameProvider, List<EvalVariable> args, EvalVariable retVar)
        {
            MethodDef methodDef = methodDetail.MethodDef;
            if (methodDef == null)
            {
                TypeSig declaringTypeSig = methodDetail.Method.DeclaringType.ToTypeSig();
                switch (declaringTypeSig.ElementType)
                {
                case ElementType.Array:
                {
                    EmitNewMdArrayIntrinsic(inst, methodDetail, methodVarNameProvider(), args, retVar);
                    return true;
                }
                default:
                {
                    throw new Exception("impossible: methodDef is null but the declaring type is not an array.");
                }
                }
            }

            UTF8String methodName = methodDef.Name;
            TypeDef declaringType = methodDef.DeclaringType;

            if (MetaUtil.IsDerivedFromMulticastDelegate(declaringType))
            {
                // We will handle delegate creation in a special way, so we won't treat it as a normal newobj.
                switch (methodName)
                {
                case VmFunctionNames.Ctor:
                {
                    EmitNewMulticastDelegateIntrinsic(inst, methodDetail, methodVarNameProvider(), args, retVar);
                    return true;
                }
                }
                return false;
            }
            if (!MetaUtil.IsCorlibOrSystemOrSystemCore(methodDef.Module))
            {
                return false;
            }

            string icallsHeader = "icalls/system_string.h";
            string icallsFuncName;
            var argsStr = CreateMethodFunctionArgsExcludedThisWithCast(methodDetail, args);
            switch (methodDef.FullName)
            {
            case "System.Void System.String::.ctor(System.Char[])":
            {
                icallsFuncName = "SystemString::newobj_char_array";
                break;
            }
            case "System.Void System.String::.ctor(System.Char[],System.Int32,System.Int32)":
            {
                icallsFuncName = "SystemString::newobj_char_array_range";
                break;
            }
            case "System.Void System.String::.ctor(System.Char*)":
            {
                icallsFuncName = "SystemString::newobj_utf16chars";
                break;
            }
            case "System.Void System.String::.ctor(System.Char*,System.Int32,System.Int32)":
            {
                icallsFuncName = "SystemString::newobj_utf16chars_range";
                break;
            }
            case "System.Void System.String::.ctor(System.SByte*)":
            {
                icallsFuncName = "SystemString::newobj_utf8chars";
                break;
            }
            case "System.Void System.String::.ctor(System.SByte*,System.Int32,System.Int32)":
            {
                icallsFuncName = "SystemString::newobj_utf8chars_range";
                break;
            }
            case "System.Void System.String::.ctor(System.Char,System.Int32)":
            {
                icallsFuncName = "SystemString::newobj_char_count";
                break;
            }
            case "System.Void System.String::.ctor(System.ReadOnlySpan`1<System.Char>)":
            {
                icallsFuncName = "SystemString::newobj_readonlyspan";
                argsStr = $"*(leanclr::vm::RtReadOnlySpan<leanclr::Utf16Char>*)&{argsStr}";
                break;
            }
            //case "System.Void System.String::.ctor(System.SByte*,System.Int32,System.Int32,System.Text.Encoding)":
            //{
            //    return false;
            //}
            default:
            {
                return false;
            }
            }
            _forwardDeclaration.AddInclude(icallsHeader);
            EmitDeclaringAssignOrThrow(inst, retVar, $"leanclr::icalls::{icallsFuncName}({argsStr})");
            EmitAssumeNotNull(retVar);
            return true;
        }

        private void EmitCallICallOrIntrinsic(Instruction inst, MethodDetail methodDetail, RuntimeApiEntry entry, List<EvalVariable> args, EvalVariable retVar)
        {
            _forwardDeclaration.AddInclude(entry.Header);
            var argsStr = CreateMethodFunctionArgsWithCast(methodDetail, args);
            string namespaceStr = entry.MethodKind == MethodKind.ICall || entry.MethodKind == MethodKind.ICallNewObj ? "leanclr::icalls" : "leanclr::intrinsics";
            string funcFullName = $"{namespaceStr}::{entry.Func}";
            if (methodDetail.IsVoidReturn)
            {
                EmitThrowOnError(inst, $"(({methodDetail.CreateMethodFunctionTypeDefineWithoutName()}){funcFullName})({argsStr})");
            }
            else
            {
                string relaxRetTypeName = MethodGenerationUtil.GetCppTypeNameAsFieldOrArgOrLoc(methodDetail.RetType, TypeNameRelaxLevel.AbiRelaxed);
                _bodyWriter.BeginBlock();
                _bodyWriter.AddLine($"using __RetType = leanclr::core::function_return<decltype({funcFullName})>::type;");
                EmitAssignOrThrow(inst, retVar, $"(({methodDetail.CreateOverrideRetTypeRelaxMethodFunctionTypeDefine("", "__RetType")}){funcFullName})({argsStr})");
                _bodyWriter.EndBlock();
            }
        }

        private void DefineLengthsAndBoundsVar(List<EvalVariable> args, int rank)
        {
            if (args.Count == rank)
            {
                _bodyWriter.AddLine($"int32_t __lengths[] = {{{string.Join(" ,", args.Select(p => GetEvalVariableExprWithCast(p, "int32_t")))}}};");
                _bodyWriter.AddLine($"int32_t* __lowerBounds = nullptr;");
            }
            else if (args.Count == rank * 2)
            {
                List<EvalVariable> lengthArgs = args.GetRange(0, rank).ToList();
                List<EvalVariable> lowerBoundArgs = args.GetRange(rank, rank).ToList();
                _bodyWriter.AddLine($"int32_t __lengths[] = {{{string.Join(" ,", lengthArgs.Select(p => GetEvalVariableExprWithCast(p, "int32_t")))}}};");
                _bodyWriter.AddLine($"int32_t* __lowerBounds = {{{string.Join(" ,", lowerBoundArgs.Select(p => GetEvalVariableExprWithCast(p, "int32_t")))}}};");
            }
            else
            {
                throw new Exception("impossible: the number of parameters of a multidimensional array constructor should be either equal to the rank of the array or equal to the rank of the array multiplied by 2.");
            }
        }

        private void DefineIndexVar(List<EvalVariable> args, int rank)
        {
            if (args.Count == rank)
            {
                _bodyWriter.AddLine($"int32_t __indexs[] = {{{string.Join(" ,", args.Select(p => GetEvalVariableExprWithCast(p, "int32_t")))}}};");
            }
            else
            {
                throw new Exception("impossible: the number of parameters of a multidimensional array constructor should be either equal to the rank of the array or equal to the rank of the array multiplied by 2.");
            }
        }

        private void EmitNewMdArrayIntrinsic(Instruction inst, MethodDetail methodDetail, string methodVarName, List<EvalVariable> args, EvalVariable retVar)
        {
            if (args.Count < 1)
            {
                throw new Exception("impossible: the constructor of a multidimensional array should always have at least 1 parameters.");
            }
            ArraySig declaringTypeSig = (ArraySig)methodDetail.Method.DeclaringType.ToTypeSig().RemoveModifiers();
            ParamDetail[] paramsIncludeThis = methodDetail.ParamsIncludeThis;
            int rank = (int)declaringTypeSig.Rank;
            _bodyWriter.AddLine($"{GetTypeName(retVar)} {GetEvalVariableName(retVar)};");
            _bodyWriter.AddLine("{");
            _bodyWriter.IncreaseIndent();
            DefineLengthsAndBoundsVar(args, rank);
            EmitAssignOrThrow(inst, retVar, $"{VmFunctionNames.NewMdArrayFromArrayKlass}({GetParentFromFullReferenceMethodVariable(methodVarName)}, __lengths, __lowerBounds)");
            _bodyWriter.DecreaseIndent();
            _bodyWriter.AddLine("}");
            EmitAssumeNotNull(retVar);
        }

        private void EmitCallMdArrayGetIntrinsic(Instruction inst, MethodDetail methodDetail, string methodVarName, List<EvalVariable> args, EvalVariable retVar)
        {
            if (args.Count < 2)
            {
                throw new Exception("impossible: the constructor of a multidimensional array should always have at least 2 parameters.");
            }
            ArraySig declaringTypeSig = (ArraySig)methodDetail.Method.DeclaringType.ToTypeSig().RemoveModifiers();
            ParamDetail[] paramsIncludeThis = methodDetail.ParamsIncludeThis;
            int rank = (int)declaringTypeSig.Rank;
            EvalVariable arrVar = args[0];
            EmitCheckNotNull(inst, arrVar);

            //_bodyWriter.AddLine($"{GetTypeName(retVar)} {GetEvalVariableName(retVar)};");
            _bodyWriter.AddLine("{");
            _bodyWriter.IncreaseIndent();
            DefineIndexVar(args.GetRange(1, args.Count - 1), rank);
            string globalIndexVarName = "__globalIndex";
            _bodyWriter.AddLine($"{VmFunctionNames.DECLARING_ASSIGN_OR_THROW}(int32_t, {globalIndexVarName}, {VmFunctionNames.GetMdArrayGlobalIndex}({GetVariableMayCast(arrVar, ConstStrings.ArrayPtrTypeName)}, __indexs), {CurMethodVar.GetFullReferenceVariableName()}, {GetCurrentIpOffset(inst)});");
            ITypeDefOrRef elementType = methodDetail.RetType.ToTypeDefOrRef();
            string elementTypeName = GetExactTypeName(elementType);
            string loadElementDataExpr = $"{VmFunctionNames.GetArrayElementDataAt}<{elementTypeName}>({GetVariableMayCast(arrVar, ConstStrings.ArrayPtrTypeName)}, {globalIndexVarName})";
            _bodyWriter.AddLine($"{GetEvalVariableName(retVar)} = {MayFoldCast(elementTypeName, GetTypeName(retVar), loadElementDataExpr)};");
            _bodyWriter.DecreaseIndent();
            _bodyWriter.AddLine("}");
        }

        private void EmitCallMdArraySetIntrinsic(Instruction inst, MethodDetail methodDetail, string methodVarName, List<EvalVariable> args, EvalVariable retVar)
        {
            if (args.Count < 3)
            {
                throw new Exception("impossible: the constructor of a multidimensional array should always have at least 3 parameters.");
            }
            ArraySig declaringTypeSig = (ArraySig)methodDetail.Method.DeclaringType.ToTypeSig().RemoveModifiers();
            ParamDetail[] paramsIncludeThis = methodDetail.ParamsIncludeThis;
            int rank = (int)declaringTypeSig.Rank;
            EvalVariable arrVar = args[0];
            EvalVariable valueVar = args.Last();
            EmitCheckNotNull(inst, arrVar);

            //_bodyWriter.AddLine($"{GetTypeName(retVar)} {GetEvalVariableName(retVar)};");
            _bodyWriter.AddLine("{");
            _bodyWriter.IncreaseIndent();
            DefineIndexVar(args.GetRange(1, args.Count - 2), rank);
            string globalIndexVarName = "__globalIndex";
            _bodyWriter.AddLine($"{VmFunctionNames.DECLARING_ASSIGN_OR_THROW}(int32_t, {globalIndexVarName}, {VmFunctionNames.GetMdArrayGlobalIndex}({GetVariableMayCast(arrVar, ConstStrings.ArrayPtrTypeName)}, __indexs), {CurMethodVar.GetFullReferenceVariableName()}, {GetCurrentIpOffset(inst)});");
            ITypeDefOrRef elementType = paramsIncludeThis.Last().Type.ToTypeDefOrRef();
            string elementTypeName = GetExactTypeName(elementType);

            string valueVarName = GetEvalVariableName(valueVar);
            _bodyWriter.AddLine($"{VmFunctionNames.SetArrayElementDataAt}<{elementTypeName}>({GetVariableMayCast(arrVar, ConstStrings.ArrayPtrTypeName)}, {globalIndexVarName}, {GetEvalVariableExprWithCast(valueVar, elementTypeName)});");
            _bodyWriter.DecreaseIndent();
            _bodyWriter.AddLine("}");
        }

        private void EmitCallMdArrayAddressIntrinsic(Instruction inst, MethodDetail methodDetail, string methodVarName, List<EvalVariable> args, EvalVariable retVar)
        {
            if (args.Count < 2)
            {
                throw new Exception("impossible: the constructor of a multidimensional array should always have at least 2 parameters.");
            }
            ArraySig declaringTypeSig = (ArraySig)methodDetail.Method.DeclaringType.ToTypeSig().RemoveModifiers();
            ParamDetail[] paramsIncludeThis = methodDetail.ParamsIncludeThis;
            int rank = (int)declaringTypeSig.Rank;
            EvalVariable arrVar = args[0];
            EmitCheckNotNull(inst, arrVar);

            //_bodyWriter.AddLine($"{GetTypeName(retVar)} {GetEvalVariableName(retVar)};");
            _bodyWriter.AddLine("{");
            _bodyWriter.IncreaseIndent();
            DefineIndexVar(args.GetRange(1, args.Count - 1), rank);
            string globalIndexVarName = "__globalIndex";
            _bodyWriter.AddLine($"{VmFunctionNames.DECLARING_ASSIGN_OR_THROW}(int32_t, {globalIndexVarName}, {VmFunctionNames.GetMdArrayGlobalIndex}({GetVariableMayCast(arrVar, ConstStrings.ArrayPtrTypeName)}, __indexs), {CurMethodVar.GetFullReferenceVariableName()}, {GetCurrentIpOffset(inst)});");
            ITypeDefOrRef elementType = methodDetail.RetType.Next.ToTypeDefOrRef();
            string elementTypeName = GetExactTypeName(elementType);
            string loadElementDataExpr = $"{VmFunctionNames.GetArrayElementAddress}<{elementTypeName}>({GetVariableMayCast(arrVar, ConstStrings.ArrayPtrTypeName)}, {globalIndexVarName})";
            _bodyWriter.AddLine($"{GetEvalVariableName(retVar)} = {MayFoldCast($"{elementTypeName}*", GetTypeName(retVar), loadElementDataExpr)};");
            _bodyWriter.DecreaseIndent();
            _bodyWriter.AddLine("}");
        }

        private void EmitNewMulticastDelegateIntrinsic(Instruction inst, MethodDetail methodDetail, string methodVarName, List<EvalVariable> args, EvalVariable retVar)
        {
            if (args.Count != 2)
            {
                throw new Exception("impossible: the constructor of a delegate should always have 2 parameters.");
            }
            EmitDeclaringAssignOrThrow(inst, retVar, $"{VmFunctionNames.NewDelegate}({GetParentFromFullReferenceMethodVariable(methodVarName)}, ({ConstStrings.ObjectPtrTypeName}){GetEvalVariableName(args[0])}, ({ConstStrings.MethodInfoPtrTypeName}){GetEvalVariableName(args[1])})");
            EmitAssumeNotNull(retVar);
        }
    }
}
