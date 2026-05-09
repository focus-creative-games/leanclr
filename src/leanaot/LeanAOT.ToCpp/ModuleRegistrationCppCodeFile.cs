using dnlib.DotNet;
using LeanAOT.GenerationPlan;

namespace LeanAOT.ToCpp
{
    // class CodeFileBase
    // {
    //     protected C
    // }

    class ModuleRegistrationCppCodeFile
    {
        private readonly CodeWriter _totalWriter;

        private readonly CodeThunkZone _headCollection;
        private readonly CodeThrunkWriter _includeWriter;

        private readonly ForwardDeclaration _forwardDeclaration;

        private readonly CodeThunkZone _implCollection;
        private readonly CodeThrunkWriter _implWriter;

        private readonly ModuleDef _mod;
        private readonly AssemblyPlan _plan;

        private readonly bool _enableTypesLayoutValidation;
        private readonly List<TypeDef> _validateTypes;

        public ModuleRegistrationCppCodeFile(string filePath, AssemblyPlan plan)
        {
            _totalWriter = new CodeWriter(filePath);
            _plan = plan;
            _mod = plan.Module;
            _headCollection = _totalWriter.CreateThunkCollection("head");
            _includeWriter = _headCollection.CreateThunk("includes");
            _forwardDeclaration = new ForwardDeclaration(_totalWriter.CreateThunkCollection("forward_declarations"));
            _implCollection = _totalWriter.CreateThunkCollection("implementations");
            _implWriter = _implCollection.CreateThunk("implementations");
            _validateTypes = _mod.Types.Where(t => !t.HasGenericParameters && !t.IsEnum && (t.ToTypeSig().ElementType == ElementType.Class || t.ToTypeSig().ElementType == ElementType.ValueType)).ToList();
            _enableTypesLayoutValidation = GlobalServices.Inst.Config.EnableLayoutValidation && _validateTypes.Count > 0;
        }

        public void Generate()
        {
            AddIncludes();
            AddGlobalVariableDeclaration();
            AddModuleMethodDefDatasDeclaration();
            if (_enableTypesLayoutValidation)
            {
                AddTypesLayoutValidation();
            }
            AddModuleInitializationMethod();
            AddModuleDeferredInitializationMethod();
            AddModuleDataDeclaration();
        }

        private string GetMethodDefDatasVariableName()
        {
            return $"s_method_def_datas_{ModuleGenerationUtil.GetStandardizedModuleNameWithoutExt(_mod)}";
        }

        private void AddIncludes()
        {
            _includeWriter.AddLine($"#include \"{ModuleGenerationUtil.GetModuleRegistrationHeaderFileNameWithExt(_mod)}\"");
        }

        private void AddGlobalVariableDeclaration()
        {
            _implWriter.AddLine();
            _implWriter.AddLine($"{ConstStrings.ModulePtrTypeName} {ModuleGenerationUtil.GetModuleGlobalVariableName(_mod)} = nullptr;");
        }

        private void AddModuleInitializationMethod()
        {
            _implWriter.AddLine();
            _implWriter.AddLine($"void {ModuleGenerationUtil.GetModuleInitializeMethodName(_mod)}({ConstStrings.ModulePtrTypeName} mod){ConstStrings.CppFunctionNoexcept}");
            _implWriter.AddLine("{");
            _implWriter.IncreaseIndent();
            _implWriter.AddLine($"{ModuleGenerationUtil.GetModuleGlobalVariableName(_mod)} = mod;");
            _implWriter.DecreaseIndent();
            _implWriter.AddLine("}");
        }

        private void AddModuleDeferredInitializationMethod()
        {
            _implWriter.AddLine();
            _implWriter.AddLine($"void {ModuleGenerationUtil.GetModuleDeferredInitializeMethodName(_mod)}({ConstStrings.ModulePtrTypeName} mod){ConstStrings.CppFunctionNoexcept}");
            _implWriter.AddLine("{");
            _implWriter.IncreaseIndent();
            if (_enableTypesLayoutValidation)
            {
                _implWriter.AddLine($"{GetTypesLayoutValidationFunctionName()}(mod);");
            }
            _implWriter.DecreaseIndent();
            _implWriter.AddLine("}");
        }

