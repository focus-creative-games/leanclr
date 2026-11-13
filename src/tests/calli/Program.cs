using System;
using System.Runtime.CompilerServices;
[AttributeUsage(AttributeTargets.Method)] public class AotMethodAttribute:Attribute {public AotMethodAttribute(bool enabled){}}
namespace CalliTests {
public class Receiver { public int Bias; public Receiver(int bias){Bias=bias;} public int Add(int value){return Bias+value;} public virtual int V(int value){return Bias+value;} }
public class DerivedReceiver:Receiver {public DerivedReceiver(int value):base(value){} public override int V(int value){return Bias+value+1;} }
public struct Cell {public int Value; public int Add(int value){return Value+value;} public override int GetHashCode(){return Value;} }
public struct Triple {public long A,B,C;}
public class Payload {public int Value;}

public unsafe class NativeSignatures {
    public delegate* unmanaged[Cdecl]<int,int> Cdecl;
    public delegate* unmanaged[Stdcall]<double,double> Stdcall;
#if NET10_0_OR_GREATER
    public delegate* unmanaged[Cdecl,SuppressGCTransition]<int,int> Suppressed;
#endif
}
public unsafe class Program {
static void Check(bool value,string message){if(!value)throw new Exception(message);}
[AotMethod(false)] public static int UnmanagedInventory(IntPtr target){return ((delegate* unmanaged[Cdecl]<int,int>)(void*)target)(41);}
public static int Add(int value){return value+1;}
public static float Float(float value){return value+2;}
public static void Change(ref int value){value=42;}
public static T Identity<T>(T value){return value;}
public static void Managed(){delegate* managed<int,int> p=&Add;Check(p(41)==42,"managed int");delegate* managed<float,float> f=&Float;Check(f(1.25f)==3.25f,"managed float");}
public static void ManagedRef(){delegate* managed<ref int,void> p=&Change;int value=1;p(ref value);Check(value==42,"managed byref");}
public static void ManagedGeneric(){delegate* managed<long,long> p=&Identity<long>;Check(p(long.MinValue)==long.MinValue,"closed generic");}
public static void BadTarget(){bool caught=false;delegate* managed<int,int> p=(delegate* managed<int,int>)(nint)1;try{p(1);}catch(ArgumentException){caught=true;}Check(caught,"invalid descriptor");}
public static void NullTarget(){bool caught=false;delegate* managed<int,int> p=null;try{p(1);}catch(NullReferenceException){caught=true;}Check(caught,"null descriptor");}
public static void WrongSignature(){bool caught=false;delegate* managed<float,float> p=(delegate* managed<float,float>)(delegate* managed<int,int>)&Add;try{p(1);}catch(ArgumentException){caught=true;}Check(caught,"full signature validation");}
public static void FromHandle(){var method=typeof(Program).GetMethod("Add");delegate* managed<int,int> p=(delegate* managed<int,int>)(void*)method.MethodHandle.GetFunctionPointer();Check(p(41)==42,"method handle descriptor");}
public static T Apply<T>(delegate* managed<T,T> pointer,T value){return pointer(value);}
public static T InContext<T>(T value){delegate* managed<T,T> p=&Identity<T>;return p(value);}
public static void GenericCaller(){Check(InContext<string>("a")=="a" && (int)InContext<object>((object)42)==42 && InContext<string>("b")=="b","shared caller context");delegate* managed<long,long> a=&Identity<long>;delegate* managed<string,string> b=&Identity<string>;Check(Apply<long>(a,91)==91 && Apply<string>(b,"context")=="context","actual generic context");}
public static Triple Flip(Triple value){value.C++;return value;}
public static void StructReturn(){delegate* managed<Triple,Triple> pointer=&Flip;Triple value=pointer(new Triple{A=1,B=2,C=3});Check(value.A==1 && value.B==2 && value.C==4,"struct return");}
public static ref int RefReturnTarget(ref int value){return ref value;}
public static void RefReturn(){delegate* managed<ref int,ref int> pointer=&RefReturnTarget;int value=1;ref int result=ref pointer(ref value);result=42;Check(value==42,"byref return");}
[AotMethod(false)] public static int InterpretedTarget(int value){return value+3;}
public static void Mixed(){delegate* managed<int,int> pointer=&InterpretedTarget;Check(pointer(39)==42,"AOT to interpreter");}
public static object CollectTarget(object value){GC.Collect();for(int i=0;i<1024;i++){var pressure=new Payload{Value=99};GC.KeepAlive(pressure);}return value;}
public static void Collect(){object value=new Payload{Value=42};delegate* managed<object,object> pointer=&CollectTarget;object result=pointer(value);Check(object.ReferenceEquals(result,value) && ((Payload)result).Value==42,"calli GC roots");}
public static int CollectInt(){CollectTarget(null);return 1;}
public static void Consume(Payload value,int ignored){Check(value.Value==42,"evaluation stack root across calli");}
public static void StackRoot(){delegate* managed<int> pointer=&CollectInt;Consume(new Payload{Value=42},pointer());}
public static int Effects;
public static void ThrowTarget(){Effects++;throw new InvalidOperationException();}
public static void Exception(){Effects=0;delegate* managed<void> pointer=&ThrowTarget;bool caught=false;try{Effects++;pointer();}catch(InvalidOperationException){caught=true;}Check(caught && Effects==2,"no replay on handoff");}
public static void Main(string[]args){typeof(Program).GetMethod(args[0]).Invoke(null,null);}
}}
