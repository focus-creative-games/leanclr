using System;
using System.Reflection;
namespace ReflectionMetadataTests
{
public class Base
{
    public virtual int V()
    {
        return 1;
    }
    protected virtual int P()
    {
        return 1;
    }
    public int Over(int x)
    {
        return x;
    }
    public int Over(string x)
    {
        return 2;
    }
    public virtual T G<T>(T x)
    {
        return x;
    }
}
public class Middle : Base
{
    public override int V()
    {
        return 2;
    }
    protected override int P()
    {
        return 2;
    }
    public override T G<T>(T x)
    {
        return x;
    }
}
public class Derived : Middle
{
    public override int V()
    {
        return 3;
    }
}
public class NewDerived : Base
{
    public new virtual int V()
    {
        return 4;
    }
}
public class GenericBase<T>
{
    public virtual T V(T x)
    {
        return x;
    }
    public virtual int V(int x)
    {
        return x;
    }
}
public class GenericDerived<T> : GenericBase<T>
{
    public override T V(T x)
    {
        return x;
    }
}
public class Program
{
    static void Check(bool b, string message)
    {
        if (!b)
            throw new Exception(message);
    }
    static int Count(Type t, string name, BindingFlags flags)
    {
        int n = 0;
        foreach (var m in t.GetMethods(flags))
            if (m.Name == name)
                n++;
        return n;
    }
    public static void Overrides()
    {
        Check(Count(typeof(Derived), "V", BindingFlags.Instance | BindingFlags.Public) == 1, "three levels");
        Check(typeof(Derived).GetMethod("V").DeclaringType == typeof(Derived), "most derived");
    }
    public static void Overloads()
    {
        Check(Count(typeof(Derived), "Over", BindingFlags.Instance | BindingFlags.Public) == 2, "overloads");
    }
    public static void Protected()
    {
        Check(Count(typeof(Derived), "P", BindingFlags.Instance | BindingFlags.NonPublic) == 1, "protected override");
    }
    public static void Declared()
    {
        Check(Count(typeof(Derived), "V", BindingFlags.Instance | BindingFlags.Public | BindingFlags.DeclaredOnly) == 1,
              "declared override");
        Check(Count(typeof(Derived), "Over", BindingFlags.Instance | BindingFlags.Public | BindingFlags.DeclaredOnly) ==
                  0,
              "declared inherited");
    }
    public static void GenericOverrides()
    {
        Check(Count(typeof(GenericDerived<string>), "V", BindingFlags.Instance | BindingFlags.Public) == 2,
              "generic class overloads");
        Check(Count(typeof(Derived), "G", BindingFlags.Instance | BindingFlags.Public) == 1, "generic method override");
    }
    public static void NewSlots()
    {
        Check(Count(typeof(NewDerived), "V", BindingFlags.Instance | BindingFlags.Public) == 2, "distinct new slots");
    }
    public static void Main(string[] args)
    {
        typeof(Program).GetMethod(args[0]).Invoke(null, null);
        Console.WriteLine("PASS " + args[0]);
    }
}
}
