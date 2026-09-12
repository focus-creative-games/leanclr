namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunCorlibDelegate()
        {
            LegacyTestRunner.RunType(typeof(CorlibTests.InternalCall.TC_System_Delegate));
        }
    }
}
