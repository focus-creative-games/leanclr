using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CorlibTests.InternalCall
{
    internal class TC_System_AppDomain : TestCaseBase
    {
        [UnitTest]
        public void CurrentDomain_NotNull()
        {
            AppDomain currentDomain = AppDomain.CurrentDomain;
            Assert.NotNull(currentDomain);
        }

        [UnitTest]
        public void GetSetup()
        {
            AppDomainSetup setup = AppDomain.CurrentDomain.SetupInformation;
            Assert.NotNull(setup);
            Assert.IsTrue(setup.GetType() == typeof(AppDomainSetup));
        }

        [UnitTest]
        public void LoadAssemblyExist_ByName()
        {
            AppDomain domain = AppDomain.CurrentDomain;
            var assembly = domain.Load("CorlibTests");
            Assert.NotNull(assembly);
        }

        //[UnitTest]
        //public void LoadAssemblyNotExist_ByName()
        //{
        //    AppDomain domain = AppDomain.CurrentDomain;
        //    var assembly = domain.Load("CorlibTests2");
        //    Assert.Null(assembly);
        //}
        [UnitTest]
        public void GetAssemblies_NotEmpty()
        {
            AppDomain domain = AppDomain.CurrentDomain;
            var assemblies = domain.GetAssemblies();
            Assert.IsTrue(assemblies.Length > 0);
        }

        [UnitTest]
        public void GetData_SetData()
        {
            AppDomain domain = AppDomain.CurrentDomain;
            string key = "TestKey";
            string value = "TestValue";
            var oldValue = domain.GetData(key);
            Assert.Null(oldValue);
            domain.SetData(key, value);
            var retrievedValue = domain.GetData(key);
            Assert.Equal(value, retrievedValue);
        }
    }
}
