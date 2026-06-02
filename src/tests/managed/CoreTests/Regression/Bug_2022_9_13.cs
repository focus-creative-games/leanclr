using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Bugs
{
    internal class Bug_2022_9_13 : GeneralTestCaseBase
    {
        private float Register(Dictionary<ushort, float> a, ushort key, float v)
        {
            if (a.ContainsKey(key))
            {
                return a[key];
            }
            else
            {
                a.Add(key, v);
                return a[key];
            }
        }

        [UnitTest]
        public void ushort_dic()
        {
            var m = new Dictionary<ushort, float>() { { 1, 1f }, { 50000, 2f } };
            Assert.True(m.TryGetValue(1, out _));
            ushort k = 50000;
            Assert.True(m.TryGetValue(k, out _));
            Assert.True(m.ContainsKey(k));
            float v = m[k];
            Assert.Equal(2f, v);
        }

        [UnitTest]
        public void ushort_dic2()
        {
            var m = new Dictionary<ushort, float>() { { 1, 1f }, { 50000, 2f } };

            float v1 = Register(m, 0, 3f);
            Assert.Equal(3f, v1);
            float v2 = Register(m, 1, 2f);
            Assert.Equal(1f, v2);
            float v3 = Register(m, 50000, 10f);
            Assert.Equal(2f, v3);
        }
    }
}
