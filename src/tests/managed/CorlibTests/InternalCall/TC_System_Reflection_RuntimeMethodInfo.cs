using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Reflection_RuntimeMethodInfo : TestCaseBase
    {
        [UnitTest]
        public void GetName_ReturnsCorrectName()
        {
            // Test get_name via public Name property
            MethodInfo method = typeof(TC_System_Reflection_RuntimeMethodInfo).GetMethod("GetName_ReturnsCorrectName");
            Assert.NotNull(method);
            Assert.Equal("GetName_ReturnsCorrectName", method.Name);
        }

        [UnitTest]
        public void GetMetadataToken_ReturnsNonZero()
        {
            // Test get_metadata_token via public MetadataToken property
            MethodInfo method = typeof(TC_System_Reflection_RuntimeMethodInfo).GetMethod("GetMetadataToken_ReturnsNonZero");
            Assert.NotNull(method);
            int token = method.MetadataToken;
            Assert.IsTrue(token != 0);
        }

        [UnitTest]
        public void Invoke_SimpleMethod()
        {
            // Test InternalInvoke via public Invoke
            MethodInfo method = typeof(TC_System_Reflection_RuntimeMethodInfo).GetMethod("SampleMethod", BindingFlags.NonPublic | BindingFlags.Instance);
            Assert.NotNull(method);
            var instance = new TC_System_Reflection_RuntimeMethodInfo();
            object result = method.Invoke(instance, new object[] { 10, 5 });
            Assert.Equal(15, result);
        }

        private int SampleMethod(int a, int b)
        {
            return a + b;
        }

        [UnitTest]
        public void GetGenericArguments_NonGeneric_ReturnsEmpty()
        {
            // Test GetGenericArguments for non-generic method
            MethodInfo method = typeof(TC_System_Reflection_RuntimeMethodInfo).GetMethod("GetName_ReturnsCorrectName");
            Assert.NotNull(method);
            Type[] args = method.GetGenericArguments();
            Assert.NotNull(args);
            Assert.Equal(0, args.Length);
        }

        [UnitTest]
        public void GetGenericArguments_GenericMethod_ReturnsTypes()
        {
            // Test GetGenericArguments for generic method
            MethodInfo method = typeof(TC_System_Reflection_RuntimeMethodInfo).GetMethod("GenericMethod", BindingFlags.NonPublic | BindingFlags.Instance);
            Assert.NotNull(method);
            Type[] args = method.GetGenericArguments();
            Assert.NotNull(args);
            Assert.Equal(1, args.Length);
        }

        private T GenericMethod<T>(T value)
        {
            return value;
        }

        [UnitTest]
        public void IsGenericMethod_NonGeneric_ReturnsFalse()
        {
            // Test get_IsGenericMethod via public property
            MethodInfo method = typeof(TC_System_Reflection_RuntimeMethodInfo).GetMethod("GetName_ReturnsCorrectName");
            Assert.NotNull(method);
            Assert.IsFalse(method.IsGenericMethod);
        }

        [UnitTest]
        public void IsGenericMethod_Generic_ReturnsTrue()
        {
            // Test get_IsGenericMethod for generic method
            MethodInfo method = typeof(TC_System_Reflection_RuntimeMethodInfo).GetMethod("GenericMethod", BindingFlags.NonPublic | BindingFlags.Instance);
            Assert.NotNull(method);
            Assert.IsTrue(method.IsGenericMethod);
        }

        [UnitTest]
        public void IsGenericMethodDefinition_NonGeneric_ReturnsFalse()
        {
            // Test get_IsGenericMethodDefinition via public property
            MethodInfo method = typeof(TC_System_Reflection_RuntimeMethodInfo).GetMethod("GetName_ReturnsCorrectName");
            Assert.NotNull(method);
            Assert.IsFalse(method.IsGenericMethodDefinition);
        }

        [UnitTest]
        public void IsGenericMethodDefinition_Generic_ReturnsTrue()
        {
            // Test get_IsGenericMethodDefinition for open generic method
            MethodInfo method = typeof(TC_System_Reflection_RuntimeMethodInfo).GetMethod("GenericMethod", BindingFlags.NonPublic | BindingFlags.Instance);
            Assert.NotNull(method);
            Assert.IsTrue(method.IsGenericMethodDefinition);
        }

        [UnitTest]
        public void MakeGenericMethod_CreatesSpecialized()
        {
            // Test MakeGenericMethod_impl via public MakeGenericMethod
            MethodInfo openMethod = typeof(TC_System_Reflection_RuntimeMethodInfo).GetMethod("GenericMethod", BindingFlags.NonPublic | BindingFlags.Instance);
            Assert.NotNull(openMethod);
            MethodInfo closedMethod = openMethod.MakeGenericMethod(typeof(int));
            Assert.NotNull(closedMethod);
            Assert.IsTrue(closedMethod.IsGenericMethod);
            Assert.IsFalse(closedMethod.IsGenericMethodDefinition);
        }

        [UnitTest]
        public void GetGenericMethodDefinition_ReturnsOpen()
        {
            // Test GetGenericMethodDefinition_impl via public GetGenericMethodDefinition
            MethodInfo openMethod = typeof(TC_System_Reflection_RuntimeMethodInfo).GetMethod("GenericMethod", BindingFlags.NonPublic | BindingFlags.Instance);
            Assert.NotNull(openMethod);
            MethodInfo closedMethod = openMethod.MakeGenericMethod(typeof(string));
            MethodInfo definition = closedMethod.GetGenericMethodDefinition();
            Assert.NotNull(definition);
            Assert.Equal(openMethod, definition);
        }

        [UnitTest]
        public void GetMethodFromHandle_ReturnsValidMethod()
        {
            // Test GetMethodFromHandleInternalType_native via public MethodBase.GetMethodFromHandle
            MethodInfo method = typeof(TC_System_Reflection_RuntimeMethodInfo).GetMethod("GetName_ReturnsCorrectName");
            Assert.NotNull(method);
            RuntimeMethodHandle handle = method.MethodHandle;
            MethodBase retrieved = MethodBase.GetMethodFromHandle(handle);
            Assert.NotNull(retrieved);
            Assert.Equal(method, retrieved);
        }

        [UnitTest]
        public void GetBaseMethod_Virtual_ReturnsBase()
        {
            // Test get_base_method via public GetBaseDefinition for virtual method
            MethodInfo derivedMethod = typeof(DerivedClass).GetMethod("VirtualMethod");
            Assert.NotNull(derivedMethod);
            MethodInfo baseMethod = derivedMethod.GetBaseDefinition();
            Assert.NotNull(baseMethod);
            Assert.Equal(typeof(BaseClass), baseMethod.DeclaringType);
        }

        private abstract class BaseClass
        {
            public abstract void VirtualMethod();
        }

        private class DerivedClass : BaseClass
        {
            public override void VirtualMethod() { }


            public void NotTrivalMethod()
            {
                int j = 0;
                try
                {
                    for (int i = 0; i < 10; i++)
                    {
                        i.ToString();
                        j = i;
                    }
                }
                catch (InvalidCastException) when (j > 3)
                {
                    throw;
                }
                catch (ArgumentException)
                {
                    throw;
                }
                finally
                {
                    j++;
                }
            }
        }

        [UnitTest]
        public void GetMethodBody_AbstractMethod_ReturnsNull()
        {
            // Test GetMethodBodyInternal via public GetMethodBody
            MethodInfo method = typeof(BaseClass).GetMethod("VirtualMethod", BindingFlags.Public | BindingFlags.Instance);
            Assert.NotNull(method);
            MethodBody body = method.GetMethodBody();
            Assert.Null(body);
        }

        [UnitTest]
        public void GetMethodBody_OverrideMethod_ReturnsNonNull()
        {
            // Test GetMethodBodyInternal via public GetMethodBody
            MethodInfo method = typeof(DerivedClass).GetMethod("VirtualMethod", BindingFlags.Public | BindingFlags.Instance);
            Assert.NotNull(method);
            MethodBody body = method.GetMethodBody();
            Assert.NotNull(body);
        }

        [UnitTest]
        public void GetMethodBody_NotTrivalMethod_ReturnsNonNull()
        {
            // Test GetMethodBodyInternal via public GetMethodBody
            MethodInfo method = typeof(DerivedClass).GetMethod("NotTrivalMethod", BindingFlags.Public | BindingFlags.Instance);
            Assert.NotNull(method);
            MethodBody body = method.GetMethodBody();
            Assert.NotNull(body);
        }
    }
}
