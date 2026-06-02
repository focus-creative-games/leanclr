using System;
using test;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Mems
{
    internal class TC_ldloca: GeneralTestCaseBase
    {
        [UnitTest]
        public static void loc_s()
        {
            int a = 8;
            ref var r = ref a;
            Assert.Equal(8, r);
        }

        [UnitTest]
        public static void loc_255()
        {
            int a255 = 255;
            ref var r = ref a255;
            Assert.Equal(255, r);
        }
    }
}
