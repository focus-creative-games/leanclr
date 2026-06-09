using System;
using System.Runtime.InteropServices;
using GcTests.Fixtures;

namespace GcTests.Scan
{
    internal class TC_GC_Arrays : GcTestCaseBase
    {
        [UnitTest]
        [GcUnitTest]
        public void Reference_szarray_element_survives_with_array_root()
        {
            object[] array = new object[4];
            object element = new object();
            array[2] = element;
            GCHandle arrayPin = KeepAlive(array);
            GCHandle weak = TrackWeak(element);
            try
            {
                element = null;
                FullCollect();
                AssertSurvives(weak);
            }
            finally
            {
                FreeHandle(ref weak);
                FreeHandle(ref arrayPin);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Reference_szarray_element_collected_when_slot_cleared()
        {
            object[] array = new object[4];
            object element = new object();
            array[0] = element;
            GCHandle arrayPin = KeepAlive(array);
            GCHandle weak = TrackWeak(element);
            try
            {
                array[0] = null;
                element = null;
                FullCollect();
                AssertCollected(weak);
            }
            finally
            {
                FreeHandle(ref weak);
                FreeHandle(ref arrayPin);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Reference_szarray_all_null_slots_collect_array()
        {
            object[] array = new object[8];
            GCHandle weak = TrackWeak(array);
            try
            {
                array = null;
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
        public void Empty_reference_szarray_collects()
        {
            object[] array = new object[0];
            GCHandle weak = TrackWeak(array);
            try
            {
                array = null;
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
        public void Int_szarray_does_not_mark_unrelated_objects()
        {
            int[] array = new int[] { 1, 2, 3, 4 };
            object orphan = new object();
            GCHandle weak = TrackWeak(orphan);
            try
            {
                array = null;
                orphan = null;
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
        public void Struct_szarray_with_reference_payload_is_scanned()
        {
            NestedRefs[] array = new NestedRefs[3];
            object element = new object();
            array[1].leaf.payload = element;
            GCHandle arrayPin = KeepAlive(array);
            GCHandle weak = TrackWeak(element);
            try
            {
                element = null;
                FullCollect();
                AssertSurvives(weak);
            }
            finally
            {
                FreeHandle(ref weak);
                FreeHandle(ref arrayPin);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Struct_szarray_element_payload_cleared_then_collected()
        {
            NestedRefs[] array = new NestedRefs[2];
            object element = new object();
            array[0].leaf.payload = element;
            GCHandle arrayPin = KeepAlive(array);
            GCHandle weak = TrackWeak(element);
            try
            {
                array[0].leaf.payload = null;
                element = null;
                FullCollect();
                AssertCollected(weak);
            }
            finally
            {
                FreeHandle(ref weak);
                FreeHandle(ref arrayPin);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Large_reference_szarray_marks_all_non_null_slots()
        {
            object[] array = new object[64];
            object[] children = new object[64];
            for (int i = 0; i < children.Length; ++i)
            {
                children[i] = new object();
                array[i] = children[i];
            }
            GCHandle arrayPin = KeepAlive(array);
            GCHandle[] weaks = new GCHandle[children.Length];
            for (int i = 0; i < children.Length; ++i)
            {
                weaks[i] = TrackWeak(children[i]);
                children[i] = null;
            }
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
        public void Jagged_object_array_marks_inner_arrays()
        {
            object[][] jagged = new object[3][];
            object innerElement = new object();
            jagged[1] = new object[] { innerElement, null };
            GCHandle jaggedPin = KeepAlive(jagged);
            GCHandle weak = TrackWeak(innerElement);
            try
            {
                innerElement = null;
                FullCollect();
                AssertSurvives(weak);
            }
            finally
            {
                FreeHandle(ref weak);
                FreeHandle(ref jaggedPin);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Multidimensional_object_array_marks_elements()
        {
            object[,] matrix = new object[2, 3];
            object element = new object();
            matrix[1, 2] = element;
            GCHandle matrixPin = KeepAlive(matrix);
            GCHandle weak = TrackWeak(element);
            try
            {
                element = null;
                FullCollect();
                AssertSurvives(weak);
            }
            finally
            {
                FreeHandle(ref weak);
                FreeHandle(ref matrixPin);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Multidimensional_object_array_element_collected_when_unrooted()
        {
            object[,] matrix = new object[2, 2];
            object element = new object();
            matrix[0, 1] = element;
            GCHandle matrixPin = KeepAlive(matrix);
            GCHandle weak = TrackWeak(element);
            try
            {
                matrix[0, 1] = null;
                element = null;
                FullCollect();
                AssertCollected(weak);
            }
            finally
            {
                FreeHandle(ref weak);
                FreeHandle(ref matrixPin);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Byte_array_big_object_survives_while_pinned()
        {
            byte[] big = new byte[BigByteArrayLength];
            GCHandle pin = KeepAlive(big);
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
                FreeHandle(ref pin);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Byte_array_big_object_collected_when_unrooted()
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
    }
}
