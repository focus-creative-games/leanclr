using GcTests.Collection;
using GcTests.Finalizer;
using GcTests.Handles;
using GcTests.Roots;
using GcTests.Scan;
using GcTests.Sweep;

namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunGcRootsAndHandles()
        {
            RunGcRoots();
            RunGcHandles();
        }

        public static void RunGcCollection()
        {
            LegacyTestRunner.RunType(typeof(TC_GC_Collection));
        }

        public static void RunGcRoots()
        {
            LegacyTestRunner.RunType(typeof(TC_GC_Roots));
            RunGcStaticBitmapBitZero();
        }

        public static void RunGcStaticBitmapBitZero()
        {
            LegacyTestRunner.RunType(typeof(TC_GC_StaticBitmapBitZero));
        }

        public static void RunGcHandles()
        {
            LegacyTestRunner.RunType(typeof(TC_GC_Handles));
        }

        public static void RunGcScanGraph()
        {
            LegacyTestRunner.RunType(typeof(TC_GC_Graph));
        }

        public static void RunGcScanArrays()
        {
            LegacyTestRunner.RunType(typeof(TC_GC_Arrays));
        }

        public static void RunGcScanArrayNonSealedElement()
        {
            LegacyTestRunner.RunType(typeof(TC_GC_ArrayNonSealedElement));
        }

        public static void RunGcScanInstanceFields()
        {
            LegacyTestRunner.RunType(typeof(TC_GC_InstanceFields));
        }

        public static void RunGcScanValueTypes()
        {
            LegacyTestRunner.RunType(typeof(TC_GC_ValueTypes));
        }

        public static void RunGcSweep()
        {
            LegacyTestRunner.RunType(typeof(TC_GC_Sweep));
        }

        public static void RunGcFinalizer()
        {
            LegacyTestRunner.RunType(typeof(TC_GC_Finalizer));
        }
    }
}
