using System;
using System.Threading;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Threading_Monitor : TestCaseBase
    {
        private readonly object _lock = new object();

        [CoversIcall("System.Threading.Monitor::Enter")]
        [CoversIcall("System.Threading.Monitor::Exit")]
        [UnitTest]
        public void EnterExit_AcquiresLock()
        {
            lock (_lock)
            {
                Assert.IsTrue(Monitor.IsEntered(_lock));
            }
            Assert.IsFalse(Monitor.IsEntered(_lock));
        }

        [CoversIcall("System.Threading.Monitor::try_enter_with_atomic_var")]
        [UnitTest]
        public void TryEnter_SucceedsWhenUnlocked()
        {
            bool lockTaken = false;
            try
            {
                Monitor.TryEnter(_lock, ref lockTaken);
                Assert.IsTrue(lockTaken);
            }
            finally
            {
                if (lockTaken)
                    Monitor.Exit(_lock);
            }
        }

        [CoversIcall("System.Threading.Monitor::Monitor_pulse")]
        [CoversIcall("System.Threading.Monitor::Monitor_pulse_all")]
        [CoversIcall("System.Threading.Monitor::Monitor_wait")]
        [UnitTest]
        public void WaitPulse_SignalWaitingThread()
        {
            int state = 0;
            var thread = new Thread(() =>
            {
                lock (_lock)
                {
                    state = 1;
                    Monitor.Pulse(_lock);

                    while (state == 1)
                    {
                        Monitor.Wait(_lock);
                    }

                    state = 3;
                    Monitor.Pulse(_lock);
                }
            });

            lock (_lock)
            {
                thread.Start();
                while (state == 0)
                {
                    Assert.IsTrue(Monitor.Wait(_lock, 2000));
                }

                Assert.Equal(1, state);
                state = 2;
                Monitor.Pulse(_lock);

                while (state == 2)
                {
                    Assert.IsTrue(Monitor.Wait(_lock, 2000));
                }
            }

            Assert.IsTrue(thread.Join(2000));
            Assert.Equal(3, state);
        }

        [CoversIcall("System.Threading.Monitor::Monitor_test_owner")]
        [UnitTest]
        public void IsEntered_ReflectsOwnership()
        {
            Assert.IsFalse(Monitor.IsEntered(_lock));
            lock (_lock)
            {
                Assert.IsTrue(Monitor.IsEntered(_lock));
            }
        }

    }
}
