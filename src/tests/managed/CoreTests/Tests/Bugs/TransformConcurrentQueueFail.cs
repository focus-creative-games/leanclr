using System.Collections.Concurrent;

namespace Tests.Bugs
{
    class TransformConcurrentQueueFail : GeneralTestCaseBase
    {
        [UnitTest]
        public void Test()
        {
            var queue = new ConcurrentQueue<int>();
            Assert.False(queue.TryDequeue(out var result));
        }
    }
}
