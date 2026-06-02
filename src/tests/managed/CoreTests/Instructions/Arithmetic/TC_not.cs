using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Ariths
{
    internal class TC_not : GeneralTestCaseBase
    {
        [UnitTest]
        public void int_1()
        {
            int b = -4;
            Assert.Equal(3, ~b);
        }

        [UnitTest]
        public void uint_1()
        {
            uint b = 0;
            Assert.Equal(0xFFFFFFFF, ~b);
        }

        [UnitTest]
        public void long_1()
        {
            long b = -0x4L;
            Assert.Equal(3L, ~b);
        }

        [UnitTest]
        public void ulong_1()
        {
            ulong b = 0;
            Assert.Equal(0xFFFFFFFF_FFFFFFFF, ~b);
        }

        [UnitTest]
        public void nint_1()
        {
            IntPtr b =  new IntPtr(-4);
            Assert.Equal(3, ~(int)b);
        }
    }
}
