
using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Test;
using AOTDefs;

namespace Tests.Instruments.Objs
{
    internal class TC_initobj : GeneralTestCaseBase
    {

        static void InitObj<T>(out T obj)
        {
            obj = default;
        }

        // 不好构造

        [UnitTest]
        public unsafe void valuetypesize1_1()
        {
            long a = -1;
            ValueTypeSize1 x = default;
            long b = -1;
            
            Assert.Equal(0, x.x1);
            Assert.Equal(-1, a);
            Assert.Equal(-1, b);
        }

        [UnitTest]
        public unsafe void valuetypesize2_1()
        {
            long a = -1;
            ValueTypeSize2 x = default;
            long b = -1;

            Assert.Equal(0, x.x1);
            Assert.Equal(-1, a);
            Assert.Equal(-1, b);
        }

        [UnitTest]
        public unsafe void valuetypesize3_1()
        {
            long a = -1;
            ValueTypeSize3 x = default;
            long b = -1;

            Assert.Equal(0, x.x1);
            Assert.Equal(-1, a);
            Assert.Equal(-1, b);
        }

        [UnitTest]
        public unsafe void valuetypesize4_1()
        {
            long a = -1;
            ValueTypeSize4 x = default;
            long b = -1;

            Assert.Equal(0, x.x1);
            Assert.Equal(-1, a);
            Assert.Equal(-1, b);
        }

        [UnitTest]
        public unsafe void valuetypesize5_1()
        {
            long a = -1;
            ValueTypeSize5 x = default;
            long b = -1;

            Assert.Equal(0, x.x1);
            Assert.Equal(-1, a);
            Assert.Equal(-1, b);
        }

        [UnitTest]
        public unsafe void valuetypesize8_1()
        {
            long a = -1;
            ValueTypeSize8 x = default;
            long b = -1;

            Assert.Equal(0, x.x1);
            Assert.Equal(-1, a);
            Assert.Equal(-1, b);
        }

        [UnitTest]
        public unsafe void valuetypesize9_1()
        {
            long a = -1;
            ValueTypeSize9 x = default;
            long b = -1;

            Assert.Equal(0, x.x1);
            Assert.Equal(-1, a);
            Assert.Equal(-1, b);
        }

        [UnitTest]
        public unsafe void valuetypesize16_1()
        {
            long a = -1;
            ValueTypeSize16 x = default;
            long b = -1;

            Assert.Equal(0, x.x1);
            Assert.Equal(-1, a);
            Assert.Equal(-1, b);
        }

        class Foo
        {
            public int x;
            public int y;
        }


        [UnitTest]
        public void class_1_null()
        {
            Foo foo;
            InitObj(out foo);
            Assert.Null(foo);
        }

        [UnitTest]
        public void class_1_notnull()
        {
            int a = 1;
            _ = (object)a;
            Foo foo = new Foo { x = 1, y = 2 };
            int b = 2;
            _ = (object)b;
            InitObj(out foo);
            Assert.Null(foo);
            Assert.Equal(1, a);
            Assert.Equal(2, b);
        }
    }
}
