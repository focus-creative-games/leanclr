using System;
using GcTests.Fixtures;

namespace GcTests.Collection
{
    internal class TC_GC_Collection : GcTestCaseBase
    {
        [UnitTest]
        [GcUnitTest]
        public void Collect_increments_count()
        {
            int before = GC.CollectionCount(0);
            GC.Collect();
            int after = GC.CollectionCount(0);
            Assert.True(after > before);
        }

        [UnitTest]
        [GcUnitTest]
        public void Unreachable_memory_can_be_reclaimed()
        {
            long before = GC.GetTotalMemory(false);
            CreateGarbage();
            FullCollect();
            long after = GC.GetTotalMemory(false);
            Assert.True(after <= before);
        }

        private static void CreateGarbage()
        {
            object[] garbage = new object[256];
            for (int i = 0; i < garbage.Length; ++i)
            {
                garbage[i] = new byte[1024];
            }
        }
    }
}
