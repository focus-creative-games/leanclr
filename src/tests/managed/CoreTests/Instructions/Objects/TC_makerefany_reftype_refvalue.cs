using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Test;

namespace Tests.Instruments.Objs
{
    internal class TC_makerefany_reftype_refvalue : GeneralTestCaseBase
    {

        [UnitTest]
        public void ref_1()
        {
            int a = 5;
            TypedReference t = __makeref(a);
            Assert.Equal(typeof(int), __reftype(t));
            Assert.Equal(5, __refvalue(t, int));
        }

        [UnitTest]
        public void ref_2()
        {
            string a = "abc";
            TypedReference t = __makeref(a);
            Assert.Equal(typeof(string), __reftype(t));
            Assert.Equal("abc", __refvalue(t, string));
        }

        //[UnitTest]
        //public void ref_invalidcast()
        //{
        //    string a = "abc";
        //    TypedReference t = __makeref(a);
        //    Assert.ExpectException<InvalidCastException>();
        //    Assert.Equal(3, __refvalue(t, int));
        //}

        //[UnitTest]
        //public void ref_invalidcast_2()
        //{
        //    int a = 5;
        //    TypedReference t = __makeref(a);
        //    Assert.ExpectException<InvalidCastException>();
        //    Assert.Equal("5", __refvalue(t, string));
        //}
    }
}
