
using dnlib.DotNet;
using dnlib.DotNet.Emit;
using LeanAOT.Core;
using System.Diagnostics;
using System.Text;

namespace LeanAOT.ToCpp
{
    class ParameterVariable
    {
        private readonly ParamDetail _parameter;
        public readonly EvalVariable _evalVariable;
        public ParameterVariable(ParamDetail parameter, EvalVariable evalVariable)
        {
            this._parameter = parameter;
            this._evalVariable = evalVariable;
        }

        public TypeSig Type => _parameter.Type;

        public int Index => _parameter.Index;

        public string Name => _parameter.Name;
    }

    class LocalVariable
    {
        private readonly Local _local;
        private readonly TypeSig _typeSig;
        public readonly EvalVariable _evalVariable;
        public LocalVariable(Local local, TypeSig inflatedType, EvalVariable evalVariable)
        {
            this._local = local;
            this._typeSig = inflatedType;
            this._evalVariable = evalVariable;
        }

        public TypeSig Type => _typeSig;
    }

    class TempVariable
    {
        private readonly string _name;
        private readonly string _typeName;
        public TempVariable(string name, string typeName)
        {
            _name = name;
            _typeName = typeName;
        }

        public string Name => _name;

        public string TypeName => _typeName;
    }


    class EvalStackState
    {
        public bool visited;
        public bool inputStackSetup;

        public readonly List<EvalVariable> inputStackDatas = new List<EvalVariable>();
        public readonly List<EvalVariable> runStackDatas = new List<EvalVariable>();
    }

    partial class MethodWriterBase
    {
        //private readonly IMethod _method;
        private readonly MethodDetail _method;
        private readonly ICorLibTypes _corlibTypes;
        private readonly MetadataService _metadataService;
        private readonly ManifestService _manifestService;

        private readonly IMethodBodyCodeFilePart _methodBodyCodeFile;
        private readonly ForwardDeclaration _forwardDeclaration;
        private readonly CodeThunkZone _writer;


        private readonly CodeThrunkWriter _beforeBodyWriter;
        private readonly CodeThrunkWriter _headWriter;
        private readonly CodeThrunkWriter _localsWriter;
        private readonly CodeThrunkWriter _bodyWriter;
        private readonly CodeThrunkWriter _tailWriter;

        private List<ParameterVariable> _parameterVariables;
        private List<LocalVariable> _localVariables;

        private int _nextTempVarId = 1;


        private Dictionary<BasicBlock, EvalStackState> _blockEvalStackStates;


        private readonly RuntimeResolvedMetadatas _runtimeResolvedMetadatas;
        private dnlib.IO.DataReader _rawILReader;
        private uint _codeSize;
        private uint _codeStartPosition;


        private RuntimeResolvedVariable __curMethodVar;
        private RuntimeResolvedVariable CurMethodVar
        {
            get
            {
                if (__curMethodVar == null)
                {
                    __curMethodVar = _runtimeResolvedMetadatas.GetMethodVariable(_method.MethodDef);
                }
                return __curMethodVar;
            }
        }

        public MethodWriterBase(MethodDetail method, IMethodBodyCodeFilePart methodBodyCodeFile)
        {
            _method = method;
            _corlibTypes = method.ModuleOfMethodDef.CorLibTypes;
            _metadataService = GlobalServices.Inst.MetadataService;
            _manifestService = GlobalServices.Inst.ManifestService;
            _methodBodyCodeFile = methodBodyCodeFile;
            _forwardDeclaration = methodBodyCodeFile.ForwardDeclaration;
            _writer = _methodBodyCodeFile.MethodWriter.CreateThunkCollection(_method.UniqueName);


            _beforeBodyWriter = _writer.CreateThunk("before_method_body");
            _beforeBodyWriter.AddLine();

            _headWriter = _writer.CreateThunk("method_header");
            _localsWriter = _writer.CreateThunk("local_vars");
            _bodyWriter = _writer.CreateThunk("method_body");
            _tailWriter = _writer.CreateThunk("method_tail");


            _runtimeResolvedMetadatas = new RuntimeResolvedMetadatas(_method);
            InitMethodVariables();
            InitRawILReader();
            //__curMethodVar = _runtimeResolvedMetadatas.GetMethodVariable(method.MethodDef);
        }

        void InitMethodVariables()
        {
            _parameterVariables = _method.ParamsIncludeThis.Select(p => new ParameterVariable(p, CreateEvalVariableFromTypeSig(p.Type))).ToList();
            _localVariables = _method.MethodDef.Body.Variables.Select(l =>
            {
                var inflatedType = _method.InflateType(l.Type);
                return new LocalVariable(l, inflatedType, CreateEvalVariableFromTypeSig(inflatedType));
            }).ToList();
        }

        void InitRawILReader()
        {
            ModuleDefMD mod = (ModuleDefMD)_method.Module;
            var peImage = mod.Metadata.PEImage;
            var offset = peImage.ToFileOffset(_method.MethodDef.RVA);
            _rawILReader = peImage.CreateReader(offset);
            if (!ReadHeader())
            {
                throw new Exception($"Failed to read method header for method: {_method.MethodDef.FullName}");
            }
        }

        bool ReadHeader()
        {
            var reader = _rawILReader;
            var startOfHeader = reader.Position;
            byte b = reader.ReadByte();
            switch (b & 7)
            {
            case 2:
            case 6:
                // Tiny header. [7:2] = code size, max stack is 8, no locals or exception handlers
                //flags = 2;
                //maxStack = 8;
                _codeSize = (uint)(b >> 2);
                //localVarSigTok = 0;
                //headerSize = 1;
                break;

            case 3:
                // Fat header. Can have locals and exception handlers
                var flags = (ushort)((reader.ReadByte() << 8) | b);
                var headerSize = (byte)(flags >> 12);
                var maxStack = reader.ReadUInt16();
                _codeSize = reader.ReadUInt32();
                var localVarSigTok = reader.ReadUInt32();

                // The CLR allows the code to start inside the method header. But if it does,
                // the CLR doesn't read any exceptions.
                reader.Position = reader.Position - 12 + headerSize * 4U;
                if (headerSize < 3)
                    flags &= 0xFFF7;
                headerSize *= 4;
                break;

            default:
                return false;
            }

            if ((ulong)reader.Position + _codeSize > reader.Length)
                return false;
            _codeStartPosition = reader.Position;
            return true;
        }

        private void AddParameterAndReturnTypeAndLocalForwardDeclarations()
        {
            foreach (var param in _parameterVariables)
            {
                _forwardDeclaration.AddTypeForwardDefine(param.Type);
            }
            _forwardDeclaration.AddTypeForwardDefine(_method.RetType);
            foreach (var local in _localVariables)
            {
                _forwardDeclaration.AddTypeForwardDefine(local.Type);
            }
        }

        public void WriteCode()
        {
            AddParameterAndReturnTypeAndLocalForwardDeclarations();
            WriteMethodHeader();
            WriteLocalDeclarations(InitLocals);
            WriteMethodBody();
            WriteMethodEnd();

            WriteDeferredCode();
            _writer.MarkAsArchived();
        }

        void WriteMethodHeader()
        {
            _headWriter.AddLine($"// Method: {_method.FullName}");
            _headWriter.AddLine(_method.GenerateMethodDeclaring());
            _headWriter.AddLine("{");
        }

        private void WriteLocalDeclarations(bool initLocals)
        {
            _localsWriter.SetIndent(1);
            _localsWriter.AddLine("// Local Variables");
            foreach (var local in _localVariables)
            {
                _forwardDeclaration.AddTypeForwardDefine(local.Type);
                _localsWriter.AddLine($"{GetExactTypeName(local.Type)} {GetLocalName(local)}{(initLocals ? " = {}" : "")};");
            }
        }

        void WriteMethodEnd()
        {
            _tailWriter.AddLine("}");
        }

        void WriteDeferredCode()
        {
            if (!_runtimeResolvedMetadatas.IsEmpty())
            {
                _beforeBodyWriter.SetIndent(0);
                _beforeBodyWriter.AddLine($"struct {_runtimeResolvedMetadatas.GetResolveMetadataStructName()}");
                _beforeBodyWriter.AddLine("{");
                _beforeBodyWriter.IncreaseIndent();
                foreach (var resolvedVar in _runtimeResolvedMetadatas.ResolvedVariables)
                {
                    _beforeBodyWriter.AddLine($"// Token: 0x{resolvedVar.token:X8}");
                    _beforeBodyWriter.AddLine($"{resolvedVar.GetVariableTypeName()} {resolvedVar.GetVariableName()};");
                }
                _beforeBodyWriter.DecreaseIndent();
                _beforeBodyWriter.AddLine("};");

                _headWriter.SetIndent(1);
                _headWriter.AddLine($"static {_runtimeResolvedMetadatas.GetResolveMetadataStructName()} {_runtimeResolvedMetadatas.GetResolveMetadatasVariableName()};");
                _headWriter.AddLine($"static {_runtimeResolvedMetadatas.GetResolveMetadataStructName()}* {_runtimeResolvedMetadatas.GetResolveMetadatasPtrVariableName()} = nullptr;");
                _headWriter.AddLine($"if ({_runtimeResolvedMetadatas.GetResolveMetadatasPtrVariableName()} == nullptr)");
                _headWriter.AddLine("{");
                _headWriter.IncreaseIndent();
                foreach (var resolvedVar in _runtimeResolvedMetadatas.ResolvedVariables)
                {
                    _headWriter.AddLine($"// Token: 0x{resolvedVar.token:X8}");
                    _headWriter.AddLine($"{resolvedVar.GetFullReferenceVariableNameBeforeInit()} = {resolvedVar.ResolveCode()};");
                }
                _headWriter.AddLine($"{_runtimeResolvedMetadatas.GetResolveMetadatasPtrVariableName()} = &{_runtimeResolvedMetadatas.GetResolveMetadatasVariableName()};");
                _headWriter.DecreaseIndent();
                _headWriter.AddLine("}");
            }
        }

        private bool InitLocals => _method.MethodDef.Body.InitLocals;

        private TypeNameService TypeNameService => GlobalServices.Inst.TypeNameService;

        string GetExactTypeName(TypeSig typeSig, bool allowStringAndTypedByRef = false)
        {
            return TypeNameService.GetCppTypeNameAsFieldOrArgOrLoc(typeSig, allowStringAndTypedByRef ? TypeNameRelaxLevel.ExactlyIncludeStringAndTypedByRef : TypeNameRelaxLevel.Exactly);
        }

        string GetExactTypeName(ITypeDefOrRef type, bool allowStringAndTypedByRef = false)
        {
            return GetExactTypeName(type.ToTypeSig(), allowStringAndTypedByRef);
        }

        string GetLocalName(LocalVariable local)
        {
            return $"__L{local._evalVariable.varId}";
        }

        string GetParameterName(ParameterVariable param)
        {
            return param.Name;
        }

        private string GetBasicBlockStartLabelName(BasicBlock bb)
        {
            return $"__IL_{bb.StartOffset}";
        }

        private void EmitBasicBlockLabel(BasicBlock bb)
        {
            _bodyWriter.AddLine($"{GetBasicBlockStartLabelName(bb)}:");
        }

        private string GetEvalVariableName(EvalVariable var)
        {
            return $"__{(var.inOutVar ? "inout_" : "e")}{var.varId}";
        }

        private string GetEvalVariableExpr(EvalVariable var, bool isUnsigned)
        {
            if (!isUnsigned)
                return GetEvalVariableName(var);
            return $"({GetUnsignedTypeName(var)}){GetEvalVariableName(var)}";
        }

        private string GetEvalVariableExprWithCast(EvalVariable var, string castToTypeName)
        {
            string typeName = GetTypeName(var);
            if (typeName == castToTypeName)
                return GetEvalVariableName(var);
            else
                return $"({castToTypeName}){GetEvalVariableName(var)}";
        }

        private string GetTypeName(EvalVariable evalVar)
        {
            switch (evalVar.type)
            {
            case EvalDataType.Int32:
                return "int32_t";
            case EvalDataType.Int64:
                return "int64_t";
            case EvalDataType.Float:
                return ConstStrings.Float32TypeName;
            case EvalDataType.Double:
                return ConstStrings.Float64TypeName;
            case EvalDataType.I:
                return "intptr_t";
            case EvalDataType.Ref:
                return ConstStrings.ObjectPtrTypeName;
            case EvalDataType.ValueType:
                return GetExactTypeName(evalVar.typeSig);
            default:
                throw new Exception($"Unsupported EvalDataType: {evalVar.type} in method: {_method.MethodDef.FullName}.");
            }
        }

        private string GetTypeName(EvalVariable evalVar, bool castToUnsigned)
        {
            if (!castToUnsigned)
                return GetTypeName(evalVar);
            return GetUnsignedTypeName(evalVar);
        }

        private string GetPrimitiveTypeName(ElementType type)
        {
            switch (type)
            {
            case ElementType.I1: return "int8_t";
            case ElementType.U1: return "uint8_t";
            case ElementType.I2: return "int16_t";
            case ElementType.U2: return "uint16_t";
            case ElementType.I4: return "int32_t";
            case ElementType.U4: return "uint32_t";
            case ElementType.I8: return "int64_t";
            case ElementType.U8: return "uint64_t";
            case ElementType.R4: return ConstStrings.Float32TypeName;
            case ElementType.R8: return ConstStrings.Float64TypeName;
            case ElementType.R: return ConstStrings.Float64TypeName;
            case ElementType.I: return "intptr_t";
            case ElementType.U: return "uintptr_t";
            case ElementType.Object: return ConstStrings.ObjectPtrTypeName;
            default:
                throw new Exception($"Unsupported primitive ElementType: {type} in method: {_method.MethodDef.FullName}.");
            }
        }

        private string GetReduceTypeName(ReduceDataType type)
        {
            switch (type)
            {
            case ReduceDataType.I1: return "int8_t";
            case ReduceDataType.U1: return "uint8_t";
            case ReduceDataType.I2: return "int16_t";
            case ReduceDataType.U2: return "uint16_t";
            case ReduceDataType.I4: return "int32_t";
            case ReduceDataType.I8: return "int64_t";
            case ReduceDataType.R4: return ConstStrings.Float32TypeName;
            case ReduceDataType.R8: return ConstStrings.Float64TypeName;
            case ReduceDataType.I: return "intptr_t";
            case ReduceDataType.Ref: return ConstStrings.ObjectPtrTypeName;
            default:
                throw new Exception($"Unsupported primitive ElementType: {type} in method: {_method.MethodDef.FullName}.");
            }
        }

        private string GetUnsignedTypeName(EvalVariable var)
        {
            switch (var.type)
            {
            case EvalDataType.Int32:
                return "uint32_t";
            case EvalDataType.Int64:
                return "uint64_t";
            case EvalDataType.I:
                return "uintptr_t";
            case EvalDataType.Float:
                return ConstStrings.Float32TypeName;
            case EvalDataType.Double:
                return ConstStrings.Float64TypeName;
            case EvalDataType.Ref:
                return "uintptr_t";
            default:
                throw new Exception($"Unsupported EvalDataType for unsigned type: {var.type} in method: {_method.MethodDef.FullName}.");
            }
        }

        private TempVariable CreateTempVariable(string typeName)
        {
            string name = $"__temp{_nextTempVarId++}";
            return new TempVariable(name, typeName);
        }

        private static string GetCurrentIpOffset(Instruction inst)
        {
            return inst.Offset.ToString();
        }


        private EvalStackState GetEvalStackState(BasicBlock basicBlock)
        {
            return _blockEvalStackStates[basicBlock];
        }

        private EvalVariable PushStack(TypeSig type)
        {
            List<EvalVariable> datas = _curState.runStackDatas;
            return PushStack0(datas, CreateEvalVariableFromTypeSig(type));
        }

        private EvalVariable CreateNewAndPushStack(EvalVariable oldVar)
        {
            var newVar = NewEvalVariable(oldVar.type, oldVar.typeSig, oldVar.inOutVar);
            return PushStack0(_curState.runStackDatas, newVar);
        }

        private int _lastVarId = 0;

        private EvalVariable NewEvalVariable(EvalDataType type, TypeSig typeSig, bool inOutVar = false)
        {
            return new EvalVariable(++_lastVarId, type, typeSig, inOutVar);
        }

        private EvalDataType GetEvalDataTypeFromPrimitiveElementType(ElementType type)
        {
            switch (type)
            {
            case ElementType.Boolean:
            case ElementType.Char:
            case ElementType.I1:
            case ElementType.U1:
            case ElementType.I2:
            case ElementType.U2:
            case ElementType.I4:
            case ElementType.U4:
                return EvalDataType.Int32;
            case ElementType.I8:
            case ElementType.U8:
                return EvalDataType.Int64;
            case ElementType.R4:
                return EvalDataType.Float;
            case ElementType.R8:
            case ElementType.R:
                return EvalDataType.Double;
            case ElementType.I:
            case ElementType.U:
                return EvalDataType.I;
            case ElementType.Object:
                return EvalDataType.Ref;
            default: throw new Exception($"Unsupported primitive ElementType: {type} in method: {_method.FullName}.");
            }
        }

        private EvalDataType GetEvalDataType(TypeSig type)
        {
            type = type.RemovePinnedAndModifiers();
            if (type.IsByRef)
            {
                return EvalDataType.I;
            }
            switch (type.ElementType)
            {
            case ElementType.Void: throw new InvalidOperationException("Cannot push void onto the stack");
            case ElementType.Boolean:
            case ElementType.Char:
            case ElementType.I1:
            case ElementType.U1:
            case ElementType.I2:
            case ElementType.U2:
            case ElementType.I4:
            case ElementType.U4:
                return EvalDataType.Int32;
            case ElementType.I8:
            case ElementType.U8:
                return EvalDataType.Int64;
            case ElementType.R4:
                return EvalDataType.Float;
            case ElementType.R8:
                return EvalDataType.Double;
            case ElementType.R:
                return EvalDataType.Double;
            case ElementType.I:
            case ElementType.U:
            case ElementType.Ptr:
            case ElementType.FnPtr:
            case ElementType.ByRef:
                return EvalDataType.I;
            case ElementType.String:
            case ElementType.Class:
            case ElementType.Array:
            case ElementType.SZArray:
            case ElementType.Object:
                return EvalDataType.Ref;
            case ElementType.ValueType:
            {
                TypeDef typeDef = type.ToTypeDefOrRef().ResolveTypeDefThrow();
                if (typeDef.IsEnum)
                {
                    return GetEvalDataType(typeDef.GetEnumUnderlyingType());
                }
                else
                {
                    return EvalDataType.ValueType;
                }
            }
            case ElementType.GenericInst:
            {
                GenericInstSig genericInstSig = (GenericInstSig)type;
                TypeDef typeDef = genericInstSig.GenericType.ToTypeDefOrRef().ResolveTypeDefThrow();
                if (!typeDef.IsValueType)
                {
                    return EvalDataType.Ref;
                }
                else if (typeDef.IsEnum)
                {
                    return GetEvalDataType(typeDef.GetEnumUnderlyingType());
                }
                else
                {
                    return EvalDataType.ValueType;
                }
            }
            case ElementType.TypedByRef:
            {
                // TypedByRef is a special type used in dynamic method invocation and reflection.
                // It is treated as a reference type in the evaluation stack.
                return EvalDataType.ValueType;
            }
            case ElementType.Var:
            case ElementType.MVar:
            {
                throw new Exception($"Generic parameter type {type} is not supported in method: {_method.FullName}.");
                //return NewEvalVariable(EvalDataType.ValueType, type);
            }
            //case ElementType.ValueArray:
            //case ElementType.CModOpt:
            //case ElementType.CModReqd:
            //case ElementType.Internal:
            //case ElementType.Module:
            //case ElementType.Sentinel:
            //    return NewEvalVariable(EvalDataType.Unknown, null);

            default: throw new Exception($"Unsupported type: {type} in method: {_method.FullName}.");
            }
        }

