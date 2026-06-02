using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Delegate : TestCaseBase
    {
        class A
        {
            public int x;
            public virtual int Foo()
            {
                return 1;
            }

            public void Add()
            {
                x += 1;
            }

            public void Add2()
            {
                x += 2;
            }
        }

        class B : A
        {
            public override int Foo()
            {
                return 2;
            }
        }

        [UnitTest]
        public void GetVirtualMethod()
        {
            A a = new B();
            Func<int> f = a.Foo;
            var m = f.Method;
            Assert.Equal(m.Name, "Foo");
            Assert.Equal(typeof(B), m.DeclaringType);
        }

        [UnitTest]
        public void Combine()
        {
            var b1 = new A();
            Action add1 = b1.Add;
            Action add2 = b1.Add2;
            Action combined = (Action)Delegate.Combine(add1, add2);
            combined();
            Assert.Equal(3, b1.x);
        }
    }
}
