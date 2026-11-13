using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
[AttributeUsage(AttributeTargets.Method)] public class AotMethodAttribute : Attribute { public AotMethodAttribute(bool enabled) { } }
namespace CalliTests
{
    public class Receiver { public int Bias; public Receiver(int bias) { Bias = bias; } public int Add(int value) { return Bias + value; } public int Constant() { return 42; } public virtual int V(int value) { return Bias + value; } }
    public class DerivedReceiver : Receiver { public DerivedReceiver(int value) : base(value) { } public override int V(int value) { return Bias + value + 1; } }
    public struct Cell { public int Value; public int Add(int value) { return Value + value; } public override int GetHashCode() { return Value; } }
    public struct Triple { public long A, B, C; }
    public struct Small { public int A, B; }
    public struct FloatPair { public float A, B; }
    public struct DoubleTriple { public double A, B, C; }
    [StructLayout(LayoutKind.Sequential, Pack = 1)] public struct Packed { public byte A; public int B; public short C; }
    [StructLayout(LayoutKind.Explicit, Size = 16)] public struct Explicit { [FieldOffset(0)] public long A; [FieldOffset(8)] public int B; }
    [StructLayout(LayoutKind.Explicit)] public struct Union { [FieldOffset(0)] public int I; [FieldOffset(0)] public float F; }
    [StructLayout(LayoutKind.Auto)] public struct Auto { public int Value; }
    public struct Nested { public Small S; public long L; }
    public struct References { public object Value; }
    public class Payload { public int Value; }

    public unsafe class NativeSignatures
    {
        public delegate* unmanaged[Cdecl]<int, int> Cdecl;
        public delegate* unmanaged[Stdcall]<double, double> Stdcall;
#if NET10_0_OR_GREATER
        public delegate* unmanaged[Cdecl, SuppressGCTransition]<int, int> Suppressed;
#endif
    }
    public unsafe class Program
    {
        static void Check(bool value, string message) { if (!value) throw new Exception(message); }
        [AotMethod(false)] public static int UnmanagedInventory(IntPtr target) { return ((delegate* unmanaged[Cdecl]<int, int>)(void*)target)(41); }
        public static IntPtr N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15, N16, N17, N18, N19, N20, N21, N22, N23, N24, N25, N26;
        public static void Unmanaged()
        {
            Check(((delegate* unmanaged[Cdecl]<int, int>)(void*)N0)(41) == 42, "native int");
            Check(((delegate* unmanaged[Cdecl]<int, double, float, long, double>)(void*)N1)(10, 20.5, 3, 9) == 42.5, "native mixed");
            Check(((delegate* unmanaged[Cdecl]<int, double, int, double, int, double, int, double, int, double, int, double, double>)(void*)N2)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12) == 78, "native register overflow");
            int x = 1; double y = 2; ((delegate* unmanaged[Cdecl]<ref int, out double, void>)(void*)N3)(ref x, out y); Check(x == 42 && y == 84, "native ref/out");
            Check(!((delegate* unmanaged[Cdecl]<bool, bool>)(void*)N4)(true) && ((delegate* unmanaged[Cdecl]<bool, bool>)(void*)N4)(false), "native bool width");

