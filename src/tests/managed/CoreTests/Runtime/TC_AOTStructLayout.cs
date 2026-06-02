
//using test;
//using System;
//using System.Collections.Generic;
//using System.Linq;
//using System.Runtime.InteropServices;
//using System.Text;
//using System.Threading.Tasks;
//using Unity.Collections.LowLevel.Unsafe;

//namespace Tests.CSharp
//{

//    internal class TC_AOTStructLayout : GeneralTestCaseBase
//    {
//        [UnitTest]
//        public void SizeofSeqP0()
//        {
//            Assert.Equal(16, UnsafeUtility.SizeOf<StructSeqP0>());
//        }

//        [UnitTest]
//        public void SizeofSeqP1()
//        {
//            Assert.Equal(9, UnsafeUtility.SizeOf<StructSeqP1>());
//        }

//        [UnitTest]
//        public void SizeofSeqP2()
//        {
//            Assert.Equal(10, UnsafeUtility.SizeOf<StructSeqP2>());
//        }

//        [UnitTest]
//        public void SizeofSeqP3()
//        {
//            Assert.Equal(16, UnsafeUtility.SizeOf<StructSeqP3>());
//        }

//        [UnitTest]
//        public void SizeofExpP0()
//        {
//            Assert.Equal(16, UnsafeUtility.SizeOf<StructExpP0>());
//        }

//        [UnitTest]
//        public void SizeofExpP1()
//        {
//            Assert.Equal(12, UnsafeUtility.SizeOf<StructExpP1>());
//        }

//        [UnitTest]
//        public void SizeofExpP2()
//        {
//            Assert.Equal(12, UnsafeUtility.SizeOf<StructExpP2>());
//        }

//        [UnitTest]
//        public void SizeofExpP3()
//        {
//            Assert.Equal(16, UnsafeUtility.SizeOf<StructExpP3>());
//        }

//        [UnitTest]
//        public void SizeofP0()
//        {
//            Assert.Equal(16, UnsafeUtility.SizeOf<StructP0>());
//        }

//        [UnitTest]
//        public void SizeofP1()
//        {
//#if UNITY_EDITOR
//            Assert.Equal(9, UnsafeUtility.SizeOf<StructP1>());
//#else
//            Assert.Equal(16, UnsafeUtility.SizeOf<StructP1>());
//#endif
//        }

//        [UnitTest]
//        public void SizeofP2()
//        {
//#if UNITY_EDITOR
//            Assert.Equal(10, UnsafeUtility.SizeOf<StructP2>());
//#else
//            Assert.Equal(16, UnsafeUtility.SizeOf<StructP2>());
//#endif
//        }

//        [UnitTest]
//        public void SizeofP3()
//        {
//            Assert.Equal(16, UnsafeUtility.SizeOf<StructP3>());
//        }

//        [UnitTest]
//        public void GetFieldSeqP0()
//        {
//            var a = new StructSeqP0() { a = 1, b = 0x11223344 };
//            Assert.Equal(0x11223344, a.Compute((x) => x.b));
//        }

//        [UnitTest]
//        public void GetFieldSeqP1()
//        {
//            var a = new StructSeqP1() { a = 1, b = 0x11223344 };
//            Assert.Equal(0x11223344, a.Compute((x) => x.b));
//        }

//        [UnitTest]
//        public void GetFieldSeqP2()
//        {
//            var a = new StructSeqP2() { a = 1, b = 0x11223344 };
//            Assert.Equal(0x11223344, a.Compute((x) => x.b));
//        }

//        [UnitTest]
//        public void GetFieldSeqP3()
//        {
//            var a = new StructSeqP3() { a = 1, b = "hello" };
//            Assert.Equal(5, a.Compute((x) => x.b.Length));
//        }

//        [UnitTest]
//        public void GetFieldExpP0()
//        {
//            var a = new StructExpP0() { a = 1, b = 0x11223344 };
//            Assert.Equal(0x11223344, a.Compute((x) => x.b));
//        }

//        [UnitTest]
//        public void GetFieldExpP1()
//        {
//            var a = new StructExpP1() { a = 1, b = 0x11223344 };
//            Assert.Equal(0x11223344, a.Compute((x) => x.b));
//        }

//        [UnitTest]
//        public void GetFieldExpP2()
//        {
//            var a = new StructExpP2() { a = 1, b = 0x11223344 };
//            Assert.Equal(0x11223344, a.Compute((x) => x.b));
//        }

//        [UnitTest]
//        public void GetFieldExpP3()
//        {
//            var a = new StructExpP3() { a = 1, b = "hello" };
//            Assert.Equal(5, a.Compute((x) => x.b.Length));
//        }

//        [UnitTest]
//        public void GetFieldP0()
//        {
//            var a = new StructP0() { a = 1, b = 0x11223344 };
//            Assert.Equal(0x11223344, a.Compute((x) => x.b));
//        }

//        [UnitTest]
//        public void GetFieldP1()
//        {
//            var a = new StructP1() { a = 1, b = 0x11223344 };
//            Assert.Equal(0x11223344, a.Compute((x) => x.b));
//        }

//        [UnitTest]
//        public void GetFieldP2()
//        {
//            var a = new StructP2() { a = 1, b = 0x11223344 };
//            Assert.Equal(0x11223344, a.Compute((x) => x.b));
//        }

//        [UnitTest]
//        public void GetFieldP3()
//        {
//            var a = new StructP3() { a = 1, b = "hello" };
//            Assert.Equal(5, a.Compute((x) => x.b.Length));
//        }
//    }
//}
