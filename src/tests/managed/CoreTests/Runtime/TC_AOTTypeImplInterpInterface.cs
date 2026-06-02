
using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using AOTDefs;

namespace Tests.CSharp
{
    internal class TC_AOTTypeImplInterpInterface : GeneralTestCaseBase
    {
        public interface ISum
        {
            int Sum();
        }

        class Foo : BaseImplNone, ISum
        {
        }

        [UnitTest]
        public void TestAOTTypeImplInterpInterface()
        {
            ISum sum = new Foo();
            Assert.Equal(1, sum.Sum());
        }
    }
}
