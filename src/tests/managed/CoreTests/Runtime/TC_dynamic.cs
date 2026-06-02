using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.CSharp.Dynamics
{

    class Foo
    {
        public int X { get; set; }

        public int y;

        public int Sum(int x, int y)
        {
            return x + y;
        }
    }

    internal class TC_dynamic : GeneralTestCaseBase
    {
        [UnitTest]
        public void test()
        {
            /*
            dynamic o = new Foo() { X = 1, y = 2};
            Assert.Equal(1, o.X);
            Assert.Equal(2, o.y);

            o.X = 10;
            Assert.Equal(10, o.X);
            o.y = 20;
            Assert.Equal(20, o.y);

            int c = o.Sum(1, 2);
            Assert.Equal(3, c);
            */
        }
    }
}
