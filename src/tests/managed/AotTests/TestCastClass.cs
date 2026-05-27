
public class TestCastClass
{
    [UnitTest]
    public void TestCastString()
    {
        object obj = "hello";
        string str = (string)obj;
        Assert.Equal(5, str.Length);
    }

    [UnitTest]
    public void TestCastInt()
    {
        object obj = 123;
        int i = (int)obj;
        Assert.Equal(123, i);
    }
}

