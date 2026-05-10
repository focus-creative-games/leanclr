namespace LeanAOT.ToCpp
{
    static class VmFunctionNames
    {
        public const string Memset = "std::memset";
        public const string Memcpy = "std::memcpy";
        public const string IsFinite = "std::isfinite";
        public const string IsNan = "std::isnan";
        public const string FMod = "std::fmod";

        public const string CastFloatToSmallInt = "leanclr::codegen::cast_float_to_small_int";

        public const string CastFloatToI32 = "leanclr::codegen::cast_float_to_i32";
        public const string CastFloatToI64 = "leanclr::codegen::cast_float_to_i64";
        public const string CastFloatToIntPtr = "leanclr::codegen::cast_float_to_intptr";

        public const string Localloc = "alloca";

        public const string SelectArch = "leanclr::codegen::select_arch";

        public const string Ctor = ".ctor";
        public const string CCtor = ".cctor";

        public const string ResolveMetadataTokens = "leanclr::codegen::resolve_metadata_tokens";

        public const string IsCctorNotFinishied = "leanclr::codegen::is_cctor_not_finished";
        public const string RunClassStaticConstructor = "leanclr::codegen::run_class_static_constructor";
        public const string NewObj = "leanclr::codegen::new_object";
        public const string GetVirtualMethodOnObj = "leanclr::codegen::get_virtual_method_impl";
        public const string IsInst = "leanclr::codegen::is_inst";
        public const string CastClass = "leanclr::codegen::cast_class";
        public const string IsAssignableFrom = "leanclr::codegen::is_assignable_from";
        public const string Box = "leanclr::codegen::box_object";
        public const string Unbox = "leanclr::codegen::unbox_ex";
        public const string UnboxAny = "leanclr::codegen::unbox_any";
        public const string IsValueType = "leanclr::codegen::is_value_type";
        public const string NewSZArrayFromEleKlass = "leanclr::codegen::new_szarray_from_ele_class";
        public const string NewSZArrayFromArrayKlass = "leanclr::codegen::new_szarray_from_array_class";
        public const string NewMdArrayFromEleKlass = "leanclr::codegen::new_mdarray_from_ele_class";
        public const string NewMdArrayFromArrayKlass = "leanclr::codegen::new_mdarray_from_array_class";
        public const string GetArrayLength = "leanclr::codegen::get_array_length";
        public const string GetArrayElementKlass = "leanclr::codegen::get_array_element_class";
        public const string IsArrayIndexOutOfRange = "leanclr::codegen::is_array_index_out_of_range";
        public const string IsPointerElementCompatibleWith = "leanclr::codegen::is_pointer_element_compatible_with";
        public const string GetArrayElementAddress = "leanclr::codegen::get_array_element_address";
        public const string GetArrayElementDataAt = "leanclr::codegen::get_array_element_data_at";
        public const string SetArrayElementDataAt = "leanclr::codegen::set_array_element_data_at";
        public const string GetMdArrayGlobalIndex = "leanclr::codegen::get_mdarray_global_index_from_indices";
        public const string NewDelegate = "leanclr::codegen::new_delegate";
        public const string GetFieldRvaData = "leanclr::codegen::get_field_rva_data";

        public const string GetStackObjectSizeForType = "leanclr::codegen::get_stack_object_size_for_type";
        public const string ExpandArgumentToEvalStack = "leanclr::codegen::expand_argument_to_eval_stack";

        public const string InvokeWithRunClassStaticConstructor = "leanclr::codegen::invoke_with_run_class_static_constructor";
        public const string InvokeWithoutRunClassStaticConstructor = "leanclr::codegen::invoke_without_run_class_static_constructor";
        public const string VirtualInvokeWithoutRunClassStaticConstructor = "leanclr::codegen::virtual_invoke_without_run_class_static_constructor";

        public const string SetRetOrReturnError = "leanclr::codegen::set_ret_or_return_error";

        public const string IsAotMethod = "leanclr::codegen::is_aot_method";

        public const string Likely = "LEANCLR_CODEGEN_LIKELY";
        public const string Unlikely = "LEANCLR_CODEGEN_UNLIKELY";
        public const string Assume = "LEANCLR_CODEGEN_ASSUME";
        public const string AssumeNotNull = "LEANCLR_CODEGEN_ASSUME_NOT_NULL";

        public const string RET_VALUE = "LEANCLR_CODEGEN_RETURN";
        public const string RET_ERROR = "LEANCLR_CODEGEN_RETURN_ERR";
        public const string RET_VOID = "LEANCLR_CODEGEN_RETURN_VOID";

        public const string THROW_ON_ERROR = "LEANCLR_CODEGEN_THROW_ON_ERROR";
        public const string DECLARING_ASSIGN_OR_THROW = "LEANCLR_CODEGEN_DECLARING_ASSIGN_OR_THROW_ON_ERROR";
        public const string ASSIGN_OR_THROW = "LEANCLR_CODEGEN_ASSIGN_OR_THROW_ON_ERROR";
        public const string THROW_RUNTIME_ERROR = "LEANCLR_CODEGEN_THROW_RUNTIME_ERROR";
        public const string CHECK_NULL_REFERENCE = "LEANCLR_CODEGEN_CHECK_NOT_NULL_OR_THROW_NULL_REFERENCE_EXCEPTION";
        public const string THROW_EXCEPTION = "LEANCLR_CODEGEN_THROW_EXCEPTION";
    }
}