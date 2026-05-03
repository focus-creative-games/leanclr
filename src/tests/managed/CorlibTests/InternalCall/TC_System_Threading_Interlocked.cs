using System;
using System.Threading;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Threading_Interlocked : GeneralTestCaseBase
    {
        [UnitTest]
        public void Add_Int32()
        {
            int location = 10;
            int result = Interlocked.Add(ref location, 5);
            Assert.Equal(15, location);
            Assert.Equal(15, result);
        }

        [UnitTest]
        public void Add_Int64()
        {
            long location = 10L;
            long result = Interlocked.Add(ref location, 5L);
            Assert.Equal(15L, location);
            Assert.Equal(15L, result);
        }

        [UnitTest]
        public void Increment_Int32()
        {
            int location = 10;
            int result = Interlocked.Increment(ref location);
            Assert.Equal(11, location);
            Assert.Equal(11, result);
        }

        [UnitTest]
        public void Increment_Int64()
        {
            long location = 10L;
            long result = Interlocked.Increment(ref location);
            Assert.Equal(11L, location);
            Assert.Equal(11L, result);
        }

        [UnitTest]
        public void Decrement_Int32()
        {
            int location = 10;
            int result = Interlocked.Decrement(ref location);
            Assert.Equal(9, location);
            Assert.Equal(9, result);
        }

        [UnitTest]
        public void Decrement_Int64()
        {
            long location = 10L;
            long result = Interlocked.Decrement(ref location);
            Assert.Equal(9L, location);
            Assert.Equal(9L, result);
        }

        [UnitTest]
        public void Exchange_Int32()
        {
            int location = 10;
            int result = Interlocked.Exchange(ref location, 20);
            Assert.Equal(20, location);
            Assert.Equal(10, result);
        }

        [UnitTest]
        public void Exchange_Int64()
        {
            long location = 10L;
            long result = Interlocked.Exchange(ref location, 20L);
            Assert.Equal(20L, location);
            Assert.Equal(10L, result);
        }

        [UnitTest]
        public void Exchange_Float()
        {
            float location = 1.0f;
            float result = Interlocked.Exchange(ref location, 2.0f);
            Assert.Equal(2.0f, location);
            Assert.Equal(1.0f, result);
        }

        [UnitTest]
        public void Exchange_Double()
        {
            double location = 1.0;
            double result = Interlocked.Exchange(ref location, 2.0);
            Assert.Equal(2.0, location);
            Assert.Equal(1.0, result);
        }

        [UnitTest]
        public void Exchange_Object()
        {
            object location = "initial";
            object value = "new";
            object result = Interlocked.Exchange(ref location, value);
            Assert.Equal("new", location);
            Assert.Equal("initial", result);
        }

        [UnitTest]
        public void CompareExchange_Int32()
        {
            int location = 10;
            int result = Interlocked.CompareExchange(ref location, 20, 10);
            Assert.Equal(20, location);
            Assert.Equal(10, result);

            result = Interlocked.CompareExchange(ref location, 30, 10);
            Assert.Equal(20, location);
            Assert.Equal(20, result);
        }

        [UnitTest]
        public void CompareExchange_Int64()
        {
            long location = 10L;
            long result = Interlocked.CompareExchange(ref location, 20L, 10L);
            Assert.Equal(20L, location);
            Assert.Equal(10L, result);

            result = Interlocked.CompareExchange(ref location, 30L, 10L);
            Assert.Equal(20L, location);
            Assert.Equal(20L, result);
        }

        [UnitTest]
        public void CompareExchange_Float()
        {
            float location = 1.0f;
            float result = Interlocked.CompareExchange(ref location, 2.0f, 1.0f);
            Assert.Equal(2.0f, location);
            Assert.Equal(1.0f, result);

            result = Interlocked.CompareExchange(ref location, 3.0f, 1.0f);
            Assert.Equal(2.0f, location);
            Assert.Equal(2.0f, result);
        }

        [UnitTest]
        public void CompareExchange_Double()
        {
            double location = 1.0;
            double result = Interlocked.CompareExchange(ref location, 2.0, 1.0);
            Assert.Equal(2.0, location);
            Assert.Equal(1.0, result);

            result = Interlocked.CompareExchange(ref location, 3.0, 1.0);
            Assert.Equal(2.0, location);
            Assert.Equal(2.0, result);
        }

        [UnitTest]
        public void CompareExchange_Object()
        {
            object location = "initial";
            object value = "new";
            object comparand = "initial";
            object result = Interlocked.CompareExchange(ref location, value, comparand);
            Assert.Equal("new", location);
            Assert.Equal("initial", result);

            result = Interlocked.CompareExchange(ref location, "another", "initial");
            Assert.Equal("new", location);
            Assert.Equal("new", result);
        }

        [UnitTest]
        public void Read_Int64()
        {
            long location = 1234567890123L;
            long result = Interlocked.Read(ref location);
            Assert.Equal(1234567890123L, result);
        }

        [UnitTest]
        public void MemoryBarrierProcessWide()
        {
            // Just ensure it doesn't crash
            Interlocked.MemoryBarrierProcessWide();
        }
    }
}
