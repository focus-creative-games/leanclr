using dnlib.DotNet;
using LeanAOT.Core;
using LeanAOT.ToCpp;
using System;
using System.Reflection;
using System.Runtime.InteropServices;
using CallingConvention = System.Runtime.InteropServices.CallingConvention;

namespace LeanAOT.ToCpp
{

    public static class MarshalUtil
    {
        public const UnmanagedType MaxManagedType = (UnmanagedType)0x50;
        public const uint InvalidElementCount = uint.MaxValue;

        public static string GetCharTypeName(CharSet charSet)
        {
            return charSet != CharSet.Ansi ? ConstStrings.Utf16CharTypeName : ConstStrings.AnsiCharTypeName;
        }

        public static string GetCharPtrTypeName(CharSet charSet)
        {
            return charSet != CharSet.Ansi ? ConstStrings.Utf16CharPtrTypeName : ConstStrings.AnsiCharPtrTypeName;
        }

        public static string GetMarshalNativeTypeName(TypeSig typeSig, MarshalAsInfo marshalAsInfo, CharSet charSet)
        {
            typeSig = typeSig.RemovePinnedAndModifiers();
            if (marshalAsInfo == null)
            {
                return GetMarshalNativeTypeName(typeSig, MaxManagedType, MaxManagedType, InvalidElementCount, InvalidElementCount, charSet);
            }
            return GetMarshalNativeTypeName(typeSig, marshalAsInfo.UnmanagedType, marshalAsInfo.ArrayElementType, marshalAsInfo.ParamIndex, marshalAsInfo.ElementCount, charSet);
        }

