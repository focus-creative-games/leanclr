using System;

class TC_conv_i8
{
    [UnitTest]
    public void byte_1()
    {
        byte x = 1;
        long y = x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public void sbyte_1()
    {
        sbyte x = 1;
        long y = x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public void sbyte_2()
    {
        sbyte x = -1;
        long y = x;
        Assert.Equal(-1, y);
    }

    [UnitTest]
    public void short_1()
    {
        short x = 1;
        long y = x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public void short_2()
    {
        short x = -1;
        long y = x;
        Assert.Equal(-1, y);
    }

    [UnitTest]
    public void ushort_1()
    {
        ushort x = 1;
        long y = x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public void char_1()
    {
        char x = (char)1;
        long y = x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public void int_1()
    {
        int x = 1;
        long y = x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public void int_2()
    {
        int x = -1;
        long y = x;
        Assert.Equal(-1, y);
    }

    [UnitTest]
    public void uint_1()
    {
        uint x = 1;
        long y = x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public void uint_2()
    {
        uint x = 0xFFFFFFFF;
        long y = x;
        Assert.Equal(0xFFFFFFFF, y);
    }

    [UnitTest]
    public void ulong_1()
    {
        ulong x = 1;
        long y = (long)x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public void ulong_overflow_up()
    {
        ulong x = ~0UL;
        long y = (long)x;
        Assert.Equal(-1L, y);
    }

    [UnitTest]
    public unsafe void nint_1()
    {
        int* x = (int*)1;
        long y = (long)x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public void float_1()
    {
        float x = 1;
        long y = (long)x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public void float_2()
    {
        float x = -1;
        long y = (long)x;
        Assert.Equal(-1, y);
    }

    //[UnitTest]
    //public void float_overflow_up()
    //{
    //    float x = 0x100000001;
    //    long y = (long)x;
    //    Assert.Equal(1, y);
    //}

    //[UnitTest]
    //public void float_overflow_down()
    //{
    //    float x = -0x100000001;
    //    long y = (long)x;
    //    Assert.Equal(-1, y);
    //}

    [UnitTest]
    public void double_1()
    {
        double x = 1;
        long y = (long)x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public void double_2()
    {
        double x = -1;
        long y = (long)x;
        Assert.Equal(-1, y);
    }

//    [UnitTest]
//    public void double_overflow_up()
//    {
//        double x = 0x1_00000001;
//        long y = (long)x;
//        Assert.Equal(1, y);
//    }

//    [UnitTest]
//    public void double_overflow_down()
//    {
//        double x = -0x1_00000001;
//        long y = (long)x;
//        Assert.Equal(-1, y);
//    }
}
