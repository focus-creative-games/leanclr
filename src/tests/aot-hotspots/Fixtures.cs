using System;
using System.Runtime.CompilerServices;

public static class HotspotFixtures
{
    public class Base { public int Value; public virtual int Add(int x) { return Value + x; } }
    public class Middle : Base { }
    public sealed class Leaf : Middle { public override int Add(int x) { return Value + x + 1; } }
    public sealed class Unrelated { }
    public sealed class Finalized { public static int Count; ~Finalized() { ++Count; } }
    public struct Pair { public long A, B; public object Ref; }
    public struct Counter { public int Value; public int Add(int x) { return Value += x; } }
    public enum Code { High = unchecked((int)0xFEDCBA98) }
    public enum WideCode : long { High = unchecked((long)0xFEDCBA9876543210UL) }
    public interface IOut<out T> { T Read(); }
    public sealed class Reader : IOut<string> { public string Read() { return "ok"; } }
    public sealed class Holder<T> { public T Value; public T Read() { return Value; } }
    public static class Delayed
    {
        static readonly int Value;
        static Delayed() { Value = 41; Log += 100; }
        public static int Read() { return Value; }
    }
    public static class Failed
    {
        static Failed() { throw new ArgumentException(); }
        public static int Read() { return 0; }
    }
    public delegate int RefCall(ref int x, out int y);
    static int Log;
    static object Root;
    static void Check(bool ok, [CallerLineNumber] int line = 0) { if (!ok) throw new Exception("hotspot boundary assertion line " + line); }
    [MethodImpl(MethodImplOptions.NoInlining)] static Base CastBase(object x) { return (Base)x; }
    [MethodImpl(MethodImplOptions.NoInlining)] static Leaf CastLeaf(object x) { return (Leaf)x; }
    [MethodImpl(MethodImplOptions.NoInlining)] static bool IsLeaf(object x) { return x is Leaf; }
    [MethodImpl(MethodImplOptions.NoInlining)] static T CastGeneric<T>(object x) { return (T)x; }
    [MethodImpl(MethodImplOptions.NoInlining)] public static int StaticStep(int x) { return x + 7; }
    [MethodImpl(MethodImplOptions.NoInlining)] public static T Identity<T>(T x) { return x; }
    [MethodImpl(MethodImplOptions.NoInlining)] public static long Wide(long x) { return unchecked(x ^ long.MinValue); }
    [MethodImpl(MethodImplOptions.NoInlining)] public static double Double(double x) { return x + 0.25; }
    [MethodImpl(MethodImplOptions.NoInlining)] public static Pair Large(Pair x) { x.A += x.B; return x; }
    [MethodImpl(MethodImplOptions.NoInlining)] public static Code EnumReturn() { return Code.High; }
    [MethodImpl(MethodImplOptions.NoInlining)] public static int IntReturn() { return unchecked((int)0xFEDCBA98); }
    [MethodImpl(MethodImplOptions.NoInlining)] public static WideCode WideEnumReturn() { return WideCode.High; }
    [MethodImpl(MethodImplOptions.NoInlining)] public static long WideIntReturn() { return unchecked((long)0xFEDCBA9876543210UL); }
    [MethodImpl(MethodImplOptions.NoInlining)] public static string ReferenceReturn() { return "ok"; }
    [MethodImpl(MethodImplOptions.NoInlining)] public static void VoidCall() { Log += 1; }
    [MethodImpl(MethodImplOptions.NoInlining)] public static int RefStep(ref int x, out int y) { y = ++x; return y + 1; }
    [MethodImpl(MethodImplOptions.NoInlining)] public static int StaticInterpreted(int x) { return x * 3; }
    public static int Bound(object target, int x) { return x + (target == null ? 2 : ((Base)target).Value); }
    static int First(int x) { Log = Log * 10 + 1; return x + 1; }
    static int Last(int x) { Log = Log * 10 + 2; return x + 2; }
    static int Fail(int x) { throw new ArgumentException(); }
    static int Reenter(int x) { Func<int, int> f = StaticStep; return f(x) + 1; }
    public static int Throws(int x) { if (x < 0) throw new ArgumentException(); return x; }
    [MethodImpl(MethodImplOptions.NoInlining)] static T[] New<T>(int length) { return new T[length]; }

