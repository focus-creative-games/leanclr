using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Cmps
{
    internal class TC_Clt_un : GeneralTestCaseBase
    {
        [UnitTest]
        public void uint_false()
        {
            uint b = 12;
            uint c = 3;
            Assert.False(b < c);
        }

        [UnitTest]
        public void uint_true()
        {
            uint b = 2;
            uint c = 3;
            Assert.True(b < c);
        }

        [UnitTest]
        public void ulong_true()
        {
            ulong b = 2;
            ulong c = 3;
            Assert.True(b < c);
        }

        [UnitTest]
        public void ulong_false()
        {
            ulong b = 4;
            ulong c = 3;
            Assert.False(b < c);
        }

        [UnitTest]
        public unsafe void unint_false()
        {
            uint* b = (uint*)12;
            uint* c = (uint*)3;
            Assert.False(b < c);
        }

        [UnitTest]
        public unsafe void unint_true()
        {
            uint* b = (uint*)2;
            uint* c = (uint*)3;
            Assert.True(b < c);
        }
    }
}
