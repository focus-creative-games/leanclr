using System;


internal class TestLdvirftn
{
    interface IMath
    {
        int Add(int x);
    }

    class A : IMath
    {
        public int x;

        public virtual int Add(int a)
        {
            this.x += a;
            return this.x;
        }

        public virtual int Foo()
        {
            return 1;
        }
    }

    class B : A
    {
        public override int Add(int a)
        {
            this.x += a * 2;
            return this.x;
        }

        public override int Foo()
        {
            return 2;
        }
    }

    struct S : IMath
    {
        public int x;

        public int Add(int a)
        {
            this.x += a;
            return this.x;
        }
    }

    [UnitTest]
    [AotMethod(false)]
    public void load_1()
    {
        A o = new A();
        Func<int> a = o.Foo;
        Assert.Equal(1, a());
    }

    [UnitTest]
    [AotMethod(false)]
    public void load_2()
    {
        A o = new B();
        Func<int> a = o.Foo;
        Assert.Equal(2, a());
    }

    [UnitTest]
    [AotMethod(false)]
    public void load_3()
    {
        IMath o = new A() { x = 1 };
        Func<int, int> a = o.Add;
        Assert.Equal(3, a(2));
    }

    [UnitTest]
    [AotMethod(false)]
    public void load_4()
    {
        IMath o = new B() { x = 1 };
        Func<int, int> a = o.Add;
        Assert.Equal(5, a(2));
    }

    [UnitTest]
    [AotMethod(false)]
    public void load_5()
    {
        IMath o = new S() { x = 1 };
        Func<int, int> a = o.Add;
        Assert.Equal(3, a(2));
    }
}

