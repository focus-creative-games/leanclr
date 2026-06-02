//using test;
//using System;
//using System.Collections.Generic;
//using System.Linq;
//using System.Text;
//using System.Threading;
//using System.Threading.Tasks;

//namespace Tests.Mics
//{
//    public class TC_Thread : GeneralTestCaseBase
//    {
//        [UnitTest]
//        public void CreateAndFreeThread()
//        {
//#if !UNITY_WEBGL
//            int a = 0;
//            var t = new Thread(() =>
//            {
//                a = 10;
//            });
//            t.Start();
//            t.Join();
//            Assert.Equal(10, a);
//#endif
//        }
//    }
//}
