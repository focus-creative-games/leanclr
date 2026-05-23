using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Security.Permissions;
using System.Text;
using System.Threading.Tasks;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Reflection_Assembly : GeneralTestCaseBase
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

#if IL2CPP_ONLY
        [UnitTest]
        public void GetEntryAssembly_Null()
        {
            var entry_ass = Assembly.GetEntryAssembly();
            Assert.Null(entry_ass);
        }
#endif

        [UnitTest]
        public void GetExecutingAssembly_Self()
        {
            var executing_ass = Assembly.GetExecutingAssembly();
            var self_ass = GetType().Assembly;
            Assert.Equal(self_ass, executing_ass);
        }

#if IL2CPP_ONLY
        [UnitTest]
        public void GetCallingAssembly_Top_CorLib()
        {
            var calling_ass = Assembly.GetCallingAssembly();
            var corlib_ass = typeof(int).Assembly;
            Assert.Equal(corlib_ass, calling_ass);
        }
#endif

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

        [UnitTest]
        public void LoadAssembly()
        {
            var loaded_ass = Assembly.Load("CorlibTests");
            var self_ass = GetType().Assembly;
            Assert.Equal(self_ass, loaded_ass);
        }
    }
}
