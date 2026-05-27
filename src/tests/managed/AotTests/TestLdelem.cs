using System;



internal class TC_ldelem_i1
{

    [UnitTest]
    public void ld_1()
    {
        var arr = new sbyte[] { -1, 1 };
        var x = arr[0];
        Assert.Equal(-1, x);
        var y = arr[1];
        Assert.Equal(1, y);

        //var x2 = ArrayVerifyUtil.Get(arr, 0);
        //Assert.Equal(-1, x2);
        //var y2 = ArrayVerifyUtil.Get(arr, 1);
        //Assert.Equal(1, y2);
    }

    //[UnitTest]
    //public void OutOfRange_lower()
    //{
    //    var arr = new sbyte[] { -1, 1 };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = arr[-1];
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void OutOfRange_upper()
    //{
    //    var arr = new sbyte[] { -1, 1 };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = arr[2];
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void NullRef()
    //{
    //    sbyte[] arr = null;
    //    Assert.ExpectException<NullReferenceException>();
    //    var s = arr[0];
    //    Assert.Fail();
    //}
}

internal class TC_ldelem_i2
{

    [UnitTest]
    public void ld_1()
    {
        var arr = new short[] { -1, 1 };
        var x = arr[0];
        Assert.Equal(-1, x);
        var y = arr[1];
        Assert.Equal(1, y);
        //var x2 = ArrayVerifyUtil.Get(arr, 0);
        //Assert.Equal(-1, x2);
        //var y2 = ArrayVerifyUtil.Get(arr, 1);
        //Assert.Equal(1, y2);
    }

    //[UnitTest]
    //public void OutOfRange_lower()
    //{
    //    var arr = new short[] { -1, 1 };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = arr[-1];
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void OutOfRange_upper()
    //{
    //    var arr = new short[] { -1, 1 };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = arr[2];
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void NullRef()
    //{
    //    short[] arr = null;
    //    Assert.ExpectException<NullReferenceException>();
    //    var s = arr[0];
    //    Assert.Fail();
    //}
}

internal class TC_ldelem_i4
{

    [UnitTest]
    public void ld_1()
    {
        var arr = new int[] { -1, 1 };
        var x = arr[0];
        Assert.Equal(-1, x);
        var y = arr[1];
        Assert.Equal(1, y);
        //var x2 = ArrayVerifyUtil.Get(arr, 0);
        //Assert.Equal(-1, x2);
        //var y2 = ArrayVerifyUtil.Get(arr, 1);
        //Assert.Equal(1, y2);
    }

    //[UnitTest]
    //public void OutOfRange_lower()
    //{
    //    var arr = new int[] { -1, 1 };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = arr[-1];
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void OutOfRange_upper()
    //{
    //    var arr = new int[] { -1, 1 };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = arr[2];
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void NullRef()
    //{
    //    int[] arr = null;
    //    Assert.ExpectException<NullReferenceException>();
    //    var s = arr[0];
    //    Assert.Fail();
    //}
}

internal class TC_ldelem_i8
{

    [UnitTest]
    public void ld_1()
    {
        var arr = new long[] { -1, 1 };
        var x = arr[0];
        Assert.Equal(-1, x);
        var y = arr[1];
        Assert.Equal(1, y);
        //var x2 = ArrayVerifyUtil.Get(arr, 0);
        //Assert.Equal(-1, x2);
        //var y2 = ArrayVerifyUtil.Get(arr, 1);
        //Assert.Equal(1, y2);
    }

    //[UnitTest]
    //public void OutOfRange_lower()
    //{
    //    var arr = new long[] { -1, 1 };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = arr[-1];
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void OutOfRange_upper()
    //{
    //    var arr = new long[] { -1, 1 };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = arr[2];
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void NullRef()
    //{
    //    long[] arr = null;
    //    Assert.ExpectException<NullReferenceException>();
    //    var s = arr[0];
    //    Assert.Fail();
    //}
}

internal class Test_ldelem_u1
{

    [UnitTest]
    public void ld_1()
    {
        var arr = new byte[] { 1, 2 };
        var x = arr[0];
        Assert.Equal(1, x);
        var y = arr[1];
        Assert.Equal(2, y);
    }

    [UnitTest]
    public void OutOfRange_lower()
    {
        Assert.ExpectException<IndexOutOfRangeException>(() =>
        {
            var arr = new byte[] { 1, 2 };
            int index = -1;
            var s = arr[index];
        });
    }

    [UnitTest]
    public void OutOfRange_upper()
    {
        Assert.ExpectException<IndexOutOfRangeException>(() =>
        {
            var arr = new byte[] { 1, 2 };
            var s = arr[2];
        });
    }

    [UnitTest]
    public void NullRef()
    {
        Assert.ExpectException<NullReferenceException>(() =>
        {
            byte[] arr = null;
            var s = arr[0];
        });
    }
}

internal class TC_ldelem_u1
{

