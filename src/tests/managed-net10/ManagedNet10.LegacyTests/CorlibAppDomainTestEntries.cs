namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunCorlibAppDomain()
        {
            LegacyTestRunner.RunTypeWithNet10Replacements(typeof(CorlibTests.InternalCall.TC_System_AppDomain));
            LegacyTestRunner.RunType(typeof(CorlibAppDomainNet10Semantics));
        }

        public static void RunCorlibAppDomainLegacy()
        {
            LegacyTestRunner.RunTypeWithNet10Replacements(typeof(CorlibTests.InternalCall.TC_System_AppDomain));
        }

        public static void RunCorlibAppDomainNet10Semantics()
        {
            LegacyTestRunner.RunType(typeof(CorlibAppDomainNet10Semantics));
        }
    }

    internal sealed class CorlibAppDomainNet10Semantics
    {
        [UnitTest]
        public void LoadCurrentAssemblyByName()
        {
            System.AppDomain domain = System.AppDomain.CurrentDomain;
            System.Reflection.Assembly assembly = domain.Load("ManagedNet10.LegacyTests");
            Assert.NotNull(assembly);
            Assert.Equal("ManagedNet10.LegacyTests", assembly.GetName().Name);
        }
    }
}
