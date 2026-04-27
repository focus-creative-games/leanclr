using System;
using System.Diagnostics;

[AttributeUsage(AttributeTargets.Method)]
public class UnitTestAttribute : Attribute
{
}

public abstract class GeneralTestCaseBase
{

}



public class AssertException : Exception
{
    public AssertException(string message) : base(message)
    {
    }
}

public class Assert
{
    public static void Fail()
    {
        Debugger.Log(0, "Assert", $"Assert.Fail:");
        throw new Exception($"Assert.Fail:");
    }

    public static void Fail(string message)
    {
        Debugger.Log(0, "Assert", $"Assert.Fail: {message}");
        throw new Exception($"Assert.Fail: {message}");
    }

    public static void IsTrue(bool condition)
    {
        if (!condition)
        {
            Debugger.Log(0, "Assert", $"Assert.IsTrue failed");
            throw new Exception($"Assert.IsTrue failed");
            //throw new Exception($"Assert.IsTrue failed");
        }
    }

    public static void IsFalse(bool condition)
    {
        if (condition)
        {
            Debugger.Log(0, "Assert", $"Assert.IsFalse failed");
            throw new Exception($"Assert.IsFalse failed");
        }
    }

    public static void True(bool condition)
    {
        if (!condition)
        {
            Debugger.Log(0, "Assert", $"Assert.IsTrue failed");
            throw new Exception($"Assert.IsTrue failed");
        }
    }

    public static void False(bool condition)
    {
        if (condition)
        {
            Debugger.Log(0, "Assert", $"Assert.IsFalse failed");
            throw new Exception($"Assert.IsFalse failed");
        }
    }

    public static void Null(object obj)
    {
        if (obj != null)
        {
            Debugger.Log(0, "Assert", $"Assert.NNull failed: object is not null");
            throw new Exception($"Assert.NNull failed: object is not null");
        }
    }

    public static void NotNull(object obj)
    {
        if (obj == null)
        {
            Debugger.Log(0, "Assert", $"Assert.NotNull failed: object is null");
            throw new Exception($"Assert.NotNull failed: object is null");
        }
    }

    public static void Equal(bool a, bool b)
    {
        if (a != b)
        {
            Debugger.Log(0, "Assert", $"Assert.Equal failed: {a} != {b}");
            throw new Exception($"Assert.Equal failed: {a} != {b}");
        }
    }

    public static void Equal(int a, int b)
    {
        if (a != b)
        {
            Debugger.Log(0, "Assert", $"Assert.Equal failed: {a} != {b}");
            throw new Exception($"Assert.Equal failed: {a} != {b}");
            //throw new Exception($"Assert.Equal failed: {a} != {b}");
        }
    }

    public static void Equal(long a, long b)
    {
        if (a != b)
        {
            Debugger.Log(0, "Assert", $"Assert.Equal failed: {a} != {b}");
            throw new Exception($"Assert.Equal failed: {a} != {b}");
            //throw new Exception($"Assert.Equal failed: {a} != {b}");
        }
    }

    public static void Equal(float a, float b)
    {
        if (float.IsNaN(a) && float.IsNaN(b))
        {
            return;
        }
        if (a != b)
        {
            Debugger.Log(0, "Assert", $"Assert.Equal failed: {a} != {b}");
            throw new Exception($"Assert.Equal failed: {a} != {b}");
        }
    }

    public static void Equal(double a, double b)
    {
        if (double.IsNaN(a) && double.IsNaN(b))
        {
            return;
        }
        if (a != b)
        {
            Debugger.Log(0, "Assert", $"Assert.Equal failed: {a} != {b}");
            throw new Exception($"Assert.Equal failed: {a} != {b}");
            //throw new Exception($"Assert.Equal failed: {a} != {b}");
        }
    }

    public static void Equal(Type a, Type b)
    {
        if (a != b)
        {
            Debugger.Log(0, "Assert", $"Assert.Equal failed: {a} != {b}");
            throw new Exception($"Assert.Equal failed: {a} != {b}");
            //throw new Exception($"Assert.Equal failed: {a} != {b}");
        }
    }

    public static void Equal(string a, string b)
    {
        if (a != b)
        {
            Debugger.Log(0, "Assert", $"Assert.Equal failed: {a} != {b}");
            throw new Exception($"Assert.Equal failed: {a} != {b}");
            //throw new Exception($"Assert.Equal failed: {a} != {b}");
        }
    }

    public unsafe static void Equal(int* a, int* b)
    {
        if (a != b)
        {
            Debugger.Log(0, "Assert", $"Assert.Equal failed: {(long)a} != {(long)b}");
            throw new Exception($"Assert.Equal failed: {(long)a} != {(long)b}");
            //throw new Exception($"Assert.Equal failed: {(long)a} != {(long)b}");
        }
    }

    //public static void Equal(decimal a, decimal b)
    //{
    //    if (a != b)
    //    {
    //        throw new Exception($"Assert.Equal failed:");
    //        //throw new Exception($"Assert.Equal failed: {a} != {b}");
    //    }
    //}

    public static void Equal(char a, char b)
    {
        if (a != b)
        {
            Debugger.Log(0, "Assert", $"Assert.Equal failed: {a} != {b}");
            throw new Exception($"Assert.Equal failed: {a} != {b}");
            //throw new Exception($"Assert.Equal failed: {a} != {b}");
        }
    }

    public static void Equal<T>(T a, T b)
    {
        if (!Object.Equals(a, b))
        {
            Debugger.Log(0, "Assert", $"Assert.Equal failed: {a} != {b}");
            throw new Exception($"Assert.Equal failed: {a} != {b}");
            //throw new Exception($"Assert.Equal failed: {a} != {b}");
        }
    }

    public static void NotEqual(int a, int b)
    {
        if (a == b)
        {
            Debugger.Log(0, "Assert", $"Assert.NotEqual failed: {a} == {b}");
            throw new Exception($"Assert.NotEqual failed: {a} == {b}");
            //throw new Exception($"Assert.NotEqual failed: {a} == {b}");
        }
    }

    public static void EqualAny(object a, object b)
    {
        if (!Object.Equals(a, b))
        {
            Debugger.Log(0, "Assert", $"Assert.NotEqual failed: {a} == {b}");
            throw new Exception($"Assert.NotEqual failed: {a} == {b}");
            //throw new Exception($"Assert.NotEqual failed: {a} == {b}");
        }
    }



    public static void ExpectException<T>(Action action)
    {
        try
        {
            action();
        }
        catch (Exception ex)
        {
            if (ex is T)
            {
                return;
            }
            else
            {
                Debugger.Log(0, "Assert", $"Assert.ExpectException failed: expected exception of type {typeof(T)}, but got {ex.GetType()}");
                throw new AssertException($"Assert.ExpectException failed: expected exception of type {typeof(T)}, but got {ex.GetType()}");
            }
        }
        Debugger.Log(0, "Assert", $"Assert.ExpectException failed: expected exception of type {typeof(T)}, but no exception was thrown");
        throw new AssertException($"Assert.ExpectException failed: expected exception of type {typeof(T)}, but no exception was thrown");
    }
}
