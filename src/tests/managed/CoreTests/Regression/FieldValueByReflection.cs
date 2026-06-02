using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Bugs
{
    class FieldValueByReflection : GeneralTestCaseBase
    {
        public const int X = 1000;
        public const uint Y = 1000;

        public const string S = "中国abc";

        [UnitTest]
        public void ConstInt()
        {
            FieldInfo field = typeof(FieldValueByReflection).GetField("X");
            Assert.Equal(1000, (int)field.GetValue(null));
        }

        [UnitTest]
        public void ConstUint()
        {
            FieldInfo field = typeof(FieldValueByReflection).GetField("Y");
            Assert.Equal(1000, (uint)field.GetValue(null));
        }

        [UnitTest]
        public void ConstString()
        {
            FieldInfo field = typeof(FieldValueByReflection).GetField("S");
            Assert.Equal("中国abc", (string)field.GetValue(null));
        }
    }
}