        private EvalVariable CreateEvalVariableFromTypeSig(TypeSig type)
        {
            type = type.RemovePinnedAndModifiers();
            if (type.IsByRef)
            {
                return NewEvalVariable(EvalDataType.I, type);
            }
            switch (type.ElementType)
            {
            case ElementType.Void: throw new InvalidOperationException("Cannot push void onto the stack");
            case ElementType.Boolean:
            case ElementType.Char:
            case ElementType.I1:
            case ElementType.U1:
            case ElementType.I2:
            case ElementType.U2:
            case ElementType.I4:
            case ElementType.U4:
                return NewEvalVariable(EvalDataType.Int32, null);
            case ElementType.I8:
            case ElementType.U8:
                return NewEvalVariable(EvalDataType.Int64, null);
            case ElementType.R4:
                return NewEvalVariable(EvalDataType.Float, null);
            case ElementType.R8:
                return NewEvalVariable(EvalDataType.Double, null);
            case ElementType.R:
                return NewEvalVariable(EvalDataType.Double, null);
            case ElementType.I:
            case ElementType.U:
            case ElementType.Ptr:
            case ElementType.FnPtr:
            case ElementType.ByRef:
                return NewEvalVariable(EvalDataType.I, type);
            case ElementType.String:
            case ElementType.Class:
            case ElementType.Array:
            case ElementType.SZArray:
            case ElementType.Object:
                return NewEvalVariable(EvalDataType.Ref, type);
            case ElementType.ValueType:
            {
                TypeDef typeDef = type.ToTypeDefOrRef().ResolveTypeDefThrow();
                if (typeDef.IsEnum)
                {
                    return CreateEvalVariableFromTypeSig(typeDef.GetEnumUnderlyingType());
                }
                else
                {
                    return NewEvalVariable(EvalDataType.ValueType, type);
                }
            }
            case ElementType.GenericInst:
            {
                GenericInstSig genericInstSig = (GenericInstSig)type;
                TypeDef typeDef = genericInstSig.GenericType.ToTypeDefOrRef().ResolveTypeDefThrow();
                if (!typeDef.IsValueType)
                {
                    return NewEvalVariable(EvalDataType.Ref, type);
                }
                else if (typeDef.IsEnum)
                {
                    return CreateEvalVariableFromTypeSig(typeDef.GetEnumUnderlyingType());
                }
                else
                {
                    return NewEvalVariable(EvalDataType.ValueType, type);
                }
            }
            case ElementType.TypedByRef:
            {
                // TypedByRef is a special type used in dynamic method invocation and reflection.
                // It is treated as a reference type in the evaluation stack.
                return NewEvalVariable(EvalDataType.ValueType, type);
            }
            case ElementType.Var:
            case ElementType.MVar:
            {
                throw new Exception($"Generic parameter type {type} is not supported in method: {_method.FullName}.");
                //return NewEvalVariable(EvalDataType.ValueType, type);
            }
            //case ElementType.ValueArray:
            //case ElementType.CModOpt:
            //case ElementType.CModReqd:
            //case ElementType.Internal:
            //case ElementType.Module:
            //case ElementType.Sentinel:
            //    return NewEvalVariable(EvalDataType.Unknown, null);

            default: throw new Exception($"Unsupported type: {type} in method: {_method.FullName}.");
            }
        }

        private EvalVariable PushStack0(List<EvalVariable> datas, EvalVariable type)
        {
            datas.Add(type);
            return type;
        }

        private EvalVariable PushStack(ITypeDefOrRef type)
        {
            List<EvalVariable> datas = _curState.runStackDatas;
            return PushStack0(datas, CreateEvalVariableFromTypeSig(type.ToTypeSig()));
        }

        private EvalVariable PushStack(EvalDataType type)
        {
            Debug.Assert(type != EvalDataType.ValueType, "Cannot push EvalDataType.Value without type sig onto the stack.");
            List<EvalVariable> datas = _curState.runStackDatas;
            return PushStack0(datas, NewEvalVariable(type, null));
        }

        private EvalVariable PushStackObject(TypeSig type)
        {
            List<EvalVariable> datas = _curState.runStackDatas;
            return PushStack0(datas, NewEvalVariable(EvalDataType.Ref, type));
        }

        private EvalVariable PushStackPointer(TypeSig type)
        {
            List<EvalVariable> datas = _curState.runStackDatas;
            return PushStack0(datas, NewEvalVariable(EvalDataType.I, type));
        }

        private EvalVariable Pop()
        {
            var runStackDatas = _curState.runStackDatas;
            int index = runStackDatas.Count - 1;
            EvalVariable top = runStackDatas[index];
            runStackDatas.RemoveAt(index);
            return top;
        }

        EvalVariable Peek()
        {
            var runStackDatas = _curState.runStackDatas;
            return runStackDatas.Last();
        }

        private void Pop(int count)
        {
            var runStackDatas = _curState.runStackDatas;
            runStackDatas.RemoveRange(runStackDatas.Count - count, count);
        }

        //private void PushStackObject(List<EvalDataTypeWithSig> datas)
        //{
        //    datas.Add(CreateEvalVariable(EvalDataType.Ref, _corlibTypes.Object));
        //}

        private EvalDataType CalcBasicBinOpRetType(EvalDataType op1, EvalDataType op2)
        {
            switch (op1)
            {
            case EvalDataType.Int32:
            {
                switch (op2)
                {
                case EvalDataType.Int32: return EvalDataType.Int32;
                case EvalDataType.Int64: return EvalDataType.Int64;
                case EvalDataType.I: return EvalDataType.I;
                default: throw new Exception($"Unsupported operand type: {op2} for {op1} in binary operation.");
                }
            }
            case EvalDataType.Int64:
            {
                switch (op2)
                {
                case EvalDataType.Int32: return EvalDataType.Int64;
                case EvalDataType.Int64:
                case EvalDataType.I:
                    return EvalDataType.Int64;
                default: throw new Exception($"Unsupported operand type: {op2} for {op1} in binary operation.");
                }
            }
            case EvalDataType.I:
            {
                switch (op2)
                {
                case EvalDataType.Int32: return EvalDataType.I;
                case EvalDataType.Int64: return EvalDataType.Int64;
                case EvalDataType.I: return EvalDataType.I;
                default: throw new Exception($"Unsupported operand type: {op2} for {op1} in binary operation.");
                }
            }
            case EvalDataType.Float:
            {
                switch (op2)
                {
                case EvalDataType.Float: return EvalDataType.Float;
                case EvalDataType.Double: return EvalDataType.Double;
                default: throw new Exception($"Unsupported operand type: {op2} for {op1} in binary operation.");
                }
            }
            case EvalDataType.Double:
            {
                switch (op2)
                {
                case EvalDataType.Float:
                case EvalDataType.Double: return EvalDataType.Double;
                default: throw new Exception($"Unsupported operand type: {op2} for {op1} in binary operation.");
                }
            }
            default: throw new Exception($"Unsupported operand type: {op1} in binary operation.");
            }
        }

        private void EmitVariableDeclaration(EvalVariable var)
        {
            string typeName = GetTypeName(var);
            _bodyWriter.AddLine($"{typeName} {GetEvalVariableName(var)};");
        }

        private string MayFoldCast(string srcTypeName, string dstTypeName, string expr)
        {
            if (srcTypeName == dstTypeName)
            {
                return expr;
            }
            else
            {
                return $"({dstTypeName})({expr})";
            }
        }

        private string GetVariableMayCast(EvalVariable srcVar, string dstTypeName)
        {
            string srcTypeName = GetTypeName(srcVar);
            return MayFoldCast(srcTypeName, dstTypeName, GetEvalVariableName(srcVar));
        }

        private string GetVariableMayCast(EvalVariable srcVar, TypeSig dstType)
        {
            string dstTypeName = GetExactTypeName(dstType);
            string srcTypeName = GetTypeName(srcVar);
            return MayFoldCast(srcTypeName, dstTypeName, GetEvalVariableName(srcVar));
        }

        private string GetVariableMayCast(EvalVariable srcVar, ParameterVariable dstVar)
        {
            string srcTypeName = GetTypeName(srcVar);
            string dstTypeName = GetExactTypeName(dstVar.Type);
            return MayFoldCast(srcTypeName, dstTypeName, GetEvalVariableName(srcVar));
        }

        private string GetVariableMayCast(ParameterVariable srcVar, EvalVariable dstVar)
        {
            string srcTypeName = GetExactTypeName(srcVar.Type);
            string dstTypeName = GetTypeName(dstVar);
            return MayFoldCast(srcTypeName, dstTypeName, GetParameterName(srcVar));
        }

        private string GetVariableMayCast(EvalVariable srcVar, LocalVariable dstVar)
        {
            string srcTypeName = GetTypeName(srcVar);
            string dstTypeName = GetExactTypeName(dstVar.Type);
            return MayFoldCast(srcTypeName, dstTypeName, GetEvalVariableName(srcVar));
        }

        private string GetVariableMayCast(LocalVariable srcVar, EvalVariable dstVar)
        {
            string srcTypeName = GetExactTypeName(srcVar.Type);
            string dstTypeName = GetTypeName(dstVar);
            return MayFoldCast(srcTypeName, dstTypeName, GetLocalName(srcVar));
        }

        private void EmitVariableDeclarationAndAssignment(EvalVariable dstVar, EvalVariable srcVar)
        {
            string typeName = GetTypeName(dstVar);
            _bodyWriter.AddLine($"{typeName} {GetEvalVariableName(dstVar)} = {GetEvalVariableName(srcVar)};");
        }

        private void EmitLoadArg(Instruction inst, ParameterVariable p)
        {
            var newVar = CreateNewAndPushStack(p._evalVariable);
            string typeName = GetTypeName(newVar);
            _bodyWriter.AddLine($"{typeName} {GetEvalVariableName(newVar)} = {GetVariableMayCast(p, newVar)};");
        }

        private void EmitLoadArgAddress(Instruction inst, ParameterVariable p)
        {
            var newVar = PushStackPointer(new ByRefSig(p.Type));
            string typeName = GetTypeName(newVar);
            _bodyWriter.AddLine($"{typeName} {GetEvalVariableName(newVar)} = ({typeName})&{GetParameterName(p)};");
        }

        private void EmitStoreArg(Instruction inst, ParameterVariable p)
        {
            var srcVar = Pop();
            _bodyWriter.AddLine($"{GetParameterName(p)} = {GetVariableMayCast(srcVar, p)};");
        }

        private void EmitLoadLocal(Instruction inst, LocalVariable local)
        {
            var newVar = CreateNewAndPushStack(local._evalVariable);
            _bodyWriter.AddLine($"{GetTypeName(newVar)} {GetEvalVariableName(newVar)} = {GetVariableMayCast(local, newVar)};");
        }

        private void EmitStoreLocal(Instruction inst, LocalVariable local)
        {
            var srcVar = Pop();
            _bodyWriter.AddLine($"{GetLocalName(local)} = {GetVariableMayCast(srcVar, local)};");
        }

        private void EmitLoadLocalAddress(Instruction inst, LocalVariable local)
        {
            var newVar = PushStackPointer(new ByRefSig(local.Type));
            string typeName = GetTypeName(newVar);
            _bodyWriter.AddLine($"{typeName} {GetEvalVariableName(newVar)} = ({typeName})&{GetLocalName(local)};");
        }

        private void EmitLoadNull(Instruction inst)
        {
            var newVar = PushStackObject(_corlibTypes.Object);
            _bodyWriter.AddLine($"{GetTypeName(newVar)} {GetEvalVariableName(newVar)} = nullptr;");
        }

        private void EmitLoadInt32(Instruction inst, int value)
        {
            var newVar = PushStack(EvalDataType.Int32);
            _bodyWriter.AddLine($"{GetTypeName(newVar)} {GetEvalVariableName(newVar)} = {FormatInt32Literal(value)};");
        }

        private void EmitLoadInt64(Instruction inst, long value)
        {
            var newVar = PushStack(EvalDataType.Int64);
            _bodyWriter.AddLine($"{GetTypeName(newVar)} {GetEvalVariableName(newVar)} = {FormatInt64Literal(value)};");
        }

        private static string FormatInt32Literal(int value)
        {
            if (value == int.MinValue)
                return "INT32_MIN";
            return $"{value}";
        }

        // int64_t min: avoid -9223372036854775808L (Clang -Wimplicitly-unsigned-literal); stdint.h is pulled in via codegen headers.
        private static string FormatInt64Literal(long value)
        {
            if (value == long.MinValue)
                return "INT64_MIN";
            return $"{value}LL";
        }

        private string FormatFloatLiteral(float value)
        {
            if (float.IsNaN(value))
                return "std::numeric_limits<float>::quiet_NaN()";
            else if (float.IsPositiveInfinity(value))
                return "std::numeric_limits<float>::infinity()";
            else if (float.IsNegativeInfinity(value))
                return "-std::numeric_limits<float>::infinity()";
            else
            {
                string str = value.ToString("R"); // "R" ensures round-trip formatting
                // Ensure the string contains a decimal point for valid C++ float literal
                if (!str.Contains('.') && !str.Contains('E') && !str.Contains('e'))
                    str += ".0";
                return str + "f";
            }
        }

        private string FormatDoubleLiteral(double value)
        {
            if (double.IsNaN(value))
                return "std::numeric_limits<double>::quiet_NaN()";
            else if (double.IsPositiveInfinity(value))
                return "std::numeric_limits<double>::infinity()";
            else if (double.IsNegativeInfinity(value))
                return "-std::numeric_limits<double>::infinity()";
            else
            {
                string str = value.ToString("R"); // "R" ensures round-trip formatting
                if (!str.Contains('.') && !str.Contains('E') && !str.Contains('e'))
                    str += ".0";
                return str;
            }
        }

        private void EmitLoadFloat(Instruction inst, float value)
        {
            var newVar = PushStack(EvalDataType.Float);
            _bodyWriter.AddLine($"{GetTypeName(newVar)} {GetEvalVariableName(newVar)} = {FormatFloatLiteral(value)};");
        }

        private void EmitLoadDouble(Instruction inst, double value)
        {
            var newVar = PushStack(EvalDataType.Double);
            _bodyWriter.AddLine($"{GetTypeName(newVar)} {GetEvalVariableName(newVar)} = {FormatDoubleLiteral(value)};");
        }

        private void EmitLoadStr(Instruction inst, string value)
        {
            var newVar = PushStackObject(_corlibTypes.String);
            _rawILReader.Position = _codeStartPosition + inst.Offset + 1; // Operand is right after opcode
            uint token = _rawILReader.ReadUInt32();
            var strVar = _runtimeResolvedMetadatas.GetUserStringVariable(value, new MDToken(token));
            var typeName = GetTypeName(newVar);
            _bodyWriter.AddLine($"{typeName} {GetEvalVariableName(newVar)} = ({typeName}){strVar.GetFullReferenceVariableName()};");
        }

        private void EmitDup(EvalVariable top)
        {
            // FIXME:  Duplicate the top value on the stack
            //_curState.runStackDatas.Add(top);
            // Duplicate the top value on the stack
            // Implementation omitted for brevity
            var newVar = PushStack0(_curState.runStackDatas, NewEvalVariable(top.type, top.typeSig));
            EmitVariableDeclarationAndAssignment(newVar, top);
        }

        private void EmitBinArithOp(Instruction inst, string opSymbol, bool isUnsigned = false)
        {
            // Implementation omitted for brevity

            var op2 = Pop();
            var op1 = Pop();
            var retType = CalcBasicBinOpRetType(op1.type, op2.type);
            var retVar = PushStack0(_curState.runStackDatas, NewEvalVariable(retType, null));
            string typeName = GetTypeName(retVar);
            if (op1.type == op2.type && !isUnsigned)
            {
                _bodyWriter.AddLine($"{typeName} {GetEvalVariableName(retVar)} = {GetEvalVariableName(op1)} {opSymbol} {GetEvalVariableName(op2)};");
            }
            else
            {
                _bodyWriter.AddLine($"{typeName} {GetEvalVariableName(retVar)} = static_cast<{typeName}>({GetEvalVariableExpr(op1, isUnsigned)} {opSymbol} {GetEvalVariableExpr(op2, isUnsigned)});");
            }
        }

        private void EmitUnaryArithOp(Instruction inst, string opSymbol)
        {
            var op = Pop();
            var retType = op.type;
            var retVar = PushStack0(_curState.runStackDatas, NewEvalVariable(retType, null));
            _bodyWriter.AddLine($"{GetTypeName(retVar)} {GetEvalVariableName(retVar)} = {opSymbol}{GetEvalVariableName(op)};");
        }

        private void EmitBinBitwiseOp(Instruction inst, string opSymbol, bool isUnsigned = false)
        {
            var op2 = Pop();
            var op1 = Pop();
            var retType = CalcBasicBinOpRetType(op1.type, op2.type);
            var retVar = PushStack0(_curState.runStackDatas, NewEvalVariable(retType, null));
            string typeName = GetTypeName(retVar);
            if (op1.type == op2.type && !isUnsigned)
            {
                _bodyWriter.AddLine($"{typeName} {GetEvalVariableName(retVar)} = {GetEvalVariableName(op1)} {opSymbol} {GetEvalVariableName(op2)};");
            }
            else
            {
                _bodyWriter.AddLine($"{typeName} {GetEvalVariableName(retVar)} = static_cast<{typeName}>({GetEvalVariableExpr(op1, isUnsigned)} {opSymbol} {GetEvalVariableExpr(op2, isUnsigned)});");
            }
        }

        private void EmitUnaryBitwiseOp(Instruction inst, string opSymbol)
        {
            var op = Pop();
            var retType = op.type;
            var retVar = PushStack0(_curState.runStackDatas, NewEvalVariable(retType, null));
            _bodyWriter.AddLine($"{GetTypeName(retVar)} {GetEvalVariableName(retVar)} = {opSymbol}{GetEvalVariableName(op)};");
        }

        private void EmitBitShiftOp(Instruction inst, string opSymbol, bool isUnsigned = false)
        {
            var shiftAmount = Pop();
            var value = Pop();
            var retType = value.type;
            var retVar = PushStack0(_curState.runStackDatas, NewEvalVariable(retType, null));
            if (value.type == retType && !isUnsigned)
            {
                _bodyWriter.AddLine($"{GetTypeName(retVar)} {GetEvalVariableName(retVar)} = {GetEvalVariableName(value)} {opSymbol} {GetEvalVariableName(shiftAmount)};");
            }
            else
            {
                _bodyWriter.AddLine($"{GetTypeName(retVar)} {GetEvalVariableName(retVar)} = static_cast<{GetTypeName(retVar)}>({GetEvalVariableExpr(value, isUnsigned)} {opSymbol} {GetEvalVariableName(shiftAmount)});");
            }
        }

        private void AddArch64ConditionMacroBegin()
        {
            _bodyWriter.AddLine("#if LEANCLR_ARCH_64BIT");
        }

        private void AddArch64ConditionMacroElse()
        {
            _bodyWriter.AddLine("#else");
        }

        private void AddArch64ConditionMacroEnd()
        {
            _bodyWriter.AddLine("#endif");
        }