    [UnitTest]
    public void ld_1()
    {
        var arr = new byte[] { 1, 2 };
        var x = arr[0];
        Assert.Equal(1, x);
        var y = arr[1];
        Assert.Equal(2, y);
    }

    //[UnitTest]
    //public void OutOfRange_lower()
    //{
    //    var arr = new byte[] { 1, 2 };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = arr[-1];
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void OutOfRange_upper()
    //{
    //    var arr = new byte[] { 1, 2 };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = arr[2];
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void NullRef()
    //{
    //    byte[] arr = null;
    //    Assert.ExpectException<NullReferenceException>();
    //    var s = arr[0];
    //    Assert.Fail();
    //}
}

internal class TC_ldelem_u2
{

    [UnitTest]
    public void ld_1()
    {
        var arr = new ushort[] { 1, 2 };
        var x = arr[0];
        Assert.Equal(1, x);
        var y = arr[1];
        Assert.Equal(2, y);
    }

    //[UnitTest]
    //public void OutOfRange_lower()
    //{
    //    var arr = new ushort[] { 1, 2 };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = arr[-1];
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void OutOfRange_upper()
    //{
    //    var arr = new ushort[] { 1, 2 };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = arr[2];
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void NullRef()
    //{
    //    ushort[] arr = null;
    //    Assert.ExpectException<NullReferenceException>();
    //    var s = arr[0];
    //    Assert.Fail();
    //}
}

internal class TC_ldelem_u4
{

    [UnitTest]
    public void ld_1()
    {
        var arr = new uint[] { 1, 2 };
        var x = arr[0];
        Assert.Equal(1, x);
        var y = arr[1];
        Assert.Equal(2, y);
    }

    //[UnitTest]
    //public void OutOfRange_lower()
    //{
    //    var arr = new uint[] { 1, 2 };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = arr[-1];
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void OutOfRange_upper()
    //{
    //    var arr = new uint[] { 1, 2 };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = arr[2];
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void NullRef()
    //{
    //    uint[] arr = null;
    //    Assert.ExpectException<NullReferenceException>();
    //    var s = arr[0];
    //    Assert.Fail();
    //}
}

internal class TC_ldelem_u8
{

    [UnitTest]
    public void ld_1()
    {
        var arr = new ulong[] { 1, 2 };
        var x = arr[0];
        Assert.Equal(1, x);
        var y = arr[1];
        Assert.Equal(2, y);
    }

    //[UnitTest]
    //public void OutOfRange_lower()
    //{
    //    var arr = new ulong[] { 1, 2 };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = arr[-1];
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void OutOfRange_upper()
    //{
    //    var arr = new ulong[] { 1, 2 };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = arr[2];
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void NullRef()
    //{
    //    ulong[] arr = null;
    //    Assert.ExpectException<NullReferenceException>();
    //    var s = arr[0];
    //    Assert.Fail();
    //}
}

internal class TC_ldelem_i
{

    static IntPtr GetArrayValueInterp(IntPtr[] arr, int index)
    {
        return arr[index];
    }


    [UnitTest]
    public void ld_1()
    {
        var arr = new IntPtr[] { new IntPtr(1), new IntPtr(2) };
        var x = arr[0];
        Assert.Equal(new IntPtr(1), x);
        var y = arr[1];
        Assert.Equal(new IntPtr(2), y);

        var x2 = GetArrayValueInterp(arr, 0);
        Assert.Equal(new IntPtr(1), x2);
        var y2 = GetArrayValueInterp(arr, 1);
        Assert.Equal(new IntPtr(2), y2);
    }

    //[UnitTest]
    //public void OutOfRange_lower()
    //{
    //    var arr = new IntPtr[] { new IntPtr(1), new IntPtr(2) };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = arr[-1];
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void OutOfRange_upper()
    //{
    //    var arr = new IntPtr[] { new IntPtr(1), new IntPtr(2) };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = arr[2];
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void NullRef()
    //{
    //    IntPtr[] arr = null;
    //    Assert.ExpectException<NullReferenceException>();
    //    var s = arr[0];
    //    Assert.Fail();
    //}
}

internal class Test_ldelem_r4
{

    [UnitTest]
    public void ld_1()
    {
        var arr = new float[] { 1, 2 };
        var x = arr[0];
        Assert.Equal(1f, x);
        var y = arr[1];
        Assert.Equal(2f, y);
        //var x2 = ArrayVerifyUtil.Get(arr, 0);
        //Assert.Equal(1f, x2);
        //var y2 = ArrayVerifyUtil.Get(arr, 1);
        //Assert.Equal(2f, y2);
    }

    [UnitTest]
    public void OutOfRange_lower()
    {
        Assert.ExpectException<IndexOutOfRangeException>(() =>
        {
            var arr = new float[] { 1, 2 };
            int index = -1;
            var s = arr[index];
        });
    }

