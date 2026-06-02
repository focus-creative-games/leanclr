//using System;
//using System.Reflection;

//using test;

//namespace Tests.Instruments.Fields
//{
//    public class TC_ExplicitLayout : GeneralTestCaseBase
//    {
//        [UnitTest]
//        public void SizeOf()
//        {
//            var size = UnsafeUtility.SizeOf(typeof(ExplicitStruct));
//            Assert.Equal(8, size);
//        }
        
//        [UnitTest]
//        public void SizeOfAOT()
//        {
//            var size = UnsafeUtility.SizeOf(typeof(AOTExplicitStruct));
//            Assert.Equal(8, size);
//        }

//        [UnitTest]
//        public void FieldOffset()
//        {
//            foreach (var field in typeof(ExplicitStruct).GetFields(BindingFlags.Public|BindingFlags.Instance))
//            {
//                int offset = UnsafeUtility.GetFieldOffset(field);
//                Assert.Equal(0, offset);
//            }
//        }

//        [UnitTest]
//        public void FieldOffsetAOT()
//        {
//            foreach (var field in typeof(AOTExplicitStruct).GetFields(BindingFlags.Public|BindingFlags.Instance))
//            {
//                int offset = UnsafeUtility.GetFieldOffset(field);
//                Assert.Equal(0, offset);
//            }
//        }

//        [UnitTest]
//        public unsafe void AccessField()
//        {
//            var a = new ExplicitStruct();
//            a.x = 1;
//            int* ptrX = (int*)&a;
//            Assert.Equal(1, *ptrX);
//            Assert.Equal(1, a.x);
//        }

//        [UnitTest]
//        public unsafe void AOTAccessField()
//        {
//            var a = new AOTExplicitStruct();
//            a.x = 1;
//            int* ptrX = (int*)&a;
//            Assert.Equal(1, *ptrX);
//            Assert.Equal(1, a.x);
//        }

//        class EnclosingClass
//        {
//            public long a;
//            public ExplicitStruct b;
//            public long c;
//        }
        
//        class EnclosingClassAOT
//        {
//            public long a;
//            public AOTExplicitStruct b;
//            public long c;
//        }

//        [UnitTest]
//        public void AccessField2()
//        {
//            var c = new EnclosingClass();
//            c.b.y = 0x1122334455667788;
//            Assert.Equal(0, c.a);
//            Assert.Equal(0x1122334455667788, c.b.y);
//            Assert.Equal(0, c.c);
//        }

//        [UnitTest]
//        public void AccessFieldAOT2()
//        {
//            var c = new EnclosingClassAOT();
//            c.b.y = 0x1122334455667788;
//            Assert.Equal(0, c.a);
//            Assert.Equal(0x1122334455667788, c.b.y);
//            Assert.Equal(0, c.c);
//        }

//        [UnitTest]
//        public void FieldOffset2()
//        {
//            var fielda = typeof(EnclosingClass).GetField("a");
//            var baseOffset = IntPtr.Size * 2;
//            Assert.Equal(baseOffset, UnsafeUtility.GetFieldOffset(fielda));
//            var fieldb = typeof(EnclosingClass).GetField("b");
//            Assert.Equal(baseOffset + 8, UnsafeUtility.GetFieldOffset(fieldb));
//            var fieldc = typeof(EnclosingClass).GetField("c");
//            Assert.Equal(baseOffset + 8 + 8, UnsafeUtility.GetFieldOffset(fieldc));
//        }

//        [UnitTest]
//        public void FieldOffsetAOT2()
//        {
//            var fielda = typeof(EnclosingClassAOT).GetField("a");
//            var baseOffset = IntPtr.Size * 2;
//            Assert.Equal(baseOffset, UnsafeUtility.GetFieldOffset(fielda));
//            var fieldb = typeof(EnclosingClassAOT).GetField("b");
//            Assert.Equal(baseOffset + 8, UnsafeUtility.GetFieldOffset(fieldb));
//            var fieldc = typeof(EnclosingClassAOT).GetField("c");
//            Assert.Equal(baseOffset + 8 + 8, UnsafeUtility.GetFieldOffset(fieldc));
//        }
//    }
//}