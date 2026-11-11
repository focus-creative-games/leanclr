using System.Reflection;

namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunCorlibReflectionAssemblyName()
        {
            LegacyTestRunner.RunTypeWithNet10Replacements(typeof(CorlibTests.InternalCall.TC_System_Reflection_AssemblyName));
            LegacyTestRunner.RunType(typeof(CorlibReflectionAssemblyNameNet10Semantics));
        }
    }

    internal sealed class CorlibReflectionAssemblyNameNet10Semantics
    {
        [UnitTest]
        public void GetNameReturnsCurrentNet10AssemblyName()
        {
            AssemblyName name = typeof(Program).Assembly.GetName();
            Assert.Equal("ManagedNet10.LegacyTests", name.Name);
        }
    }
}