        private void EmitBinArithOpOverflow(Instruction inst, string opSymbol, bool isUnsigned = false)
        {
            var op2 = Pop();
            var op1 = Pop();
            var retType = CalcBasicBinOpRetType(op1.type, op2.type);
            var retVar = PushStack0(_curState.runStackDatas, NewEvalVariable(retType, null));
            string functionName;
            switch (retVar.type)
            {
            case EvalDataType.Int32:
            {
                functionName = isUnsigned ? $"CHECK_{opSymbol}_OVERFLOW_U32" : $"CHECK_{opSymbol}_OVERFLOW_I32";
                break;
            }
            case EvalDataType.Int64:
            {
                functionName = isUnsigned ? $"CHECK_{opSymbol}_OVERFLOW_U64" : $"CHECK_{opSymbol}_OVERFLOW_I64";
                break;
            }
            case EvalDataType.I:
            {
                functionName = isUnsigned ? $"CHECK_{opSymbol}_OVERFLOW_UINTPTR" : $"CHECK_{opSymbol}_OVERFLOW_INTPTR";
                break;
            }
            default:
                throw new Exception($"Unsupported operand type for overflow check: {retVar.type} in method: {_method.FullName}.");
            }
            string retTypeName = isUnsigned ? GetUnsignedTypeName(retVar) : GetTypeName(retVar);
            _bodyWriter.AddLine($"{GetTypeName(retVar)} {GetEvalVariableName(retVar)};");
            _bodyWriter.AddLine($"if ({functionName}({GetEvalVariableExpr(op1, isUnsigned)}, {GetEvalVariableExpr(op2, isUnsigned)}, ({retTypeName}*)&{GetEvalVariableName(retVar)}))");
            _bodyWriter.AddLine("{");
            _bodyWriter.IncreaseIndent();
            EmitThrowRuntimeError(inst, "Overflow");
            _bodyWriter.DecreaseIndent();
            _bodyWriter.AddLine("}");
        }

        private void EmitDivOrRemOp(Instruction inst, string opSymbol, string floatFuncName, bool isUnsigned)
        {
            var op2 = Pop();
            var op1 = Pop();
            var retType = CalcBasicBinOpRetType(op1.type, op2.type);
            var retVar = PushStack0(_curState.runStackDatas, NewEvalVariable(retType, null));
            string opType = isUnsigned ? GetUnsignedTypeName(retVar) : GetTypeName(retVar);
            string op1Expr = GetEvalVariableExprWithCast(op1, opType);
            string op2Expr = GetEvalVariableExprWithCast(op2, opType);
            bool isInteger = false;
            switch (retVar.type)
            {
            case EvalDataType.Int32:
            case EvalDataType.Int64:
            case EvalDataType.I:
            {
                isInteger = true;
                _bodyWriter.AddLine($"if ({op2Expr} == 0)");
                _bodyWriter.AddLine("{");
                _bodyWriter.IncreaseIndent();
                EmitThrowRuntimeError(inst, "DivideByZero");
                _bodyWriter.DecreaseIndent();
                _bodyWriter.AddLine("}");
                if (!isUnsigned)
                {
                    _bodyWriter.AddLine($"if ({op1Expr} == std::numeric_limits<{opType}>::min() && {op2Expr} == -1)");
                    _bodyWriter.AddLine("{");
                    _bodyWriter.IncreaseIndent();
                    EmitThrowRuntimeError(inst, "Overflow");
                    _bodyWriter.DecreaseIndent();
                    _bodyWriter.AddLine("}");
                }
                break;
            }
            case EvalDataType.Float:
            case EvalDataType.Double:
            {
                // nothing to do;
                break;
            }
            default:
                throw new Exception($"Unsupported operand type for overflow check: {retVar.type} in method: {_method.FullName}.");
            }
            string retTypeName = GetTypeName(retVar);
            string retVarName = GetEvalVariableName(retVar);
            if (isInteger || string.IsNullOrEmpty(floatFuncName))
            {
                _bodyWriter.AddLine($"{retTypeName} {retVarName} = {MayFoldCast(opType, retTypeName, $"{op1Expr} {opSymbol} {op2Expr}")};");
            }
            else
            {
                _bodyWriter.AddLine($"{retTypeName} {retVarName} = {MayFoldCast(opType, retTypeName, $"{floatFuncName}({op1Expr}, {op2Expr})")};");
            }
        }

        private void EmitRet(EvalVariable ret)
        {
            if (ret != null)
            {
                // fix compiler warning C4312: "conversion from 'int' to 'void*' of greater size"
                if (ret.type == EvalDataType.Int32 && GetEvalDataType(_method.RetType) == EvalDataType.Ref)
                {
                    _bodyWriter.AddLine($"{ConstStrings.CodegenReturn}(({ConstStrings.ObjectPtrTypeName})({ConstStrings.IntPtrTypeName}){GetEvalVariableName(ret)});");
                }
                else
                {
                    _bodyWriter.AddLine($"{ConstStrings.CodegenReturn}({GetVariableMayCast(ret, TypeNameService.GetCppTypeNameAsFieldOrArgOrLoc(_method.RetType, TypeNameRelaxLevel.AbiRelaxed))});");
                }
            }
            else
            {
                _bodyWriter.AddLine($"{ConstStrings.CodegenReturnVoid}();");
            }
            _curState.runStackDatas.Clear();
        }

        private void SetupInOutVars(BasicBlock fromBb, BasicBlock toBb)
        {
            EvalStackState fromState = _blockEvalStackStates[fromBb];
            EvalStackState toState = _blockEvalStackStates[toBb];
            Debug.Assert(fromState.visited);
            if (toState.visited || toState.inputStackSetup)
            {
                Debug.Assert(fromState.runStackDatas.Count == toState.inputStackDatas.Count);
                for (int i = 0; i < fromState.runStackDatas.Count; i++)
                {
                    var fromVar = fromState.runStackDatas[i];
                    var toVar = toState.inputStackDatas[i];
                    if (fromVar.varId == toVar.varId)
                        continue;
                    Debug.Assert(fromVar.type == toVar.type);
                    _bodyWriter.AddLine($"{GetEvalVariableName(toVar)} = {GetEvalVariableName(fromVar)};");
                }
            }
            else
            {
                toState.inputStackSetup = true;
                Debug.Assert(toState.inputStackDatas.Count == 0);
                for (int i = 0; i < fromState.runStackDatas.Count; i++)
                {
                    var fromVar = fromState.runStackDatas[i];
                    var toVar = NewEvalVariable(fromVar.type, fromVar.typeSig, true);
                    toState.inputStackDatas.Add(toVar);
                    _localsWriter.AddLine($"{GetTypeName(toVar)} {GetEvalVariableName(toVar)};");
                    _bodyWriter.AddLine($"{GetEvalVariableName(toVar)} = {GetEvalVariableName(fromVar)};");
                }
            }
        }

        private void EmitGotoWithSetupInOutVars(BasicBlock targetBb)
        {
            // setup in/out vars should be done before the goto
            SetupInOutVars(_curBb, targetBb);
            _bodyWriter.AddLine($"goto {GetBasicBlockStartLabelName(targetBb)};");
        }

        private void EmitFallThroughSetupInOutVars(BasicBlock targetBb)
        {
            Debug.Assert(_curBb.nextBlock == targetBb);
            // setup in/out vars should be done before the goto
            SetupInOutVars(_curBb, targetBb);
        }

        private void EmitBranchUnconditional(Instruction inst, Instruction target)
        {
            BasicBlock targetBb = _instToBbMap[target];
            EmitGotoWithSetupInOutVars(targetBb);
        }

        private void EmitBranchTrueOrFalse(Instruction inst, Instruction target, bool branchIfTrue)
        {
            var conditionVar = Pop();
            BasicBlock targetBb = _instToBbMap[target];
            // setup in/out vars should be done before the goto
            SetupInOutVars(_curBb, targetBb);

            _bodyWriter.AddLine($"if ({(branchIfTrue ? "" : "!")}{GetEvalVariableName(conditionVar)})");
            _bodyWriter.AddLine("{");
            _bodyWriter.IncreaseIndent();
            EmitGotoWithSetupInOutVars(targetBb);
            _bodyWriter.DecreaseIndent();
            _bodyWriter.AddLine("}");

            // we will handle in end of the current block, so do nothing here
            // SetupInOutVars(_curBb, _curBb.nextBlock);
        }

        private static bool IsFloatType(EvalDataType type)
        {
            return type == EvalDataType.Float || type == EvalDataType.Double;
        }

        private void EmitBranchComparison(Instruction inst, Instruction target, string comparisonOp, bool isUnOrdered = false)
        {
            var op2 = Pop();
            var op1 = Pop();
            BasicBlock targetBb = _instToBbMap[target];

            bool isFloatComparison = IsFloatType(op1.type);
            if (isFloatComparison != IsFloatType(op2.type))
            {
                throw new Exception($"Mismatched operand types for comparison in method: {_method.FullName}.");
            }
            if (!isFloatComparison)
            {
                if (op1.type == op2.type && !isUnOrdered)
                {
                    _bodyWriter.AddLine($"if ({GetEvalVariableName(op1)} {comparisonOp} {GetEvalVariableName(op2)})");
                }
                else
                {
                    _bodyWriter.AddLine($"if ({GetEvalVariableExpr(op1, isUnOrdered)} {comparisonOp} {GetEvalVariableExpr(op2, isUnOrdered)})");
                }
            }
            else
            {
                if (isUnOrdered)
                {
                    string nanCheck = $"(std::isnan({GetEvalVariableExpr(op1, isUnOrdered)}) || std::isnan({GetEvalVariableExpr(op2, isUnOrdered)}))";
                    _bodyWriter.AddLine($"if ( {nanCheck} || ({GetEvalVariableExpr(op1, isUnOrdered)} {comparisonOp} {GetEvalVariableExpr(op2, isUnOrdered)}))");
                }
                else
                {
                    _bodyWriter.AddLine($"if ({GetEvalVariableExpr(op1, isUnOrdered)} {comparisonOp} {GetEvalVariableExpr(op2, isUnOrdered)})");
                }
            }
            _bodyWriter.AddLine("{");
            _bodyWriter.IncreaseIndent();
            EmitGotoWithSetupInOutVars(targetBb);
            _bodyWriter.DecreaseIndent();
            _bodyWriter.AddLine("}");

            // we will handle in end of the current block, so do nothing here
            // SetupInOutVars(_curBb, _curBb.nextBlock);
        }

        private void EmitComparisonOp(Instruction inst, string comparisonOp, bool isUnOrdered = false)
        {
            var op2 = Pop();
            var op1 = Pop();
            var retVar = PushStack0(_curState.runStackDatas, NewEvalVariable(EvalDataType.Int32, null));
            bool isFloatComparison = IsFloatType(op1.type);
            if (isFloatComparison != IsFloatType(op2.type))
            {
                throw new Exception($"Mismatched operand types for comparison in method: {_method.FullName}.");
            }
            if (!isFloatComparison)
            {
                if (op1.type == op2.type && !isUnOrdered)
                {
                    _bodyWriter.AddLine($"{GetTypeName(retVar)} {GetEvalVariableName(retVar)} = ({GetEvalVariableName(op1)} {comparisonOp} {GetEvalVariableName(op2)}) ? 1 : 0;");
                }
                else
                {
                    _bodyWriter.AddLine($"{GetTypeName(retVar)} {GetEvalVariableName(retVar)} = ({GetEvalVariableExpr(op1, isUnOrdered)} {comparisonOp} {GetEvalVariableExpr(op2, isUnOrdered)}) ? 1 : 0;");
                }
            }
            else
            {
                if (isUnOrdered)
                {
                    string nanCheck = $"(std::isnan({GetEvalVariableExpr(op1, isUnOrdered)}) || std::isnan({GetEvalVariableExpr(op2, isUnOrdered)}))";
                    _bodyWriter.AddLine($"{GetTypeName(retVar)} {GetEvalVariableName(retVar)} = ( {nanCheck} || ({GetEvalVariableExpr(op1, isUnOrdered)} {comparisonOp} {GetEvalVariableExpr(op2, isUnOrdered)})) ? 1 : 0;");
                }
                else
                {
                    _bodyWriter.AddLine($"{GetTypeName(retVar)} {GetEvalVariableName(retVar)} = ({GetEvalVariableExpr(op1, isUnOrdered)} {comparisonOp} {GetEvalVariableExpr(op2, isUnOrdered)}) ? 1 : 0;");
                }
            }
        }

        private void EmitSwitch(Instruction inst, Instruction[] targets)
        {
            var switchValue = Pop();
            _bodyWriter.AddLine("switch (" + GetEvalVariableName(switchValue) + ")");
            _bodyWriter.AddLine("{");
            _bodyWriter.IncreaseIndent();
            for (int i = 0; i < targets.Length; i++)
            {
                BasicBlock targetBb = _instToBbMap[targets[i]];
                if (_curState.runStackDatas.Count > 0)
                {
                    _bodyWriter.AddLine($"case {i}:");
                    EmitGotoWithSetupInOutVars(targetBb);
                }
                else
                {
                    // more clear code without stack values
                    SetupInOutVars(_curBb, targetBb);
                    _bodyWriter.AddLine($"case {i}: goto {GetBasicBlockStartLabelName(targetBb)};");
                }
            }
            _bodyWriter.AddLine("default: break;");
            _bodyWriter.DecreaseIndent();
            _bodyWriter.AddLine("}");
        }

        private void EmitLoadInd(Instruction inst, ElementType type)
        {
            var addressVar = Pop();
            string eleTypeName = GetPrimitiveTypeName(type);
            EvalDataType dataType = GetEvalDataTypeFromPrimitiveElementType(type);
            var retVar = PushStack(dataType);
            _bodyWriter.AddLine($"{GetTypeName(retVar)} {GetEvalVariableName(retVar)} = *({eleTypeName}*){GetEvalVariableName(addressVar)};");
        }

        private void EmitStoreInd(Instruction inst, ReduceDataType type)
        {
            var valueVar = Pop();
            var addressVar = Pop();
            string eleTypeName = GetReduceTypeName(type);
            _bodyWriter.AddLine($"*({eleTypeName}*){GetEvalVariableName(addressVar)} = {GetEvalVariableExprWithCast(valueVar, eleTypeName)};");
        }

        private string ReduceCast(string type1, string type2)
        {
            if (type1 == type2)
            {
                return $"({type1})";
            }
            else
            {
                return $"({type2})({type1})";
            }
        }

        private string ReduceCast(string type1, string type2, string type3)
        {
            if (type1 == type2)
            {
                return ReduceCast(type1, type3);
            }
            else
            {
                return $"{ReduceCast(type2, type3)}({type1})";
            }
        }

        private string ReduceCast(string type1, string type2, string type3, string type4)
        {
            if (type1 == type2)
            {
                return ReduceCast(type1, type3, type4);
            }
            else
            {
                return $"{ReduceCast(type2, type3, type4)}({type1})";
            }
        }

        private void EmitConvert(Instruction inst, ElementType dstType, bool isUnsigned = false)
        {
            var srcVar = Pop();
            var retVar = PushStack(GetEvalDataTypeFromPrimitiveElementType(dstType));
            var (_, dstIsUnsigned) = GetSizeLevelAndUnsignedness(dstType);
            string srcExpr = GetEvalVariableExpr(srcVar, isUnsigned);
            string srcTypeName = GetTypeName(srcVar, isUnsigned);
            string dstTypeName = GetTypeName(retVar);
            string castToTypeName = GetPrimitiveTypeName(dstType);
            string retVarName = GetEvalVariableName(retVar);
            if (!IsFloatType(srcVar.type) && !IsFloatType(retVar.type))
            {
                string intermediateTypeBeforeCast = GetTypeName(srcVar, isUnsigned || dstIsUnsigned);
                _bodyWriter.AddLine($"{dstTypeName} {retVarName} = {ReduceCast(srcTypeName, intermediateTypeBeforeCast, castToTypeName, dstTypeName)}{srcExpr};");
            }
            else
            {
                switch (dstType)
                {
                case ElementType.I1:
                case ElementType.U1:
                case ElementType.I2:
                case ElementType.U2:
                {
                    _bodyWriter.AddLine($"{dstTypeName} {retVarName} = {VmFunctionNames.CastFloatToSmallInt}<{srcTypeName}, {castToTypeName}>({srcExpr});");
                    break;
                }
                case ElementType.I4:
                case ElementType.U4:
                {
                    _bodyWriter.AddLine($"{dstTypeName} {retVarName} = {VmFunctionNames.CastFloatToI32}<{srcTypeName}, {castToTypeName}>({srcExpr});");
                    break;
                }
                case ElementType.I8:
                case ElementType.U8:
                {
                    _bodyWriter.AddLine($"{dstTypeName} {retVarName} = {VmFunctionNames.CastFloatToI64}<{srcTypeName}, {castToTypeName}>({srcExpr});");
                    break;
                }
                case ElementType.I:
                case ElementType.U:
                {
                    _bodyWriter.AddLineIgnoreIndent($"#if LEANCLR_ARCH_64BIT");
                    _bodyWriter.AddLine($"{dstTypeName} {retVarName} = {VmFunctionNames.CastFloatToI64}<{srcTypeName}, {castToTypeName}>({srcExpr});");
                    _bodyWriter.AddLineIgnoreIndent($"#else");
                    _bodyWriter.AddLine($"{dstTypeName} {retVarName} = {VmFunctionNames.CastFloatToI32}<{srcTypeName}, {castToTypeName}>({srcExpr});");
                    _bodyWriter.AddLineIgnoreIndent($"#endif");
                    break;
                }
                case ElementType.R4:
                case ElementType.R8:
                {
                    _bodyWriter.AddLine($"{dstTypeName} {retVarName} = ({dstTypeName}){srcExpr};");
                    break;
                }
                default:
                    throw new Exception($"Unsupported convert to ElementType: {dstType} in method: {_method.FullName}.");
                }
            }
        }

        private ElementType GetElementTypeFromEvalDataTypeWithUnsigned(EvalDataType dataType, bool isUnsigned)
        {
            switch (dataType)
            {
            case EvalDataType.Int32:
                return isUnsigned ? ElementType.U4 : ElementType.I4;
            case EvalDataType.Int64:
                return isUnsigned ? ElementType.U8 : ElementType.I8;
            case EvalDataType.I:
                return isUnsigned ? ElementType.U : ElementType.I;
            case EvalDataType.Float:
                return ElementType.R4;
            case EvalDataType.Double:
                return ElementType.R8;
            default:
                throw new Exception($"Unsupported EvalDataType: {dataType} for convert in method: {_method.FullName}.");
            }
        }

        enum SizeLevel
        {
            OneByte,
            TwoBytes,
            FourBytes,
            NativeBytes,
            EightBytes,
            Uncomparable,
        }

        private (SizeLevel, bool) GetSizeLevelAndUnsignedness(ElementType type)
        {
            switch (type)
            {
            case ElementType.I1:
                return (SizeLevel.OneByte, false);
            case ElementType.U1:
                return (SizeLevel.OneByte, true);
            case ElementType.I2:
                return (SizeLevel.TwoBytes, false);
            case ElementType.U2:
                return (SizeLevel.TwoBytes, true);
            case ElementType.I4:
                return (SizeLevel.FourBytes, false);
            case ElementType.U4:
                return (SizeLevel.FourBytes, true);
            case ElementType.I8:
                return (SizeLevel.EightBytes, false);
            case ElementType.U8:
                return (SizeLevel.EightBytes, true);
            case ElementType.I:
                return (SizeLevel.NativeBytes, false);
            case ElementType.U:
                return (SizeLevel.NativeBytes, true);
            case ElementType.R4:
            case ElementType.R8:
                return (SizeLevel.Uncomparable, false);
            default:
                throw new Exception($"Unsupported ElementType: {type} for size level and signedness in method: {_method.FullName}.");
            }
        }

