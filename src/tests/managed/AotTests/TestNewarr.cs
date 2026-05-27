using System;


internal class TestNewarr
{

    [UnitTest]
    public void new_int()
    {
        var arr = new int[2];
        Assert.Equal(2, arr.Length);
    }

    [UnitTest]
    public void new_str()
    {
        var arr = new string[2];
        Assert.Equal(2, arr.Length);
    }

    [UnitTest]
    public void raise_overflow_when_len_is_negative()
    {
        Assert.ExpectException<OverflowException>(() =>
        {
            int a = -1;
            var arr = new int[a];
        });
    }
}
