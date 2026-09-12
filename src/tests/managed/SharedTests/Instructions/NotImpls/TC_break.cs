using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.NotImpls
{
    internal class TC_break : TestCaseBase
    {
        [UnitTest]
        public void break_is_noop_without_debugger()
        {
            Assert.Equal(42, TestInstructionOpcodes.BreakPassThrough(41));
        }
    }
}
