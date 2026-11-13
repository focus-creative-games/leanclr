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
    record Abi(int Kind, string Cpp, TypeDetail Detail=null, List<(FieldDetail Field,Abi Type)> Fields=null) {
        public string Key => Kind==17?"s"+Cpp:Kind.ToString();
    }
    record Bridge(Abi Ret, List<Abi> Args) { public string Key => "1:"+Ret.Key+"("+string.Join(",",Args.Select(a=>a.Key))+")"; }
    readonly SortedDictionary<string,Bridge> bridges = new(StringComparer.Ordinal);
    readonly List<object> sites = new();
    static Abi Type(TypeSig type, bool ret=false)
    {
        if (type is ModifierSig) {
            var raw=type;
            while(raw is CModReqdSig m && m.Modifier.FullName is ("System.Runtime.InteropServices.InAttribute" or "System.Runtime.InteropServices.OutAttribute")) raw=raw.Next;
            if(raw.ElementType==ElementType.ByRef)type=raw;
        }
        if(type == null || type is ModifierSig || type is PinnedSig)throw new NotSupportedException("type-modifier");
        if(type.ElementType==ElementType.ByRef) {
            var payload=Type(type.Next); if(payload.Kind is 2 or 3)throw new NotSupportedException("byref-bool-char-marshalling"); return new Abi(15,"void*");
        }
        if(type.ElementType==ElementType.Ptr || type.ElementType==ElementType.I || type.ElementType==ElementType.U)return new Abi(15,"void*");
        if(type.ElementType==ElementType.ValueType && type.ToTypeDefOrRef().ResolveTypeDef() is { IsEnum:true } e)
            return Type(e.Fields.First(f=>!f.IsStatic).FieldType);
        if(type.ElementType is ElementType.ValueType or ElementType.GenericInst) {
            if(!CalliPolicy.Type(type))throw new NotSupportedException("non-blittable-struct:"+type.FullName);
            var detail=GlobalServices.Inst.MetadataService.GetTypeDetail(type.ToTypeDefOrRef());
            return new Abi(17,detail.InstanceTypeName,detail,detail.InstanceFieldsExcludeParent.Select(f=>(f,Type(f.Type))).ToList());
        }
        return type.ElementType switch {
            ElementType.Void when ret => new Abi(1,"void"),
            ElementType.Boolean => new Abi(2,"int32_t"), ElementType.Char=>new Abi(3,"uint8_t"),
            ElementType.I1=>new Abi(4,"int8_t"), ElementType.U1=>new Abi(5,"uint8_t"),
            ElementType.I2=>new Abi(6,"int16_t"), ElementType.U2=>new Abi(7,"uint16_t"),
            ElementType.I4=>new Abi(8,"int32_t"), ElementType.U4=>new Abi(9,"uint32_t"),
            ElementType.I8=>new Abi(10,"int64_t"), ElementType.U8=>new Abi(11,"uint64_t"),
            ElementType.R4=>new Abi(12,"float"), ElementType.R8=>new Abi(13,"double"),
            _=>throw new NotSupportedException("unsupported-ABI-type:"+type.FullName)
        };
    }
    static Bridge Parse(MethodSig sig)
    {
        int conv=(byte)sig.CallingConvention&15;
        if(((byte)sig.CallingConvention&0xf0)!=0 || sig.GenParamCount!=0 || sig.Params.Count>256 || sig.ParamsAfterSentinel?.Count>0)
            throw new NotSupportedException("unmanaged-signature-flags");
        var ret=sig.RetType;
        if(conv==9) {
            int specified=0;
            while(ret is ModifierSig modifier) {
                string name=modifier.Modifier.FullName;
                int c=name=="System.Runtime.CompilerServices.CallConvCdecl"?1:name=="System.Runtime.CompilerServices.CallConvStdcall"?2:0;
                if(modifier is CModReqdSig || c==0 || specified!=0)throw new NotSupportedException("unsupported-callconv-modifier:"+name);
                specified=c;ret=ret.Next;
            }
        } else if(conv!=1 && conv!=2)throw new NotSupportedException("unsupported-native-convention");
        return new Bridge(Type(ret,true),sig.Params.Select(p=>Type(p)).ToList());
    }
    void Scan(MethodDef method,GenericArgumentContext context=null)
    {
        if(!method.HasBody)return;
        foreach(var instruction in method.Body.Instructions) {
            if(instruction.OpCode.Code!=Code.Calli)continue;
            var sig=(MethodSig)instruction.Operand;
            if(((byte)sig.CallingConvention&15)==0)continue;
            try {
                if(context!=null)sig=MetaUtil.InflateMethodSig(sig,context);
                var bridge=Parse(sig);bridges.TryAdd(bridge.Key,bridge);
                sites.Add(new { method=method.FullName, offset=instruction.Offset, signature=bridge.Key, status="generated" });
            } catch(NotSupportedException e) { sites.Add(new {method=method.FullName,offset=instruction.Offset,signature=sig.ToString(),status=e.Message}); }
        }
    }
    public void Generate(string directory, Manifest manifest)
    {
        foreach(var assembly in manifest.AssemblyPlans.Values)
            foreach(var type in assembly.Module.GetTypes())foreach(var method in type.Methods)Scan(method);
        foreach(var plan in manifest.GenericMethodPlans)Scan(plan.Method.ResolveMethodDef(),MetaUtil.GetMethodGenericArgumentContext(plan.Method));
        var writer=new CodeWriter(Path.Combine(directory,"calli_bridge_types.h"));
        var declarations=new ForwardDeclaration(writer.CreateThunkCollection("types"));
        declarations.AddInclude(ConstStrings.CodegenCommonHeaderFileName);
        var text=new StringBuilder("#include \"calli_bridge_types.h\"\nusing namespace leanclr;\n");
        var emitted=new HashSet<string>();
        void EmitType(Abi t) {
            if(!emitted.Add(t.Key))return;
            if(t.Fields!=null)foreach(var f in t.Fields)EmitType(f.Type);
            if(t.Detail!=null) {
                declarations.AddTypeForwardDefine(t.Detail.TypeSig);
                text.AppendLine($"static const metadata::RtCalliAbiField fields_{t.Key}[] = {{ {string.Join(", ",t.Fields.Select(f=>$"{{offsetof({t.Cpp}, {f.Field.Name}), &type_{f.Type.Key}}}"))} }};");
            }
            text.AppendLine($"static const metadata::RtCalliAbiType type_{t.Key} = {{ {t.Kind}, {(t.Kind==1?"0":$"sizeof({t.Cpp})")}, {(t.Kind==1?"1":$"alignof({t.Cpp})")}, {(t.Detail==null?"nullptr":"fields_"+t.Key)}, {t.Fields?.Count??0} }};");
        }
        foreach(var bridge in bridges.Values)foreach(var type in bridge.Args.Prepend(bridge.Ret))EmitType(type);
        writer.Save();
        int n=0;
        foreach(var b in bridges.Values) {
            text.AppendLine($"static void bridge_{n}(void* target, interp::RtStackObject* args, interp::RtStackObject* ret) {{");
            text.AppendLine($"using Fn = {b.Ret.Cpp} (LEANCLR_PINVOKE_CALL_CDECL *)({string.Join(", ",b.Args.Select(a=>a.Cpp))});");
            string offset="0";
            for(int i=0;i<b.Args.Count;++i) {
                string read=$"codegen::get_eval_stack_value_as_type<{(b.Args[i].Kind==2?"bool":b.Args[i].Kind==3?"uint16_t":b.Args[i].Cpp)}>(args + {offset})";
                if(b.Args[i].Kind==3)read=$"vm::Calli::char_to_native({read})";
                if(b.Args[i].Kind==17)text.AppendLine($"{b.Args[i].Cpp} a{i}{{}}; std::memcpy(&a{i}, args + {offset}, sizeof(a{i}));");
                else text.AppendLine($"{b.Args[i].Cpp} a{i} = {read};");
                offset+=$" + codegen::get_stack_object_size_for_type<{b.Args[i].Cpp}>()";
            }
            string call=$"reinterpret_cast<Fn>(target)({string.Join(", ",Enumerable.Range(0,b.Args.Count).Select(i=>$"a{i}"))})";
            string result=b.Ret.Kind==2?$"bool value = {call} != 0":b.Ret.Kind==3?$"uint16_t value = vm::Calli::char_from_native({call})":$"auto value = {call}";
            text.AppendLine(b.Ret.Kind==1?$"{call};":$"{result}; codegen::expand_argument_to_eval_stack(value, ret);");
            text.AppendLine("}");
            if(b.Args.Count>0)text.AppendLine($"static const metadata::RtCalliAbiType* args_{n}[] = {{ {string.Join(", ",b.Args.Select(a=>"&type_"+a.Key))} }};");
            ++n;
        }
        text.AppendLine("extern const metadata::RtCalliBridge g_calli_bridges[] = {");n=0;
        foreach(var b in bridges.Values)text.AppendLine($"{{ 1, &type_{b.Ret.Key}, {(b.Args.Count>0?$"args_{n}":"nullptr")}, {b.Args.Count}, bridge_{n++}, \"{b.Key}\" }},");
        if(bridges.Count==0)text.AppendLine("{},");
        text.AppendLine($"}};\nextern const uint32_t g_calli_bridge_count = {bridges.Count};");
        File.WriteAllText(Path.Combine(directory,"calli_bridges.cpp"),text.ToString());
        File.WriteAllText(Path.Combine(directory,"calli_bridges.json"),JsonSerializer.Serialize(new {bridge_count=bridges.Count,sites},new JsonSerializerOptions{WriteIndented=true}));
    }
}
