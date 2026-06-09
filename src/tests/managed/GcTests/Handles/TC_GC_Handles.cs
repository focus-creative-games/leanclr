using System;
using System.Runtime.InteropServices;
using GcTests.Fixtures;

namespace GcTests.Handles
{
    internal class TC_GC_Handles : GcTestCaseBase
    {
        [UnitTest]
        [GcUnitTest]
        public void Strong_handle_is_a_gc_root()
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
        public void Weak_handle_cleared_after_collect_when_unrooted()
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

        [UnitTest]
        [GcUnitTest]
        public void Weak_handle_survives_while_strong_reference_exists()
        {
            object target = new object();
            GCHandle strongPin = KeepAlive(target);
            GCHandle weak = GCHandle.Alloc(target, GCHandleType.Weak);
            try
            {
                target = null;
                FullCollect();
                Assert.NotNull(weak.Target);
            }
            finally
            {
                FreeHandle(ref weak);
                FreeHandle(ref strongPin);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Weak_handle_survives_while_strong_gchandle_exists()
        {
            object target = new object();
            GCHandle strong = GCHandle.Alloc(target);
            GCHandle weak = GCHandle.Alloc(target, GCHandleType.Weak);
            try
            {
                target = null;
                FullCollect();
                Assert.NotNull(strong.Target);
                Assert.NotNull(weak.Target);
            }
            finally
            {
                FreeHandle(ref weak);
                FreeHandle(ref strong);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Weak_handle_clears_after_strong_gchandle_freed()
        {
            object target = new object();
            GCHandle strong = GCHandle.Alloc(target);
            GCHandle weak = GCHandle.Alloc(target, GCHandleType.Weak);
            try
            {
                target = null;
                FreeHandle(ref strong);
                FullCollect();
                Assert.Null(weak.Target);
            }
            finally
            {
                FreeHandle(ref weak);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Pinned_handle_keeps_byte_array_alive()
        {
            byte[] data = new byte[] { 1, 2, 3, 4, 5 };
            GCHandle pinned = GCHandle.Alloc(data, GCHandleType.Pinned);
            GCHandle weak = TrackWeak(data);
            try
            {
                data = null;
                FullCollect();
                AssertSurvives(weak);
                Assert.NotNull(pinned.Target);
            }
            finally
            {
                FreeHandle(ref weak);
                FreeHandle(ref pinned);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Multiple_weak_handles_to_same_object_track_one_target()
        {
            object target = new object();
            GCHandle weak0 = GCHandle.Alloc(target, GCHandleType.Weak);
            GCHandle weak1 = GCHandle.Alloc(target, GCHandleType.Weak);
            try
            {
                target = null;
                FullCollect();
                Assert.Null(weak0.Target);
                Assert.Null(weak1.Target);
            }
            finally
            {
                FreeHandle(ref weak0);
                FreeHandle(ref weak1);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Strong_handle_root_marks_transitive_children()
        {
            object leaf = new object();
            GcNode root = BuildChain(3, leaf);
            GCHandle strong = GCHandle.Alloc(root);
            GCHandle weak = TrackWeak(leaf);
            try
            {
                root = null;
                leaf = null;
                FullCollect();
                AssertSurvives(weak);
            }
            finally
            {
                FreeHandle(ref weak);
                FreeHandle(ref strong);
            }
        }
    }
}
