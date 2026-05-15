using dnlib.DotNet;
using LeanAOT.Core;
using System.Runtime.InteropServices;

namespace LeanAOT.ToCpp
{
    public class MarshalParamOrRet
    {
        private static readonly NLog.Logger s_logger = NLog.LogManager.GetCurrentClassLogger();

        public int ParamIndex { get; }

        public TypeSig Type { get; }

        public MarshalType MarshalType { get; }

        public CharSet CharSet { get; }

        public string ManagedVarName { get; }

        public string NativeVarName { get; }

        public string MethodInfoVarName { get; }

        public bool IsReturn { get; }

        public string ManagedTypeName { get; }

        public string NativeTypeName { get; }

        public bool MarshalManagedToNative { get; }

        public bool MarshalNativeToManaged { get; }

        public bool Supported { get; }

        public List<string> ManagedToNativeSetupCode { get; }
        // public string ManagedToNativeAssignCode { get; private set;}

        public List<string> ManagedToNativeCleanupCode { get; }

        public List<string> NativeToManagedSetupCode { get; }

        // public string NativeToManagedAssignCode { get; private set;}

        public List<string> NativeToManagedCleanupCode { get; }

        public MarshalParamOrRet(int paramIndex, TypeSig type, MarshalType marshalType, CharSet charSet, bool isReturn, string managedVarName, string nativeVarName, bool marshalManagedToNative, string methodInfoVarName)
        {
            ParamIndex = paramIndex;
            Type = type;
            MarshalType = marshalType;
            CharSet = charSet;
            IsReturn = isReturn;
            ManagedVarName = managedVarName;
            NativeVarName = nativeVarName;
            MarshalManagedToNative = marshalManagedToNative;
            MarshalNativeToManaged = !marshalManagedToNative;
            MethodInfoVarName = methodInfoVarName;
            ManagedTypeName = MethodGenerationUtil.GetCppTypeNameAsFieldOrArgOrLoc(type, TypeNameRelaxLevel.Exactly);
            NativeTypeName = MarshalUtil.GetMarshalNativeTypeName(type, marshalType, charSet);
            ManagedToNativeSetupCode = new List<string>();
            // ManagedToNativeAssignCode = string.Empty;
            ManagedToNativeCleanupCode = new List<string>();
            NativeToManagedSetupCode = new List<string>();
            // NativeToManagedAssignCode = string.Empty;
            NativeToManagedCleanupCode = new List<string>();
            Supported = InitCodes();
        }


        private bool SetupVoid()
        {
            return false;
        }

        private bool SetupChar()
        {
            string marshalFunctionName = this.CharSet == CharSet.Ansi ? VmFunctionNames.MarshalManagedCharToAnsiChar : VmFunctionNames.MarshalManagedCharToUtf8Char;
            ManagedToNativeSetupCode.Add($"{NativeTypeName} {NativeVarName} = {marshalFunctionName}({ManagedVarName});");
            string unmarshalFunctionName = this.CharSet == CharSet.Ansi ? VmFunctionNames.MarshalAnsiCStrToManagedChar : VmFunctionNames.MarshalUtf8CStrToManagedChar;
            NativeToManagedSetupCode.Add($"{ManagedTypeName} {ManagedVarName} = {unmarshalFunctionName}({NativeVarName});");
            return true;
        }

        private bool SetupPrimitive()
        {
            ManagedToNativeSetupCode.Add($"{NativeTypeName} {NativeVarName} = ({NativeTypeName}){ManagedVarName};");
            NativeToManagedSetupCode.Add($"{ManagedTypeName} {ManagedVarName} = ({ManagedTypeName}){NativeVarName};");
            return true;
        }

        private bool SetupBStr()
        {
            return false;
        }

        private string GetStringBuilderVarName()
        {
            return $"__string_builder_{NativeVarName}";
        }

        private bool SetupAnsiString()
        {
            string stringBuilderVarName = GetStringBuilderVarName();
            ManagedToNativeSetupCode.Add($"{ConstStrings.StringBuilderTypeName} {stringBuilderVarName};");
            string managed2NativeArgs = IsReturn ? ManagedVarName : $"{ManagedVarName}, {stringBuilderVarName}";
            ManagedToNativeSetupCode.Add($"{NativeTypeName} {NativeVarName} = {VmFunctionNames.MarshalManagedStringToAnsiString}({managed2NativeArgs});");
            NativeToManagedSetupCode.Add($"{ManagedTypeName} {ManagedVarName} = {VmFunctionNames.MarshalAnsiStringToManagedString}({NativeVarName});");
            return true;
        }

