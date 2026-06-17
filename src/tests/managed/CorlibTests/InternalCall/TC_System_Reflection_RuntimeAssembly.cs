using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Reflection_RuntimeAssembly : TestCaseBase
    {
        [CoversIcall("System.Reflection.RuntimeAssembly::get_fullname(System.Reflection.Assembly)")]
        [CoversIcall("System.RuntimeType::getFullName")]
        [UnitTest]
        public void GetFullName()
        {
            var assembly = typeof(TC_System_Reflection_RuntimeAssembly).Assembly;
            var fullName = assembly.FullName;
            Assert.True(fullName.StartsWith("CorlibTests, Version="));
        }

        [UnitTest]
        public void GetRuntimeVersion()
        {
            var assembly = typeof(TC_System_Reflection_RuntimeAssembly).Assembly;
            var runtimeVersion = assembly.ImageRuntimeVersion;
            Assert.Equal("v4.0.30319", runtimeVersion);
        }

        [CoversIcall("System.Reflection.RuntimeAssembly::get_global_assembly_cache")]
        [UnitTest]
        public void GetGlobalAssemblyCache()
        {
            var assembly = typeof(TC_System_Reflection_RuntimeAssembly).Assembly;
            var gac = assembly.GlobalAssemblyCache;
            Assert.False(gac);
        }

        [CoversIcall("System.Reflection.RuntimeAssembly::get_EntryPoint")]
        [UnitTest]
        public void GetEntryPoint()
        {
            var assembly = typeof(TC_System_Reflection_RuntimeAssembly).Assembly;
            var entryPoint = assembly.EntryPoint;
            Assert.Null(entryPoint);
        }

        [CoversIcall("System.Reflection.RuntimeAssembly::get_ReflectionOnly")]
        [UnitTest]
        public void GetReflectionOnly()
        {
            var assembly = typeof(TC_System_Reflection_RuntimeAssembly).Assembly;
            var reflectionOnly = assembly.ReflectionOnly;
            Assert.False(reflectionOnly);
        }

        [CoversIcall("System.Reflection.RuntimeAssembly::GetManifestResourceNames")]
        [UnitTest]
        public void GetManifestResourceNames()
        {
            var assembly = typeof(TC_System_Reflection_RuntimeAssembly).Assembly;
            var resourceNames = assembly.GetManifestResourceNames();
            Assert.Equal(0, resourceNames.Length);
        }

        [UnitTest]
        public void GetManifestModule()
        {
            var assembly = typeof(TC_System_Reflection_RuntimeAssembly).Assembly;
            var manifestModule = assembly.ManifestModule;
            Assert.Equal("CorlibTests.dll", manifestModule.Name);
        }


    }
}
