using dnlib.DotNet;
using LeanAOT.Core;
using System.Diagnostics;

namespace LeanAOT.ToCpp
{

    class ForwardDeclaration
    {
        private readonly CodeThrunkWriter _includesWriter;
        private readonly CodeThrunkWriter _typeDeclsWriter;
        private readonly CodeThrunkWriter _methodDeclsWriter;
        private readonly CodeThrunkWriter _classDefinesWriter;
        private readonly CodeThrunkWriter _structDefinesWriter;

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
            _typeDeclsWriter = writer.CreateThunk("type_declarations");
            _structDefinesWriter = writer.CreateThunk("struct_definitions");
            _classDefinesWriter = writer.CreateThunk("class_definitions");
            _methodDeclsWriter = writer.CreateThunk("method_declarations");

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
            _methodDeclsWriter.AddLine(ModuleGenerationUtil.GetModuleForwardDeclaration(mod));
        }


        private void AddTypeNotStaticDefinition(TypeDetail type, CodeThrunkWriter typeDefinesWriter)
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

            _typeDeclsWriter.AddLine($"struct {type.InstanceTypeName};");
            uint packingSize = typeDef.ClassLayout != null ? typeDef.PackingSize : 0u;
            uint classSize = typeDef.ClassLayout != null ? typeDef.ClassSize : 0;
            if (typeDef.IsValueType && typeDef.IsExplicitLayout)
            {
                typeDefinesWriter.AddLine($"struct {type.InstanceTypeName}");
                typeDefinesWriter.AddLine("{");
                typeDefinesWriter.IncreaseIndent();
                typeDefinesWriter.AddLine("union");
                typeDefinesWriter.AddLine("{");
                typeDefinesWriter.IncreaseIndent();
                if (classSize > 0)
                {
                    typeDefinesWriter.AddLine($"uint8_t __classSize[{classSize}];");
                }
                foreach (var field in type.InstanceFieldsIncludeParent)
                {
                    uint offset = field.FieldBase.FieldOffset.Value;
                    typeDefinesWriter.AddLine("#pragma pack(push, 1)");
                    string fieldTypeName = _typeNameService.GetCppTypeNameAsFieldOrArgOrLoc(field.Type, TypeNameRelaxLevel.Exactly);
                    typeDefinesWriter.AddLine($"struct {{{(offset > 0 ? $" char __offsetPadding{field.Name}[{offset}];" : "")} {fieldTypeName} {field.Name}; }};");
                    typeDefinesWriter.AddLine("#pragma pack(pop)");
                    if (packingSize > 0)
                    {
                        typeDefinesWriter.AddLine($"#pragma pack(push, {packingSize})");
                    }
                    typeDefinesWriter.AddLine($"struct {{{(offset > 0 ? $" char __offsetPaddingForPacking{field.Name}[{offset}];" : "")} {fieldTypeName} __packing_{field.Name}; }};");
                    if (packingSize > 0)
                    {
                        typeDefinesWriter.AddLine($"#pragma pack(pop)");
                    }
                }
                typeDefinesWriter.DecreaseIndent();
                typeDefinesWriter.AddLine("};");
                typeDefinesWriter.DecreaseIndent();
                typeDefinesWriter.AddLine("};");
            }
            else
            {
                if (packingSize > 0)
                {
                    typeDefinesWriter.AddLine($"#pragma pack(push, {packingSize})");
                }
                typeDefinesWriter.AddLine($"struct {type.InstanceTypeName}");
                typeDefinesWriter.AddLine("{");
                typeDefinesWriter.IncreaseIndent();
                if (classSize > 0)
                {
                    typeDefinesWriter.AddLine($"union");
                    typeDefinesWriter.AddLine("{");
                    typeDefinesWriter.IncreaseIndent();
                    typeDefinesWriter.AddLine($"uint8_t __classSize[{classSize}];");
                    typeDefinesWriter.AddLine("struct");
                    typeDefinesWriter.AddLine("{");
                    typeDefinesWriter.IncreaseIndent();
                }

                if (type.HasObjectHeader)
                {
                    typeDefinesWriter.AddLine($"{ConstStrings.ObjectTypeName} {type.ObjectHeaderFieldName};");
                }
                foreach (var field in type.InstanceFieldsIncludeParent)
                {
                    typeDefinesWriter.AddLine($"{_typeNameService.GetCppTypeNameAsFieldOrArgOrLoc(field.Type, TypeNameRelaxLevel.Exactly)} {field.Name};");
                }
                if (!type.HasObjectHeader && type.InstanceFieldsIncludeParent.Count == 0)
                {
                    typeDefinesWriter.AddLine($"uint8_t __placeholderForEmptyStruct;");
                }
                if (classSize > 0)
                {
                    typeDefinesWriter.DecreaseIndent();
                    typeDefinesWriter.AddLine("};");
                    typeDefinesWriter.DecreaseIndent();
                    typeDefinesWriter.AddLine("};");
                }

                if (_typeNameService.IsPtrLikeSystemValueType(typeDef))
                {
                    var firstField = type.InstanceFieldsIncludeParent[0];
                    var fieldTypeName = _typeNameService.GetCppTypeNameAsFieldOrArgOrLoc(firstField.Type, TypeNameRelaxLevel.Exactly);
                    typeDefinesWriter.AddLine($"{type.InstanceTypeName}() = default;");
                    typeDefinesWriter.AddLine($"{type.InstanceTypeName}(const void* ptr) {{ {firstField.Name} = ({fieldTypeName})ptr; }}");
                    typeDefinesWriter.AddLine($"operator void*() const {{ return (void*){firstField.Name}; }}");
                    typeDefinesWriter.AddLine($"{type.InstanceTypeName}(intptr_t ptr) {{ {firstField.Name} = ({fieldTypeName})ptr; }}");
                    typeDefinesWriter.AddLine($"operator intptr_t() const {{ return (intptr_t){firstField.Name}; }}");
                }

                typeDefinesWriter.DecreaseIndent();
                typeDefinesWriter.AddLine("};");
                if (packingSize > 0)
                {
                    typeDefinesWriter.AddLine($"#pragma pack(pop)");
                }
            }
            if (_typeNameService.IsPtrLikeSystemValueType(typeDef))
            {
                typeDefinesWriter.AddLine($"static_assert(sizeof({type.InstanceTypeName}) == sizeof(void*), \"Size mismatch for ptr-like system value type\");");
            }
        }

        private void AddTypeDefinition(TypeDetail type)
        {

            foreach (var field in type.InstanceFieldsIncludeParent)
            {
                AddTypeForwardDeclaration(field.Type);
            }
            foreach (var field in type.StaticFields)
            {
                AddTypeForwardDeclaration(field.Type);
            }

            var typeDefineWriter = type.TypeDef != null && type.TypeDef.IsValueType ? _structDefinesWriter : _classDefinesWriter;
            AddTypeNotStaticDefinition(type, typeDefineWriter);

            typeDefineWriter.AddLine();

            if (type.TypeDef == null)
            {
                return;
            }
            typeDefineWriter.AddLine($"struct {type.StaticTypeName}");
            typeDefineWriter.AddLine("{");
            foreach (var field in type.StaticFields)
            {
                typeDefineWriter.AddLine($"    {_typeNameService.GetCppTypeNameAsFieldOrArgOrLoc(field.Type, TypeNameRelaxLevel.Exactly)} {field.Name};");
            }
            typeDefineWriter.AddLine("};");
            typeDefineWriter.AddLine();
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
                TypeDef typeDef = type.ResolveTypeDef();
                if (typeDef != null && _addedTypes.Add(typeDef))
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
            _methodDeclsWriter.AddLine($"{methodDetail.GenerateMethodDeclaring()};");
            _methodDeclsWriter.AddLine();
        }

        public void AddInvokerForwardDeclaration(MethodInvokerInfo invoker)
        {
            if (!_addedInvokers.Add(invoker))
                return;
            _methodDeclsWriter.AddLine($"{ConstStrings.RtResultVoidTypeName} {invoker.name}({ConstStrings.ManagedMethodPointerTypeName}, {ConstStrings.MethodInfoPtrTypeName}, const {ConstStrings.StackObjectTypeName}*, {ConstStrings.StackObjectTypeName}*){ConstStrings.CppFunctionNoexcept};");
            _methodDeclsWriter.AddLine();
        }
    }
}
