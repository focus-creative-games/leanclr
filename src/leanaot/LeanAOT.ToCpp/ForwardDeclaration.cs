using dnlib.DotNet;
using LeanAOT.Core;
using System.Diagnostics;

namespace LeanAOT.ToCpp
{

    class ForwardDeclaration
    {
        private readonly CodeThrunkWriter _includesWriter;
        private readonly CodeThrunkWriter _forwardDeclsWriter;
        private readonly CodeThrunkWriter _typeDefinesWriter;

        private readonly MetadataService _metadataService;
        private readonly TypeNameService _typeNameService;
        private readonly ManifestService _manifestService;

        private readonly HashSet<string> _addedIncludes = new HashSet<string>();
        private readonly HashSet<ModuleDef> _addedModules = new HashSet<ModuleDef>();
        private readonly HashSet<IMethod> _addedMethods = new HashSet<IMethod>(MethodEqualityComparer.CompareDeclaringTypes);
        private readonly HashSet<ITypeDefOrRef> _addedTypes = new HashSet<ITypeDefOrRef>(TypeEqualityComparer.Instance);
        private readonly HashSet<MethodInvokerInfo> _addedInvokers = new HashSet<MethodInvokerInfo>();

        public ForwardDeclaration(CodeThunkZone writer)
        {
            _includesWriter = writer.CreateThunk("includes");
            _forwardDeclsWriter = writer.CreateThunk("forward_declarations");
            _typeDefinesWriter = writer.CreateThunk("type_definitions");

            var globalServices = GlobalServices.Inst;
            _metadataService = globalServices.MetadataService;
            _typeNameService = globalServices.TypeNameService;
            _manifestService = globalServices.ManifestService;
        }

        public void AddCommonIncludes(ModuleDef mod)
        {
            AddInclude($"{ModuleGenerationUtil.GetModuleRegistrationHeaderFileNameWithExt(mod)}");
        }

        private void AddInclude(string include)
        {
            if (!_addedIncludes.Add(include))
                return;
            _includesWriter.AddLine($"#include \"{include}\"");
        }

        public void AddModuleForwardDeclaration(ModuleDef mod)
        {
            if (!_addedModules.Add(mod))
                return;
            _forwardDeclsWriter.AddLine(ModuleGenerationUtil.GetModuleForwardDeclaration(mod));
        }


