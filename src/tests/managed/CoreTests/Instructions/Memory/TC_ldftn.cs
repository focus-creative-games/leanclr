using System;
using test;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Mems
{

    class TestFun
    {
        public static int StaticFunc()
        {
            return 1;
        }

        public int InstanceFunc()
        {
            return 10;
        }

        public override int GetHashCode()
        {
            return 100;
        }
    }

    internal class TC_ldftn : GeneralTestCaseBase
    {
        [UnitTest]
        public static void static_1()
        {
            Func<int> func = TestFun.StaticFunc;
            Assert.Equal(1, func());
        }

        [UnitTest]
        public static void instance_2()
        {
            Func<int> func = new TestFun().InstanceFunc;
            Assert.Equal(10, func());
        }
    }
}
