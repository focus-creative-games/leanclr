using System.Collections;
using System.Collections.Generic;


public interface IFoo
{
    int Calc();
}

public struct ForFunValue : System.IEquatable<ForFunValue>, IFoo
{
    public int a;

    public int b;

    public ForFunValue(int a, int b)
    {
        this.a = a;
        this.b = b;
    }

    public void Inc()
    {
        a += 1;
    }

    public int Show()
    {
        return a;
    }

    public override string ToString()
    {
        return a.ToString();
    }

    public bool Equals(ForFunValue x)
    {
        return a == x.a && b == x.b;
    }

    public int Calc()
    {
        return a + b;
    }

    public static int Default()
    {
        return 10;
    }

    public int CallGeneric<T>(T x)
    {
        return 100;
    }

    public static object CallStaticGeneric<T>(T x)
    {
        return x;
    }
}

public class ForFunClass : System.IEquatable<ForFunClass>, IFoo
{
    public int a;

    public int b;

    public ForFunClass(int a, int b)
    {
        this.a = a;
        this.b = b;
    }

    public int Show()
    {
        return a;
    }

    public int Show2()
    {
        return 2;
    }

    public void Inc()
    {
        a += 1;
    }

    public override string ToString()
    {
        return a.ToString();
    }

    public bool Equals(ForFunClass x)
    {
        return a == x.a && b == x.b;
    }

    public int Calc()
    {
        return a + b;
    }

    public static int Default()
    {
        return 10;
    }

    public string CallGeneric<T>(T x)
    {
        return $"{x}-{a}";
    }

    public static object CallStaticGeneric<T>(T x)
    {
        return x;
    }
    
}


public interface IBar<out T>
{
    T Sum(int a);
}

public interface IRun<in T>
{
    int Comput(T a);
}


public struct ForBarValue : IBar<object>, IRun<string>
{
    public object x;

    public int Comput(string a)
    {
        return 1;
    }

    public object Sum(int a)
    {
        return a;
    }
}

public class ForBarClass : IBar<object>, IRun<string>
{
    public object x;
    public object Sum(int a)
    {
        return 1;
    }
    public int Comput(string a)
    {
        return 1;
    }
}

public class ForBarClass2 : ForBarClass, IBar<string>, IRun<object>
{
    public string y;
    public new string Sum(int a)
    {
        return 2.ToString();
    }

    public int Comput(object a)
    {
        return 2;
    }
}

