using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Bugs
{
    public class ReadRuntimeHandleFromMemberRef : GeneralTestCaseBase
    {

        [UnitTest]
        public void GenericMemberRef()
        {
            FindByID<MyClass>("hello");
        }

        public class MyClass
        {
            public string ID;
        }

        public static void FindByID<T>(string id) where T : MyClass
        {
            Expression<Func<T, bool>> exp = x => x.ID == id;
        }
    }
}