        private void AddTypeNotStaticDefinition(TypeDetail type)
        {
            TypeDef typeDef = type.TypeDef;
            if (typeDef != null)
            {
                var typeDefSig = typeDef.ToTypeSig();
                switch (typeDefSig.ElementType)
                {
                case ElementType.Class:
                case ElementType.ValueType:
                case ElementType.String:
                case ElementType.TypedByRef:
                    break;
                default: return;
                }
            }

            _forwardDeclsWriter.AddLine($"struct {type.InstanceTypeName};");
            foreach (var field in type.InstanceFieldsIncludeParent)
            {
                AddTypeForwardDeclaration(field.Type);
            }
            foreach (var field in type.StaticFields)
            {
                AddTypeForwardDeclaration(field.Type);
            }
            uint packingSize = typeDef.ClassLayout != null ? typeDef.PackingSize : 0u;
            uint classSize = typeDef.ClassLayout != null ? typeDef.ClassSize : 0;
            if (typeDef.IsValueType && typeDef.IsExplicitLayout)
            {
                _typeDefinesWriter.AddLine($"struct {type.InstanceTypeName}");
                _typeDefinesWriter.AddLine("{");
                _typeDefinesWriter.IncreaseIndent();
                _typeDefinesWriter.AddLine("union");
                _typeDefinesWriter.AddLine("{");
                _typeDefinesWriter.IncreaseIndent();
                if (classSize > 0)
                {
                    _typeDefinesWriter.AddLine($"uint8_t __classSize[{classSize}];");
                }
                foreach (var field in type.InstanceFieldsIncludeParent)
                {
                    uint offset = field.FieldBase.FieldOffset.Value;
                    _typeDefinesWriter.AddLine("#pragma pack(push, 1)");
                    string fieldTypeName = _typeNameService.GetCppTypeNameAsFieldOrArgOrLoc(field.Type, TypeNameRelaxLevel.Exactly);
                    _typeDefinesWriter.AddLine($"struct {{{(offset > 0 ? $" char __offsetPadding{field.Name}[{offset}];" : "")} {fieldTypeName} {field.Name}; }};");
                    _typeDefinesWriter.AddLine("#pragma pack(pop)");
                    if (packingSize > 0)
                    {
                        _typeDefinesWriter.AddLine($"#pragma pack(push, {packingSize})");
                    }
                    _typeDefinesWriter.AddLine($"struct {{{(offset > 0 ? $" char __offsetPaddingForPacking{field.Name}[{offset}];" : "")} {fieldTypeName} __packing_{field.Name}; }};");
                    if (packingSize > 0)
                    {
                        _typeDefinesWriter.AddLine($"#pragma pack(pop)");
                    }
                }
                _typeDefinesWriter.DecreaseIndent();
                _typeDefinesWriter.AddLine("};");
                _typeDefinesWriter.DecreaseIndent();
                _typeDefinesWriter.AddLine("};");
            }
            else
            {
                if (packingSize > 0)
                {
                    _typeDefinesWriter.AddLine($"#pragma pack(push, {packingSize})");
                }
                _typeDefinesWriter.AddLine($"struct {type.InstanceTypeName}");
                _typeDefinesWriter.AddLine("{");
                _typeDefinesWriter.IncreaseIndent();
                if (classSize > 0)
                {
                    _typeDefinesWriter.AddLine($"union");
                    _typeDefinesWriter.AddLine("{");
                    _typeDefinesWriter.IncreaseIndent();
                    _typeDefinesWriter.AddLine($"uint8_t __classSize[{classSize}];");
                    _typeDefinesWriter.AddLine("struct");
                    _typeDefinesWriter.AddLine("{");
                    _typeDefinesWriter.IncreaseIndent();
                }

                if (type.HasObjectHeader)
                {
                    _typeDefinesWriter.AddLine($"{ConstStrings.ObjectTypeName} {type.ObjectHeaderFieldName};");
                }
                foreach (var field in type.InstanceFieldsIncludeParent)
                {
                    _typeDefinesWriter.AddLine($"{_typeNameService.GetCppTypeNameAsFieldOrArgOrLoc(field.Type, TypeNameRelaxLevel.Exactly)} {field.Name};");
                }
                if (classSize > 0)
                {
                    _typeDefinesWriter.DecreaseIndent();
                    _typeDefinesWriter.AddLine("};");
                    _typeDefinesWriter.DecreaseIndent();
                    _typeDefinesWriter.AddLine("};");
                }

                if (_typeNameService.IsPtrLikeSystemValueType(typeDef))
                {
                    var firstField = type.InstanceFieldsIncludeParent[0];
                    var fieldTypeName = _typeNameService.GetCppTypeNameAsFieldOrArgOrLoc(firstField.Type, TypeNameRelaxLevel.Exactly);
                    _typeDefinesWriter.AddLine($"{type.InstanceTypeName}() = default;");
                    _typeDefinesWriter.AddLine($"{type.InstanceTypeName}(const void* ptr) {{ {firstField.Name} = ({fieldTypeName})ptr; }}");
                    _typeDefinesWriter.AddLine($"operator void*() const {{ return (void*){firstField.Name}; }}");
                    _typeDefinesWriter.AddLine($"{type.InstanceTypeName}(intptr_t ptr) {{ {firstField.Name} = ({fieldTypeName})ptr; }}");
                    _typeDefinesWriter.AddLine($"operator intptr_t() const {{ return (intptr_t){firstField.Name}; }}");
                }

                _typeDefinesWriter.DecreaseIndent();
                _typeDefinesWriter.AddLine("};");
                if (packingSize > 0)
                {
                    _typeDefinesWriter.AddLine($"#pragma pack(pop)");
                }
            }
            if (_typeNameService.IsPtrLikeSystemValueType(typeDef))
            {
                _typeDefinesWriter.AddLine($"static_assert(sizeof({type.InstanceTypeName}) == sizeof(void*), \"Size mismatch for ptr-like system value type\");");
            }
        }

        private void AddTypeDefinition(TypeDetail type)
        {
            AddTypeNotStaticDefinition(type);

            _typeDefinesWriter.AddLine();

            if (type.TypeDef == null)
            {
                return;
            }
            _typeDefinesWriter.AddLine($"struct {type.StaticTypeName}");
            _typeDefinesWriter.AddLine("{");
            foreach (var field in type.StaticFields)
            {
                _typeDefinesWriter.AddLine($"    {_typeNameService.GetCppTypeNameAsFieldOrArgOrLoc(field.Type, TypeNameRelaxLevel.Exactly)} {field.Name};");
            }
            _typeDefinesWriter.AddLine("};");
            _typeDefinesWriter.AddLine();
        }