        private bool IsOverflowCheckRequiredForConvert(ElementType srcType, ElementType dstType)
        {
            if (srcType == dstType)
                return false;
            switch (srcType)
            {
            case ElementType.I4:
            case ElementType.U4:
            case ElementType.I8:
            case ElementType.U8:
            case ElementType.I:
            case ElementType.U:
            {
                var (srcSizeLevel, srcIsUnsigned) = GetSizeLevelAndUnsignedness(srcType);
                var (dstSizeLevel, dstIsUnsigned) = GetSizeLevelAndUnsignedness(dstType);
                return dstSizeLevel < srcSizeLevel || srcIsUnsigned != dstIsUnsigned;
            }
            case ElementType.R4:
            case ElementType.R8:
            {
                return true;
            }
            default:
                throw new Exception($"Unsupported source type for convert overflow check: {srcType} in method: {_method.FullName}.");
            }
        }

        private string GetFloatToIntCastFunctionName(EvalDataType dataType)
        {
            switch (dataType)
            {
            case EvalDataType.Int32:
                return VmFunctionNames.CastFloatToI32;
            case EvalDataType.Int64:
                return VmFunctionNames.CastFloatToI64;
            case EvalDataType.I:
                return VmFunctionNames.CastFloatToIntPtr;
            default:
                throw new Exception($"Unsupported float type for convert overflow check: {dataType} in method: {_method.FullName}.");
            }
        }

        private void EmitOverflowCheck(SizeLevel srcSizeLevel, bool srcIsUnsigned, SizeLevel dstSizeLevel, bool dstIsUnsigned, string srcExpr, string srcTypeNameWithCast, string dstTypeNameWithCast, ElementType srcElementType, ElementType dstType)
        {
            if (srcIsUnsigned == dstIsUnsigned)
            {
                _bodyWriter.AddLine($"if ({srcExpr} < std::numeric_limits<{dstTypeNameWithCast}>::min() || {srcExpr} > std::numeric_limits<{dstTypeNameWithCast}>::max())");
            }
            else
            {
                if (dstIsUnsigned)
                {
                    if (dstSizeLevel < srcSizeLevel)
                    {
                        _bodyWriter.AddLine($"if ({srcExpr} < 0 || {srcExpr} > static_cast<{srcTypeNameWithCast}>(std::numeric_limits<{dstTypeNameWithCast}>::max()))");
                    }
                    else
                    {
                        _bodyWriter.AddLine($"if ({srcExpr} < 0)");
                    }
                }
                else
                {
                    if (dstSizeLevel <= srcSizeLevel)
                    {
                        _bodyWriter.AddLine($"if ({srcExpr} > static_cast<{srcTypeNameWithCast}>(std::numeric_limits<{dstTypeNameWithCast}>::max()))");
                    }
                    else
                    {
                        _bodyWriter.AddLine("if (false)"); // no check needed, just emit false condition
                    }
                }
            }
        }

        private void EmitConvertOverflow(Instruction inst, ElementType dstType, bool srcIsUnsigned)
        {
            var srcVar = Peek();
            var srcElementType = GetElementTypeFromEvalDataTypeWithUnsigned(srcVar.type, srcIsUnsigned);
            if (IsOverflowCheckRequiredForConvert(srcElementType, dstType))
            {
                var (srcSizeLevel, srcIsUnsignedActual) = GetSizeLevelAndUnsignedness(srcElementType);
                var (dstSizeLevel, dstIsUnsigned) = GetSizeLevelAndUnsignedness(dstType);

                string srcTypeNameWithCast = GetPrimitiveTypeName(srcElementType);
                string dstTypeNameWithCast = GetPrimitiveTypeName(dstType);
                string srcExpr = GetEvalVariableExpr(srcVar, srcIsUnsigned);
                switch (srcVar.type)
                {
                case EvalDataType.Int32:
                case EvalDataType.Int64:
                case EvalDataType.I:
                {
                    AddArch64ConditionMacroBegin();
                    {
                        SizeLevel actualSrcSizeLevel = srcSizeLevel == SizeLevel.NativeBytes ? SizeLevel.EightBytes : srcSizeLevel;
                        SizeLevel actualDstSizeLevel = dstSizeLevel == SizeLevel.NativeBytes ? SizeLevel.EightBytes : dstSizeLevel;
                        EmitOverflowCheck(actualSrcSizeLevel, srcIsUnsignedActual, actualDstSizeLevel, dstIsUnsigned, srcExpr, srcTypeNameWithCast, dstTypeNameWithCast, srcElementType, dstType);
                    }
                    AddArch64ConditionMacroElse();
                    {
                        SizeLevel actualSrcSizeLevel = srcSizeLevel == SizeLevel.NativeBytes ? SizeLevel.FourBytes : srcSizeLevel;
                        SizeLevel actualDstSizeLevel = dstSizeLevel == SizeLevel.NativeBytes ? SizeLevel.FourBytes : dstSizeLevel;
                        EmitOverflowCheck(actualSrcSizeLevel, srcIsUnsignedActual, actualDstSizeLevel, dstIsUnsigned, srcExpr, srcTypeNameWithCast, dstTypeNameWithCast, srcElementType, dstType);
                    }
                    AddArch64ConditionMacroEnd();
                    break;
                }
                case EvalDataType.Float:
                case EvalDataType.Double:
                {
                    _bodyWriter.AddLine($"if ({VmFunctionNames.IsNan}({srcExpr}) || {srcExpr} < static_cast<{GetTypeName(srcVar)}>(std::numeric_limits<{dstTypeNameWithCast}>::min()) || {srcExpr} > static_cast<{GetTypeName(srcVar)}>(std::numeric_limits<{dstTypeNameWithCast}>::max()))");
                    break;
                }
                default:
                    throw new Exception($"Unsupported source type for convert overflow check: {srcVar.type} in method: {_method.FullName}.");
                }
                _bodyWriter.AddLine("{");
                _bodyWriter.IncreaseIndent();
                EmitThrowRuntimeError(inst, "Overflow");
                _bodyWriter.DecreaseIndent();
                _bodyWriter.AddLine("}");
            }
            EmitConvert(inst, dstType, srcIsUnsigned);
        }

        private void EmitInitObj(Instruction inst, ITypeDefOrRef type)
        {
            var addressVar = Pop();
            var inflatedTypeSig = _method.InflateType(type.ToTypeSig());
            string typeName = GetExactTypeName(inflatedTypeSig);
            _bodyWriter.AddLine($"std::memset(({typeName}*){GetEvalVariableName(addressVar)}, 0, sizeof({typeName}));");
        }

        private void EmitCpObj(Instruction inst, ITypeDefOrRef type)
        {
            var srcVar = Pop();
            var dstVar = Pop();
            var inflatedTypeSig = _method.InflateType(type.ToTypeSig());
            string typeName = GetExactTypeName(inflatedTypeSig);
            _bodyWriter.AddLine($"*({typeName}*){GetEvalVariableName(dstVar)} = *({typeName}*){GetEvalVariableName(srcVar)};");
        }

        private void EmitLdObj(Instruction inst, ITypeDefOrRef type)
        {
            var addressVar = Pop();
            var inflatedTypeSig = _method.InflateType(type.ToTypeSig());
            var retVar = PushStack(inflatedTypeSig);
            string typeName = GetExactTypeName(inflatedTypeSig);
            _bodyWriter.AddLine($"{typeName} {GetEvalVariableName(retVar)} = *({typeName}*){GetEvalVariableName(addressVar)};");
        }

        private void EmitStObj(Instruction inst, ITypeDefOrRef type)
        {
            var valueVar = Pop();
            var addressVar = Pop();
            var inflatedTypeSig = _method.InflateType(type.ToTypeSig());
            string typeName = GetExactTypeName(inflatedTypeSig);
            _bodyWriter.AddLine($"*({typeName}*){GetEvalVariableName(addressVar)} = {GetEvalVariableExprWithCast(valueVar, typeName)};");
        }

        private string CreateMethodFunctionArgsWithCast(MethodDetail methodDetail, List<EvalVariable> args)
        {
            var sb = new StringBuilder();
            foreach (var param in methodDetail.ParamsIncludeThis)
            {
                if (param.Index > 0)
                {
                    sb.Append(", ");
                }
                sb.Append($"{GetVariableMayCast(args[param.Index], param.Type)}");
            }
            return sb.ToString();
        }

        private string CreateMethodFunctionArgsExcludedThisWithCast(MethodDetail methodDetail, List<EvalVariable> args)
        {
            var sb = new StringBuilder();
            var paramsIncludeThis = methodDetail.ParamsIncludeThis;
            int paramIndexOffset = methodDetail.IsStatic ? 0 : 1;
            for (int i = 0; i < args.Count; i++)
            {
                if (i > 0)
                {
                    sb.Append(", ");
                }
                var param = paramsIncludeThis[i + paramIndexOffset];
                Debug.Assert(param.Index == i + paramIndexOffset);
                sb.Append($"{GetVariableMayCast(args[i], param.Type)}");
            }
            return sb.ToString();
        }

        private string CreateMethodFunctionArgsWithCast(MethodSig methodSig, List<EvalVariable> args)
        {
            var sb = new StringBuilder();
            int index = 0;
            foreach (var param in methodSig.Params)
            {
                if (index > 0)
                {
                    sb.Append(", ");
                }
                sb.Append($"{GetVariableMayCast(args[index], param)} ");
                index++;
            }
            return sb.ToString();
        }

        public string GetMethodPointerFromFullReferenceMethodVariable(string methodVarName)
        {
            return $"{methodVarName}->{ConstStrings.MethodPointerFieldName}";
        }

        public string GetParentFromFullReferenceMethodVariable(string methodVarName)
        {
            return $"{methodVarName}->{ConstStrings.ParentFieldName}";
        }

        public string GetParentFromFullReferenceFieldVariable(string fieldVarName)
        {
            return $"{fieldVarName}->{ConstStrings.ParentFieldName}";
        }

        private void EmitRunClassStaticConstructor(Instruction inst, string klassVar)
        {
            _bodyWriter.AddLine($"if ({VmFunctionNames.IsCctorNotFinishied}({klassVar}))");
            _bodyWriter.AddLine("{");
            _bodyWriter.IncreaseIndent();
            _bodyWriter.AddLine($"{VmFunctionNames.THROW_ON_ERROR}({VmFunctionNames.RunClassStaticConstructor}({klassVar}), {CurMethodVar.GetFullReferenceVariableName()}, {GetCurrentIpOffset(inst)});");
            _bodyWriter.DecreaseIndent();
            _bodyWriter.AddLine("}");
        }

        private void EmitThrowRuntimeError(Instruction inst, string errName)
        {
            _bodyWriter.AddLine($"{VmFunctionNames.THROW_RUNTIME_ERROR}(leanclr::RtErr::{errName}, {CurMethodVar.GetFullReferenceVariableName()}, {GetCurrentIpOffset(inst)});");
        }

        private void EmitCheckNotNull(Instruction inst, EvalVariable objVar)
        {
            _bodyWriter.AddLine($"{VmFunctionNames.CHECK_NULL_REFERENCE}({GetEvalVariableName(objVar)}, {CurMethodVar.GetFullReferenceVariableName()}, {GetCurrentIpOffset(inst)});");
        }

        private void EmitThrowOnError(Instruction inst, string sourceExpr)
        {
            _bodyWriter.AddLine($"{VmFunctionNames.THROW_ON_ERROR}({sourceExpr}, {CurMethodVar.GetFullReferenceVariableName()}, {GetCurrentIpOffset(inst)});");
        }

        private void EmitDeclaringAssignOrThrow(Instruction inst, EvalVariable targetVar, string sourceExpr)
        {
            _bodyWriter.AddLine($"{VmFunctionNames.DECLARING_ASSIGN_OR_THROW}({GetTypeName(targetVar)}, {GetEvalVariableName(targetVar)}, {sourceExpr}, {CurMethodVar.GetFullReferenceVariableName()}, {GetCurrentIpOffset(inst)});");
        }

        private void EmitAssignOrThrow(Instruction inst, EvalVariable targetVar, string sourceExpr)
        {
            _bodyWriter.AddLine($"{VmFunctionNames.ASSIGN_OR_THROW}({GetEvalVariableName(targetVar)}, {sourceExpr}, {CurMethodVar.GetFullReferenceVariableName()}, {GetCurrentIpOffset(inst)});");
        }


        private void EmitCallByMethodPointerDirectly(Instruction inst, MethodDetail methodDetail, string methodVarName, List<EvalVariable> args, EvalVariable retVar)
        {
            var argsStr = CreateMethodFunctionArgsWithCast(methodDetail, args);
            if (!methodDetail.IsVoidReturn)
            {
                EmitAssignOrThrow(inst, retVar, $"(({methodDetail.CreateMethodFunctionTypeDefineWithoutName()}){GetMethodPointerFromFullReferenceMethodVariable(methodVarName)})({argsStr})");
            }
            else
            {
                EmitThrowOnError(inst, $"(({methodDetail.CreateMethodFunctionTypeDefineWithoutName()}){GetMethodPointerFromFullReferenceMethodVariable(methodVarName)})({argsStr})");
            }
        }

        private void EmitCallByInvoker(Instruction inst, MethodDetail methodDetail, string methodVarName, List<EvalVariable> args, EvalVariable retVar, bool callVir)
        {
            int paramCount = methodDetail.ParamCountIncludeThis;
            bool hasReturnValue = methodDetail.HasNotVoidReturn;
            _bodyWriter.AddLine("{");
            _bodyWriter.IncreaseIndent();
            string argsStr;
            if (paramCount == 0)
            {
                argsStr = "nullptr";
            }
            else
            {
                _bodyWriter.AddLine("constexpr size_t ARG0_OFFSET = 0;");
                for (int paramIndex = 0, last = paramCount - 1; paramIndex < last; paramIndex++)
                {
                    ParamDetail param = methodDetail.ParamsIncludeThis[paramIndex];
                    _bodyWriter.AddLine($"constexpr size_t ARG{paramIndex + 1}_OFFSET = ARG{paramIndex}_OFFSET + {ConstStrings.CodegenNamespace}::get_stack_object_size_for_type<{MethodGenerationUtil.GetExactTypeName(param.Type)}>();");
                }
                _bodyWriter.AddLine($"constexpr size_t ARGS_SIZE = ARG{paramCount - 1}_OFFSET + {ConstStrings.CodegenNamespace}::get_stack_object_size_for_type<{MethodGenerationUtil.GetExactTypeName(methodDetail.ParamsIncludeThis.Last().Type)}>();");
                argsStr = "__argsBuf";
                _bodyWriter.AddLine($"{ConstStrings.StackObjectTypeName} {argsStr}[ARGS_SIZE];");
                foreach (var param in methodDetail.ParamsIncludeThis)
                {
                    int paramIndex = param.Index;
                    string argTypeName = GetExactTypeName(param.Type);
                    _bodyWriter.AddLine($"{ConstStrings.CodegenNamespace}::expand_argument_to_eval_stack({GetEvalVariableName(args[paramIndex])}, {argsStr} + ARG{paramIndex}_OFFSET);");
                }
            }
            string retStr;
            if (hasReturnValue)
            {
                _bodyWriter.AddLine($"constexpr size_t RET_SIZE = {ConstStrings.CodegenNamespace}::get_stack_object_size_for_type<{MethodGenerationUtil.GetExactTypeName(methodDetail.RetType)}>();");
                retStr = "__retBuf";
                _bodyWriter.AddLine($"{ConstStrings.StackObjectTypeName} {retStr}[RET_SIZE];");
            }
            else
            {
                retStr = "nullptr";
            }
            string invokeMethodName = callVir ? VmFunctionNames.VirtualInvokeWithoutRunClassStaticConstructor : $"{(methodDetail.IsStatic ? VmFunctionNames.InvokeWithRunClassStaticConstructor : VmFunctionNames.InvokeWithoutRunClassStaticConstructor)}";

            EmitThrowOnError(inst, $"{invokeMethodName}({methodVarName}, {argsStr}, {retStr})");
            if (hasReturnValue)
            {
                string getFromEvalStackStr = $"{ConstStrings.CodegenNamespace}::get_eval_stack_value_as_type<{GetExactTypeName(methodDetail.RetType)}>({retStr})";
                _bodyWriter.AddLine($"{GetEvalVariableName(retVar)} = {MayFoldCast(GetExactTypeName(methodDetail.RetType), GetTypeName(retVar), getFromEvalStackStr)};");
            }
            _bodyWriter.DecreaseIndent();
            _bodyWriter.AddLine("}");
        }

        private void EmitCallCommon(Instruction inst, MethodDetail methodDetail, Func<string> methodVarNameProvider, List<EvalVariable> args, EvalVariable retVar)
        {
            if (TryEmitCallInstrinsic(inst, methodDetail, methodVarNameProvider, args, retVar))
            {
                return;
            }
            if (_manifestService.ShouldAOT(methodDetail.Method))
            {
                var argsStr = CreateMethodFunctionArgsWithCast(methodDetail, args);
                if (!methodDetail.IsVoidReturn)
                {
                    EmitAssignOrThrow(inst, retVar, $"{methodDetail.UniqueName}({argsStr})");
                }
                else
                {
                    EmitThrowOnError(inst, $"{methodDetail.UniqueName}({argsStr})");
                }
                //EmitCallByMethodPointerDirectly(inst, methodDetail, methodVarName, args, retVar);
            }
            else
            {
                EmitCallByInvoker(inst, methodDetail, methodVarNameProvider(), args, retVar, false);
            }
        }

        private void EmitCall(Instruction inst, IMethod method, uint token, bool emitCheckNullForInstanceMethod)
        {
            MethodDetail methodDetail = _metadataService.GetMethodDetail(method);
            _forwardDeclaration.AddMethodForwardDeclaration(method);

            int paramCount = methodDetail.ParamCountIncludeThis;
            bool hasReturnValue = !methodDetail.IsVoidReturn;
            var args = new List<EvalVariable>(_curState.runStackDatas.GetRange(_curState.runStackDatas.Count - paramCount, paramCount));
            if (emitCheckNullForInstanceMethod && !methodDetail.IsStatic)
            {
                EmitCheckNotNull(inst, args[0]);
            }
            Pop(paramCount);

            EvalVariable retVar = null;
            if (hasReturnValue)
            {
                retVar = PushStack(methodDetail.RetType);
                _bodyWriter.AddLine($"{GetTypeName(retVar)} {GetEvalVariableName(retVar)};");
            }

            EmitCallCommon(inst, methodDetail, () => _runtimeResolvedMetadatas.GetMethodVariable(method).GetFullReferenceVariableName(), args, retVar);
        }

        private bool IsParentOrInterfaceOfValueType(TypeDef declaringTypeDef)
        {
            if (declaringTypeDef.IsInterface)
            {
                // for interface, we should check value type at runtime since it can be implemented by value type
                return true;
            }
            if (!MetaUtil.IsCorlibOrSystemOrSystemCore(declaringTypeDef.Module))
            {
                return false;
            }
            string name = declaringTypeDef.Name;
            if (name == "Object" || name == "ValueType" || name == "Enum")
            {
                // for System.Object, we should check value type at runtime since value type can be boxed to System.Object
                return true;
            }
            return false;
        }


