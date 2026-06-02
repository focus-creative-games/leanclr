
using test;

namespace AOTDefs
{
    public enum MyNotInitedEnum
    {
        A,
        B,
    }
    public enum MyNotInitedEnum2
    {
        A,
        B,
    }

    public class TestNotInitedNullableType
    {
        public static object TryBoxNullable<T>(T value) where T: struct
        {
            T? a = value;
            return a;
        }

        public static void AOTBoxNotInitializedGenericStruct()
        {
            var o = TryBoxNullable<MyNotInitedEnum2>(MyNotInitedEnum2.A);
            Assert.NotNull(o);
        }
    }
}
