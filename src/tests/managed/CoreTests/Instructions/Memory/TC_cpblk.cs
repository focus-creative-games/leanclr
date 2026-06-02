using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using test;

namespace Tests.Instruments.Mems
{
    internal class TC_cpblk : GeneralTestCaseBase
    {
        [UnitTest]
        public unsafe void init_array_from_localloc()
        {
            byte* b = stackalloc byte[] { 1, 2, 3, 4, 5, 6 };
            Assert.Equal(1, b[0]);
        }
    }
}