        private static bool IsMethodSignatureMatch(MethodDetail m1, MethodDetail m2)
        {
            if (m1.MethodDef.GenericParameters.Count != m2.MethodDef.GenericParameters.Count)
            {
                return false;
            }
            if (m1.IsStatic != m2.IsStatic)
            {
                return false;
            }
            if (m1.ParamCountIncludeThis != m2.ParamCountIncludeThis)
            {
                return false;
            }
            for (int i = m1.MethodDef.IsStatic ? 0 : 1, n = m1.ParamCountIncludeThis; i < n; i++)
            {
                TypeSig paramType1 = m1.ParamsIncludeThis[i].Type;
                TypeSig paramType2 = m2.ParamsIncludeThis[i].Type;
                if (!TypeEqualityComparer.Instance.Equals(paramType1, paramType2))
                {
                    return false;
                }
            }
            return true;
        }

        private IMethod FindVirtualMethodImplOnKlass(TypeDetail type, MethodDetail method)
        {
            foreach (var methodDef in type.TypeDef.Methods)
            {
                if (!methodDef.IsVirtual)
                {
                    continue;
                }
                if (methodDef.IsNewSlot)
                {
                    // if method is interface method, it should be implemented in the type implements the interface
                    if (!method.DeclaringTypeDef.IsInterface)
                    {
                        continue;
                    }
                }
                GenericArgumentContext gac = type.GAC;
                IMethod inflatedMethod = gac.ContainsGenericArguments ? new MemberRefUser(methodDef.Module, methodDef.Name, methodDef.MethodSig, type.Type) : methodDef;
                MethodDetail inflatedMethodDetail = _metadataService.GetMethodDetail(inflatedMethod);
                if (!IsMethodSignatureMatch(method, inflatedMethodDetail))
                {
                    continue;
                }
                if (method.MethodDef.Name == methodDef.Name)
                {
                    return inflatedMethod;
                }
                // find in TypeDef explicit method overrides
                foreach (var methodOverride in methodDef.Overrides)
                {
                    if (MethodEqualityComparer.CompareDeclaringTypes.Equals(methodOverride.MethodDeclaration, method.Method))
                    {
                        return inflatedMethod;
                    }
                }
            }

            // find in TypeDef explicit method o
            return null;
        }

        private bool IsTypeImplementConstraintedMethod(ITypeDefOrRef type, MethodDetail method, out IMethod implMethod)
        {
            var typeDetail = _metadataService.GetTypeDetail(type);
            if (typeDetail.TypeDef == null)
            {
                implMethod = null;
                return false;
            }
            Debug.Assert(typeDetail.IsValueType);
            implMethod = FindVirtualMethodImplOnKlass(typeDetail, method);
            if (method.DeclaringTypeDef.IsInterface)
            {
                // if a interface method is not implemented in its declaring type, it should be implemented in the type implements the interface
                if (!method.MethodDef.HasBody)
                {
                    Debug.Assert(implMethod != null);
                    return true;
                }
                return implMethod != null;
            }
            else
            {
                Debug.Assert(method.DeclaringType.ToTypeSig().ElementType == ElementType.Object);
                return implMethod != null;
            }
        }

        private void EmitCallVirt(Instruction inst, IMethod method, uint token)
        {
            var methodDetail = _metadataService.GetMethodDetail(method);

            int paramCount = methodDetail.ParamCountIncludeThis;
            bool hasReturnValue = !methodDetail.IsVoidReturn;
            var args = new List<EvalVariable>(_curState.runStackDatas.GetRange(_curState.runStackDatas.Count - paramCount, paramCount));

            if (_curPrefixs.HasFlag(PrefixFlags.Constrained))
            {
                EvalVariable originalThisVar = args[0];
                ConstraintedData constrainedData = (ConstraintedData)_prefixData;
                ITypeDefOrRef constaintedType = constrainedData.ConstrainedType;
                if (MetaUtil.IsValueType(constaintedType.ToTypeSig()))
                {
                    // FIXME: we should emitcall for generic method here
                    if (IsTypeImplementConstraintedMethod(constaintedType, methodDetail, out IMethod implMethod) && implMethod.IsMethodDef)
                    {
                        // for constrained callvirt on value type, we can treat it as direct call since the this pointer is already a pointer to the value type and no null check is needed
                        EmitCall(inst, implMethod, token, true);
                        return;
                    }
                    else
                    {
                        // box the original this pointer to the constrained type
                        var actualThisVar = PushStack(_corlibTypes.Object);
                        RuntimeResolvedVariable constrainedTypeVar = _runtimeResolvedMetadatas.GetTypeVariable(constaintedType);
                        EmitDeclaringAssignOrThrow(inst, actualThisVar, $"{VmFunctionNames.Box}({constrainedTypeVar.GetFullReferenceVariableName()}, {GetEvalVariableExprWithCast(originalThisVar, "void*")})");
                        args[0] = actualThisVar;
                    }
                }
                else
                {
                    var actualThisVar = PushStack(_corlibTypes.Object);
                    _bodyWriter.AddLine($"{GetTypeName(actualThisVar)} {GetEvalVariableName(actualThisVar)} = *({ConstStrings.ObjectPtrTypeName}*){GetEvalVariableName(args[0])};");
                    args[0] = actualThisVar;
                }

                Pop();
            }

            if (methodDetail.IsNotVirtualOrSealed)
            {
                // callvirt can be used to call non-virtual method, in this case we can just treat it as call
                EmitCall(inst, method, token, true);
                return;
            }
            _forwardDeclaration.AddMethodForwardDeclaration(method);

            Pop(paramCount);

            EvalVariable thisVar = args[0];
            EmitCheckNotNull(inst, thisVar);

            var methodVar = _runtimeResolvedMetadatas.GetMethodVariable(method);

            EvalVariable retVar = null;
            if (hasReturnValue)
            {
                retVar = PushStack(methodDetail.RetType);
                _bodyWriter.AddLine($"{GetTypeName(retVar)} {GetEvalVariableName(retVar)};");
            }
            if (TryEmitCallvirIntrinsic(inst, methodDetail, methodVar.GetFullReferenceVariableName(), args, retVar))
            {
                return;
            }

            var finalMethodVar = CreateTempVariable(ConstStrings.MethodInfoPtrTypeName);
            _bodyWriter.AddLine($"{VmFunctionNames.DECLARING_ASSIGN_OR_THROW}({finalMethodVar.TypeName}, {finalMethodVar.Name}, {VmFunctionNames.GetVirtualMethodOnObj}({GetEvalVariableName(thisVar)}, {methodVar.GetFullReferenceVariableName()}), {CurMethodVar.GetFullReferenceVariableName()}, {GetCurrentIpOffset(inst)});");


            _bodyWriter.AddLine($"if ({finalMethodVar.Name}->invoker_type == leanclr::metadata::RtInvokerType::Aot)");
            _bodyWriter.AddLine("{");
            _bodyWriter.IncreaseIndent();
            TypeDef declaringTypeDef = methodDetail.DeclaringTypeDef;
            if (declaringTypeDef != null)
            {
                if (IsParentOrInterfaceOfValueType(declaringTypeDef))
                {
                    _bodyWriter.AddLine($"if ({VmFunctionNames.IsValueType}({GetParentFromFullReferenceMethodVariable(finalMethodVar.Name)}))");
                    _bodyWriter.AddLine("{");
                    _bodyWriter.IncreaseIndent();
                    _bodyWriter.AddLine($"{GetEvalVariableName(thisVar)} += 1;");
                    _bodyWriter.DecreaseIndent();
                    _bodyWriter.AddLine("}");
                }
                else if (declaringTypeDef.IsValueType)
                {
                    _bodyWriter.AddLine($"{GetEvalVariableName(thisVar)} += 1;");
                }
            }
            // declaringTypeDef may be null for Array's get/set methods since we treat them as declared in System.Array which is a type forward to System.Private.CoreLib's Array, and we currently don't have the definition of System.Private.CoreLib's Array, but we can still check if it's interface or not by the metadata of System.Array

            EmitCallByMethodPointerDirectly(inst, methodDetail, finalMethodVar.Name, args, retVar);
            _bodyWriter.DecreaseIndent();
            _bodyWriter.AddLine("}");
            _bodyWriter.AddLine("else");
            _bodyWriter.AddLine("{");
            _bodyWriter.IncreaseIndent();
            EmitCallByInvoker(inst, methodDetail, finalMethodVar.Name, args, retVar, true);
            _bodyWriter.DecreaseIndent();
            _bodyWriter.AddLine("}");
        }

        private string CreateMethodFunctionTypeDefine(MethodSig methodSig)
        {
            StringBuilder sb = new StringBuilder();
            sb.Append(MethodGenerationUtil.GetResultTypeName(methodSig.RetType));
            sb.Append($" (*)(");
            bool first = true;
            foreach (var param in methodSig.Params)
            {
                if (first)
                    first = false;
                else
                {
                    sb.Append(", ");
                }
                sb.Append(MethodGenerationUtil.GetExactTypeName(param));
            }
            sb.Append(')');
            sb.Append(ConstStrings.CppFunctionNoexcept);
            return sb.ToString();
        }

        private void EmitCalli(Instruction inst, MethodSig callSite)
        {
            int paramCount = callSite.Params.Count;
            var args = new List<EvalVariable>(_curState.runStackDatas.GetRange(_curState.runStackDatas.Count - paramCount - 1, paramCount));
            var ftnVar = _curState.runStackDatas.Last();
            Pop(paramCount + 1);

            var argsStr = CreateMethodFunctionArgsWithCast(callSite, args);
            string methodFunctionTypeDefine = CreateMethodFunctionTypeDefine(callSite);
            string methodPointerVarName = GetMethodPointerFromFullReferenceMethodVariable($"({GetEvalVariableExprWithCast(ftnVar, ConstStrings.MethodInfoPtrTypeName)})");
            if (!MetaUtil.IsVoidType(callSite.RetType))
            {
                var retVar = PushStack(callSite.RetType);
                EmitDeclaringAssignOrThrow(inst, retVar, $"(({methodFunctionTypeDefine}){methodPointerVarName})({argsStr})");
            }
            else
            {
                EmitThrowOnError(inst, $"(({methodFunctionTypeDefine}){methodPointerVarName})({argsStr})");
            }
        }

        private void EmitLdftn(Instruction inst, IMethod method)
        {
            _forwardDeclaration.AddMethodForwardDeclaration(method);
            var methodVar = _runtimeResolvedMetadatas.GetMethodVariable(method);
            var retVar = PushStack(_corlibTypes.IntPtr);
            _bodyWriter.AddLine($"{ConstStrings.MethodInfoPtrTypeName} {GetEvalVariableName(retVar)} = {methodVar.GetFullReferenceVariableName()};");
        }

        private void EmitLdvirtftn(Instruction inst, IMethod method)
        {
            _forwardDeclaration.AddMethodForwardDeclaration(method);
            var thisVar = Pop();
            var methodVar = _runtimeResolvedMetadatas.GetMethodVariable(method);
            var retVar = PushStack(_corlibTypes.IntPtr);
            //var finalMethodVar = CreateTempVariable(ConstStrings.MethodInfoPtrTypeName);
            _bodyWriter.AddLine($"{VmFunctionNames.DECLARING_ASSIGN_OR_THROW}({ConstStrings.MethodInfoPtrTypeName}, {GetEvalVariableName(retVar)}, {VmFunctionNames.GetVirtualMethodOnObj}({GetEvalVariableName(thisVar)}, {methodVar.GetFullReferenceVariableName()}), {CurMethodVar.GetFullReferenceVariableName()}, {GetCurrentIpOffset(inst)});");
        }

        private void EmitNewObj(Instruction inst, IMethod method, uint token)
        {
            MethodDetail methodDetail = _metadataService.GetMethodDetail(method);
            _forwardDeclaration.AddMethodForwardDeclaration(method);
            if (TryRedirectNewObjIntrinsic(inst, methodDetail, out MethodDef redirectedMethod))
            {
                EmitCall(inst, redirectedMethod, 0, false);
                return;
            }

            int paramCount = methodDetail.ParamCountIncludeThis - 1;
            bool hasReturnValue = !methodDetail.IsVoidReturn;
            var args = new List<EvalVariable>(_curState.runStackDatas.GetRange(_curState.runStackDatas.Count - paramCount, paramCount));
            Pop(paramCount);

            TypeDetail declaringTypeDetail = _metadataService.GetTypeDetail(methodDetail.DeclaringType);
            EvalVariable retVar = PushStack(declaringTypeDetail.Type);
            var methodVarNameProvider = () => _runtimeResolvedMetadatas.GetMethodVariable(method).GetFullReferenceVariableName();

            if (TryEmitNewobjIntrinsic(inst, methodDetail, methodVarNameProvider, args, retVar))
            {
                return;
            }

            if (declaringTypeDetail.IsValueType)
            {
                _bodyWriter.AddLine($"{GetTypeName(retVar)} {GetEvalVariableName(retVar)} = {{}};"); // default construct the value type
                EvalVariable ptrOfRetVar = PushStack(_corlibTypes.IntPtr);
                _bodyWriter.AddLine($"{GetTypeName(retVar)}* {GetEvalVariableName(ptrOfRetVar)} = &{GetEvalVariableName(retVar)};");
                Pop(); // pop the pointer since it's only used for passing to the constructor, it won't be used after this
                args.Insert(0, ptrOfRetVar); // insert this as the first argument
            }
            else
            {
                args.Insert(0, retVar); // insert this as the first argument
                EmitDeclaringAssignOrThrow(inst, retVar, $"{VmFunctionNames.NewObj}({GetParentFromFullReferenceMethodVariable(methodVarNameProvider())})");
            }
            EmitCallCommon(inst, methodDetail, methodVarNameProvider, args, retVar);
        }

        private void EmitCastClass(Instruction inst, ITypeDefOrRef targetType)
        {
            var srcObj = Pop();
            var dstObj = PushStack(_corlibTypes.Object);
            string srcVarName = GetEvalVariableName(srcObj);
            RuntimeResolvedVariable targetTypeVar = _runtimeResolvedMetadatas.GetTypeVariable(targetType);
            _bodyWriter.AddLine($"if ({srcVarName} != nullptr && {VmFunctionNames.CastClass}({srcVarName}, {targetTypeVar.GetFullReferenceVariableName()}) == nullptr)");
            _bodyWriter.AddLine("{");
            _bodyWriter.IncreaseIndent();
            EmitThrowRuntimeError(inst, "InvalidCast");
            _bodyWriter.DecreaseIndent();
            _bodyWriter.AddLine("}");
            _bodyWriter.AddLine($"{GetTypeName(dstObj)} {GetEvalVariableName(dstObj)} = ({GetTypeName(dstObj)}){srcVarName};");
        }

        private void EmitIsInst(Instruction inst, ITypeDefOrRef targetType)
        {
            var srcObj = Pop();
            var dstObj = PushStack(_corlibTypes.Object);
            string srcVarName = GetEvalVariableName(srcObj);
            RuntimeResolvedVariable targetTypeVar = _runtimeResolvedMetadatas.GetTypeVariable(targetType);
            _bodyWriter.AddLine($"{GetTypeName(dstObj)} {GetEvalVariableName(dstObj)} = {srcVarName} ? {VmFunctionNames.IsInst}(({ConstStrings.ObjectPtrTypeName}){srcVarName}, {targetTypeVar.GetFullReferenceVariableName()}) : nullptr;");
        }

        private void EmitUnbox(Instruction inst, ITypeDefOrRef targetType)
        {
            if (!MetaUtil.IsValueType(targetType.ToTypeSig()))
            {
                throw new Exception($"Target type of unbox must be value type. Method: {_method.FullName}, Target Type: {targetType.FullName}.");
            }
            var srcObj = Pop();
            var dstObj = PushStack(_corlibTypes.IntPtr);
            RuntimeResolvedVariable targetTypeVar = _runtimeResolvedMetadatas.GetTypeVariable(targetType);
            EmitDeclaringAssignOrThrow(inst, dstObj, $"{VmFunctionNames.Unbox}({GetEvalVariableName(srcObj)}, {targetTypeVar.GetFullReferenceVariableName()})");
        }

        private void EmitUnboxAny(Instruction inst, ITypeDefOrRef targetType)
        {
            if (!MetaUtil.IsValueType(targetType.ToTypeSig()))
            {
                EmitCastClass(inst, targetType);
                return;
            }
            var srcObj = Pop();
            var dstObj = PushStack(targetType);
            RuntimeResolvedVariable targetTypeVar = _runtimeResolvedMetadatas.GetTypeVariable(targetType);
            _bodyWriter.AddLine($"{GetTypeName(dstObj)} {GetEvalVariableName(dstObj)};");
            EmitThrowOnError(inst, $"{VmFunctionNames.UnboxAny}({GetEvalVariableName(srcObj)}, {targetTypeVar.GetFullReferenceVariableName()}, &{GetEvalVariableName(dstObj)}, true)");
        }

        private void EmitBox(Instruction inst, ITypeDefOrRef targetType)
        {
            if (!MetaUtil.IsValueType(targetType.ToTypeSig()))
            {
                Pop();
                PushStack(targetType);
                // do nothing since it's already a reference type, just need to change the type on stack
                // EmitCastClass(inst, targetType);
                return;
            }
            var srcObj = Pop();
            var dstObj = PushStack(_corlibTypes.Object);
            RuntimeResolvedVariable targetTypeVar = _runtimeResolvedMetadatas.GetTypeVariable(targetType);
            EmitDeclaringAssignOrThrow(inst, dstObj, $"{VmFunctionNames.Box}({targetTypeVar.GetFullReferenceVariableName()}, &{GetEvalVariableName(srcObj)})");
        }

        private void EmitThrow(Instruction inst)
        {
            var exObj = Pop();
            _bodyWriter.AddLine($"{VmFunctionNames.THROW_EXCEPTION}({GetEvalVariableName(exObj)}, {CurMethodVar.GetFullReferenceVariableName()}, {GetCurrentIpOffset(inst)});");
        }

        private void EmitRethrow(Instruction inst)
        {
            throw new NotSupportedException();
        }

        private void EmitLdfld(Instruction inst, IField field)
        {
            FieldDetail fd = _metadataService.GetFieldDetail(field);
            TypeDef declaringTypeDef = fd.FieldBase.DeclaringType;
            ElementType elementType = declaringTypeDef.ToTypeSig().ElementType;
            switch (elementType)
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
            case ElementType.I:
            case ElementType.U:
            case ElementType.R4:
            case ElementType.R8:
            {
                EmitLoadInd(inst, elementType);
                return;
            }
            default:
            {
                break;
            }
            }

            var objVar = Pop();
            var retVar = PushStack(fd.Type);
            string retTypeName = GetTypeName(retVar);
            string exactFieldTypeName = GetExactTypeName(fd.Type);
            string loadFieldExpr;
            if (objVar.type == EvalDataType.I || objVar.type == EvalDataType.Ref)
            {
                EmitCheckNotNull(inst, objVar);
                if (fd.FieldBase.DeclaringType.IsValueType)
                {
                    loadFieldExpr = $"(({GetExactTypeName(fd.ParentType, true)}*){GetEvalVariableName(objVar)})->{fd.Name}";
                }
                else
                {
                    loadFieldExpr = $"({GetEvalVariableExprWithCast(objVar, GetExactTypeName(fd.ParentType, true))})->{fd.Name}";
                }
            }
            else if (objVar.type == EvalDataType.ValueType)
            {
                loadFieldExpr = $"({GetEvalVariableExprWithCast(objVar, GetExactTypeName(fd.ParentType, true))}).{fd.Name}";
            }
            else
            {
                throw new Exception($"invalid this type for ldfld");
            }
            string valueStr = MayFoldCast(exactFieldTypeName, retTypeName, loadFieldExpr);
            _bodyWriter.AddLine($"{retTypeName} {GetEvalVariableName(retVar)} = {valueStr};");
        }

