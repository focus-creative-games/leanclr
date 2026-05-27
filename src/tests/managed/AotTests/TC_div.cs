using System;


class TC_div
{
    [UnitTest]
    public static void byte_1()
    {
        byte a = 7;
        byte b = 3;
        Assert.Equal(2, a / b);
    }

    [UnitTest]
    public static void sbyte_1()
    {
        sbyte a = -7;
        sbyte b = 3;
        Assert.Equal(-2, a / b);
    }

    [UnitTest]
    public static void short_1()
    {
        short a = -7;
        short b = 3;
        Assert.Equal(-2, a / b);
    }

    [UnitTest]
    public static void ushort_1()
    {
        ushort a = 7;
        ushort b = 3;
        Assert.Equal(2, a / b);
    }

    [UnitTest]
    public static void int_1()
    {
        int a = 7;
        int b = 3;
        Assert.Equal(2, a / b);
    }

    [UnitTest]
    public static void int_2()
    {
        int a = -7;
        int b = 3;
        Assert.Equal(-2, a / b);
    }

    [UnitTest]
    public static void int_DivideByZeroException()
    {
        int a = 1;
        int b = 0;
        Assert.ExpectException<DivideByZeroException>(() =>
        {
            var c = a / b;
        });
    }

    [UnitTest]
    public static void int_OverflowException()
    {
        int a = int.MinValue;
        int b = -1;
        Assert.ExpectException<OverflowException>(() =>
        {
            var c = a / b;
        });
    }

    [UnitTest]
    public static void uint_1()
    {
        uint a = 7;
        uint b = 3;
        Assert.Equal(2, a / b);
    }

    [UnitTest]
    public static void long_1()
    {
        long a = 7;
        long b = 3;
        Assert.Equal(2, a / b);
    }

    [UnitTest]
    public static void long_2()
    {
        long a = -7;
        long b = 3;
        Assert.Equal(-2, a / b);
    }

    [UnitTest]
    public static void long_DivideByZeroException()
    {
        long a = 1;
        long b = 0;
        Assert.ExpectException<DivideByZeroException>(() =>
        {
            var c = a / b;
        });
    }

    [UnitTest]
    public static void long_OverflowException()
    {
        long a = long.MinValue;
        long b = -1;
        // throw OverflowException actually
        Assert.ExpectException<ArithmeticException>(() =>
        {
            var c = a / b;
        });

    }

    [UnitTest]
    public static void ulong_1()
    {
        ulong a = 7;
        ulong b = 3;
        Assert.Equal(2L, a / b);
    }

    [UnitTest]
    public static void float_1()
    {
        float a = 6f;
        float b = 3f;
        Assert.Equal(2f, a / b);
    }

    [UnitTest]
    public static void float_NaN()
    {
        float a = 0f;
        float b = 0f;
        Assert.True(float.IsNaN(a / b));
    }

    [UnitTest]
    public static void float_NaN_1()
    {
        float a = float.PositiveInfinity;
        float b = float.PositiveInfinity;
        Assert.True(float.IsNaN(a / b));
    }

    [UnitTest]
    public static void float_0()
    {
        float a = 1;
        float b = float.PositiveInfinity;
        Assert.Equal(0, a / b);
    }

    [UnitTest]
    public static void double_1()
    {
        double a = 6.0;
        double b = 3.0;
        Assert.Equal(2.0, a / b);
    }

    [UnitTest]
    public static void double_NaN()
    {
        double a = 0d;
        double b = 0d;
        Assert.True(double.IsNaN(a / b));
    }

    [UnitTest]
    public static void double_NaN_1()
    {
        double a = double.PositiveInfinity;
        double b = double.PositiveInfinity;
        Assert.True(double.IsNaN(a / b));
    }

    [UnitTest]
    public static void double_0()
    {
        double a = 1d;
        double b = double.PositiveInfinity;
        Assert.Equal(0d, a / b);
    }
}