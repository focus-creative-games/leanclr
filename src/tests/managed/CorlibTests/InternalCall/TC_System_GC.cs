using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CorlibTests.InternalCall
{
    internal class TC_System_GC : TestCaseBase
    {
        [UnitTest]
        public void Collect1()
        {
            GC.Collect();
        }
    }
}
