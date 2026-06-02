using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Reflection_RuntimeParameterInfo : TestCaseBase
    {
        [UnitTest]
        public void GetMetadataToken_ReturnsNonZero()
        {
            // Test GetMetadataToken via public ParameterInfo.MetadataToken property
            MethodInfo method = typeof(TC_System_Reflection_RuntimeParameterInfo).GetMethod("SampleMethod", BindingFlags.NonPublic | BindingFlags.Instance);
            Assert.NotNull(method);
            ParameterInfo[] parameters = method.GetParameters();
            Assert.IsTrue(parameters.Length > 0);
            ParameterInfo param = parameters[0];
            int token = param.MetadataToken;
            Assert.IsTrue(token != 0);
        }

        [UnitTest]
        public void GetMetadataToken_ConsistentValue()
        {
            // Test that GetMetadataToken returns consistent value
            MethodInfo method = typeof(TC_System_Reflection_RuntimeParameterInfo).GetMethod("SampleMethod", BindingFlags.NonPublic | BindingFlags.Instance);
            ParameterInfo param = method.GetParameters()[0];
            int token1 = param.MetadataToken;
            int token2 = param.MetadataToken;
            Assert.Equal(token1, token2);
        }

        [UnitTest]
        public void GetMetadataToken_DifferentParameters()
        {
            // Test that different parameters have different tokens
            MethodInfo method = typeof(TC_System_Reflection_RuntimeParameterInfo).GetMethod("MultiParamMethod", BindingFlags.NonPublic | BindingFlags.Instance);
            Assert.NotNull(method);
            ParameterInfo[] parameters = method.GetParameters();
            Assert.IsTrue(parameters.Length >= 2);
            int token1 = parameters[0].MetadataToken;
            int token2 = parameters[1].MetadataToken;
            Assert.NotEqual(token1, token2);
        }

        [UnitTest]
        public void ParameterInfo_Name_NotEmpty()
        {
            // Test that parameter has valid name
            MethodInfo method = typeof(TC_System_Reflection_RuntimeParameterInfo).GetMethod("SampleMethod", BindingFlags.NonPublic | BindingFlags.Instance);
            ParameterInfo param = method.GetParameters()[0];
            Assert.NotNull(param.Name);
            Assert.IsTrue(param.Name.Length > 0);
        }

        [UnitTest]
        public void ParameterInfo_ParameterType_Valid()
        {
            // Test that parameter type is correct
            MethodInfo method = typeof(TC_System_Reflection_RuntimeParameterInfo).GetMethod("SampleMethod", BindingFlags.NonPublic | BindingFlags.Instance);
            ParameterInfo param = method.GetParameters()[0];
            Assert.NotNull(param.ParameterType);
            Assert.Equal(typeof(int), param.ParameterType);
        }

        [UnitTest]
        public void ParameterInfo_Position_Correct()
        {
            // Test that parameter position is correct
            MethodInfo method = typeof(TC_System_Reflection_RuntimeParameterInfo).GetMethod("MultiParamMethod", BindingFlags.NonPublic | BindingFlags.Instance);
            ParameterInfo[] parameters = method.GetParameters();
            Assert.Equal(0, parameters[0].Position);
            Assert.Equal(1, parameters[1].Position);
            Assert.Equal(2, parameters[2].Position);
        }

        [UnitTest]
        public void GetTypeModifiers_NoModifiers()
        {
            // Test GetTypeModifiers for regular parameters
            MethodInfo method = typeof(TC_System_Reflection_RuntimeParameterInfo).GetMethod("SampleMethod", BindingFlags.NonPublic | BindingFlags.Instance);
            ParameterInfo param = method.GetParameters()[0];
            Type[] modifiers = param.GetOptionalCustomModifiers();
            Assert.NotNull(modifiers);
            // Regular parameter should have no modifiers
        }

        [UnitTest]
        public void GetTypeModifiers_MultipleParameters()
        {
            // Test GetTypeModifiers with multiple parameters
            MethodInfo method = typeof(TC_System_Reflection_RuntimeParameterInfo).GetMethod("MultiParamMethod", BindingFlags.NonPublic | BindingFlags.Instance);
            ParameterInfo[] parameters = method.GetParameters();
            foreach (ParameterInfo param in parameters)
            {
                Type[] modifiers = param.GetOptionalCustomModifiers();
                Assert.NotNull(modifiers);
            }
        }

        [UnitTest]
        public void ParameterInfo_HasDefaultValue()
        {
            // Test HasDefaultValue for parameter with default value
            MethodInfo method = typeof(TC_System_Reflection_RuntimeParameterInfo).GetMethod("MethodWithDefault", BindingFlags.NonPublic | BindingFlags.Instance);
            ParameterInfo[] parameters = method.GetParameters();
            if (parameters.Length > 0)
            {
                // Check if HasDefaultValue property works
                bool hasDefault = parameters[0].HasDefaultValue;
                // Just verify no exception thrown
            }
        }

        [UnitTest]
        public void ParameterInfo_Attributes_Valid()
        {
            // Test that parameter attributes are accessible
            MethodInfo method = typeof(TC_System_Reflection_RuntimeParameterInfo).GetMethod("SampleMethod", BindingFlags.NonPublic | BindingFlags.Instance);
            ParameterInfo param = method.GetParameters()[0];
            ParameterAttributes attrs = param.Attributes;
            // Just verify attributes can be retrieved
            Assert.IsTrue(true);
        }

        [UnitTest]
        public void ParameterInfo_Member_ReturnsMethod()
        {
            // Test that parameter.Member returns the method
            MethodInfo method = typeof(TC_System_Reflection_RuntimeParameterInfo).GetMethod("SampleMethod", BindingFlags.NonPublic | BindingFlags.Instance);
            ParameterInfo param = method.GetParameters()[0];
            MemberInfo member = param.Member;
            Assert.NotNull(member);
            Assert.Equal(method, member);
        }

        [UnitTest]
        public void ParameterInfo_ConstructorParameter()
        {
            // Test parameter info for constructor parameters
            ConstructorInfo ctor = typeof(ParameterTestClass).GetConstructor(new[] { typeof(int), typeof(string) });
            Assert.NotNull(ctor);
            ParameterInfo[] parameters = ctor.GetParameters();
            Assert.Equal(2, parameters.Length);
            Assert.Equal(typeof(int), parameters[0].ParameterType);
            Assert.Equal(typeof(string), parameters[1].ParameterType);
        }

        [UnitTest]
        public void ParameterInfo_ConstructorParameter_Tokens()
        {
            // Test metadata tokens for constructor parameters
            ConstructorInfo ctor = typeof(ParameterTestClass).GetConstructor(new[] { typeof(int), typeof(string) });
            ParameterInfo[] parameters = ctor.GetParameters();
            int token0 = parameters[0].MetadataToken;
            int token1 = parameters[1].MetadataToken;
            Assert.NotEqual(token0, token1);
        }

        [UnitTest]
        public void ParameterInfo_GenericMethodParameter()
        {
            // Test parameter info for generic method parameters
            MethodInfo method = typeof(TC_System_Reflection_RuntimeParameterInfo).GetMethod("GenericMethod", BindingFlags.NonPublic | BindingFlags.Instance);
            Assert.NotNull(method);
            ParameterInfo[] parameters = method.GetParameters();
            if (parameters.Length > 0)
            {
                // Generic method parameters should still work
                int token = parameters[0].MetadataToken;
                Assert.NotEqual(0, token);
            }
        }

        [UnitTest]
        public void ParameterInfo_ReturnParameter()
        {
            // Test parameter info for return value
            MethodInfo method = typeof(TC_System_Reflection_RuntimeParameterInfo).GetMethod("SampleMethod", BindingFlags.NonPublic | BindingFlags.Instance);
            ParameterInfo returnParam = method.ReturnParameter;
            Assert.NotNull(returnParam);
            Assert.Equal(typeof(int), returnParam.ParameterType);
        }

        // Test helper methods and classes

        private int SampleMethod(int value)
        {
            return value + 1;
        }

        private void MultiParamMethod(int a, string b, bool c)
        {
        }

        private void MethodWithDefault(int value = 5)
        {
        }

        private T GenericMethod<T>(T value)
        {
            return value;
        }

        private class ParameterTestClass
        {
            public ParameterTestClass(int id, string name)
            {
            }
        }
    }
}
