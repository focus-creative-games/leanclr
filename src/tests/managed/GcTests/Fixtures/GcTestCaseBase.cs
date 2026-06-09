using System;
using System.Runtime.InteropServices;

namespace GcTests.Fixtures
{
    internal abstract class GcTestCaseBase : TestCaseBase
    {
        protected const int BigByteArrayLength = 9000;

        protected static void FullCollect()
        {
            GC.Collect();
            GC.WaitForPendingFinalizers();
            GC.Collect();
        }

        protected static int s_unique_id;

        /// Managed locals are not GC roots; pin reachable graphs with a strong handle.
        protected static GCHandle KeepAlive(object target)
        {
            return GCHandle.Alloc(target);
        }

        protected static string UniqueString(string seed)
        {
            s_unique_id++;
            return seed + "-" + s_unique_id;
        }

        protected static GCHandle TrackWeak(object target)
        {
            return GCHandle.Alloc(target, GCHandleType.Weak);
        }

        protected static void AssertCollected(GCHandle weak)
        {
            Assert.Null(weak.Target);
        }

        protected static void AssertSurvives(GCHandle weak)
        {
            Assert.NotNull(weak.Target);
        }

        protected static void FreeHandle(ref GCHandle handle)
        {
            if (handle.IsAllocated)
            {
                handle.Free();
            }
            handle = default(GCHandle);
        }

        protected static void ClearStaticRoots()
        {
            GcStaticRoots.root = null;
            GcStaticRoots.staticString = null;
            GcStaticRoots.staticNested = default(NestedRefs);
            GcStaticRoots.staticDeepNested = default(DeepNestedRefs);
            GcStaticRoots.staticContainer = null;
            GcStaticRoots.staticArray = null;
        }

        protected static void CreateGarbage(int objectCount, int payloadBytes)
        {
            object[] garbage = new object[objectCount];
            for (int i = 0; i < garbage.Length; ++i)
            {
                garbage[i] = new byte[payloadBytes];
            }
        }

        protected static GcNode BuildChain(int length, object leafData)
        {
            GcNode head = null;
            GcNode tail = null;
            for (int i = 0; i < length; ++i)
            {
                var node = new GcNode();
                if (head == null)
                {
                    head = node;
                    tail = node;
                }
                else
                {
                    tail.next = node;
                    tail = node;
                }
            }
            tail.data = leafData;
            return head;
        }
    }
}
