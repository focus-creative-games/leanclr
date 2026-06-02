using System;
using test;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Diagnostics;

namespace Tests.Instruments.Branches
{
    class TC_Br : GeneralTestCaseBase
    {

        [UnitTest]
        public static void s_blank()
        {
            int a = 1;
            goto GotoLabel;
        GotoLabel:
            a = 2;
            Assert.Equal(2, a);
        }

        [UnitTest]
        public static void s_forward()
        {
            int a = 1;
            goto GotoLabel;
            Assert.Fail();
        GotoLabel:
            return;
        }

        [UnitTest]
        public static void s_back()
        {
            int a = 1;
            goto GotoLabel;
            Assert.Fail();
        LabelBack:
            return;
        GotoLabel:
            goto LabelBack;
            Assert.Fail();
            return;
        }

        [UnitTest]
        public static void far_forward()
        {
            int a = 1;
            goto GotoLabel;
            Assert.Fail();
            a = a + 1;
            var b = 245;
            var c = 231;
            var d = 897;
            d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c;
            c = b + c; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1;
            b = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c;
            d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c;
            c = b + c; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1;
            b = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c;
            d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c;
            c = b + c; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1;
            b = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c;
            d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c;
            c = b + c; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1;
            b = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c;
            d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c;
            c = b + c; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1;
            b = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c;
            d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c;
            c = b + c; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1;
            b = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c;
            d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c;
            c = b + c; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1;
            b = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c;
            Assert.Fail();
        GotoLabel:
            return;
        }

        [UnitTest]
        public static void far_back()
        {
            int a = 1;
            goto GotoLabel;
            Assert.Fail();
        Labelback:
            return;
            a = a + 1;
            var b = 245;
            var c = 231;
            var d = 897;
            d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c;
            c = b + c; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1;
            b = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c;
            d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c;
            c = b + c; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1;
            b = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c;
            d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c;
            c = b + c; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1;
            b = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c;
            d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c;
            c = b + c; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1;
            b = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c;
            d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c;
            c = b + c; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1;
            b = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c;
            d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c;
            c = b + c; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1;
            b = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c;
            d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c;
            c = b + c; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1; b = b + 1;
            b = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c; d = b + c;
            Assert.Fail();
        GotoLabel:
            goto Labelback;
        }
    }

}
