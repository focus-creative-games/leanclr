using System;
using System.Runtime.InteropServices;
using GcTests.Fixtures;

namespace GcTests.Handles
{
    internal class TC_GC_Handles : GcTestCaseBase
    {
        [UnitTest]
        [GcUnitTest]
        public void Strong_handle_keeps_alive()
        {
            object target = new object();
            GCHandle handle = GCHandle.Alloc(target);
            try
            {
                target = null;
                FullCollect();
                Assert.NotNull(handle.Target);
            }
            finally
            {
                if (handle.IsAllocated)
                {
                    handle.Free();
                }
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Weak_handle_cleared_after_collect()
        {
            object target = new object();
            GCHandle handle = GCHandle.Alloc(target, GCHandleType.Weak);
            try
            {
                target = null;
                FullCollect();
                Assert.Null(handle.Target);
            }
            finally
            {
                if (handle.IsAllocated)
                {
                    handle.Free();
                }
            }
        }
    }
}
