using System;


internal class Test_ldelem_any_i1
{

    public static T GetEle<T>(T[] arr, int index)
    {
        return arr[index];
    }


    public static void SetEle<T>(T[] arr, int index, T value)
    {
        arr[index] = value;
    }

    [UnitTest]
    public void ld_1()
    {
        var arr = new sbyte[] { 1, 2 };
        var x = GetEle(arr, 0);
        Assert.Equal(1, x);
        var y = GetEle(arr, 1);
        Assert.Equal(2, y);
    }

    [UnitTest]
    public void OutOfRange_lower()
    {
        Assert.ExpectException<IndexOutOfRangeException>(() =>
        {
            var arr = new sbyte[] { 1, 2 };
            var s = GetEle(arr, -1);
        });
    }

    [UnitTest]
    public void OutOfRange_upper()
    {
        Assert.ExpectException<IndexOutOfRangeException>(() =>
        {
            var arr = new sbyte[] { 1, 2 };
            var s = GetEle(arr, 2);
        });
    }

    [UnitTest]
    public void NullRef()
    {
        Assert.ExpectException<NullReferenceException>(() =>
        {
            sbyte[] arr = null;
            var s = GetEle(arr, 0);
        });
    }
}

internal class TC_ldelem_any_i2
{

    public static T GetEle<T>(T[] arr, int index)
    {
        return arr[index];
    }


    public static void SetEle<T>(T[] arr, int index, T value)
    {
        arr[index] = value;
    }

    [UnitTest]
    public void ld_1()
    {
        var arr = new short[] { 1, 2 };
        var x = GetEle(arr, 0);
        Assert.Equal(1, x);
        var y = GetEle(arr, 1);
        Assert.Equal(2, y);
    }

    //[UnitTest]
    //public void OutOfRange_lower()
    //{
    //    var arr = new short[] { 1, 2 };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = GetEle(arr, -1);
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void OutOfRange_upper()
    //{
    //    var arr = new short[] { 1, 2 };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = GetEle(arr, 2);
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void NullRef()
    //{
    //    short[] arr = null;
    //    Assert.ExpectException<NullReferenceException>();
    //    var s = GetEle(arr, 0);
    //    Assert.Fail();
    //}
}

internal class Test_ldelem_any_i4
{

    public static T GetEle<T>(T[] arr, int index)
    {
        return arr[index];
    }


    public static void SetEle<T>(T[] arr, int index, T value)
    {
        arr[index] = value;
    }

    [UnitTest]
    public void ld_1()
    {
        var arr = new int[] { 1, 2 };
        var x = GetEle(arr, 0);
        Assert.Equal(1, x);
        var y = GetEle(arr, 1);
        Assert.Equal(2, y);
    }

    [UnitTest]
    public void OutOfRange_lower()
    {
        Assert.ExpectException<IndexOutOfRangeException>(() =>
        {
            var arr = new int[] { 1, 2 };
            var s = GetEle(arr, -1);
        });
    }

    [UnitTest]
    public void OutOfRange_upper()
    {
        Assert.ExpectException<IndexOutOfRangeException>(() =>
        {
            var arr = new int[] { 1, 2 };
            var s = GetEle(arr, 2);
        });
    }

    [UnitTest]
    public void NullRef()
    {
        Assert.ExpectException<NullReferenceException>(() =>
        {
            int[] arr = null;
            var s = GetEle(arr, 0);
        });
    }
}
internal class TC_ldelem_any_i8
{

    public static T GetEle<T>(T[] arr, int index)
    {
        return arr[index];
    }


    public static void SetEle<T>(T[] arr, int index, T value)
    {
        arr[index] = value;
    }

    [UnitTest]
    public void ld_1()
    {
        var arr = new long[] { 1, 2 };
        var x = GetEle(arr, 0);
        Assert.Equal(1, x);
        var y = GetEle(arr, 1);
        Assert.Equal(2, y);
    }

    //[UnitTest]
    //public void OutOfRange_lower()
    //{
    //    var arr = new long[] { 1, 2 };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = GetEle(arr, -1);
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void OutOfRange_upper()
    //{
    //    var arr = new long[] { 1, 2 };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = GetEle(arr, 2);
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void NullRef()
    //{
    //    long[] arr = null;
    //    Assert.ExpectException<NullReferenceException>();
    //    var s = GetEle(arr, 0);
    //    Assert.Fail();
    //}
}

internal class TC_ldelem_any_u1
{

    public static T GetEle<T>(T[] arr, int index)
    {
        return arr[index];
    }


    public static void SetEle<T>(T[] arr, int index, T value)
    {
        arr[index] = value;
    }

    [UnitTest]
    public void ld_1()
    {
        var arr = new byte[] { 1, 2 };
        var x = GetEle(arr, 0);
        Assert.Equal(1, x);
        var y = GetEle(arr, 1);
        Assert.Equal(2, y);
    }

