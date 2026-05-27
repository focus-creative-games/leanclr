using System;

internal class TC_stelem_i
{

    [UnitTest]
    public void st_1()
    {
        var arr = new IntPtr[2];
        arr[0] = (IntPtr)1;
        arr[1] = (IntPtr)2;

        var x = arr[0];
        Assert.Equal((IntPtr)1, x);
        var y = arr[1];
        Assert.Equal((IntPtr)2, y);
    }

    [UnitTest]
    public void OutOfRange_lower()
    {
        Assert.ExpectException<IndexOutOfRangeException>(() =>
        {
            var arr = new IntPtr[2];
            int index = -1;
            arr[index] = (IntPtr)1;
        });
    }

    [UnitTest]
    public void OutOfRange_upper()
    {
        Assert.ExpectException<IndexOutOfRangeException>(() =>
        {
            var arr = new IntPtr[2];
            arr[2] = (IntPtr)1;
        });
    }

    [UnitTest]
    public void NullRef()
    {
        Assert.ExpectException<NullReferenceException>(() =>
        {
            IntPtr[] arr = null;
            arr[0] = (IntPtr)1;
        });
    }
}

internal class TC_stelem_i1
{

    [UnitTest]
    public void st_1()
    {
        var arr = new sbyte[2];
        arr[0] = -1;
        arr[1] = 1;

        var x = arr[0];
        Assert.Equal(-1, x);
        var y = arr[1];
        Assert.Equal(1, y);
    }

    [UnitTest]
    public void OutOfRange_lower()
    {
        Assert.ExpectException<IndexOutOfRangeException>(() =>
        {
            var arr = new sbyte[2];
            int index = -1;
            arr[index] = 1;
        });
    }

    [UnitTest]
    public void OutOfRange_upper()
    {
        Assert.ExpectException<IndexOutOfRangeException>(() =>
        {
            var arr = new sbyte[2];
            arr[2] = 1;
        });
    }

    [UnitTest]
    public void NullRef()
    {
        Assert.ExpectException<NullReferenceException>(() =>
        {
            sbyte[] arr = null;
            arr[0] = 1;
        });
    }
}

internal class TC_stelem_i2
{

    [UnitTest]
    public void st_1()
    {
        var arr = new short[2];
        arr[0] = -1;
        arr[1] = 1;

        var x = arr[0];
        Assert.Equal(-1, x);
        var y = arr[1];
        Assert.Equal(1, y);
    }

    //[UnitTest]
    //public void OutOfRange_lower()
    //{
    //    var arr = new short[2];
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    arr[-1] = 1;
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void OutOfRange_upper()
    //{
    //    var arr = new short[2];
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    arr[2] = 1;
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void NullRef()
    //{
    //    short[] arr = null;
    //    Assert.ExpectException<NullReferenceException>();
    //    arr[0] = 1;
    //    Assert.Fail();
    //}
}

internal class TC_stelem_i4
{

    [UnitTest]
    public void st_1()
    {
        var arr = new int[2];
        arr[0] = -1;
        arr[1] = 1;

        var x = arr[0];
        Assert.Equal(-1, x);
        var y = arr[1];
        Assert.Equal(1, y);
    }

    //[UnitTest]
    //public void OutOfRange_lower()
    //{
    //    var arr = new int[2];
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    int i = -1;
    //    arr[i] = 1;
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void OutOfRange_upper()
    //{
    //    var arr = new int[2];
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    arr[2] = 1;
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void NullRef()
    //{
    //    int[] arr = null;
    //    Assert.ExpectException<NullReferenceException>();
    //    arr[0] = 1;
    //    Assert.Fail();
    //}
}

internal class TC_stelem_i8
{

    [UnitTest]
    public void st_1()
    {
        var arr = new long[2];
        arr[0] = -1;
        arr[1] = 1;

        var x = arr[0];
        Assert.Equal(-1, x);
        var y = arr[1];
        Assert.Equal(1, y);
    }

    //[UnitTest]
    //public void OutOfRange_lower()
    //{
    //    var arr = new long[2];
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    int i = -1;
    //    arr[i] = 1;
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void OutOfRange_upper()
    //{
    //    var arr = new long[2];
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    arr[2] = 1;
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void NullRef()
    //{
    //    long[] arr = null;
    //    Assert.ExpectException<NullReferenceException>();
    //    arr[0] = 1;
    //    Assert.Fail();
    //}
}
internal class TC_stelem_r4
{

    [UnitTest]
    public void st_1()
    {
        var arr = new float[2];
        arr[0] = 1;
        arr[1] = 2;

        var x = arr[0];
        Assert.Equal(1, x);
        var y = arr[1];
        Assert.Equal(2, y);
    }

    //[UnitTest]
    //public void OutOfRange_lower()
    //{
    //    var arr = new float[2];
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    int i = -1;
    //    arr[i] = 1;
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void OutOfRange_upper()
    //{
    //    var arr = new float[2];
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    arr[2] = 1;
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void NullRef()
    //{
    //    float[] arr = null;
    //    Assert.ExpectException<NullReferenceException>();
    //    arr[0] = 1;
    //    Assert.Fail();
    //}
}
internal class TC_stelem_r8
{

    [UnitTest]
    public void st_1()
    {
        var arr = new double[2];
        arr[0] = 1;
        arr[1] = 2;

        var x = arr[0];
        Assert.Equal(1, x);
        var y = arr[1];
        Assert.Equal(2, y);
    }

    //[UnitTest]
    //public void OutOfRange_lower()
    //{
    //    var arr = new double[2];
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    int i = -1;
    //    arr[i] = 1;
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void OutOfRange_upper()
    //{
    //    var arr = new double[2];
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    arr[2] = 1;
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void NullRef()
    //{
    //    double[] arr = null;
    //    Assert.ExpectException<NullReferenceException>();
    //    arr[0] = 1;
    //    Assert.Fail();
    //}
}

internal class TC_stelem_ref
{

    [UnitTest]
    public void st_1()
    {
        var arr = new object[2];
        arr[0] = "1";
        arr[1] = "2";

        var x = arr[0];
        Assert.Equal("1", x);
        var y = arr[1];
        Assert.Equal("2", y);
    }

    //[UnitTest]
    //public void OutOfRange_lower()
    //{
    //    var arr = new object[2];
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    int i = -1;
    //    arr[i] = 1;
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void OutOfRange_upper()
    //{
    //    var arr = new object[2];
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    arr[2] = 1;
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void NullRef()
    //{
    //    object[] arr = null;
    //    Assert.ExpectException<NullReferenceException>();
    //    arr[0] = 1;
    //    Assert.Fail();
    //}
}

internal class TC_stelem_struct
{

    [UnitTest]
    public void st_1()
    {
        var arr = new ValueTypeSize4[2];
        arr[0] = new ValueTypeSize4 { x1 = 1 };
        arr[1] = new ValueTypeSize4 { x1 = 2 };
        var x = arr[0];
        Assert.Equal(1, x.x1);
        var y = arr[1];
        Assert.Equal(2, y.x1);
    }

    //[UnitTest]
    //public void OutOfRange_lower()
    //{
    //    var arr = new ValueTypeSize4[2];
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    int i = -1;
    //    arr[i] = new ValueTypeSize4 { x1 = 1 };
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void OutOfRange_upper()
    //{
    //    var arr = new ValueTypeSize4[2];
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    arr[2] = new ValueTypeSize4();
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void NullRef()
    //{
    //    ValueTypeSize4[] arr = null;
    //    Assert.ExpectException<NullReferenceException>();
    //    arr[0] = new ValueTypeSize4();
    //    Assert.Fail();
    //}
}
