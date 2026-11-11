using System;
public sealed class AotMethodAttribute : Attribute
{
    public AotMethodAttribute(bool enabled) { }
}
public struct GenericPair<T>
{
    public T Value;
    public T Set(T value) { Value = value; return Value; }
}
public class GenericHolder<T>
{
    public static int Count = 3;
    public T Value;
    public GenericHolder(T value) { Value = value; }
    public T Echo(T value) { Count++; Value = value; return GenericFixtures.Identity<T>(Value); }
    public static T Forward(T value) { return GenericFixtures.Identity<T>(value); }
}
public class FieldOnly<T> { public static int Value = 1; }
public sealed class VirtualOnly<T> : GenericFixtures.IRead { public int Read() { return typeof(T) == typeof(int) ? 71 : 73; } }
public static class GenericFixtures
{
    public static object NullReferenceResult() { return null; }
    public static void VoidResult(int divisor) { int value = 12 / divisor; if (value == 0) throw new Exception(); }
    public interface IReference { object Get(); }
    public interface IVoid { void Touch(); }
    public sealed class ReferenceReader : IReference, IVoid { public object Get() { return null; } public void Touch() { } }
    public static object ReferenceBridge(IReference value) { return value.Get(); }
    public static void VoidBridge(IVoid value) { value.Touch(); }
    public interface IRead { int Read(); }
    public interface IGenericRead<T> { T Read(); }
    public sealed class GenericReader<T> : IRead { public int Read() { return typeof(T) == typeof(int) ? 19 : 23; } }
    public sealed class PlainReader : IGenericRead<int> { public int Read() { return 29; } }
    public static int CallReader(IRead reader) { return reader.Read(); }
    public static int CallGenericReader(IGenericRead<int> reader) { return reader.Read(); }
    public struct Reader : IRead { public int Value; public int Read() { return Value; } }
    public static int Constrained<T>(T value) where T : struct, IRead { return value.Read(); }
    public static T ReferenceConstraint<T>(T value) where T : class { return Identity<T>(value); }
    public static T Identity<T>(T value) { return value; }
    public static T Forward<T>(T value) { return GenericHolder<T>.Forward(value); }
    public static T BoxRoundtrip<T>(T value) { return (T)(object)value; }
    public static T[] MakeArray<T>(T value) { T[] a = new T[1]; a[0] = value; return a; }
    public static int NoArguments<T>() { return typeof(T) == typeof(int) ? 11 : 22; }
    public static int Recurse<T>(int n) { return n == 0 ? 0 : Recurse<T>(n - 1) + 1; }
    public static int Grow<T>(int n) { return n == 0 ? 0 : Grow<T[]>(n - 1) + 1; }
    public static int Divide<T>(int denominator) { return 12 / denominator; }
    [AotMethod(false)] public static T Excluded<T>(T value) { return value; }
    public static T WithCatch<T>(T value) { try { return Identity<T>(value); } catch { return value; } }
    public static int Run()
    {
        if (Identity<int>(37) != 37 || Identity<long>(0x123456789L) != 0x123456789L) return 1;
        if (Identity<string>("hello") != "hello" || Identity<object>(null) != null) return 2;
        if (Forward<int>(17) != 17 || Forward<string>("forward") != "forward") return 3;
        var h = new GenericHolder<int>(5);
        if (h.Echo(23) != 23 || h.Value != 23 || GenericHolder<int>.Count != 4) return 4;
        var hs = new GenericHolder<string>("a");
        if (hs.Echo("b") != "b" || GenericHolder<string>.Count != 4) return 5;
        GenericPair<long> pair = new GenericPair<long>();
        if (pair.Set(0x234567891L) != 0x234567891L) return 6;
        if (Identity<GenericPair<long>>(pair).Value != pair.Value) return 7;
        if (BoxRoundtrip<int>(42) != 42 || BoxRoundtrip<string>("box") != "box") return 8;
        if (MakeArray<int>(19)[0] != 19 || MakeArray<string>("array")[0] != "array") return 9;
        if (NoArguments<int>() != 11 || NoArguments<string>() != 22) return 10;
        if (Recurse<int>(5) != 5 || Grow<int>(0) != 0) return 11;
        if (Divide<int>(3) != 4) return 12;
        if (Excluded<int>(9) != 9 || WithCatch<int>(10) != 10) return 13;
        if (Constrained<Reader>(new Reader { Value = 39 }) != 39) return 14;
        if (ReferenceConstraint<string>("constraint") != "constraint") return 15;
        if (GenericLibrary.Identity<int>(61) != 61) return 16;
        var genericReader = new GenericReader<int>();
        if (genericReader.Read() != 19 || CallReader(genericReader) != 19) return 17;
        if (CallGenericReader(new PlainReader()) != 29) return 18;
        Func<int, int> identity = Identity<int>;
        if (identity(43) != 43) return 19;
        Func<int> reader = genericReader.Read;
        if (reader() != 19) return 20;
        if ((string)GenericLibrary.Reference("cross-module") != "cross-module") return 21;
        if (GenericLibrary.Identity<string>("cross-generic") != "cross-generic") return 22;
        GenericLibrary.Touch(3);
        if (FieldOnly<ulong>.Value != 1) return 23;
        if (CallReader(new VirtualOnly<int>()) != 71) return 24;
        return 0;
    }
}
