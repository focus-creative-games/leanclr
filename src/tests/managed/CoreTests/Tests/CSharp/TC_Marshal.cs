using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;


namespace Tests.CSharp
{
    [System.Serializable]
    public struct BoundingInfo
    {
        public Vector3 BoundMin;
        public Vector3 BoundMax;
        public float MaxDistance;
        public float DistanceLOD1;
        public float DistanceLOD2;
    }

    [System.Serializable]
    public struct InstanceGroupInfo
    {
        public int target_id;
        public int group_id;
        string a;
    }

    [System.Serializable]
    public class InstanceGroupInfo2
    {
        public int target_id;
        public int group_id;
        string a;
    }

    public class TC_Marshal : GeneralTestCaseBase
    {

        [UnitTest]
        public void SizeOf2()
        {
            var g = new InstanceGroupInfo();
            int size = Marshal.SizeOf(g);
            Assert.Equal(8 + IntPtr.Size, size);
        }

        [StructLayout(LayoutKind.Explicit)]
        struct NetPack
        {
            [FieldOffset(0)]
            public uint len;
            [FieldOffset(4)]
            public ushort cmd;
            [FieldOffset(6)]
            public byte flag;
        }

        [UnitTest]
        public void SizeOf3()
        {
            int size = Marshal.SizeOf<NetPack>();
            Assert.Equal(8, size);
        }

        [StructLayout(LayoutKind.Explicit, Pack = 1)]
        struct NetPack1
        {
            [FieldOffset(0)]
            public uint len;
            [FieldOffset(4)]
            public ushort cmd;
            [FieldOffset(6)]
            public byte flag;
            [FieldOffset(8)]
            public byte flag2;
        }

        [UnitTest]
        public void SizeOf3_1()
        {
            int size = Marshal.SizeOf<NetPack1>();
            Assert.Equal(9, size);
        }

        [StructLayout(LayoutKind.Explicit, Pack = 2)]
        struct NetPack2
        {
            [FieldOffset(0)]
            public uint len;
            [FieldOffset(4)]
            public ushort cmd;
            [FieldOffset(6)]
            public byte flag;
            [FieldOffset(8)]
            public byte flag2;
        }

        [UnitTest]
        public void SizeOf3_2()
        {
            int size = Marshal.SizeOf<NetPack2>();
            Assert.Equal(10, size);
        }

        [StructLayout(LayoutKind.Explicit, Pack = 4)]
        struct NetPack3
        {
            [FieldOffset(0)]
            public uint len;
            [FieldOffset(4)]
            public ushort cmd;
            [FieldOffset(6)]
            public byte flag;
            [FieldOffset(8)]
            public byte flag2;
        }

        [UnitTest]
        public void SizeOf3_3()
        {
            int size = Marshal.SizeOf<NetPack3>();
            Assert.Equal(12, size);
        }

        [StructLayout(LayoutKind.Explicit, Pack = 8)]
        struct NetPack4
        {
            [FieldOffset(0)]
            public uint len;
            [FieldOffset(4)]
            public ushort cmd;
            [FieldOffset(6)]
            public byte flag;
            [FieldOffset(8)]
            public byte flag2;
        }

        [UnitTest]
        public void SizeOf3_4()
        {
            int size = Marshal.SizeOf<NetPack4>();
            Assert.Equal(12, size);
        }

        [StructLayout(LayoutKind.Explicit, Size = 100)]
        struct NetPack5
        {

        }

        [UnitTest]
        public void SizeOf3_5()
        {
            int size = Marshal.SizeOf<NetPack5>();
            Assert.Equal(100, size);
        }


        [StructLayout(LayoutKind.Sequential, Pack = 2)]
        struct SoType1
        {
            public int y;
            public ushort z;
            public byte w;
            public short a;
        }

        [UnitTest]
        public void SizeOf4()
        {
            int size = Marshal.SizeOf<SoType1>();
            Assert.Equal(10, size);
        }


        [StructLayout(LayoutKind.Sequential, Pack = 8)]
        struct SoType2
        {
            public int y;
            public ushort z;
            public byte w;
            public short a;
        }

        [UnitTest]
        public void SizeOf5()
        {
            int size = Marshal.SizeOf<SoType2>();
            Assert.Equal(12, size);
        }


        [StructLayout(LayoutKind.Sequential, Size = 100)]
        struct SoType3
        {

        }

        [UnitTest]
        public void SizeOf6()
        {
            int size = Marshal.SizeOf<SoType3>();
            Assert.Equal(100, size);
        }

        [StructLayout(LayoutKind.Explicit)]
        public unsafe partial struct BitSet2048
        {
            [FieldOffset(0)]
            private fixed UInt64 bits[32];
        }

        [UnitTest]
        public void SizeOf7()
        {
            int size = Marshal.SizeOf<BitSet2048>();
            Assert.Equal(256, size);
        }
    }
}