        public static string GetMarshalNativeTypeName(TypeSig typeSig, UnmanagedType unmanagedType, UnmanagedType arrayElementType, uint paramIndex, uint elementCount, CharSet charSet)
        {
            if (unmanagedType == MaxManagedType)
            {
                switch (typeSig.ElementType)
                {
                case ElementType.Void: return "void";
                case ElementType.Boolean:
                {
                    unmanagedType = UnmanagedType.Bool;
                    break;
                }
                case ElementType.Char:
                {
                    return GetCharTypeName(charSet);
                }
                case ElementType.I1:
                {
                    unmanagedType = UnmanagedType.I1;
                    break;
                }
                case ElementType.U1:
                {
                    unmanagedType = UnmanagedType.U1;
                    break;
                }
                case ElementType.I2:
                {
                    unmanagedType = UnmanagedType.I2;
                    break;
                }
                case ElementType.U2:
                {
                    unmanagedType = UnmanagedType.U2;
                    break;
                }
                case ElementType.I4:
                {
                    unmanagedType = UnmanagedType.I4;
                    break;
                }
                case ElementType.U4:
                {
                    unmanagedType = UnmanagedType.U4;
                    break;
                }
                case ElementType.I8:
                {
                    unmanagedType = UnmanagedType.I8;
                    break;
                }
                case ElementType.U8:
                {
                    unmanagedType = UnmanagedType.U8;
                    break;
                }
                case ElementType.R4:
                {
                    unmanagedType = UnmanagedType.R4;
                    break;
                }
                case ElementType.R8:
                {
                    unmanagedType = UnmanagedType.R8;
                    break;
                }
                case ElementType.I:
                {
                    unmanagedType = UnmanagedType.SysInt;
                    break;
                }
                case ElementType.U:
                {
                    unmanagedType = UnmanagedType.SysUInt;
                    break;
                }
                case ElementType.String:
                {
                    unmanagedType = UnmanagedType.LPStr;
                    break;
                }
                case ElementType.Object:
                {
                    unmanagedType = UnmanagedType.IUnknown;
                    break;
                }
                case ElementType.Class:
                {
                    if (MetaUtil.IsDerivedFromMulticastDelegate(typeSig.ToTypeDefOrRef()))
                    {
                        unmanagedType = UnmanagedType.FunctionPtr;
                    }
                    else
                    {
                        unmanagedType = UnmanagedType.IUnknown;
                    }
                    break;
                }
                case ElementType.ValueType:
                {
                    unmanagedType = UnmanagedType.Struct;
                    break;
                }
                case ElementType.GenericInst:
                {
                    throw new NotSupportedException("GenericInst is not supported for marshal.");
                }
                case ElementType.SZArray:
                case ElementType.Array:
                {
                    unmanagedType = UnmanagedType.LPArray;
                    break;
                }
                case ElementType.Ptr:
                {
                    unmanagedType = UnmanagedType.SysInt;
                    break;
                }
                case ElementType.ByRef:
                {
                    return $"{GetMarshalNativeTypeName(typeSig.Next, unmanagedType, arrayElementType, paramIndex, elementCount, charSet)}*";
                }
                case ElementType.FnPtr:
                {
                    unmanagedType = UnmanagedType.FunctionPtr;
                    break;
                }
                default:
                {
                    throw new NotSupportedException($"Unsupported ElementType: {typeSig.ElementType}");
                }
                }
            }
            switch (unmanagedType)
            {
            case UnmanagedType.Bool:
                return ConstStrings.MarshalBooleanTypeName;
            case UnmanagedType.I1:
                return "int8_t";
            case UnmanagedType.U1:
                return "uint8_t";
            case UnmanagedType.I2:
                return "int16_t";
            case UnmanagedType.U2:
                return "uint16_t";
            case UnmanagedType.I4:
                return "int32_t";
            case UnmanagedType.U4:
                return "uint32_t";
            case UnmanagedType.I8:
                return "int64_t";
            case UnmanagedType.U8:
                return "uint64_t";
            case UnmanagedType.R4:
                return ConstStrings.Float32TypeName;
            case UnmanagedType.R8:
                return ConstStrings.Float64TypeName;
            case UnmanagedType.SysInt:
                return "intptr_t";
            case UnmanagedType.SysUInt:
                return "uintptr_t";
            // obsolete
            // case UnmanagedType.Currency:
            //     return "leanclr::metadata::RtMarshalCurrency";
            case UnmanagedType.BStr:
            {
                return ConstStrings.Utf16CharPtrTypeName;
            }
            case UnmanagedType.LPStr:
            {
                return ConstStrings.AnsiCharPtrTypeName;
            }
            case UnmanagedType.LPWStr:
            case UnmanagedType.LPTStr:
            {
                return ConstStrings.Utf16CharPtrTypeName;
            }
            case UnmanagedType.ByValTStr:
            {
                if (elementCount == InvalidElementCount)
                {
                    throw new NotSupportedException("ByValTStr must have a fixed length.");
                }
                return $"{GetCharTypeName(charSet)}[{elementCount}]";
            }
            case UnmanagedType.IUnknown:
            {
                return ConstStrings.MarshalIUnknownTypeName;
            }
            case UnmanagedType.IDispatch:
            {
                return ConstStrings.MarshalIDispatchTypeName;
            }
            case UnmanagedType.Interface:
            {
                return ConstStrings.MarshalInterfaceTypeName;
            }
            case UnmanagedType.Struct:
            {
                return GlobalServices.Inst.TypeNameService.GetCppTypeNameAsFieldOrArgOrLoc(typeSig, TypeNameRelaxLevel.Exactly);
            }
            case UnmanagedType.SafeArray:
            {
                return ConstStrings.MarshalSafeArrayPtrTypeName;
            }
            case UnmanagedType.ByValArray:
            {
                if (elementCount == InvalidElementCount)
                {
                    throw new NotSupportedException("ByValArray must have a fixed length.");
                }
                if (typeSig.ElementType != ElementType.SZArray)
                {
                    throw new NotSupportedException("Only SZArray is supported for array elements.");
                }
                return $"{GetMarshalNativeTypeName(typeSig.Next, arrayElementType, MaxManagedType, InvalidElementCount, InvalidElementCount, charSet)}[{elementCount}]";
            }
            // obsolete
            // case UnmanagedType.VBByRefStr:
            // case UnmanagedType.AnsiBStr:
            // case UnmanagedType.TBStr:
            // {
            //     break;
            // }
            case UnmanagedType.VariantBool:
            {
                // A 2-byte, OLE-defined VARIANT_BOOL type (true = -1, false = 0).
                return ConstStrings.MarshalVariantBoolTypeName;
            }
            case UnmanagedType.FunctionPtr:
            {
                return ConstStrings.NativeMethodPointerTypeName;
            }
            // obsolete
            // case UnmanagedType.AsAny:
            // {

            // }
            case UnmanagedType.LPArray:
            {
                string eleTypeName;
                if (arrayElementType == MaxManagedType)
                {
                    eleTypeName = GlobalServices.Inst.TypeNameService.GetCppTypeNameAsFieldOrArgOrLoc(typeSig, TypeNameRelaxLevel.Exactly);
                }
                else
                {
                    if (typeSig.ElementType != ElementType.SZArray)
                    {
                        throw new NotSupportedException("Only SZArray is supported for array elements.");
                    }
                    eleTypeName = GetMarshalNativeTypeName(typeSig.Next, arrayElementType, MaxManagedType, InvalidElementCount, InvalidElementCount, charSet);
                }
                return $"{eleTypeName}*";
            }
            case UnmanagedType.LPStruct:
                return "void*";
            case UnmanagedType.CustomMarshaler:
            {
                return ConstStrings.MarshalCustomMarshalerTypeName;
            }
            case UnmanagedType.Error:
            {
                return ConstStrings.MarshalErrorTypeName;
            }
            case UnmanagedType.IInspectable:
            {
                return ConstStrings.MarshalIInspectableTypeName;
            }
            case UnmanagedType.HString:
            {
                return ConstStrings.MarshalHStringTypeName;
            }
            case UnmanagedType.LPUTF8Str:
            {
                return ConstStrings.MarshalLPUTF8StrTypeName;
            }
            default:
            {
                throw new NotSupportedException($"Unsupported UnmanagedType: {unmanagedType}");
            }
            }
        }


