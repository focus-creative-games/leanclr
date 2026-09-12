namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunCorlibEnvironment()
        {
            LegacyTestRunner.RunType(typeof(CorlibTests.InternalCall.TC_System_Environment));
        }

        public static void RunCorlibEnvironmentGetExitCode()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Environment), "GetExitCode_Zero");
        }

        public static void RunCorlibEnvironmentSetExitCode()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Environment), "SetExitCode_NonZero");
        }

        public static void RunCorlibEnvironmentHasShutdownStarted()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Environment), "HasShutdownStarted_False");
        }

        public static void RunCorlibEnvironmentMachineName()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Environment), "MachineName_NotEmpty");
        }

        public static void RunCorlibEnvironmentNewLine()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Environment), "NewLine_CorrectValue");
        }

        public static void RunCorlibEnvironmentPlatform()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Environment), "GetPlatform_Unix");
        }

        public static void RunCorlibEnvironmentOSVersionString()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Environment), "GetOSVersionString_NotEmpty");
        }

        public static void RunCorlibEnvironmentUserName()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Environment), "GetUserName_NotEmpty");
        }

        public static void RunCorlibEnvironmentIs64BitOperatingSystem()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Environment), "Is64BitOperatingSystem_True");
        }

        public static void RunCorlibEnvironmentProcessorCount()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Environment), "GetProcessorCount_Positive");
        }

        public static void RunCorlibEnvironmentPageSize()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Environment), "GetPageSize_4096");
        }

        public static void RunCorlibEnvironmentGetVariableNull()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Environment), "GetEnvironmentVariable_Null");
        }

        public static void RunCorlibEnvironmentSetAndGetVariable()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Environment), "SetEnvironmentVariable_And_Get");
        }
    }
}
