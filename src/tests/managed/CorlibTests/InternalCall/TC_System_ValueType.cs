using System;

namespace CorlibTests.InternalCall
{
    internal class TC_System_ValueType : TestCaseBase
    {
        private struct Pair
        {
            public int A;
            public int B;

            public Pair(int a, int b)
            {
                A = a;
                B = b;
            }
        }

        [CoversIcall("System.ValueType::InternalEquals(System.Object,System.Object,System.Object[]&)")]
        [UnitTest]
        public void Equals_StructValueTypes()
        {
            Pair left = new Pair(1, 2);
            Pair right = new Pair(1, 2);
            Pair other = new Pair(3, 4);
            Assert.IsTrue(left.Equals(right));
            Assert.IsFalse(left.Equals(other));
        }

        [CoversIcall("System.ValueType::InternalGetHashCode(System.Object,System.Object[]&)")]
        [UnitTest]
        public void GetHashCode_StructIsStable()
        {
            Pair p = new Pair(7, 8);
            int h1 = p.GetHashCode();
            int h2 = p.GetHashCode();
            Assert.Equal(h1, h2);
        }
    }
}
