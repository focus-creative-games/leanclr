
using System;

public class TestCallVir
{
    interface IFoo
    {
        int Sum(int a, int b);

        int IncInterp(int a);
    }

    class Foo1 : IFoo
    {
        public int x;

        public int Sum(int a, int b)
        {
            return a + b + x;
        }

        [AotMethod(false)]
        public int IncInterp(int a)
        {
            return a + x;
        }

        public virtual int Sum2(int a, int b)
        {
            return a + b + x;
        }

        [AotMethod(false)]
        public virtual int AddInterp(int a)
        {
            return a + x;
        }
    }

    class Foo2 : Foo1
    {
        public int y;

        public override int Sum2(int a, int b)
        {
            return a + b + y;
        }

        [AotMethod(false)]
        public override int AddInterp(int a)
        {
            return a + y;
        }
    }

    struct StructFoo : IFoo
    {
        public int x;

        public int Sum(int a, int b)
        {
            return a + b + x;
        }

        [AotMethod(false)]
        public int IncInterp(int a)
        {
            return a + x;
        }

        public override string ToString()
        {
            return x.ToString();
        }
    }

    [UnitTest]
    public void CallClassVirAot()
    {
        Foo1 foo = new Foo1() { x = 10 };
        Assert.Equal(13, foo.Sum(1, 2));
    }

    [UnitTest]
    public void CallClassVirInterp()
    {
        Foo1 foo = new Foo1() { x = 10 };
        Assert.Equal(11, foo.IncInterp(1));
    }

    [UnitTest]
    public void CallClassOverrideVirAot()
    {
        Foo1 foo = new Foo2() { y = 100 };
        Assert.Equal(103, foo.Sum2(1, 2));
    }

    [UnitTest]
    public void CallClassOverrideVirInterp()
    {
        Foo1 foo = new Foo2() { y = 100 };
        Assert.Equal(101, foo.AddInterp(1));
    }

    [UnitTest]
    public void CallInterfaceVirAot()
    {
        IFoo foo = new Foo1() { x = 10 };
        Assert.Equal(13, foo.Sum(1, 2));
    }

    [UnitTest]
    public void CallInterfaceVirInterp()
    {
        IFoo foo = new Foo1() { x = 10 };
        Assert.Equal(11, foo.IncInterp(1));
    }

    [UnitTest]
    public void CallStructInterfaceVirAot()
    {
        IFoo foo = new StructFoo() { x = 10 };
        Assert.Equal(13, foo.Sum(1, 2));
    }

    [UnitTest]
    public void CallStructInterfaceVirInterp()
    {
        IFoo foo = new StructFoo() { x = 10 };
        Assert.Equal(11, foo.IncInterp(1));
    }

    [UnitTest]
    [AotMethod(false)]
    public void CallStructVirAotFromInterpreter()
    {
        IFoo foo = new StructFoo() { x = 10 };
        Assert.Equal(13, foo.Sum(1, 2));
    }

    [UnitTest]
    public void struct_contraint_tostring()
    {
        var a = new StructFoo() { x = 1 };
        Assert.Equal("1", a.ToString());
    }


    struct StructFoo2 : IDisposable
    {
        public int x;
        void IDisposable.Dispose()
        {
            x += 1;
        }
    }

    [UnitTest]
    public void struct_contraint_dispose()
    {
        var a = new StructFoo2() { x = 1 };
        IDisposable b = a;
        b.Dispose();
        var c = (StructFoo2)b;
        Assert.Equal(2, c.x);
    }
}

