using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Bugs
{


    internal class Bug_Interface : GeneralTestCaseBase
    {
        interface IA
        {
            int Sum();
        }

        interface IB
        {
            int Sum();
        }

        class A : IA
        {
            public int Sum()
            {
                return 1;
            }
        }

        class B : A, IA
        {
            public new int Sum()
            {
                return 2;
            }
        }

        class C : IA, IB
        {
            public int Sum()
            {
                return 1;
            }
        }

        [UnitTest]
        public void test()
        {
            A a = new B();
            Assert.Equal(1, a.Sum());
            IA b = a;
            Assert.Equal(2, b.Sum());
        }

        [UnitTest]
        public void MultiInterfaceSameMethod()
        {
            var c = new C();
            Assert.Equal(1, c.Sum());
            IA a = c;
            Assert.Equal(1, a.Sum());
            IB b = c;
            Assert.Equal(1, b.Sum());
        }
    }
}
