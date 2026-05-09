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
        private readonly HashSet<string> _addedTypeDecls = new HashSet<string>();
        private readonly HashSet<ITypeDefOrRef> _addedTypes = new HashSet<ITypeDefOrRef>(TypeEqualityComparer.Instance);
        private readonly HashSet<MethodInvokerInfo> _addedInvokers = new HashSet<MethodInvokerInfo>();

        private readonly HashSet<(string, string)> _addedPinvokeEntries = new HashSet<(string, string)>();

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

        // File-scope extern "C" for static P/Invoke (not inside a method — avoids Clang/Emscripten parse issues).
        public void AddPInvokeNativeExternDeclaration(string dllNameNoExt, string standardedDllLiteral, string externDeclLine)
        {
            if (!_addedPinvokeEntries.Add((dllNameNoExt, externDeclLine)))
            {
                return;
            }
            if (dllNameNoExt == ConstStrings.InternalDllName)
            {
                _methodDeclsWriter.AddLine(externDeclLine);
                return;
            }
            _methodDeclsWriter.AddLine($"#if FORCE_PINVOKE_INTERNAL || FORCE_PINVOKE_{standardedDllLiteral}_INTERNAL");
            _methodDeclsWriter.AddLine(externDeclLine);
            _methodDeclsWriter.AddLine("#endif");
        }

        public void AddCommonIncludes(ModuleDef mod)
        {
            AddInclude($"{ModuleGenerationUtil.GetModuleRegistrationHeaderFileNameWithExt(mod)}");
        }

        public void AddInclude(string include)
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

        private void AddTypeDeclaration(TypeDetail type)
        {
            if (!ShouldGenerateStructForType(type))
            {
                return;
            }
            if (!_addedTypeDecls.Add(type.InstanceTypeName))
                return;
            _typeDeclsWriter.AddLine($"struct {type.InstanceTypeName};");
        }


        private bool ShouldGenerateStructForType(TypeDetail type)
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
                case ElementType.GenericInst:
                case ElementType.TypedByRef:
                    return true;
                default: return false;
                }
            }
            return false;
        }

        private void AddTypeNotStaticDefinition(TypeDetail type, CodeThrunkWriter typeDefinesWriter)
        {
            TypeDef typeDef = type.TypeDef;
            if (!ShouldGenerateStructForType(type))
            {
                return;
            }

            AddTypeDeclaration(type);
            uint packingSize = typeDef.ClassLayout != null ? typeDef.PackingSize : 0u;
            uint classSize = typeDef.ClassLayout != null ? typeDef.ClassSize : 0;
            // ignore class size and packing for reference types, as they don't have instance fields and their static fields are laid out by the runtime
            if (!MetaUtil.IsValueType(type.TypeSig))
            {
                classSize = 0;
                packingSize = 0;
            }
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

        private void AddTypeStaticDefinition(TypeDetail type, CodeThrunkWriter typeDefineWriter)
        {
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
        }

        private void AddTypeDefinitionOnlyForStruct(TypeSig typeSig)
        {
            if (MetaUtil.IsEnumType(typeSig))
            {
                // don't generate forward declaration or defines or static definitions for enum types
                return;
            }
            typeSig = typeSig.RemovePinnedAndModifiers();
            ITypeDefOrRef type = typeSig.ToTypeDefOrRef();
            switch (typeSig.ElementType)
            {
            case ElementType.Class:
            case ElementType.String:
            {
                TypeDetail typeDetail = _metadataService.GetTypeDetail(type);
                AddTypeDeclaration(typeDetail);
                break;
            }
            case ElementType.ValueType:
            case ElementType.TypedByRef:
            {
                TypeDef typeDef = type.ResolveTypeDefThrow();
                if (typeDef.HasGenericParameters)
                {
                    return;
                }
                TypeDetail typeDetail = _metadataService.GetTypeDetail(type);
                AddTypeDefinitionImpl(typeDetail);
                break;
            }
            case ElementType.GenericInst:
            {
                GenericInstSig genericInstSig = (GenericInstSig)typeSig;
                if (genericInstSig.GenericArguments.Any(arg => arg.ContainsGenericParameter))
                {
                    return;
                }
                TypeDetail typeDetail = _metadataService.GetTypeDetail(type);
                if (!typeDetail.IsValueType)
                {
                    AddTypeDeclaration(typeDetail);
                }
                else
                {
                    AddTypeDefinitionImpl(typeDetail);
                }
                break;
            }
            case ElementType.Ptr:
            case ElementType.ByRef:
            {
                AddTypeForwardDefineAny(typeSig.Next, true);
                break;
            }
            }
        }

        private void AddTypeDefinitionImpl(TypeDetail type)
        {
            AddTypeDeclaration(type);
            if (!_addedTypes.Add(type.Type))
            {
                return;
            }
            foreach (var field in type.InstanceFieldsIncludeParent)
            {
                AddTypeDefinitionOnlyForStruct(field.Type);
            }
            foreach (var field in type.StaticFields)
            {
                AddTypeDefinitionOnlyForStruct(field.Type);
            }

            var typeDefineWriter = type.TypeDef != null && type.TypeDef.IsValueType ? _structDefinesWriter : _classDefinesWriter;
            AddTypeNotStaticDefinition(type, typeDefineWriter);

            typeDefineWriter.AddLine();
            AddTypeStaticDefinition(type, typeDefineWriter);

            typeDefineWriter.AddLine();
        }

        public void AddTypeForwardDefine(ITypeDefOrRef type)
        {
            AddTypeForwardDefineAny(type.ToTypeSig(), false);
        }

        public void AddTypeForwardDefine(TypeSig typeSig)
        {
            AddTypeForwardDefineAny(typeSig, false);
        }

        private void AddTypeForwardDefineAny(TypeSig typeSig, bool declaringOnly)
        {
            if (MetaUtil.IsEnumType(typeSig))
            {
                // don't generate forward declaration or defines or static definitions for enum types
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
                // if (!declaringOnly && !staticOnly)
                // {
                //     ITypeDefOrRef baseType = type.GetBaseType();
                //     if (baseType != null)
                //     {
                //         AddTypeForwardDefineAny(baseType.ToTypeSig(), false, false);
                //     }
                // }
                TypeDetail typeDetail = _metadataService.GetTypeDetail(type);
                AddTypeDeclaration(typeDetail);
                if (!declaringOnly)
                {
                    AddTypeDefinitionImpl(typeDetail);
                }
                break;
            }
            case ElementType.GenericInst:
            {
                // if (!_addedTypes.Add(type))
                // {
                //     break;
                // }

                // ITypeDefOrRef baseType = type.GetBaseType();
                // if (baseType != null)
                // {
                //     AddTypeForwardDefine(baseType);
                // }
                GenericInstSig genericInstSig = (GenericInstSig)typeSig;
                if (genericInstSig.GenericArguments.Any(arg => arg.ContainsGenericParameter))
                {
                    return;
                }

                // bool hasGenericParam = false;
                // foreach (var arg in genericInstSig.GenericArguments)
                // {
                //     AddTypeForwardDefine(arg);
                //     hasGenericParam = hasGenericParam || arg.ContainsGenericParameter;
                // }
                // Debug.Assert(!hasGenericParam);
                //AddTypeDefinition(_metadataService.GetTypeDetail(genericType));
                // if (hasGenericParam)
                // {
                //     return;
                // }
                TypeDetail typeDetail = _metadataService.GetTypeDetail(type);
                AddTypeDeclaration(typeDetail);
                if (!declaringOnly)
                {
                    AddTypeDefinitionImpl(typeDetail);
                }
                break;
            }
            case ElementType.Ptr:
            case ElementType.ByRef:
            {
                AddTypeForwardDefineAny(typeSig.Next, true);
                break;
            }
            case ElementType.Array:
            case ElementType.SZArray:
            {
                AddTypeForwardDefineAny(typeSig.Next, true);
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
                if (!declaringOnly)
                {
                    AddTypeDefinitionImpl(_metadataService.GetTypeDetail(typeDef));
                }
                break;
            }
            }
        }

        public void AddFieldForwardDeclaration(IField field)
        {
            var fieldDetail = _metadataService.GetFieldDetail(field);
            AddTypeForwardDefine(fieldDetail.ParentType);
            AddTypeForwardDefine(fieldDetail.Type);
        }

        public void AddMethodForwardDeclaration(IMethod method)
        {
            if (!_addedMethods.Add(method))
                return;
            MethodDetail methodDetail = _metadataService.GetMethodDetail(method);
            AddTypeForwardDefine(methodDetail.RetType);
            foreach (var param in methodDetail.ParamsIncludeThis)
            {
                AddTypeForwardDefine(param.Type);
            }
            MethodDef methodDef = methodDetail.MethodDef;
            if (methodDef == null || methodDef.IsAbstract)
            {
                return;
            }
            if (methodDef.Name == VmFunctionNames.Ctor)
            {
                AddTypeForwardDefine(methodDetail.DeclaringType);
            }

            if (!_manifestService.ShouldAOT(method))
            {
                return;
            }
            _methodDeclsWriter.AddLine($"{methodDetail.GenerateMethodDeclaring()};");
            if (methodDetail.ShouldGenerateVirtualMethod)
            {
                _methodDeclsWriter.AddLine($"{methodDetail.GenerateVirtualMethodDeclaring()};");
            }
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
