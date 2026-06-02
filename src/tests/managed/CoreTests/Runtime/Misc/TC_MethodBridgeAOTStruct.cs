
//using test;
//using System;
//using System.Collections.Generic;
//using System.Linq;
//using System.Numerics;
//using System.Text;
//using System.Threading.Tasks;
//using Tests.Bugs.Bug20220927;
//using Unity.Collections.LowLevel.Unsafe;

//namespace Tests.Mics
//{
//    public class TC_MethodBridgeAOTStruct : GeneralTestCaseBase
//    {

//        [UnitTest]
//        public void SizeofSeqP4()
//        {
//            Assert.Equal(56, UnsafeUtility.SizeOf<MBStructSeqP4>());
//        }

//        [UnitTest]
//        public void SizeofExpP4()
//        {
//            Assert.Equal(24, UnsafeUtility.SizeOf<MBStructExpP4>());
//        }


//        [UnitTest]
//        public void SizeofP4()
//        {
//            Assert.Equal(56, UnsafeUtility.SizeOf<MBStructP4>());
//        }


//        [UnitTest]
//        public void GetFieldSeqP3()
//        {
//            var a = new MBStructSeqP3() { a = 1, b = "hello" };
//            Assert.Equal(0x5, a.Compute((x) => x.b.Length));
//        }

//        [UnitTest]
//        public void GetFieldSeqP4()
//        {
//            var a = new MBStructSeqP3() { a = 1, b = "hello" };
//            Assert.Equal(0x5, a.Compute((x) => x.b.Length));
//        }

//        [UnitTest]
//        public void GetFieldExpP3()
//        {
//            var a = new MBStructExpP3() { a = 1, b = "hello" };
//            Assert.Equal(0x5, a.Compute((x) => x.b.Length));
//        }

//        [UnitTest]
//        public void GetFieldAutoP3()
//        {
//            var a = new MBStructP3() { a = 1, b = "hello" };
//            Assert.Equal(0x5, a.Compute((x) => x.b.Length));
//        }
//    }
//}
