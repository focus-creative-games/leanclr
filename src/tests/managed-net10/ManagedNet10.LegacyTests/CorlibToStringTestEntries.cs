namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunCorlibToString()
        {
            RunCorlibDoubleToString();
            RunCorlibStackTraceToString();
            RunCorlibRuntimeInformationToString();
        }

        public static void RunCorlibDoubleToString()
        {
            LegacyTestRunner.RunType(typeof(CorlibTests.InternalCall.TC_Interop_Sys));
        }

        public static void RunCorlibStackTraceToString()
        {
            LegacyTestRunner.RunMethod(
                typeof(CorlibTests.InternalCall.TC_System_Diagnostics_StackTrace),
                "StackTrace_ToString_NotEmpty");
            LegacyTestRunner.RunMethod(
                typeof(CorlibTests.InternalCall.TC_System_Diagnostics_StackTrace),
                "StackTrace_Exception_ToString");
        }

        public static void RunCorlibRuntimeInformationToString()
        {
            LegacyTestRunner.RunMethod(
                typeof(CorlibTests.InternalCall.TC_System_Runtime_InteropServices_RuntimeInformation),
                "GetRuntimeArchitecture_ReturnsWasm32");
        }
    }
}
