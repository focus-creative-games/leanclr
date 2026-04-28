using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace Tests.CSharp
{

    internal class TC_InterpStructLayout : GeneralTestCaseBase
    {

        [StructLayout(LayoutKind.Sequential)]
        public struct StructSeqP0
        {
            byte a;
            long b;

            public int Compute(Func<StructSeqP0, int> func)
            {
                return func(this);
            }
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct StructSeqP1
        {
            byte a;
            long b;

            public int Compute(Func<StructSeqP1, int> func)
            {
                return func(this);
            }
        }

        [StructLayout(LayoutKind.Sequential, Pack = 2)]
        public struct StructSeqP2
        {
            byte a;
            long b;

            public int Compute(Func<StructSeqP2, int> func)
            {
                return func(this);
            }
        }

        [StructLayout(LayoutKind.Sequential, Pack = 2)]
        public struct StructSeqP3
        {
            byte a;
            string b;

            public int Compute(Func<StructSeqP3, int> func)
            {
                return func(this);
            }
        }

        [StructLayout(LayoutKind.Explicit)]
        public struct StructExpP0
        {
            [FieldOffset(0)]
            byte a;
            [FieldOffset(4)]
            long b;

            public int Compute(Func<StructExpP0, int> func)
            {
                return func(this);
            }
        }

        [StructLayout(LayoutKind.Explicit, Pack = 1)]
        public struct StructExpP1
        {
            [FieldOffset(0)]
            byte a;
            [FieldOffset(4)]
            long b;

            public int Compute(Func<StructExpP1, int> func)
            {
                return func(this);
            }
        }

        [StructLayout(LayoutKind.Explicit, Pack = 2)]
        public struct StructExpP2
        {
            [FieldOffset(0)]
            byte a;
            [FieldOffset(4)]
            long b;

            public int Compute(Func<StructExpP2, int> func)
            {
                return func(this);
            }
        }

        [StructLayout(LayoutKind.Explicit, Pack = 2)]
        public struct StructExpP3
        {
            [FieldOffset(0)]
            byte a;
            [FieldOffset(8)]
            string b;

            public int Compute(Func<StructExpP3, int> func)
            {
                return func(this);
            }
        }

        [StructLayout(LayoutKind.Auto)]
        public struct StructP0
        {
            byte a;
            long b;

            public int Compute(Func<StructP0, int> func)
            {
                return func(this);
            }
        }

        [StructLayout(LayoutKind.Auto, Pack = 1)]
        public struct StructP1
        {
            byte a;
            long b;

            public int Compute(Func<StructP1, int> func)
            {
                return func(this);
            }
        }

        [StructLayout(LayoutKind.Auto, Pack = 2)]
        public struct StructP2
        {
            byte a;
            long b;

            public int Compute(Func<StructP2, int> func)
            {
                return func(this);
            }
        }

        [StructLayout(LayoutKind.Auto, Pack = 2)]
        public struct StructP3
        {
            byte a;
            string b;

            public int Compute(Func<StructP3, int> func)
            {
                return func(this);
            }
        }

        [UnitTest]
        public void SizeofSeqP0()
        {
            Assert.Equal(16, Marshal.SizeOf<StructSeqP0>());
        }

        [UnitTest]
        public void SizeofSeqP1()
        {
            Assert.Equal(9, Marshal.SizeOf<StructSeqP1>());
        }

        [UnitTest]
        public void SizeofSeqP2()
        {
            Assert.Equal(10, Marshal.SizeOf<StructSeqP2>());
        }

        [UnitTest]
        public void SizeofSeqP3()
        {
            Assert.Equal(16, Marshal.SizeOf<StructSeqP3>());
        }

        [UnitTest]
        public void SizeofExpP0()
        {
            Assert.Equal(16, Marshal.SizeOf<StructExpP0>());
        }

        [UnitTest]
        public void SizeofExpP1()
        {
            Assert.Equal(12, Marshal.SizeOf<StructExpP1>());
        }

        [UnitTest]
        public void SizeofExpP2()
        {
            Assert.Equal(12, Marshal.SizeOf<StructExpP2>());
        }

        [UnitTest]
        public void SizeofExpP3()
        {
            Assert.Equal(16, Marshal.SizeOf<StructExpP3>());
        }

        [UnitTest]
        public void SizeofP0()
        {
            Assert.Equal(16, Marshal.SizeOf<StructP0>());
        }

        [UnitTest]
        public void SizeofP1()
        {
            Assert.Equal(9, Marshal.SizeOf<StructP1>());
        }

        [UnitTest]
        public void SizeofP2()
        {
            Assert.Equal(10, Marshal.SizeOf<StructP2>());
        }

        [UnitTest]
        public void SizeofP3()
        {
            Assert.Equal(16, Marshal.SizeOf<StructP3>());
        }
    }
}
