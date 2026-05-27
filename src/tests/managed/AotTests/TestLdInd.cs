using System;


public class TestLdInd
{

    [UnitTest]
    public void i1_1()
    {
        sbyte i = 1;
        ref sbyte j = ref i;
        Assert.Equal(1, j);
    }


    [UnitTest]
    public void i1_2()
    {
        sbyte i = -1;
        ref sbyte j = ref i;
        Assert.Equal(-1, j);
    }


    [UnitTest]
    public void u1()
    {
        byte i = 1;
        ref byte j = ref i;
        Assert.Equal(1, j);
    }


    [UnitTest]
    public void i2()
    {
        short i = 1;
        ref short j = ref i;
        Assert.Equal(1, j);
    }


    [UnitTest]
    public void i2_2()
    {
        short i = -1;
        ref short j = ref i;
        Assert.Equal(-1, j);
    }


    [UnitTest]
    public void u2()
    {
        ushort i = 1;
        ref ushort j = ref i;
        Assert.Equal(1, j);
    }


    [UnitTest]
    public void i4()
    {
        int i = 1;
        ref int j = ref i;
        Assert.Equal(1, j);
    }


    [UnitTest]
    public void i4_2()
    {
        int i = -1;
        ref int j = ref i;
        Assert.Equal(-1, j);
    }


    [UnitTest]
    public void u4()
    {
        uint i = 1U;
        ref uint j = ref i;
        Assert.Equal(1, j);
    }


    [UnitTest]
    public void i8()
    {
        long i = 1;
        ref long j = ref i;
        Assert.Equal(1, j);
    }


    [UnitTest]
    public void i8_2()
    {
        long i = -1;
        ref long j = ref i;
        Assert.Equal(-1, j);
    }

    [UnitTest]
    public void u8()
    {
        ulong i = 1;
        ref ulong j = ref i;
        Assert.Equal(1, j);
    }


    [UnitTest]
    public void nint_1()
    {
        IntPtr i = new IntPtr(1);
        ref var j = ref i;
        Assert.Equal(new IntPtr(1), j);
    }


    [UnitTest]
    public void nint_2()
    {
        IntPtr i = new IntPtr(-1);
        ref var j = ref i;
        Assert.Equal(new IntPtr(-1), j);
    }


    [UnitTest]
    public void r4()
    {
        float i = 1f;
        ref float j = ref i;
        Assert.Equal(1f, j);
    }


    [UnitTest]
    public void r8()
    {
        double i = 1.0;
        ref double j = ref i;
        Assert.Equal(1.0, j);
    }


    [UnitTest]
    public void obj()
    {
        object o = "a";
        ref object j = ref o;
        Assert.Equal("a", j);
    }
}