        private void EmitStfld(Instruction inst, IField field)
        {
            FieldDetail fd = _metadataService.GetFieldDetail(field);
            FieldDef fieldDef = fd.FieldBase;
            TypeDef declaringTypeDef = fd.FieldBase.DeclaringType;
            ElementType elementType = declaringTypeDef.ToTypeSig().ElementType;
            switch (elementType)
            {
            case ElementType.Boolean: EmitStoreInd(inst, ReduceDataType.U1); return;
            case ElementType.Char: EmitStoreInd(inst, ReduceDataType.U2); return;
            case ElementType.I1: EmitStoreInd(inst, ReduceDataType.I1); return;
            case ElementType.U1: EmitStoreInd(inst, ReduceDataType.U1); return;
            case ElementType.I2: EmitStoreInd(inst, ReduceDataType.I2); return;
            case ElementType.U2: EmitStoreInd(inst, ReduceDataType.U2); return;
            case ElementType.I4: EmitStoreInd(inst, ReduceDataType.I4); return;
            case ElementType.U4: EmitStoreInd(inst, ReduceDataType.I4); return;
            case ElementType.I8: EmitStoreInd(inst, ReduceDataType.I8); return;
            case ElementType.U8: EmitStoreInd(inst, ReduceDataType.I8); return;
            case ElementType.I: EmitStoreInd(inst, ReduceDataType.I); return;
            case ElementType.U: EmitStoreInd(inst, ReduceDataType.I); return;
            case ElementType.R4: EmitStoreInd(inst, ReduceDataType.R4); return;
            case ElementType.R8: EmitStoreInd(inst, ReduceDataType.R8); return;
            default: break;
            }

            var valueVar = Pop();
            var objVar = Pop();
            string valueStr = GetEvalVariableExprWithCast(valueVar, GetExactTypeName(fd.Type));
            if (objVar.type == EvalDataType.I || objVar.type == EvalDataType.Ref)
            {
                EmitCheckNotNull(inst, objVar);
                if (fieldDef.DeclaringType.IsValueType)
                {
                    _bodyWriter.AddLine($"(({GetExactTypeName(fd.ParentType, true)}*){GetEvalVariableName(objVar)})->{fd.Name} = {valueStr};");
                }
                else
                {
                    _bodyWriter.AddLine($"({GetEvalVariableExprWithCast(objVar, GetExactTypeName(fd.ParentType, true))})->{fd.Name} = {valueStr};");
                }
            }
            else if (objVar.type == EvalDataType.ValueType)
            {
                _bodyWriter.AddLine($"({GetEvalVariableExprWithCast(objVar, GetExactTypeName(fd.ParentType, true))}).{fd.Name} = {valueStr};");
            }
            else
            {
                throw new Exception($"invalid this type for stfld");
            }
        }

        private string GetConstantValueExpr(Constant value)
        {
            if (value.Type != ElementType.String)
            {
                return value.Value.ToString();
            }
            else
            {
                throw new NotSupportedException($"String literal is not supported as constant field value. Field: {value.Value}.");
            }
        }

        private void EmitLdsfld(Instruction inst, IField field)
        {
            FieldDetail fd = _metadataService.GetFieldDetail(field);
            FieldDef fieldDef = fd.FieldBase;
            var retVar = PushStack(fd.Type);
            if (fieldDef.HasConstant)
            {
                _bodyWriter.AddLine($"{GetTypeName(retVar)} {GetEvalVariableName(retVar)} = {GetConstantValueExpr(fieldDef.Constant)};");
                return;
            }
            else if (fieldDef.HasFieldRVA)
            {
                throw new NotSupportedException($"Static field with RVA is not supported. Field: {field.FullName} in method: {_method.FullName}.");
            }
            RuntimeResolvedVariable fieldVar = _runtimeResolvedMetadatas.GetFieldVariable(field);
            string klassVarName = GetParentFromFullReferenceFieldVariable(fieldVar.GetFullReferenceVariableName());
            EmitRunClassStaticConstructor(inst, klassVarName);
            string retTypeName = GetTypeName(retVar);
            string exactFieldTypeName = GetExactTypeName(fd.Type);
            string loadFieldExpr = $"(({fd.Parent.StaticTypeName}*)({klassVarName}->{ConstStrings.KlassFieldNameStaticFieldsData}))->{fd.Name}";
            _bodyWriter.AddLine($"{retTypeName} {GetEvalVariableName(retVar)} = {MayFoldCast(exactFieldTypeName, retTypeName, loadFieldExpr)};");
        }

        private void EmitStsfld(Instruction inst, IField field, uint token)
        {
            var valueVar = Pop();
            FieldDetail fd = _metadataService.GetFieldDetail(field);
            FieldDef fieldDef = fd.FieldBase;
            if (fieldDef.HasConstant || fieldDef.HasFieldRVA)
            {
                throw new NotSupportedException($"Static field with const or RVA is not supported. Field: {field.FullName} in method: {_method.FullName}.");
            }
            RuntimeResolvedVariable fieldVar = _runtimeResolvedMetadatas.GetFieldVariable(field);
            string klassVarName = GetParentFromFullReferenceFieldVariable(fieldVar.GetFullReferenceVariableName());
            EmitRunClassStaticConstructor(inst, klassVarName);
            string exactFieldTypeName = GetExactTypeName(fd.Type);
            _bodyWriter.AddLine($"(({fd.Parent.StaticTypeName}*)({klassVarName}->{ConstStrings.KlassFieldNameStaticFieldsData}))->{fd.Name} = {GetEvalVariableExprWithCast(valueVar, exactFieldTypeName)};");
        }

        private void EmitLdflda(Instruction inst, IField field, uint token)
        {
            var objVar = Pop();
            var retVar = PushStack(EvalDataType.I);

            FieldDetail fd = _metadataService.GetFieldDetail(field);
            string retTypeName = GetTypeName(retVar);
            string loadFieldExpr;
            if (objVar.type == EvalDataType.I || objVar.type == EvalDataType.Ref)
            {
                if (fd.FieldBase.DeclaringType.IsValueType)
                {
                    loadFieldExpr = $"&(({GetExactTypeName(fd.ParentType, true)}*){GetEvalVariableName(objVar)})->{fd.Name}";
                }
                else
                {
                    loadFieldExpr = $"&({GetEvalVariableExprWithCast(objVar, GetExactTypeName(fd.ParentType, true))})->{fd.Name}";
                }
            }
            else if (objVar.type == EvalDataType.ValueType)
            {
                loadFieldExpr = $"&({GetEvalVariableExprWithCast(objVar, GetExactTypeName(fd.ParentType, true))}).{fd.Name}";
            }
            else
            {
                throw new Exception($"invalid this type for ldflda");
            }
            _bodyWriter.AddLine($"{retTypeName} {GetEvalVariableName(retVar)} = ({retTypeName})({loadFieldExpr});");
        }

        private void EmitLdsflda(Instruction inst, IField field, uint token)
        {
            var retVar = PushStack(EvalDataType.I);

            FieldDetail fd = _metadataService.GetFieldDetail(field);
            RuntimeResolvedVariable fieldVar = _runtimeResolvedMetadatas.GetFieldVariable(field);
            string retTypeName = GetTypeName(retVar);
            FieldDef fieldDef = fd.FieldBase;
            if (fieldDef.HasConstant)
            {
                throw new NotSupportedException($"ldsflda: static field with const is not supported. Field: {field.FullName} in method: {_method.FullName}.");
            }
            if (fieldDef.HasFieldRVA)
            {
                EmitDeclaringAssignOrThrow(inst, retVar, $"{VmFunctionNames.GetFieldRvaData}({fieldVar.GetFullReferenceVariableName()})");
            }
            else
            {
                string klassVarName = GetParentFromFullReferenceFieldVariable(fieldVar.GetFullReferenceVariableName());
                EmitRunClassStaticConstructor(inst, klassVarName);
                string loadFieldExpr = $"&(({fd.Parent.StaticTypeName}*)({klassVarName}->{ConstStrings.KlassFieldNameStaticFieldsData}))->{fd.Name}";
                _bodyWriter.AddLine($"{retTypeName} {GetEvalVariableName(retVar)} = ({retTypeName})({loadFieldExpr});");
            }
        }

        private void EmitCheckArrayIndexOutOfRange(Instruction inst, EvalVariable objVar, EvalVariable indexVar)
        {
            string arrVarExpr = GetVariableMayCast(objVar, ConstStrings.ArrayPtrTypeName);
            string indexVarExpr = GetVariableMayCast(indexVar, "int32_t");
            _bodyWriter.AddLine($"if ({VmFunctionNames.IsArrayIndexOutOfRange}({arrVarExpr}, {indexVarExpr}))");
            _bodyWriter.AddLine("{");
            _bodyWriter.IncreaseIndent();
            EmitThrowRuntimeError(inst, "IndexOutOfRange");
            _bodyWriter.DecreaseIndent();
            _bodyWriter.AddLine("}");
        }

        private void EmitNewarr(Instruction inst, ITypeDefOrRef operand)
        {
            var indexVar = Pop();
            var retVar = PushStack(new SZArraySig(operand.ToTypeSig()));
            RuntimeResolvedVariable elementKlassVar = _runtimeResolvedMetadatas.GetTypeVariable(operand);
            string indexVarExpr = GetVariableMayCast(indexVar, "int32_t");
            EmitDeclaringAssignOrThrow(inst, retVar, $"{VmFunctionNames.NewSZArrayFromEleKlass}({elementKlassVar.GetFullReferenceVariableName()}, {indexVarExpr})");
        }

        private void EmitLdlen(Instruction inst)
        {
            var arrayVar = Pop();
            var retVar = PushStack(EvalDataType.Int32);
            EmitCheckNotNull(inst, arrayVar);
            _bodyWriter.AddLine($"{GetTypeName(retVar)} {GetEvalVariableName(retVar)} = {VmFunctionNames.GetArrayLength}({GetVariableMayCast(arrayVar, ConstStrings.ArrayPtrTypeName)});");
        }

        private void EmitLdelema(Instruction inst, ITypeDefOrRef elementType, uint token)
        {
            var indexVar = Pop();
            var arrayVar = Pop();
            var retVar = PushStack(EvalDataType.I);
            EmitCheckNotNull(inst, arrayVar);
            EmitCheckArrayIndexOutOfRange(inst, arrayVar, indexVar);
            string retTypeName = GetTypeName(retVar);
            string elementTypeName = GetExactTypeName(elementType);
            EvalDataType evalDataType = GetEvalDataType(elementType.ToTypeSig());
            switch (evalDataType)
            {
            case EvalDataType.Ref:
            case EvalDataType.ValueType:
            {
                if (evalDataType == EvalDataType.Ref)
                {
                    RuntimeResolvedVariable elementKlassVar = _runtimeResolvedMetadatas.GetTypeVariable(elementType);
                    _bodyWriter.AddLine($"if (!{VmFunctionNames.IsPointerElementCompatibleWith}({VmFunctionNames.GetArrayElementKlass}({GetEvalVariableExprWithCast(arrayVar, ConstStrings.ArrayPtrTypeName)}), {elementKlassVar.GetFullReferenceVariableName()}))");
                    _bodyWriter.AddLine("{");
                    _bodyWriter.IncreaseIndent();
                    EmitThrowRuntimeError(inst, "ArrayTypeMismatch");
                    _bodyWriter.DecreaseIndent();
                    _bodyWriter.AddLine("}");
                }
                string getElementAddressExpr = $"{VmFunctionNames.GetArrayElementAddress}<{elementTypeName}>({GetVariableMayCast(arrayVar, ConstStrings.ArrayPtrTypeName)}, {GetVariableMayCast(indexVar, "int32_t")})";
                _bodyWriter.AddLine($"{retTypeName} {GetEvalVariableName(retVar)} = {MayFoldCast($"{elementTypeName}*", retTypeName, getElementAddressExpr)};");
                break;
            }
            default:
            {
                string getElementAddressExpr = $"{VmFunctionNames.GetArrayElementAddress}<{elementTypeName}>({GetVariableMayCast(arrayVar, ConstStrings.ArrayPtrTypeName)}, {GetVariableMayCast(indexVar, "int32_t")})";
                _bodyWriter.AddLine($"{retTypeName} {GetEvalVariableName(retVar)} = {MayFoldCast($"{elementTypeName}*", retTypeName, getElementAddressExpr)};");
                break;
            }
            }
        }

        private void EmitLdelem(Instruction inst, TypeSig elementType)
        {
            var indexVar = Pop();
            var arrayVar = Pop();
            var retVar = PushStack(elementType);
            EmitCheckNotNull(inst, arrayVar);
            EmitCheckArrayIndexOutOfRange(inst, arrayVar, indexVar);
            string elementTypeName = GetExactTypeName(elementType);
            string retTypeName = GetTypeName(retVar);
            string getElementExpr = $"{VmFunctionNames.GetArrayElementDataAt}<{elementTypeName}>({GetVariableMayCast(arrayVar, ConstStrings.ArrayPtrTypeName)}, {GetVariableMayCast(indexVar, "int32_t")})";
            _bodyWriter.AddLine($"{retTypeName} {GetEvalVariableName(retVar)} = {MayFoldCast(elementTypeName, retTypeName, getElementExpr)};");
        }

        private void EmitStelem(Instruction inst, TypeSig elementType)
        {
            var valueVar = Pop();
            var indexVar = Pop();
            var arrayVar = Pop();
            EmitCheckNotNull(inst, arrayVar);
            EmitCheckArrayIndexOutOfRange(inst, arrayVar, indexVar);
            string elementTypeName = GetExactTypeName(elementType);
            _bodyWriter.AddLine($"{VmFunctionNames.SetArrayElementDataAt}<{elementTypeName}>({GetVariableMayCast(arrayVar, ConstStrings.ArrayPtrTypeName)}, {GetVariableMayCast(indexVar, "int32_t")}, {GetVariableMayCast(valueVar, elementTypeName)});");
        }

        //private void EmitLdelemRef(Instruction inst)
        //{
        //    TypeSig elementType = _corlibTypes.Object;
        //    var indexVar = Pop();
        //    var arrayVar = Pop();
        //    var retVar = PushStack(elementType);
        //    EmitCheckNotNull(inst, arrayVar);
        //    EmitCheckArrayIndexOutOfRange(inst, arrayVar, indexVar);
        //    string elementTypeName = GetExactTypeName(elementType);
        //    string retTypeName = GetTypeName(retVar);
        //    string getElementAddressExpr = $"{VmFunctionNames.GetArrayElementDataAt}<{elementTypeName}>({GetVariableMayCast(arrayVar, ConstStrings.ArrayPtrTypeName)}, {GetVariableMayCast(indexVar, "int32_t")})";
        //    _bodyWriter.AddLine($"{retTypeName} {GetEvalVariableName(retVar)} = {MayFoldCast(elementTypeName, retTypeName, getElementAddressExpr)}));");
        //}

        private void EmitStelemRef(Instruction inst)
        {
            var valueVar = Pop();
            var indexVar = Pop();
            var arrayVar = Pop();
            EmitCheckNotNull(inst, arrayVar);
            EmitCheckArrayIndexOutOfRange(inst, arrayVar, indexVar);

            string valueVarName = GetEvalVariableName(valueVar);
            _bodyWriter.AddLine($"if ({valueVarName} != nullptr && !{VmFunctionNames.IsAssignableFrom}({valueVarName}->klass, {VmFunctionNames.GetArrayElementKlass}({GetEvalVariableExprWithCast(arrayVar, ConstStrings.ArrayPtrTypeName)})))");
            _bodyWriter.AddLine("{");
            _bodyWriter.IncreaseIndent();
            EmitThrowRuntimeError(inst, "ArrayTypeMismatch");
            _bodyWriter.DecreaseIndent();
            _bodyWriter.AddLine("}");

            string elementTypeName = ConstStrings.ObjectPtrTypeName;
            _bodyWriter.AddLine($"{VmFunctionNames.SetArrayElementDataAt}<{elementTypeName}>({GetVariableMayCast(arrayVar, ConstStrings.ArrayPtrTypeName)}, {GetVariableMayCast(indexVar, "int32_t")}, {GetVariableMayCast(valueVar, elementTypeName)});");
        }

        private void EmitLdelemAny(Instruction inst, ITypeDefOrRef elementType, uint token)
        {
            EvalDataType evalDataType = GetEvalDataType(elementType.ToTypeSig());
            if (evalDataType != EvalDataType.Ref && evalDataType != EvalDataType.ValueType)
            {
                EmitLdelem(inst, elementType.ToTypeSig());
                return;
            }
            var indexVar = Pop();
            var arrayVar = Pop();
            var retVar = PushStack(elementType);
            EmitCheckNotNull(inst, arrayVar);
            EmitCheckArrayIndexOutOfRange(inst, arrayVar, indexVar);

            string elementTypeName = GetExactTypeName(elementType);
            string retTypeName = GetTypeName(retVar);
            string getElementExpr = $"{VmFunctionNames.GetArrayElementDataAt}<{elementTypeName}>({GetVariableMayCast(arrayVar, ConstStrings.ArrayPtrTypeName)}, {GetVariableMayCast(indexVar, "int32_t")})";
            _bodyWriter.AddLine($"{retTypeName} {GetEvalVariableName(retVar)} = {MayFoldCast(elementTypeName, retTypeName, getElementExpr)};");
        }

        private void EmitStelemAny(Instruction inst, ITypeDefOrRef elementType, uint token)
        {
            EvalDataType evalDataType = GetEvalDataType(elementType.ToTypeSig());
            if (evalDataType != EvalDataType.Ref && evalDataType != EvalDataType.ValueType)
            {
                EmitStelem(inst, elementType.ToTypeSig());
                return;
            }

            var valueVar = Pop();
            var indexVar = Pop();
            var arrayVar = Pop();
            EmitCheckNotNull(inst, arrayVar);
            EmitCheckArrayIndexOutOfRange(inst, arrayVar, indexVar);

            if (evalDataType == EvalDataType.Ref)
            {
                string valueVarName = GetEvalVariableName(valueVar);
                RuntimeResolvedVariable elementKlassVar = _runtimeResolvedMetadatas.GetTypeVariable(elementType);
                _bodyWriter.AddLine($"if ({valueVarName} != nullptr && !{VmFunctionNames.IsAssignableFrom}({valueVarName}->klass, {VmFunctionNames.GetArrayElementKlass}({GetEvalVariableExprWithCast(arrayVar, ConstStrings.ArrayPtrTypeName)})))");
                _bodyWriter.AddLine("{");
                _bodyWriter.IncreaseIndent();
                EmitThrowRuntimeError(inst, "ArrayTypeMismatch");
                _bodyWriter.DecreaseIndent();
                _bodyWriter.AddLine("}");
            }

            string elementTypeName = GetExactTypeName(elementType);
            _bodyWriter.AddLine($"{VmFunctionNames.SetArrayElementDataAt}<{elementTypeName}>({GetVariableMayCast(arrayVar, ConstStrings.ArrayPtrTypeName)}, {GetVariableMayCast(indexVar, "int32_t")}, {GetVariableMayCast(valueVar, elementTypeName)});");
        }

