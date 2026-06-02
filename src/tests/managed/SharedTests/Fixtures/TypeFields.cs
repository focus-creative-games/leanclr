using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Test;

namespace AOTDefs
{
    public class TypeFields
    {
        public byte x1 = 1;
        public sbyte x2 = 2;
        public bool x3 = true;
        public byte x4 = 4;
        public short x5 = 5;
        public ushort x6 = 6;
        public int x7 = 7;
        public uint x8 = 8;
        public long x9 = 9;
        public ulong x10 = 10;
        public float y1 = 1f;
        public float y2 = 2f;
        public double y3 = 3.0;
        public object y4 = "a";
        public ValueTypeSize1 s1 = new ValueTypeSize1 { x1 = 1};
        public ValueTypeSize2 s2 = new ValueTypeSize2 { x1 = 2};
        public ValueTypeSize3 s3 = new ValueTypeSize3 { x1 = 3};
        public ValueTypeSize4 s4 = new ValueTypeSize4 { x1 = 4};
        public ValueTypeSize5 s5 = new ValueTypeSize5 { x1 = 5};
        public ValueTypeSize8 s8 = new ValueTypeSize8 { x1 = 6};
        public ValueTypeSize9 s9 = new ValueTypeSize9 { x1 = 7};
        public ValueTypeSize16 s16 = new ValueTypeSize16 { x1 = 8};

        public AOT_Enum_byte e1 = AOT_Enum_byte.A;
        public AOT_Enum_sbyte e2 = AOT_Enum_sbyte.B;
        public AOT_Enum_short e3 = AOT_Enum_short.A;
        public AOT_Enum_ushort e4 = AOT_Enum_ushort.A;
        public AOT_Enum_int e5 = AOT_Enum_int.B;
        public AOT_Enum_uint e6 = AOT_Enum_uint.A;
        public AOT_Enum_long e7 = AOT_Enum_long.B;
        public AOT_Enum_ulong e8 = AOT_Enum_ulong.A;
    }
}
