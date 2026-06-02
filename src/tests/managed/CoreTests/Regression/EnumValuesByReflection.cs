//
//using test;
//using System;
//using System.Collections.Generic;
//using System.Linq;
//using System.Text;
//using System.Threading.Tasks;


//namespace Tests.Bugs
//{
//    class EnumValuesByReflection : GeneralTestCaseBase
//    {
//        [UnitTest]
//        public void AOTEnumValueByte()
//        {
//            Array values = Enum.GetValues(typeof(AOT_Enum_byte2));
//            Assert.Equal(1, (byte)values.GetValue(0));
//            Assert.Equal(3, (byte)values.GetValue(2));
//        }

//        [UnitTest]
//        public void AOTEnumValueInt()
//        {
//            Array values = Enum.GetValues(typeof(AOT_Enum_int2));
//            Array names = Enum.GetNames(typeof(AOT_Enum_int));
//            Assert.Equal(1, (int)values.GetValue(0));
//            Assert.Equal(10, (int)values.GetValue(1));
//            Assert.Equal(1000, (int)values.GetValue(2));
//        }


//        public enum Interp_Enum_byte : byte
//        {
//            A = 1,
//            B = 2,
//            C = 3,
//        }

//        public enum Interp_Enum_int : int
//        {
//            B = 1,
//            C = 10,
//            D = 1000,
//        }

//        public enum Interp_Enum_uint : uint
//        {
//            B = 1,
//            C = 10,
//            D = 1000,
//        }


//        [UnitTest]
//        public void InterpEnumValueByte()
//        {
//            Array values = Enum.GetValues(typeof(Interp_Enum_byte));
//            Assert.Equal(1, (byte)values.GetValue(0));
//            Assert.Equal(3, (byte)values.GetValue(2));
//        }

//        [UnitTest]
//        public void InterpEnumValueInt()
//        {
//            Array values = Enum.GetValues(typeof(Interp_Enum_int));
//            Assert.Equal(1, (int)values.GetValue(0));
//            Assert.Equal(10, (int)values.GetValue(1));
//            Assert.Equal(1000, (int)values.GetValue(2));
//        }

//        [UnitTest]
//        public void InterpEnumValueUint()
//        {
//            Array values = Enum.GetValues(typeof(Interp_Enum_uint));
//            Assert.Equal(1, (uint)values.GetValue(0));
//            Assert.Equal(10, (uint)values.GetValue(1));
//            Assert.Equal(1000, (uint)values.GetValue(2));
//        }
//    }
//}
