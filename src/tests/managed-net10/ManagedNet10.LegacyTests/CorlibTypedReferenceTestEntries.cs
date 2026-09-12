namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunCorlibTypedReference()
        {
            LegacyTestRunner.RunType(typeof(CorlibTests.InternalCall.TC_System_TypedReference));
        }

        public static void RunCorlibTypedReferenceMakeTypedReference()
        {
            LegacyTestRunner.RunMethod(
                typeof(CorlibTests.InternalCall.TC_System_TypedReference),
                "MakeTypedReference_Test");
        }

        public static void RunCorlibTypedReferenceInternalToObject()
        {
            LegacyTestRunner.RunMethod(
                typeof(CorlibTests.InternalCall.TC_System_TypedReference),
                "InternalToObject_Test");
        }
    }
}
