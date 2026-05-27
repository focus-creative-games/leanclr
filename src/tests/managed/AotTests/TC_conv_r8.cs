using System;


class TC_conv_r8
{
    [UnitTest]
    public void byte_1()
    {
        byte x = (byte)(int)(object)1;
        double y = x;
        Assert.Equal(1.0, y);
    }

    [UnitTest]
    public void sbyte_1()
    {
        sbyte x = (sbyte)(int)(object)1;
        double y = x;
        Assert.Equal(1.0, y);
    }

    [UnitTest]
    public void sbyte_2()
    {
        sbyte x = (sbyte)(int)(object)-1;
        double y = x;
        Assert.Equal(-1.0, y);
    }

    [UnitTest]
    public void short_1()
    {
        short x = (short)(int)(object)1;
        double y = x;
        Assert.Equal(1.0, y);
    }

    [UnitTest]
    public void short_2()
    {
        short x = (short)(int)(object)-1;
        double y = x;
        Assert.Equal(-1.0, y);
    }

    [UnitTest]
    public void ushort_1()
    {
        ushort x = (ushort)(int)(object)1;
        double y = x;
        Assert.Equal(1.0, y);
    }

    [UnitTest]
    public void char_1()
    {
        char x = (char)(int)(object)1;
        double y = x;
        Assert.Equal(1.0, y);
    }

    [UnitTest]
    public void int_1()
    {
        int x = (int)(object)1;
        double y = x;
        Assert.Equal(1.0, y);
    }

    [UnitTest]
    public void int_2()
    {
        int x = (int)(object)-1;
        double y = x;
        Assert.Equal(-1.0, y);
    }

    [UnitTest]
    public void long_1()
    {
        long x = (long)(object)1L;
        double y = x;
        Assert.Equal(1.0, y);
    }

    [UnitTest]
    public unsafe void nint_1()
    {
        IntPtr x = (IntPtr)(int)(object)1;
        double y = (double)x;
        Assert.Equal(1.0, y);
    }

    [UnitTest]
    public void float_1()
    {
        float x = (float)(object)1f;
        double y = x;
        Assert.Equal(1.0, y);
    }

    [UnitTest]
    public void double_1()
    {
        double x = (double)(object)1.0;
        double y = x;
        Assert.Equal(1.0, y);
    }
}
