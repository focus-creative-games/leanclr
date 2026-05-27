
public class TestBgt
{
    [UnitTest]
    public void s_blank()
    {
        int a = 1;
        if (a > 0)
        {
            Assert.Equal(1, a);
            return;
        }
        a = 2;
        Assert.Fail("This test should fail because the value of a is 2, not 1.");
    }

    [UnitTest]
    public void s_int_1()
    {
        int a = 3;
        int b = 2;
        if (a > b)
        {
            a += 1;
            Assert.Equal(4, a);
        }
        else
        {
            b += 1;
            Assert.Equal(3, b);
        }
    }

    [UnitTest]
    public void s_int_3()
    {
        int a = 3;
        int b = 3;
        if (a > b)
        {
            a += 1;
            Assert.Equal(4, a);
        }
        else
        {
            b += 1;
            Assert.Equal(4, b);
        }
    }

    [UnitTest]
    public void s_long_1()
    {
        long a = 3L;
        long b = 2L;
        if (a > b)
        {
            ++a;
            Assert.Equal(4L, a);
        }
        else
        {
            ++b;
            Assert.Equal(3L, b);
        }
    }

    [UnitTest]
    public unsafe void s_ref_1()
    {
        int x = 1;
        int y = 2;
        int* a = &x;
        int* b = &y;
        if (a > b)
        {
            Assert.Equal(1, *a);
        }
        else
        {
            Assert.Equal(2, *b);
        }
    }
}

