using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Ariths
{
    internal class TC_div_un : TestCaseBase
    {
        [UnitTest]
        public static void uint_1()
        {
            uint a = 7;
            uint b = 3;
            Assert.Equal(2U, a / b);
        }

        [UnitTest]
        public static void ulong_1()
        {
            ulong a = 7;
            ulong b = 3;
            Assert.Equal(2UL, a / b);
        }

        //[UnitTest]
        //public static void uint_DivideByZeroException()
        //{
        //    uint a = 1;
        //    uint b = 0;
        //    Assert.ExpectException(new DivideByZeroException());
        //    uint c = a / b;
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public static void ulong_DivideByZeroException()
        //{
        //    ulong a = 1;
        //    ulong b = 0;
        //    Assert.ExpectException(new DivideByZeroException());
        //    ulong c = a / b;
        //    Assert.Fail();
        //}
    }
}