    public static int Casts()
    {
        object leaf = new Leaf { Value = 13 }, middle = new Middle();
        Check(CastBase(leaf).Value == 13 && CastLeaf(leaf).Value == 13 && IsLeaf(leaf));
        Check(CastBase(middle) != null && !IsLeaf(middle));
        Check(CastBase(null) == null && CastLeaf(null) == null && !IsLeaf(null));
        IOut<object> variant = (IOut<object>)(object)new Reader(); Check((string)variant.Read() == "ok");
        object array = new string[] { "ok" }; Check(((object[])array)[0].Equals("ok"));
        Check(CastGeneric<string>("ok") == "ok" && CastGeneric<Base>(leaf).Value == 13);
        Check(CastGeneric<int>(17) == 17 && CastGeneric<int?>(null) == null && CastGeneric<int?>(17) == 17);
        Check(CastGeneric<Holder<string>>(new Holder<string> { Value = "value" }).Value == "value");
        return 0;
    }

    public static int Delegates()
    {
        Log = 0; Func<int> delayed = Delayed.Read; Check(Log == 0); Check(delayed() == 41 && Log == 100);
        Func<int, int> f = StaticStep; Check(f(3) == 10);
        Base instance = new Leaf { Value = 13 }; f = instance.Add; Check(f(3) == 17);
        f = StaticInterpreted; Check(f(3) == 9);
        Func<int, int> generic = Identity<int>; Check(generic(int.MinValue) == int.MinValue);
        Func<string, string> refGeneric = Identity<string>; Check(refGeneric("value") == "value");
        Func<string> closedGeneric = new Holder<string> { Value = "ok" }.Read; Check(closedGeneric() == "ok");
        Func<long, long> wide = Wide; Check(wide(0x123456789ABCDEF) == unchecked(0x123456789ABCDEF ^ long.MinValue));
        Func<double, double> fp = Double; Check(fp(0.5) == 0.75);
        Func<Pair, Pair> large = Large; Pair p = large(new Pair { A = 17, B = 19, Ref = instance }); Check(p.A == 36 && object.ReferenceEquals(p.Ref, instance));
        Func<Code> code = EnumReturn; Check(code() == Code.High);
        Func<object> reference = ReferenceReturn; Check((string)reference() == "ok");
        Action action = VoidCall; Log = 0; action(); Check(Log == 1);
        RefCall byref = RefStep; int x = 7, y; Check(byref(ref x, out y) == 9 && x == 8 && y == 8);
        Counter counter = new Counter { Value = 7 }; f = counter.Add; Check(f(2) == 9 && f(2) == 11);
        Func<int, int> chain = First; chain += Last; Log = 0; Check(chain(7) == 9 && Log == 12);
        f = Reenter; Check(f(3) == 11);
        f = Throws; Check(f(17) == 17);
        var boundMethod = typeof(HotspotFixtures).GetMethod("Bound");
        f = (Func<int, int>)Delegate.CreateDelegate(typeof(Func<int, int>), instance, boundMethod); Check(f(3) == 16);
        f = (Func<int, int>)Delegate.CreateDelegate(typeof(Func<int, int>), null, boundMethod); Check(f(3) == 5);
        var openMethod = typeof(Base).GetMethod("Add");
        var open = (Func<Base, int, int>)Delegate.CreateDelegate(typeof(Func<Base, int, int>), openMethod);
        Check(open(instance, 3) == 17);
        return 0;
    }

