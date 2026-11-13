using dnlib.DotNet;
using dnlib.DotNet.Emit;
using LeanAOT.Core;
namespace LeanAOT.GenerationPlan;
public static class CalliPolicy
{
    static bool Type(TypeSig t, bool ret=false)
    {
        if (t is ModifierSig) {
            var raw=t;
            while(raw is CModReqdSig m && m.Modifier.FullName is ("System.Runtime.InteropServices.InAttribute" or "System.Runtime.InteropServices.OutAttribute")) raw=raw.Next;
            if(raw.ElementType==ElementType.ByRef)t=raw;
        }
        if(t==null || t is ModifierSig || t is PinnedSig)return false;
        if(t.ElementType==ElementType.ByRef)return t.Next.ElementType is not (ElementType.Boolean or ElementType.Char) && Type(t.Next);
        if(t.ElementType==ElementType.ValueType && t.ToTypeDefOrRef().ResolveTypeDef() is {IsEnum:true} e)
            return Type(e.Fields.First(f=>!f.IsStatic).FieldType);
        return t.ElementType==ElementType.Void && ret || (int)t.ElementType>=2 && (int)t.ElementType<=13 ||
            t.ElementType==ElementType.Ptr || t.ElementType==ElementType.I || t.ElementType==ElementType.U;
    }
    public static string Fallback(MethodDef method, GenericArgumentContext context=null)
    {
        if(!method.HasBody)return null;
        foreach(var i in method.Body.Instructions) {
            if(i.OpCode.Code!=Code.Calli)continue;
            var sig=(MethodSig)i.Operand;
            int c=(byte)sig.CallingConvention&15;
            if(c==0)continue;
            if(context!=null)sig=MetaUtil.InflateMethodSig(sig,context);
            if(((byte)sig.CallingConvention&0xf0)!=0 || sig.GenParamCount!=0 || sig.Params.Count>256 || sig.ParamsAfterSentinel?.Count>0)return "unmanaged-calli-signature";
            var ret=sig.RetType;
            if(c==9) {
                bool specified=false;
                while(ret is ModifierSig m) {
                    if(specified || m is CModReqdSig || m.Modifier.FullName is not ("System.Runtime.CompilerServices.CallConvCdecl" or "System.Runtime.CompilerServices.CallConvStdcall"))return "unmanaged-calli-modifier";
                    specified=true;ret=ret.Next;
                }
            } else if(c!=1 && c!=2)return "unmanaged-calli-convention";
            if(!Type(ret,true) || sig.Params.Any(p=>!Type(p)))return "unmanaged-calli-type";
        }
        return null;
    }
}