    //[UnitTest]
    //public void OutOfRange_lower()
    //{
    //    var arr = new byte[] { 1, 2 };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = GetEle(arr, -1);
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void OutOfRange_upper()
    //{
    //    var arr = new byte[] { 1, 2 };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = GetEle(arr, 2);
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void NullRef()
    //{
    //    byte[] arr = null;
    //    Assert.ExpectException<NullReferenceException>();
    //    var s = GetEle(arr, 0);
    //    Assert.Fail();
    //}
}
internal class TC_ldelem_any_u2
{

    public static T GetEle<T>(T[] arr, int index)
    {
        return arr[index];
    }


    public static void SetEle<T>(T[] arr, int index, T value)
    {
        arr[index] = value;
    }

    [UnitTest]
    public void ld_1()
    {
        var arr = new ushort[] { 1, 2 };
        var x = GetEle(arr, 0);
        Assert.Equal(1, x);
        var y = GetEle(arr, 1);
        Assert.Equal(2, y);
    }

    //[UnitTest]
    //public void OutOfRange_lower()
    //{
    //    var arr = new ushort[] { 1, 2 };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = GetEle(arr, -1);
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void OutOfRange_upper()
    //{
    //    var arr = new ushort[] { 1, 2 };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = GetEle(arr, 2);
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void NullRef()
    //{
    //    ushort[] arr = null;
    //    Assert.ExpectException<NullReferenceException>();
    //    var s = GetEle(arr, 0);
    //    Assert.Fail();
    //}
}

internal class TC_ldelem_any_u4
{

    public static T GetEle<T>(T[] arr, int index)
    {
        return arr[index];
    }


    public static void SetEle<T>(T[] arr, int index, T value)
    {
        arr[index] = value;
    }

    [UnitTest]
    public void ld_1()
    {
        var arr = new uint[] { 1, 2 };
        var x = GetEle(arr, 0);
        Assert.Equal(1, x);
        var y = GetEle(arr, 1);
        Assert.Equal(2, y);
    }

    //[UnitTest]
    //public void OutOfRange_lower()
    //{
    //    var arr = new uint[] { 1, 2 };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = GetEle(arr, -1);
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void OutOfRange_upper()
    //{
    //    var arr = new uint[] { 1, 2 };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = GetEle(arr, 2);
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void NullRef()
    //{
    //    uint[] arr = null;
    //    Assert.ExpectException<NullReferenceException>();
    //    var s = GetEle(arr, 0);
    //    Assert.Fail();
    //}
}

internal class TC_ldelem_any_u8
{

    public static T GetEle<T>(T[] arr, int index)
    {
        return arr[index];
    }


    public static void SetEle<T>(T[] arr, int index, T value)
    {
        arr[index] = value;
    }

    [UnitTest]
    public void ld_1()
    {
        var arr = new ulong[] { 1, 2 };
        var x = GetEle(arr, 0);
        Assert.Equal(1, x);
        var y = GetEle(arr, 1);
        Assert.Equal(2, y);
    }

    //[UnitTest]
    //public void OutOfRange_lower()
    //{
    //    var arr = new ulong[] { 1, 2 };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = GetEle(arr, -1);
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void OutOfRange_upper()
    //{
    //    var arr = new ulong[] { 1, 2 };
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    var s = GetEle(arr, 2);
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void NullRef()
    //{
    //    ulong[] arr = null;
    //    Assert.ExpectException<NullReferenceException>();
    //    var s = GetEle(arr, 0);
    //    Assert.Fail();
    //}
}

internal class Test_ldelem_any_ref
{

    public static T GetEle<T>(T[] arr, int index)
    {
        return arr[index];
    }


    public static void SetEle<T>(T[] arr, int index, T value)
    {
        arr[index] = value;
    }

    [UnitTest]
    public void ld_1()
    {
        var arr = new string[] { "1", "2" };
        var x = GetEle(arr, 0);
        Assert.Equal("1", x);
        var y = GetEle(arr, 1);
        Assert.Equal("2", y);
    }

    [UnitTest]
    public void OutOfRange_lower()
    {
        Assert.ExpectException<IndexOutOfRangeException>(() =>
        {
            var arr = new string[] { "1", "2" };
            var s = GetEle(arr, -1);
        });
    }

    [UnitTest]
    public void OutOfRange_upper()
    {
        Assert.ExpectException<IndexOutOfRangeException>(() =>
        {
            var arr = new string[] { "1", "2" };
            var s = GetEle(arr, 2);
        });
    }

    [UnitTest]
    public void NullRef()
    {
        Assert.ExpectException<NullReferenceException>(() =>
        {
            string[] arr = null;
            var s = GetEle(arr, 0);
        });
    }
}
