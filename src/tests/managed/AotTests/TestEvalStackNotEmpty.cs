
public class TestEvalStackNotEmpty
{
    int test1(int a, int b)
    {
        int c = a > 5 ? a + 1 : b + 2;


        return c;
    }

    [UnitTest]
    public void Test2()
    {
        Assert.Equal(test1(6, 10), 7);
    }
}

