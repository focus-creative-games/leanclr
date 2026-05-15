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
        public static string GetCharTypeName(CharSet charSet)
        {
            return charSet != CharSet.Ansi ? ConstStrings.MarshalUtf16CharTypeName : ConstStrings.MarshalAnsiCharTypeName;
        }

        public static string GetCharPtrTypeName(CharSet charSet)
        {
            return charSet != CharSet.Ansi ? ConstStrings.MarshalUTF16StrTypeName : ConstStrings.MarshalAnsiStrTypeName;
        }

        public static bool IsSafeHandleType(TypeSig typeSig)
        {
            TypeDef typeDef = typeSig.ToTypeDefOrRef().ResolveTypeDef();
            if (typeDef == null)
            {
                return false;
            }
            return MetaUtil.IsInheritFrom(typeDef, "System.Runtime.InteropServices.SafeHandle") || MetaUtil.IsInheritFrom(typeDef, "System.Runtime.InteropServices.CriticalHandle");
        }
        public static string GetMarshalNativeTypeName(TypeSig typeSig, MarshalType marshalType, CharSet charSet)
        {
            if (typeSig.IsByRef)
            {
                return GetMarshalNativeTypeName(typeSig.Next, marshalType, charSet) + "*";
            }
            if (marshalType == null)
            {
                switch (typeSig.ElementType)
                {
                case ElementType.Void: return "void";
                case ElementType.Boolean:
                {
                    marshalType = new MarshalType(NativeType.Boolean);
                    break;
                }
                case ElementType.Char:
                {
                    return GetCharTypeName(charSet);
                }
                case ElementType.I1:
                {
                    marshalType = new MarshalType(NativeType.I1);
                    break;
                }
                case ElementType.U1:
                {
                    marshalType = new MarshalType(NativeType.U1);
                    break;
                }
                case ElementType.I2:
                {
                    marshalType = new MarshalType(NativeType.I2);
                    break;
                }
                case ElementType.U2:
                {
                    marshalType = new MarshalType(NativeType.U2);
                    break;
                }
                case ElementType.I4:
                {
                    marshalType = new MarshalType(NativeType.I4);
                    break;
                }
                case ElementType.U4:
                {
                    marshalType = new MarshalType(NativeType.U4);
                    break;
                }
                case ElementType.I8:
                {
                    marshalType = new MarshalType(NativeType.I8);
                    break;
                }
                case ElementType.U8:
                {
                    marshalType = new MarshalType(NativeType.U8);
                    break;
                }
                case ElementType.R4:
                {
                    marshalType = new MarshalType(NativeType.R4);
                    break;
                }
                case ElementType.R8:
                {
                    marshalType = new MarshalType(NativeType.R8);
                    break;
                }
                case ElementType.I:
                {
                    marshalType = new MarshalType(NativeType.Int);
                    break;
                }
                case ElementType.U:
                {
                    marshalType = new MarshalType(NativeType.UInt);
                    break;
                }
                case ElementType.String:
                {
                    marshalType = new MarshalType(NativeType.LPStr);
                    break;
                }
                case ElementType.Object:
                {
                    marshalType = new InterfaceMarshalType(NativeType.IUnknown);
                    break;
                }
                case ElementType.Class:
                {
                    if (MetaUtil.IsDerivedFromMulticastDelegate(typeSig.ToTypeDefOrRef()))
                    {
                        marshalType = new MarshalType(NativeType.Func);
                    }
                    else if (IsSafeHandleType(typeSig))
                    {
                        return ConstStrings.MarshalHandleTypeName;
                    }
                    marshalType = new InterfaceMarshalType(NativeType.IUnknown);
                    break;
                }
                case ElementType.ValueType:
                {
                    marshalType = new MarshalType(NativeType.Struct);
                    break;
                }
                case ElementType.GenericInst:
                {
                    throw new NotSupportedException("GenericInst is not supported for marshal.");
                }
                case ElementType.SZArray:
                case ElementType.Array:
                {
                    marshalType = new ArrayMarshalType(NativeType.Array);
                    break;
                }
                case ElementType.Ptr:
                {
                    marshalType = new MarshalType(NativeType.Int);
                    break;
                }
                case ElementType.ByRef:
                {
                    return $"{GetMarshalNativeTypeName(typeSig.Next, null, charSet)}*";
                }
                case ElementType.FnPtr:
                {
                    marshalType = new MarshalType(NativeType.Func);
                    break;
                }
                default:
                {
                    throw new NotSupportedException($"Unsupported ElementType: {typeSig.ElementType}");
                }
                }
            }
            switch (marshalType.NativeType)
            {
            case NativeType.Boolean:
                return ConstStrings.MarshalBooleanTypeName;
            case NativeType.I1:
                return "int8_t";
            case NativeType.U1:
                return "uint8_t";
            case NativeType.I2:
                return "int16_t";
            case NativeType.U2:
                return "uint16_t";
            case NativeType.I4:
                return "int32_t";
            case NativeType.U4:
                return "uint32_t";
            case NativeType.I8:
                return "int64_t";
            case NativeType.U8:
                return "uint64_t";
            case NativeType.R4:
                return ConstStrings.Float32TypeName;
            case NativeType.R8:
                return ConstStrings.Float64TypeName;
            case NativeType.Int:
                return "intptr_t";
            case NativeType.UInt:
                return "uintptr_t";
            // obsolete
            // case NativeType.Currency:
            //     return "leanclr::metadata::RtMarshalCurrency";
            case NativeType.BStr:
            {
                return ConstStrings.MarshalUTF16StrTypeName;
            }
            case NativeType.LPStr:
            {
                return ConstStrings.MarshalAnsiStrTypeName;
            }
            case NativeType.LPWStr:
            case NativeType.LPTStr:
            {
                return ConstStrings.MarshalUTF16StrTypeName;
            }
            case NativeType.FixedSysString:
            {
                var actualMarshalType = (FixedSysStringMarshalType)marshalType;
                if (!actualMarshalType.IsSizeValid)
                {
                    throw new NotSupportedException("FixedSysString must have a fixed length.");
                }
                return $"{GetCharTypeName(charSet)}[{actualMarshalType.Size}]";
            }
            case NativeType.IUnknown:
            {
                return ConstStrings.MarshalIUnknownTypeName;
            }
            case NativeType.IDispatch:
            {
                return ConstStrings.MarshalIDispatchTypeName;
            }
            case NativeType.IntF:
            {
                return ConstStrings.MarshalInterfaceTypeName;
            }
            case NativeType.Struct:
            {
                return GlobalServices.Inst.TypeNameService.GetCppTypeNameAsFieldOrArgOrLoc(typeSig, TypeNameRelaxLevel.Exactly);
            }
            case NativeType.SafeArray:
            {
                return ConstStrings.MarshalSafeArrayPtrTypeName;
            }
            case NativeType.FixedArray:
            {
                var actualMarshalType = (FixedArrayMarshalType)marshalType;
                if (!actualMarshalType.IsSizeValid)
                {
                    throw new NotSupportedException("FixedArray must have a fixed length.");
                }
                return $"{GetMarshalNativeTypeName(typeSig.Next, null, charSet)}[{actualMarshalType.Size}]";
            }
            // obsolete
            // case NativeType.VBByRefStr:
            // case NativeType.AnsiBStr:
            // case NativeType.TBStr:
            // {
            //     break;
            // }
            case NativeType.VariantBool:
            {
                // A 2-byte, OLE-defined VARIANT_BOOL type (true = -1, false = 0).
                return ConstStrings.MarshalVariantBoolTypeName;
            }
            case NativeType.Func:
            {
                return ConstStrings.NativeMethodPointerTypeName;
            }
            // obsolete
            // case NativeType.AsAny:
            // {

            // }
            case NativeType.Array:
            {
                if (typeSig.ElementType != ElementType.SZArray)
                {
                    throw new NotSupportedException("Only SZArray is supported for array elements.");
                }
                var actualMarshalType = (ArrayMarshalType)marshalType;
                string eleTypeName;
                if (actualMarshalType.IsElementTypeValid)
                {
                    eleTypeName = GetMarshalNativeTypeName(typeSig.Next, null, charSet);
                }
                else
                {
                    try
                    {
                        eleTypeName = GetMarshalNativeTypeName(typeSig.Next, new MarshalType(actualMarshalType.ElementType), charSet);
                    }
                    catch (Exception e)
                    {
                        throw new NotSupportedException($"Unsupported GetMarshalNativeTypeName for array type: {typeSig}, marshal type: {marshalType}.", e);
                    }
                }
                return $"{eleTypeName}*";
            }
            case NativeType.LPStruct:
                return "void*";
            case NativeType.CustomMarshaler:
            {
                return ConstStrings.MarshalCustomMarshalerTypeName;
            }
            case NativeType.Error:
            {
                return ConstStrings.MarshalErrorTypeName;
            }
            case NativeType.IInspectable:
            {
                return ConstStrings.MarshalIInspectableTypeName;
            }
            case NativeType.HString:
            {
                return ConstStrings.MarshalHStringTypeName;
            }
            case NativeType.LPUTF8Str:
            {
                return ConstStrings.MarshalUTF8StrTypeName;
            }
            default:
            {
                throw new NotSupportedException($"Unsupported NativeType: {marshalType.NativeType}");
            }
            }
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