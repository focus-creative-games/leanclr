namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunCorlibThreading()
        {
            RunCorlibInterlocked();
            RunCorlibVolatile();
            RunCorlibThread();
            RunCorlibOSSpecificSynchronizationContext();
            RunCorlibThreadPool();
            RunCorlibWaitHandle();
        }

        public static void RunCorlibThread()
        {
            LegacyTestRunner.RunTypeWithNet10Replacements(typeof(CorlibTests.InternalCall.TC_System_Threading_Thread));
        }

        public static void RunCorlibInterlocked()
        {
            LegacyTestRunner.RunType(typeof(CorlibTests.InternalCall.TC_System_Threading_Interlocked));
        }

        public static void RunCorlibVolatile()
        {
            LegacyTestRunner.RunType(typeof(CorlibTests.InternalCall.TC_System_Threading_Volatile));
        }

        public static void RunCorlibOSSpecificSynchronizationContext()
        {
            LegacyTestRunner.RunType(typeof(CorlibTests.InternalCall.TC_System_Threading_OSSpecificSynchronizationContext));
        }

        public static void RunCorlibThreadPool()
        {
            LegacyTestRunner.RunType(typeof(CorlibTests.InternalCall.TC_System_Threading_ThreadPool));
            LegacyTestRunner.RunType(typeof(CorlibThreadPoolNet10Semantics));
        }

        public static void RunCorlibWaitHandle()
        {
            LegacyTestRunner.RunType(typeof(CorlibTests.InternalCall.TC_System_Threading_WaitHandle));
            LegacyTestRunner.RunType(typeof(CorlibWaitHandleNet10Semantics));
        }
    }

    internal sealed class CorlibThreadPoolNet10Semantics
    {
        [UnitTest]
        public void QueueUserWorkItemRunsCallback()
        {
            using (System.Threading.ManualResetEventSlim completed = new System.Threading.ManualResetEventSlim(false))
            {
                bool queued = System.Threading.ThreadPool.QueueUserWorkItem(
                    delegate
                    {
                        completed.Set();
                    });

                Assert.Equal(true, queued);
                Assert.Equal(true, completed.Wait(System.TimeSpan.FromSeconds(5)));
            }
        }

        [UnitTest]
        public void ThreadPoolReportsPositiveWorkerCapacity()
        {
            int workerThreads;
            int completionPortThreads;
            int maxWorkerThreads;
            int maxCompletionPortThreads;

            System.Threading.ThreadPool.GetAvailableThreads(out workerThreads, out completionPortThreads);
            System.Threading.ThreadPool.GetMaxThreads(out maxWorkerThreads, out maxCompletionPortThreads);

            Assert.IsTrue(workerThreads > 0);
            Assert.IsTrue(completionPortThreads > 0);
            Assert.IsTrue(maxWorkerThreads >= workerThreads);
            Assert.IsTrue(maxCompletionPortThreads >= completionPortThreads);
        }
    }

    internal sealed class CorlibWaitHandleNet10Semantics
    {
        [UnitTest]
        public void ManualResetEventWaitOneFollowsSignalState()
        {
            using (System.Threading.ManualResetEvent waitHandle = new System.Threading.ManualResetEvent(false))
            {
                Assert.Equal(false, waitHandle.WaitOne(0));

                waitHandle.Set();
                Assert.Equal(true, waitHandle.WaitOne(0));

                waitHandle.Reset();
                Assert.Equal(false, waitHandle.WaitOne(0));
            }
        }

        [UnitTest]
        public void WaitAnyReturnsSignaledHandleIndex()
        {
            using (System.Threading.ManualResetEvent first = new System.Threading.ManualResetEvent(false))
            using (System.Threading.ManualResetEvent second = new System.Threading.ManualResetEvent(true))
            {
                int index = System.Threading.WaitHandle.WaitAny(new System.Threading.WaitHandle[] { first, second }, 0);
                Assert.Equal(1, index);
            }
        }
    }
}
