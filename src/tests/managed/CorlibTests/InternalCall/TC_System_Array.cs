using System;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Array : TestCaseBase
    {
        [CoversIcall("System.Array::ClearInternal")]
        [UnitTest]
        public void Clear_IntArray_AllElements()
        {
            int[] arr = { 1, 2, 3, 4, 5 };
            Array.Clear(arr, 0, arr.Length);
            Assert.Equal(0, arr[0]);
            Assert.Equal(0, arr[1]);
            Assert.Equal(0, arr[2]);
            Assert.Equal(0, arr[3]);
            Assert.Equal(0, arr[4]);
        }

        [UnitTest]
        public void Clear_IntArray_PartialRange()
        {
            int[] arr = { 10, 20, 30, 40, 50 };
            Array.Clear(arr, 1, 3);
            Assert.Equal(10, arr[0]);
            Assert.Equal(0, arr[1]);
            Assert.Equal(0, arr[2]);
            Assert.Equal(0, arr[3]);
            Assert.Equal(50, arr[4]);
        }

        [UnitTest]
        public void Clear_ByteArray_PartialRange()
        {
            byte[] arr = { 0xAA, 0xBB, 0xCC, 0xDD };
            Array.Clear(arr, 1, 2);
            Assert.Equal(0xAA, arr[0]);
            Assert.Equal(0, arr[1]);
            Assert.Equal(0, arr[2]);
            Assert.Equal(0xDD, arr[3]);
        }

        [UnitTest]
        public void Clear_BoolArray_AllElements()
        {
            bool[] arr = { true, true, false, true };
            Array.Clear(arr, 0, arr.Length);
            Assert.Equal(false, arr[0]);
            Assert.Equal(false, arr[1]);
            Assert.Equal(false, arr[2]);
            Assert.Equal(false, arr[3]);
        }

        [UnitTest]
        public void Clear_IntArray_ZeroLength_NoOp()
        {
            int[] arr = { 1, 2, 3 };
            Array.Clear(arr, 1, 0);
            Assert.Equal(1, arr[0]);
            Assert.Equal(2, arr[1]);
            Assert.Equal(3, arr[2]);
        }

        [UnitTest]
        public void ClearEmptyIntArray()
        {
            int[] arr = { };
            Array.Clear(arr, 0, arr.Length);
            Assert.Equal(0, arr.Length);
        }

        [UnitTest]
        public void ClearIntArrayAtEndWithLengthZero()
        {
            int[] arr = { 1, 2, 3 };
            Array.Clear(arr, 3, 0);
            Assert.Equal(1, arr[0]);
            Assert.Equal(2, arr[1]);
            Assert.Equal(3, arr[2]);
        }
    }
}
