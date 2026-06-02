using test;
using System;


namespace Tests.Instruments.Ariths
{
    internal class TC_shl : GeneralTestCaseBase
    {
        [UnitTest]
        public void int_int()
        {
            int a = 1;
            Assert.Equal(4, a << 2);
        }

        [UnitTest]
        public void long_int()
        {
            long a = 1L;
            Assert.Equal(0x1_00000000L, a << 32);
        }
    }
}
