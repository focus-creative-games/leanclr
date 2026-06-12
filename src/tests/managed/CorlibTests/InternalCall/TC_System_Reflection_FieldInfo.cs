using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Threading.Tasks.Sources;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Reflection_FieldInfo : TestCaseBase
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

        public struct StructWithFields
        {
            public int i;
            public long l;
            public bool b;
            public string s;
            public static int staticField;
            private int _privateField;

            public StructWithFields(int i, long l, bool b, string s, int privateField)
            {
                this.i = i;
                this.l = l;
                this.b = b;
                this.s = s;
                _privateField = privateField;
            }

            public int PrivateField
            {
                get { return _privateField; }
            }
        }

        public struct NestedStructContainer
        {
            public StructWithFields inner;

            public NestedStructContainer(int value)
            {
                inner = new StructWithFields(value, value * 2L, value % 2 == 0, value.ToString(), value + 100);
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

        [CoversIcall("System.Array::GetValue(System.Int32[])")]
        [UnitTest]
        public void GetValue()
        {
            var a = new A() { value = 2 };
            var f = typeof(A).GetField("value");
            object v = f.GetValue(a);
            Assert.Equal(2, v);
        }

        [CoversIcall("System.Array::SetValue(System.Object,System.Int32[])")]
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

        [CoversIcall("System.Reflection.RuntimeConstructorInfo::get_metadata_token")]
        [CoversIcall("System.Reflection.RuntimeEventInfo::get_metadata_token(System.Reflection.RuntimeEventInfo)")]
        [CoversIcall("System.Reflection.RuntimeMethodInfo::get_metadata_token(System.Reflection.RuntimeMethodInfo)")]
        [CoversIcall("System.Reflection.RuntimeModule::get_MetadataToken(System.Reflection.Module)")]
        [CoversIcall("System.Reflection.RuntimeParameterInfo::GetMetadataToken()")]
        [CoversIcall("System.Reflection.RuntimePropertyInfo::get_metadata_token(System.Reflection.RuntimePropertyInfo)")]
        [CoversIcall("System.RuntimeTypeHandle::GetMetadataToken(System.RuntimeType)")]
        [UnitTest]
        public void GetMetadataToken()
        {
            var f = typeof(A).GetField("value");
            int token = f.MetadataToken;
            Assert.True(token != 0);
        }

        [CoversIcall("System.Reflection.RuntimeFieldInfo::GetTypeModifiers(System.Boolean)")]
        [CoversIcall("System.Reflection.RuntimeParameterInfo::GetTypeModifiers(System.Type,System.Reflection.MemberInfo,System.Int32,System.Boolean)")]
        [CoversIcall("System.Reflection.RuntimePropertyInfo::GetTypeModifiers(System.Reflection.RuntimePropertyInfo,System.Boolean)")]
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
        public void StructField_DeclaringType_IsValueType()
        {
            FieldInfo field = typeof(StructWithFields).GetField("i");
            Assert.NotNull(field);
            Assert.IsTrue(field.DeclaringType.IsValueType);
            Assert.Equal(typeof(StructWithFields), field.DeclaringType);
        }

        [UnitTest]
        public void StructField_FieldType()
        {
            Assert.Equal(typeof(int), typeof(StructWithFields).GetField("i").FieldType);
            Assert.Equal(typeof(long), typeof(StructWithFields).GetField("l").FieldType);
            Assert.Equal(typeof(bool), typeof(StructWithFields).GetField("b").FieldType);
            Assert.Equal(typeof(string), typeof(StructWithFields).GetField("s").FieldType);
        }

        [UnitTest]
        public void StructField_GetValue_Int()
        {
            var s = new StructWithFields(42, 100L, true, "hello", 7);
            FieldInfo field = typeof(StructWithFields).GetField("i");
            Assert.Equal(42, field.GetValue(s));
        }

        [UnitTest]
        public void StructField_GetValue_AllPublicFields()
        {
            var s = new StructWithFields(1, 2L, false, "x", 3);
            Assert.Equal(1, typeof(StructWithFields).GetField("i").GetValue(s));
            Assert.Equal(2L, typeof(StructWithFields).GetField("l").GetValue(s));
            Assert.Equal(false, typeof(StructWithFields).GetField("b").GetValue(s));
            Assert.Equal("x", typeof(StructWithFields).GetField("s").GetValue(s));
        }

        [UnitTest]
        public void StructField_GetValue_PrivateField()
        {
            var s = new StructWithFields(0, 0L, false, null, 99);
            FieldInfo field = typeof(StructWithFields).GetField("_privateField", BindingFlags.Instance | BindingFlags.NonPublic);
            Assert.NotNull(field);
            Assert.Equal(99, field.GetValue(s));
        }

        [UnitTest]
        public void StructField_SetValue_RequiresBoxedTarget()
        {
            var s = new StructWithFields(10, 20L, false, "before", 1);
            FieldInfo field = typeof(StructWithFields).GetField("i");

            // SetValue on an unboxed struct does not mutate the caller's copy.
            field.SetValue(s, 99);
            Assert.Equal(10, s.i);

            object boxed = s;
            field.SetValue(boxed, 99);
            s = (StructWithFields)boxed;
            Assert.Equal(99, s.i);
        }

        [UnitTest]
        public void StructField_SetValue_MultipleFields_ViaBoxing()
        {
            var s = new StructWithFields(1, 2L, false, "a", 0);
            object boxed = s;

            typeof(StructWithFields).GetField("i").SetValue(boxed, 10);
            typeof(StructWithFields).GetField("l").SetValue(boxed, 20L);
            typeof(StructWithFields).GetField("b").SetValue(boxed, true);
            typeof(StructWithFields).GetField("s").SetValue(boxed, "updated");

            s = (StructWithFields)boxed;
            Assert.Equal(10, s.i);
            Assert.Equal(20L, s.l);
            Assert.IsTrue(s.b);
            Assert.Equal("updated", s.s);
        }

        [UnitTest]
        public void StructField_SetValue_PrivateField_ViaBoxing()
        {
            var s = new StructWithFields(0, 0L, false, null, 1);
            FieldInfo field = typeof(StructWithFields).GetField("_privateField", BindingFlags.Instance | BindingFlags.NonPublic);
            object boxed = s;
            field.SetValue(boxed, 55);
            s = (StructWithFields)boxed;
            Assert.Equal(55, s.PrivateField);
        }

        [CoversIcall("Interop/Sys::Stat(System.Byte&,Interop/Sys/FileStatus&)")]
        [CoversIcall("Interop/Sys::Stat(System.String,Interop/Sys/FileStatus&)")]
        [UnitTest]
        public void StructField_Static_GetSet()
        {
            StructWithFields.staticField = 0;
            FieldInfo field = typeof(StructWithFields).GetField("staticField", BindingFlags.Public | BindingFlags.Static);
            Assert.NotNull(field);
            Assert.Equal(0, field.GetValue(null));

            field.SetValue(null, 123);
            Assert.Equal(123, StructWithFields.staticField);
            Assert.Equal(123, field.GetValue(null));
        }

        [UnitTest]
        public void StructField_NestedStructField_GetValue()
        {
            var container = new NestedStructContainer(5);
            FieldInfo innerField = typeof(NestedStructContainer).GetField("inner");
            Assert.NotNull(innerField);

            object inner = innerField.GetValue(container);
            Assert.NotNull(inner);
            Assert.Equal(typeof(StructWithFields), inner.GetType());

            var innerStruct = (StructWithFields)inner;
            Assert.Equal(5, innerStruct.i);
            Assert.Equal(10L, innerStruct.l);
            Assert.Equal("5", innerStruct.s);
        }

        [UnitTest]
        public void StructField_NestedStructField_SetValue_ViaBoxing()
        {
            var container = new NestedStructContainer(3);
            FieldInfo innerField = typeof(NestedStructContainer).GetField("inner");
            object boxedContainer = container;

            var newInner = new StructWithFields(100, 200L, true, "nested", 0);
            innerField.SetValue(boxedContainer, newInner);
            container = (NestedStructContainer)boxedContainer;

            Assert.Equal(100, container.inner.i);
            Assert.Equal(200L, container.inner.l);
            Assert.IsTrue(container.inner.b);
            Assert.Equal("nested", container.inner.s);
        }

        [UnitTest]
        public void StructField_GetFields_ReturnsInstanceFields()
        {
            FieldInfo[] fields = typeof(StructWithFields).GetFields(BindingFlags.Public | BindingFlags.Instance);
            Assert.True(fields.Length >= 4);
            Assert.NotNull(Array.Find(fields, f => f.Name == "i"));
            Assert.NotNull(Array.Find(fields, f => f.Name == "l"));
            Assert.NotNull(Array.Find(fields, f => f.Name == "b"));
            Assert.NotNull(Array.Find(fields, f => f.Name == "s"));
        }

        [UnitTest]
        public void ClassField_GetValue_Object()
        {
            var a = new A { value = 5, value2 = 6L };
            FieldInfo valueField = typeof(A).GetField("value");
            FieldInfo value2Field = typeof(A).GetField("value2");

            Assert.Equal(5, valueField.GetValue(a));
            Assert.Equal(6L, value2Field.GetValue(a));
        }

        [UnitTest]
        public void ClassField_SetValue_WithInvokeAttrBinderCulture()
        {
            var a = new A();
            FieldInfo field = typeof(A).GetField("value");

            field.SetValue(a, 42, BindingFlags.Default, null, CultureInfo.InvariantCulture);
            Assert.Equal(42, a.value);

            field.SetValue(a, 7, BindingFlags.SetField, null, null);
            Assert.Equal(7, a.value);
        }

        [UnitTest]
        public void StructField_SetValue_WithInvokeAttrBinderCulture_ViaBoxing()
        {
            var s = new StructWithFields(1, 2L, false, "a", 0);
            object boxed = s;
            FieldInfo field = typeof(StructWithFields).GetField("i");

            field.SetValue(boxed, 33, BindingFlags.Default, null, CultureInfo.InvariantCulture);
            s = (StructWithFields)boxed;
            Assert.Equal(33, s.i);
        }

        [UnitTest]
        public unsafe void StructField_GetValueDirect_Int()
        {
            var s = new StructWithFields(77, 0L, false, null, 0);
            FieldInfo field = typeof(StructWithFields).GetField("i");
            TypedReference tr = __makeref(s);

            Assert.Equal(77, field.GetValueDirect(tr));
        }

        [UnitTest]
        public unsafe void StructField_GetValueDirect_AllPrimitivePublicFields()
        {
            var s = new StructWithFields(1, 2L, true, "hello", 0);
            TypedReference tr = __makeref(s);

            Assert.Equal(1, typeof(StructWithFields).GetField("i").GetValueDirect(tr));
            Assert.Equal(2L, typeof(StructWithFields).GetField("l").GetValueDirect(tr));
            Assert.Equal(true, typeof(StructWithFields).GetField("b").GetValueDirect(tr));
            Assert.Equal("hello", typeof(StructWithFields).GetField("s").GetValueDirect(tr));
        }

        [UnitTest]
        public unsafe void StructField_GetValueDirect_PrivateField()
        {
            var s = new StructWithFields(0, 0L, false, null, 99);
            FieldInfo field = typeof(StructWithFields).GetField("_privateField", BindingFlags.Instance | BindingFlags.NonPublic);
            TypedReference tr = __makeref(s);

            Assert.Equal(99, field.GetValueDirect(tr));
        }

        [UnitTest]
        public unsafe void StructField_SetValueDirect_Int_MutatesUnboxedStruct()
        {
            var s = new StructWithFields(1, 0L, false, null, 0);
            FieldInfo field = typeof(StructWithFields).GetField("i");
            TypedReference tr = __makeref(s);

            field.SetValueDirect(tr, 88);

            Assert.Equal(88, s.i);
        }

        [UnitTest]
        public unsafe void StructField_SetValueDirect_String_MutatesUnboxedStruct()
        {
            var s = new StructWithFields(0, 0L, false, "old", 0);
            FieldInfo field = typeof(StructWithFields).GetField("s");
            TypedReference tr = __makeref(s);

            field.SetValueDirect(tr, "new");

            Assert.Equal("new", s.s);
        }

        [UnitTest]
        public unsafe void StructField_SetValueDirect_MultipleFields()
        {
            var s = new StructWithFields(0, 0L, false, "a", 0);
            TypedReference tr = __makeref(s);

            typeof(StructWithFields).GetField("i").SetValueDirect(tr, 10);
            typeof(StructWithFields).GetField("l").SetValueDirect(tr, 20L);
            typeof(StructWithFields).GetField("b").SetValueDirect(tr, true);
            typeof(StructWithFields).GetField("s").SetValueDirect(tr, "updated");

            Assert.Equal(10, s.i);
            Assert.Equal(20L, s.l);
            Assert.IsTrue(s.b);
            Assert.Equal("updated", s.s);
        }

        [UnitTest]
        public unsafe void StructField_SetValueDirect_PrivateField()
        {
            var s = new StructWithFields(0, 0L, false, null, 1);
            FieldInfo field = typeof(StructWithFields).GetField("_privateField", BindingFlags.Instance | BindingFlags.NonPublic);
            TypedReference tr = __makeref(s);

            field.SetValueDirect(tr, 55);

            Assert.Equal(55, s.PrivateField);
        }

        [UnitTest]
        public unsafe void StructField_SetValueDirect_DiffersFromSetValueOnUnboxedCopy()
        {
            var s = new StructWithFields(10, 0L, false, null, 0);
            FieldInfo field = typeof(StructWithFields).GetField("i");

            field.SetValue(s, 99);
            Assert.Equal(10, s.i);

            TypedReference tr = __makeref(s);
            field.SetValueDirect(tr, 99);
            Assert.Equal(99, s.i);
        }

        [UnitTest]
        public unsafe void ClassField_GetValueDirect()
        {
            var a = new A { value = 1 };
            FieldInfo field = typeof(A).GetField("value");
            TypedReference tr = __makeref(a);
            object value = field.GetValueDirect(tr);
            Assert.Equal(1, value);
        }

        [UnitTest]
        public unsafe void ClassField_SetValueDirect()
        {
            var a = new A { value = 1 };
            FieldInfo field = typeof(A).GetField("value");
            TypedReference tr = __makeref(a);
            field.SetValueDirect(tr, 2);
            Assert.Equal(2, a.value);
        }

#if IL2CPP_ONLY
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
#endif
    }
}