        private void EmitSizeOf(Instruction inst, ITypeDefOrRef type)
        {
            var retVar = PushStack(EvalDataType.Int32);
            string subObjectHeaderSize = MetaUtil.IsValueType(type.ToTypeSig()) ? "" : $"- sizeof({ConstStrings.ObjectTypeName})";
            _bodyWriter.AddLine($"{GetTypeName(retVar)} {GetEvalVariableName(retVar)} = sizeof({TypeNameService.GetDirectlyCppTypeName(type.ToTypeSig())}){subObjectHeaderSize};");
        }

        private void EmitCkfinite(Instruction inst)
        {
            var valueVar = Peek();
            if (valueVar.type != EvalDataType.Float && valueVar.type != EvalDataType.Double)
            {
                throw new Exception($"ckfinite can only be applied to float or double. Method: {_method.FullName}, Value Type: {valueVar.type}.");
            }
            _bodyWriter.AddLine($"if (!{VmFunctionNames.IsFinite}({GetEvalVariableName(valueVar)}))");
            _bodyWriter.AddLine("{");
            _bodyWriter.IncreaseIndent();
            EmitThrowRuntimeError(inst, "Arithmetic");
            _bodyWriter.DecreaseIndent();
            _bodyWriter.AddLine("}");
        }

        private void EmitCpblk(Instruction inst)
        {
            var countVar = Pop();
            var srcVar = Pop();
            var destVar = Pop();
            EmitCheckNotNull(inst, destVar);
            EmitCheckNotNull(inst, srcVar);
            _bodyWriter.AddLine($"{VmFunctionNames.Memcpy}({GetEvalVariableExprWithCast(destVar, "void*")}, {GetEvalVariableExprWithCast(srcVar, "const void*")}, {GetEvalVariableExprWithCast(countVar, "size_t")});");
        }

        private void EmitInitblk(Instruction inst)
        {
            var countVar = Pop();
            var valueVar = Pop();
            var addrVar = Pop();
            EmitCheckNotNull(inst, addrVar);
            _bodyWriter.AddLine($"{VmFunctionNames.Memset}({GetEvalVariableExprWithCast(addrVar, "void*")}, {GetEvalVariableExprWithCast(valueVar, "uint8_t")}, {GetEvalVariableExprWithCast(countVar, "size_t")});");
        }

        private void EmitLdToken(Instruction inst, object operand, uint token)
        {
            var retVar = PushStack(_corlibTypes.IntPtr);
            if (operand is ITypeDefOrRef type)
            {
                RuntimeResolvedVariable typeVar = _runtimeResolvedMetadatas.GetTypeVariable(type);
                _bodyWriter.AddLine($"{ConstStrings.TypeSigPtrTypeName} {GetEvalVariableName(retVar)} = {typeVar.GetFullReferenceVariableName()}->by_val;");
            }
            else if (operand is IMethod method && method.IsMethod)
            {
                RuntimeResolvedVariable methodVar = _runtimeResolvedMetadatas.GetMethodVariable(method);
                _bodyWriter.AddLine($"{ConstStrings.MethodInfoPtrTypeName} {GetEvalVariableName(retVar)} = {methodVar.GetFullReferenceVariableName()};");
            }
            else if (operand is IField field && field.IsField)
            {
                RuntimeResolvedVariable fieldVar = _runtimeResolvedMetadatas.GetFieldVariable(field);
                _bodyWriter.AddLine($"{ConstStrings.FieldInfoPtrTypeName} {GetEvalVariableName(retVar)} = {fieldVar.GetFullReferenceVariableName()};");
            }
            else
            {
                throw new Exception($"invalid operand for ldtoken");
            }
        }

        private void EmitLocalloc(Instruction inst)
        {
            var sizeVar = Pop();
            var retVar = PushStack(EvalDataType.I);
            _bodyWriter.AddLine($"{GetTypeName(retVar)} {GetEvalVariableName(retVar)} = ({GetTypeName(retVar)}){VmFunctionNames.Localloc}({GetEvalVariableExprWithCast(sizeVar, "size_t")});");
            if (InitLocals)
            {
                _bodyWriter.AddLine($"{VmFunctionNames.Memset}({GetEvalVariableExprWithCast(retVar, "void*")}, 0, {GetEvalVariableExprWithCast(sizeVar, "size_t")});");
            }
        }

        private void EmitArglist(Instruction inst)
        {
            throw new NotImplementedException("arglist is not implemented");
        }

        private void EmitEndFilter(Instruction inst)
        {
            throw new NotImplementedException("endfilter is not implemented");
        }

        private void EmitLeave(Instruction inst, Instruction operand)
        {
            throw new NotImplementedException("leave is not implemented");
        }

        private void EmitEndfinally(Instruction inst)
        {
            throw new NotImplementedException("endfinally is not implemented");
        }

        private void EmitRefanyval(Instruction inst, ITypeDefOrRef inflatedType, uint inlineToken)
        {
            EvalVariable typedRefVar = Pop();
            var retVar = PushStack(EvalDataType.I);
            string typeRefVarName = GetEvalVariableName(typedRefVar);
            RuntimeResolvedVariable typeVar = _runtimeResolvedMetadatas.GetTypeVariable(inflatedType);
            if (MetaUtil.IsValueType(inflatedType.ToTypeSig()))
            {
                _bodyWriter.AddLine($"if ({typeRefVarName}.klass != {typeVar.GetFullReferenceVariableName()})");
            }
            else
            {
                _bodyWriter.AddLine($"if ( !{VmFunctionNames.IsAssignableFrom}({typeRefVarName}.klass, {typeVar.GetFullReferenceVariableName()}))");
            }
            _bodyWriter.AddLine("{");
            _bodyWriter.IncreaseIndent();
            EmitThrowRuntimeError(inst, "InvalidCast");
            _bodyWriter.DecreaseIndent();
            _bodyWriter.AddLine("}");
            string retTypeName = GetTypeName(retVar);
            _bodyWriter.AddLine($"{retTypeName} {GetEvalVariableName(retVar)} = ({retTypeName}){typeRefVarName}.value;");
        }

        private void EmitRefanytype(Instruction inst)
        {
            EvalVariable typedRefVar = Pop();
            var retVar = PushStack(_corlibTypes.IntPtr);
            string retTypeName = GetTypeName(retVar);
            _bodyWriter.AddLine($"{retTypeName} {GetEvalVariableName(retVar)} = ({retTypeName})({GetEvalVariableName(typedRefVar)}.type_handle);");
        }

        private void EmitMkrefany(Instruction inst, ITypeDefOrRef inflatedType, uint inlineToken)
        {
            EvalVariable addrVar = Pop();
            var retVar = PushStack(_corlibTypes.TypedReference);
            RuntimeResolvedVariable typeVar = _runtimeResolvedMetadatas.GetTypeVariable(inflatedType);
            string retVarName = GetEvalVariableName(retVar);
            _bodyWriter.AddLine($"{ConstStrings.TypedByRefTypeName} {retVarName};");
            _bodyWriter.AddLine($"{retVarName}.type_handle = {typeVar.GetFullReferenceVariableName()}->by_val;");
            _bodyWriter.AddLine($"{retVarName}.klass = {typeVar.GetFullReferenceVariableName()};");
            _bodyWriter.AddLine($"{retVarName}.value = {GetEvalVariableExprWithCast(addrVar, "void*")};");
        }

        [Flags]
        enum PrefixFlags
        {
            None = 0,
            Unaligned = 0x1,
            Volatile = 0x2,
            Tailcall = 0x4,
            No = 0x8,
            Readonly = 0x10,
            Constrained = 0x20,
        }

        private BasicBlock _curBb;
        private EvalStackState _curState;
        private Dictionary<Instruction, BasicBlock> _instToBbMap;

        private PrefixFlags _curPrefixs;
        private object _prefixData;
        private bool _dontCleanPrefixInBeforeNextInstruction;

        private void SetPrefixFlags(PrefixFlags flags)
        {
            _curPrefixs |= flags;
            _dontCleanPrefixInBeforeNextInstruction = true;
        }

        private void SetPrefixUnalignmentData(int alignment)
        {
            _curPrefixs |= PrefixFlags.Unaligned;
            _prefixData = alignment;
            _dontCleanPrefixInBeforeNextInstruction = true;
        }

        class ConstraintedData
        {
            public ITypeDefOrRef ConstrainedType { get; }
            public uint InlineToken { get; }

            public ConstraintedData(ITypeDefOrRef constrainedType, uint inlineToken)
            {
                ConstrainedType = constrainedType;
                InlineToken = inlineToken;
            }
        }

        private void SetPrefixConstrainedData(ITypeDefOrRef constrainedType, uint token)
        {
            _curPrefixs |= PrefixFlags.Constrained;
            _prefixData = new ConstraintedData(constrainedType, token);
            _dontCleanPrefixInBeforeNextInstruction = true;
        }

