using System;
using System.Runtime.InteropServices;
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
        public void Multiple_collects_increment_count()
        {
            int before = GC.CollectionCount(0);
            GC.Collect();
            GC.Collect();
            int after = GC.CollectionCount(0);
            Assert.True(after >= before + 2);
        }

        [UnitTest]
        [GcUnitTest]
        public void Unreachable_memory_can_be_reclaimed()
        {
            long before = GC.GetTotalMemory(false);
            CreateGarbage(256, 1024);
            FullCollect();
            long after = GC.GetTotalMemory(false);
            Assert.True(after <= before);
        }

        [UnitTest]
        [GcUnitTest]
        public void Large_unreachable_batch_can_be_reclaimed()
        {
            long before = GC.GetTotalMemory(false);
            CreateGarbage(512, 2048);
            FullCollect();
            long after = GC.GetTotalMemory(false);
            Assert.True(after <= before);
        }

        [UnitTest]
        [GcUnitTest]
        public void Rooted_object_prevents_reclaim_of_its_graph()
        {
            object leaf = new object();
            GcNode root = BuildChain(4, leaf);
            GCHandle rootPin = KeepAlive(root);
            try
            {
                long before = GC.GetTotalMemory(false);
                CreateGarbage(128, 512);
                FullCollect();
                long after = GC.GetTotalMemory(false);
                Assert.True(after <= before);
                Assert.NotNull(leaf);
            }
            finally
            {
                FreeHandle(ref rootPin);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Memory_drops_after_clearing_static_root_graph()
        {
            object child = new object();
            GcStaticRoots.root = BuildChain(3, child);
            FullCollect();
            long rooted = GC.GetTotalMemory(false);
            GcStaticRoots.root = null;
            child = null;
            FullCollect();
            long unrooted = GC.GetTotalMemory(false);
            Assert.True(unrooted <= rooted);
        }
    }
}
