//using System;

//[AttributeUsage(AttributeTargets.Method)]
//public class UnitTestAttribute : Attribute
//{
//}

//public class AssertException : Exception
//{
//    public AssertException(string message) : base(message)
//    {
//    }
//}

//public class Assert
//{
//    public static void Fail()
//    {
//        throw new AssertException($"Assert.Fail:");
//    }

//    public static void Fail(string message)
//    {
//        throw new AssertException($"Assert.Fail: {message}");
//    }

//    public static void IsTrue(bool condition)
//    {
//        if (!condition)
//        {
//            throw new AssertException($"Assert.IsTrue failed");
//        }
//    }

//    public static void IsFalse(bool condition)
//    {
//        if (condition)
//        {
//            throw new AssertException($"Assert.IsFalse failed");
//        }
//    }

//    public static void True(bool condition)
//    {
//        if (!condition)
//        {
//            throw new AssertException($"Assert.IsTrue failed");
//        }
//    }

//    public static void False(bool condition)
//    {
//        if (condition)
//        {
//            throw new AssertException($"Assert.IsFalse failed");
//        }
//    }

//    public static void Null(object obj)
//    {
//        if (obj != null)
//        {
//            throw new AssertException($"Assert.NNull failed: object is not null");
//        }
//    }

//    public static void NotNull(object obj)
//    {
//        if (obj == null)
//        {
//            throw new AssertException($"Assert.NotNull failed: object is null");
//        }
//    }

//    public static void Equal(bool a, bool b)
//    {
//        if (a != b)
//        {
//            throw new AssertException($"Assert.Equal failed: {a} != {b}");
//        }
//    }

//    public static void Equal(int a, int b)
//    {
//        if (a != b)
//        {
//            throw new AssertException($"Assert.Equal failed: {a} != {b}");
//        }
//    }

//    public static void Equal(long a, long b)
//    {
//        if (a != b)
//        {
//            throw new AssertException($"Assert.Equal failed: {a} != {b}");
//        }
//    }

//    public static void Equal(float a, float b)
//    {
//        if (float.IsNaN(a) && float.IsNaN(b))
//        {
//            return;
//        }
//        if (a != b)
//        {
//            throw new AssertException($"Assert.Equal failed: {a} != {b}");
//        }
//    }

//    public static void Equal(double a, double b)
//    {
//        if (double.IsNaN(a) && double.IsNaN(b))
//        {
//            return;
//        }
//        if (a != b)
//        {
//            throw new AssertException($"Assert.Equal failed: {a} != {b}");
//        }
//    }

//    public static void Equal(Type a, Type b)
//    {
//        if (a != b)
//        {
//            throw new AssertException($"Assert.Equal failed: {a} != {b}");
//        }
//    }

//    public static void Equal(string a, string b)
//    {
//        if (a != b)
//        {
//            throw new AssertException($"Assert.Equal failed: {a} != {b}");
//        }
//    }

//    public unsafe static void Equal(int* a, int* b)
//    {
//        if (a != b)
//        {
//            throw new AssertException($"Assert.Equal failed: {(long)a} != {(long)b}");
//        }
//    }

//    //public static void Equal(decimal a, decimal b)
//    //{
//    //    if (a != b)
//    //    {
//    //        throw new AssertException($"Assert.Equal failed:");
//    //        //throw new AssertException($"Assert.Equal failed: {a} != {b}");
//    //    }
//    //}

//    public static void Equal(char a, char b)
//    {
//        if (a != b)
//        {
//            throw new AssertException($"Assert.Equal failed: {a} != {b}");
//        }
//    }

//    public static void Equal<T>(T a, T b)
//    {
//        if (!Object.Equals(a, b))
//        {
//            throw new AssertException($"Assert.Equal failed: {a} != {b}");
//        }
//    }

//    public static void NotEqual(int a, int b)
//    {
//        if (a == b)
//        {
//            throw new AssertException($"Assert.NotEqual failed: {a} == {b}");
//        }
//    }

//    public static void EqualAny(object a, object b)
//    {
//        if (!Object.Equals(a, b))
//        {
//            throw new AssertException($"Assert.NotEqual failed: {a} == {b}");
//        }
//    }

//    public static void ExpectException<T>(Action action)
//    {
//        try
//        {
//            action();
//        }
//        catch (Exception ex)
//        {
//            if (ex is T)
//            {
//                return;
//            }
//            else
//            {
//                throw new AssertException($"Assert.ExpectException failed: expected exception of type {typeof(T)}, but got {ex.GetType()}");
//            }
//        }
//        throw new AssertException($"Assert.ExpectException failed: expected exception of type {typeof(T)}, but no exception was thrown");
//    }
//}
