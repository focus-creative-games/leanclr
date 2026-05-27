
public class TestIsInst
{
    [UnitTest]
    public void TestIsInstString()
    {
        object obj = "hello";
        if (obj is string str)
        {

        }
        else
        {
            Assert.Fail();
        }
    }


    [UnitTest]
    public void TestIsInstInt()
    {
        object obj = 123;
        if (obj is int i)
        {

        }
        else
        {
            Assert.Fail();
        }
    }
}
