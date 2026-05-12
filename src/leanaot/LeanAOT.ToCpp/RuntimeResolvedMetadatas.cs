
using dnlib.DotNet;
using System.Diagnostics;

namespace LeanAOT.ToCpp
{
    class SubRuntimeResolvedVariable
    {
        public string Key { get; }

        public string VariableName { get; }

        public string FullReferenceVariableName { get; }

        public string TypeName { get; }

        public string InitExpression { get; }

        public SubRuntimeResolvedVariable(string key, string variableName, string fullReferenceVariableName, string typeName, string initExpression)
        {
            Key = key;
            VariableName = variableName;
            FullReferenceVariableName = fullReferenceVariableName;
            TypeName = typeName;
            InitExpression = initExpression;
        }
    }

    class RuntimeResolvedVariable
    {

        public readonly RuntimeResolvedMetadatas owner;
        public readonly int varId;
        public readonly uint token;

        public readonly object value;

        public readonly object detailValue;

        private readonly Dictionary<string, SubRuntimeResolvedVariable> subVariables = new Dictionary<string, SubRuntimeResolvedVariable>();

        public RuntimeResolvedVariable(RuntimeResolvedMetadatas owner, int varId, uint token, object value, object detailValue)
        {
            this.owner = owner;
            this.varId = varId;
            this.token = token;
            this.value = value;
            this.detailValue = detailValue;
        }

        public object Value => value;

        public string GetVariableName()
        {
            return $"__r{varId}";
        }

        public string GetVariableTypeName()
        {
            if (value is IMethod method && method.IsMethod)
            {
                return "const leanclr::metadata::RtMethodInfo*";
            }
            else if (value is IField field && field.IsField)
            {
                return "leanclr::metadata::RtFieldInfo*";
            }
            else if (value is ITypeDefOrRef type && type.IsType)
            {
                return "leanclr::metadata::RtClass*";
            }
            else if (value is string)
            {
                return "leanclr::vm::RtString*";
            }
            else
            {
                throw new Exception($"Unsupported value type for runtime resolved variable: {value}");
            }
        }

        public IReadOnlyList<SubRuntimeResolvedVariable> GetSubVariables()
        {
            return subVariables.Values.ToList();
        }

        public string GetFullReferenceVariableName()
        {
            return $"{owner.GetResolveMetadatasVariableName()}.{GetVariableName()}";
        }

        //public string GetFullReferenceVariableNameAsMethodInfo()
        //{
        //    return $"((const leanclr::metadata::RtMethodInfo*){owner.GetResolveMetadatasPtrVariableName()}->{GetVariableName()})";
        //}

        //public string GetFullReferenceVariableNameAsClass()
        //{
        //    return $"((const leanclr::metadata::RtClass*){owner.GetResolveMetadatasPtrVariableName()}->{GetVariableName()})";
        //}

        public SubRuntimeResolvedVariable GetFieldStaticDataSubVariable()
        {
            FieldDetail fieldDetail = (FieldDetail)detailValue;
            Debug.Assert(fieldDetail.FieldBase.IsStatic);
            string key = "static_data";
            if (subVariables.TryGetValue(key, out var subVariable))
            {
                return subVariable;
            }
            string name = $"{GetVariableName()}_{key}";
            string fullReferenceVariableName = $"{owner.GetSubResolveMetadatasVariableName()}.{name}";
            string fieldType = $"{fieldDetail.Parent.StaticTypeName}*";
            subVariable = new SubRuntimeResolvedVariable(key, name, fullReferenceVariableName, fieldType, $"({fieldType})({GetFullReferenceVariableName()})->parent->{ConstStrings.KlassFieldNameStaticFieldsData}");
            subVariables.Add(key, subVariable);
            return subVariable;
        }

        public string GetFullReferenceVariableNameBeforeInit()
        {
            return $"{owner.GetResolveMetadatasVariableName()}.{GetVariableName()}";
        }

