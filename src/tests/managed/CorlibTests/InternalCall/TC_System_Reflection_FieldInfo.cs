using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Threading.Tasks.Sources;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Reflection_FieldInfo : GeneralTestCaseBase
    {
        public class A
        {
            public int value;
            public long value2;

            public const int constValue = 10;

            public A()
            {
                value = 1;
                value2 = 2;
            }

            public int GetValue()
            {
                return value;
            }
        }

        public class MarshalA
        {
            [MarshalAs(UnmanagedType.I4)]
            public int value;


            public MarshalA()
            {
                value = 1;
            }
        }

        [UnitTest]
        public void FieldInfo_Name_ok()
        {
            var fieldInfo = typeof(A).GetField("value");
            Assert.NotNull(fieldInfo);
            Assert.Equal("value", fieldInfo.Name);
        }

        [UnitTest]
        public void ResolveType()
        {
            var fieldInfo = typeof(A).GetField("value");
            Assert.NotNull(fieldInfo);
            Assert.Equal(typeof(int), fieldInfo.FieldType);
        }

        [UnitTest]
        public void GetParentType()
        {
            var fieldInfo = typeof(A).GetField("value");
            var parentType = fieldInfo.DeclaringType;
            Assert.NotNull(parentType);
            Assert.Equal(typeof(A), parentType);
        }

        [UnitTest]
        public void GetValue()
        {
            var a = new A() { value = 2 };
            var f = typeof(A).GetField("value");
            object v = f.GetValue(a);
            Assert.Equal(2, v);
        }

        [UnitTest]
        public void SetValue()
        {
            var a = new A();
            var f = typeof(A).GetField("value");
            f.SetValue(a, 3);
            Assert.Equal(3, a.value);
        }

        [UnitTest]
        public void GetRawConstantValue()
        {
            var f = typeof(A).GetField("constValue");
            object v = f.GetRawConstantValue();
            Assert.Equal(10, v);
        }

        [UnitTest]
        public void GetMetadataToken()
        {
            var f = typeof(A).GetField("value");
            int token = f.MetadataToken;
            Assert.True(token != 0);
        }

        [UnitTest]
        public void GetTypeModifiers()
        {
            var f = typeof(A).GetField("value");
            var modifiers = f.GetOptionalCustomModifiers();
            Assert.Equal(0, modifiers.Length);
            modifiers = f.GetRequiredCustomModifiers();
            Assert.Equal(0, modifiers.Length);
        }

        [UnitTest]
        public void GetFieldOffset()
        {
            var f = typeof(A).GetField("value2");
            int offset = System.Runtime.InteropServices.Marshal.OffsetOf(typeof(A), "value2").ToInt32();
            Assert.Equal(8, offset);
        }

        [UnitTest]
        public void GetMarshalInfo_WithoutMarshalAs_ReturnsNull()
        {
            var method = typeof(FieldInfo).GetMethod("get_marshal_info", BindingFlags.Instance | BindingFlags.NonPublic);
            Assert.NotNull(method);

            var field = typeof(A).GetField("value");
            Assert.NotNull(field);

            object marshalInfo = method.Invoke(field, null);
            Assert.Null(marshalInfo);
        }

        [UnitTest]
        public void GetMarshalInfo_WithMarshalAs_ReturnsMarshalAsAttribute()
        {
            var method = typeof(FieldInfo).GetMethod("get_marshal_info", BindingFlags.Instance | BindingFlags.NonPublic);
            Assert.NotNull(method);

            var field = typeof(MarshalA).GetField("value");
            Assert.NotNull(field);

            object marshalInfo = method.Invoke(field, null);
            Assert.NotNull(marshalInfo);
            Assert.Equal(typeof(MarshalAsAttribute), marshalInfo.GetType());
            Assert.Equal(UnmanagedType.I4, ((MarshalAsAttribute)marshalInfo).Value);
        }
    }
}
