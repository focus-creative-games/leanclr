namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunCorlibTypeSystem()
        {
            RunCorlibEnum();
            RunCorlibType();
        }

        public static void RunCorlibEnum()
        {
            LegacyTestRunner.RunType(typeof(CorlibTests.InternalCall.TC_System_Enum));
        }

        public static void RunCorlibType()
        {
            LegacyTestRunner.RunType(typeof(CorlibTests.InternalCall.TC_System_Type));
        }

        public static void RunCorlibTypeGetTypeFromHandle()
        {
            new CorlibTests.InternalCall.TC_System_Type().GetTypeFromHandle_RoundTripsRuntimeType();
        }

        public static void RunCorlibTypeGetTypeFromHandleReferenceEquals()
        {
            System.Type expected = typeof(string);
            System.RuntimeTypeHandle handle = expected.TypeHandle;
            System.Type resolved = System.Type.GetTypeFromHandle(handle);
            Assert.IsTrue(object.ReferenceEquals(expected, resolved));
        }

        public static void RunCorlibTypeRepeatedTypeofReferenceEquals()
        {
            System.Type first = typeof(string);
            System.Type second = typeof(string);
            Assert.IsTrue(object.ReferenceEquals(first, second));
        }

        public static void RunCorlibTypeInequalityOperator()
        {
            System.Type expected = typeof(string);
            System.RuntimeTypeHandle handle = expected.TypeHandle;
            System.Type resolved = System.Type.GetTypeFromHandle(handle);
            Assert.IsFalse(expected != resolved);
        }

        public static void RunCorlibEnumGetHashCode()
        {
            new CorlibTests.InternalCall.TC_System_Enum().GetHashCode_ReturnsStableValue();
        }

        public static void RunCorlibEnumCompareTo()
        {
            new CorlibTests.InternalCall.TC_System_Enum().CompareTo_OrdersValues();
        }

        public static void RunCorlibEnumHasFlag()
        {
            new CorlibTests.InternalCall.TC_System_Enum().HasFlag_MatchesFlags();
        }

        public static void RunCorlibEnumUnderlyingType()
        {
            new CorlibTests.InternalCall.TC_System_Enum().GetUnderlyingType_ReturnsInt32();
        }

        public static void RunCorlibEnumValuesAndNames()
        {
            new CorlibTests.InternalCall.TC_System_Enum().GetValuesAndNames_ReturnAllMembers();
        }

        public static void RunCorlibEnumBox()
        {
            new CorlibTests.InternalCall.TC_System_Enum().BoxEnum_PreservesValue();
        }

        public static void RunCorlibEnumGetValue()
        {
            new CorlibTests.InternalCall.TC_System_Enum().GetValue_ReturnsUnderlyingValue();
        }
    }
}
