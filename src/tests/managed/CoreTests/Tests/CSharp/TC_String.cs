using System;
using System.Text;

namespace Tests.CSharp
{
    class TC_String : GeneralTestCaseBase
    {
        [UnitTest]
        public void EmptyString()
        {
            string s = string.Empty;
            Assert.NotNull(s);
            Assert.Equal(0, s.Length);
        }

        [UnitTest]
        public void CreateStringFromCharArray()
        {
            char[] arr = new char[] { 'a', 'b', 'c', 'd', 'e' };
            var s = new string(arr);
            Assert.Equal("abcde", s);
        }

        [UnitTest]
        public void CreateStringFromCharArrayWithOffset()
        {
            char[] arr = new char[] { 'a', 'b', 'c', 'd', 'e' };
            var s = new string(arr, 1, 2);
            Assert.Equal("bc", s);
        }

        [UnitTest]
        public unsafe void CreateStringFromCharPtr()
        {
            char[] arr = new char[] { 'a', 'b', 'c', 'd', 'e', '\0' };
            fixed (char* p = &arr[0])
            {
                var s = new string(p);
                Assert.Equal("abcde", s);
            }
        }

        [UnitTest]
        public unsafe void CreateStringFromCharPtrWithOffset()
        {
            char[] arr = new char[] { 'a', 'b', 'c', 'd', 'e', '\0' };
            fixed (char* p = &arr[0])
            {
                var s = new string(p, 1, 2);
                Assert.Equal("bc", s);
            }
        }

        [UnitTest]
        public unsafe void CreateStringFromSBytePtr()
        {
            var arr = new sbyte[] { (sbyte)'a', (sbyte)'b', (sbyte)'c', (sbyte)'d', (sbyte)'e', 0 };
            fixed (sbyte* p = &arr[0])
            {
                var s = new string(p);
                Assert.Equal("abcde", s);
            }
        }

        [UnitTest]
        public unsafe void CreateStringFromSBytePtrWithOffset()
        {
            var arr = new sbyte[] { (sbyte)'a', (sbyte)'b', (sbyte)'c', (sbyte)'d', (sbyte)'e', 0 };
            fixed (sbyte* p = &arr[0])
            {
                var s = new string(p, 1, 2);
                Assert.Equal("bc", s);
            }
        }

        [UnitTest]
        public unsafe void CreateStringFromSBytePtrWithOffsetAndEncoding()
        {
            var arr = new sbyte[] { (sbyte)'a', (sbyte)'b', (sbyte)'c', (sbyte)'d', (sbyte)'e', 0 };
            fixed (sbyte* p = &arr[0])
            {
                // this function will be redirected to string.Ctor(sbyte*, int, int, Encoding)
                var s = new string(p, 1, 2, Encoding.UTF8);
                Assert.Equal("bc", s);
            }
        }

        [UnitTest]
        public void CreateStringFromReadOnlySpanChar()
        {
            char[] arr = new char[] { 'a', 'b', 'c', 'd', 'e' };
            var s = new string(new ReadOnlySpan<char>(arr));
            Assert.Equal("abcde", s);
        }

        [UnitTest]
        public void CreateEmptyStringFromEmptySpan()
        {
            var s = new string(new ReadOnlySpan<char>());
            Assert.Equal(string.Empty, s);
        }

        [UnitTest]
        public void EmptySpanToString()
        {
            ReadOnlySpan<char> span = ReadOnlySpan<char>.Empty;
            var s = span.ToString();
            Assert.Equal(string.Empty, s);
        }
    }
}
