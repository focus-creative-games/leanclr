using System;

namespace GenericAotLibrary
{
    public class EhError<T> : Exception { }
    public class EhRef<T> { public int Marker; }
    public static class Eh
    {
        public static int Effects, Catches, Finals, Tails;
        public static void Reset() { Effects = Catches = Finals = Tails = 0; }
        public static T Echo<T>(T value, int divisor)
        {
            T local = value;
            try { ++Effects; int check = 12 / divisor; if (check == -1) ++Effects; return local; }
            catch (DivideByZeroException) { ++Catches; return local; }
            finally { ++Finals; }
        }
        public static T Mutate<T>(T value, T replacement, int divisor)
        {
            T local = value;
            try { value = replacement; local = value; ++Effects; int check = 12 / divisor; if (check == -1) ++Effects; return local; }
            catch (DivideByZeroException) { ++Catches; return local; }
            finally { ++Finals; }
        }
        public static int TypedCatch<T>(bool fail)
        {
            try { ++Effects; if (fail) throw new EhError<T>(); return 7; }
            catch (EhError<T>) { ++Catches; return 11; }
            finally { ++Finals; }
        }
        public static int Nested<T>(int divisor)
        {
            try { try { ++Effects; return 12 / divisor; } finally { ++Finals; } }
            catch (DivideByZeroException) { ++Catches; return -1; }
            finally { ++Finals; }
        }
        public static T FinallyThrows<T>(T value)
        {
            try { ++Effects; return value; }
            finally { ++Finals; throw new InvalidOperationException(); }
        }
        public static void Rethrow<T>()
        {
            try { ++Effects; throw new EhError<T>(); }
            catch (EhError<T>) { ++Catches; throw; }
            finally { ++Finals; }
        }
        public static void Void<T>(int divisor)
        {
            try { ++Effects; int value = 12 / divisor; if (value == -1) ++Effects; }
            catch (DivideByZeroException) { ++Catches; }
            finally { ++Finals; }
        }
        public static int Continuation<T>(int divisor)
        {
            int result;
            try { ++Effects; result = 12 / divisor; }
            catch (DivideByZeroException) { ++Catches; result = 100; }
            ++Tails;
            return result + (typeof(T) == typeof(int) ? 1 : 2);
        }
        public static T[] Array<T>(T[][] values, int index)
        {
            T[][] alias = values;
            try { ++Effects; return alias[index]; }
            catch (IndexOutOfRangeException) { ++Catches; return alias[0]; }
            finally { ++Finals; }
        }
        public static T ByRef<T>(ref T value)
        {
            try { return value; } catch { return value; }
        }
        public static void Change(ref int value) { value += 1; }
        public static int Address<T>()
        {
            int value = 7;
            try { Change(ref value); return value; } catch { return -1; }
        }
        public static T Filter<T>(T value)
        {
            try { throw new InvalidOperationException(); }
            catch (Exception) when (typeof(T) == typeof(int)) { return value; }
        }
        public static T ComplexFinally<T>(T value)
        {
            try { return value; }
            finally { try { ++Finals; } catch { ++Catches; } }
        }
    }
    public class EhHolder<T>
    {
        public static int Initializations;
        public T Value;
        public int Marker;
        static EhHolder() { try { Initializations = 1; } catch { Initializations = 2; } }
        public EhHolder(T value, int divisor)
        {
            try { ++Eh.Effects; Value = value; Marker = 12 / divisor; }
            catch (DivideByZeroException) { ++Eh.Catches; Marker = -1; }
            finally { ++Eh.Finals; }
        }
        public U Combine<U>(T value, U other, int divisor)
        {
            T saved = Value;
            U local = other;
            try { ++Eh.Effects; Value = value; int result = 12 / divisor; if (result == -1) ++Eh.Effects; return local; }
            catch (DivideByZeroException) { ++Eh.Catches; Value = saved; return local; }
            finally { ++Eh.Finals; }
        }
    }
    public struct EhValue<T>
    {
        public int Read(int divisor) { try { return 12 / divisor; } catch (DivideByZeroException) { return -1; } }
    }
}
