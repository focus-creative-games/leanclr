
using dnlib.DotNet;

namespace LeanAOT.ToCpp
{

    class RuntimeResolvedVariable
    {
        public readonly RuntimeResolvedMetadatas owner;
        public readonly int varId;
        public readonly uint token;

        public readonly object value;

        public RuntimeResolvedVariable(RuntimeResolvedMetadatas owner, int varId, uint token, object value)
        {
            this.owner = owner;
            this.varId = varId;
            this.token = token;
            this.value = value;
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

        public string GetFullReferenceVariableName()
        {
            return $"{owner.GetResolveMetadatasPtrVariableName()}->{GetVariableName()}";
        }

        //public string GetFullReferenceVariableNameAsMethodInfo()
        //{
        //    return $"((const leanclr::metadata::RtMethodInfo*){owner.GetResolveMetadatasPtrVariableName()}->{GetVariableName()})";
        //}

        //public string GetFullReferenceVariableNameAsClass()
        //{
        //    return $"((const leanclr::metadata::RtClass*){owner.GetResolveMetadatasPtrVariableName()}->{GetVariableName()})";
        //}


        public string GetFullReferenceVariableNameBeforeInit()
        {
            return $"{owner.GetResolveMetadatasVariableName()}.{GetVariableName()}";
        }

        public string ResolveCode()
        {
            if (value is string str)
            {
                return $"leanclr::codegen::resolve_string_literal({ModuleGenerationUtil.GetModuleGlobalVariableName(owner.Module)}, 0x{token:X8})";
            }
            else
            {
                return $"({GetVariableTypeName()})leanclr::codegen::resolve_metadata_token({ModuleGenerationUtil.GetModuleGlobalVariableName(owner.Module)}, 0x{token:X8}, nullptr)";
            }
        }
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

        private RuntimeResolvedVariable NewResolvedVariable(uint token, object value)
        {
            var variable = new RuntimeResolvedVariable(this, ++_lastResolvedVariableId, token, value);
            _resolvedVariables.Add(variable);
            return variable;
        }

        public string GetResolveMetadataStructName()
        {
            return $"{_method.UniqueName}__ResolvedMetadatas{(_method.IsGeneric ? "_Generic" : "")}";
        }

        public string GetResolveMetadatasVariableName()
        {
            return $"__resolvedMetadatas";
        }

        public string GetResolveMetadatasPtrVariableName()
        {
            return $"__resolvedMetadatas_ptr";
        }

        public RuntimeResolvedVariable GetUserStringVariable(string value, MDToken token)
        {
            if (!_userStrings.TryGetValue(value, out var userString))
            {
                var variable = NewResolvedVariable(token.ToUInt32(), value); // User string token base
                userString = new UserString(variable);
                _userStrings.Add(value, userString);
            }
            return userString.variable;
        }

        public RuntimeResolvedVariable GetMethodVariable(IMethod method)
        {
            uint token = method.MDToken.ToUInt32();
            if (!_tokenVariables.TryGetValue(token, out var variable))
            {
                variable = NewResolvedVariable(token, method);
                _tokenVariables.Add(token, variable);
            }
            return variable;
        }

        public RuntimeResolvedVariable GetTypeVariable(ITypeDefOrRef type)
        {
            uint token = type.MDToken.ToUInt32();
            if (!_tokenVariables.TryGetValue(token, out var variable))
            {
                variable = NewResolvedVariable(token, type);
                _tokenVariables.Add(token, variable);
            }
            return variable;
        }

        public RuntimeResolvedVariable GetFieldVariable(IField field)
        {
            uint token = field.MDToken.ToUInt32();
            if (!_tokenVariables.TryGetValue(token, out var variable))
            {
                variable = NewResolvedVariable(token, field);
                _tokenVariables.Add(token, variable);
            }
            return variable;
        }
    }
}