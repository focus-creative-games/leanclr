using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Mems
{
    internal class TC_ldnull: TestCaseBase
    {
        [UnitTest]
        public static void ldnull()
        {
            Assert.Null(null);
        }
    }
}
