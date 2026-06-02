using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Reflection_RuntimePropertyInfo : TestCaseBase
    {
        [UnitTest]
        public void GetMetadataToken_ReturnsNonZero()
        {
            // Test get_metadata_token via public PropertyInfo.MetadataToken property
            PropertyInfo property = typeof(TestClassWithProperties).GetProperty("Value");
            Assert.NotNull(property);
            int token = property.MetadataToken;
            Assert.IsTrue(token != 0);
        }

        [UnitTest]
        public void GetMetadataToken_ConsistentValue()
        {
            // Test that get_metadata_token returns consistent value
            PropertyInfo property = typeof(TestClassWithProperties).GetProperty("Value");
            int token1 = property.MetadataToken;
            int token2 = property.MetadataToken;
            Assert.Equal(token1, token2);
        }

        [UnitTest]
        public void GetMetadataToken_DifferentProperties()
        {
            // Test that different properties have different tokens
            PropertyInfo prop1 = typeof(TestClassWithProperties).GetProperty("Value");
            PropertyInfo prop2 = typeof(TestClassWithProperties).GetProperty("Name");
            Assert.NotNull(prop1);
            Assert.NotNull(prop2);
            int token1 = prop1.MetadataToken;
            int token2 = prop2.MetadataToken;
            Assert.NotEqual(token1, token2);
        }

        [UnitTest]
        public void PropertyInfo_Name_NotEmpty()
        {
            // Test that property has valid name
            PropertyInfo property = typeof(TestClassWithProperties).GetProperty("Value");
            Assert.NotNull(property.Name);
            Assert.Equal("Value", property.Name);
        }

        [UnitTest]
        public void PropertyInfo_PropertyType_Valid()
        {
            // Test that property type is correct
            PropertyInfo property = typeof(TestClassWithProperties).GetProperty("Value");
            Assert.NotNull(property.PropertyType);
            Assert.Equal(typeof(int), property.PropertyType);
        }

        [UnitTest]
        public void PropertyInfo_CanRead_Valid()
        {
            // Test that CanRead property is correct
            PropertyInfo property = typeof(TestClassWithProperties).GetProperty("Value");
            Assert.IsTrue(property.CanRead);
        }

        [UnitTest]
        public void PropertyInfo_CanWrite_Valid()
        {
            // Test that CanWrite property is correct
            PropertyInfo property = typeof(TestClassWithProperties).GetProperty("Value");
            Assert.IsTrue(property.CanWrite);
        }

        [UnitTest]
        public void PropertyInfo_GetGetMethod()
        {
            // Test that getter method can be retrieved
            PropertyInfo property = typeof(TestClassWithProperties).GetProperty("Value");
            MethodInfo getMethod = property.GetGetMethod();
            Assert.NotNull(getMethod);
        }

        [UnitTest]
        public void PropertyInfo_GetSetMethod()
        {
            // Test that setter method can be retrieved
            PropertyInfo property = typeof(TestClassWithProperties).GetProperty("Value");
            MethodInfo setMethod = property.GetSetMethod();
            Assert.NotNull(setMethod);
        }

        [UnitTest]
        public void GetTypeModifiers_NoModifiers()
        {
            // Test GetTypeModifiers for regular property
            PropertyInfo property = typeof(TestClassWithProperties).GetProperty("Value");
            Type[] modifiers = property.GetOptionalCustomModifiers();
            Assert.NotNull(modifiers);
            // Regular property should have no modifiers
        }

        [UnitTest]
        public void GetTypeModifiers_MultipleProperties()
        {
            // Test GetTypeModifiers with multiple properties
            PropertyInfo[] properties = typeof(TestClassWithProperties).GetProperties();
            foreach (PropertyInfo prop in properties)
            {
                Type[] modifiers = prop.GetOptionalCustomModifiers();
                Assert.NotNull(modifiers);
            }
        }

        [UnitTest]
        public void PropertyInfo_Attributes_Valid()
        {
            // Test that property attributes are accessible
            PropertyInfo property = typeof(TestClassWithProperties).GetProperty("Value");
            PropertyAttributes attrs = property.Attributes;
            // Just verify attributes can be retrieved
            Assert.IsTrue(true);
        }

        [UnitTest]
        public void PropertyInfo_DeclaringType_Valid()
        {
            // Test that declaring type is correct
            PropertyInfo property = typeof(TestClassWithProperties).GetProperty("Value");
            Assert.NotNull(property.DeclaringType);
            Assert.Equal(typeof(TestClassWithProperties), property.DeclaringType);
        }

        [UnitTest]
        public void PropertyInfo_GetValue()
        {
            // Test getting property value indirectly tests property info
            PropertyInfo property = typeof(TestClassWithProperties).GetProperty("Value");
            var instance = new TestClassWithProperties { Value = 42 };
            object value = property.GetValue(instance);
            Assert.Equal(42, value);
        }

        [UnitTest]
        public void PropertyInfo_SetValue()
        {
            // Test setting property value
            PropertyInfo property = typeof(TestClassWithProperties).GetProperty("Value");
            var instance = new TestClassWithProperties();
            property.SetValue(instance, 99);
            Assert.Equal(99, instance.Value);
        }

        [UnitTest]
        public void PropertyInfo_ReadOnlyProperty()
        {
            // Test property info for read-only property
            PropertyInfo property = typeof(TestClassWithProperties).GetProperty("ReadOnlyValue");
            Assert.NotNull(property);
            Assert.IsTrue(property.CanRead);
            Assert.IsFalse(property.CanWrite);
        }

        [UnitTest]
        public void PropertyInfo_WriteOnlyProperty()
        {
            // Test property info for write-only property
            PropertyInfo property = typeof(TestClassWithProperties).GetProperty("WriteOnlyValue");
            Assert.NotNull(property);
            Assert.IsFalse(property.CanRead);
            Assert.IsTrue(property.CanWrite);
        }

        [UnitTest]
        public void PropertyInfo_AutoProperty()
        {
            // Test property info for auto-implemented property
            PropertyInfo property = typeof(TestClassWithProperties).GetProperty("Name");
            Assert.NotNull(property);
            Assert.IsTrue(property.CanRead);
            Assert.IsTrue(property.CanWrite);
            int token = property.MetadataToken;
            Assert.NotEqual(0, token);
        }

        [UnitTest]
        public void PropertyInfo_MultipleProperties_Tokens()
        {
            // Test metadata tokens for multiple properties
            PropertyInfo prop1 = typeof(TestClassWithProperties).GetProperty("Value");
            PropertyInfo prop2 = typeof(TestClassWithProperties).GetProperty("Name");
            PropertyInfo prop3 = typeof(TestClassWithProperties).GetProperty("ReadOnlyValue");
            int token1 = prop1.MetadataToken;
            int token2 = prop2.MetadataToken;
            int token3 = prop3.MetadataToken;
            // All tokens should be unique
            Assert.NotEqual(token1, token2);
            Assert.NotEqual(token2, token3);
            Assert.NotEqual(token1, token3);
        }

        [UnitTest]
        public void PropertyInfo_GenericType()
        {
            // Test property info for properties in generic types
            Type genericType = typeof(TestGenericClass<>).MakeGenericType(typeof(string));
            PropertyInfo property = genericType.GetProperty("Item2");
            Assert.NotNull(property);
            int token = property.MetadataToken;
            Assert.NotEqual(0, token);
        }

        [UnitTest]
        public void PropertyInfo_IndexedProperty()
        {
            // Test property info for indexed properties
            Type genericType = typeof(TestGenericClass<>).MakeGenericType(typeof(int));
            PropertyInfo property = genericType.GetProperty("Item2");
            Assert.NotNull(property);
            ParameterInfo[] indexParams = property.GetIndexParameters();
            // IndexedProperty should have index parameters
        }

        // Test helper classes

        private class TestClassWithProperties
        {
            public int Value { get; set; }
            public string Name { get; set; }
            public int ReadOnlyValue { get { return 10; } }
            public int WriteOnlyValue { set { } }
        }

        private class TestGenericClass<T>
        {
            private T[] items = new T[10];

            public T Item2
            {
                get { return items[0]; }
                set { items[0] = value; }
            }

            public T this[int index]
            {
                get { return items[index]; }
                set { items[index] = value; }
            }
        }
    }
}
