using System;


internal class TestLdlen
{

    [UnitTest]
    public void len_empty()
    {
        var arr = new int[0];
        Assert.Equal(0, arr.Length);
        Assert.Equal(0, arr.LongLength);
    }

    [UnitTest]
    public void len_size2()
    {
        var arr = new int[2];
        Assert.Equal(2, arr.Length);
        Assert.Equal(2, arr.LongLength);
    }

    [UnitTest]
    public void raise_null_exception_when_arr_is_null()
    {
        Assert.ExpectException<NullReferenceException>(() =>
        {
            int[] arr = null;
            int n = arr.Length;
        });
    }
}

