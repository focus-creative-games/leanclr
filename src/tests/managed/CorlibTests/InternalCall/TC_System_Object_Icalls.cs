using System;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Object_Icalls : TestCaseBase
    {
        private class Cloneable
        {
            public int Value = 42;

            public Cloneable CloneViaMemberwiseClone()
            {
                return (Cloneable)MemberwiseClone();
            }
        }

        [CoversIcall("System.Object::GetType")]
        [UnitTest]
        public void GetType_ReturnsRuntimeType()
        {
            object obj = "hello";
            Assert.Equal(typeof(string), obj.GetType());
        }

        [CoversIcall("System.Object::InternalGetHashCode")]
        [UnitTest]
        public void GetHashCode_ReferenceType()
        {
            object obj = new object();
            int hash = obj.GetHashCode();
            Assert.Equal(hash, obj.GetHashCode());
        }

        [CoversIcall("System.Object::MemberwiseClone")]
        [UnitTest]
        public void MemberwiseClone_CopiesFields()
        {
            Cloneable source = new Cloneable { Value = 99 };
            Cloneable copy = source.CloneViaMemberwiseClone();
            Assert.NotNull(copy);
            Assert.Equal(99, copy.Value);
            Assert.IsFalse(ReferenceEquals(source, copy));
        }
    }
}
