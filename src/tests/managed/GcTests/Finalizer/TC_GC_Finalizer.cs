using System;
using System.Runtime.InteropServices;
using GcTests.Fixtures;

namespace GcTests.Finalizer
{
    internal class TC_GC_Finalizer : GcTestCaseBase
    {
        private sealed class FinalizableObject
        {
            public static int FinalizeCount;

            ~FinalizableObject()
            {
                FinalizeCount++;
            }
        }

        private static void CreateUnreachableFinalizable()
        {
            var obj = new FinalizableObject();
        }

        [UnitTest]
        [GcUnitTest]
        public void Finalizer_runs_when_object_is_collected()
        {
            FinalizableObject.FinalizeCount = 0;
            CreateUnreachableFinalizable();
            FullCollect();
            Assert.Equal(1, FinalizableObject.FinalizeCount);
        }

        [UnitTest]
        [GcUnitTest]
        public void SuppressFinalize_prevents_finalizer()
        {
            FinalizableObject.FinalizeCount = 0;
            var obj = new FinalizableObject();
            GC.SuppressFinalize(obj);
            obj = null;
            FullCollect();
            Assert.Equal(0, FinalizableObject.FinalizeCount);
        }

        [UnitTest]
        [GcUnitTest]
        public void WaitForPendingFinalizers_runs_finalizer()
        {
            FinalizableObject.FinalizeCount = 0;
            CreateUnreachableFinalizable();
            GC.Collect();
            GC.WaitForPendingFinalizers();
            Assert.Equal(1, FinalizableObject.FinalizeCount);
        }

        [UnitTest]
        [GcUnitTest]
        public void ReRegisterForFinalize_runs_finalizer_again()
        {
            FinalizableObject.FinalizeCount = 0;
            FinalizableObject obj = new FinalizableObject();
            GCHandle weak = GCHandle.Alloc(obj, GCHandleType.Weak);
            obj = null;

            GC.Collect();
            GC.WaitForPendingFinalizers();
            Assert.Equal(1, FinalizableObject.FinalizeCount);

            object resurrected = weak.Target;
            Assert.NotNull(resurrected);
            GC.ReRegisterForFinalize(resurrected);
            resurrected = null;

            GC.Collect();
            GC.WaitForPendingFinalizers();
            Assert.Equal(2, FinalizableObject.FinalizeCount);
            weak.Free();
        }

        [UnitTest]
        [GcUnitTest]
        public void Plain_object_does_not_invoke_custom_finalizer_counter()
        {
            FinalizableObject.FinalizeCount = 0;
            object obj = new object();
            obj = null;
            FullCollect();
            Assert.Equal(0, FinalizableObject.FinalizeCount);
        }
    }
}
