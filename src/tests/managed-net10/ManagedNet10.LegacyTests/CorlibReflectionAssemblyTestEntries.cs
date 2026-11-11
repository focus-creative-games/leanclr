using System.Reflection;

namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunCorlibReflectionAssembly()
        {
            LegacyTestRunner.RunTypeWithNet10Replacements(typeof(CorlibTests.InternalCall.TC_System_Reflection_Assembly));
            LegacyTestRunner.RunType(typeof(CorlibReflectionAssemblyNet10Semantics));
        }
    }

    internal sealed class CorlibReflectionAssemblyNet10Semantics
    {
        [UnitTest]
        public void LoadCurrentAssemblyBySimpleName()
        {
            Assembly loaded = Assembly.Load("ManagedNet10.LegacyTests");
            Assert.Equal(typeof(CorlibReflectionAssemblyNet10Semantics).Assembly, loaded);
        }

        [UnitTest]
        public void LoadCurrentAssemblyByFullName()
        {
            Assembly current = typeof(CorlibReflectionAssemblyNet10Semantics).Assembly;
            Assembly loaded = Assembly.Load(current.FullName);
            Assert.Equal(current, loaded);
        }

        [UnitTest]
        public void LoadCurrentAssemblyByAssemblyName()
        {
            Assembly current = typeof(CorlibReflectionAssemblyNet10Semantics).Assembly;
            Assembly loaded = Assembly.Load(current.GetName());
            Assert.Equal(current, loaded);
        }
    }
}
