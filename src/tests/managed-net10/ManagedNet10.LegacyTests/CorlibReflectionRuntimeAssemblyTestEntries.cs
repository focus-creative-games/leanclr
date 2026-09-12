using System.Reflection;

namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunCorlibReflectionRuntimeAssembly()
        {
            LegacyTestRunner.RunTypeWithNet10Replacements(typeof(CorlibTests.InternalCall.TC_System_Reflection_RuntimeAssembly));
            LegacyTestRunner.RunType(typeof(CorlibReflectionRuntimeAssemblyNet10Semantics));
        }
    }

    internal sealed class CorlibReflectionRuntimeAssemblyNet10Semantics
    {
        [UnitTest]
        public void FullNameUsesCurrentNet10AssemblyName()
        {
            string fullName = typeof(Program).Assembly.FullName;
            Assert.IsTrue(fullName.StartsWith("ManagedNet10.LegacyTests, Version="));
        }

        [UnitTest]
        public void EntryPointResolvesProgramMain()
        {
            MethodInfo entryPoint = typeof(Program).Assembly.EntryPoint;
            Assert.NotNull(entryPoint);
            Assert.Equal("Main", entryPoint.Name);
            Assert.Equal(typeof(Program), entryPoint.DeclaringType);
        }

        [UnitTest]
        public void ManifestModuleUsesCurrentNet10AssemblyName()
        {
            Module manifestModule = typeof(Program).Assembly.ManifestModule;
            Assert.Equal("ManagedNet10.LegacyTests.dll", manifestModule.Name);
        }
    }
}
