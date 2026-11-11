using System;
using System.Diagnostics;
using System.Runtime.CompilerServices;

// The same source is used by LeanAOT and IL2CPP. No engine APIs are required.
public static class Benchmarks
{
    public struct Pair { public uint X, Y; }
    public interface ITransform { uint Apply(uint x); }
    public sealed class Transform : ITransform
    {
        public uint Apply(uint x) { return unchecked(x * 1664525u + 1013904223u); }
    }
    public sealed class OtherTransform : ITransform
    {
        public uint Apply(uint x) { return unchecked(x * 22695477u + 1u); }
    }
    public interface IGenericTransform { uint Apply<T>(uint x); }
    public sealed class GenericTransform : IGenericTransform
    {
        public uint Apply<T>(uint x) { return unchecked(x * 1664525u + 1013904223u); }
    }
    public sealed class Box { public uint Value { get; set; } }
    static readonly uint[] Data = CreateData();
    static readonly ITransform Transformer = new Transform();
    static readonly ITransform[] Transformers = { new Transform(), new OtherTransform() };
    static readonly GenericTransform GenericTransformer = new GenericTransform();
    static readonly Box PropertyBox = new Box();
    [MethodImpl(MethodImplOptions.NoInlining)]
    public static void VoidReturnLeaf(Box box, int divisor) { box.Value = unchecked(box.Value * 1664525u + (uint)(12345 / divisor)); }
    [MethodImpl(MethodImplOptions.NoInlining)]
    public static void VoidReturnMiddle(Box box, int divisor) { VoidReturnLeaf(box, divisor); box.Value ^= 19u; }
    [MethodImpl(MethodImplOptions.NoInlining)]
    public static Box ReferenceReturnLeaf(Box box, int value) { return value == 0 ? null : box; }
    [MethodImpl(MethodImplOptions.NoInlining)]
    public static Box ReferenceReturnMiddle(Box box, int value) { Box result = ReferenceReturnLeaf(box, value); if (result != null) result.Value = unchecked(result.Value * 1664525u + 1013904223u); return result; }
    public static uint VoidReturns(int n, uint seed)
    {
        Box box = new Box { Value = seed };
        for (int i = 0; i < n; ++i) VoidReturnMiddle(box, 7);
        return box.Value;
    }
    public static uint ReferenceReturns(int n, uint seed)
    {
        Box box = new Box { Value = seed };
        for (int i = 0; i < n; ++i) box = ReferenceReturnMiddle(box, 1);
        return box.Value;
    }
    static uint[] CreateData()
    {
        uint[] data = new uint[1024];
        for (uint i = 0; i < data.Length; ++i) data[i] = i * 17u;
        return data;
    }
    [MethodImpl(MethodImplOptions.NoInlining)]
    public static uint Step(uint x) { return unchecked(x * 1664525u + 1013904223u); }
    [MethodImpl(MethodImplOptions.NoInlining)]
    public static uint Chain1(uint x) { return Step(x); }
    [MethodImpl(MethodImplOptions.NoInlining)]
    public static uint Chain2(uint x) { return Chain1(x); }
    [MethodImpl(MethodImplOptions.NoInlining)]
    public static Pair PairStep(Pair p) { p.X = Step(p.X); p.Y ^= p.X; return p; }
    [MethodImpl(MethodImplOptions.NoInlining)]
    public static T Identity<T>(T x) { return x; }
    public static uint Arithmetic(int n, uint seed)
    {
        for (int i = 0; i < n; ++i) seed = unchecked(seed * 1664525u + 1013904223u);
        return seed;
    }
    public static uint Calls(int n, uint seed)
    {
        for (int i = 0; i < n; ++i) seed = Chain2(seed);
        return seed;
    }
    public static uint Properties(int n, uint seed)
    {
        Box box = PropertyBox;
        for (int i = 0; i < n; ++i) { box.Value = seed; seed = unchecked(box.Value * 1664525u + 1013904223u); }
        return seed;
    }
    public static uint ArrayLoop(int n, uint seed)
    {
        uint[] data = Data;
        for (int i = 0; i < n; ++i) seed = unchecked(seed * 1664525u + data[i & 1023]);
        return seed;
    }
    public static uint Interfaces(int n, uint seed)
    {
        ITransform t = Transformer;
        for (int i = 0; i < n; ++i) seed = t.Apply(seed);
        return seed;
    }
    public static uint InterfacesAlternating(int n, uint seed)
    {
        ITransform[] values = Transformers;
        for (int i = 0; i < n; ++i) seed = values[i & 1].Apply(seed);
        return seed;
    }
    public static uint InterfacesGeneric(int n, uint seed)
    {
        seed = GenericTransformer.Apply<int>(seed); // Concrete override anchor.
        IGenericTransform value = GenericTransformer;
        for (int i = 0; i < n; ++i) seed = value.Apply<int>(seed);
        return seed;
    }
    [MethodImpl(MethodImplOptions.NoInlining)]
    public static uint InterfaceOnce(ITransform value, uint seed) { return value.Apply(seed); }
    public static uint InterfacesStraightLine(int n, uint seed)
    {
        ITransform value = Transformer;
        for (int i = 0; i < n; ++i) seed = InterfaceOnce(value, seed);
        return seed;
    }
    public static uint Structs(int n, uint seed)
    {
        Pair p = new Pair { X = seed, Y = 17 };
        for (int i = 0; i < n; ++i) p = PairStep(p);
        return p.X ^ p.Y;
    }
    public static uint Generics(int n, uint seed)
    {
        for (int i = 0; i < n; ++i) seed = Step(Identity<uint>(seed));
        return seed;
    }
    public static uint Exceptions(int n, uint seed)
    {
        for (int i = 0; i < n; ++i)
        {
            try { if ((i & 1023) == 0) throw new ArgumentException(); seed = Step(seed); }
            catch (ArgumentException) { seed ^= 13; }
            finally { seed = unchecked(seed + 7); }
        }
        return seed;
    }
    public static uint Dispatch(int kind, int n, uint seed)
    {
        switch (kind)
        {
            case 0: return Arithmetic(n, seed);
            case 1: return Calls(n, seed);
            case 2: return Properties(n, seed);
            case 3: return ArrayLoop(n, seed);
            case 4: return Interfaces(n, seed);
            case 5: return Structs(n, seed);
            case 6: return Generics(n, seed);
            default: return Exceptions(n, seed);
        }
    }
    public static int Main(string[] args)
    {
        int n = args.Length > 0 ? int.Parse(args[0]) : 2000000;
        string[] names = { "arithmetic", "calls", "properties", "array", "interfaces", "structs", "generics", "exceptions" };
        for (int kind = 0; kind < names.Length; ++kind)
        {
            Dispatch(kind, n / 8 + 1, 19);
            long start = Stopwatch.GetTimestamp();
            uint checksum = Dispatch(kind, n, 19);
            long end = Stopwatch.GetTimestamp();
            double ns = (end - start) * (1000000000.0 / Stopwatch.Frequency) / n;
            Console.WriteLine("{\"workload\":\"" + names[kind] + "\",\"iterations\":" + n + ",\"checksum\":\"" + checksum + "\",\"ns_per_op\":" + ns.ToString("F6", System.Globalization.CultureInfo.InvariantCulture) + "}");
        }
        return 0;
    }
}
