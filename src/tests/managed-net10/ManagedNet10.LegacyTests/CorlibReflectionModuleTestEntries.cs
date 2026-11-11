namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunCorlibReflectionRuntimeModule()
        {
            LegacyTestRunner.RunType(typeof(CorlibTests.InternalCall.TC_System_Reflection_RuntimeModule));
        }
    }
}
