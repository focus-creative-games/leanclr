using System;
using System.Reflection;
namespace ReflectionInvokeTests
{
public struct Payload
{
    public object Value;
    public int Number;
}
public enum Code : short
{
    A = 1,
    B = 2
}
public static class ConstructorCounters
{
    public static int Calls;
    public static int Initializers;
    public static object Last;
}
public class Existing
{
    public int Value;
    static Existing()
    {
        ConstructorCounters.Initializers++;
    }
    public Existing()
    {
        ConstructorCounters.Calls++;
        ConstructorCounters.Last = this;
        Value = 42;
    }
}
public struct ExistingValue
{
    public int Value;
    public ExistingValue(int value)
    {
        ConstructorCounters.Calls++;
        Value = value;
    }
}
public class Program
{
    static void Check(bool b, string label)
    {
        if (!b)
            throw new Exception(label);
    }
    static MethodInfo Method(string name)
    {
        return typeof(Program).GetMethod(name);
    }
    public static void Change(ref int x)
    {
        x = 42;
    }
    public static void Output(out int x)
    {
        x = 43;
    }
    public static void References(ref object x, out string y)
    {
        x = new object();
        y = "changed";
        GC.Collect();
    }
    public static void Nullable(ref int? x, out int? y)
    {
        x = null;
        y = 44;
    }
    public static void Enum(ref Code x)
    {
        x = Code.B;
    }
    public static Payload Complex(ref Payload x, out int y)
    {
        x.Value = new object();
        x.Number = 45;
        GC.Collect();
        y = 46;
        return new Payload { Value = x.Value, Number = 47 };
    }
    public static void Throw(ref int x, ref object y)
    {
        x = 48;
        y = "changed";
        throw new InvalidOperationException();
    }
    public static void RefValue()
    {
        object box = 1;
        object[] a = { box };
        Method("Change").Invoke(null, a);
        Check((int)a[0] == 42 && (int)box == 1, "ref copy and alias");
    }
    public static void OutValue()
    {
        object[] a = { null };
        Method("Output").Invoke(null, a);
        Check((int)a[0] == 43, "out null");
    }
    public static void NullRefValue()
    {
        object[] a = { null };
        Method("Change").Invoke(null, a);
        Check((int)a[0] == 42, "ref null");
    }
    public static void RefObjects()
    {
        object old = new object();
        object[] a = { old, null };
        Method("References").Invoke(null, a);
        Check(!object.ReferenceEquals(old, a[0]) && (string)a[1] == "changed", "ref objects");
    }
    public static void RefNullable()
    {
        object[] a = { 1, null };
        Method("Nullable").Invoke(null, a);
        Check(a[0] == null && (int)a[1] == 44, "nullable");
    }
    public static void RefEnum()
    {
        object[] a = { Code.A };
        Method("Enum").Invoke(null, a);
        Check((Code)a[0] == Code.B, "enum");
    }
    public static void RefStruct()
    {
        object original = new Payload { Value = "old", Number = 1 };
        object[] a = { original, null };
        Payload r = (Payload)Method("Complex").Invoke(null, a);
        Payload p = (Payload)a[0];
        Check(p.Number == 45 && (int)a[1] == 46 && r.Number == 47 && object.ReferenceEquals(r.Value, p.Value) &&
                  ((Payload)original).Number == 1,
              "struct and return roots");
    }
    public static void OnException()
    {
        object[] a = { 1, "old" };
        try
        {
            Method("Throw").Invoke(null, a);
        }
        catch (Exception)
        {
        }
        Check((int)a[0] == 1 && (string)a[1] == "old", "no copyback after throw");
    }
    public static void Main(string[] args)
    {
        typeof(Program).GetMethod(args[0]).Invoke(null, null);
        Console.WriteLine("PASS " + args[0]);
    }
}
}
