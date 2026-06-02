using System;


class TestLdslfda
{
    public static int a;

    [ThreadStatic]
    public static int b;

    [UnitTest]
    public void int_1()
    {
        a = 0;
        ref int x = ref a;
        x = 1;
        Assert.Equal(1, a);
    }

    [UnitTest]
    public void int_threadstatic_1()
    {
        b = 0;
        ref int x = ref b;
        x = 1;
        Assert.Equal(1, b);
    }
}