        void WriteMethodBody()
        {
            _bodyWriter.SetIndent(1);
            MethodDef methodDef = _method.MethodDef;
            var body = methodDef.Body;
            var insts = body.Instructions;

            var basicBlockCollection = new BasicBlockCollection(body);
            _blockEvalStackStates = basicBlockCollection.Blocks.ToDictionary(b => b, b => new EvalStackState());
            _instToBbMap = new Dictionary<Instruction, BasicBlock>();
            foreach (var bb in basicBlockCollection.Blocks)
            {
                foreach (var inst in bb.instructions)
                {
                    _instToBbMap[inst] = bb;
                }
            }

            bool methodHasReturnValue = !_method.IsVoidReturn;
            foreach (BasicBlock block in basicBlockCollection.Blocks)
            {
                _bodyWriter.AddLine($"// Basic Block begin. IL: 0X{block.StartOffset.ToString("X8")} - 0X{(block.nextBlock != null ? block.nextBlock.StartOffset.ToString("X8") : " End")}");

                EvalStackState state = _blockEvalStackStates[block];
                if (state.visited)
                    continue;
                _curBb = block;
                _curState = state;
                EmitBasicBlockLabel(block);
                _bodyWriter.AddLine("{");
                _bodyWriter.IncreaseIndent();
                state.visited = true;

                var runStackDatas = state.runStackDatas;
                runStackDatas.AddRange(state.inputStackDatas);
                foreach (var inst in block.instructions)
                {
                    if (_dontCleanPrefixInBeforeNextInstruction)
                    {
                        _dontCleanPrefixInBeforeNextInstruction = false;
                    }
                    else
                    {
                        _curPrefixs = PrefixFlags.None;
                        _prefixData = null;
                    }

                    IField inflatedField = null;
                    IMethod inflatedMethod = null;
                    ITypeDefOrRef inflatedType = null;
                    MethodSig inflatedMethodSig = null;
                    object inflatedTokenOperand = null;
                    uint inlineToken = 0;
                    switch (inst.OpCode.OperandType)
                    {
                    case OperandType.InlineField:
                    {
                        var field = _method.InflateField((IField)inst.Operand);
                        inflatedField = field;
                        inlineToken = field.MDToken.ToUInt32();
                        _forwardDeclaration.AddFieldForwardDeclaration(field);
                        break;
                    }
                    case OperandType.InlineMethod:
                    {
                        var method = _method.InflateMethod((IMethod)inst.Operand);
                        inflatedMethod = method;
                        inlineToken = method.MDToken.ToUInt32();
                        _forwardDeclaration.AddMethodForwardDeclaration(method);
                        break;
                    }
                    case OperandType.InlineSig:
                    {
                        var methodSig = _method.InflateMethodSig((MethodSig)inst.Operand);
                        inflatedMethodSig = methodSig;
                        break;
                    }
                    case OperandType.InlineType:
                    {
                        var type = _method.InflateType((ITypeDefOrRef)inst.Operand);
                        inflatedType = type;
                        inlineToken = type.MDToken.ToUInt32();
                        _forwardDeclaration.AddTypeForwardDefine(type);
                        break;
                    }
                    case OperandType.InlineTok:
                    {
                        if (inst.Operand is IField fieldOp && fieldOp.IsField)
                        {
                            var field = _method.InflateField(fieldOp);
                            inflatedTokenOperand = field;
                            inlineToken = field.MDToken.ToUInt32();
                            _forwardDeclaration.AddFieldForwardDeclaration(field);
                        }
                        else if (inst.Operand is IMethod methodOp && methodOp.IsMethod)
                        {
                            var method = _method.InflateMethod(methodOp);
                            inflatedTokenOperand = method;
                            inlineToken = method.MDToken.ToUInt32();
                            _forwardDeclaration.AddMethodForwardDeclaration(method);
                        }
                        else if (inst.Operand is ITypeDefOrRef typeOp && typeOp.IsType)
                        {
                            var type = _method.InflateType(typeOp);
                            inflatedTokenOperand = type;
                            inlineToken = type.MDToken.ToUInt32();
                            _forwardDeclaration.AddTypeForwardDefine(type);
                        }
                        else
                        {
                            throw new Exception($"invalid operand for inlinetok");
                        }
                        break;
                    }
                    }
                    string instComment = Encoding.ASCII.GetString(Encoding.ASCII.GetBytes(inst.ToString()));
                    _bodyWriter.AddLine($"// {instComment}");
                    switch (inst.OpCode.Code)
                    {
                    case Code.Nop: break;
                    case Code.Break: break;
                    case Code.Ldarg_0:
                    case Code.Ldarg_1:
                    case Code.Ldarg_2:
                    case Code.Ldarg_3:
                    case Code.Ldarg_S:
                    case Code.Ldarg:
                    {
                        EmitLoadArg(inst, _parameterVariables[inst.GetParameter(methodDef.Parameters).Index]);
                        break;
                    }
                    case Code.Ldarga:
                    case Code.Ldarga_S:
                    {
                        EmitLoadArgAddress(inst, _parameterVariables[inst.GetParameter(methodDef.Parameters).Index]);
                        break;
                    }
                    case Code.Ldloc_0:
                    case Code.Ldloc_1:
                    case Code.Ldloc_2:
                    case Code.Ldloc_3:
                    case Code.Ldloc:
                    case Code.Ldloc_S:
                    {
                        EmitLoadLocal(inst, _localVariables[inst.GetLocal(body.Variables).Index]);
                        break;
                    }
                    case Code.Ldloca:
                    case Code.Ldloca_S:
                    {
                        EmitLoadLocalAddress(inst, _localVariables[inst.GetLocal(body.Variables).Index]);
                        break;
                    }
                    case Code.Stloc_0:
                    case Code.Stloc_1:
                    case Code.Stloc_2:
                    case Code.Stloc_3:
                    case Code.Stloc:
                    case Code.Stloc_S:
                    {
                        EmitStoreLocal(inst, _localVariables[inst.GetLocal(body.Variables).Index]);
                        break;
                    }
                    case Code.Starg:
                    case Code.Starg_S:
                    {
                        EmitStoreArg(inst, _parameterVariables[inst.GetParameter(methodDef.Parameters).Index]);
                        break;
                    }
                    case Code.Ldnull:
                    {
                        EmitLoadNull(inst);
                        break;
                    }
                    case Code.Ldc_I4_M1:
                    case Code.Ldc_I4_0:
                    case Code.Ldc_I4_1:
                    case Code.Ldc_I4_2:
                    case Code.Ldc_I4_3:
                    case Code.Ldc_I4_4:
                    case Code.Ldc_I4_5:
                    case Code.Ldc_I4_6:
                    case Code.Ldc_I4_7:
                    case Code.Ldc_I4_8:
                    case Code.Ldc_I4:
                    case Code.Ldc_I4_S:
                    {
                        EmitLoadInt32(inst, inst.GetLdcI4Value());
                        break;
                    }
                    case Code.Ldc_I8:
                    {
                        EmitLoadInt64(inst, (long)inst.Operand);
                        break;
                    }
                    case Code.Ldc_R4:
                    {
                        EmitLoadFloat(inst, (float)inst.Operand);
                        break;
                    }
                    case Code.Ldc_R8:
                    {
                        EmitLoadDouble(inst, (double)inst.Operand);
                        break;
                    }
                    case Code.Dup:
                    {
                        EmitDup(runStackDatas.Last());
                        break;
                    }
                    case Code.Pop:
                    {
                        runStackDatas.RemoveAt(runStackDatas.Count - 1);
                        break;
                    }
                    case Code.Jmp:
                    {
                        throw new NotSupportedException($"Jmp instruction is not supported in method: {_method.FullName}.");
                    }
                    case Code.Call:
                    {
                        EmitCall(inst, inflatedMethod, inlineToken, false);
                        break;
                    }
                    case Code.Callvirt:
                    {
                        EmitCallVirt(inst, inflatedMethod, inlineToken);
                        break;
                    }
                    case Code.Calli:
                    {
                        EmitCalli(inst, inflatedMethodSig);
                        break;
                    }
                    case Code.Ret:
                    {
                        EmitRet(methodHasReturnValue ? Pop() : null);
                        break;
                    }
                    case Code.Br:
                    case Code.Br_S:
                    {
                        EmitBranchUnconditional(inst, inst.Operand as Instruction);
                        break;
                    }
                    case Code.Brfalse:
                    case Code.Brfalse_S:
                    {
                        EmitBranchTrueOrFalse(inst, inst.Operand as Instruction, branchIfTrue: false);
                        break;
                    }
                    case Code.Brtrue:
                    case Code.Brtrue_S:
                    {
                        EmitBranchTrueOrFalse(inst, inst.Operand as Instruction, branchIfTrue: true);
                        break;
                    }
                    case Code.Beq:
                    case Code.Beq_S:
                    {
                        EmitBranchComparison(inst, inst.Operand as Instruction, "==");
                        break;
                    }
                    case Code.Bge:
                    case Code.Bge_S:
                    {
                        EmitBranchComparison(inst, inst.Operand as Instruction, ">=");
                        break;
                    }
                    case Code.Bge_Un:
                    case Code.Bge_Un_S:
                    {
                        EmitBranchComparison(inst, inst.Operand as Instruction, ">=", isUnOrdered: true);
                        break;
                    }
                    case Code.Bgt:
                    case Code.Bgt_S:
                    {
                        EmitBranchComparison(inst, inst.Operand as Instruction, ">");
                        break;
                    }
                    case Code.Bgt_Un:
                    case Code.Bgt_Un_S:
                    {
                        EmitBranchComparison(inst, inst.Operand as Instruction, ">", isUnOrdered: true);
                        break;
                    }
                    case Code.Ble:
                    case Code.Ble_S:
                    {
                        EmitBranchComparison(inst, inst.Operand as Instruction, "<=");
                        break;
                    }
                    case Code.Ble_Un:
                    case Code.Ble_Un_S:
                    {
                        EmitBranchComparison(inst, inst.Operand as Instruction, "<=", isUnOrdered: true);
                        break;
                    }
                    case Code.Blt:
                    case Code.Blt_S:
                    {
                        EmitBranchComparison(inst, inst.Operand as Instruction, "<");
                        break;
                    }
                    case Code.Blt_Un:
                    case Code.Blt_Un_S:
                    {
                        EmitBranchComparison(inst, inst.Operand as Instruction, "<", isUnOrdered: true);
                        break;
                    }
                    case Code.Bne_Un:
                    case Code.Bne_Un_S:
                    {
                        EmitBranchComparison(inst, inst.Operand as Instruction, "!=", isUnOrdered: true);
                        break;
                    }
                    case Code.Ceq:
                    {
                        EmitComparisonOp(inst, "==");
                        break;
                    }
                    case Code.Cgt:
                    {
                        EmitComparisonOp(inst, ">");
                        break;
                    }
                    case Code.Cgt_Un:
                    {
                        EmitComparisonOp(inst, ">", isUnOrdered: true);
                        break;
                    }
                    case Code.Clt:
                    {
                        EmitComparisonOp(inst, "<");
                        break;
                    }
                    case Code.Clt_Un:
                    {
                        EmitComparisonOp(inst, "<", isUnOrdered: true);
                        break;
                    }
                    case Code.Switch:
                    {
                        EmitSwitch(inst, (Instruction[])inst.Operand);
                        break;
                    }
                    case Code.Ldind_I1:
                    {
                        EmitLoadInd(inst, ElementType.I1);
                        break;
                    }
                    case Code.Ldind_U1:
                    {
                        EmitLoadInd(inst, ElementType.U1);
                        break;
                    }
                    case Code.Ldind_I2:
                    {
                        EmitLoadInd(inst, ElementType.I2);
                        break;
                    }
                    case Code.Ldind_U2:
                    {
                        EmitLoadInd(inst, ElementType.U2);
                        break;
                    }
                    case Code.Ldind_I4:
                    {
                        EmitLoadInd(inst, ElementType.I4);
                        break;
                    }
                    case Code.Ldind_U4:
                    {
                        EmitLoadInd(inst, ElementType.U4);
                        break;
                    }
                    case Code.Ldind_I8:
                    {
                        EmitLoadInd(inst, ElementType.I8);
                        break;
                    }
                    case Code.Ldind_I:
                    {
                        EmitLoadInd(inst, ElementType.I);
                        break;
                    }
                    case Code.Ldind_Ref:
                    {
                        EmitLoadInd(inst, ElementType.Object);
                        break;
                    }
                    case Code.Ldind_R4:
                    {
                        EmitLoadInd(inst, ElementType.R4);
                        break;
                    }
                    case Code.Ldind_R8:
                    {
                        EmitLoadInd(inst, ElementType.R8);
                        break;
                    }
                    case Code.Stind_I1:
                    {
                        EmitStoreInd(inst, ReduceDataType.I1);
                        break;
                    }
                    case Code.Stind_I2:
                    {
                        EmitStoreInd(inst, ReduceDataType.I2);
                        break;
                    }
                    case Code.Stind_I4:
                    {
                        EmitStoreInd(inst, ReduceDataType.I4);
                        break;
                    }
                    case Code.Stind_I8:
                    {
                        EmitStoreInd(inst, ReduceDataType.I8);
                        break;
                    }
                    case Code.Stind_I:
                    {
                        EmitStoreInd(inst, ReduceDataType.I);
                        break;
                    }
                    case Code.Stind_R4:
                    {
                        EmitStoreInd(inst, ReduceDataType.R4);
                        break;
                    }
                    case Code.Stind_R8:
                    {
                        EmitStoreInd(inst, ReduceDataType.R8);
                        break;
                    }
                    case Code.Stind_Ref:
                    {
                        EmitStoreInd(inst, ReduceDataType.Ref);
                        break;
                    }
                    case Code.Add:
                    {
                        EmitBinArithOp(inst, "+");
                        break;
                    }
                    case Code.Add_Ovf:
                    {
                        EmitBinArithOpOverflow(inst, "ADD", isUnsigned: false);
                        break;
                    }
                    case Code.Add_Ovf_Un:
                    {
                        EmitBinArithOpOverflow(inst, "ADD", isUnsigned: true);
                        break;
                    }
                    case Code.Sub:
                    {
                        EmitBinArithOp(inst, "-");
                        break;
                    }
                    case Code.Sub_Ovf:
                    {
                        EmitBinArithOpOverflow(inst, "SUB", isUnsigned: false);
                        break;
                    }
                    case Code.Sub_Ovf_Un:
                    {
                        EmitBinArithOpOverflow(inst, "SUB", isUnsigned: true);
                        break;
                    }
                    case Code.Mul:
                    {
                        EmitBinArithOp(inst, "*");
                        break;
                    }
                    case Code.Mul_Ovf:
                    {
                        EmitBinArithOpOverflow(inst, "MUL", isUnsigned: false);
                        break;
                    }
                    case Code.Mul_Ovf_Un:
                    {
                        EmitBinArithOpOverflow(inst, "MUL", isUnsigned: true);
                        break;
                    }
                    case Code.Div:
                    {
                        EmitDivOrRemOp(inst, "/", "", isUnsigned: false);
                        break;
                    }
                    case Code.Div_Un:
                    {
                        EmitDivOrRemOp(inst, "/", "", isUnsigned: true);
                        break;
                    }
                    case Code.Rem:
                    {
                        EmitDivOrRemOp(inst, "%", VmFunctionNames.FMod, isUnsigned: false);
                        break;
                    }
                    case Code.Rem_Un:
                    {
                        EmitDivOrRemOp(inst, "%", VmFunctionNames.FMod, isUnsigned: true);
                        break;
                    }
                    case Code.And:
                    {
                        EmitBinBitwiseOp(inst, "&");
                        break;
                    }
                    case Code.Or:
                    {
                        EmitBinBitwiseOp(inst, "|");
                        break;
                    }
                    case Code.Xor:
                    {
                        EmitBinBitwiseOp(inst, "^");
                        break;
                    }
                    case Code.Shl:
                    {
                        EmitBitShiftOp(inst, "<<");
                        break;
                    }
                    case Code.Shr:
                    {
                        EmitBitShiftOp(inst, ">>");
                        break;
                    }
                    case Code.Shr_Un:
                    {
                        EmitBitShiftOp(inst, ">>", isUnsigned: true);
                        break;
                    }
                    case Code.Neg:
                    {
                        EmitUnaryArithOp(inst, "-");
                        break;
                    }
                    case Code.Not:
                    {
                        EmitUnaryBitwiseOp(inst, "~");
                        break;
                    }
                    case Code.Conv_I1:
                    {
                        EmitConvert(inst, ElementType.I1);
                        break;
                    }
                    case Code.Conv_U1:
                    {
                        EmitConvert(inst, ElementType.U1);
                        break;
                    }
                    case Code.Conv_I2:
                    {
                        EmitConvert(inst, ElementType.I2);
                        break;
                    }
                    case Code.Conv_U2:
                    {
                        EmitConvert(inst, ElementType.U2);
                        break;
                    }
                    case Code.Conv_I4:
                    {
                        EmitConvert(inst, ElementType.I4);
                        break;
                    }
                    case Code.Conv_U4:
                    {
                        EmitConvert(inst, ElementType.U4);
                        break;
                    }
                    case Code.Conv_I8:
                    {
                        EmitConvert(inst, ElementType.I8);
                        break;
                    }
                    case Code.Conv_U8:
                    {
                        EmitConvert(inst, ElementType.U8);
                        break;
                    }
                    case Code.Conv_I:
                    {
                        EmitConvert(inst, ElementType.I);
                        break;
                    }
                    case Code.Conv_U:
                    {
                        EmitConvert(inst, ElementType.U);
                        break;
                    }
                    case Code.Conv_R4:
                    {
                        EmitConvert(inst, ElementType.R4);
                        break;
                    }
                    case Code.Conv_R8:
                    {
                        EmitConvert(inst, ElementType.R8);
                        break;
                    }
                    case Code.Conv_Ovf_I1:
                    {
                        EmitConvertOverflow(inst, ElementType.I1, srcIsUnsigned: false);
                        break;
                    }
                    case Code.Conv_Ovf_I1_Un:
                    {
                        EmitConvertOverflow(inst, ElementType.I1, srcIsUnsigned: true);
                        break;
                    }
                    case Code.Conv_Ovf_U1:
                    {
                        EmitConvertOverflow(inst, ElementType.U1, srcIsUnsigned: false);
                        break;
                    }
                    case Code.Conv_Ovf_U1_Un:
                    {
                        EmitConvertOverflow(inst, ElementType.U1, srcIsUnsigned: true);
                        break;
                    }
                    case Code.Conv_Ovf_I2:
                    {
                        EmitConvertOverflow(inst, ElementType.I2, srcIsUnsigned: false);
                        break;
                    }
                    case Code.Conv_Ovf_I2_Un:
                    {
                        EmitConvertOverflow(inst, ElementType.I2, srcIsUnsigned: true);
                        break;
                    }
                    case Code.Conv_Ovf_U2:
                    {
                        EmitConvertOverflow(inst, ElementType.U2, srcIsUnsigned: false);
                        break;
                    }
                    case Code.Conv_Ovf_U2_Un:
                    {
                        EmitConvertOverflow(inst, ElementType.U2, srcIsUnsigned: true);
                        break;
                    }
                    case Code.Conv_Ovf_I4:
                    {
                        EmitConvertOverflow(inst, ElementType.I4, srcIsUnsigned: false);
                        break;
                    }
                    case Code.Conv_Ovf_I4_Un:
                    {
                        EmitConvertOverflow(inst, ElementType.I4, srcIsUnsigned: true);
                        break;
                    }
                    case Code.Conv_Ovf_U4:
                    {
                        EmitConvertOverflow(inst, ElementType.U4, srcIsUnsigned: false);
                        break;
                    }
                    case Code.Conv_Ovf_U4_Un:
                    {
                        EmitConvertOverflow(inst, ElementType.U4, srcIsUnsigned: true);
                        break;
                    }
                    case Code.Conv_Ovf_I8:
                    {
                        EmitConvertOverflow(inst, ElementType.I8, srcIsUnsigned: false);
                        break;
                    }
                    case Code.Conv_Ovf_I8_Un:
                    {
                        EmitConvertOverflow(inst, ElementType.I8, srcIsUnsigned: true);
                        break;
                    }
                    case Code.Conv_Ovf_U8:
                    {
                        EmitConvertOverflow(inst, ElementType.U8, srcIsUnsigned: false);
                        break;
                    }
                    case Code.Conv_Ovf_U8_Un:
                    {
                        EmitConvertOverflow(inst, ElementType.U8, srcIsUnsigned: true);
                        break;
                    }
                    case Code.Conv_Ovf_I:
                    {
                        EmitConvertOverflow(inst, ElementType.I, srcIsUnsigned: false);
                        break;
                    }
                    case Code.Conv_Ovf_I_Un:
                    {
                        EmitConvertOverflow(inst, ElementType.I, srcIsUnsigned: true);
                        break;
                    }
                    case Code.Conv_Ovf_U:
                    {
                        EmitConvertOverflow(inst, ElementType.U, srcIsUnsigned: false);
                        break;
                    }
                    case Code.Conv_Ovf_U_Un:
                    {
                        EmitConvertOverflow(inst, ElementType.U, srcIsUnsigned: true);
                        break;
                    }
                    case Code.Conv_R_Un:
                    {
                        EmitConvert(inst, ElementType.R8, true);
                        break;
                    }
                    case Code.Cpobj:
                    {
                        EmitCpObj(inst, inflatedType);
                        break;
                    }
                    case Code.Initobj:
                    {
                        EmitInitObj(inst, inflatedType);
                        break;
                    }
                    case Code.Stobj:
                    {
                        EmitStObj(inst, inflatedType);
                        break;
                    }
                    case Code.Ldobj:
                    {
                        EmitLdObj(inst, inflatedType);
                        break;
                    }
                    case Code.Ldstr:
                    {
                        EmitLoadStr(inst, (string)inst.Operand);
                        break;
                    }
                    case Code.Newobj:
                    {
                        EmitNewObj(inst, inflatedMethod, inlineToken);
                        break;
                    }
                    case Code.Castclass:
                    {
                        EmitCastClass(inst, inflatedType);
                        break;
                    }
                    case Code.Isinst:
                    {
                        EmitIsInst(inst, inflatedType);
                        break;
                    }
                    case Code.Unbox:
                    {
                        EmitUnbox(inst, inflatedType);
                        break;
                    }
                    case Code.Unbox_Any:
                    {
                        EmitUnboxAny(inst, inflatedType);
                        break;
                    }
                    case Code.Box:
                    {
                        EmitBox(inst, inflatedType);
                        break;
                    }
                    case Code.Throw:
                    {
                        EmitThrow(inst);
                        break;
                    }
                    case Code.Rethrow:
                    {
                        EmitRethrow(inst);
                        break;
                    }
                    case Code.Ldfld:
                    {
                        EmitLdfld(inst, inflatedField);
                        break;
                    }
                    case Code.Ldsfld:
                    {
                        EmitLdsfld(inst, inflatedField);
                        break;
                    }
                    case Code.Ldflda:
                    {
                        EmitLdflda(inst, inflatedField, inlineToken);
                        break;
                    }
                    case Code.Ldsflda:
                    {
                        EmitLdsflda(inst, inflatedField, inlineToken);
                        break;
                    }
                    case Code.Stfld:
                    {
                        EmitStfld(inst, inflatedField);
                        break;
                    }
                    case Code.Stsfld:
                    {
                        EmitStsfld(inst, inflatedField, inlineToken);
                        break;
                    }
                    case Code.Newarr:
                    {
                        EmitNewarr(inst, inflatedType);
                        break;
                    }
                    case Code.Ldlen:
                    {
                        EmitLdlen(inst);
                        break;
                    }
                    case Code.Ldelema:
                    {
                        EmitLdelema(inst, inflatedType, inlineToken);
                        break;
                    }
                    case Code.Ldelem_I1:
                    {
                        EmitLdelem(inst, _corlibTypes.SByte);
                        break;
                    }
                    case Code.Ldelem_U1:
                    {
                        EmitLdelem(inst, _corlibTypes.Byte);
                        break;
                    }
                    case Code.Ldelem_I2:
                    {
                        EmitLdelem(inst, _corlibTypes.Int16);
                        break;
                    }
                    case Code.Ldelem_U2:
                    {
                        EmitLdelem(inst, _corlibTypes.UInt16);
                        break;
                    }
                    case Code.Ldelem_I4:
                    {
                        EmitLdelem(inst, _corlibTypes.Int32);
                        break;
                    }
                    case Code.Ldelem_U4:
                    {
                        EmitLdelem(inst, _corlibTypes.UInt32);
                        break;
                    }
                    case Code.Ldelem_I8:
                    {
                        EmitLdelem(inst, _corlibTypes.Int64);
                        break;
                    }
                    case Code.Ldelem_I:
                    {
                        EmitLdelem(inst, _corlibTypes.IntPtr);
                        break;
                    }
                    case Code.Ldelem_R4:
                    {
                        EmitLdelem(inst, _corlibTypes.Single);
                        break;
                    }
                    case Code.Ldelem_R8:
                    {
                        EmitLdelem(inst, _corlibTypes.Double);
                        break;
                    }
                    case Code.Ldelem_Ref:
                    {
                        EmitLdelem(inst, _corlibTypes.Object);
                        break;
                    }
                    case Code.Ldelem:
                    {
                        EmitLdelemAny(inst, inflatedType, inlineToken);
                        break;
                    }
                    case Code.Stelem_I1:
                    {
                        EmitStelem(inst, _corlibTypes.SByte);
                        break;
                    }
                    case Code.Stelem_I2:
                    {
                        EmitStelem(inst, _corlibTypes.Int16);
                        break;
                    }
                    case Code.Stelem_I4:
                    {
                        EmitStelem(inst, _corlibTypes.Int32);
                        break;
                    }
                    case Code.Stelem_I8:
                    {
                        EmitStelem(inst, _corlibTypes.Int64);
                        break;
                    }
                    case Code.Stelem_I:
                    {
                        EmitStelem(inst, _corlibTypes.IntPtr);
                        break;
                    }
                    case Code.Stelem_R4:
                    {
                        EmitStelem(inst, _corlibTypes.Single);
                        break;
                    }
                    case Code.Stelem_R8:
                    {
                        EmitStelem(inst, _corlibTypes.Double);
                        break;
                    }
                    case Code.Stelem_Ref:
                    {
                        EmitStelemRef(inst);
                        break;
                    }
                    case Code.Stelem:
                    {
                        EmitStelemAny(inst, inflatedType, inlineToken);
                        break;
                    }
                    case Code.Mkrefany:
                    {
                        EmitMkrefany(inst, inflatedType, inlineToken);
                        break;
                    }
                    case Code.Refanytype:
                    {
                        EmitRefanytype(inst);
                        break;
                    }
                    case Code.Refanyval:
                    {
                        EmitRefanyval(inst, inflatedType, inlineToken);
                        break;
                    }
                    case Code.Ldtoken:
                    {
                        EmitLdToken(inst, inflatedTokenOperand, inlineToken);
                        break;
                    }
                    case Code.Endfinally:
                    {
                        EmitEndfinally(inst);
                        break;
                    }
                    case Code.Leave:
                    case Code.Leave_S:
                    {
                        EmitLeave(inst, (Instruction)inst.Operand);
                        break;
                    }
                    case Code.Endfilter:
                    {
                        EmitEndFilter(inst);
                        break;
                    }
                    case Code.Arglist:
                    {
                        EmitArglist(inst);
                        break;
                    }
                    case Code.Ldftn:
                    {
                        EmitLdftn(inst, (IMethod)inst.Operand);
                        break;
                    }
                    case Code.Ldvirtftn:
                    {
                        EmitLdvirtftn(inst, (IMethod)inst.Operand);
                        break;
                    }
                    case Code.Localloc:
                    {
                        // PushStackPointer(newPushedDatas, corLibTypes.IntPtr);
                        EmitLocalloc(inst);
                        break;
                    }
                    case Code.Unaligned:
                    {
                        SetPrefixUnalignmentData((int)(byte)inst.Operand);
                        break;
                    }
                    case Code.Volatile:
                    {
                        SetPrefixFlags(PrefixFlags.Volatile);
                        break;
                    }
                    case Code.Tailcall:
                    {
                        SetPrefixFlags(PrefixFlags.Tailcall);
                        break;
                    }
                    case Code.No:
                    {
                        SetPrefixFlags(PrefixFlags.No);
                        break;
                    }
                    case Code.Readonly:
                    {
                        SetPrefixFlags(PrefixFlags.Readonly);
                        break;
                    }
                    case Code.Constrained:
                    {
                        SetPrefixConstrainedData(inflatedType, inlineToken);
                        break;
                    }
                    case Code.Cpblk:
                    {
                        EmitCpblk(inst);
                        break;
                    }
                    case Code.Initblk:
                    {
                        EmitInitblk(inst);
                        break;
                    }
                    case Code.Sizeof:
                    {
                        // PushStack(newPushedDatas, EvalDataType.Int32);
                        EmitSizeOf(inst, (ITypeDefOrRef)inst.Operand);
                        break;
                    }
                    case Code.Ckfinite:
                    {
                        EmitCkfinite(inst);
                        break;
                    }
                    case Code.UNKNOWN1:
                    case Code.UNKNOWN2:
                    case Code.Prefix1:
                    case Code.Prefix2:
                    case Code.Prefix3:
                    case Code.Prefix4:
                    case Code.Prefix5:
                    case Code.Prefix6:
                    case Code.Prefix7:
                    case Code.Prefixref:
                    {
                        throw new NotSupportedException($"Prefix instruction {inst.OpCode} is not supported in method: {_method.FullName}.");
                    }
                    default:
                        throw new NotSupportedException($"Unsupported instruction: {inst} in method: {_method.FullName}.");
                    }
                }
                Instruction lastInst = _curBb.instructions.Last();
                if (_curBb.nextBlock != null && lastInst.OpCode.FlowControl != FlowControl.Branch &&
                    lastInst.OpCode.FlowControl != FlowControl.Return &&
                    lastInst.OpCode.FlowControl != FlowControl.Throw)
                {
                    EmitFallThroughSetupInOutVars(_curBb.nextBlock);
                }
                _bodyWriter.DecreaseIndent();
                _bodyWriter.AddLine("}");
                _bodyWriter.AddLine($"// Basic Block end");
            }
        }
    }
}