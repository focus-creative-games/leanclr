using System;

internal class TC_stelem_any_i1
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
    public void st_1()
    {
        var arr = new sbyte[3] { 1, 1, 1 };
        SetEle<sbyte>(arr, 1, -1);

        var x = GetEle(arr, 0);
        Assert.Equal(1, x);
        var y = GetEle(arr, 1);
        Assert.Equal(-1, y);
        var z = GetEle(arr, 2);
        Assert.Equal(1, z);
    }

    [UnitTest]
    public void OutOfRange_lower()
    {
        Assert.ExpectException<IndexOutOfRangeException>(() =>
        {
            var arr = new sbyte[2];
            SetEle<sbyte>(arr, -1, 1);
        });
    }

    [UnitTest]
    public void OutOfRange_upper()
    {
        Assert.ExpectException<IndexOutOfRangeException>(() =>
        {
            var arr = new sbyte[2];
            SetEle<sbyte>(arr, 2, 1);
        });
    }

    [UnitTest]
    public void NullRef()
    {
        Assert.ExpectException<NullReferenceException>(() =>
        {
            sbyte[] arr = null;
            SetEle<sbyte>(arr, 0, 1);
        });
    }
}

internal class TC_stelem_any_i2
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
    public void st_1()
    {
        var arr = new short[3] { 1, 1, 1 };
        SetEle<short>(arr, 1, -1);

        var x = GetEle(arr, 0);
        Assert.Equal(1, x);
        var y = GetEle(arr, 1);
        Assert.Equal(-1, y);
        var z = GetEle(arr, 2);
        Assert.Equal(1, z);
    }

    //[UnitTest]
    //public void OutOfRange_lower()
    //{
    //    var arr = new short[2];
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    SetEle<short>(arr, -1, 1);
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void OutOfRange_upper()
    //{
    //    var arr = new short[2];
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    SetEle<short>(arr, 2, 1);
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void NullRef()
    //{
    //    short[] arr = null;
    //    Assert.ExpectException<NullReferenceException>();
    //    SetEle<short>(arr, 0, 1);
    //    Assert.Fail();
    //}
}

internal class TC_stelem_any_i4
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
    public void st_1()
    {
        var arr = new int[3] { 1, 1, 1 };
        SetEle(arr, 1, -1);

        var x = GetEle(arr, 0);
        Assert.Equal(1, x);
        var y = GetEle(arr, 1);
        Assert.Equal(-1, y);
        var z = GetEle(arr, 2);
        Assert.Equal(1, z);
    }

    //[UnitTest]
    //public void OutOfRange_lower()
    //{
    //    var arr = new int[2];
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    SetEle(arr, -1, 1);
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void OutOfRange_upper()
    //{
    //    var arr = new int[2];
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    SetEle(arr, 2, 1);
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void NullRef()
    //{
    //    int[] arr = null;
    //    Assert.ExpectException<NullReferenceException>();
    //    SetEle(arr, 0, 1);
    //    Assert.Fail();
    //}
}

internal class TC_stelem_any_i8
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
    public void st_1()
    {
        var arr = new long[3] { 1, 1, 1 };
        SetEle(arr, 1, -1);

        var x = GetEle(arr, 0);
        Assert.Equal(1, x);
        var y = GetEle(arr, 1);
        Assert.Equal(-1, y);
        var z = GetEle(arr, 2);
        Assert.Equal(1, z);
    }

    //[UnitTest]
    //public void OutOfRange_lower()
    //{
    //    var arr = new long[2];
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    SetEle(arr, -1, 1);
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void OutOfRange_upper()
    //{
    //    var arr = new long[2];
    //    Assert.ExpectException<IndexOutOfRangeException>();
    //    SetEle(arr, 2, 1);
    //    Assert.Fail();
    //}

    //[UnitTest]
    //public void NullRef()
    //{
    //    long[] arr = null;
    //    Assert.ExpectException<NullReferenceException>();
    //    SetEle(arr, 0, 1);
    //    Assert.Fail();
    //}
}

internal class TC_stelem_any_ref
{
    class Animal { }
    class Dog : Animal { }
    class Cat : Animal { }

    public static T GetEle<T>(T[] arr, int index)
    {
        return arr[index];
    }

    public static void SetEle<T>(T[] arr, int index, T value)
    {
        arr[index] = value;
    }

    [UnitTest]
    public void st_derived_to_base_array()
    {
        // Store derived-type objects into a base-type array.
        // Regression: is_assignable_from parameter order was reversed in StelemAnyRef,
        // causing this valid assignment to throw ArrayTypeMismatchException.
        var arr = new Animal[2];
        SetEle<Animal>(arr, 0, new Dog());
        SetEle<Animal>(arr, 1, new Cat());

        var x = GetEle<Animal>(arr, 0);
        Assert.IsTrue(x is Dog);
        var y = GetEle<Animal>(arr, 1);
        Assert.IsTrue(y is Cat);
    }

    [UnitTest]
    public void st_to_object_array()
    {
        // Store various derived types into object[].
        // With the bug, any non-object type stored via stelem.any would throw.
        var arr = new object[3];
        SetEle<object>(arr, 0, "hello");
        SetEle<object>(arr, 1, new Dog());
        SetEle<object>(arr, 2, 42);

        Assert.Equal("hello", (string)GetEle<object>(arr, 0));
        Assert.IsTrue(GetEle<object>(arr, 1) is Dog);
        Assert.Equal(42, (int)GetEle<object>(arr, 2));
    }

    [UnitTest]
    public void st_null_to_ref_array()
    {
        // Storing null should always succeed for reference type arrays.
        var arr = new Animal[1];
        SetEle<Animal>(arr, 0, null);
        Assert.Null(GetEle<Animal>(arr, 0));
    }

    [UnitTest]
    public void st_base_to_derived_array_throws()
    {
        // Array covariance: Dog[] can be assigned to Animal[].
        // But storing an Animal into the underlying Dog[] should throw ArrayTypeMismatchException.
        // Regression: with reversed parameter order, this would incorrectly succeed.
        Animal[] arr = new Dog[1];
        bool threw = false;
        try
        {
            SetEle<Animal>(arr, 0, new Animal());
        }
        catch (ArrayTypeMismatchException)
        {
            threw = true;
        }
        Assert.IsTrue(threw);
    }
}
