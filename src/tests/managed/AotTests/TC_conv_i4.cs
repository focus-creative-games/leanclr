using System;


class TC_conv_i4
{
    [UnitTest]
    public void byte_1()
    {
        byte x = 1;
        int y = x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public void sbyte_1()
    {
        sbyte x = 1;
        int y = x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public void sbyte_2()
    {
        sbyte x = -1;
        int y = x;
        Assert.Equal(-1, y);
    }

    [UnitTest]
    public void short_1()
    {
        short x = 1;
        int y = x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public void short_2()
    {
        short x = -1;
        int y = x;
        Assert.Equal(-1, y);
    }

    [UnitTest]
    public void ushort_1()
    {
        ushort x = 1;
        int y = x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public void char_1()
    {
        char x = (char)1;
        int y = x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public void uint_1()
    {
        uint x = 1;
        int y = (int)x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public void uint_overflow_up()
    {
        uint x = 0xFFFFFFFF;
        int y = (int)x;
        Assert.Equal(-1, y);
    }

    [UnitTest]
    public void long_1()
    {
        long x = 1;
        int y = (int)x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public void long_overflow_up()
    {
        long x = 0x1_00000001;
        int y = (int)x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public void long_overflow_down()
    {
        long x = -0x1_00000001;
        int y = (int)x;
        Assert.Equal(-1, y);
    }

    [UnitTest]
    public void ulong_1()
    {
        ulong x = 1;
        int y = (int)x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public void ulong_overflow_up()
    {
        ulong x = 0x100000001;
        int y = (int)x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public unsafe void nint_1()
    {
        int* x = (int*)1;
        int y = (int)x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public unsafe void nint_overflow_up()
    {
        int* x = (int*)0x100000001;
        int y = (int)x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public unsafe void nint_overflow_down()
    {
        int* x = (int*)-0x100000001;
        int y = (int)x;
        Assert.Equal(-1, y);
    }

    [UnitTest]
    public unsafe void unint_1()
    {
        uint* x = (uint*)1;
        int y = (int)x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public unsafe void unint_overflow_up()
    {
        uint* x = (uint*)0x100000001;
        int y = (int)x;
        Assert.Equal(1, y);
    }


    [UnitTest]
    public void float_1()
    {
        float x = 1;
        int y = (int)x;
        Assert.Equal(1, y);
    }

    //[UnitTest]
    //public void float_overflow_up()
    //{
    //    float x = 0x100000001;
    //    int y = (int)x;
    //    Assert.Equal(1, y);
    //}

    //[UnitTest]
    //public void float_overflow_down()
    //{
    //    float x = -0x100000001;
    //    int y = (int)x;
    //    Assert.Equal(-1, y);
    //}

    [UnitTest]
    public void double_1()
    {
        double x = 1;
        int y = (int)x;
        Assert.Equal(1, y);
    }

    //[UnitTest]
    //public void double_overflow_up()
    //{
    //    double x = 0x1_00000001;
    //    int y = (int)x;
    //    Assert.Equal(1, y);
    //}

    //[UnitTest]
    //public void double_overflow_down()
    //{
    //    double x = -0x1_00000001;
    //    int y = (int)x;
    //    Assert.Equal(-1, y);
    //}
}