        private bool SetupUtf16String()
        {
            ManagedToNativeSetupCode.Add($"{NativeTypeName} {NativeVarName} = {VmFunctionNames.MarshalManagedStringToUtf16String}({ManagedVarName});");
            NativeToManagedSetupCode.Add($"{ManagedTypeName} {ManagedVarName} = {VmFunctionNames.MarshalUtf16StringToManagedString}({NativeVarName});");
            return true;
        }

        private bool SetupUtf8String()
        {
            string stringBuilderVarName = GetStringBuilderVarName();
            ManagedToNativeSetupCode.Add($"{ConstStrings.StringBuilderTypeName} {stringBuilderVarName};");
            string managed2NativeArgs = IsReturn ? ManagedVarName : $"{ManagedVarName}, {stringBuilderVarName}";
            ManagedToNativeSetupCode.Add($"{NativeTypeName} {NativeVarName} = {VmFunctionNames.MarshalManagedStringToUtf8String}({managed2NativeArgs});");
            NativeToManagedSetupCode.Add($"{ManagedTypeName} {ManagedVarName} = {VmFunctionNames.MarshalUtf8StringToManagedString}({NativeVarName});");
            return true;
        }

        private bool SetupHString()
        {
            return SetupUtf16String();
        }

        private bool SetupByValStr()
        {
            return false;
        }

        private bool SetupStruct()
        {
            // First, convert the managed struct to a native struct.
            ManagedToNativeSetupCode.Add($"{NativeTypeName} {NativeVarName} = *({NativeTypeName}*)(&{ManagedVarName});");
            NativeToManagedSetupCode.Add($"{ManagedTypeName} {ManagedVarName} = *({ManagedTypeName}*)(&{NativeVarName});");

            return true;
        }

        private bool SetupSafeArray()
        {
            return false;
        }

        private bool SetupByValArray()
        {
            var marshalAsInfo = (FixedArrayMarshalType)MarshalType;
            ManagedToNativeSetupCode.Add($"{VmFunctionNames.MarshalManagedArrayToNativeValArray}({ManagedVarName}, {NativeVarName}, {marshalAsInfo.Size});");
            ManagedToNativeCleanupCode.Add($"{VmFunctionNames.FreeNativeArray}({NativeVarName});");
            NativeToManagedSetupCode.Add($"{VmFunctionNames.DECLARING_ASSIGN_OR_ABORT_ON_ERROR}({ManagedTypeName}, {ManagedVarName}, {VmFunctionNames.MarshalNativeValArrayToManagedArray}({NativeVarName}, {marshalAsInfo.Size}, {Type}), {MethodInfoVarName});");
            return true;
        }

        private bool SetupLPArray()
        {
            if (MarshalNativeToManaged)
            {
                return false;
            }
            string lengthVarName = $"{NativeVarName}_length";
            ManagedToNativeSetupCode.Add($"size_t {lengthVarName} = static_cast<size_t>({VmFunctionNames.GetArrayLength}({ManagedVarName}));");
            ManagedToNativeSetupCode.Add($"{NativeTypeName} {NativeVarName} = ({NativeTypeName}){VmFunctionNames.MarshalManagedArrayToNativeArray}({ManagedVarName}, {lengthVarName});");
            ManagedToNativeCleanupCode.Add($"{VmFunctionNames.FreeNativeArray}({NativeVarName});");
            // NativeToManagedSetupCode.Add($"{VmFunctionNames.DECLARING_ASSIGN_OR_THROW_WITHOUT_IP}({ManagedTypeName}, {ManagedVarName}, {VmFunctionNames.MarshalNativeArrayToManagedArray}({NativeVarName}, {lengthVarName}, {Type}));");
            return true;
        }

        private string GetParamOrRetTypeSig()
        {
            return IsReturn ? $"{MethodInfoVarName}->return_type" : $"{MethodInfoVarName}->parameters[{ParamIndex}]";
        }

