using test;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Bugs
{
    public class BugSizeOf : GeneralTestCaseBase
    {
        public enum GraphicsFormat
        {

        }

        public struct Int2
        {
            public int a;
            public int b;
        }

        [StructLayout(LayoutKind.Explicit)]
        public struct LongInt
        {
            [FieldOffset(0)]
            public long value;
            [FieldOffset(0)]
            public Int2 value2;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        public struct ShortGuid
        {
            public LongInt low;
            public LongInt high;
        }

        public struct TexInfo100
        {
            public byte mip;
            public ShortGuid guid;
            public int size;
        }

        //[UnitTest]
        //public void ComplexSize()
        //{
        //    Assert.Equal(24, System.Runtime.CompilerServices.SizeOf<TexInfo100>());
        //}

        //[UnitTest]
        //public void ComplexSize2()
        //{
        //    Assert.Equal(24, UnsafeUtility.SizeOf<ValueTuple<int, MyDecimal>>());
        //}

        //[UnitTest]
        //public void ComplexSize3()
        //{
        //    Assert.Equal(24, UnsafeUtility.SizeOf<Pair>());
        //}
    }
}