    public static int Arrays()
    {
        foreach (int n in new int[] { 0, 1, 15, 16, 17, 1024 })
        {
            int[] a = New<int>(n); object[] refs = New<object>(n); Pair[] pairs = New<Pair>(n);
            Check(a.Length == n && refs.Length == n && pairs.Length == n);
            for (int i = 0; i < n; ++i) Check(a[i] == 0 && refs[i] == null && pairs[i].A == 0 && pairs[i].B == 0 && pairs[i].Ref == null);
            if (n > 0) { refs[n - 1] = Root = new Base { Value = n }; pairs[n - 1].Ref = refs[n - 1]; Check(object.ReferenceEquals(pairs[n - 1].Ref, Root)); }
        }
        Check(!object.ReferenceEquals(New<int>(0), New<int>(0)));
        Check(New<int>(3).Length == 3);
        // Distinct closed instantiations must never reuse another T's layout.
        string[] s = New<string>(7); object[] o = New<object>(11); long[] l = New<long>(13); byte[] b = New<byte>(19);
        Check(s.Length == 7 && o.Length == 11 && l.Length == 13 && b.Length == 19);
        Check(s.GetType() == typeof(string[]) && o.GetType() == typeof(object[]) && l.GetType() == typeof(long[]) && b.GetType() == typeof(byte[]));
        s[6] = "last"; l[12] = long.MinValue; b[18] = 255; Check(s[6] == "last" && l[12] == long.MinValue && b[18] == 255);
        return 0;
    }

    public static Func<Code> GetEnumDelegate() { return EnumReturn; }
    public static Func<int> GetIntDelegate() { return IntReturn; }
    public static int CallEnum(Func<Code> f) { return unchecked((int)f()); }
    public static int CallInt(Func<int> f) { return f(); }
    public static Func<WideCode> GetWideEnumDelegate() { return WideEnumReturn; }
    public static Func<long> GetWideIntDelegate() { return WideIntReturn; }
    public static long CallWideEnum(Func<WideCode> f) { return unchecked((long)f()); }
    public static long CallWideInt(Func<long> f) { return f(); }

    public static int PrepareGC()
    {
        Pair[] pairs = New<Pair>(17);
        pairs[16] = new Pair { A = 123, B = 456, Ref = new Leaf { Value = 77 } };
        Root = pairs;
        return 0;
    }
    public static int VerifyGC()
    {
        Pair[] pairs = (Pair[])Root;
        Check(pairs.Length == 17 && pairs[16].A == 123 && pairs[16].B == 456 && ((Leaf)pairs[16].Ref).Value == 77);
        return 0;
    }
    public static int ReleaseGC() { Root = null; return 0; }
    public static int PrepareFinalizerArray()
    {
        Finalized.Count = 0;
        Finalized[] values = New<Finalized>(3);
        values[1] = new Finalized();
        Root = values;
        return 0;
    }
    public static int VerifyFinalizerAlive() { Check(Finalized.Count == 0 && ((Finalized[])Root)[1] != null); return 0; }
    public static int VerifyFinalizerReleased() { Check(Finalized.Count == 1); return 0; }

    [MethodImpl(MethodImplOptions.NoInlining)] static int Invoke(Func<int, int> f, int x) { return f(x); }
    public static int Errors()
    {
        Func<int> failed = Failed.Read;
        try { failed(); return 8; } catch (TypeInitializationException) { }
        try { failed(); return 9; } catch (TypeInitializationException) { }
        try { CastBase(new Unrelated()); return 1; } catch (InvalidCastException) { }
        try { CastLeaf(new Middle()); return 2; } catch (InvalidCastException) { }
        try { CastGeneric<string>(17); return 3; } catch (InvalidCastException) { }
        Func<int, int> chain = First; chain += Fail; chain += Last; Log = 0;
        try { Invoke(chain, 1); return 4; } catch (ArgumentException) { Check(Log == 1); }
        try { Invoke(Throws, -1); return 5; } catch (ArgumentException) { }
        try { Invoke(null, 1); return 6; } catch (NullReferenceException) { }
        try { New<int>(-1); return 7; } catch (OverflowException) { }
        Check(New<int>(3).Length == 3 && Invoke(StaticStep, 1) == 8);
        return 0;
    }

    public static int Main() { int r = Casts() | Delegates() | Arrays() | Errors(); Console.WriteLine(r); return r; }
}
