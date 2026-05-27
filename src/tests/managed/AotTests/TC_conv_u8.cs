using System;

class TC_conv_u8
{
    [UnitTest]
    public void byte_1()
    {
        byte x = 1;
        ulong y = x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public void sbyte_1()
    {
        sbyte x = 1;
        ulong y = (ulong)x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public void sbyte_2()
    {
        sbyte x = -1;
        ulong y = (ulong)x;
        Assert.Equal(~0UL, y);
    }

    [UnitTest]
    public void short_1()
    {
        short x = 1;
        ulong y = (ulong)x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public void short_2()
    {
        short x = -1;
        ulong y = (ulong)x;
        Assert.Equal(~0UL, y);
    }

    [UnitTest]
    public void ushort_1()
    {
        ushort x = 1;
        ulong y = x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public void char_1()
    {
        char x = (char)1;
        ulong y = x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public void int_1()
    {
        int x = 1;
        ulong y = (ulong)x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public void int_2()
    {
        int x = -1;
        ulong y = (ulong)x;
        Assert.Equal(~0UL, y);
    }

    [UnitTest]
    public void uint_1()
    {
        uint x = 1;
        ulong y = x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public void uint_2()
    {
        uint x = 0xFFFFFFFF;
        ulong y = x;
        Assert.Equal(0xFFFFFFFF, y);
    }

    [UnitTest]
    public void long_1()
    {
        long x = 1;
        ulong y = (ulong)x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public void long_2()
    {
        long x = 0xFFFFFFFF;
        long x2 = (0x1L << 32);
        Assert.Equal(x2 - 1, x);
        ulong y = (ulong)x;
        Assert.Equal(0xFFFFFFFFUL, y);
    }

    [UnitTest]
    public void long_3()
    {
        long x = 0x10FFFFFFFF;
        ulong y = (ulong)x;
        Assert.Equal(0x10FFFFFFFFUL, y);
    }

    [UnitTest]
    public unsafe void nint_1()
    {
        int* x = (int*)1;
        ulong y = (ulong)x;
        Assert.Equal(1, y);
    }

    [UnitTest]
    public unsafe void nint_2()
    {
        int* x = (int*)-1;
        ulong y = (ulong)x;
        if (sizeof(int*) == 8)
        {
            Assert.Equal(~0UL, y);
        }
        else
        {
            Assert.Equal(~(uint)0, y);
        }
    }

    [UnitTest]
    public void float_1()
    {
        float x = 1;
        ulong y = (ulong)x;
        Assert.Equal(1, y);
    }

//     [UnitTest]
//     public void float_2()
//     {
// #if !UNITY_WEBGL
//         float x = -1;
//         ulong y = (ulong)x;
//         Assert.Equal(~0UL, y);
// #endif
//     }

    //[UnitTest]
    //public void float_overflow_up()
    //{
    //   float x = 0x100000001;
    //   ulong y = (ulong)x;
    //   Assert.Equal(1, y);
    //}

    //[UnitTest]
    //public void float_overflow_down()
    //{
    //   float x = -0x100000001;
    //   ulong y = (ulong)x;
    //   Assert.Equal(-1, y);
    //}

    [UnitTest]
    public void double_1()
    {
        double x = 1;
        ulong y = (ulong)x;
        Assert.Equal(1, y);
    }

//     [UnitTest]
//     public void double_2()
//     {

// #if !UNITY_WEBGL
//         double x = -1;
//         ulong y = (ulong)x;
//         Assert.Equal(~0UL, y);
// #endif
//     }

    //[UnitTest]
    //public void double_overflow_up()
    //{
    //   double x = 0x1_00000001;
    //   ulong y = (ulong)x;
    //   Assert.Equal(1, y);
    //}

    //[UnitTest]
    //public void double_overflow_down()
    //{
    //   double x = -0x1_00000001;
    //   ulong y = (ulong)x;
    //   Assert.Equal(-1, y);
    //}
}
