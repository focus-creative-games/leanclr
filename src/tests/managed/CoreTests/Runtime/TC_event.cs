using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.CSharp
{
    public class TC_event : GeneralTestCaseBase
    {
        class TestEvent1
        {
            public event Func<int, int> func;

            public int Run(int x)
            {
                return func(x);
            }

            public int Inc(int x)
            {
                return x + 1;
            }

            public int Dec(int x)
            {
                return x - 1;
            }

            public void AddDec()
            {
                func += Dec;
            }
        }

        [UnitTest]
        public void Test1()
        {
            var e = new TestEvent1();
            e.func += e.Inc;
            int result = e.Run(1);
            Assert.Equal(result, 2);
        }

        [UnitTest]
        public void Test2()
        {
            var e = new TestEvent1();
            e.AddDec();
            int result = e.Run(1);
            Assert.Equal(result, 0);
        }
    }
}
