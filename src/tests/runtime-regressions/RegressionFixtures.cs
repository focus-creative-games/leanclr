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
