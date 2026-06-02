using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Ariths
{
    internal class TC_or : GeneralTestCaseBase
    {
        [UnitTest]
        public void bool_1()
        {
            bool b = false;
            bool c = true;
            Assert.True(b || c);
        }

        [UnitTest]
        public void bool_2()
        {
            bool b = false;
            bool c = false;
            Assert.False(b || c);
        }

        [UnitTest]
        public void int_1()
        {
            int b = 0x1;
            int c = 0x2;
            Assert.Equal(0x3, b | c);
        }

        [UnitTest]
        public void long_1()
        {
            long b = 0x7000000000000003L;
            long c = 0x7000000000000006L;
            Assert.Equal(0x7000000000000007L, b | c);
        }
    }
}
