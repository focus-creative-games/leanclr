namespace ManagedNet10.LegacyTests
{
    internal sealed class CoreApp2DirectTests
    {
        [UnitTest]
        public void ObjectIdentityAndClone()
        {
            object value = new object();
            Assert.Equal(value.GetHashCode(), test.App.Run0(value));
            Assert.Equal(typeof(object), test.App.Run1());
            Assert.Equal(20, test.App.Run2());
        }

        [UnitTest]
        public void StringConstructors()
        {
            Assert.Equal("abcde", test.App.Run3());
            Assert.Equal("bc", test.App.Run4());
            Assert.Equal("abcde", test.App.Run5());
            Assert.Equal("bc", test.App.Run6());
            Assert.Equal("abcde", test.App.Run7());
            Assert.Equal("bc", test.App.Run8());
            Assert.Equal("bc", test.App.Run9());
        }

        [UnitTest]
        public void StringInternAndLength()
        {
            Assert.Equal("abcdef", test.App.Run11());
            Assert.Equal("abc", test.App.Run12());
            Assert.Equal("abc", test.App.Run13());
            Assert.Equal(5, test.App.Run14());
        }
    }

    internal static partial class Program
    {
        public static void RunCoreApp2Direct()
        {
            LegacyTestRunner.RunType(typeof(CoreApp2DirectTests));
        }
    }
}
