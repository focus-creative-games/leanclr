
public class TestCall
{
    class ClassFoo
    {
        public int x;
        public int Sum(int a, int b)
        {
            return a + b + x;
        }

        [AotMethod(false)]
        public int SumInterp(int a, int b)
        {
            return a + b + x;
        }
    }

    struct StructFoo
    {
        public int x;

        public int Sum(int a, int b)
        {
            return a + b + x;
        }

        [AotMethod(false)]
        public int SumInterp(int a, int b)
        {
            return a + b + x;
        }
    }

    [UnitTest]
    public void CallAot()
    {
        var o = new ClassFoo() { x = 10 };
        Assert.Equal(13, o.Sum(1, 2));
    }

    [UnitTest]
    public void CallInterp()
    {
        var o = new ClassFoo() { x = 10 };
        Assert.Equal(13, o.SumInterp(1, 2));
    }

    [UnitTest]
    public void CallStructAot()
    {
        var o = new StructFoo() { x = 10 };
        Assert.Equal(13, o.Sum(1, 2));
    }

    [UnitTest]
    public void CallStructInterp()
    {
        var o = new StructFoo() { x = 10 };
        Assert.Equal(13, o.SumInterp(1, 2));
    }
}

