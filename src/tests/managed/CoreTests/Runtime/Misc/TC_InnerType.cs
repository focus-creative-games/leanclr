
using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using AOTDefs;

namespace Tests.Mics
{
    public class TC_InnerType : GeneralTestCaseBase
    {
        [UnitTest]
        public void Test1()
        {
            Assert.Equal(3, InnerTypes.Sum(new InnerTypes.A { x = 1 }, new InnerTypes.B { x = 2 }));
        }

        //[UnitTest]
        public void Test2()
        {
            string s = InnerTypes.SumC1(new InnerTypes.C<InnerTypes.A, InnerTypes.B> { x = new InnerTypes.A { x = 1 }, y = new InnerTypes.B { x = 2 } });
            Assert.Equal("A:1,B:2", s);
        }
    }
}
