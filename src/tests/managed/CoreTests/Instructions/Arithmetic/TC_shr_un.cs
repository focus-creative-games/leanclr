using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Ariths
{
    internal class TC_shr_un: GeneralTestCaseBase
    {
        [UnitTest]
        public void uint_1()
        {
            uint a = 16u;
            Assert.Equal(4u, a >> 2);
        }

        [UnitTest]
        public void uint_2()
        {
            uint a = 0x7FFFFFFFu;
            Assert.Equal(0x3FFFFFFFu, a >> 1);
        }

        [UnitTest]
        public void ulong_1()
        {
            ulong a = 16UL;
            Assert.Equal(4UL, a >> 2);
        }

        [UnitTest]
        public void ulong_2()
        {
            ulong a = 0x7FFFFFFF_FFFFFFFFUL;
            Assert.Equal(0x3FFFFFFF_FFFFFFFFUL, a >> 1);
        }
    }
}
