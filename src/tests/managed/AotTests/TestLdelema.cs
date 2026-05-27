
using System;

internal class TestLdelema
{

    [UnitTest]
    public void ld_1()
    {
        var arr = new int[] { -1, 1 };
        ref int x = ref arr[0];
        Assert.Equal(-1, x);
        ref int y = ref arr[1];
        Assert.Equal(1, y);
    }

    [UnitTest]
    public void OutOfRange_lower()
    {
        Assert.ExpectException<IndexOutOfRangeException>(() =>
        {
            var arr = new int[] { -1, 1 };
            int index = -1;
            ref int s = ref arr[index];
        });
    }

    [UnitTest]
    public void OutOfRange_upper()
    {
        Assert.ExpectException<IndexOutOfRangeException>(() =>
        {
            var arr = new int[] { -1, 1 };
            ref int s = ref arr[2];
        });
    }

    [UnitTest]
    public void NullRef()
    {
        Assert.ExpectException<NullReferenceException>(() =>
        {
            int[] arr = null;
            ref int s = ref arr[0];
        });
    }

    [UnitTest]
    public void LoadCovariantElement()
    {
        Assert.ExpectException<System.ArrayTypeMismatchException>(() =>
        {
            var arr = new string[] { "a", "b" };
            object[] arr2 = arr;
            ref object s = ref arr2[0];
            s = new object();
        });
    }


    interface ITest
    {
        int Test();
    }

    static int CallArrayMemberFunction<T>(T[] arr) where T : ITest
    {
        return arr[0].Test();
    }

    struct TestStruct : ITest
    {
        public int x;

        public int Test()
        {
            return x;
        }
    }

    class TestClass : ITest
    {
        public int x;

        public int Test()
        {
            return x;
        }
    }

    [UnitTest]
    public void ReadOnlyPrefixStructArray()
    {
        TestStruct[] arr = new TestStruct[] { new TestStruct { x = 1 } };
        Assert.Equal(1, CallArrayMemberFunction(arr));
    }

    [UnitTest]
    public void ReadOnlyPrefixStructClass()
    {
        TestClass[] arr = new TestClass[] { new TestClass { x = 1 } };
        ITest[] arr2 = arr;
        Assert.Equal(1, CallArrayMemberFunction(arr2));
    }
}
