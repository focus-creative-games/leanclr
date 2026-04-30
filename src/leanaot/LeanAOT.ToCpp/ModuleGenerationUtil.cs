using dnlib.DotNet;
using LeanAOT.Core;

namespace LeanAOT.ToCpp
{
    public static class ModuleGenerationUtil
    {
        public static string GetModuleRegistrationHeaderFileNameWithExt(ModuleDef mod)
        {
            return $"{GetStandardizedModuleNameWithoutExt(mod)}.module_registration.h";
        }

        public static string GetModuleRegistrationCppFileNameWithExt(ModuleDef mod)
        {
            return $"{GetStandardizedModuleNameWithoutExt(mod)}.module_registration.cpp";
        }

        public static string GetModuleMethodBodyFileNameWithExt(ModuleDef mod, int partIndex)
        {
            return $"{GetStandardizedModuleNameWithoutExt(mod)}.method_body_part{partIndex}.cpp";
        }

        public static string GetMethodInvokerCppFileNameWithExt(int partIndex)
        {
            return $"method_invokers_part{partIndex}.cpp";
        }

        public static string GetAllModuleRegistrationCppFileNameWithExt()
        {
            return $"modules_registration.cpp";
        }

        public static string GetStandardizedModuleNameWithoutExt(ModuleDef mod)
        {
            return mod.Assembly.Name.Replace('.', '_').Replace('-', '_');
        }

        public static string GetModuleNameNoExt(ModuleDef mod)
        {
            return mod.Assembly.Name;
        }

        public static string GetModuleGlobalVariableName(ModuleDef mod)
        {
            return $"g_module_{GetStandardizedModuleNameWithoutExt(mod)}";
        }

        public static string GetModuleGlobalDataVariableName(ModuleDef mod)
        {
            return $"g_module_data_{GetStandardizedModuleNameWithoutExt(mod)}";
        }

        public static string GetModuleForwardDeclaration(ModuleDef mod)
        {
            return $"extern {ConstStrings.ModulePtrTypeName} {GetModuleGlobalVariableName(mod)};";
        }

        public static string GetModuleDefinitionInitialization(ModuleDef mod)
        {
            return $"{ConstStrings.ModulePtrTypeName} {GetModuleGlobalVariableName(mod)} = nullptr;";
        }

        public static string GetModuleInitializeMethodName(ModuleDef mod)
        {
            return $"initialize_module_{GetStandardizedModuleNameWithoutExt(mod)}";
        }

        public static string GetModuleDeferredInitializeMethodName(ModuleDef mod)
        {
            return $"deferred_initialize_module_{GetStandardizedModuleNameWithoutExt(mod)}";
        }

        public static string GetModuleInitializeMethodDeclaration(ModuleDef mod)
        {
            return $"void {GetModuleInitializeMethodName(mod)}({ConstStrings.ModulePtrTypeName} mod){ConstStrings.CppFunctionNoexcept};";
        }

        public static string GetGlobalAotModulesDataVariableName()
        {
            return "g_aot_modules_data";
        }

        // public static string GetAllModuleInitializationMethodDeclaration()
        // {
        //     return $"void {GetAllModuleInitializationMethodName()}();";
        // }
    }
}
