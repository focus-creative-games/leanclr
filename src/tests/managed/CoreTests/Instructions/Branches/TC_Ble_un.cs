using test;


namespace Tests.Instruments.Branches
{
    internal class TC_Ble_un : GeneralTestCaseBase
    {
        [UnitTest]
        public static void s_blank()
        {
            uint a = 1;
            if (a <= 2u)
            {

            }
            a = 2;
        }

        [UnitTest]
        public static void ble_un_s_uint()
        {
            uint a = 32U;
            uint aa = 31U;
            if (aa <= a)
            {
                Assert.True(true);
            }
            else
            {
                Assert.Fail();
            }
        }

        [UnitTest]
        public static void ble_un_s_ulong()
        {
            ulong a = 32U;
            ulong aa = 31U;
            if (aa <= a)
            {
                Assert.True(true);
            }
            else
            {
                Assert.Fail();
            }
        }

        [UnitTest]
        public static void float_1()
        {
            float x = 3;
            float y = 3;
            if (x <= y)
            {
                x = default;
            }
            else
            {
                Assert.Fail();
            }
        }

        [UnitTest]
        public static void float_2()
        {
            float x = 3;
            float y = 4;
            if (x <= y)
            {
                x = default;
            }
            else
            {
                Assert.Fail();
            }
        }

        [UnitTest]
        public static void float_3()
        {
            float x = 3;
            float y = 2;
            if (x <= y)
            {
                Assert.Fail();
            }
            else
            {
                x = 0;
            }
        }

        [UnitTest]
        public static void s_double_1()
        {
            double x = 3;
            double y = 4;
            if (x <= y)
            {
                x = default;
            }
            else
            {
                Assert.Fail();
            }
        }

        [UnitTest]
        public static void s_double_2()
        {
            double x = 3;
            double y = 3;
            if (x <= y)
            {
                x = default;
            }
            else
            {
                Assert.Fail();
            }
        }

        [UnitTest]
        public static void s_double_3()
        {
            double x = 3;
            double y = 2;
            if (x <= y)
            {
                Assert.Fail();
            }
            else
            {
                x = 0;
            }
        }

        [UnitTest]
        public void CmpInfinite_float()
        {
            float x = 3;
            float y = float.NaN;
            if (x <= y)
            {
                Assert.Fail();
            }
            if (y <= x)
            {
                Assert.Fail();
            }
        }

        [UnitTest]
        public void CmpInfinite_double()
        {
            double x = 3;
            double y = double.NaN;
            if (x <= y)
            {
                Assert.Fail();
            }
            if (y <= x)
            {
                Assert.Fail();
            }
        }

        [UnitTest]
        public static void ble_un_int()
        {
            uint a = 32U;
            uint aa = 31U;
            if (aa <= a)
            {
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
                Assert.True(true);
            }
            else
            {
                Assert.Fail();
            }
        }

        [UnitTest]
        public static void ble_un_long()
        {
            ulong a = 32U;
            ulong aa = 31U;
            if (aa <= a)
            {
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
                Assert.True(true);
            }
            else
            {
                Assert.Fail();
            }
        }
    }
}
