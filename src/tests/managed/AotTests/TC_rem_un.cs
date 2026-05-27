using System;


internal class TC_un
{
    [UnitTest]
    public static void un_uint()
    {
        uint a = 9;
        uint b = 7;
        Assert.Equal(2, a % b);
    }

    [UnitTest]
    public static void un_uint_DivideByZeroException()
    {
        uint a = 9;
        uint b = 0;
        Assert.ExpectException<DivideByZeroException>(() =>
        {
            ulong c = a % b;
        });
    }

    [UnitTest]
    public static void un_ulong()
    {
        ulong a = 9UL;
        ulong b = 7UL;
        Assert.Equal(2UL, a % b);
    }

    [UnitTest]
    public static void un_ulong_DivideByZeroException()
    {
        Assert.ExpectException<DivideByZeroException>(() =>
        {
            ulong a = 9UL;
            ulong b = 0UL;
            ulong c = a % b;
        });
    }
}


