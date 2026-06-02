using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Branches
{
    internal class TC_brtrue : GeneralTestCaseBase
    {
        [UnitTest]
        public static void s_blank()
        {
            int a = 2;
            bool b = a == 1;
            if (!b)
            {
            }
            a = 2;
            Assert.Equal(2, a);
        }

        [UnitTest]
        public static void s_true()
        {
            var a = true;
            if (a)
            {
                a = false;
            }
            else
            {
                Assert.Fail();
            }
        }

        [UnitTest]
        public static void true_long()
        {
            var a = 123L << 32;
            if (a != 0)
            {
                Assert.True(true);
            }
            else
            {
                Assert.Fail();
            }
        }

        [UnitTest]
        public static void s_false()
        {
            var a = false;
            if (a)
            {
                Assert.Fail();
            }
            else
            {
                a = true;
            }
        }

        [UnitTest]
        public static void far_1()
        {
            var a = false;
            if (a)
            {
                Assert.Fail();
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
            }
            else
            {
                a = true;
            }
        }

        [UnitTest]
        public static void far_2()
        {
            var a = true;
            if (a)
            {
                a = false;
            }
            else
            {
                Assert.Fail();
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
            }
        }
    }
}