        public static MarshalAsInfo ParseMarshalInfo(CustomAttribute marshalAsAttribute)
        {
            var result = new MarshalAsInfo()
            {
                UnmanagedType = MaxManagedType,
                ArrayElementType = MaxManagedType,
                ParamIndex = InvalidElementCount,
                ElementCount = InvalidElementCount,
            };
            if (marshalAsAttribute.ConstructorArguments.Count != 1)
            {
                throw new NotSupportedException("MarshalAs attribute must have exactly one parameter.");
            }
            object managedTypeArg = marshalAsAttribute.ConstructorArguments[0].Value;
            if (managedTypeArg is int unmanagedType)
            {
                result.UnmanagedType = (UnmanagedType)unmanagedType;
            }
            else if (managedTypeArg is short shortUnmanagedType)
            {
                result.UnmanagedType = (UnmanagedType)shortUnmanagedType;
            }
            else
            {
                throw new NotSupportedException("MarshalAs attribute parameter must be an integer.");
            }

            foreach (CANamedArgument namedArgument in marshalAsAttribute.NamedArguments)
            {
                switch (namedArgument.Name)
                {
                case "MarshalCookie":
                {
                    break;
                }
                case "MarshalType":
                {
                    break;
                }
                case "MarshalTypeRef":
                {
                    break;
                }
                case "SafeArrayUserDefinedSubType":
                {
                    break;
                }
                case "ArraySubType":
                {
                    result.ArrayElementType = (UnmanagedType)namedArgument.Argument.Value;
                    break;
                }
                case "SafeArraySubType":
                {
                    throw new NotSupportedException("SafeArraySubType is not supported.");
                }
                case "SizeConst":
                {
                    result.ElementCount = (uint)namedArgument.Argument.Value;
                    break;
                }
                case "IidParameterIndex":
                {
                    throw new NotSupportedException("IidParameterIndex is not supported.");
                }
                case "SizeParamIndex":
                {
                    result.ParamIndex = (uint)namedArgument.Argument.Value;
                    break;
                }
                default:
                {
                    throw new NotSupportedException($"Unsupported MarshalAs attribute named argument: {namedArgument.Name}.");
                }
                }
            }
            return result;
        }

        public static MarshalAsInfo GetParameterOrFieldMarshalInfo(IHasCustomAttribute param)
        {
            if (param == null)
            {
                return null;
            }
            var marshalAsAttribute = param.CustomAttributes.FirstOrDefault(ca => ca.Constructor.DeclaringType.FullName == "System.Runtime.InteropServices.MarshalAsAttribute");
            if (marshalAsAttribute == null)
            {
                return null;
            }
            return ParseMarshalInfo(marshalAsAttribute);
        }

