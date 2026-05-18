using System;

namespace Tests.CSharp
{
    public class TestCCtor
    {

        class Common
        {
            public static int x = 0;
        }

        class A
        {
            public static int y;

            static A()
            {
                Common.x = 1;
                y = 2;
            }

            public void Run()
            {

            }

            public int Run2()
            {
                return y;
            }
        }

        class B : A
        {

        }

        [UnitTest]
        public void CCotrNotRunWhenNewChildObject()
        {
            Assert.Equal(0, Common.x);
            new B();
            Assert.Equal(0, Common.x);
        }

        [UnitTest]
        public void CCtorRunWhenCallMethodAccessStaticField()
        {
            Assert.Equal(0, Common.x);
            var b = new B();
            Assert.Equal(2, b.Run2());
            Assert.Equal(1, Common.x);
        }
    }
}