using System;
using System.Threading;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Threading_Thread : TestCaseBase
    {
        [UnitTest]
        public void GetCurrentThread_icall()
        {
            var t = Thread.CurrentThread;
            Assert.NotNull(t);
        }

        [UnitTest]
        public void DomainId()
        {
            int id = Thread.GetDomainID();
            Assert.Equal(1, id);
        }

        [UnitTest]
        public void YieldInternal()
        {
            bool yielded = Thread.Yield();
            Assert.Equal(false, yielded);
        }

        [UnitTest]
        public void SleepInternal()
        {
            Thread.Sleep(0);
            Assert.True(true);
        }

#if SINGLE_THREAD
        [UnitTest]
        public void JoinInternal()
        {
            // On single-threaded WASM, starting new threads isn't supported.
            // Validate that joining the current thread behaves trivially.
            bool joined = Thread.CurrentThread.Join(0);
            Assert.Equal(true, joined);
        }
#endif

        [UnitTest]
        public void PriorityDefaults()
        {
            var p = Thread.CurrentThread.Priority;
            Assert.Equal(ThreadPriority.Normal, p);
        }

        [UnitTest]
        public void VolatileReadWrite_Int32()
        {
            int x = 1;
            int r1 = Thread.VolatileRead(ref x);
            Assert.Equal(1, r1);
            Thread.VolatileWrite(ref x, 2);
            int r2 = Thread.VolatileRead(ref x);
            Assert.Equal(2, r2);
        }

        [UnitTest]
        public void VolatileReadWrite_Object()
        {
            object o = new object();
            object r1 = Thread.VolatileRead(ref o);
            Assert.NotNull(r1);
            object o2 = new object();
            Thread.VolatileWrite(ref o, o2);
            object r2 = Thread.VolatileRead(ref o);
            Assert.Equal(o2, r2);
        }

        [UnitTest]
        public void VolatileReadWrite_Byte()
        {
            byte x = 3;
            byte r1 = Thread.VolatileRead(ref x);
            Assert.Equal((byte)3, r1);
            Thread.VolatileWrite(ref x, (byte)4);
            byte r2 = Thread.VolatileRead(ref x);
            Assert.Equal((byte)4, r2);
        }

        [UnitTest]
        public void VolatileReadWrite_SByte()
        {
            sbyte x = -3;
            sbyte r1 = Thread.VolatileRead(ref x);
            Assert.Equal((sbyte)-3, r1);
            Thread.VolatileWrite(ref x, (sbyte)-4);
            sbyte r2 = Thread.VolatileRead(ref x);
            Assert.Equal((sbyte)-4, r2);
        }

        [UnitTest]
        public void VolatileReadWrite_Int16()
        {
            short x = -10;
            short r1 = Thread.VolatileRead(ref x);
            Assert.Equal((short)-10, r1);
            Thread.VolatileWrite(ref x, (short)-11);
            short r2 = Thread.VolatileRead(ref x);
            Assert.Equal((short)-11, r2);
        }

        [UnitTest]
        public void VolatileReadWrite_UInt16()
        {
            ushort x = 10;
            ushort r1 = Thread.VolatileRead(ref x);
            Assert.Equal((ushort)10, r1);
            Thread.VolatileWrite(ref x, (ushort)11);
            ushort r2 = Thread.VolatileRead(ref x);
            Assert.Equal((ushort)11, r2);
        }

        [UnitTest]
        public void VolatileReadWrite_UInt32()
        {
            uint x = 100u;
            uint r1 = Thread.VolatileRead(ref x);
            Assert.Equal(100u, r1);
            Thread.VolatileWrite(ref x, 101u);
            uint r2 = Thread.VolatileRead(ref x);
            Assert.Equal(101u, r2);
        }

        [UnitTest]
        public void VolatileReadWrite_Int64()
        {
            long x = -1000L;
            long r1 = Thread.VolatileRead(ref x);
            Assert.Equal(-1000L, r1);
            Thread.VolatileWrite(ref x, -1001L);
            long r2 = Thread.VolatileRead(ref x);
            Assert.Equal(-1001L, r2);
        }

        [UnitTest]
        public void VolatileReadWrite_UInt64()
        {
            ulong x = 1000UL;
            ulong r1 = Thread.VolatileRead(ref x);
            Assert.Equal(1000UL, r1);
            Thread.VolatileWrite(ref x, 1001UL);
            ulong r2 = Thread.VolatileRead(ref x);
            Assert.Equal(1001UL, r2);
        }

        [UnitTest]
        public void VolatileReadWrite_Single()
        {
            float x = 1.5f;
            float r1 = Thread.VolatileRead(ref x);
            Assert.Equal(1.5f, r1);
            Thread.VolatileWrite(ref x, 2.5f);
            float r2 = Thread.VolatileRead(ref x);
            Assert.Equal(2.5f, r2);
        }

        [UnitTest]
        public void VolatileReadWrite_Double()
        {
            double x = -2.25;
            double r1 = Thread.VolatileRead(ref x);
            Assert.Equal(-2.25, r1);
            Thread.VolatileWrite(ref x, -3.5);
            double r2 = Thread.VolatileRead(ref x);
            Assert.Equal(-3.5, r2);
        }

        [UnitTest]
        public void VolatileReadWrite_IntPtr()
        {
            IntPtr x = new IntPtr(42);
            IntPtr r1 = Thread.VolatileRead(ref x);
            Assert.Equal(new IntPtr(42), r1);
            Thread.VolatileWrite(ref x, new IntPtr(43));
            IntPtr r2 = Thread.VolatileRead(ref x);
            Assert.Equal(new IntPtr(43), r2);
        }

        [UnitTest]
        public void VolatileReadWrite_UIntPtr()
        {
            UIntPtr x = new UIntPtr(42);
            UIntPtr r1 = Thread.VolatileRead(ref x);
            Assert.Equal(new UIntPtr(42), r1);
            Thread.VolatileWrite(ref x, new UIntPtr(43));
            UIntPtr r2 = Thread.VolatileRead(ref x);
            Assert.Equal(new UIntPtr(43), r2);
        }

        [UnitTest]
        public void MemoryBarrier_NoThrow()
        {
            Thread.MemoryBarrier();
            Assert.True(true);
        }
    }
}
