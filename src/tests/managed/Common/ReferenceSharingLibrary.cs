using System.Runtime.CompilerServices;

namespace GenericAotLibrary
{
    public static class SharedReferences
    {
        [MethodImpl(MethodImplOptions.NoInlining)]
        public static T Select<T, U>(T first, T fallback, U ignored, int divisor) where T : class
        {
            int result = 12 / divisor;
            return result < 0 ? fallback : first;
        }
        [MethodImpl(MethodImplOptions.NoInlining)]
        public static T Loop<T>(T first, T fallback, int count, uint seed) where T : class
        {
            for (int i = 0; i < count; ++i) seed = unchecked(seed * 1664525u + 1013904223u);
            return (seed & 1) == 0 ? first : fallback;
        }
        public static int Context<T>() { return typeof(T) == typeof(string) ? 7 : 11; }
    }
    public class SharedReferenceHolder<T> where T : class
    {
        public U Second<U>(T first, U second) { return second; }
    }
}
