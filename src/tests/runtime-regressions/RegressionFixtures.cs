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