        // public string ResolveCode()
        // {
        //     if (value is string str)
        //     {
        //         return $"leanclr::codegen::resolve_string_literal({ModuleGenerationUtil.GetModuleGlobalVariableName(owner.Module)}, 0x{token:X8})";
        //     }
        //     else
        //     {
        //         return $"({GetVariableTypeName()})leanclr::codegen::resolve_metadata_token({ModuleGenerationUtil.GetModuleGlobalVariableName(owner.Module)}, 0x{token:X8}, nullptr)";
        //     }
        // }
    }

    class UserString
    {
        public readonly RuntimeResolvedVariable variable;

        public UserString(RuntimeResolvedVariable variable)
        {
            this.variable = variable;
        }
    }

    class RuntimeResolvedMetadatas
    {
        private readonly MethodDetail _method;
        private readonly Dictionary<string, UserString> _userStrings = new Dictionary<string, UserString>();
        private readonly Dictionary<uint, RuntimeResolvedVariable> _tokenVariables = new Dictionary<uint, RuntimeResolvedVariable>();
        private int _lastResolvedVariableId;
        private readonly List<RuntimeResolvedVariable> _resolvedVariables = new List<RuntimeResolvedVariable>();

        public RuntimeResolvedMetadatas(MethodDetail method)
        {
            _method = method;
        }

        public ModuleDef Module => _method.Module;

        public bool IsEmpty()
        {
            return _lastResolvedVariableId == 0;
        }

        public IReadOnlyList<RuntimeResolvedVariable> ResolvedVariables => _resolvedVariables;

        private RuntimeResolvedVariable NewResolvedVariable(uint token, object value, object detailValue)
        {
            var variable = new RuntimeResolvedVariable(this, ++_lastResolvedVariableId, token, value, detailValue);
            _resolvedVariables.Add(variable);
            return variable;
        }

        public string GetResolveMetadataStructName()
        {
            return $"{_method.UniqueName}__ResolvedMetadatas{(_method.IsGeneric ? "_Generic" : "")}";
        }

        public string GetResolveMetadatasVariableName()
        {
            return $"{_method.UniqueName}__resolvedMetadatas";
        }

        public string getSubResolveMetadataStructName()
        {
            return $"{_method.UniqueName}__SubResolvedMetadatas{(_method.IsGeneric ? "_Generic" : "")}";
        }

        public string GetSubResolveMetadatasVariableName()
        {
            return $"{_method.UniqueName}__subResolvedMetadatas";
        }

        public string GetResolveMetadatasInitedVariableName()
        {
            return $"{_method.UniqueName}__resolvedMetadatas_inited";
        }

        public string GetResolveMetadatasTokensVariableName()
        {
            return $"{_method.UniqueName}__resolvedMetadatas_tokens";
        }

        public RuntimeResolvedVariable GetUserStringVariable(string value, MDToken token)
        {
            if (!_userStrings.TryGetValue(value, out var userString))
            {
                var variable = NewResolvedVariable(token.ToUInt32(), value, null); // User string token base
                userString = new UserString(variable);
                _userStrings.Add(value, userString);
            }
            return userString.variable;
        }

        public RuntimeResolvedVariable GetMethodVariable(IMethod method, MethodDetail methodDetail)
        {
            uint token = method.MDToken.ToUInt32();
            if (!_tokenVariables.TryGetValue(token, out var variable))
            {
                variable = NewResolvedVariable(token, method, methodDetail);
                _tokenVariables.Add(token, variable);
            }
            return variable;
        }

        public RuntimeResolvedVariable GetTypeVariable(ITypeDefOrRef type, TypeDetail typeDetail)
        {
            uint token = type.MDToken.ToUInt32();
            if (!_tokenVariables.TryGetValue(token, out var variable))
            {
                variable = NewResolvedVariable(token, type, typeDetail);
                _tokenVariables.Add(token, variable);
            }
            return variable;
        }

        public RuntimeResolvedVariable GetFieldVariable(IField field, FieldDetail fieldDetail)
        {
            uint token = field.MDToken.ToUInt32();
            if (!_tokenVariables.TryGetValue(token, out var variable))
            {
                variable = NewResolvedVariable(token, field, fieldDetail);
                _tokenVariables.Add(token, variable);
            }
            return variable;
        }
    }
}