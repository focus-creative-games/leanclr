namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunCorlibCoreObjectValue()
        {
            RunCorlibObjectIcalls();
            RunCorlibValueType();
            RunCorlibDateTime();
        }

        public static void RunCorlibObjectIcalls()
        {
            LegacyTestRunner.RunType(typeof(CorlibTests.InternalCall.TC_System_Object_Icalls));
        }

        public static void RunCorlibObjectGetTypeReturnsRuntimeType()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Object_Icalls), "GetType_ReturnsRuntimeType");
        }

        public static void RunCorlibObjectGetHashCodeReferenceType()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Object_Icalls), "GetHashCode_ReferenceType");
        }

        public static void RunCorlibObjectMemberwiseCloneCopiesFields()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Object_Icalls), "MemberwiseClone_CopiesFields");
        }

        public static void RunCorlibValueType()
        {
            LegacyTestRunner.RunType(typeof(CorlibTests.InternalCall.TC_System_ValueType));
        }

        public static void RunCorlibValueTypeEqualsStructValueTypes()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_ValueType), "Equals_StructValueTypes");
        }

        public static void RunCorlibValueTypeGetHashCodeStructIsStable()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_ValueType), "GetHashCode_StructIsStable");
        }

        public static void RunCorlibDateTime()
        {
            LegacyTestRunner.RunType(typeof(CorlibTests.InternalCall.TC_System_DateTime));
        }

        public static void RunCorlibDateTimeUtcNowUsesSystemTime()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_DateTime), "UtcNow_UsesSystemTime");
        }
    }
}
