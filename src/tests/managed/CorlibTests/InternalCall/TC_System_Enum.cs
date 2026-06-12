using System;
using System.Reflection;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Enum : TestCaseBase
    {
        private enum SampleFlags
        {
            None = 0,
            A = 1,
            B = 2,
        }

        [CoversIcall("System.Enum::get_hashcode")]
        [UnitTest]
        public void GetHashCode_ReturnsStableValue()
        {
            int h1 = SampleFlags.A.GetHashCode();
            int h2 = SampleFlags.A.GetHashCode();
            Assert.Equal(h1, h2);
        }

        [CoversIcall("System.Enum::InternalCompareTo")]
        [UnitTest]
        public void CompareTo_OrdersValues()
        {
            IComparable a = SampleFlags.A;
            Assert.Equal(0, a.CompareTo(SampleFlags.A));
            Assert.IsTrue(a.CompareTo(SampleFlags.B) < 0);
        }

        [CoversIcall("System.Enum::InternalHasFlag")]
        [UnitTest]
        public void HasFlag_MatchesFlags()
        {
            Assert.IsTrue(SampleFlags.A.HasFlag(SampleFlags.A));
            Assert.IsFalse(SampleFlags.A.HasFlag(SampleFlags.B));
        }

        [CoversIcall("System.Enum::InternalGetUnderlyingType")]
        [UnitTest]
        public void GetUnderlyingType_ReturnsInt32()
        {
            Assert.Equal(typeof(int), Enum.GetUnderlyingType(typeof(SampleFlags)));
        }

        [CoversIcall("System.Enum::GetEnumValuesAndNames")]
        [UnitTest]
        public void GetValuesAndNames_ReturnAllMembers()
        {
            Array values = Enum.GetValues(typeof(SampleFlags));
            string[] names = Enum.GetNames(typeof(SampleFlags));
            Assert.Equal(values.Length, names.Length);
            Assert.IsTrue(names.Length >= 3);
        }

        [CoversIcall("System.Enum::InternalBoxEnum")]
        [UnitTest]
        public void BoxEnum_PreservesValue()
        {
            object boxed = SampleFlags.B;
            Assert.NotNull(boxed);
            Assert.Equal(SampleFlags.B, boxed);
        }

        [CoversIcall("System.Enum::get_value")]
        [UnitTest]
        public void GetValue_ReturnsUnderlyingValue()
        {
            Enum valueEnum = SampleFlags.B;
            var method = IcallReflection.GetMethod("System.Enum", "GetValue", BindingFlags.Instance, Type.EmptyTypes);
            Assert.NotNull(method);
            object value = method.Invoke(valueEnum, null);
            Assert.Equal((long)2, Convert.ToInt64(value));
        }
    }
}
