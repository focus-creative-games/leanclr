
public class TestBrTrueFalse
{
    [UnitTest]
    public void BrFalse()
    {
        int a = 1;
        bool b = a == 2;
        if (b)
        {
            Assert.Fail();
        }
        Assert.Equal(1, a);
    }

    [UnitTest]
    public void BrTrue()
    {
        var a = true;
        if (!a)
        {
            Assert.Fail();
        }
        else
        {
            Assert.True(a);
        }
    }
}
