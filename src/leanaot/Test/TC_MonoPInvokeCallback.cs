using System;
using System.Runtime.InteropServices;

class TC_MonoPInvokeCallback
{
    struct Foo
    {
        public int x;
        public int y;
    }

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    delegate void RefFooMonoPInvokeCallback(ref Foo foo);

    [MonoPInvokeCallback(typeof(RefFooMonoPInvokeCallback))]
    static void RefStructInArgument(ref Foo foo)
    {

    }

    [UnitTest]
    public void TestRefStructInArgument()
    {
        var foo = new Foo { x = 1, y = 2 };
        RefStructInArgument(ref foo);
        Assert.Equal(1, foo.x);
        Assert.Equal(2, foo.y);
    }
}