    [UnitTest]
    public void OutOfRange_upper()
    {
        Assert.ExpectException<IndexOutOfRangeException>(() =>
        {
            var arr = new float[] { 1, 2 };
            var s = arr[2];
        });
    }

    [UnitTest]
    public void NullRef()
    {
        Assert.ExpectException<NullReferenceException>(() =>
        {
            float[] arr = null;
            var s = arr[0];
        });
    }
}

internal class TC_ldelem_r8
{

    [UnitTest]
    public void ld_1()
    {
        var arr = new double[] { 1, 2 };
        var x = arr[0];
        Assert.Equal(1, x);
        var y = arr[1];
        Assert.Equal(2, y);
        //var x2 = ArrayVerifyUtil.Get(arr, 0);
        //Assert.Equal(1, x2);
        //var y2 = ArrayVerifyUtil.Get(arr, 1);
        //Assert.Equal(2, y2);
    }

    //[UnitTest]
    //public void OutOfRange_lower()
    //{
    //    var arr = new double[] { 1, 2 };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = arr[-1];
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void OutOfRange_upper()
    //{
    //    var arr = new double[] { 1, 2 };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = arr[2];
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void NullRef()
    //{
    //    double[] arr = null;
    //    Assert.ExpectException<NullReferenceException>();
    //    var s = arr[0];
    //    Assert.Fail();
    //}
}

internal class TC_ldelem_ref
{

    [UnitTest]
    public void string_1()
    {
        var arr = new string[] { "1", "2", "3" };
        var s = arr[1];
        Assert.Equal("2", s);
        //var x2 = ArrayVerifyUtil.Get(arr, 1);
        //Assert.Equal("2", x2);
    }

    //[UnitTest]
    //public void string_OutOfRange_lower()
    //{
    //    var arr = new string[] { "1", "2", "3" };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = arr[-1];
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void string_OutOfRange_upper()
    //{
    //    var arr = new string[] { "1", "2", "3" };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = arr[3];
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void string_NullRef()
    //{
    //    string[] arr = null;
    //    Assert.ExpectException<NullReferenceException>();
    //    var s = arr[0];
    //    Assert.Fail();
    //}
}

internal class TC_ldelem
{

    [UnitTest]
    public void ld_1()
    {
        var arr = new ValueTypeSize1[] { new ValueTypeSize1 { x1 = 1 }, new ValueTypeSize1 { x1 = 2 } };

        var x = arr[0];
        Assert.Equal(1, x.x1);
        var y = arr[1];
        Assert.Equal(2, y.x1);
    }


    [UnitTest]
    public void ld_4()
    {
        var arr = new ValueTypeSize4[] { new ValueTypeSize4 { x1 = 1 }, new ValueTypeSize4 { x1 = 2 } };

        var x = arr[0];
        Assert.Equal(1, x.x1);
        var y = arr[1];
        Assert.Equal(2, y.x1);

    }


    [UnitTest]
    public void ld_8()
    {
        var arr = new ValueTypeSize8[] { new ValueTypeSize8 { x1 = 1 }, new ValueTypeSize8 { x1 = 2 } };

        var x = arr[0];
        Assert.Equal(1, x.x1);
        var y = arr[1];
        Assert.Equal(2, y.x1);

    }


    [UnitTest]
    public void ld_9()
    {
        var arr = new ValueTypeSize9[] { new ValueTypeSize9 { x1 = 1 }, new ValueTypeSize9 { x1 = 2 } };

        var x = arr[0];
        Assert.Equal(1, x.x1);
        var y = arr[1];
        Assert.Equal(2, y.x1);

    }

    [UnitTest]
    public void ld_16()
    {
        var arr = new ValueTypeSize16[] { new ValueTypeSize16 { x1 = 1 }, new ValueTypeSize16 { x1 = 2 } };

        var x = arr[0];
        Assert.Equal(1, x.x1);
        var y = arr[1];
        Assert.Equal(2, y.x1);

    }

    [UnitTest]
    public void ld_20()
    {
        var arr = new ValueTypeSize20[] { new ValueTypeSize20 { x1 = 1 }, new ValueTypeSize20 { x1 = 2 } };

        var x = arr[0];
        Assert.Equal(1, x.x1);
        var y = arr[1];
        Assert.Equal(2, y.x1);

    }

    //[UnitTest]
    //public void OutOfRange_lower()
    //{
    //    var arr = new ValueTypeSize4[] { new ValueTypeSize4 { x1 = 1 }, new ValueTypeSize4 { x1 = 2 } };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = arr[-1];
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void OutOfRange_upper()
    //{
    //    var arr = new ValueTypeSize4[] { new ValueTypeSize4 { x1 = 1 }, new ValueTypeSize4 { x1 = 2 } };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = arr[2];
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void NullRef()
    //{
    //    ValueTypeSize4[] arr = null;
    //    Assert.ExpectException<NullReferenceException>();
    //    var s = arr[0];
    //    Assert.Fail();
    //}
}
