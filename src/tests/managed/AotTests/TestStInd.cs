using System;


public class TestStInd
{

    [UnitTest]
    public void i1_1()
    {
        sbyte i = 1;
        ref sbyte j = ref i;
        j = 2;
        Assert.Equal(2, i);
    }


    [UnitTest]
    public void i1_2()
    {
        sbyte i = 1;
        ref sbyte j = ref i;
        j = -1;
        Assert.Equal(-1, i);
    }


    [UnitTest]
    public void u1()
    {
        byte i = 1;
        ref byte j = ref i;
        j = 2;
        Assert.Equal(2, i);
    }


    [UnitTest]
    public void i2()
    {
        short i = 1;
        ref short j = ref i;
        j = 2;
        Assert.Equal(2, i);
    }


    [UnitTest]
    public void i2_2()
    {
        short i = 1;
        ref short j = ref i;
        j = -1;
        Assert.Equal(-1, i);
    }


    [UnitTest]
    public void u2()
    {
        ushort i = 1;
        ref ushort j = ref i;
        j = 2;
        Assert.Equal(2, i);
    }


    [UnitTest]
    public void i4()
    {
        int i = 1;
        ref int j = ref i;
        j = 2;
        Assert.Equal(2, i);
    }


    [UnitTest]
    public void i4_2()
    {
        int i = 1;
        ref int j = ref i;
        j = -1;
        Assert.Equal(-1, i);
    }


    [UnitTest]
    public void u4()
    {
        uint i = 1U;
        ref uint j = ref i;
        j = 2;
        Assert.Equal(2, (int)i);
    }


    [UnitTest]
    public void i8()
    {
        long i = 1;
        ref long j = ref i;
        j = 2;
        Assert.Equal(2, i);
    }


    [UnitTest]
    public void i8_2()
    {
        long i = 1;
        ref long j = ref i;
        j = -1;
        Assert.Equal(-1, i);
    }

    [UnitTest]
    public void u8()
    {
        ulong i = 1;
        ref ulong j = ref i;
        j = 2;
        Assert.Equal(2, (long)i);
    }


    [UnitTest]
    public void nint_1()
    {
        IntPtr i = new IntPtr(1);
        ref var j = ref i;
        j = new IntPtr(2);
        Assert.Equal(new IntPtr(2), i);
    }


    [UnitTest]
    public void nint_2()
    {
        IntPtr i = new IntPtr(1);
        ref var j = ref i;
        j = new IntPtr(-1);
        Assert.Equal(new IntPtr(-1), i);
    }


    [UnitTest]
    public void r4()
    {
        float i = 1f;
        ref float j = ref i;
        j = 2f;
        Assert.Equal(2f, i);
    }


    [UnitTest]
    public void r8()
    {
        double i = 1.0;
        ref double j = ref i;
        j = 2.0;
        Assert.Equal(2.0, i);
    }


    [UnitTest]
    public void obj()
    {
        object o = "a";
        ref object j = ref o;
        j = "b";
        Assert.Equal("b", o);
    }
}
