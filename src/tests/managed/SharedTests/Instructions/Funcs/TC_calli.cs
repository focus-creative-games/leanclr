using System;

namespace Tests.Instruments.Funcs
{
    internal unsafe class TC_calli : TestCaseBase
    {
        private static int Increment(int value) { return value + 1; }
        private static T Identity<T>(T value) { return value; }
        private static void Change(ref int value) { value = 42; }
        private static int effects;
        private static void ThrowTarget() { effects++; throw new InvalidOperationException(); }

        [UnitTest]
        public void managed_scalar()
        {
            delegate* managed<int, int> pointer = &Increment;
            Assert.Equal(42, pointer(41));
        }

        [UnitTest]
        public void managed_generic_and_byref()
        {
            delegate* managed<long, long> identity = &Identity<long>;
            Assert.Equal(long.MinValue, identity(long.MinValue));
            delegate* managed<ref int, void> change = &Change;
            int value = 1;
            change(ref value);
            Assert.Equal(42, value);
        }

        [UnitTest]
        public void managed_exception_does_not_replay()
        {
            effects = 0;
            delegate* managed<void> pointer = &ThrowTarget;
            bool caught = false;
            try { effects++; pointer(); }
            catch (InvalidOperationException) { caught = true; }
            Assert.IsTrue(caught);
            Assert.Equal(2, effects);
        }
    }
}