        public static CharSet GetMethodDefaultCharSet(MethodDef method)
        {
            if (!method.HasImplMap)
            {
                return CharSet.None;
            }

            ImplMap implMap = method.ImplMap;
            PInvokeAttributes cc = implMap.Attributes & PInvokeAttributes.CharSetMask;
            switch (cc)
            {
            case PInvokeAttributes.CharSetNotSpec:
                return CharSet.None;
            case PInvokeAttributes.CharSetAnsi:
                return CharSet.Ansi;
            case PInvokeAttributes.CharSetUnicode:
                return CharSet.Unicode;
            case PInvokeAttributes.CharSetAuto:
                return CharSet.Auto;
            default:
                throw new NotSupportedException($"Unsupported PInvoke attribute: {implMap.Attributes}");
            }
        }

        public static string GetPInvokeCallConvCppMacro(MethodDef method)
        {
            ImplMap implMap = method.ImplMap;
            if (implMap == null)
            {
                return "LEANCLR_PINVOKE_CALL_WINAPI";
            }
            PInvokeAttributes cc = implMap.Attributes & PInvokeAttributes.CallConvMask;
            switch (cc)
            {
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
                throw new NotSupportedException($"PInvoke calling convention mask 0x{(ushort)cc:X4} is not supported for {method.FullName}.");
            }
        }

        public static string GetMonoPInvokeCallbackCallingConventionCppMacro(MethodDef method)
        {
            var ca = method.CustomAttributes.FirstOrDefault(MetaUtil.IsMonoPInvokeCallbackAttribute);
            if (ca == null)
            {
                return "LEANCLR_PINVOKE_CALL_WINAPI";
            }
            TypeSig delegateType = ((TypeSig)ca.ConstructorArguments[0].Value).RemovePinnedAndModifiers();
            TypeDef delegateTypeDef = delegateType.ToTypeDefOrRef().ResolveTypeDefThrow();
            CustomAttribute unmanagedFunctionPointerAttribute = delegateTypeDef.CustomAttributes.FirstOrDefault(ca => ca.Constructor.DeclaringType.FullName == "System.Runtime.InteropServices.UnmanagedFunctionPointerAttribute");
            if (unmanagedFunctionPointerAttribute == null)
            {
                return "LEANCLR_PINVOKE_CALL_WINAPI";
            }
            CallingConvention callingConvention = (CallingConvention)unmanagedFunctionPointerAttribute.ConstructorArguments[0].Value;
            switch (callingConvention)
            {
            case CallingConvention.Winapi:
                return "LEANCLR_PINVOKE_CALL_WINAPI";
            case CallingConvention.Cdecl:
                return "LEANCLR_PINVOKE_CALL_CDECL";
            case CallingConvention.StdCall:
                return "LEANCLR_PINVOKE_CALL_STDCALL";
            case CallingConvention.ThisCall:
                return "LEANCLR_PINVOKE_CALL_THISCALL";
            case CallingConvention.FastCall:
                return "LEANCLR_PINVOKE_CALL_FASTCALL";
            default:
                throw new NotSupportedException($"Unsupported calling convention: {callingConvention}.");
            }
        }

        public static string GetPInvokeImportName(MethodDef method)
        {
            ImplMap implMap = method.ImplMap;
            if (implMap == null || implMap.Name == (UTF8String)null)
            {
                return method.Name;
            }
            string name = implMap.Name.String;
            return string.IsNullOrEmpty(name) ? method.Name : name;
        }

        public static string GetPInvokeModuleName(MethodDef method)
        {
            ImplMap implMap = method.ImplMap;
            if (implMap == null)
            {
                return string.Empty;
            }
            string dllName = implMap.Module.Name;
            return string.IsNullOrEmpty(dllName) ? ConstStrings.InternalDllName : dllName;
        }

        public static bool IsInternalDll(string dllNameNoExt)
        {
            return dllNameNoExt == ConstStrings.InternalDllName;
        }
    }
}