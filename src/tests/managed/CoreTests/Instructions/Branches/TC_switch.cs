using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Branches
{
    internal class TC_switch : GeneralTestCaseBase
    {
        // 空测试，用于测试空switch跳转问题
        [UnitTest]
        public static void all_empty_case()
        {
            int a = 1;
            switch (a)
            {
                case 0:
                    break;
                case 1:
                    break;
                case 2:
                    break;
                default:
                    break;
            }
        }

        [UnitTest]
        public static void seq_1()
        {
            int a = 0;
            int b = 0;
            switch (a)
            {
                case 0:
                    b = 1;
                    break;
                case 1:
                    b = 2;
                    break;
                case 2:
                    b = 3;
                    break;
                default:
                    b = 4;
                    break;
            }
            Assert.Equal(1, b);
        }

        [UnitTest]
        public static void seq_2()
        {
            int a = 1;
            int b = 0;
            switch (a)
            {
                case 0:
                    b = 1;
                    break;
                case 1:
                    b = 2;
                    break;
                case 2:
                    b = 3;
                    break;
                default:
                    b = 4;
                    break;
            }
            Assert.Equal(2, b);
        }

        [UnitTest]
        public static void seq_3()
        {
            int a = 4;
            int b = 0;
            switch (a)
            {
                case 0:
                    b = 1;
                    break;
                case 1:
                    b = 2;
                    break;
                case 2:
                    b = 3;
                    break;
                default:
                    b = 4;
                    break;
            }
            Assert.Equal(4, b);
        }


        [UnitTest]
        public static void not_seq_1()
        {
            int a = 100;
            int b = 0;
            switch (a)
            {
                case 100:
                    b = 1;
                    break;
                case 10:
                    b = 2;
                    break;
                case 1000:
                    b = 3;
                    break;
                default:
                    b = 4;
                    break;
            }
            Assert.Equal(1, b);
        }


        [UnitTest]
        public static void not_seq_2()
        {
            int a = 10;
            int b = 0;
            switch (a)
            {
                case 100:
                    b = 1;
                    break;
                case 10:
                    b = 2;
                    break;
                case 1000:
                    b = 3;
                    break;
                default:
                    b = 4;
                    break;
            }
            Assert.Equal(2, b);
        }



        [UnitTest]
        public static void not_seq_3()
        {
            int a = 50;
            int b = 0;
            switch (a)
            {
                case 100:
                    b = 1;
                    break;
                case 10:
                    b = 2;
                    break;
                case 1000:
                    b = 3;
                    break;
                default:
                    b = 4;
                    break;
            }
            Assert.Equal(4, b);
        }

        // 空测试，用于测试空switch跳转问题
        [UnitTest]
        public static void s_switch()
        {
            int a = 1;
            switch (a)
            {
            }
        }
    }
}
