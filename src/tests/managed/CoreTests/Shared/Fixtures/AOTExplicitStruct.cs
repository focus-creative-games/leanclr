using System;
using System.Runtime.InteropServices;

namespace AOTDefs
{
    [StructLayout(LayoutKind.Explicit)]
    public struct AOTExplicitStruct
    {
        [FieldOffset(0)] public int x;
        [FieldOffset(0)] public long y;
        [FieldOffset(0)] public float a;
        [FieldOffset(0)] public double b;
    }


    [StructLayout(LayoutKind.Explicit)]
    public struct MyDecimal
    {
        [FieldOffset(0)]
        private readonly int flags;

        [FieldOffset(4)]
        private readonly int hi;

        [FieldOffset(8)]
        private readonly int lo;

        [FieldOffset(12)]
        private readonly int mid;

        [FieldOffset(8)]
        [NonSerialized]
        private readonly ulong ulomidLE;
    }

    public struct Pair
    {
        public int a;
        public MyDecimal b;

        public ValueTuple<int, MyDecimal> ToValueTuple()
        {
            return (a, b);
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct StructSeqP0
    {
        public byte a;
        public long b;

        public long Compute(Func<StructSeqP0, long> func)
        {
            return func(this);
        }
    }

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct StructSeqP1
    {
        public byte a;
        public long b;

        public long Compute(Func<StructSeqP1, long> func)
        {
            return func(this);
        }
    }

    [StructLayout(LayoutKind.Sequential, Pack = 2)]
    public struct StructSeqP2
    {
        public byte a;
        public long b;

        public long Compute(Func<StructSeqP2, long> func)
        {
            return func(this);
        }
    }

    [StructLayout(LayoutKind.Sequential, Pack = 2)]
    public struct StructSeqP3
    {
        public byte a;
        public string b;

        public long Compute(Func<StructSeqP3, long> func)
        {
            return func(this);
        }
    }

    [StructLayout(LayoutKind.Explicit)]
    public struct StructExpP0
    {
        [FieldOffset(0)]
        public byte a;
        [FieldOffset(4)]
        public long b;

        public long Compute(Func<StructExpP0, long> func)
        {
            return func(this);
        }
    }

    [StructLayout(LayoutKind.Explicit, Pack = 1)]
    public struct StructExpP1
    {
        [FieldOffset(0)]
        public byte a;
        [FieldOffset(4)]
        public long b;

        public long Compute(Func<StructExpP1, long> func)
        {
            return func(this);
        }
    }

    [StructLayout(LayoutKind.Explicit, Pack = 2)]
    public struct StructExpP2
    {
        [FieldOffset(0)]
        public byte a;
        [FieldOffset(4)]
        public long b;

        public long Compute(Func<StructExpP2, long> func)
        {
            return func(this);
        }
    }

    [StructLayout(LayoutKind.Explicit, Pack = 2)]
    public struct StructExpP3
    {
        [FieldOffset(0)]
        public byte a;
        [FieldOffset(8)]
        public string b;

        public long Compute(Func<StructExpP3, long> func)
        {
            return func(this);
        }
    }

    [StructLayout(LayoutKind.Auto)]
    public struct StructP0
    {
        public byte a;
        public long b;

        public long Compute(Func<StructP0, long> func)
        {
            return func(this);
        }
    }

    [StructLayout(LayoutKind.Auto, Pack = 1)]
    public struct StructP1
    {
        public byte a;
        public long b;

        public long Compute(Func<StructP1, long> func)
        {
            return func(this);
        }
    }

    [StructLayout(LayoutKind.Auto, Pack = 2)]
    public struct StructP2
    {
        public byte a;
        public long b;

        public long Compute(Func<StructP2, long> func)
        {
            return func(this);
        }
    }

    [StructLayout(LayoutKind.Auto, Pack = 2)]
    public struct StructP3
    {
        public byte a;
        public string b;

        public long Compute(Func<StructP3, long> func)
        {
            return func(this);
        }
    }



    [StructLayout(LayoutKind.Sequential, Pack = 2)]
    public struct MBStructSeqP3
    {
        public string x1;
        public string x2;
        public string x3;
        public string x4;
        public byte a;
        public string b;

        public long Compute(Func<MBStructSeqP3, long> func)
        {
            return func(this);
        }
    }

    public struct MBStructSeqP4
    {
        public byte a;
        public MBStructSeqP3 b;
    }


    [StructLayout(LayoutKind.Explicit, Pack = 2)]
    public struct MBStructExpP3
    {
        [FieldOffset(0)]
        public byte a;
        [FieldOffset(8)]
        public string b;

        public long Compute(Func<MBStructExpP3, long> func)
        {
            return func(this);
        }
    }


    public struct MBStructExpP4
    {
        public byte a;
        public MBStructExpP3 b;
    }


    [StructLayout(LayoutKind.Auto, Pack = 2)]
    public struct MBStructP3
    {
        public string x1;
        public string x2;
        public string x3;
        public string x4;
        public byte a;
        public string b;

        public long Compute(Func<MBStructP3, long> func)
        {
            return func(this);
        }
    }

    public struct MBStructP4
    {
        public byte a;
        public MBStructP3 b;
    }
}