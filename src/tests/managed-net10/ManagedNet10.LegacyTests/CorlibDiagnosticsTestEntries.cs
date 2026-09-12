namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunCorlibDiagnostics()
        {
            RunCorlibDiagnosticsDebugger();
            RunCorlibDiagnosticsStackFrame();
            RunCorlibDiagnosticsStopwatch();
        }

        public static void RunCorlibDiagnosticsDebugger()
        {
            LegacyTestRunner.RunType(typeof(CorlibTests.InternalCall.TC_System_Diagnostics_Debugger));
        }

        public static void RunCorlibDiagnosticsStackFrame()
        {
            LegacyTestRunner.RunType(typeof(CorlibTests.InternalCall.TC_System_Diagnostics_StackFrame));
        }

        public static void RunCorlibDiagnosticsStopwatch()
        {
            LegacyTestRunner.RunType(typeof(CorlibTests.InternalCall.TC_Stopwatch));
        }
    }
}
