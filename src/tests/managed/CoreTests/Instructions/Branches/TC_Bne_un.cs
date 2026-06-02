using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Branches
{
    internal class TC_Bne_un: GeneralTestCaseBase
    {
        [UnitTest]
        public static void s_blank()
        {
            uint a = 1;
            if (a != 1u)
            {

            }
            a = 2;
        }

        [UnitTest]
        public static void s_uint_1()
        {
            uint a = 1;
            uint b = 1;
            if (a != b)
            {
                Assert.Fail();
            }
        }

        [UnitTest]
        public static void s_uint_2()
        {
            uint a = 1;
            uint b = 2;
            if (a != b)
            {
            }
            else
            {
                Assert.Fail();
            }
        }

        [UnitTest]
        public static void s_ulong_1()
        {
            ulong a = 1L;
            ulong b = 1L;
            if (a != b)
            {
                Assert.Fail();
            }
        }

        [UnitTest]
        public static void s_ulong_2()
        {
            ulong a = 1L;
            ulong b = 2L;
            if (a != b)
            {
            }
            else
            {
                Assert.Fail();
            }
        }

        [UnitTest]
        public static void s_nint_1()
        {
            IntPtr a = new IntPtr(1);
            IntPtr b = new IntPtr(1);
            if (a != b)
            {
                Assert.Fail();
            }
        }

        [UnitTest]
        public static void s_nint_2()
        {
            IntPtr a = new IntPtr(1);
            IntPtr b = new IntPtr(2);
            if (a != b)
            {
                a = default;
            }
            else
            {
                Assert.Fail();
            }
        }

        [UnitTest]
        public static unsafe void s_ref_1()
        {
            int data = 0;
            int* x = &data;
            int* y = &data;
            if (x != y)
            {
                Assert.Fail();
            }
        }

        [UnitTest]
        public static unsafe void s_ref_2()
        {
            int data = 0;
            int* x = &data;
            int* y = null;
            if (x != y)
            {
                x = default;
            }
            else
            {
                Assert.Fail();
            }
        }

        [UnitTest]
        public static void s_object_1()
        {
            object a = "a";
            object b = a;
            if (a != b)
            {
                Assert.Fail();
            }
        }

        [UnitTest]
        public static void s_object_2()
        {
            object a = "a";
            object b = null;
            if (a != b)
            {
                a = null;
            }
            else
            {
                Assert.Fail();
            }
        }

        [UnitTest]
        public static void s_float_1()
        {
            float a = 1;
            float b = 1;
            if (a != b)
            {
                Assert.Fail();
            }
        }

        [UnitTest]
        public static void s_float_2()
        {
            float a = 1;
            float b = 2;
            if (a != b)
            {
            }
            else
            {
                Assert.Fail();
            }
        }


        [UnitTest]
        public static void s_double_1()
        {
            double a = 1;
            double b = 1;
            if (a != b)
            {
                Assert.Fail();
            }
        }

        [UnitTest]
        public static void s_double_2()
        {
            double a = 1;
            double b = 2;
            if (a != b)
            {
            }
            else
            {
                Assert.Fail();
            }
        }

        [UnitTest]
        public void CmpInfinite_float()
        {
            float x = 3;
            float y = float.NaN;
            if (x != y)
            {
                x = default;
            }
            else
            {
                Assert.Fail();
            }
            if (y != x)
            {
                x = default;
            }
            else
            {
                Assert.Fail();
            }
        }

        [UnitTest]
        public void CmpInfinite_double()
        {
            double x = 3;
            double y = double.NaN;
            if (x != y)
            {
                x = default;
            }
            else
            {
                Assert.Fail();
            }
            if (y != x)
            {
                x = default;
            }
            else
            {
                Assert.Fail();
            }
        }

        [UnitTest]
        public static void uint_1()
        {
            uint x = 1;
            uint y = 1;
            if (x != y)
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
                x = 0;
            }
        }

        [UnitTest]
        public static void uint_2()
        {
            uint x = 1;
            uint y = 2;
            if (x != y)
            {
                x = 1;
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

        [UnitTest]
        public static void ulong_1()
        {
            ulong x = 1;
            ulong y = 1;
            if (x != y)
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
                x = 0;
            }
        }

        [UnitTest]
        public static void ulong_2()
        {
            ulong x = 1;
            ulong y = 2;
            if (x != y)
            {
                x = 0;
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

        [UnitTest]
        public static void object_1()
        {
            object x = "a";
            object y = x;
            if (x != y)
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
                x = null;
            }
        }

        [UnitTest]
        public static void object_2()
        {
            object x = "a";
            object y = null;
            if (x != y)
            {
                x = null;
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

        [UnitTest]
        public static void nint_1()
        {
            IntPtr x = new IntPtr(1);
            IntPtr y = new IntPtr(2);
            if (x != y)
            {
                x = default;
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

        [UnitTest]
        public static void nint_2()
        {
            IntPtr x = new IntPtr(1);
            IntPtr y = new IntPtr(1);
            if (x != y)
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
                x = default;
            }
        }

        [UnitTest]
        public static unsafe void ref_1()
        {
            int data = 0;
            int* x = &data;
            int* y = null;
            if (x != y)
            {
                x = default;
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

        [UnitTest]
        public static unsafe void ref_2()
        {
            int data = 0;
            int* x = &data;
            int* y = &data;
            if (x != y)
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
                x = default;
            }
        }

        [UnitTest]
        public static void float_1()
        {
            float x = 1;
            float y = 1;
            if (x != y)
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
                x = 0;
            }
        }

        [UnitTest]
        public static void float_2()
        {
            float x = 1;
            float y = 2;
            if (x != y)
            {
                x = 1;
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

        [UnitTest]
        public static void double_1()
        {
            double x = 1;
            double y = 1;
            if (x != y)
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
                x = 0;
            }
        }

        [UnitTest]
        public static void double_2()
        {
            double x = 1;
            double y = 2;
            if (x != y)
            {
                x = 1;
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
