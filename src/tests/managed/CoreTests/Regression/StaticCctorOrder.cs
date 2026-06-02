using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Bugs
{
    public class StaticCctorOrder : GeneralTestCaseBase
    {
        class Common
        {
            public static int x = 0;
        }

        class A
        {
            static A()
            {
                Common.x += 1;
            }
        }

        class B
        {
            static B()
            {
                Common.x += 100;
            }
        }

        [UnitTest]
        public void Test()
        {
            new B();
            Assert.Equal(100, Common.x);
        }
    }
}
