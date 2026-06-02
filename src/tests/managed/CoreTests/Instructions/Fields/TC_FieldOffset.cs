//using test;
//using System;
//using System.Collections.Generic;
//using System.Linq;
//using System.Reflection;
//using System.Runtime.InteropServices;
//using System.Text;
//using System.Threading.Tasks;

//namespace Tests.Instruments.Fields
//{
//    public class TC_FieldOffset : GeneralTestCaseBase
//    {
//        [StructLayout(LayoutKind.Sequential, Pack = 1, Size = 60)]
//        public struct ExplicitStruct
//        {
//            public int x;
//        }

//        public struct S1
//        {
//            public ExplicitStruct a;
//            public string b;


//            public static ExplicitStruct x;
//            public static string y;
//        }

//        [UnitTest]
//        public void TestFieldOffset1()
//        {
//            var t = typeof(S1);
//            FieldInfo f = t.GetField("a");
//            Assert.Equal(0, UnsafeUtility.GetFieldOffset(f));
//        }

//        [UnitTest]
//        public void TestFieldOffset2()
//        {
//            var t = typeof(S1);
//            FieldInfo f = t.GetField("b");
//            Assert.Equal(64, UnsafeUtility.GetFieldOffset(f));
//        }

//        [UnitTest]
//        public void TestStaticFieldOffset1()
//        {
//            var t = typeof(S1);
//            FieldInfo f = t.GetField("x");
//            Assert.Equal(-16, UnsafeUtility.GetFieldOffset(f));
//        }

//        [UnitTest]
//        public void TestStaticFieldOffset2()
//        {
//            var t = typeof(S1);
//            FieldInfo f = t.GetField("y");
//            Assert.Equal(48, UnsafeUtility.GetFieldOffset(f));
//        }
//    }
//}
