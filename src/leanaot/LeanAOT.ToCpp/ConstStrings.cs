namespace LeanAOT.ToCpp
{
    static class ConstStrings
    {
        public const string CodegenCommonHeaderFileName = "codegen/leanclr_common.h";
        public const string CodegenNamespace = "leanclr::codegen";

        public const string RtResultVoidTypeName = "leanclr::RtResultVoid";
        public const string RtResultTypeName = "leanclr::RtResult";

        public const string RtErrTypeName = "leanclr::RtErr";

        public const string MethodDefDataTypeName = "leanclr::metadata::RtAotMethodDefData";
        public const string MonoPInvokeCallbackDataTypeName = "leanclr::metadata::RtAotMethodMonoPInvokeCallbackData";
        public const string ModuleDataTypeName = "leanclr::metadata::RtAotModuleData";
        public const string ModulesDataDataTypeName = "leanclr::metadata::RtAotModulesData";

        public const string MethodInfoParameterName = "___methodInfo";
        public const string ManagedMethodPointerTypeName = "leanclr::metadata::RtManagedMethodPointer";
        public const string InvokeMethodPointerTypeName = "leanclr::metadata::RtInvokeMethodPointer";
        public const string NativeMethodPointerTypeName = "leanclr::metadata::RtNativeMethodPointer";

        public const string MethodPointerFieldName = "method_ptr";
        public const string VirtualMethodPointerFieldName = "virtual_method_ptr";
        public const string InvokerMethodPointerFieldName = "invoker_method_ptr";
        public const string ParentFieldName = "parent";

        public const string IntPtrTypeName = "intptr_t";
        public const string UIntPtrTypeName = "uintptr_t";
        public const string Float32TypeName = "float32_t";
        public const string Float64TypeName = "float64_t";

        public const string ModulePtrTypeName = "leanclr::metadata::RtModuleDef*";
        public const string ClassPtrTypeName = "leanclr::metadata::RtClass*";
        public const string MethodInfoPtrTypeName = "const leanclr::metadata::RtMethodInfo*";
        public const string FieldInfoPtrTypeName = "const leanclr::metadata::RtFieldInfo*";
        public const string TypeSigPtrTypeName = "const leanclr::metadata::RtTypeSig*";

        public const string StackObjectTypeName = "leanclr::interp::RtStackObject";

        public const string ObjectPtrTypeName = "leanclr::vm::RtObject*";
        public const string StringPtrTypeName = "leanclr::vm::RtString*";
        public const string ArrayPtrTypeName = "leanclr::vm::RtArray*";
        public const string DelegatePtrTypeName = "leanclr::vm::RtDelegate*";
        public const string TypedByRefTypeName = "leanclr::vm::RtTypedReference";

        public const string ObjectTypeName = "leanclr::vm::RtObject";
        public const string StringTypeName = "leanclr::vm::RtString";
        public const string ArrayTypeName = "leanclr::vm::RtArray";
        public const string DelegateTypeName = "leanclr::vm::RtDelegate";

        public const string errNameNullReference = "NullReference";

        public const string KlassFieldNameStaticFieldsData = "static_fields_data";

        public const string CppFunctionNoexcept = " noexcept";

        public const string InternalDllName = "__Internal";

        // marshal types
        public const string MarshalBooleanTypeName = "leanclr::metadata::RtMarshalBoolean";
        public const string MarshalVariantBoolTypeName = "leanclr::metadata::RtMarshalVariantBool";
        public const string MarshalAnsiCharTypeName = "leanclr::NativeChar";
        public const string MarshalUtf16CharTypeName = "leanclr::Utf16Char";
        public const string MarshalCustomMarshalerTypeName = "leanclr::metadata::RtMarshalCustomMarshaler";
        public const string MarshalErrorTypeName = "leanclr::metadata::RtMarshalError";
        public const string MarshalIInspectableTypeName = "leanclr::metadata::RtMarshalIInspectable";
        public const string MarshalHStringTypeName = "leanclr::metadata::RtMarshalHString";
        public const string MarshalIUnknownTypeName = "leanclr::metadata::RtMarshalIUnknown";
        public const string MarshalIDispatchTypeName = "leanclr::metadata::RtMarshalIDispatch";
        public const string MarshalInterfaceTypeName = "leanclr::metadata::RtMarshalInterface";
        public const string MarshalSafeArrayPtrTypeName = "leanclr::metadata::RtSafeArray*";
        public const string MarshalHandleTypeName = "leanclr::metadata::RtMarshalHandle";
        public const string MarshalAnsiStrTypeName = "leanclr::codegen::RtMarshalAnsiStr";
        public const string MarshalUTF16StrTypeName = "leanclr::codegen::RtMarshalUTF16Str";
        public const string MarshalUTF8StrTypeName = "leanclr::codegen::RtMarshalUTF8Str";

        public const string StringBuilderTypeName = "leanclr::utils::StringBuilder";
    }
}