using System;

namespace CorlibTests.InternalCall
{
    /// <summary>
    /// Tests for System.RuntimeType::make_array_type (MakeArrayType / MakeArrayType(int)).
    /// Convention: parameterless MakeArrayType() passes rank 0 to native (SZArray T[]);
    /// MakeArrayType(rank) with rank >= 1 creates a bounded multi-dimensional array.
    /// </summary>
    internal class TC_System_RuntimeType : GeneralTestCaseBase
    {
        [UnitTest]
        public void MakeArrayType_NoArgs_ReturnsSZArray()
        {
            Type arrayType = typeof(int).MakeArrayType();
            Assert.Equal(typeof(int[]), arrayType);
        }

        [UnitTest]
        public void MakeArrayType_NoArgs_IsArrayWithRankOne()
        {
            Type arrayType = typeof(int).MakeArrayType();
            Assert.IsTrue(arrayType.IsArray);
            Assert.Equal(1, arrayType.GetArrayRank());
        }

        [UnitTest]
        public void MakeArrayType_NoArgs_ElementType()
        {
            Type arrayType = typeof(string).MakeArrayType();
            Assert.Equal(typeof(string), arrayType.GetElementType());
        }

        [UnitTest]
        public void MakeArrayType_Rank1_NotSameAsSZArray()
        {
            Type boundRank1 = typeof(int).MakeArrayType(1);
            Assert.IsFalse(boundRank1 == typeof(int[]));
            Assert.Equal(1, boundRank1.GetArrayRank());
            Assert.Equal(typeof(int), boundRank1.GetElementType());
        }

        [UnitTest]
        public void MakeArrayType_Rank2_ReturnsMultiDimensional()
        {
            Type array2d = typeof(int).MakeArrayType(2);
            Assert.Equal(typeof(int[,]), array2d);
            Assert.Equal(2, array2d.GetArrayRank());
            Assert.Equal(typeof(int), array2d.GetElementType());
            Assert.IsFalse(array2d == typeof(int[][]));
        }

        [UnitTest]
        public void MakeArrayType_Rank3()
        {
            Type array3d = typeof(int).MakeArrayType(3);
            Assert.Equal(3, array3d.GetArrayRank());
            Assert.Equal(typeof(int), array3d.GetElementType());
        }

        [UnitTest]
        public void MakeArrayType_Chained_OnSZArray()
        {
            Type jagged = typeof(int).MakeArrayType().MakeArrayType();
            Assert.Equal(typeof(int[][]), jagged);
        }

        [UnitTest]
        public void MakeArrayType_ReferenceType()
        {
            Type arrayType = typeof(object).MakeArrayType();
            Assert.Equal(typeof(object[]), arrayType);
        }

        [UnitTest]
        public void MakeArrayType_RankZero_Throws()
        {
            Assert.ExpectException<IndexOutOfRangeException>(() => typeof(int).MakeArrayType(0));
        }

        [UnitTest]
        public void MakeArrayType_NegativeRank_Throws()
        {
            Assert.ExpectException<IndexOutOfRangeException>(() => typeof(int).MakeArrayType(-1));
        }

        [UnitTest]
        public void MakeArrayType_RankAboveMax_Throws()
        {
            // Native RT_MAX_ARRAY_RANK is 50 (0x32); managed mscorlib does not cap rank before the icall.
            Assert.ExpectException<TypeLoadException>(() => typeof(int).MakeArrayType(51));
        }

        [UnitTest]
        public void MakeArrayType_OnByRefElement_Throws()
        {
            Type byRef = typeof(int).MakeByRefType();
            Assert.ExpectException<TypeLoadException>(() => byRef.MakeArrayType());
        }
    }
}
