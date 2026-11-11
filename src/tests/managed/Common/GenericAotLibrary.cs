using System;

// Definition assembly deliberately stays outside the ordinary AOT root set.
namespace GenericAotLibrary
{
    public static class Functions
    {
        public static T Identity<T>(T value) { return value; }
        public static int Context<T>() { return typeof(T) == typeof(int) ? 11 : typeof(T) == typeof(string) ? 22 : 33; }
        public static bool SameAssembly<T, U>() { return typeof(T).Assembly == typeof(U).Assembly; }
        public static T[] Array<T>(T value) { return new T[] { value }; }
        public static T Box<T>(T value) { return (T)(object)value; }
        public static int Recurse<T>(int n) { return n == 0 ? 0 : Recurse<T>(n - 1) + 1; }
        public static int Grow<T>(int n) { return n == 0 ? 0 : Grow<T[]>(n - 1) + 1; }
        public static ref T ByRef<T>(ref T value) { return ref value; }
        public static int Divide<T>(int value) { return 12 / value; }
        public static T WithCatch<T>(T value) { try { return Identity<T>(value); } catch { return value; } }
        [AotMethod(false)] public static T Excluded<T>(T value) { return value; }
    }
    public class Box<T>
    {
        public static int Initializations;
        public static T Static;
        public T Value;
        static Box() { Initializations = 1; }
        public Box(T value) { Value = value; }
        public T Read() { return Value; }
        public U Combine<U>(T value, U other) { Value = value; return Functions.Identity<U>(other); }
    }
    public interface IRead { int Read(); }
    public interface IGeneric<T> { T Read(); }
    public struct Reader : IRead { public int Value; public int Read() { return Value; } }
    public static class Constraints
    {
        public static int Read<T>(T value) where T : struct, IRead { return value.Read(); }
        public static T Reference<T>(T value) where T : class { return value; }
    }
    public class VirtualBase
    {
        public virtual T Echo<T>(T value) { return value; }
        public virtual T Dynamic<T>(T value) { return value; }
    }
}
namespace GenericCollision
{
    public struct Payload { public int Marker; }
}
namespace GenericAotLibrary
{
    public class GenericBase<T>
    {
        public T Value;
        public virtual T ReadValue() { return Value; }
    }
}
