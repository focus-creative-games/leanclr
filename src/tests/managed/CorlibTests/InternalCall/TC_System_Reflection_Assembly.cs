using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.Serialization.Formatters.Binary;
using System.Security.Permissions;
using System.Text;
using System.Threading.Tasks;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Reflection_Assembly : TestCaseBase
    {
        //[UnitTest]
        //public void Assembly_FullName_ok()
        //{
        //    var assembly = typeof(TC_System_Reflection_Assembly).Assembly;
        //    Assert.NotNull(assembly);
        //    string fullName = assembly.FullName;
        //    Assert.True(fullName.Contains("CorlibTests"));
        //}

        [UnitTest]
        public void GetTypes_Ok()
        {
            var types = GetType().Assembly.GetTypes();
            Assert.True(types.Length > 10);
        }


        [UnitTest]
        public void GetExecutingAssembly_Self()
        {
            var executing_ass = Assembly.GetExecutingAssembly();
            var self_ass = GetType().Assembly;
            Assert.Equal(self_ass, executing_ass);
        }


        private void NestMethod()
        {
            var calling_ass = Assembly.GetCallingAssembly();
            var self_ass = GetType().Assembly;
            Assert.Equal(self_ass, calling_ass);
        }

        [UnitTest]
        public void GetCallingAssembly_Self()
        {
            NestMethod();
        }

        [UnitTest]
        public void GetReferenceAssemblies()
        {
            var refs = GetType().Assembly.GetReferencedAssemblies();
            Assert.True(refs.Length >= 1);
        }

        [CoversIcall("System.AppDomain::LoadAssembly(System.String,System.Security.Policy.Evidence,System.Boolean,System.Threading.StackCrawlMark&)")]
        [UnitTest]
        public void LoadAssembly()
        {
            var loaded_ass = Assembly.Load("CorlibTests");
            var self_ass = GetType().Assembly;
            Assert.Equal(self_ass, loaded_ass);
        }

        [UnitTest]
        public void LoadAssemblyFullQualifiedName()
        {
            var loaded_ass = Assembly.Load("CorlibTests, Version=1.0.0.0, Culture=neutral, PublicKeyToken=null");
            Assert.NotNull(loaded_ass);
        }

        [Serializable]
        public class LaunchOption
        {
            public string Path { get; set; }

            public Dictionary<string, string> Extra { get; set; }

            public LaunchOption GetClone()
            {
                BinaryFormatter binaryFormatter = new BinaryFormatter();
                MemoryStream memoryStream = new MemoryStream();
                binaryFormatter.Serialize(memoryStream, this);
                memoryStream.Seek(0L, SeekOrigin.Begin);
                return binaryFormatter.Deserialize(memoryStream) as LaunchOption;
            }
        }

        [UnitTest]
        public void Serialization_Call_Assembly_Load_Internal()
        {
            var option = new LaunchOption() { Path = "CorlibTests", Extra = new Dictionary<string, string>() { { "test", "test" } } };
            var serialized = option.GetClone();
            Assert.Equal(option.Path, serialized.Path);
            Assert.Equal(option.Extra["test"], serialized.Extra["test"]);
        }
    }
}
