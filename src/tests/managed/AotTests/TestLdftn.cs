using System;


internal class TestLdftn
{
    class A
    {
        public int x = 1;

        public void Inc()
        {
            ++x;
        }

        public int Foo()
        {
            return x;
        }

        public static int Sqr(int a)
        {
            return a * a;
        }
    }

    struct A2
    {
        public int x;

        public void Inc()
        {
            ++x;
        }

        public int Foo()
        {
            return x;
        }

        public static int Sqr(int a)
        {
            return a * a;
        }
    }

    [UnitTest]
    [AotMethod(false)]
    public void class_1()
    {
        var a = new A();
        Action f1 = a.Inc;
        f1();
        Assert.Equal(2, a.x);
        Func<int> f2 = a.Foo;
        Assert.Equal(2, f2());
    }

    [UnitTest]
    [AotMethod(false)]
    public void struct_1()
    {
        var a = new A2() { x = 1 };
        Action f1 = a.Inc;
        f1();
        Assert.Equal(1, a.x);
        Func<int> f2 = a.Foo;
        Assert.Equal(1, f2());
    }

    [UnitTest]
    [AotMethod(false)]
    public void class_static()
    {
        Func<int, int> a = A.Sqr;
        Assert.Equal(4, a(2));
    }

    [UnitTest]
    [AotMethod(false)]
    public void struct_static()
    {
        Func<int, int> a = A2.Sqr;
        Assert.Equal(4, a(2));
    }
}
