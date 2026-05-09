using dnlib.DotNet;
using LeanAOT.Core;
using System.Text;

namespace LeanAOT.ToCpp
{

    public class ParamDetail
    {
        private readonly TypeSig _type;
        private readonly int _index;
        private readonly bool _isThis;
        private readonly string _name;

        public TypeSig Type => _type;
        public int Index => _index;
        public bool IsThis => _isThis;
        public string Name => _name;

        public ParamDetail(TypeSig type, int index, bool isThis, string name)
        {
            _type = type;
            _index = index;
            _isThis = isThis;
            _name = name;
        }
    }

    public class MethodDetail
    {
        private readonly IMethod _method;
        private readonly MethodDef _methodDef;
        private readonly string _uniqueName;
        private readonly GenericArgumentContext _gac;
        private readonly bool _isGeneric;
        private readonly TypeSig _retType;
        private readonly ParamDetail[] _paramsIncludeThis;
        private readonly ITypeDefOrRef _declaringType;

        public MethodDetail(IMethod method)
        {
            _method = method;
            _methodDef = method.ResolveMethodDef();
            _gac = MetaUtil.GetMethodGenericArgumentContext(method);
            _isGeneric = _methodDef != null && (_methodDef.GenericParameters.Count > 0 || _methodDef.DeclaringType.GenericParameters.Count > 0);

            MethodSig methodSig = method.MethodSig;
            _retType = InflateType(methodSig.RetType);

            var paramsIncludeThis = new List<ParamDetail>();
            if (!methodSig.HasThis && !methodSig.ExplicitThis)
            {
                // static method, no this
            }
            else
            {
                // instance method, add this
                TypeSig thisType = MetaUtil.GetThisType(method);
                paramsIncludeThis.Add(new ParamDetail(InflateType(thisType), paramsIncludeThis.Count, true, "__this"));
            }
            foreach (var param in methodSig.Params)
            {
                paramsIncludeThis.Add(new ParamDetail(InflateType(param), paramsIncludeThis.Count, true, $"__p{paramsIncludeThis.Count}"));
            }
            _paramsIncludeThis = paramsIncludeThis.ToArray();
            _declaringType = InflateType(method.DeclaringType.ToTypeSig()).ToTypeDefOrRef();
            _uniqueName = GenerateMethodUniqueName();
        }

        public string FullName => _method.FullName;

        public string UniqueName => _uniqueName;

        public string VirtualMethodUniqueName => $"{_uniqueName}_virtual";

        public IMethod Method => _method;

        public MethodDef MethodDef => _methodDef;

        public GenericArgumentContext GAC => _gac;

        public bool IsGeneric => _isGeneric;

        public ModuleDef Module => _isGeneric ? null : _methodDef.Module;

        public ModuleDef ModuleOfMethodDef => _methodDef.Module;

        public string ModuleNameNotExt => _methodDef.Module.Assembly.Name;

        public TypeSig RetType => _retType;

        public ParamDetail[] ParamsIncludeThis => _paramsIncludeThis;

        public TypeSig[] ParamTypesIncludeThis => _paramsIncludeThis.Select(p => p.Type).ToArray();

        public int ParamCountIncludeThis => _paramsIncludeThis.Length;

        public bool IsVoidReturn => MetaUtil.IsVoidType(_retType);

        public bool HasNotVoidReturn => !MetaUtil.IsVoidType(_retType);

        public bool IsStatic => _methodDef.IsStatic;

        public ITypeDefOrRef DeclaringType => _declaringType;

        public TypeDef DeclaringTypeDef => _methodDef?.DeclaringType;

        public bool IsNotVirtualOrSealed => !_methodDef.IsVirtual || _methodDef.IsFinal || _methodDef.DeclaringType.IsSealed;

        public bool ShouldGenerateVirtualMethod => _methodDef != null && MetaUtil.IsValueType(_methodDef.DeclaringType.ToTypeSig()) && !IsStatic;

        public TypeSig InflateType(TypeSig type)
        {
            if (!_isGeneric)
            {
                return type;
            }
            return MetaUtil.Inflate(type, _gac);
        }

        public ITypeDefOrRef InflateType(ITypeDefOrRef type)
        {
            if (!_isGeneric)
            {
                return type;
            }
            return MetaUtil.Inflate(type.ToTypeSig(), _gac).ToTypeDefOrRef();
        }

        public IMethod InflateMethod(IMethod method)
        {
            return !_isGeneric ? method : MetaUtil.InflateMethod(method, _gac);
        }

        public MethodSig InflateMethodSig(MethodSig methodSig)
        {
            return !_isGeneric ? methodSig : MetaUtil.InflateMethodSig(methodSig, _gac);
        }

        public IField InflateField(IField field)
        {
            return !_isGeneric ? field : MetaUtil.InflateField(field, _gac);
        }

        private string CreateFullQualifiedMethodName()
        {
            var result = new StringBuilder();
            NameUtil.AppendFullQualifiedTypeName(result, _retType);
            result.Append(ModuleNameNotExt);
            result.Append('_');
            NameUtil.AppendFullQualifiedTypeName(result, _declaringType.ToTypeSig());
            result.Append('_');
            result.Append(_methodDef.Name);
            if (_method is MethodSpec methodSpec)
            {
                var gis = methodSpec.GenericInstMethodSig;
                result.Append('<');
                foreach (var ga in gis.GenericArguments)
                {
                    result.Append(',');
                    NameUtil.AppendFullQualifiedTypeName(result, ga);
                }
                result.Append('>');
            }
            result.Append('(');
            foreach (var param in _paramsIncludeThis)
            {
                NameUtil.AppendFullQualifiedTypeName(result, param.Type);
                result.Append(',');
            }
            result.Append(')');
            return result.ToString();
        }

