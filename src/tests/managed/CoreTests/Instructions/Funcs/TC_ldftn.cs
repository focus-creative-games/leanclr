
using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using AOTDefs;

namespace Tests.Instruments.Funcs
{

    internal class TC_ldftn : GeneralTestCaseBase
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
        public void interp_class_1()
        {
            var a = new A();
            Action f1 = a.Inc;
            f1();
            Assert.Equal(2, a.x);
            Func<int> f2 = a.Foo;
            Assert.Equal(2, f2());
        }

        [UnitTest]
        public void interp_struct_1()
        {
            var a = new A2() { x = 1};
            Action f1 = a.Inc;
            f1();
            Assert.Equal(1, a.x);
            Func<int> f2 = a.Foo;
            Assert.Equal(1, f2());
        }

        [UnitTest]
        public void interp_class_static()
        {
            Func<int, int> a = A.Sqr;
            Assert.Equal(4, a(2));
        }

        [UnitTest]
        public void interp_struct_static()
        {
            Func<int, int> a = A2.Sqr;
            Assert.Equal(4, a(2));
        }

        [UnitTest]
        public void aot_class_1()
        {
            var a = new Ldftn_A();
            Action f1 = a.Inc;
            f1();
            Assert.Equal(2, a.x);
            Func<int> f2 = a.Foo;
            Assert.Equal(2, f2());
        }

        [UnitTest]
        public void aot_struct_1()
        {
            var a = new Ldftn_A2() { x = 1 };
            Action f1 = a.Inc;
            f1();
            Assert.Equal(1, a.x);
            Func<int> f2 = a.Foo;
            Assert.Equal(1, f2());
        }

        [UnitTest]
        public void aot_class_static()
        {
            Func<int, int> a = Ldftn_A.Sqr;
            Assert.Equal(4, a(2));
        }

        [UnitTest]
        public void aot_struct_static()
        {
            Func<int, int> a = Ldftn_A2.Sqr;
            Assert.Equal(4, a(2));
        }
    }
}
