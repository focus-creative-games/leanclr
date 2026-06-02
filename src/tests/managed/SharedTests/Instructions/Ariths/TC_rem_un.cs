using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Ariths
{
    internal class TC_un : GeneralTestCaseBase
    {
        [UnitTest]
        public static void un_uint()
        {
            uint a = 9;
            uint b = 7;
            Assert.Equal(2, a % b);
        }

        //[UnitTest]
        //public static void un_uint_DivideByZeroException()
        //{
        //    uint a = 9;
        //    uint b = 0;
        //    Assert.ExpectException(new DivideByZeroException());
        //    ulong c = a % b;
        //    Assert.Fail();
        //}

        [UnitTest]
        public static void un_ulong()
        {
            ulong a = 9UL;
            ulong b = 7UL;
            Assert.Equal(2UL, a % b);
        }

        //[UnitTest]
        //public static void un_ulong_DivideByZeroException()
        //{
        //    ulong a = 9UL;
        //    ulong b = 0UL;
        //    Assert.ExpectException(new DivideByZeroException());
        //    ulong c = a % b;
        //    Assert.Fail();
        //}
    }

}
