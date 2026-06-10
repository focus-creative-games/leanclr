using System;
using System.Runtime.InteropServices;
using GcTests.Fixtures;

namespace GcTests.Scan
{
    /// <summary>
    /// Arrays whose static element type is not sealed must still scan each slot:
    /// runtime instances may be derived types that carry reference fields invisible
    /// to the array type's static has_references metadata.
    /// </summary>
    internal class TC_GC_ArrayNonSealedElement : GcTestCaseBase
    {
        [UnitTest]
        [GcUnitTest]
        public void Non_sealed_base_szarray_marks_derived_element_reference()
        {
            GcNoRefOpenBase[] array = new GcNoRefOpenBase[2];
            object child = new object();
            array[1] = new GcNoRefOpenDerived { marker = 7, payload = child };
            GCHandle arrayPin = KeepAlive(array);
            GCHandle weak = TrackWeak(child);
            try
            {
                child = null;
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
        public void Non_sealed_base_szarray_collects_derived_child_when_slot_cleared()
        {
            GcNoRefOpenBase[] array = new GcNoRefOpenBase[2];
            object child = new object();
            array[0] = new GcNoRefOpenDerived { payload = child };
            GCHandle arrayPin = KeepAlive(array);
            GCHandle weak = TrackWeak(child);
            try
            {
                array[0] = null;
                child = null;
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
        public void Non_sealed_interface_szarray_marks_implementor_reference()
        {
            IGcOpenMarker[] array = new IGcOpenMarker[2];
            object child = new object();
            array[0] = new GcOpenMarkerImpl { Marker = 3, payload = child };
            GCHandle arrayPin = KeepAlive(array);
            GCHandle weak = TrackWeak(child);
            try
            {
                child = null;
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
        public void Non_sealed_base_multidim_array_marks_derived_element_reference()
        {
            GcNoRefOpenBase[,] matrix = new GcNoRefOpenBase[2, 2];
            object child = new object();
            matrix[1, 0] = new GcNoRefOpenDerived { payload = child };
            GCHandle matrixPin = KeepAlive(matrix);
            GCHandle weak = TrackWeak(child);
            try
            {
                child = null;
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
        public void Sealed_primitive_struct_szarray_does_not_reach_unrelated_object()
        {
            PrimitiveOnlyStruct[] array = new PrimitiveOnlyStruct[4];
            for (int i = 0; i < array.Length; ++i)
            {
                array[i].a = i;
                array[i].b = i * 2;
                array[i].c = i * 0.25;
            }
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
    }
}
