using dnlib.DotNet;
using LeanAOT.Core;
using System.Runtime.InteropServices;

namespace LeanAOT.ToCpp
{
    public class MarshalParamOrRet
    {
        private static readonly NLog.Logger s_logger = NLog.LogManager.GetCurrentClassLogger();

        public TypeSig Type { get; }

        public MarshalAsInfo MarshalAsInfo { get; }

        public CharSet CharSet { get; }

        public string ManagedVarName { get; }

        public string NativeVarName { get; }

        public bool IsReturn { get; }

        public string ManagedTypeName { get; }

        public string NativeTypeName { get; }

        public bool Supported { get; }

        public List<string> ManagedToNativeSetupCode { get; }
        // public string ManagedToNativeAssignCode { get; private set;}

        public List<string> ManagedToNativeCleanupCode { get; }

        public List<string> NativeToManagedSetupCode { get; }

        // public string NativeToManagedAssignCode { get; private set;}

        public List<string> NativeToManagedCleanupCode { get; }

        public MarshalParamOrRet(TypeSig type, MarshalAsInfo marshalAsInfo, CharSet charSet, bool isReturn, string managedVarName, string nativeVarName)
        {
            Type = type;
            MarshalAsInfo = marshalAsInfo;
            CharSet = charSet;
            IsReturn = isReturn;
            ManagedVarName = managedVarName;
            NativeVarName = nativeVarName;
            ManagedTypeName = MethodGenerationUtil.GetCppTypeNameAsFieldOrArgOrLoc(type, TypeNameRelaxLevel.Exactly);
            NativeTypeName = MarshalUtil.GetMarshalNativeTypeName(type, marshalAsInfo, charSet);
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
            var marshalAsInfo = MarshalAsInfo;
            if (marshalAsInfo == null)
            {
                throw new NotSupportedException("MarshalAsInfo is required for ByValArray.");
            }
            if (marshalAsInfo.ParamIndex != MarshalUtil.InvalidElementCount)
            {
                s_logger.Error($"ParamIndex is not supported for ByValArray. ParamIndex: {marshalAsInfo.ParamIndex}");
                return false;
            }
            ManagedToNativeSetupCode.Add($"{VmFunctionNames.MarshalManagedArrayToNativeValArray}({ManagedVarName}, {NativeVarName}, {marshalAsInfo.ElementCount});");
            ManagedToNativeCleanupCode.Add($"{VmFunctionNames.FreeNativeArray}({NativeVarName});");
            NativeToManagedSetupCode.Add($"{VmFunctionNames.DECLARING_ASSIGN_OR_THROW_WITHOUT_IP}({ManagedTypeName}, {ManagedVarName}, {VmFunctionNames.MarshalNativeValArrayToManagedArray}({NativeVarName}, {marshalAsInfo.ElementCount}, {Type}));");
            return true;
        }

        private bool SetupLPArray()
        {
            string lengthVarName = $"{NativeVarName}_length";
            ManagedToNativeSetupCode.Add($"size_t {lengthVarName} = {VmFunctionNames.StringGetLength}({ManagedVarName});");
            ManagedToNativeSetupCode.Add($"{NativeVarName} = {VmFunctionNames.MarshalManagedArrayToNativeArray}({ManagedVarName}, {lengthVarName});");
            ManagedToNativeCleanupCode.Add($"{VmFunctionNames.FreeNativeArray}({NativeVarName});");
            NativeToManagedSetupCode.Add($"{VmFunctionNames.DECLARING_ASSIGN_OR_THROW_WITHOUT_IP}({ManagedTypeName}, {ManagedVarName}, {VmFunctionNames.MarshalNativeArrayToManagedArray}({NativeVarName}, {lengthVarName}, {Type}));");
            return false;
        }

        private bool SetupFunctionPtr()
        {
            ManagedToNativeSetupCode.Add($"{VmFunctionNames.DECLARING_ASSIGN_OR_THROW_WITHOUT_IP}({NativeTypeName}, {NativeVarName}, {VmFunctionNames.MarshalDelegateToFnPtr}({ManagedVarName}));");
            NativeToManagedSetupCode.Add($"{VmFunctionNames.DECLARING_ASSIGN_OR_THROW_WITHOUT_IP}({NativeTypeName}, {NativeVarName}, {VmFunctionNames.MarshalDelegateToFnPtr}({ManagedVarName}));");
            return false;
        }

        private bool SetupLPStruct()
        {
            return false;
        }

        private bool InitCodes()
        {
            UnmanagedType unmanagedType = MarshalAsInfo?.UnmanagedType ?? MarshalUtil.MaxManagedType;
            TypeSig typeSig = Type;
            if (unmanagedType == MarshalUtil.MaxManagedType)
            {
                switch (typeSig.ElementType)
                {
                case ElementType.Void:
                {
                    return SetupVoid();
                }
                case ElementType.Boolean:
                {
                    unmanagedType = UnmanagedType.Bool;
                    break;
                }
                case ElementType.Char:
                {
                    return SetupChar();
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
                    return false;
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
            case UnmanagedType.I1:
            case UnmanagedType.U1:
            case UnmanagedType.I2:
            case UnmanagedType.U2:
            case UnmanagedType.I4:
            case UnmanagedType.U4:
            case UnmanagedType.I8:
            case UnmanagedType.U8:
            case UnmanagedType.R4:
            case UnmanagedType.R8:
            case UnmanagedType.SysInt:
            case UnmanagedType.SysUInt:
            {
                return SetupPrimitive();
            }
            // obsolete
            // case UnmanagedType.Currency:
            //     return "leanclr::metadata::RtMarshalCurrency";
            case UnmanagedType.BStr:
            {
                return SetupBStr();
            }
            case UnmanagedType.LPStr:
            {
                return SetupAnsiString();
            }
            case UnmanagedType.LPWStr:
            case UnmanagedType.LPTStr:
            {
                return SetupUtf16String();
            }
            case UnmanagedType.ByValTStr:
            {
                return SetupByValStr();
            }
            case UnmanagedType.IUnknown:
            {
                return false;
            }
            case UnmanagedType.IDispatch:
            {
                return false;
            }
            case UnmanagedType.Interface:
            {
                return false;
            }
            case UnmanagedType.Struct:
            {
                return SetupStruct();
            }
            case UnmanagedType.SafeArray:
            {
                return SetupSafeArray();
            }
            case UnmanagedType.ByValArray:
            {
                return SetupByValArray();
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
                return SetupPrimitive();
            }
            case UnmanagedType.FunctionPtr:
            {
                return SetupFunctionPtr();
            }
            // obsolete
            // case UnmanagedType.AsAny:
            // {

            // }
            case UnmanagedType.LPArray:
            {
                return SetupLPArray();
            }
            case UnmanagedType.LPStruct:
            {
                return SetupLPStruct();
            }
            case UnmanagedType.CustomMarshaler:
            {
                return false;
            }
            case UnmanagedType.Error:
            {
                return false;
            }
            case UnmanagedType.IInspectable:
            {
                return false;
            }
            case UnmanagedType.HString:
            {
                return SetupHString();
            }
            case UnmanagedType.LPUTF8Str:
            {
                return SetupUtf8String();
            }
            default:
            {
                throw new NotSupportedException($"Unsupported UnmanagedType: {unmanagedType}");
            }
            }
        }
    }
}