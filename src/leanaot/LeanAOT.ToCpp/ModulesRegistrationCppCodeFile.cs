using dnlib.DotNet;

namespace LeanAOT.ToCpp
{
    class ModulesRegistrationCppCodeFile
    {
        private readonly CodeWriter _totalWriter;

        private readonly CodeThunkZone _headZone;
        private readonly CodeThrunkWriter _includeWriter;

        private readonly CodeThunkZone _bodyZone;
        private readonly CodeThrunkWriter _implWriter;

        private readonly List<ModuleDef> _mods;

        public ModulesRegistrationCppCodeFile(string filePath, List<ModuleDef> mods)
        {
            _totalWriter = new CodeWriter(filePath);

            _headZone = _totalWriter.CreateThunkCollection("head");
            _includeWriter = _headZone.CreateThunk("includes");
            _bodyZone = _totalWriter.CreateThunkCollection("body");
            _implWriter = _bodyZone.CreateThunk("implementations");
            _mods = mods;
        }

        public void Generate()
        {
            AddIncludes();
            AddModuleListInitialization();
            AddModulesInitialization();
        }

        private void AddIncludes()
        {
            foreach (var mod in _mods)
            {
                _includeWriter.AddLine($"#include \"{ModuleGenerationUtil.GetModuleRegistrationHeaderFileNameWithExt(mod)}\"");
            }
        }

        private string GetModuleListInitializationVariableName()
        {
            return $"s_aot_modules";
        }

        private void AddModuleListInitialization()
        {
            _implWriter.AddLine();

            _implWriter.AddLine($"static const {ConstStrings.ModuleDataTypeName}* {GetModuleListInitializationVariableName()}[] = {{");
            _implWriter.IncreaseIndent();
            foreach (var mod in _mods)
            {
                _implWriter.AddLine($"&{ModuleGenerationUtil.GetModuleGlobalDataVariableName(mod)},");
            }
            if (_mods.Count == 0)
            {
                _implWriter.AddLine($"nullptr,");
            }
            _implWriter.DecreaseIndent();
            _implWriter.AddLine("};");
        }

        private void AddModulesInitialization()
        {
            _implWriter.AddLine();
            int genericCount = GlobalServices.Inst.ManifestService.Manifest.GenericMethodPlans.Count;
            if (genericCount > 0)
                _implWriter.AddLine("extern const leanclr::metadata::RtAotGenericMethodData g_aot_generic_methods[];");
            _implWriter.AddLine($"{ConstStrings.ModulesDataDataTypeName} {ModuleGenerationUtil.GetGlobalAotModulesDataVariableName()} = {{");
            _implWriter.IncreaseIndent();
            _implWriter.AddLine($"{GetModuleListInitializationVariableName()},");
            _implWriter.AddLine($"{_mods.Count},");
            _implWriter.AddLine(genericCount > 0 ? "g_aot_generic_methods," : "nullptr,");
            _implWriter.AddLine($"{genericCount},");
            _implWriter.DecreaseIndent();
            _implWriter.AddLine("};");
        }

        public void Save()
        {
            _totalWriter.Save();
        }
    }
}
