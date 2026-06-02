using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Reflection_RuntimeConstructorInfo : TestCaseBase
    {
        [UnitTest]
        public void GetMetadataToken_ReturnsNonZero()
        {
            // Test get_metadata_token via public MetadataToken property
            ConstructorInfo ctor = typeof(TC_System_Reflection_RuntimeConstructorInfo).GetConstructor(Type.EmptyTypes);
            Assert.NotNull(ctor);
            int token = ctor.MetadataToken;
            Assert.IsTrue(token != 0);
        }

        [UnitTest]
        public void InternalInvoke_NoArgs_CreatesInstance()
        {
            // Test InternalInvoke via public Invoke
            ConstructorInfo ctor = typeof(TestClass).GetConstructor(Type.EmptyTypes);
            Assert.NotNull(ctor);
            object instance = ctor.Invoke(new object[0]);
            Assert.NotNull(instance);
            Assert.IsTrue(instance is TestClass);
        }

        [UnitTest]
        public void InternalInvoke_WithArgs_CreatesInstanceWithValues()
        {
            // Test InternalInvoke with constructor arguments
            ConstructorInfo ctor = typeof(TestClassWithArgs).GetConstructor(new[] { typeof(int), typeof(string) });
            Assert.NotNull(ctor);
            object instance = ctor.Invoke(new object[] { 42, "test" });
            Assert.NotNull(instance);
            Assert.IsTrue(instance is TestClassWithArgs);
            TestClassWithArgs obj = (TestClassWithArgs)instance;
            Assert.Equal(42, obj.Value);
            Assert.Equal("test", obj.Name);
        }

        [UnitTest]
        public void GetConstructor_ViaReflection_ValidConstructor()
        {
            // Verify we can get constructor info for testing
            Type type = typeof(TestClass);
            ConstructorInfo[] ctors = type.GetConstructors();
            Assert.IsTrue(ctors.Length > 0);
            ConstructorInfo ctor = ctors[0];
            Assert.NotNull(ctor);
            Assert.Equal(".ctor", ctor.Name);
        }

        [UnitTest]
        public void GetConstructor_MultipleCtor_CanDistinguish()
        {
            // Test distinguishing between multiple constructors
            Type type = typeof(TestClassWithMultipleCtors);
            ConstructorInfo ctor1 = type.GetConstructor(Type.EmptyTypes);
            ConstructorInfo ctor2 = type.GetConstructor(new[] { typeof(int) });
            Assert.NotNull(ctor1);
            Assert.NotNull(ctor2);
            Assert.NotEqual(ctor1.MetadataToken, ctor2.MetadataToken);
        }

        //[UnitTest]
        //public void InternalInvoke_ExceptionHandling()
        //{
        //    // Test InternalInvoke with exception handling
        //    ConstructorInfo ctor = typeof(TestClassWithException).GetConstructor(Type.EmptyTypes);
        //    Assert.NotNull(ctor);
        //    try
        //    {
        //        object instance = ctor.Invoke(new object[0]);
        //        Assert.Fail("Expected exception to be thrown");
        //    }
        //    catch (TargetInvocationException ex)
        //    {
        //        Assert.NotNull(ex.InnerException);
        //        Assert.IsTrue(ex.InnerException is InvalidOperationException);
        //    }
        //}

        // Test classes for constructor testing

        private class TestClass
        {
            public TestClass() { }
        }

        private class TestClassWithArgs
        {
            public int Value { get; private set; }
            public string Name { get; private set; }

            public TestClassWithArgs(int value, string name)
            {
                Value = value;
                Name = name;
            }
        }

        private class TestClassWithMultipleCtors
        {
            public int Value { get; private set; }

            public TestClassWithMultipleCtors()
            {
                Value = 0;
            }

            public TestClassWithMultipleCtors(int value)
            {
                Value = value;
            }
        }

        private class TestClassWithException
        {
            public TestClassWithException()
            {
                throw new InvalidOperationException("Constructor error");
            }
        }
    }
}