            Check(((delegate* unmanaged[Cdecl]<sbyte, sbyte>)(void*)N6)(-128) == -127, "native signed byte");
            Check(((delegate* unmanaged[Cdecl]<byte, byte>)(void*)N7)(254) == 255, "native byte");
            Check(((delegate* unmanaged[Cdecl]<short, short>)(void*)N8)(-32768) == -32767, "native signed short");
            Check(((delegate* unmanaged[Cdecl]<ushort, ushort>)(void*)N9)(65534) == 65535, "native ushort");
            Check(((delegate* unmanaged[Cdecl]<ulong, ulong>)(void*)N10)(ulong.MaxValue) == 0, "native unsigned long");
            int value = 123; Check(((delegate* unmanaged[Cdecl]<void*, void*>)(void*)N11)(&value) == &value, "native pointer");
            Check(((delegate* unmanaged[Stdcall]<double, double>)(void*)N12)(41) == 42, "native stdcall");
            Check(((delegate* unmanaged[Cdecl]<float, float>)(void*)N13)(1.5f) == 3, "native float");
            Check(((delegate* unmanaged[Cdecl]<nint, nint>)(void*)N14)(41) == 42, "native intptr");
        }
        public static void NativeStructs()
        {
            var small = ((delegate* unmanaged[Cdecl]<Small, Small>)(void*)N15)(new Small { A = 1, B = 2 }); Check(small.A == 2 && small.B == 4, "small aggregate");
            var large = ((delegate* unmanaged[Cdecl]<Triple, Triple>)(void*)N16)(new Triple { A = 3, B = 4, C = 5 }); Check(large.A == 4 && large.B == 6 && large.C == 8, "hidden return aggregate");
            var floats = ((delegate* unmanaged[Cdecl]<FloatPair, FloatPair>)(void*)N17)(new FloatPair { A = 1.25f, B = 2.5f }); Check(floats.A == 2.25f && floats.B == 4.5f, "float aggregate");
            var doubles = ((delegate* unmanaged[Cdecl]<DoubleTriple, DoubleTriple>)(void*)N18)(new DoubleTriple { A = 1.25, B = 2.5, C = 3.75 }); Check(doubles.A == 2.25 && doubles.B == 4.5 && doubles.C == 6.75, "double aggregate");
            var packed = ((delegate* unmanaged[Cdecl]<Packed, Packed>)(void*)N19)(new Packed { A = 4, B = 5, C = 6 }); Check(packed.A == 5 && packed.B == 7 && packed.C == 9, "packed aggregate");
            var explicitValue = ((delegate* unmanaged[Cdecl]<Explicit, Explicit>)(void*)N20)(new Explicit { A = 5, B = 6 }); Check(explicitValue.A == 6 && explicitValue.B == 8, "explicit aggregate");
            var union = ((delegate* unmanaged[Cdecl]<Union, Union>)(void*)N22)(new Union { I = 40 }); Check(union.I == 42, "overlapping explicit aggregate");
            var nested = new Nested { S = new Small { A = 1, B = 2 }, L = 3 }; ((delegate* unmanaged[Cdecl]<ref Nested, void>)(void*)N21)(ref nested); Check(nested.S.A == 2 && nested.S.B == 4 && nested.L == 6, "nested byref aggregate");
        }
        [AotMethod(false)] public static T ExcludedNativeGeneric<T>(IntPtr target, T value) where T : unmanaged { return ((delegate* unmanaged[Cdecl]<T, T>)(void*)target)(value); }
        public static void NativeGeneric() { Check(ExcludedNativeGeneric<uint>(N26, uint.MaxValue) == 0x7fffffff, "interpreter-only closed generic bridge census"); }
        [DllImport("__Internal", EntryPoint = "calli_bench_add", CallingConvention = CallingConvention.Cdecl)] public static extern int NamedAdd(int value);
        public static int BenchCalli(int count) { int sum = 0; var pointer = (delegate* unmanaged[Cdecl]<int, int>)(void*)N0; for (int i = 0; i < count; i++) sum += pointer(i & 255); return sum; }
        public static int BenchNamed(int count) { int sum = 0; for (int i = 0; i < count; i++) sum += NamedAdd(i & 255); return sum; }
        public static void NativeCollect() { var value = new Payload { Value = 42 }; Check(((delegate* unmanaged[Cdecl]<ref int, int>)(void*)N23)(ref value.Value) == 42, "native callback interior root"); Consume(new Payload { Value = 42 }, ((delegate* unmanaged[Cdecl]<int>)(void*)N24)()); }
        public static void NativeException() { Effects = 0; try { Effects++; ((delegate* unmanaged[Cdecl]<int, int>)(void*)N25)(41); throw new InvalidOperationException(); } catch (InvalidOperationException) { Check(Effects == 1, "native EH no replay"); } }
        public static void NativeChar() { Check(((delegate* unmanaged[Cdecl]<char, char>)(void*)N5)('A') == 'B', "ANSI char ASCII"); Check(((delegate* unmanaged[Cdecl]<char, char>)(void*)N5)('\ufffe') != '\uffff', "ANSI replacement char"); }
        [AotMethod(false)] public static void RejectReferences() { ((delegate* unmanaged[Cdecl]<References, void>)(void*)1)(new References()); }
        [AotMethod(false)] public static void RejectAuto() { ((delegate* unmanaged[Cdecl]<Auto, void>)(void*)1)(new Auto()); }
        [AotMethod(false)] public static void RejectBoolByref() { bool value = false; ((delegate* unmanaged[Cdecl]<ref bool, void>)(void*)1)(ref value); }
        public static void NativeRejected() { int count = 0; try { RejectReferences(); } catch (NotSupportedException) { count++; } try { RejectAuto(); } catch (NotSupportedException) { count++; } try { RejectBoolByref(); } catch (NotSupportedException) { count++; } Check(count == 3, "unsupported ABI signatures rejected before target"); }
        public static void NativeFailures() { bool caught = false; try { ((delegate* unmanaged[Cdecl]<int, int>)(void*)0)(1); } catch (NullReferenceException) { caught = true; } Check(caught, "null native target"); caught = false; try { ((delegate* unmanaged[Cdecl]<int, int>)(delegate* managed<int, int>)&Add)(1); } catch (ArgumentException) { caught = true; } Check(caught, "managed descriptor is not a native target"); }
        public static void SharedSuite() { var tests = new Tests.Instruments.Funcs.TC_calli(); tests.managed_scalar(); tests.managed_generic_and_byref(); tests.managed_exception_does_not_replay(); }
        public static int Add(int value) { return value + 1; }
        public static float Float(float value) { return value + 2; }
        public static void Change(ref int value) { value = 42; }
        public static T Identity<T>(T value) { return value; }
        public static string Text() { return "ok"; }
        public static int ReadObject(object value) { return ((string)value).Length; }
        public static void ManagedVariance() { delegate* managed<object> result = &Text; delegate* managed<string, int> parameter = &ReadObject; Check((string)result() == "ok" && parameter("abc") == 3, "reference function pointer variance"); }
        public static void Managed() { ManagedVariance(); delegate* managed<int, int> p = &Add; Check(p(41) == 42, "managed int"); delegate* managed<float, float> f = &Float; Check(f(1.25f) == 3.25f, "managed float"); }
        public static void ManagedRef() { delegate* managed<ref int, void> p = &Change; int value = 1; p(ref value); Check(value == 42, "managed byref"); }
        public static void ManagedGeneric() { delegate* managed<long, long> p = &Identity<long>; Check(p(long.MinValue) == long.MinValue, "closed generic"); }
        public static void BadTarget() { bool caught = false; delegate* managed<int, int> p = (delegate* managed<int, int>)(nint)1; try { p(1); } catch (ArgumentException) { caught = true; } Check(caught, "invalid descriptor"); }
        public static void NullTarget() { bool caught = false; delegate* managed<int, int> p = null; try { p(1); } catch (NullReferenceException) { caught = true; } Check(caught, "null descriptor"); }
        public static void WrongSignature() { bool caught = false; delegate* managed<float, float> p = (delegate* managed<float, float>)(delegate* managed<int, int>)&Add; try { p(1); } catch (ArgumentException) { caught = true; } Check(caught, "full signature validation"); }
        public static void FromHandle() { var method = typeof(Program).GetMethod("Add"); delegate* managed<int, int> p = (delegate* managed<int, int>)(void*)method.MethodHandle.GetFunctionPointer(); Check(p(41) == 42, "method handle descriptor"); }
        public static T Apply<T>(delegate* managed<T, T> pointer, T value) { return pointer(value); }
        public static T InContext<T>(T value) { delegate* managed<T, T> p = &Identity<T>; return p(value); }
        public static void GenericCaller() { Check(InContext<string>("a") == "a" && (int)InContext<object>((object)42) == 42 && InContext<string>("b") == "b", "shared caller context"); delegate* managed<long, long> a = &Identity<long>; delegate* managed<string, string> b = &Identity<string>; Check(Apply<long>(a, 91) == 91 && Apply<string>(b, "context") == "context", "actual generic context"); }
        public static Triple Flip(Triple value) { value.C++; return value; }
        public static void StructReturn() { delegate* managed<Triple, Triple> pointer = &Flip; Triple value = pointer(new Triple { A = 1, B = 2, C = 3 }); Check(value.A == 1 && value.B == 2 && value.C == 4, "struct return"); }
        public static ref int RefReturnTarget(ref int value) { return ref value; }
        public static void RefReturn() { delegate* managed<ref int, ref int> pointer = &RefReturnTarget; int value = 1; ref int result = ref pointer(ref value); result = 42; Check(value == 42, "byref return"); }
        [AotMethod(false)] public static int InterpretedTarget(int value) { return value + 3; }
        public static void Mixed() { delegate* managed<int, int> pointer = &InterpretedTarget; Check(pointer(39) == 42, "AOT to interpreter"); }
        public static object CollectTarget(object value) { GC.Collect(); for (int i = 0; i < 1024; i++) { var pressure = new Payload { Value = 99 }; GC.KeepAlive(pressure); } return value; }
        public static void Collect() { object value = new Payload { Value = 42 }; delegate* managed<object, object> pointer = &CollectTarget; object result = pointer(value); Check(object.ReferenceEquals(result, value) && ((Payload)result).Value == 42, "calli GC roots"); }
        public static int CollectInt() { CollectTarget(null); return 1; }
        public static void Consume(Payload value, int ignored) { Check(value.Value == 42, "evaluation stack root across calli"); }
        public static void StackRoot() { delegate* managed<int> pointer = &CollectInt; Consume(new Payload { Value = 42 }, pointer()); }
        public static int Effects;
        public static void ThrowTarget() { Effects++; throw new InvalidOperationException(); }
        public static void Exception() { Effects = 0; delegate* managed<void> pointer = &ThrowTarget; bool caught = false; try { Effects++; pointer(); } catch (InvalidOperationException) { caught = true; } Check(caught && Effects == 2, "no replay on handoff"); }
        public static void Main(string[] args)
        {
#if NET10_0_OR_GREATER
            if (args.Length > 1) { var library = System.Runtime.InteropServices.NativeLibrary.Load(System.IO.Path.GetFullPath(args[1])); var get = (delegate* unmanaged[Cdecl]<int, IntPtr>)(void*)System.Runtime.InteropServices.NativeLibrary.GetExport(library, "calli_test_get_function"); System.Runtime.InteropServices.NativeLibrary.SetDllImportResolver(typeof(Program).Assembly, (name, assembly, path) => library); for (int i = 0; i < 27; i++) typeof(Program).GetField("N" + i).SetValue(null, get(i)); }
#endif
            typeof(Program).GetMethod(args[0]).Invoke(null, null);
        }

    }
}
