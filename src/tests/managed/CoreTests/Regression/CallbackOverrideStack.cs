//
//using test;
//using System;
//using System.Collections.Generic;
//using System.Linq;
//using System.Reflection;
//using System.Text;
//using System.Threading.Tasks;

//namespace Tests.Bugs
//{
//    internal class CallbackOverrideStack : GeneralTestCaseBase
//    {
//        static class ForTest101
//        {
//            public static int x;
//            public static int y;

//            static ForTest101()
//            {
//                x = 1;
//                y = 2;
//                x = y + x;
//            }

//            public static int Sum(int a)
//            {
//                return a;
//            }
//        }

//        [UnitTest]
//        public void Test()
//        {
//            int v = CallDelegateFuncs.Invoke(ForTest101.Sum, 1000);
//            Assert.Equal(1000, v);
//        }

//    }
//}
