using System;

namespace Tests.CSharp
{
    public class TestCCtor
    {

        [IgnoreTest]
        class Common
        {
            public static int x = 0;
            public static int y = 0;
        }

        [IgnoreTest]
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

        [IgnoreTest]
        class B : A
        {

        }

        [UnitTest]
        public void CCotrRunWhenNewChildObject()
        {
            Assert.Equal(0, Common.x);
            new B();
            Assert.Equal(1, Common.x);
        }

        class StaticA
        {
            static StaticA()
            {
                Common.y = 1;
            }
        }

        class StaticB
        {
            public static int y;
            static StaticB()
            {
                y = 1;
            }
        }

        [UnitTest]
        public void ParentCCtorNotRunWhenAccessChildStaticField()
        {
            Assert.Equal(0, Common.y);
            Assert.Equal(1, StaticB.y);
            Assert.Equal(0, Common.y);
        }
    }
}