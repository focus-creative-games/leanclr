using System;

internal class TC_div_un
{
    [UnitTest]
    public static void uint_1()
    {
        uint a = 7;
        uint b = 3;
        Assert.Equal(2U, a / b);
    }

    [UnitTest]
    public static void ulong_1()
    {
        ulong a = 7;
        ulong b = 3;
        Assert.Equal(2UL, a / b);
    }

    [UnitTest]
    public static void uint_DivideByZeroException()
    {
        uint a = 1;
        uint b = 0;
        Assert.ExpectException<DivideByZeroException>(() =>
        {
            uint c = a / b;
        });
    }

    [UnitTest]
    public static void ulong_DivideByZeroException()
    {
        ulong a = 1;
        ulong b = 0;
        Assert.ExpectException<DivideByZeroException>(() =>
        {
            ulong c = a / b;
        });

    }
}
