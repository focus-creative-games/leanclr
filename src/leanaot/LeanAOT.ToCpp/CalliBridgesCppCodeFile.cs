using dnlib.DotNet;
using dnlib.DotNet.Emit;
using LeanAOT.Core;
using LeanAOT.GenerationPlan;
using System.Text;
using System.Text.Json;

namespace LeanAOT.ToCpp;

// Scans IL independently of method-body selection, including excluded methods.
// One table belongs to the whole generation rather than any defining assembly.
sealed class CalliBridgesCppCodeFile
{
    record Abi(int Kind, string Cpp, TypeDetail Detail = null, List<(FieldDetail Field, Abi Type)> Fields = null)
    {
        public string Key => Kind == 17 ? "s" + Cpp : Kind.ToString();
    }
    record Bridge(Abi Ret, List<Abi> Args) { public string Key => "1:" + Ret.Key + "(" + string.Join(",", Args.Select(a => a.Key)) + ")"; }
    readonly SortedDictionary<string, Bridge> bridges = new(StringComparer.Ordinal);
    readonly List<object> sites = new();
    static Abi Type(TypeSig type, bool ret = false)
    {
        if (type is ModifierSig)
        {
            var raw = type;
            while (raw is CModReqdSig m && m.Modifier.FullName is ("System.Runtime.InteropServices.InAttribute" or "System.Runtime.InteropServices.OutAttribute")) raw = raw.Next;
            if (raw.ElementType == ElementType.ByRef) type = raw;
        }
        if (type == null || type is ModifierSig || type is PinnedSig) throw new NotSupportedException("type-modifier");
        if (type.ElementType == ElementType.ByRef)
        {
            var payload = Type(type.Next); if (payload.Kind is 2 or 3) throw new NotSupportedException("byref-bool-char-marshalling"); return new Abi(15, "void*");
        }
        if (type.ElementType == ElementType.Ptr || type.ElementType == ElementType.I || type.ElementType == ElementType.U) return new Abi(15, "void*");
        if (type.ElementType == ElementType.ValueType && type.ToTypeDefOrRef().ResolveTypeDef() is { IsEnum: true } e)
            return Type(e.Fields.First(f => !f.IsStatic).FieldType);
        if (type.ElementType is ElementType.ValueType or ElementType.GenericInst)
        {
            if (!CalliPolicy.Type(type)) throw new NotSupportedException("non-blittable-struct:" + type.FullName);
            var detail = GlobalServices.Inst.MetadataService.GetTypeDetail(type.ToTypeDefOrRef());
            return new Abi(17, detail.InstanceTypeName, detail, detail.InstanceFieldsExcludeParent.Select(f => (f, Type(f.Type))).ToList());
        }
        return type.ElementType switch
        {
            ElementType.Void when ret => new Abi(1, "void"),
            ElementType.Boolean => new Abi(2, "int32_t"),
            ElementType.Char => new Abi(3, "uint8_t"),
            ElementType.I1 => new Abi(4, "int8_t"),
            ElementType.U1 => new Abi(5, "uint8_t"),
            ElementType.I2 => new Abi(6, "int16_t"),
            ElementType.U2 => new Abi(7, "uint16_t"),
            ElementType.I4 => new Abi(8, "int32_t"),
            ElementType.U4 => new Abi(9, "uint32_t"),
            ElementType.I8 => new Abi(10, "int64_t"),
            ElementType.U8 => new Abi(11, "uint64_t"),
            ElementType.R4 => new Abi(12, "float"),
            ElementType.R8 => new Abi(13, "double"),
            _ => throw new NotSupportedException("unsupported-ABI-type:" + type.FullName)
        };
    }
    static Bridge Parse(MethodSig sig)
    {
        int conv = (byte)sig.CallingConvention & 15;
        if (((byte)sig.CallingConvention & 0xf0) != 0 || sig.GenParamCount != 0 || sig.Params.Count > 256 || sig.ParamsAfterSentinel?.Count > 0)
            throw new NotSupportedException("unmanaged-signature-flags");
        var ret = sig.RetType;
        if (conv == 9)
        {
            int specified = 0;
            while (ret is ModifierSig modifier)
            {
                string name = modifier.Modifier.FullName;
                int c = name == "System.Runtime.CompilerServices.CallConvCdecl" ? 1 : name == "System.Runtime.CompilerServices.CallConvStdcall" ? 2 : 0;
                if (modifier is CModReqdSig || c == 0 || specified != 0) throw new NotSupportedException("unsupported-callconv-modifier:" + name);
                specified = c; ret = ret.Next;
            }
        }
        else if (conv != 1 && conv != 2) throw new NotSupportedException("unsupported-native-convention");
        return new Bridge(Type(ret, true), sig.Params.Select(p => Type(p)).ToList());
    }
    void Scan(MethodDef method, GenericArgumentContext context = null)
    {
        if (!method.HasBody) return;
        foreach (var instruction in method.Body.Instructions)
        {
            if (instruction.OpCode.Code != Code.Calli) continue;
            var sig = (MethodSig)instruction.Operand;
            if (((byte)sig.CallingConvention & 15) == 0) continue;
            try
            {
                if (context != null) sig = MetaUtil.InflateMethodSig(sig, context);
                var bridge = Parse(sig); bridges.TryAdd(bridge.Key, bridge);
                sites.Add(new { method = method.FullName, offset = instruction.Offset, signature = bridge.Key, status = "generated" });
            }
            catch (NotSupportedException e) { sites.Add(new { method = method.FullName, offset = instruction.Offset, signature = sig.ToString(), status = e.Message }); }
        }
    }
    public void Generate(string directory, Manifest manifest)
    {
        var inputs = manifest.AssemblyPlans.Values.Select(p => p.Module).ToHashSet();
        var planned = manifest.GenericMethodPlans.Select(p => p.Method.ResolveMethodDef()).ToHashSet();
        // Determine reachability on method definitions before expanding type arguments.
        // Unrelated expanding recursion (for example Grow<T> -> Grow<T[]>) must
        // not consume the native calli census budget.
        bool HasNativeCalli(MethodDef method) => method?.Body?.Instructions.Any(i =>
            i.OpCode.Code == Code.Calli && ((byte)((MethodSig)i.Operand).CallingConvention & 15) != 0) == true;
        bool Eligible(MethodDef method) => method?.Body != null &&
            (inputs.Contains(method.Module) || planned.Contains(method) || HasNativeCalli(method));
        var definitions = new HashSet<MethodDef>();
        var pendingDefinitions = new Queue<MethodDef>();
        var callers = new Dictionary<MethodDef, HashSet<MethodDef>>();
        var nativeReachable = new HashSet<MethodDef>();
        void AddDefinition(MethodDef method)
        {
            if (Eligible(method) && definitions.Add(method)) pendingDefinitions.Enqueue(method);
        }
        foreach (var module in inputs)
            foreach (var type in module.GetTypes()) foreach (var method in type.Methods) AddDefinition(method);
        foreach (var method in planned) AddDefinition(method);
        while (pendingDefinitions.Count > 0)
        {
            var method = pendingDefinitions.Dequeue();
            if (HasNativeCalli(method)) nativeReachable.Add(method);
            void AddDependency(MethodDef dependency, MethodDef caller = null)
            {
                if (!Eligible(dependency)) return;
                AddDefinition(dependency);
                if (!callers.TryGetValue(dependency, out var parents)) callers.Add(dependency, parents = new());
                parents.Add(caller ?? method);
            }
            foreach (var instruction in method.Body.Instructions)
            {
                if (instruction.Operand is IMethod target && target.IsMethod)
                {
                    var targetDefinition = target.ResolveMethodDef();
                    AddDependency(targetDefinition);
                    if (targetDefinition != null && inputs.Contains(targetDefinition.Module) && targetDefinition.DeclaringType.HasGenericParameters)
                        foreach (var member in targetDefinition.DeclaringType.Methods.Where(m => !m.HasGenericParameters)) AddDependency(member, targetDefinition);
                }
                else if (instruction.Operand is IField field && field.IsField)
                {
                    var cctor = field.DeclaringType.ResolveTypeDef()?.FindStaticConstructor();
                    AddDependency(cctor);
                    if (cctor != null && inputs.Contains(cctor.Module) && cctor.DeclaringType.HasGenericParameters)
                        foreach (var member in cctor.DeclaringType.Methods.Where(m => !m.HasGenericParameters)) AddDependency(member, cctor);
                }
            }
        }
        var pendingReachable = new Queue<MethodDef>(nativeReachable);
        while (pendingReachable.Count > 0)
            if (callers.TryGetValue(pendingReachable.Dequeue(), out var parents))
                foreach (var parent in parents) if (nativeReachable.Add(parent)) pendingReachable.Enqueue(parent);
        var closedClasses = new HashSet<string>(StringComparer.Ordinal);
        var queue = new Queue<(IMethod Method, int Depth)>();
        var seen = new HashSet<string>(StringComparer.Ordinal);
        void Consider(IMethod candidate, int depth)
        {
            if (candidate == null || !candidate.IsMethod) return;
            var def = candidate.ResolveMethodDef();
            if (def?.Body == null) return;
            if (!nativeReachable.Contains(def)) return;
            var context = MetaUtil.GetMethodGenericArgumentContext(candidate);
            if ((context?.typeArgsStack?.Count ?? 0) == 0 && (context?.methodArgsStack?.Count ?? 0) == 0) return;
            if ((context.typeArgsStack ?? new List<TypeSig>()).Concat(context.methodArgsStack ?? new List<TypeSig>()).Any(t => t.ContainsGenericParameter)) return;
            if (!seen.Add(Manifest.GenericInstanceKey(candidate))) return;
            if (depth > 32 || seen.Count > 2048) throw new InvalidOperationException("Native calli signature census exceeded its closed-generic budget; supply a bounded input closure.");
            queue.Enqueue((candidate, depth));
            if (inputs.Contains(def.Module) && context.typeArgsStack?.Count > 0 && closedClasses.Add(Manifest.TypeIdentityKey(candidate.DeclaringType.ToTypeSig())))
                foreach (var member in def.DeclaringType.Methods.Where(m => !m.HasGenericParameters)) Consider(MetaUtil.BindMethod(member, new GenericArgumentContext(context.typeArgsStack, null)), depth + 1);
        }
        void Follow(MethodDef method, GenericArgumentContext context, int depth)
        {
            if (!method.HasBody) return;
            foreach (var instruction in method.Body.Instructions)
            {
                if (instruction.Operand is IMethod target && target.IsMethod) Consider(MetaUtil.BindMethod(target, context), depth + 1);
                else if (instruction.Operand is IField field && field.IsField)
                {
                    var bound = MetaUtil.InflateField(field, context);
                    if (bound.DeclaringType.ToTypeSig() is GenericInstSig type && !type.ContainsGenericParameter)
                    {
                        var cctor = type.GenericType.TypeDefOrRef.ResolveTypeDef()?.FindStaticConstructor();
                        if (cctor != null) Consider(MetaUtil.BindMethod(cctor, new GenericArgumentContext(type.GenericArguments, null)), depth + 1);
                    }
                }
            }
        }
        foreach (var assembly in manifest.AssemblyPlans.Values)
            foreach (var type in assembly.Module.GetTypes()) foreach (var method in type.Methods) { Scan(method); Follow(method, null, 0); }
        foreach (var plan in manifest.GenericMethodPlans) Consider(plan.Method, 0);
        while (queue.Count > 0)
        {
            var item = queue.Dequeue(); var method = item.Method.ResolveMethodDef(); var context = MetaUtil.GetMethodGenericArgumentContext(item.Method);
            Scan(method, context); Follow(method, context, item.Depth);
        }
        var writer = new CodeWriter(Path.Combine(directory, "calli_bridge_types.h"));
        var declarations = new ForwardDeclaration(writer.CreateThunkCollection("types"));
        declarations.AddInclude(ConstStrings.CodegenCommonHeaderFileName);
        var text = new StringBuilder("#include \"calli_bridge_types.h\"\nusing namespace leanclr;\n");
        var emitted = new HashSet<string>();
        void EmitType(Abi t)
        {
            if (!emitted.Add(t.Key)) return;
            if (t.Fields != null) foreach (var f in t.Fields) EmitType(f.Type);
            if (t.Detail != null)
            {
                declarations.AddTypeForwardDefine(t.Detail.TypeSig);
                text.AppendLine($"static const metadata::RtCalliAbiField fields_{t.Key}[] = {{ {string.Join(", ", t.Fields.Select(f => $"{{offsetof({t.Cpp}, {f.Field.Name}), &type_{f.Type.Key}}}"))} }};");
            }
            text.AppendLine($"static const metadata::RtCalliAbiType type_{t.Key} = {{ {t.Kind}, {(t.Kind == 1 ? "0" : $"sizeof({t.Cpp})")}, {(t.Kind == 1 ? "1" : $"alignof({t.Cpp})")}, {(t.Detail == null ? "nullptr" : "fields_" + t.Key)}, {t.Fields?.Count ?? 0} }};");
        }
        foreach (var bridge in bridges.Values) foreach (var type in bridge.Args.Prepend(bridge.Ret)) EmitType(type);
        writer.Save();
        int n = 0;
        foreach (var b in bridges.Values)
        {
            if (b.Ret.Kind == 3 || b.Args.Any(a => a.Kind == 3)) text.AppendLine("#if LEANCLR_PLATFORM_WIN");
            text.AppendLine($"static void bridge_{n}(void* target, interp::RtStackObject* args, interp::RtStackObject* ret) {{");
            text.AppendLine($"using Fn = {b.Ret.Cpp} (LEANCLR_PINVOKE_CALL_CDECL *)({string.Join(", ", b.Args.Select(a => a.Cpp))});");
            string offset = "0";
            for (int i = 0; i < b.Args.Count; ++i)
            {
                string read = $"codegen::get_eval_stack_value_as_type<{(b.Args[i].Kind == 2 ? "int32_t" : b.Args[i].Kind == 3 ? "uint16_t" : b.Args[i].Cpp)}>(args + {offset})";
                if (b.Args[i].Kind == 2) read = $"({read} != 0)";
                if (b.Args[i].Kind == 3) read = $"vm::Calli::char_to_native({read})";
                if (b.Args[i].Kind == 17) text.AppendLine($"{b.Args[i].Cpp} a{i}{{}}; std::memcpy(&a{i}, args + {offset}, sizeof(a{i}));");
                else text.AppendLine($"{b.Args[i].Cpp} a{i} = {read};");
                offset += $" + codegen::get_stack_object_size_for_type<{b.Args[i].Cpp}>()";
            }
            string call = $"reinterpret_cast<Fn>(target)({string.Join(", ", Enumerable.Range(0, b.Args.Count).Select(i => $"a{i}"))})";
            string result = b.Ret.Kind == 2 ? $"bool value = {call} != 0" : b.Ret.Kind == 3 ? $"uint16_t value = vm::Calli::char_from_native({call})" : $"auto value = {call}";
            text.AppendLine(b.Ret.Kind == 1 ? $"{call};" : $"{result}; codegen::expand_argument_to_eval_stack(value, ret);");
            text.AppendLine("}");
            if (b.Args.Count > 0) text.AppendLine($"static const metadata::RtCalliAbiType* args_{n}[] = {{ {string.Join(", ", b.Args.Select(a => "&type_" + a.Key))} }};");
            if (b.Ret.Kind == 3 || b.Args.Any(a => a.Kind == 3)) text.AppendLine("#endif");
            ++n;
        }
        text.AppendLine("extern const metadata::RtCalliBridge g_calli_bridges[] = {"); n = 0;
        foreach (var b in bridges.Values)
        {
            bool ansi = b.Ret.Kind == 3 || b.Args.Any(a => a.Kind == 3);
            if (ansi) text.AppendLine("#if LEANCLR_PLATFORM_WIN");
            text.AppendLine($"{{ 1, &type_{b.Ret.Key}, {(b.Args.Count > 0 ? $"args_{n}" : "nullptr")}, {b.Args.Count}, bridge_{n++}, \"{b.Key}\" }},");
            if (ansi) text.AppendLine("#endif");
        }
        text.AppendLine("{} };\nextern const uint32_t g_calli_bridge_count = sizeof(g_calli_bridges) / sizeof(g_calli_bridges[0]) - 1;");
        File.WriteAllText(Path.Combine(directory, "calli_bridges.cpp"), text.ToString());
        File.WriteAllText(Path.Combine(directory, "calli_bridges.json"), JsonSerializer.Serialize(new { bridge_count = bridges.Count, sites }, new JsonSerializerOptions { WriteIndented = true }));
    }
}