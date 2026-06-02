using test;
using System;

namespace Tests.Instruments.Ariths
{
    internal class TC_and : GeneralTestCaseBase
    {
        [UnitTest]
        public void bool_true()
        {
            bool b = true;
            bool c = true;
            Assert.True(b && c);
        }

        [UnitTest]
        public void bool_false()
        {
            bool b = false;
            bool c = true;
            Assert.False(b && c);
        }

        [UnitTest]
        public void int_1()
        {
            int b = 0b11;
            int c = 0b10;
            Assert.Equal(0b10, b & c);
        }

        [UnitTest]
        public void long_1()
        {
            long b = 0x3_00000003L;
            long c = 0x2_00000002L;
            Assert.Equal(0x2_00000002L, b & c);
        }
    }
}
