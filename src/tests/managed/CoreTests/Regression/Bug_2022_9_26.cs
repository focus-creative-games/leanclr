
using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using AOTDefs;


namespace Tests.Bugs
{
    enum HotUpdateEnum : ushort
    {
        A,
        B,
    }

    internal class Bug_2022_9_26 : GeneralTestCaseBase
    {
        [UnitTest]
        public void list_enum_byte()
        {
            var arr = new List<AOT_Enum_byte>();
            arr.Add(AOT_Enum_byte.A);
            Assert.True(arr.Contains(AOT_Enum_byte.A));
        }

        [UnitTest]
        public void list_enum_sbyte()
        {
            var arr = new List<AOT_Enum_sbyte>();
            arr.Add(AOT_Enum_sbyte.A);
            Assert.True(arr.Contains(AOT_Enum_sbyte.A));
        }

        [UnitTest]
        public void list_enum_short()
        {
            var arr = new List<AOT_Enum_short>();
            arr.Add(AOT_Enum_short.A);
            Assert.True(arr.Contains(AOT_Enum_short.A));
        }

        [UnitTest]
        public void list_enum_ushort()
        {
            var arr = new List<AOT_Enum_ushort>();
            arr.Add(AOT_Enum_ushort.A);
            Assert.True(arr.Contains(AOT_Enum_ushort.A));
        }

        [UnitTest]
        public void list_enum_int()
        {
            var arr = new List<AOT_Enum_int>();
            arr.Add(AOT_Enum_int.A);
            Assert.True(arr.Contains(AOT_Enum_int.A));
        }

        [UnitTest]
        public void list_enum_uint()
        {
            var arr = new List<AOT_Enum_uint>();
            arr.Add(AOT_Enum_uint.A);
            Assert.True(arr.Contains(AOT_Enum_uint.A));
        }

        [UnitTest]
        public void list_enum_long()
        {
            var arr = new List<AOT_Enum_long>();
            arr.Add(AOT_Enum_long.A);
            Assert.True(arr.Contains(AOT_Enum_long.A));
        }


        [UnitTest]
        public void list_enum_ulong()
        {
            var arr = new List<AOT_Enum_ulong>();
            arr.Add(AOT_Enum_ulong.A);
            Assert.True(arr.Contains(AOT_Enum_ulong.A));
        }
    }
}