        private string GenerateMethodUniqueName()
        {
            if (_methodDef == null)
            {
                return "__impossible method__";
            }
            var result = new StringBuilder();

            NameUtil.AppendFullQualifiedTypeName(result, _retType);
            result.Append(ModuleNameNotExt);
            result.Append('_');
            TypeDef typeDef = _methodDef.DeclaringType;
            result.Append(typeDef.FullName);
            result.Append('_');
            result.Append(_methodDef.Name);
            result.Append('_');
            result.Append(_methodDef.MDToken.ToInt32().ToString("X8"));
            result.Append('_');
            result.Append(HashUtil.CreateMd5Hash(CreateFullQualifiedMethodName()));
            return NameUtil.StandardizeName(result.ToString());
        }

        public string CreateMethodParameters()
        {
            var typeNameService = GlobalServices.Inst.TypeNameService;
            var paramList = _paramsIncludeThis.Select(param => $"{typeNameService.GetCppTypeNameAsFieldOrArgOrLoc(param.Type, TypeNameRelaxLevel.Exactly)} {param.Name}");
            //if (includeMethodInfo)
            //{
            //    paramList = paramList.Append($"{ConstStrings.MethodInfoPtrTypeName} {ConstStrings.MethodInfoParameterName}");
            //}
            return string.Join(", ", paramList);
        }

        public string GenerateMethodDeclaring(string overrideMethodName = null)
        {
            return $"{MethodGenerationUtil.GetResultTypeName(_retType)} {overrideMethodName ?? _uniqueName}({CreateMethodParameters()}){ConstStrings.CppFunctionNoexcept}";
        }

        public string GenerateVirtualMethodDeclaring()
        {
            return GenerateMethodDeclaring(VirtualMethodUniqueName);
        }

        public string CreateMethodFunctionTypedefStatement(string cppTypedefName)
        {
            return $"typedef {CreateMethodFunctionTypeDefine(cppTypedefName)}";
        }

        public string CreateRelaxMethodFunctionTypedefStatement(string cppTypedefName)
        {
            return $"typedef {CreateRelaxMethodFunctionTypeDefine(cppTypedefName)}";
        }

        public string CreateRelaxMethodFunctionPointerTypeForCast()
        {
            var sb = new StringBuilder();
            sb.Append(MethodGenerationUtil.GetResultTypeName(_retType));
            sb.Append(" (*)(");
            bool first = true;
            foreach (var param in _paramsIncludeThis)
            {
                if (first)
                    first = false;
                else
                    sb.Append(", ");
                sb.Append(MethodGenerationUtil.GetCppTypeNameAsFieldOrArgOrLoc(param.Type, TypeNameRelaxLevel.AbiRelaxed));
            }
            sb.Append(')');
            sb.Append(ConstStrings.CppFunctionNoexcept);
            return sb.ToString();
        }

        public string CreateMethodFunctionTypeDefineWithoutName()
        {
            return CreateMethodFunctionTypeDefine("");
        }

        public string CreateNativeMethodFunctionTypeDefine(string cppTypedefName)
        {
            StringBuilder sb = new StringBuilder();
            sb.Append(MethodGenerationUtil.GetCppTypeNameAsFieldOrArgOrLoc(_retType, TypeNameRelaxLevel.AbiRelaxed));
            sb.Append($" (*{cppTypedefName})(");
            bool first = true;
            foreach (var param in _paramsIncludeThis)
            {
                if (first)
                    first = false;
                else
                {
                    sb.Append(", ");
                }
                sb.Append(MethodGenerationUtil.GetExactTypeName(param.Type));
            }
            sb.Append(')');
            sb.Append(ConstStrings.CppFunctionNoexcept);
            return sb.ToString();
        }

        private string CreateMethodFunctionTypeDefine(string cppTypedefName)
        {
            StringBuilder sb = new StringBuilder();
            sb.Append(MethodGenerationUtil.GetResultTypeName(_retType));
            sb.Append($" (*{cppTypedefName})(");
            bool first = true;
            foreach (var param in _paramsIncludeThis)
            {
                if (first)
                    first = false;
                else
                {
                    sb.Append(", ");
                }
                sb.Append(MethodGenerationUtil.GetExactTypeName(param.Type));
            }
            sb.Append(')');
            sb.Append(ConstStrings.CppFunctionNoexcept);
            return sb.ToString();
        }

        private string CreateRelaxMethodFunctionTypeDefine(string cppTypedefName)
        {
            StringBuilder sb = new StringBuilder();
            sb.Append(MethodGenerationUtil.GetResultTypeName(_retType));
            sb.Append($" (*{cppTypedefName})(");
            bool first = true;
            foreach (var param in _paramsIncludeThis)
            {
                if (first)
                    first = false;
                else
                {
                    sb.Append(", ");
                }
                sb.Append(MethodGenerationUtil.GetCppTypeNameAsFieldOrArgOrLoc(param.Type, TypeNameRelaxLevel.AbiRelaxed));
            }
            sb.Append(')');
            sb.Append(ConstStrings.CppFunctionNoexcept);
            return sb.ToString();
        }

        public string CreateOverrideRetTypeRelaxMethodFunctionTypeDefine(string cppTypedefName, string retType)
        {
            StringBuilder sb = new StringBuilder();
            sb.Append($"{retType}");
            sb.Append($" (*{cppTypedefName})(");
            bool first = true;
            foreach (var param in _paramsIncludeThis)
            {
                if (first)
                    first = false;
                else
                {
                    sb.Append(", ");
                }
                sb.Append(MethodGenerationUtil.GetExactTypeName(param.Type));
            }
            sb.Append(')');
            sb.Append(ConstStrings.CppFunctionNoexcept);
            return sb.ToString();
        }
    }
}
