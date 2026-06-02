using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Ariths
{
    internal class TC_xor : GeneralTestCaseBase
    {
        [UnitTest]
        public void int_1()
        {
            int b = 0x1;
            int c = 0x3;
            Assert.Equal(0x2, b ^ c);
        }

        [UnitTest]
        public void long_1()
        {
            long b = 0x1L;
            long c = 0x3L;
            Assert.Equal(0x2L, b ^ c);
        }

    }
}
