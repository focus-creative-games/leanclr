//using test;
//using System;
//using System.Collections.Generic;
//using System.Linq;
//using System.Text;
//using System.Threading.Tasks;

//namespace Tests.CSharp
//{
//    internal class TC_CrashReport : GeneralTestCaseBase
//    {
//        private unsafe void Sub1(int* p)
//        {
//            UnityEngine.Debug.Log("before crash");
//            *p = 1;
//            UnityEngine.Debug.Log("after crash");
//        }

//        private unsafe void Sub0(int* p)
//        {
//            Sub1(p);
//        }

//        [UnitTest]
//        public unsafe void Crash()
//        {
//            //Sub0(null);
//        }
//    }
//}
