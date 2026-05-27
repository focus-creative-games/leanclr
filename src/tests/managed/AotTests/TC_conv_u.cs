using System;

class TC_conv_u
{
    [UnitTest]
    public unsafe void byte_1()
    {
        byte x = 1;
        int* y = (int*)x;
        Assert.Equal((int*)1, y);
    }

    [UnitTest]
    public unsafe void sbyte_1()
    {
        sbyte x = 1;
        int* y = (int*)x;
        Assert.Equal((int*)1, y);
    }

    [UnitTest]
    public unsafe void sbyte_2()
    {
        sbyte x = -1;
        int* y = (int*)x;
        Assert.Equal((int*)(-1), y);
    }

    [UnitTest]
    public unsafe void short_1()
    {
        short x = 1;
        int* y = (int*)x;
        Assert.Equal((int*)1, y);
    }

    [UnitTest]
    public unsafe void short_2()
    {
        short x = -1;
        int* y = (int*)x;
        Assert.Equal((int*)(-1), y);
    }

    [UnitTest]
    public unsafe void ushort_1()
    {
        ushort x = 1;
        int* y = (int*)x;
        Assert.Equal((int*)1, y);
    }

    [UnitTest]
    public unsafe void char_1()
    {
        char x = (char)1;
        int* y = (int*)(ushort)x;
        Assert.Equal((int*)1, y);
    }

    [UnitTest]
    public unsafe void int_1()
    {
        int x = 1;
        int* y = (int*)x;
        Assert.Equal((int*)1, y);
    }

    [UnitTest]
    public unsafe void int_2()
    {
        int x = -1;
        int* y = (int*)x;
        Assert.Equal((int*)(-1), y);
    }

    [UnitTest]
    public unsafe void uint_1()
    {
        uint x = 1;
        int* y = (int*)x;
        Assert.Equal((int*)1, y);
    }

    [UnitTest]
    public unsafe void uint_2()
    {
        uint x = 0xFFFFFFFF;
        int* y = (int*)x;
        Assert.Equal((int*)0xFFFFFFFF, y);
    }

    [UnitTest]
    public unsafe void ulong_1()
    {
        ulong x = 1;
        int* y = (int*)x;
        Assert.Equal((int*)1, y);
    }

    [UnitTest]
    public unsafe void ulong_overflow_up()
    {
        ulong x = ~0UL;
        int* y = (int*)x;
        Assert.Equal((int*)(-1), y);
    }

    [UnitTest]
    public unsafe void nint_1()
    {
        int* x = (int*)1;
        int* y = (int*)x;
        Assert.Equal((int*)1, y);
    }

    [UnitTest]
    public unsafe void float_1()
    {
        float x = 1;
        int* y = (int*)(long)x;
        Assert.Equal((int*)1, y);
    }

    [UnitTest]
    public unsafe void float_2()
    {
        float x = -1;
        int* y = (int*)(long)x;
        Assert.Equal((int*)(-1), y);
    }

    //[UnitTest]
    //public unsafe void float_overflow_up()
    //{
    //    float x = 0x100000001;
    //    int* y = (int*)x;
    //    Assert.Equal((int*)1, y);
    //}

    //[UnitTest]
    //public unsafe void float_overflow_down()
    //{
    //    float x = -0x100000001;
    //    int* y = (int*)x;
    //    Assert.Equal((int*)(-1), y);
    //}

    [UnitTest]
    public unsafe void double_1()
    {
        double x = 1;
        int* y = (int*)(IntPtr)x;
        Assert.Equal((int*)1, y);
    }

    [UnitTest]
    public unsafe void double_2()
    {
        double x = -1;
        int* y = (int*)(IntPtr)x;
        Assert.Equal((int*)(-1), y);
    }

    //[UnitTest]
    //public unsafe void double_overflow_up()
    //{
    //    double x = 0x1_00000001;
    //    int* y = (int*)x;
    //    Assert.Equal((int*)1, y);
    //}

    //[UnitTest]
    //public unsafe void double_overflow_down()
    //{
    //    double x = -0x1_00000001;
    //    int* y = (int*)x;
    //    Assert.Equal((int*)(-1), y);
    //}
}
