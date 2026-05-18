using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Buffer : GeneralTestCaseBase
    {
        [UnitTest]
        public void ByteLength_ByteArray()
        {
            byte[] arr = new byte[10];
            int length = Buffer.ByteLength(arr);
            Assert.Equal(10, length);
        }

        [UnitTest]
        public void ByteLength_IntArray()
        {
            int[] arr = new int[5];
            int length = Buffer.ByteLength(arr);
            Assert.Equal(20, length); // 5 integers * 4 bytes each
        }

        //[UnitTest]
        //public void ByteLength_StringArray()
        //{
        //    string[] arr = new string[3];
        //    int length = Buffer.ByteLength(arr);
        //    Assert.Equal(-1, length); // Reference types return -1
        //}

        // MemoryCopy
        [UnitTest]
        public unsafe void MemoryCopy_ByteArray()
        {
            byte[] src = new byte[] { 1, 2, 3, 4, 5 };
            byte[] dest = new byte[5];
            fixed (byte* pSrc = src)
            {
                fixed (byte* pDest = dest)
                {
                    Buffer.MemoryCopy(pSrc, pDest, dest.Length, src.Length);
                }
            }
            Assert.Equal(1, dest[0]);
            Assert.Equal(2, dest[1]);
            Assert.Equal(3, dest[2]);
            Assert.Equal(4, dest[3]);
            Assert.Equal(5, dest[4]);
        }

        // BlockCopy — element size 1 (byte)
        [UnitTest]
        public void BlockCopy_ByteArray_DifferentArrays()
        {
            byte[] src = new byte[] { 1, 2, 3, 4, 5 };
            byte[] dest = new byte[5];
            Buffer.BlockCopy(src, 1, dest, 2, 3);
            Assert.Equal(0, dest[0]);
            Assert.Equal(0, dest[1]);
            Assert.Equal(2, dest[2]);
            Assert.Equal(3, dest[3]);
            Assert.Equal(4, dest[4]);
        }

        [UnitTest]
        public void BlockCopy_ByteArray_SameArray_Overlap()
        {
            byte[] arr = new byte[] { 1, 2, 3, 4, 5, 6 };
            Buffer.BlockCopy(arr, 0, arr, 2, 4);
            Assert.Equal(1, arr[0]);
            Assert.Equal(2, arr[1]);
            Assert.Equal(1, arr[2]);
            Assert.Equal(2, arr[3]);
            Assert.Equal(3, arr[4]);
            Assert.Equal(4, arr[5]);
        }

        // BlockCopy — element size > 1 (int, 4 bytes per element)
        [UnitTest]
        public void BlockCopy_IntArray_DifferentArrays()
        {
            int[] src = new int[] { 0x01020304, 0x05060708, 0x090A0B0C };
            int[] dest = new int[3];
            Buffer.BlockCopy(src, 4, dest, 4, 8);
            Assert.Equal(0, dest[0]);
            Assert.Equal(0x05060708, dest[1]);
            Assert.Equal(0x090A0B0C, dest[2]);
        }

        [UnitTest]
        public void BlockCopy_IntArray_SameArray_Overlap()
        {
            int[] arr = new int[] { 1, 2, 3, 4, 5 };
            Buffer.BlockCopy(arr, 0, arr, 4, 8);
            Assert.Equal(1, arr[0]);
            Assert.Equal(1, arr[1]);
            Assert.Equal(2, arr[2]);
            Assert.Equal(4, arr[3]);
            Assert.Equal(5, arr[4]);
        }

        [UnitTest]
        public void BlockCopy_IntArray_PartialElementBytes()
        {
            int[] src = new int[] { unchecked((int)0xAABBCCDD) };
            int[] dest = new int[1];
            // Little-endian layout: DD CC BB AA — copy 2 bytes at byte offset 1 (CC BB).
            Buffer.BlockCopy(src, 1, dest, 0, 2);
            Assert.Equal(unchecked((int)0x0000BBCC), dest[0]);
        }
    }
}
