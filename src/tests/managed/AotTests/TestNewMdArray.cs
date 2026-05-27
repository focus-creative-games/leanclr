using System;


internal class TestNewMdArray
{

    [UnitTest]
    public void new_int()
    {
        var arr = new int[2, 3];
        Assert.Equal(6, arr.Length);
    }

    [UnitTest]
    public void new_str()
    {
        var arr = new string[3, 4];
        Assert.Equal(12, arr.Length);
    }

    [UnitTest]
    public void raise_overflow_when_len_is_negative1()
    {
        Assert.ExpectException<OverflowException>(() =>
        {
            int a = -1;
            int b = 2;
            var arr = new int[a, b];
        });
    }

    [UnitTest]
    public void raise_overflow_when_len_is_negative2()
    {
        Assert.ExpectException<OverflowException>(() =>
        {
            int a = -1;
            int b = -2;
            var arr = new int[a, b];
        });
    }
}
