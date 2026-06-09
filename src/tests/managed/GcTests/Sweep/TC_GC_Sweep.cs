using System;
using System.Runtime.InteropServices;
using GcTests.Fixtures;

namespace GcTests.Sweep
{
    internal class TC_GC_Sweep : GcTestCaseBase
    {
        [UnitTest]
        [GcUnitTest]
        public void Many_small_objects_reclaimed_when_unrooted()
        {
            object[] objects = new object[128];
            GCHandle[] weaks = new GCHandle[objects.Length];
            for (int i = 0; i < objects.Length; ++i)
            {
                objects[i] = new RefContainer();
                weaks[i] = TrackWeak(objects[i]);
            }
            try
            {
                objects = null;
                FullCollect();
                for (int i = 0; i < weaks.Length; ++i)
                {
                    AssertCollected(weaks[i]);
                }
            }
            finally
            {
                for (int i = 0; i < weaks.Length; ++i)
                {
                    FreeHandle(ref weaks[i]);
                }
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Many_small_objects_survive_while_array_rooted()
        {
            object[] objects = new object[64];
            GCHandle[] weaks = new GCHandle[objects.Length];
            for (int i = 0; i < objects.Length; ++i)
            {
                objects[i] = new object();
                weaks[i] = TrackWeak(objects[i]);
            }
            GCHandle arrayPin = KeepAlive(objects);
            try
            {
                FullCollect();
                for (int i = 0; i < weaks.Length; ++i)
                {
                    AssertSurvives(weaks[i]);
                }
            }
            finally
            {
                for (int i = 0; i < weaks.Length; ++i)
                {
                    FreeHandle(ref weaks[i]);
                }
                FreeHandle(ref arrayPin);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Big_byte_array_swept_when_unrooted()
        {
            byte[] big = new byte[BigByteArrayLength];
            GCHandle weak = TrackWeak(big);
            try
            {
                big = null;
                FullCollect();
                AssertCollected(weak);
            }
            finally
            {
                FreeHandle(ref weak);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Big_byte_array_retained_while_strong_handle_rooted()
        {
            byte[] big = new byte[BigByteArrayLength];
            GCHandle strong = GCHandle.Alloc(big);
            GCHandle weak = TrackWeak(big);
            try
            {
                big = null;
                FullCollect();
                AssertSurvives(weak);
            }
            finally
            {
                FreeHandle(ref weak);
                FreeHandle(ref strong);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Mixed_small_and_big_objects_reclaimed_together()
        {
            object[] small = new object[32];
            byte[][] big = new byte[8][];
            GCHandle[] weakSmall = new GCHandle[small.Length];
            GCHandle[] weakBig = new GCHandle[big.Length];
            for (int i = 0; i < small.Length; ++i)
            {
                small[i] = new object();
                weakSmall[i] = TrackWeak(small[i]);
            }
            for (int i = 0; i < big.Length; ++i)
            {
                big[i] = new byte[BigByteArrayLength];
                weakBig[i] = TrackWeak(big[i]);
            }
            try
            {
                small = null;
                big = null;
                FullCollect();
                for (int i = 0; i < weakSmall.Length; ++i)
                {
                    AssertCollected(weakSmall[i]);
                }
                for (int i = 0; i < weakBig.Length; ++i)
                {
                    AssertCollected(weakBig[i]);
                }
            }
            finally
            {
                for (int i = 0; i < weakSmall.Length; ++i)
                {
                    FreeHandle(ref weakSmall[i]);
                }
                for (int i = 0; i < weakBig.Length; ++i)
                {
                    FreeHandle(ref weakBig[i]);
                }
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Sweep_after_static_array_root_cleared()
        {
            object element = new object();
            GcStaticRoots.staticArray = new object[] { element };
            GCHandle weak = TrackWeak(element);
            try
            {
                element = null;
                GcStaticRoots.staticArray = null;
                FullCollect();
                AssertCollected(weak);
            }
            finally
            {
                FreeHandle(ref weak);
            }
        }
    }
}
