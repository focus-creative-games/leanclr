using Tests.Fixtures;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Funcs
{
    internal class TC_calli : TestCaseBase
    {
        [UnitTest]
        public void static_function_pointer_call()
        {
            Assert.Equal(42, TestInstructionOpcodes.CalliAddOne(41));
        }
    }
}
