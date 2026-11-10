using System;
public static class EhFixtures
{
    public static int Counter;
    public static int Catch(int divisor)
    {
        int x = 7; Counter = 1;
        try { Counter++; x = 100 / divisor; Counter += 3; }
        catch (DivideByZeroException) { Counter += 10; x += 5; }
        Counter += 100;
        return x + Counter;
    }
    public static int MutatedArguments(int x)
    {
        try { x += 5; Counter++; x += 100 / (x - 8); }
        catch (DivideByZeroException) { return x; }
        return x;
    }
    public static int Nested(int divisor)
    {
        int x = 1; Counter = 0;
        try
        {
            try { Counter++; x = 100 / divisor; }
            catch (DivideByZeroException) { Counter += 10; x += 2; }
            finally { Counter += 100; x += 3; }
        }
        finally { Counter += 1000; x += 4; }
        return x + Counter;
    }
    public static int FinallyThrows()
    {
        Counter = 0;
        try
        {
            try { Counter++; }
            finally { Counter += 10; throw new InvalidOperationException(); }
        }
        catch (InvalidOperationException) { Counter += 100; }
        return Counter;
    }
    public static int ReturnFinally(int value)
    {
        try { if (value > 0) return value + 1; return value - 1; }
        finally { Counter++; }
    }
    public static void ThrowOnce() { Counter++; throw new InvalidOperationException(); }
    public static int CalleeFailure()
    {
        Counter = 0;
        try { Counter++; ThrowOnce(); Counter += 1000; }
        catch (InvalidOperationException) { Counter += 10; }
        return Counter;
    }
    public static int Rethrow()
    {
        Counter = 0;
        try
        {
            try { ThrowOnce(); }
            catch (InvalidOperationException) { Counter += 10; throw; }
        }
        catch (InvalidOperationException) { Counter += 100; }
        return Counter;
    }
    public static void Void()
    {
        try { ThrowOnce(); }
        catch (InvalidOperationException) { Counter += 2; }
        finally { Counter += 4; }
    }
    public static double Floating(double x, float y)
    {
        try { x += y; ThrowOnce(); }
        catch (InvalidOperationException) { return x; }
        return 0;
    }
    public static long Wide(long x)
    {
        try { x += 0x123456789L; ThrowOnce(); }
        catch (InvalidOperationException) { return x; }
        return 0;
    }
    public static int NullThrow()
    {
        try { throw (Exception)null; }
        catch (NullReferenceException) { return 17; }
    }
    public static int Loop()
    {
        int total = 0; Counter = 0;
        for (int i = 0; i < 10; ++i)
        {
            try { Counter++; total += 10 / (i - 3); }
            catch (DivideByZeroException) { total += 7; }
            finally { total += 2; }
        }
        return Counter * 1000 + total;
    }
    public static string ReferenceLocal()
    {
        string value = "preserved";
        int[] array = new int[] { 17 };
        try { Counter += array[2]; }
        catch (IndexOutOfRangeException) { return value + array[0]; }
        return "bad";
    }
    public static int AliasFallback(ref int value)
    {
        try { value++; ThrowOnce(); }
        catch (InvalidOperationException) { value++; }
        return value;
    }
    public static int ComplexFinallyFallback()
    {
        try { Counter++; }
        finally { try { ThrowOnce(); } catch (InvalidOperationException) { Counter++; } }
        return Counter;
    }
    public static uint NormalLoop(int n, int divisor)
    {
        uint value = 19;
        for (int i = 0; i < n; ++i)
        {
            try { value = unchecked(value * 1664525u + (uint)(100 / divisor)); }
            catch (DivideByZeroException) { value ^= 13; }
            finally { value = unchecked(value + 7); }
        }
        return value;
    }
    public static int Run()
    {
        if (Catch(0) != 124 || Catch(2) != 155) return 1;
        if (MutatedArguments(3) != 8) return 2;
        if (Nested(0) != 1121 || Nested(2) != 1158) return 3;
        if (FinallyThrows() != 111) return 4;
        Counter = 0;
        if (ReturnFinally(1) != 2 || ReturnFinally(-1) != -2 || Counter != 2) return 5;
        if (CalleeFailure() != 12 || Rethrow() != 111) return 6;
        if (Loop() != 10032) return 7;
        if (ReferenceLocal() != "preserved17") return 8;
        int alias = 1;
        if (AliasFallback(ref alias) != 3 || alias != 3) return 9;
        Counter = 0; Void();
        if (Counter != 7 || Floating(10.5, 1.25f) != 11.75) return 10;
        if (Wide(0x100000000L) != 0x223456789L || NullThrow() != 17) return 11;
        return 0;
    }
}
