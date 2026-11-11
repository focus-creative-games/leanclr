using System;
public class FailedInitializer
{
    static FailedInitializer() { throw new Exception("initializer failed"); }
    public static int Read() { return 1; }
}
public class RecursiveInitializer
{
    public static int Value;
    static RecursiveInitializer() { Value = Read() + 1; }
    public static int Read() { return Value; }
}
public struct LargeReturn
{
    public long A, B, C, D, E;
}
public class InvocationFixture
{
    public static LargeReturn ManyArguments(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j,
        int k, int l, int m, int n, int o, int p, int q, int r, int s, int t)
    {
        return new LargeReturn { A = a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t, B = 2, C = 3, D = 4, E = 5 };
    }
}
public class FinalizerFixture
{
    public static bool Revive;
    public static FinalizerFixture Resurrected;
    ~FinalizerFixture() { if (Revive) Resurrected = this; }
}
public class IntegerFixture
{
    public static int Add32(int a, int b) { return unchecked(a+b); }
    public static int Sub32(int a, int b) { return unchecked(a-b); }
    public static int Mul32(int a, int b) { return unchecked(a*b); }
    public static int Neg32(int a) { return unchecked(-a); }
    public static int Shl32(int a, int b) { return a << b; }
    public static int Shr32(int a, int b) { return a >> b; }
    public static int ShrUn32(int a, int b) { return (int)((uint)a >> b); }
    public static long Add64(long a, long b) { return unchecked(a+b); }
    public static long Sub64(long a, long b) { return unchecked(a-b); }
    public static long Mul64(long a, long b) { return unchecked(a*b); }
    public static long Neg64(long a) { return unchecked(-a); }
    public static long Shl64(long a, int b) { return a << b; }
    public static long Shr64(long a, int b) { return a >> b; }
    public static long ShrUn64(long a, int b) { return (long)((ulong)a >> b); }
}
