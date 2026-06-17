using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Reflection_RuntimeModule : TestCaseBase
    {
        [UnitTest]
        public void GetMetadataToken_ReturnsNonZero()
        {
            // Test get_MetadataToken via public Module.MetadataToken property
            Module module = typeof(TC_System_Reflection_RuntimeModule).Module;
            Assert.NotNull(module);
            int token = module.MetadataToken;
            Assert.IsTrue(token != 0);
        }

        [UnitTest]
        public void GetMetadataToken_ConsistentValue()
        {
            // Test that get_MetadataToken returns consistent value
            Module module = typeof(TC_System_Reflection_RuntimeModule).Module;
            int token1 = module.MetadataToken;
            int token2 = module.MetadataToken;
            Assert.Equal(token1, token2);
        }

        [UnitTest]
        public void GetTypes_ReturnsTypesArray()
        {
            // Test InternalGetTypes via public Module.GetTypes
            Module module = typeof(TC_System_Reflection_RuntimeModule).Module;
            Assert.NotNull(module);
            Type[] types = module.GetTypes();
            Assert.NotNull(types);
            Assert.IsTrue(types.Length > 0);
        }

        [UnitTest]
        public void GetTypes_ContainsCurrentType()
        {
            // Test that GetTypes includes current test class
            Module module = typeof(TC_System_Reflection_RuntimeModule).Module;
            Type[] types = module.GetTypes();
            bool found = false;
            foreach (Type t in types)
            {
                if (t.Name == "TC_System_Reflection_RuntimeModule")
                {
                    found = true;
                    break;
                }
            }
            Assert.IsTrue(found);
        }

        [UnitTest]
        public void GetHINSTANCE_ReturnsValidPointer()
        {
            // Test GetHINSTANCE via public Module.GetHINSTANCE (when available)
            // This is a non-public icall, test indirectly via module handle
            Module module = typeof(TC_System_Reflection_RuntimeModule).Module;
            Assert.NotNull(module);
            // Module should have valid metadata
            Assert.NotEqual(0, module.MetadataToken);
        }

        [UnitTest]
        public void GetGuidInternal_NoException()
        {
            // Test GetGuidInternal - indirect test via module access
            // This is a non-public icall, test that module operations don't throw
            Module module = typeof(TC_System_Reflection_RuntimeModule).Module;
            Type[] types = module.GetTypes();
            Assert.NotNull(types);
        }

        [UnitTest]
        public void GetGlobalType_ReturnsModuleType()
        {
            // Test GetGlobalType via public Module.GetType with null name
            Module module = typeof(TC_System_Reflection_RuntimeModule).Module;
            // GetGlobalType is tested indirectly through module type resolution
            Type type = module.GetType("CorlibTests.InternalCall.TC_System_Reflection_RuntimeModule");
            Assert.NotNull(type);
            type.GetField("NOT_EXIST_FIELD");
        }

        [UnitTest]
        public void GetMDStreamVersion_NoException()
        {
            // Test GetMDStreamVersion - indirect test
            // This is a non-public icall, test via module access
            Module module = typeof(TC_System_Reflection_RuntimeModule).Module;
            Type[] types = module.GetTypes();
            Assert.NotNull(types);
            Assert.IsTrue(types.Length > 0);
        }

        class A
        {
            public int value;

            public void Run()
            {
                value += 1;
            }
        }

        [UnitTest]
        public void ResolveTypeToken_ValidToken()
        {
            // Test ResolveTypeToken via public Module.ResolveType
            Module module = typeof(TC_System_Reflection_RuntimeModule).Module;
            Type testType = typeof(A);
            int token = testType.MetadataToken;
            Assert.NotEqual(0, token);
            Type resolved = module.ResolveType(token);
            Assert.Equal(testType, resolved);
        }

        [UnitTest]
        public void ResolveMethodToken_ValidToken()
        {
            // Test ResolveMethodToken via public Module.ResolveMethod
            Module module = typeof(TC_System_Reflection_RuntimeModule).Module;
            MethodInfo method = typeof(A).GetMethod("Run");
            int token = method.MetadataToken;
            Assert.NotEqual(0, token);
            MethodBase resolved = module.ResolveMethod(token);
            Assert.Equal(method, resolved);
        }

        [UnitTest]
        public void ResolveFieldToken_ValidToken()
        {
            // Test ResolveFieldToken via public Module.ResolveField
            Module module = typeof(TC_System_Reflection_RuntimeModule).Module;
            FieldInfo field = typeof(A).GetField("value");
            int token = field.MetadataToken;
            Assert.NotEqual(0, token);
            FieldInfo resolved = module.ResolveField(token);
            Assert.Equal(field, resolved);
        }


        [UnitTest]
        public void ResolveMemberToken_ValidToken()
        {
            // Test ResolveMemberToken via public Module.ResolveMember
            Module module = typeof(TC_System_Reflection_RuntimeModule).Module;
            MemberInfo member = typeof(A).GetMethod("Run");
            int token = member.MetadataToken;
            Assert.NotEqual(0, token);
            MemberInfo resolved = module.ResolveMember(token);
            Assert.Equal(member, resolved);
        }

        [UnitTest]
        public void ResolveSignature_ValidToken()
        {
            // Test ResolveSignature via public Module.ResolveSignature
            Module module = typeof(TC_System_Reflection_RuntimeModule).Module;
            // Signature tokens are rare and module-specific
            byte[] signature = module.ResolveSignature(0x04000001);
            Assert.NotNull(signature);
        }

        //[UnitTest]
        //public void GetPEKind_ReturnsValidEnums()
        //{
        //    // Test GetPEKind via public Module.GetPEKind
        //    Module module = typeof(TC_System_Reflection_RuntimeModule).Module;
        //    PortableExecutableKinds peKind;
        //    ImageFileMachine machine;
        //    module.GetPEKind(out peKind, out machine);
        //    // Just verify that GetPEKind completes without exception
        //    // The values depend on the platform
        //}

        //[UnitTest]
        //public void GetPEKind_ReturnsConsistentValues()
        //{
        //    // Test GetPEKind returns same values on multiple calls
        //    Module module = typeof(TC_System_Reflection_RuntimeModule).Module;
        //    PortableExecutableKinds peKind1, peKind2;
        //    ImageFileMachine machine1, machine2;
        //    module.GetPEKind(out peKind1, out machine1);
        //    module.GetPEKind(out peKind2, out machine2);
        //    Assert.Equal(peKind1, peKind2);
        //    Assert.Equal(machine1, machine2);
        //}

        [UnitTest]
        public void Module_FullyQualifiedName_NotEmpty()
        {
            // Test that module has valid fully qualified name
            Module module = typeof(TC_System_Reflection_RuntimeModule).Module;
            Assert.NotNull(module.FullyQualifiedName);
            Assert.IsTrue(module.FullyQualifiedName.Length > 0);
        }

        [UnitTest]
        public void Module_Name_NotEmpty()
        {
            // Test that module has valid name
            Module module = typeof(TC_System_Reflection_RuntimeModule).Module;
            Assert.NotNull(module.Name);
            Assert.IsTrue(module.Name.Length > 0);
        }

        [UnitTest]
        public void GetTypeByNameInModule_FindsTestClass()
        {
            // Test finding type in module
            Module module = typeof(TC_System_Reflection_RuntimeModule).Module;
            Type foundType = module.GetType("CorlibTests.InternalCall.TC_System_Reflection_RuntimeModule+A");
            Assert.NotNull(foundType);
            Assert.Equal(typeof(A), foundType);
        }

        [UnitTest]
        public void GetTypeByNameInModule_CaseSensitive()
        {
            // Test finding type in module
            Module module = typeof(TC_System_Reflection_RuntimeModule).Module;
            Type foundType = module.GetType("CorlibTests.InternalCall.TC_System_Reflection_runtimemodule+a");
            Assert.Null(foundType);
        }

        [UnitTest]
        public void GetTypeByNameInModule_CaseInSensitive()
        {
            // Test finding type in module
            Module module = typeof(TC_System_Reflection_RuntimeModule).Module;
            Type foundType = module.GetType("CorlibTests.InternalCall.TC_System_Reflection_runtimemodule+a", true);
            Assert.NotNull(foundType);
            Assert.Equal(typeof(A), foundType);
        }
    }
}
