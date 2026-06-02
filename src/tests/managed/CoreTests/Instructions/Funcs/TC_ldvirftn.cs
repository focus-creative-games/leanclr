
using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using AOTDefs;

namespace Tests.Instruments.Funcs
{


    internal class TC_ldvirftn : GeneralTestCaseBase
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
        public void interp_1()
        {
            A o = new A();
            Func<int> a = o.Foo;
            Assert.Equal(1, a());
        }

        [UnitTest]
        public void interp_2()
        {
            A o = new B();
            Func<int> a = o.Foo;
            Assert.Equal(2, a());
        }

        [UnitTest]
        public void interp_3()
        {
            IMath o = new A() { x = 1 };
            Func<int, int> a = o.Add;
            Assert.Equal(3, a(2));
        }

        [UnitTest]
        public void interp_4()
        {
            IMath o = new B() { x = 1 };
            Func<int, int> a = o.Add;
            Assert.Equal(5, a(2));
        }

        [UnitTest]
        public void interp_5()
        {
            IMath o = new S() { x = 1 };
            Func<int, int> a = o.Add;
            Assert.Equal(3, a(2));
        }


        [UnitTest]
        public void aot_1()
        {
            var o = new Ldvirftn_A();
            Func<int> a = o.Foo;
            Assert.Equal(1, a());
        }

        [UnitTest]
        public void aot_2()
        {
            var o = new Ldvirftn_B();
            Func<int> a = o.Foo;
            Assert.Equal(2, a());
        }

        [UnitTest]
        public void aot_3()
        {
            Ldvirftn_IMath o = new Ldvirftn_A() { x = 1 };
            Func<int, int> a = o.Add;
            Assert.Equal(3, a(2));
        }

        [UnitTest]
        public void aot_4()
        {
            Ldvirftn_IMath o = new Ldvirftn_B() { x = 1 };
            Func<int, int> a = o.Add;
            Assert.Equal(5, a(2));
        }

        [UnitTest]
        public void aot_5()
        {
            Ldvirftn_IMath o = new Ldvirftn_S() { x = 1 };
            Func<int, int> a = o.Add;
            Assert.Equal(3, a(2));
        }
    }
}
