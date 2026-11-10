using dnlib.DotNet;
using LeanAOT.Core;

namespace LeanAOT.ToCpp
{
    partial class ModuleRegistrationCppCodeFile
    {
        private string GenericEntriesName => "s_generic_methods_" + ModuleGenerationUtil.GetStandardizedModuleNameWithoutExt(_mod);
        private int _identityId;

        private string WriteIdentities(IList<TypeSig> types)
        {
            if (types == null || types.Count == 0) return "nullptr";
            var values = new List<string>();
            foreach (var type in types)
            {
                IList<TypeSig> children = type is GenericInstSig gi ? gi.GenericArguments :
                    type.ElementType == ElementType.SZArray ? new[] { type.Next } : null;
                string childName = WriteIdentities(children);
                var def = type is GenericInstSig generic ? generic.GenericType.TypeDefOrRef.ResolveTypeDef() :
                    type is ClassOrValueTypeSig named ? named.TypeDefOrRef.ResolveTypeDef() : null;
                string module = def == null ? "nullptr" : "\"" + def.Module.Assembly.Name + "\"";
                values.Add($"{{ static_cast<leanclr::metadata::RtElementType>({(int)type.ElementType}), {module}, 0x{(def?.MDToken.Raw ?? 0):X8}, {childName}, {children?.Count ?? 0} }}");
            }
            string name = $"s_generic_identity_{ModuleGenerationUtil.GetStandardizedModuleNameWithoutExt(_mod)}_{_identityId++}";
            _implWriter.AddLine($"static const leanclr::metadata::RtAotTypeIdentity {name}[] = {{ {string.Join(", ", values)} }};");
            return name;
        }

        private void AddGenericMethodEntries()
        {
            if (_plan.GenericMethodPlans.Count == 0) return;
            var entries = new List<string>();
            foreach (var plan in _plan.GenericMethodPlans)
            {
                var method = plan.Method;
                var detail = GlobalServices.Inst.MetadataService.GetMethodDetail(method);
                var ctx = MetaUtil.GetMethodGenericArgumentContext(method);
                string classes = WriteIdentities(ctx.typeArgsStack);
                string methods = WriteIdentities(ctx.methodArgsStack);
                _forwardDeclaration.AddMethodForwardDeclaration(method);
                var invoker = GlobalServices.Inst.InvokerService.GetInvoker(method);
                _forwardDeclaration.AddInvokerForwardDeclaration(invoker);
                entries.Add($"{{ {{ 0x{detail.MethodDef.MDToken.Raw:X8}, (leanclr::metadata::RtManagedMethodPointer){detail.UniqueName}, (leanclr::metadata::RtManagedMethodPointer){(detail.ShouldGenerateVirtualMethod ? detail.VirtualMethodUniqueName : detail.UniqueName)}, {invoker.name} }}, {classes}, {ctx.typeArgsStack?.Count ?? 0}, {methods}, {ctx.methodArgsStack?.Count ?? 0} }}");
            }
            _implWriter.AddLine($"static const leanclr::metadata::RtAotGenericMethodData {GenericEntriesName}[] = {{");
            foreach (string entry in entries) _implWriter.AddLine(entry + ",");
            _implWriter.AddLine("};");
        }
    }
}
