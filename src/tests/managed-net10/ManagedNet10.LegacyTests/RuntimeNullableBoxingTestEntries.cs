namespace ManagedNet10.LegacyTests
{
    internal sealed class RuntimeNullableBoxingTests
    {
        [UnitTest]
        public void BoxGenericNullableEnum()
        {
            Tests.Fixtures.TestNotInitedNullableType.AOTBoxNotInitializedGenericStruct();
        }
    }

    internal static partial class Program
    {
        public static void RunRuntimeNotInitializedNullableBoxing()
        {
            LegacyTestRunner.RunType(typeof(RuntimeNullableBoxingTests));
        }
    }
}
