using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AOTDefs
{

    public struct ValueTypeSize0
    {
        public unsafe void InitAs1()
        {
            ValueTypeSize0 v = default;
            *(byte*)&v = 1;
            this = v;
        }

        public unsafe int GetValue()
        {
            ValueTypeSize0 v = this;
            return *(byte*)&v;
        }
    }

    public struct ValueTypeSize1
    {
        public byte x1;

        public override string ToString()
        {
            return $"x1={x1}";
        }
    }

    public struct ValueTypeSize2
    {
        public byte x1;
        public byte x2;
    }

    public struct ValueTypeSize3
    {
        public byte x1;
        public byte x2;
        public byte x3;
    }

    public struct ValueTypeSize4
    {
        public int x1;
    }

    public struct ValueTypeSize5
    {
        public byte x1;
        public byte x2;
        public byte x3;
        public byte x4;
        public byte x5;
    }

    public struct ValueTypeSize8
    {
        public long x1;
    }

    public struct ValueTypeSize9
    {
        public byte x1;
        public byte x2;
        public byte x3;
        public byte x4;
        public byte x5;
        public byte x6;
        public byte x7;
        public byte x8;
        public byte x9;
    }

    public struct ValueTypeSize16
    {
        public long x1;
        public long x2;
    }

    public struct ValueTypeSize20
    {
        public int x1;
        public int x2;
        public int x3;
        public int x4;
        public int x5;
    }

}
