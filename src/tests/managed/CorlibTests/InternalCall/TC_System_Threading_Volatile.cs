using System;
using System.Threading;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Threading_Volatile : TestCaseBase
    {
        [CoversIcall("System.Threading.Volatile::Read(System.Int64&)")]
        [CoversIcall("System.Threading.Volatile::Write(System.Int64&,System.Int64)")]
        [UnitTest]
        public void ReadWrite_Int64()
        {
            long x = -1234567890123L;
            long r1 = Volatile.Read(ref x);
            Assert.Equal(-1234567890123L, r1);
            Volatile.Write(ref x, -1234567890124L);
            long r2 = Volatile.Read(ref x);
            Assert.Equal(-1234567890124L, r2);
        }

        [CoversIcall("System.Threading.Volatile::Read(System.UInt64&)")]
        [CoversIcall("System.Threading.Volatile::Write(System.UInt64&,System.UInt64)")]
        [UnitTest]
        public void ReadWrite_UInt64()
        {
            ulong x = 1234567890123UL;
            ulong r1 = Volatile.Read(ref x);
            Assert.Equal(1234567890123UL, r1);
            Volatile.Write(ref x, 1234567890124UL);
            ulong r2 = Volatile.Read(ref x);
            Assert.Equal(1234567890124UL, r2);
        }

        [CoversIcall("System.Threading.Volatile::Read(System.Double&)")]
        [CoversIcall("System.Threading.Volatile::Write(System.Double&,System.Double)")]
        [UnitTest]
        public void ReadWrite_Double()
        {
            double x = -3.1415926535;
            double r1 = Volatile.Read(ref x);
            Assert.Equal(-3.1415926535, r1);
            Volatile.Write(ref x, -2.7182818284);
            double r2 = Volatile.Read(ref x);
            Assert.Equal(-2.7182818284, r2);
        }
    }
}
