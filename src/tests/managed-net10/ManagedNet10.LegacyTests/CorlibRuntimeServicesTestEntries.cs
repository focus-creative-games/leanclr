namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunCorlibRuntimeServices()
        {
            RunCorlibMath();
            RunCorlibGC();
            RunCorlibRuntimeHelpers();
            RunCorlibGCHandle();
            RunCorlibMonitor();
        }

        public static void RunCorlibMath()
        {
            LegacyTestRunner.RunType(typeof(CorlibTests.InternalCall.TC_System_Math));
        }

        public static void RunCorlibGC()
        {
            LegacyTestRunner.RunType(typeof(CorlibTests.InternalCall.TC_System_GC));
        }

        public static void RunCorlibGCCollect1()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_GC), "Collect1");
        }

        public static void RunCorlibRuntimeHelpers()
        {
            LegacyTestRunner.RunType(typeof(CorlibTests.InternalCall.TC_System_Runtime_CompilerServices_RuntimeHelper));
        }

        public static void RunCorlibRuntimeHelpersRunClassConstructor()
        {
            LegacyTestRunner.RunMethod(
                typeof(CorlibTests.InternalCall.TC_System_Runtime_CompilerServices_RuntimeHelper),
                "RunClassConstructor_ok");
        }

        public static void RunCorlibRuntimeHelpersRunModuleConstructor()
        {
            LegacyTestRunner.RunMethod(
                typeof(CorlibTests.InternalCall.TC_System_Runtime_CompilerServices_RuntimeHelper),
                "RunModuleConstructor_ok");
        }

        public static void RunCorlibGCHandle()
        {
            LegacyTestRunner.RunType(typeof(CorlibTests.InternalCall.TC_System_Runtime_InteropServices_GCHandle));
        }

        public static void RunCorlibGCHandleAllocNormal()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Runtime_InteropServices_GCHandle), "Alloc_Normal");
        }

        public static void RunCorlibGCHandleAllocPinnedByteArray()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Runtime_InteropServices_GCHandle), "Alloc_Pinned_ByteArray");
        }

        public static void RunCorlibGCHandleAllocWeak()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Runtime_InteropServices_GCHandle), "Alloc_Weak");
        }

        public static void RunCorlibMonitor()
        {
            LegacyTestRunner.RunType(typeof(CorlibTests.InternalCall.TC_System_Threading_Monitor));
        }

        public static void RunCorlibMonitorEnterExit()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Threading_Monitor), "EnterExit_AcquiresLock");
        }

        public static void RunCorlibMonitorTryEnter()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Threading_Monitor), "TryEnter_SucceedsWhenUnlocked");
        }

        public static void RunCorlibMonitorWaitPulse()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Threading_Monitor), "WaitPulse_SignalWaitingThread");
        }

        public static void RunCorlibMonitorIsEntered()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Threading_Monitor), "IsEntered_ReflectsOwnership");
        }
    }
}
