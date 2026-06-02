using System;
using test;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

/// <summary>
/// 对于local变量，如果变量未使用，在Release编译中该变量会被裁剪掉，本文件的测试只有再Debug编译下才有效
/// </summary>
namespace Tests.Instruments.Mems
{
    internal class TC_stloc : GeneralTestCaseBase
    {
        [UnitTest]
        public static void stloc_0()
        {
            int a = 0;
            Assert.Equal(0, a);
        }

        [UnitTest]
        public static void stloc_1()
        {
            int b = 0;
            int a = 1;
            Assert.Equal(1, a);
        }

        [UnitTest]
        public static void stloc_2()
        {
            int c = 2;
            int b = 1;
            int a = 1;
            Assert.Equal(1, a);
        }

        [UnitTest]
        public static void stloc_3()
        {
            int d = 4;
            int c = 3;
            int b = 2;
            int a = 1;
            Assert.Equal(1, a);
        }

        [UnitTest]
        public static void stloc_s_4()
        {
            int e = 5;
            int d = 4;
            int c = 3;
            int b = 2;
            int a = 1;
            Assert.Equal(1, a);
        }

        [UnitTest]
        public static void stloc_s_255()
        {
            int a = 255;
            Assert.Equal(255, a);
        }

        [UnitTest]
        public static void stloc_6001()
        {
            int d = 60001;
            Assert.Equal(60001, d);
        }
    }
}
