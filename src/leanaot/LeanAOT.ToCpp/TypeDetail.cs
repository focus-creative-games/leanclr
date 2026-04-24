using dnlib.DotNet;
using LeanAOT.Core;
using System;
using System.Text;

namespace LeanAOT.ToCpp
{

    public class TypeDetail
    {
        private readonly ITypeDefOrRef _type;
        private readonly TypeSig _classBaseTypeSig;
        private readonly TypeSig _typeSig;
        private readonly TypeDef _typeDef;
        private readonly GenericArgumentContext _gac;


        private readonly List<FieldDetail> _instanceFieldsExcludeParent;
        private readonly List<FieldDetail> _instanceFieldsIncludeParent;
        private readonly List<FieldDetail> _staticFields;
        private readonly List<FieldDetail> _literalOrRvaFields;

        private readonly string _instanceTypeName;
        private readonly string _staticTypeName;

        public ITypeDefOrRef Type => _type;

        public TypeDef TypeDef => _typeDef;

        public TypeSig ClassBaseTypeSig => _classBaseTypeSig;
        public TypeSig TypeSig => _typeSig;

        public string InstanceTypeName => _instanceTypeName;
        public string StaticTypeName => _staticTypeName;

        public string ObjectHeaderFieldName => "__object_header";

        public bool HasObjectHeader => _classBaseTypeSig != null;

        public IReadOnlyList<FieldDetail> InstanceFieldsIncludeParent => _instanceFieldsIncludeParent;

        public IReadOnlyList<FieldDetail> InstanceFieldsExcludeParent => _instanceFieldsExcludeParent;
        public IReadOnlyList<FieldDetail> StaticFields => _staticFields;

        public bool IsValueType => _typeDef.IsValueType;

        public GenericArgumentContext GAC => _gac;

        public static bool IsComposableType(TypeSig type)
        {
            switch (type.ElementType)
            {
            case ElementType.Class:
            case ElementType.ValueType:
            case ElementType.GenericInst:
                return true;
            default: return false;
            }
        }

        public TypeDetail(ITypeDefOrRef type, Func<ITypeDefOrRef, TypeDetail> typeDetailResover)
        {
            _type = type;
            _typeSig = type.ToTypeSig();
            //if (!IsComposableType(_typeSig))
            //{
            //    throw new ArgumentException($"Unsupported type: {_typeSig}");
            //}
            _typeDef = type.ResolveTypeDefThrow();
            _gac = MetaUtil.GetTypeGenericArgumentContext(type);
            _classBaseTypeSig = _typeDef.IsValueType || _typeDef.BaseType == null ? null : MetaUtil.Inflate(_typeDef.BaseType.ToTypeSig(), _gac);

            var parentInstanceFieldsIncludeParent = _classBaseTypeSig != null && _classBaseTypeSig.ElementType != ElementType.Object ? typeDetailResover(_classBaseTypeSig.ToTypeDefOrRef())._instanceFieldsIncludeParent : new List<FieldDetail>();
            (_instanceFieldsExcludeParent, _instanceFieldsIncludeParent, _staticFields, _literalOrRvaFields) = BuildInstanceAndStaticFields(_typeSig, parentInstanceFieldsIncludeParent);
            _instanceTypeName = BuildTypeName(_typeSig, _typeDef);
            _staticTypeName = _instanceTypeName + "_StaticFields";
        }

        private string BuildTypeName(TypeSig type, TypeDef typeDef)
        {
            var sb = new StringBuilder();
            sb.Append(MetaUtil.GetModuleNameWithoutExt(typeDef.Module));
            sb.Append('_');
            sb.Append(typeDef.FullName.Replace('.', '_').Replace('/', '_'));
            sb.Append('_');
            sb.Append(typeDef.MDToken.ToInt32().ToString("X8"));
            sb.Append('_');
            sb.Append(HashUtil.CreateMd5Hash(NameUtil.CreateFullQualifiedTypeName(type)));
            return NameUtil.StandardizeName(sb.ToString());
        }

        public FieldDetail GetFieldDetail(IField field)
        {
            FieldDef fieldDef = field.ResolveFieldDefThrow();
            var fields = fieldDef.IsLiteral || fieldDef.HasFieldRVA ? _literalOrRvaFields : (fieldDef.IsStatic ? _staticFields : _instanceFieldsExcludeParent);
            foreach (var f in fields)
            {
                if (f.FieldBase == fieldDef)
                {
                    return f;
                }
            }
            throw new MissingMemberException($"{field} not found in type:{_type}");
        }

        private string BuildFieldName(FieldDef fieldDef, int index)
        {
            // if (!fieldDef.IsStatic)
            // {
            //     TypeDef typeDef = fieldDef.DeclaringType;
            //     switch (typeDef.ToTypeSig().ElementType)
            //     {
            //     case ElementType.String:
            //     {
            //         switch (index)
            //         {
            //         case 0: return "length";
            //         case 1: return "first_char";
            //         default: throw new NotSupportedException($"Unexpected field index {index} for type {typeDef}");
            //         }
            //     }
            //     case ElementType.TypedByRef:
            //     {
            //         switch (index)
            //         {
            //         case 0: return "type_handle";
            //         case 1: return "value";
            //         case 2: return "klass";
            //         default: throw new NotSupportedException($"Unexpected field index {index} for type {typeDef}");
            //         }
            //     }
            //     default: break;
            //     }
            // }
            return $"__field_{index}";
        }

        private (List<FieldDetail>, List<FieldDetail>, List<FieldDetail>, List<FieldDetail>) BuildInstanceAndStaticFields(TypeSig type, List<FieldDetail> parentInstanceFieldsIncludeParent)
        {
            var instanceFieldsExcludeParent = new List<FieldDetail>();
            var instanceFieldsIncludeParent = new List<FieldDetail>(parentInstanceFieldsIncludeParent);
            var staticFields = new List<FieldDetail>();
            var literalOrRvaFields = new List<FieldDetail>();
            ITypeDefOrRef typeDefOrRef = type.ToTypeDefOrRef();
            var typeDef = typeDefOrRef.ResolveTypeDefThrow();
            var gac = MetaUtil.GetTypeGenericArgumentContext(type.ToTypeDefOrRef());
            foreach (var field in typeDef.Fields)
            {
                var fieldType = MetaUtil.Inflate(field.FieldType, gac);
                if (field.IsStatic)
                {
                    if (field.IsLiteral || field.HasFieldRVA)
                    {
                        literalOrRvaFields.Add(new FieldDetail(this, field, "", fieldType, -1));
                        continue;
                    }
                }
                int index = field.IsStatic ? staticFields.Count : instanceFieldsIncludeParent.Count;
                string fieldName = BuildFieldName(field, index);
                var fieldDetail = new FieldDetail(this, field, fieldName, fieldType, index);
                if (field.IsStatic)
                {
                    staticFields.Add(fieldDetail);
                }
                else
                {
                    instanceFieldsExcludeParent.Add(fieldDetail);
                    instanceFieldsIncludeParent.Add(fieldDetail);
                }
                literalOrRvaFields.Add(fieldDetail);
            }
            return (instanceFieldsExcludeParent, instanceFieldsIncludeParent, staticFields, literalOrRvaFields);
        }

    }
}
