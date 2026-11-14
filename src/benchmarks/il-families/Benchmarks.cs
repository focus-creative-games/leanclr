using System;
using System.Diagnostics;
using System.Globalization;
using System.Runtime.CompilerServices;

// Deliberately compatible with Framework CSC: the identical IL runs on all four engines.
public static class Benchmarks
{
    public struct Quad { public uint A, B, C, D; }
    public class Box { public uint Value; public virtual uint Apply(uint x) { return x * 1664525u + 1013904223u; } }
    public sealed class OtherBox : Box { public override uint Apply(uint x) { return x * 22695477u + 1; } }
    public interface IOp { uint Apply(uint x); }
    public sealed class Op : IOp { public uint Apply(uint x) { return x * 1664525u + 1013904223u; } }
    public sealed class OtherOp : IOp { public uint Apply(uint x) { return x * 22695477u + 1; } }
    static readonly uint[] Data = MakeData();
    static readonly Box[] Boxes = { new Box(), new OtherBox() };
    static readonly IOp[] Ops = { new Op(), new OtherOp() };
    static readonly Func<uint, uint> Callback = Step;
    static uint StaticValue;
    static object Escape;
    static uint[] MakeData() { uint[] a = new uint[1024]; for (uint i = 0; i < a.Length; ++i) a[i] = i * 17 + 3; return a; }
    [MethodImpl(MethodImplOptions.NoInlining)] public static uint Step(uint x) { return x * 1664525u + 1013904223u; }
    [MethodImpl(MethodImplOptions.NoInlining)] public static T Identity<T>(T x) { return x; }
    [MethodImpl(MethodImplOptions.NoInlining)] public static void RefStep(ref uint x) { x = x * 1664525u + 1013904223u; }
    [MethodImpl(MethodImplOptions.NoInlining)] public static Quad QuadStep(Quad x) { x.A = x.A * 1664525u + x.B; x.B ^= x.A; x.C += x.D; x.D ^= x.C; return x; }
    [MethodImpl(MethodImplOptions.NoInlining)] public static object BoxValue(uint x) { return x; }
    [MethodImpl(MethodImplOptions.NoInlining)] public static Box NewBox(uint x) { return new Box { Value = x }; }
    [MethodImpl(MethodImplOptions.NoInlining)] public static void Keep(object x) { Escape = x; }
    public static uint Int32(int n, uint x) { for (int i = 0; i < n; ++i) x = (x * 1664525u + 1013904223u) ^ (x >> 13); return x; }
    public static uint Int64(int n, uint x) { ulong a = x; for (int i = 0; i < n; ++i) a = (a * 6364136223846793005UL + 1442695040888963407UL) ^ (a >> 29); return (uint)(a ^ (a >> 32)); }
    public static uint DivRem(int n, uint x) { for (int i = 0; i < n; ++i) { uint d = ((uint)i & 31) + 3; x = (x / d) + (x % d) + 0xf1234567u; } return x; }
    public static uint Float32(int n, uint x) { float a = (x & 255) + 1; for (int i = 0; i < n; ++i) a = a * 0.5f + (i & 127); return (uint)(a * 1024); }
    public static uint Float64(int n, uint x) { double a = (x & 255) + 1; for (int i = 0; i < n; ++i) a = a * 0.5 + (i & 127); return (uint)(a * 1024); }
    public static uint Convert(int n, uint x) { for (int i = 0; i < n; ++i) { double a = (int)(x & 0xffff); x = (uint)(a * 1.25) + (uint)(ushort)(x >> 8) + (uint)i; } return x; }
    public static uint Checked(int n, uint x) { for (int i = 0; i < n; ++i) x = checked((x & 0xffff) * 17u + (uint)(i & 255)); return x; }
    public static uint Branch(int n, uint x) { for (int i = 0; i < n; ++i) { if ((x & 0x10000) == 0) x = x * 1664525u + 1013904223u; else x = (x >> 3) ^ 0xfedcba98u; } return x; }
    public static uint Switch(int n, uint x) { for (int i = 0; i < n; ++i) { switch (x & 7) { case 0: x += 17; break; case 1: x ^= 0x12345678; break; case 2: x *= 33; break; case 3: x >>= 1; break; case 4: x += 71; break; case 5: x ^= 0x87654321; break; case 6: x *= 17; break; default: x += 101; break; } x = x * 1664525u + 1013904223u; } return x; }
    public static uint ArrayLoad(int n, uint x) { uint[] a = Data; for (int i = 0; i < n; ++i) x = x * 33 + a[(x >> 8) & 1023]; return x; }
    public static uint ArrayStore(int n, uint x) { uint[] a = MakeData(); for (int i = 0; i < n; ++i) { int j = i & 1023; a[j] = x = x * 33 + a[j]; } return x ^ a[0]; }
    public static uint Field(int n, uint x) { Box a = new Box { Value = x }; for (int i = 0; i < n; ++i) a.Value = a.Value * 1664525u + (uint)i; return a.Value; }
    public static uint StaticField(int n, uint x) { StaticValue = x; for (int i = 0; i < n; ++i) StaticValue = StaticValue * 1664525u + (uint)i; return StaticValue; }
    public static uint ByRef(int n, uint x) { for (int i = 0; i < n; ++i) RefStep(ref x); return x; }
    public static uint Calls(int n, uint x) { for (int i = 0; i < n; ++i) x = Step(x); return x; }
    public static uint Virtual(int n, uint x) { Box[] a = Boxes; for (int i = 0; i < n; ++i) x = a[i & 1].Apply(x); return x; }
    public static uint Interface(int n, uint x) { IOp[] a = Ops; for (int i = 0; i < n; ++i) x = a[i & 1].Apply(x); return x; }
    public static uint Delegate(int n, uint x) { Func<uint, uint> f = Callback; for (int i = 0; i < n; ++i) x = f(x); return x; }
    public static uint Struct(int n, uint x) { Quad a = new Quad { A = x, B = 17, C = 31, D = 7 }; for (int i = 0; i < n; ++i) a = QuadStep(a); return a.A ^ a.B ^ a.C ^ a.D; }
    public static uint Generic(int n, uint x) { for (int i = 0; i < n; ++i) x = Identity<uint>(x * 1664525u + 1013904223u); return x; }
    public static uint Cast(int n, uint x) { object[] a = Boxes; for (int i = 0; i < n; ++i) { object v = a[i & 1]; Box b = (Box)v; x = x * 33 + (v is OtherBox ? 7u : 3u) + b.Value; } return x; }
    public static uint Boxing(int n, uint x) { for (int i = 0; i < n; ++i) { object b = BoxValue(x); Keep(b); x = (uint)b * 1664525u + 1013904223u; } return x; }
    public static uint Allocation(int n, uint x) { for (int i = 0; i < n; ++i) { Box b = NewBox(x); Keep(b); x = b.Value * 1664525u + 1013904223u; } return x; }
    public static uint NewArray(int n, uint x) { for (int i = 0; i < n; ++i) { uint[] a = new uint[16]; Keep(a); a[i & 15] = x; x = a[i & 15] * 1664525u + (uint)a.Length; } return x; }
    public static uint EhNormal(int n, uint x) { for (int i = 0; i < n; ++i) { try { x = x * 1664525u + 1013904223u; } finally { x ^= 17; } } return x; }
    public static uint EhThrow(int n, uint x) { for (int i = 0; i < n; ++i) { try { if ((i & 1023) == 0) throw new ArgumentException(); x = x * 1664525u + 1013904223u; } catch (ArgumentException) { x ^= 13; } finally { x += 7; } } return x; }
    public static readonly string[] Names = { "Int32", "Int64", "DivRem", "Float32", "Float64", "Convert", "Checked", "Branch", "Switch", "ArrayLoad", "ArrayStore", "Field", "StaticField", "ByRef", "Calls", "Virtual", "Interface", "Delegate", "Struct", "Generic", "Cast", "Boxing", "Allocation", "NewArray", "EhNormal", "EhThrow" };
    public static uint Dispatch(int k, int n, uint x)
    {
        switch (k) {
            case 0: return Int32(n,x); case 1: return Int64(n,x); case 2: return DivRem(n,x); case 3: return Float32(n,x); case 4: return Float64(n,x); case 5: return Convert(n,x); case 6: return Checked(n,x); case 7: return Branch(n,x); case 8: return Switch(n,x); case 9: return ArrayLoad(n,x); case 10: return ArrayStore(n,x); case 11: return Field(n,x); case 12: return StaticField(n,x); case 13: return ByRef(n,x); case 14: return Calls(n,x); case 15: return Virtual(n,x); case 16: return Interface(n,x); case 17: return Delegate(n,x); case 18: return Struct(n,x); case 19: return Generic(n,x); case 20: return Cast(n,x); case 21: return Boxing(n,x); case 22: return Allocation(n,x); case 23: return NewArray(n,x); case 24: return EhNormal(n,x); default: return EhThrow(n,x);
        }
    }
    public static int Main(string[] args)
    {
        int k = int.Parse(args[0]), n = int.Parse(args[1]);
        for (int w = 0; w < 3; ++w) Dispatch(k, Math.Min(n, 16384), 19);
        long start = Stopwatch.GetTimestamp();
        uint checksum = Dispatch(k, n, 19);
        long end = Stopwatch.GetTimestamp();
        Console.WriteLine("{\"workload\":\"" + Names[k] + "\",\"iterations\":" + n + ",\"checksum\":\"" + checksum + "\",\"ns_per_op\":" + ((end-start) * (1e9 / Stopwatch.Frequency) / n).ToString("F6", CultureInfo.InvariantCulture) + "}");
        return 0;
    }
}