        private void AddModuleMethodDefDatasDeclaration()
        {
            _implWriter.AddLine();
            _implWriter.AddLine($"static {ConstStrings.MethodDefDataTypeName} {GetMethodDefDatasVariableName()}[] = {{");
            _implWriter.IncreaseIndent();
            var invokerService = GlobalServices.Inst.InvokerService;
            if (_plan.MethodPlans.Count > 0)
            {
                foreach (var methodPlan in _plan.MethodPlans)
                {
                    MethodDef method = methodPlan.MethodDef;
                    _forwardDeclaration.AddMethodForwardDeclaration(method);
                    MethodInvokerInfo invoker = invokerService.GetInvoker(method);
                    _forwardDeclaration.AddInvokerForwardDeclaration(invoker);
                    MethodDetail md = GlobalServices.Inst.MetadataService.GetMethodDetail(method);
                    _implWriter.AddLine($"{{ 0x{method.MDToken.ToInt32():X8}, ({ConstStrings.ManagedMethodPointerTypeName}){md.UniqueName}, ({ConstStrings.ManagedMethodPointerTypeName}){(md.ShouldGenerateVirtualMethod ? md.VirtualMethodUniqueName : md.UniqueName)}, ({ConstStrings.InvokeMethodPointerTypeName}){invoker.name} }},");
                }
            }
            else
            {
                _implWriter.AddLine($"{{ 0x0, nullptr, nullptr, nullptr}},");
            }
            _implWriter.DecreaseIndent();
            _implWriter.AddLine("};");
        }

        private void AddModuleDataDeclaration()
        {
            _implWriter.AddLine();
            _implWriter.AddLine($"{ConstStrings.ModuleDataTypeName} {ModuleGenerationUtil.GetModuleGlobalDataVariableName(_mod)} = {{");
            _implWriter.IncreaseIndent();
            _implWriter.AddLine($"\"{ModuleGenerationUtil.GetModuleNameNoExt(_mod)}\",");
            _implWriter.AddLine($"{ModuleGenerationUtil.GetModuleInitializeMethodName(_mod)},");
            _implWriter.AddLine($"{ModuleGenerationUtil.GetModuleDeferredInitializeMethodName(_mod)},");
            _implWriter.AddLine($"{GetMethodDefDatasVariableName()},");
            _implWriter.AddLine($"{_plan.MethodPlans.Count},");
            _implWriter.DecreaseIndent();
            _implWriter.AddLine("};");
        }


        private string GetTypesLayoutValidationFunctionName()
        {
            return $"Validate_{ModuleGenerationUtil.GetStandardizedModuleNameWithoutExt(_mod)}_TypesLayout";
        }



        class TypeLayoutInfo
        {
            public TypeDef typeDef;
            public TypeDetail typeDetail;
        }


        private TypeLayoutInfo BuildTypeLayoutInfo(TypeDef typeDef)
        {
            return new TypeLayoutInfo
            {
                typeDef = typeDef,
                typeDetail = GlobalServices.Inst.MetadataService.GetTypeDetail(typeDef),
            };
        }

        private void AddDefineTypeLayoutInfoStruct()
        {
            _implWriter.AddLine();
            _implWriter.AddLine($"struct TypeLayoutInfo");
            _implWriter.AddLine("{");
            _implWriter.IncreaseIndent();
            _implWriter.AddLine("uint32_t token;");
            _implWriter.AddLine("const char* typeName;");
            _implWriter.AddLine("size_t class_instance_size;");
            _implWriter.AddLine("size_t class_static_size;");
            _implWriter.AddLine("size_t field_count;");
            _implWriter.AddLine("size_t* field_offsets;");
            _implWriter.DecreaseIndent();
            _implWriter.AddLine("};");
        }

        private void AddTypeFieldOffsets(TypeLayoutInfo typeLayoutInfo)
        {
            string fieldOffsetsVariableName = GetClassFieldOffsetsVariableName(typeLayoutInfo.typeDetail);
            var fields = typeLayoutInfo.typeDef.Fields;
            if (fields.Count == 0)
            {
                _implWriter.AddLine($"static size_t* {fieldOffsetsVariableName} = nullptr;");
            }
            else
            {
                _implWriter.AddLine($"static size_t {fieldOffsetsVariableName}[] = {{");
                _implWriter.IncreaseIndent();
                foreach (var field in fields)
                {
                    FieldDetail fd = typeLayoutInfo.typeDetail.GetFieldDetail(field);
                    if (!field.IsStatic)
                    {
                        _implWriter.AddLine($"offsetof({typeLayoutInfo.typeDetail.InstanceTypeName}, {fd.Name}){(typeLayoutInfo.typeDetail.IsValueType ? $" + {ConstStrings.CodegenNamespace}::RT_OBJECT_HEADER_SIZE" : "")}, // {field.Name}");
                    }
                    else
                    {
                        if (field.IsLiteral || field.HasFieldRVA)
                        {
                            // literal or rva field is treated as static field with offset 0 for layout validation, since it is not stored in instance object and does not affect instance size and field offsets.
                            _implWriter.AddLine($"0, // {field.Name} (literal or rva)");
                            continue;
                        }
                        // static field is treated as instance field with offset 0 for layout validation, since it is not stored in instance object and does not affect instance size and field offsets.
                        _implWriter.AddLine($"offsetof({typeLayoutInfo.typeDetail.StaticTypeName}, {fd.Name}), // {field.Name} (static)");
                    }
                }
                _implWriter.DecreaseIndent();
                _implWriter.AddLine("};");
            }
        }

