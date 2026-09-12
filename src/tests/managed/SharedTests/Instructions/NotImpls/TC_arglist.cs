using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.NotImpls
{
    internal class TC_arglist : TestCaseBase
    {
        [UnitTest]
        public void empty_arglist_count()
        {
            Assert.Equal(0, TestInstructionOpcodes.CountArglist(__arglist()));
        }

        [UnitTest]
        public void mixed_arglist_count()
        {
            Assert.Equal(3, TestInstructionOpcodes.CountArglist(__arglist(1, "two", 3.0)));
        }
    }
}
