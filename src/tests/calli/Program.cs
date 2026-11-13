using System;
using System.Runtime.CompilerServices;
namespace CalliTests {
public unsafe class NativeSignatures {
    public delegate* unmanaged[Cdecl]<int,int> Cdecl;
    public delegate* unmanaged[Stdcall]<double,double> Stdcall;
#if NET10_0_OR_GREATER
    public delegate* unmanaged[Cdecl,SuppressGCTransition]<int,int> Suppressed;
#endif
}
public unsafe class Program {
static void Check(bool value,string message){if(!value)throw new Exception(message);}
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
public static void Main(string[]args){typeof(Program).GetMethod(args[0]).Invoke(null,null);}
}}
