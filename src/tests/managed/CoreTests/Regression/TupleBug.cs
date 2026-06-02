using test;
using System;
using System.Collections.Generic;


namespace Tests.Bugs
{
    public class TupleBug : GeneralTestCaseBase
    {

        public  static T Ldind<T>(T v)
        {
            T r = default;
            ref T r2 = ref r;
            r2 = v;
            return v;
        }

        [UnitTest]
        public void TestRef()
        {
            var s = "abc";
            var s2 = Ldind(s);
            Assert.Equal(s, s2);
        }

        [UnitTest]
        public void TestInt()
        {
            var s = 5;
            var s2 = Ldind(s);
            Assert.Equal(s, s2);
        }

        [UnitTest]
        public void TestVec()
        {
            var s = new Vector3(1f, 2f ,3f);
            var s2 = Ldind(s);
            Assert.Equal(1f, s2.x);
            Assert.Equal(2f, s2.y);
            Assert.Equal(3f, s2.z);
        }
    }
}