        public void AddTypeForwardDeclaration(ITypeDefOrRef type)
        {
            AddTypeForwardDeclaration(type.ToTypeSig());
        }

        public void AddTypeForwardDeclaration(TypeSig typeSig)
        {
            if (MetaUtil.IsEnumType(typeSig))
            {
                return;
            }
            typeSig = typeSig.RemovePinnedAndModifiers();
            ITypeDefOrRef type = typeSig.ToTypeDefOrRef();
            switch (typeSig.ElementType)
            {
            case ElementType.Class:
            case ElementType.ValueType:
            case ElementType.String:
            case ElementType.TypedByRef:
            {
                TypeDef typeDef = type.ResolveTypeDefThrow();
                if (typeDef.HasGenericParameters)
                {
                    return;
                }
                ITypeDefOrRef baseType = type.GetBaseType();
                if (baseType != null)
                {
                    AddTypeForwardDeclaration(baseType);
                }

                if (_addedTypes.Add(type))
                {
                    AddTypeDefinition(_metadataService.GetTypeDetail(type));
                }
                break;
            }
            case ElementType.GenericInst:
            {
                if (!_addedTypes.Add(type))
                {
                    break;
                }

                ITypeDefOrRef baseType = type.GetBaseType();
                if (baseType != null)
                {
                    AddTypeForwardDeclaration(baseType);
                }
                GenericInstSig genericInstSig = (GenericInstSig)typeSig;

                bool hasGenericParam = false;
                foreach (var arg in genericInstSig.GenericArguments)
                {
                    AddTypeForwardDeclaration(arg);
                    hasGenericParam = hasGenericParam || arg.ContainsGenericParameter;
                }
                //AddTypeDefinition(_metadataService.GetTypeDetail(genericType));
                if (hasGenericParam)
                {
                    return;
                }
                AddTypeDefinition(_metadataService.GetTypeDetail(type));
                break;
            }
            case ElementType.Ptr:
            case ElementType.ByRef:
            {
                AddTypeForwardDeclaration(typeSig.Next);
                break;
            }
            case ElementType.Void:
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
            case ElementType.R4:
            case ElementType.R8:
            case ElementType.I:
            case ElementType.U:
            case ElementType.Object:
            {
                if (!_addedTypes.Add(type))
                {
                    break;
                }
                TypeDef typeDef = type.ResolveTypeDef();
                if (typeDef != null)
                {
                    AddTypeDefinition(_metadataService.GetTypeDetail(typeDef));
                }
                break;
            }
            }
        }

        public void AddFieldForwardDeclaration(IField field)
        {
            var fieldDetail = _metadataService.GetFieldDetail(field);
            AddTypeForwardDeclaration(fieldDetail.ParentType);
            AddTypeForwardDeclaration(fieldDetail.Type);
        }

        public void AddMethodForwardDeclaration(IMethod method)
        {
            if (!_addedMethods.Add(method))
                return;
            MethodDetail methodDetail = _metadataService.GetMethodDetail(method);
            AddTypeForwardDeclaration(methodDetail.RetType);
            foreach (var param in methodDetail.ParamsIncludeThis)
            {
                AddTypeForwardDeclaration(param.Type);
            }
            MethodDef methodDef = methodDetail.MethodDef;
            if (methodDef == null || methodDef.IsAbstract)
            {
                return;
            }

            if (!_manifestService.ShouldAOT(method))
            {
                return;
            }
            _forwardDeclsWriter.AddLine($"{methodDetail.GenerateMethodDeclaring()};");
            _forwardDeclsWriter.AddLine();
        }

        public void AddInvokerForwardDeclaration(MethodInvokerInfo invoker)
        {
            if (!_addedInvokers.Add(invoker))
                return;
            _forwardDeclsWriter.AddLine($"{ConstStrings.RtResultVoidTypeName} {invoker.name}({ConstStrings.ManagedMethodPointerTypeName}, {ConstStrings.MethodInfoPtrTypeName}, const {ConstStrings.StackObjectTypeName}*, {ConstStrings.StackObjectTypeName}*){ConstStrings.CppFunctionNoexcept};");
            _forwardDeclsWriter.AddLine();
        }
    }
}