        private bool SetupFunctionPtr()
        {
            ManagedToNativeSetupCode.Add($"{VmFunctionNames.DECLARING_ASSIGN_OR_THROW_WITHOUT_IP}({NativeTypeName}, {NativeVarName}, {VmFunctionNames.MarshalDelegateToFnPtr}(({ConstStrings.DelegatePtrTypeName}){ManagedVarName}), {MethodInfoVarName});");
            string paramOrRetTypeSig = GetParamOrRetTypeSig();
            NativeToManagedSetupCode.Add($"{VmFunctionNames.DECLARING_ASSIGN_OR_ABORT_ON_ERROR}({ManagedTypeName}, {ManagedVarName}, {VmFunctionNames.MarshalFnPtrToDelegate}({NativeVarName}, {paramOrRetTypeSig}), {MethodInfoVarName});");
            return true;
        }

        private bool SetupLPStruct()
        {
            return false;
        }

        private bool SetupSafeHandle()
        {
            ManagedToNativeSetupCode.Add($"{NativeTypeName} {NativeVarName} = {VmFunctionNames.MarshalSafeHandleToHandle}(({ConstStrings.ObjectPtrTypeName}){ManagedVarName});");
            string paramOrRetTypeSig = GetParamOrRetTypeSig();
            NativeToManagedSetupCode.Add($"{VmFunctionNames.DECLARING_ASSIGN_OR_ABORT_ON_ERROR}({ManagedTypeName}, {ManagedVarName}, {VmFunctionNames.MarshalHandleToSafeHandle}({NativeVarName}, {paramOrRetTypeSig}), {MethodInfoVarName});");
            return true;
        }

        private bool InitCodes()
        {
            MarshalType marshalType = MarshalType;
            TypeSig typeSig = Type;
            if (marshalType == null)
            {
                switch (typeSig.ElementType)
                {
                case ElementType.Void:
                {
                    return SetupVoid();
                }
                case ElementType.Boolean:
                {
                    marshalType = new MarshalType(NativeType.Boolean);
                    break;
                }
                case ElementType.Char:
                {
                    return SetupChar();
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
                    else if (MarshalUtil.IsSafeHandleType(typeSig))
                    {
                        return SetupSafeHandle();
                    }
                    else
                    {
                        marshalType = new InterfaceMarshalType(NativeType.IUnknown);
                    }
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
                    // TODO:
                    marshalType = new MarshalType(NativeType.Int);
                    break;
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
            case NativeType.I1:
            case NativeType.U1:
            case NativeType.I2:
            case NativeType.U2:
            case NativeType.I4:
            case NativeType.U4:
            case NativeType.I8:
            case NativeType.U8:
            case NativeType.R4:
            case NativeType.R8:
            case NativeType.Int:
            case NativeType.UInt:
            {
                return SetupPrimitive();
            }
            // obsolete
            // case NativeType.Currency:
            //     return "leanclr::metadata::RtMarshalCurrency";
            case NativeType.BStr:
            {
                return SetupBStr();
            }
            case NativeType.LPStr:
            {
                return SetupAnsiString();
            }
            case NativeType.LPWStr:
            case NativeType.LPTStr:
            {
                return SetupUtf16String();
            }
            case NativeType.FixedSysString:
            {
                return SetupByValStr();
            }
            case NativeType.IUnknown:
            {
                return false;
            }
            case NativeType.IDispatch:
            {
                return false;
            }
            case NativeType.IntF:
            {
                return false;
            }
            case NativeType.Struct:
            {
                return SetupStruct();
            }
            case NativeType.SafeArray:
            {
                return SetupSafeArray();
            }
            case NativeType.FixedArray:
            {
                return SetupByValArray();
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
                return SetupPrimitive();
            }
            case NativeType.Func:
            {
                return SetupFunctionPtr();
            }
            // obsolete
            // case NativeType.AsAny:
            // {

            // }
            case NativeType.Array:
            {
                return SetupLPArray();
            }
            case NativeType.LPStruct:
            {
                return SetupLPStruct();
            }
            case NativeType.CustomMarshaler:
            {
                return false;
            }
            case NativeType.Error:
            {
                return false;
            }
            case NativeType.IInspectable:
            {
                return false;
            }
            case NativeType.HString:
            {
                return SetupHString();
            }
            case NativeType.LPUTF8Str:
            {
                return SetupUtf8String();
            }
            default:
            {
                throw new NotSupportedException($"Unsupported marshal type: {marshalType.NativeType} {marshalType}");
            }
            }
        }
    }
}