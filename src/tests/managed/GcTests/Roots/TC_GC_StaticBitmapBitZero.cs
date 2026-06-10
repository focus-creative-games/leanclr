using System;
using System.Runtime.InteropServices;
using GcTests.Fixtures;

namespace GcTests.Roots
{
    /// <summary>
    /// Regression tests for static_gc_bitmap finalize when the only set bit is index 0.
    /// A buggy finalize_gc_bitmap treated max_bitmap_index == 0 as "no references" and
    /// dropped a non-empty bitmap, so static roots at layout offset 0 were never scanned.
    /// </summary>
    internal class TC_GC_StaticBitmapBitZero : GcTestCaseBase
    {
        [UnitTest]
        [GcUnitTest]
        public void Static_object_at_layout_offset_zero_marks_target()
        {
            object child = new object();
            GcStaticBitmapBitZeroRefHolder.onlyReference = child;
            GCHandle weak = TrackWeak(child);
            try
            {
                child = null;
                FullCollect();
                AssertSurvives(weak);
            }
            finally
            {
                GcStaticBitmapBitZeroRefHolder.onlyReference = null;
                FreeHandle(ref weak);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Static_value_type_at_layout_offset_zero_marks_first_nested_ref()
        {
            object child = new object();
            GcStaticBitmapBitZeroValueHolder.valueAtZero.payload = child;
            GCHandle weak = TrackWeak(child);
            try
            {
                child = null;
                FullCollect();
                AssertSurvives(weak);
            }
            finally
            {
                GcStaticBitmapBitZeroValueHolder.valueAtZero = default(StaticValueWithRefAtLayoutZero);
                FreeHandle(ref weak);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Clearing_static_object_at_layout_offset_zero_allows_collection()
        {
            object child = new object();
            GcStaticBitmapBitZeroRefHolder.onlyReference = child;
            GCHandle weak = TrackWeak(child);
            try
            {
                child = null;
                GcStaticBitmapBitZeroRefHolder.onlyReference = null;
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
