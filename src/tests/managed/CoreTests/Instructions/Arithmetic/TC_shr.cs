using test;
using System;


namespace Tests.Instruments.Ariths
{
    internal class TC_shr : GeneralTestCaseBase
    {
        [UnitTest]
        public void int_1()
        {
            int a = 16;
            Assert.Equal(4, a >> 2);
        }

        [UnitTest]
        public void int_2()
        {
            int a = -16;
            Assert.Equal(-4, a >> 2);
        }

        [UnitTest]
        public void long_1()
        {
            long a = 16L;
            Assert.Equal(4L, a >> 2);
        }

        [UnitTest]
        public void long_2()
        {
            long a = -16L;
            Assert.Equal(-4L, a >> 2);
        }
    }
}
