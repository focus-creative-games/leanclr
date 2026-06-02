//using test;
//using System;
//using System.Collections.Generic;
//using System.Linq;
//using System.Text;
//using System.Threading.Tasks;


//namespace Tests.CSharp
//{

//    internal class TC_Instinct : GeneralTestCaseBase
//    {
//        [UnitTest]
//        public void NewString()
//        {
//            var cs = new char[] { 'a', 'b' };
//            var s = new string(cs);
//            Assert.Equal("ab", s);
//        }

//        [UnitTest]
//        public void NewString2()
//        {
//            var cs = new char[] { 'a', 'b', 'c', 'd', 'e' };
//            var s = new string(cs, 1, 2);
//            Assert.Equal("bc", s);
//        }

//        [UnitTest]
//        public void NewString2_2()
//        {
//            var cs = new char[] { 'a', 'b', 'c', 'd', 'e' };
//            var s = new string(cs, 2, 3);
//            Assert.Equal("cde", s);
//        }

//        [UnitTest]
//        public void NewString2_3()
//        {
//            var cs = new char[] { 'a', 'b', 'c', 'd', 'e' };
//            var s = new string(cs, 2, 0);
//            Assert.Equal("", s);
//        }

//        [UnitTest]
//        public void NewString2_OutRange1()
//        {
//            var chars = new char[] { 'h', 'e', 'l', 'l', '0' };

//            Assert.ExpectException<ArgumentOutOfRangeException>();
//            new string(chars, 2, -1);
//            Assert.Fail();
//        }

//        [UnitTest]
//        public void NewString2_OutRange2()
//        {
//            var chars = new char[] { 'h', 'e', 'l', 'l', '0' };

//            Assert.ExpectException<ArgumentOutOfRangeException>();
//            new string(chars, 0, -1);
//            Assert.Fail();
//        }

//        [UnitTest]
//        public void NewString3()
//        {
//            var s = new string('a', 3);
//            Assert.Equal("aaa", s);
//        }

//        [UnitTest]
//        public unsafe void NewStringEncoding()
//        {
//            var ch = stackalloc sbyte[] { (sbyte)'a', (sbyte)'b', (sbyte)'c' };
//            var s = new string(ch, 0, 3, Encoding.UTF8);
//            Assert.Equal("abc", s);
//        }

//        [UnitTest]
//        public unsafe void NewStringSBytePtr()
//        {
//            var ch = stackalloc sbyte[] { (sbyte)'a', (sbyte)'b', (sbyte)'c', 0 };
//            var s = new string(ch);
//            Assert.Equal("abc", s);
//        }

//        [UnitTest]
//        public unsafe void NewStringSBytePtr2()
//        {
//            var ch = stackalloc sbyte[] { (sbyte)'a', (sbyte)'b', (sbyte)'c' };
//            var s = new string(ch, 1, 2);
//            Assert.Equal("bc", s);
//        }

//        [UnitTest]
//        public unsafe void NewStringCharPtr()
//        {
//            var ch = stackalloc char[] { 'a', 'b', 'c', (char)0 };
//            var s = new string(ch);
//            Assert.Equal("abc", s);
//        }

//        [UnitTest]
//        public unsafe void NewStringCharPtr2()
//        {
//            var ch = stackalloc char[] { 'a', 'b', 'c' };
//            var s = new string(ch, 1, 2);
//            Assert.Equal("bc", s);
//        }

//        struct MyInt
//        {
//            public int value;
//        }

//        [UnitTest]
//        public void Vector2Ctor()
//        {
//            Vector2 v = default;
//            v = new Vector2(1, 2);
//            Assert.Equal(1f, v.x);
//            Assert.Equal(2f, v.y);
//        }

//        [UnitTest]
//        public void Vector3Ctor2()
//        {
//            Vector3 v = default;
//            v = new Vector3(1, 2);
//            Assert.Equal(1f, v.x);
//            Assert.Equal(2f, v.y);
//            Assert.Equal(0f, v.z);
//        }

//        [UnitTest]
//        public void Vector3Ctor3()
//        {
//            Vector3 v = default;
//            v = new Vector3(1, 2, 3);
//            Assert.Equal(1f, v.x);
//            Assert.Equal(2f, v.y);
//            Assert.Equal(3f, v.z);
//        }

//        [UnitTest]
//        public void Vector4Ctor2()
//        {
//            Vector4 v = default;
//            v = new Vector4(1, 2);
//            Assert.Equal(1f, v.x);
//            Assert.Equal(2f, v.y);
//            Assert.Equal(0f, v.z);
//            Assert.Equal(0f, v.w);
//        }

//        [UnitTest]
//        public void Vector4Ctor3()
//        {
//            Vector4 v = default;
//            v = new Vector4(1, 2, 3);
//            Assert.Equal(1f, v.x);
//            Assert.Equal(2f, v.y);
//            Assert.Equal(3f, v.z);
//            Assert.Equal(0f, v.w);
//        }

//        [UnitTest]
//        public void Vector4Ctor4()
//        {
//            Vector4 v = default;
//            v = new Vector4(1, 2, 3, 4);
//            Assert.Equal(1f, v.x);
//            Assert.Equal(2f, v.y);
//            Assert.Equal(3f, v.z);
//            Assert.Equal(4f, v.w);
//        }



//        [UnitTest]
//        public void Vector2New()
//        {
//            Assert.Equal(1f, new Vector2(1, 2).x);
//            Assert.Equal(2f, new Vector2(1, 2).y);
//        }

//        [UnitTest]
//        public void Vector3New2()
//        {
//            Assert.Equal(1f, new Vector2(1, 2).x);
//            Assert.Equal(2f, new Vector2(1, 2).y);
//            Assert.Equal(0f, new Vector3(1, 2).z);
//        }

//        [UnitTest]
//        public void Vector3New3()
//        {
//            Assert.Equal(1f, new Vector3(1, 2, 3).x);
//            Assert.Equal(2f, new Vector3(1, 2, 3).y);
//            Assert.Equal(3f, new Vector3(1, 2, 3).z);
//        }

//        [UnitTest]
//        public void Vector4New2()
//        {
//            Assert.Equal(1f, new Vector4(1, 2).x);
//            Assert.Equal(2f, new Vector4(1, 2).y);
//            Assert.Equal(0f, new Vector4(1, 2).z);
//            Assert.Equal(0f, new Vector4(1, 2).w);
//        }

//        [UnitTest]
//        public void Vector4New3()
//        {
//            Assert.Equal(1f, new Vector4(1, 2, 3).x);
//            Assert.Equal(2f, new Vector4(1, 2, 3).y);
//            Assert.Equal(3f, new Vector4(1, 2, 3).z);
//            Assert.Equal(0f, new Vector4(1, 2, 3).w);
//        }

//        [UnitTest]
//        public void Vector4New4()
//        {
//            Assert.Equal(1f, new Vector4(1, 2, 3, 4).x);
//            Assert.Equal(2f, new Vector4(1, 2, 3, 4).y);
//            Assert.Equal(3f, new Vector4(1, 2, 3, 4).z);
//            Assert.Equal(4f, new Vector4(1, 2, 3, 4).w);
//        }
//    }
//}