        private void AddTypeLayoutInfo(TypeLayoutInfo typeLayoutInfo)
        {
            string fieldOffsetsVariableName = GetClassFieldOffsetsVariableName(typeLayoutInfo.typeDetail);
            // interface and `<Module>` types do not have object header, so we treat them as value types.
            _implWriter.AddLine($"{{ 0x{typeLayoutInfo.typeDef.MDToken.ToInt32():X8}, \"{typeLayoutInfo.typeDef.FullName}\", sizeof({typeLayoutInfo.typeDetail.InstanceTypeName}){(typeLayoutInfo.typeDetail.IsValueType || typeLayoutInfo.typeDef.BaseType == null ? $" + {ConstStrings.CodegenNamespace}::RT_OBJECT_HEADER_SIZE" : "")},  sizeof({typeLayoutInfo.typeDetail.StaticTypeName}), {typeLayoutInfo.typeDef.Fields.Count}, {fieldOffsetsVariableName} }},");
        }

        private string GetClassFieldOffsetsVariableName(TypeDetail typeDetail)
        {
            return $"s_field_offsets_{typeDetail.InstanceTypeName}";
        }

        private string GetClassesLayoutInfoArrayVariableName()
        {
            return $"s_class_layout_infos_{ModuleGenerationUtil.GetStandardizedModuleNameWithoutExt(_mod)}";
        }

        private void AddTypesLayoutValidationFunction()
        {
            _implWriter.AddLine();
            _implWriter.AddLine($"void {GetTypesLayoutValidationFunctionName()}({ConstStrings.ModulePtrTypeName} mod){ConstStrings.CppFunctionNoexcept}");
            _implWriter.AddLine("{");
            _implWriter.IncreaseIndent();
            _implWriter.AddLine($"for (const TypeLayoutInfo& info : {GetClassesLayoutInfoArrayVariableName()})");
            _implWriter.AddLine("{");
            _implWriter.IncreaseIndent();
            _implWriter.AddLine($"{ConstStrings.ClassPtrTypeName} klass = ({ConstStrings.ClassPtrTypeName}){ConstStrings.CodegenNamespace}::resolve_metadata_token(mod, info.token, nullptr);");
            _implWriter.AddLine($"assert(klass != nullptr && \"Type not found for layout validation\");");
            _implWriter.AddLine($"// assert(({ConstStrings.CodegenNamespace}::get_class_instance_size_with_object_header(klass) == info.class_instance_size || {ConstStrings.CodegenNamespace}::get_class_instance_size_without_object_header(klass) == 0) && \"Class instance size mismatch\");");
            _implWriter.AddLine($"assert(({ConstStrings.CodegenNamespace}::get_class_static_size(klass) == info.class_static_size || {ConstStrings.CodegenNamespace}::get_class_static_size(klass) == 0) && \"Class static size mismatch\");");
            _implWriter.AddLine($"assert({ConstStrings.CodegenNamespace}::get_class_field_count(klass) == info.field_count && \"Class field count mismatch\");");
            _implWriter.AddLine($"for(size_t i = 0; i < info.field_count; i++)");
            _implWriter.AddLine("{");
            _implWriter.IncreaseIndent();
            _implWriter.AddLine($"assert({ConstStrings.CodegenNamespace}::get_field_offset_includes_object_header(klass->fields + i) == info.field_offsets[i] && \"Field offset mismatch\");");
            _implWriter.DecreaseIndent();
            _implWriter.AddLine("}");
            _implWriter.DecreaseIndent();
            _implWriter.AddLine("}");
            _implWriter.DecreaseIndent();
            _implWriter.AddLine("}");
        }

        private void AddTypesLayoutValidation()
        {
            _implWriter.AddLine();
            AddDefineTypeLayoutInfoStruct();

            _implWriter.AddLine();

            foreach (TypeDef type in _validateTypes)
            {
                _forwardDeclaration.AddTypeForwardDefine(type);
                var typeLayoutInfo = BuildTypeLayoutInfo(type);
                AddTypeFieldOffsets(typeLayoutInfo);
            }

            _implWriter.AddLine();
            _implWriter.AddLine($"static TypeLayoutInfo {GetClassesLayoutInfoArrayVariableName()}[] = {{");
            _implWriter.IncreaseIndent();
            foreach (TypeDef type in _validateTypes)
            {
                var typeLayoutInfo = BuildTypeLayoutInfo(type);
                AddTypeLayoutInfo(typeLayoutInfo);
            }
            _implWriter.DecreaseIndent();
            _implWriter.AddLine("};");

            AddTypesLayoutValidationFunction();
        }

        public void Save()
        {
            _totalWriter.Save();
        }
    }
}
