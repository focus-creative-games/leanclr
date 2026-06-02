using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Ariths
{
    internal class TC_rem : TestCaseBase
    {
        [UnitTest]
        public static void int_1()
        {
            int a = 10;
            int b = 3;
            Assert.Equal(1, a % b);
        }

        [UnitTest]
        public static void int_2()
        {
            int a = -10;
            int b = 3;
            Assert.Equal(-1, a % b);
        }

        //[UnitTest]
        //public static void int_DivideByZeroException()
        //{
        //    int a = 9;
        //    int b = 0;
        //    Assert.ExpectException(new DivideByZeroException());
        //    var c = a % b;
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public static void int_OverflowException()
        //{
        //    int a = int.MinValue;
        //    int b = -1;
        //    Assert.ExpectException(new OverflowException());
        //    var c = a % b;
        //    Assert.Fail();
        //}

        [UnitTest]
        public static void long_1()
        {
            long a = 10;
            long b = 3;
            Assert.Equal(1L, a % b);
        }

        [UnitTest]
        public static void long_2()
        {
            long a = -10;
            long b = 3;
            Assert.Equal(-1L,  a % b);
        }

        //[UnitTest]
        //public static void long_DivideByZeroException()
        //{
        //    long a = 9;
        //    long b = 0;
        //    Assert.ExpectException(new DivideByZeroException());
        //    var c = a % b;
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public static void long_OverflowException()
        //{
        //    long a = long.MinValue;
        //    long b = -1;
        //    Assert.ExpectException(new OverflowException());
        //    var c = a % b;
        //    Assert.Fail();
        //}

        [UnitTest]
        public static void float_1()
        {
            float a = 9f;
            float b = 7f;
            float c = a % b;
            Assert.Equal(2f, c);
        }

        [UnitTest]
        public static void float_2()
        {
            float a = -9f;
            float b = 7f;
            Assert.Equal(-2f, a % b);
        }

        [UnitTest]
        public static void float_NaN()
        {
            float a = 9f;
            float b = 0f;
            Assert.True(float.IsNaN(a % b));
        }

        [UnitTest]
        public static void float_0()
        {
            float a = float.MinValue;
            float b = -1;
            var c = a % b;
            Assert.Equal(0f, c);
        }

        [UnitTest]
        public static void double_1()
        {
            double a = 9.0;
            double b = 7.0;
            Assert.Equal(2.0, a % b);
        }

        [UnitTest]
        public static void double_2()
        {
            double a = -9.0;
            double b = 7.0;
            Assert.Equal(-2.0, a % b);
        }


        [UnitTest]
        public static void double_NaN()
        {
            double a = 9.0;
            double b = 0.0;
            Assert.True(double.IsNaN(a % b));
        }

        [UnitTest]
        public static void double_OverflowException()
        {
            double a = double.MinValue;
            double b = -1;
            var c = a % b;
            Assert.Equal(0d, c);
        }
    }
}
