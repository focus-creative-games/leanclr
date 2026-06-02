using System;
using test;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Mems
{
    internal class TC_ldc : GeneralTestCaseBase
    {
        [UnitTest]
        public void ldc_i4_0()
        {
            int a = 0;
            Assert.Equal("0", a.ToString());
        }

        [UnitTest]
        public void ldc_i4_1()
        {
            int a = 1;
            Assert.Equal("1", a.ToString());
        }

        [UnitTest]
        public void ldc_i4_2()
        {
            int a = 2;
            Assert.Equal("2", a.ToString());
        }

        [UnitTest]
        public void ldc_i4_3()
        {
            int a = 3;
            Assert.Equal("3", a.ToString());
        }

        [UnitTest]
        public void ldc_i4_4()
        {
            int a = 4;
            Assert.Equal("4", a.ToString());
        }

        [UnitTest]
        public void ldc_i4_5()
        {
            int a = 5;
            Assert.Equal("5", a.ToString());
        }

        [UnitTest]
        public void ldc_i4_6()
        {
            int a = 6;
            Assert.Equal("6", a.ToString());
        }

        [UnitTest]
        public void ldc_i4_7()
        {
            int a = 7;
            Assert.Equal("7", a.ToString());
        }

        [UnitTest]
        public void ldc_i4_8()
        {
            int a = 8;
            Assert.Equal("8", a.ToString());
        }

        [UnitTest]
        public void ldc_i4_m1()
        {
            int a = -1;
            Assert.Equal("-1", a.ToString());
        }

        [UnitTest]
        public void ldc_i4_9()
        {
            int a = 9;
            Assert.Equal("9", a.ToString());
        }

        [UnitTest]
        public void ldc_i8_0xFFFFFFFFF()
        {
            long a = 123456789L;
            Assert.Equal("123456789", a.ToString());
        }

        [UnitTest]
        public void ldc_r4_999()
        {
            float a = 999f;
            int b = (int)a;
            Assert.Equal(999, b);
        }

        [UnitTest]
        public void ldc_r8_999()
        {
            double a = 999.0;
            int b = (int)a;
            Assert.Equal(999, b);
        }
    }
}
