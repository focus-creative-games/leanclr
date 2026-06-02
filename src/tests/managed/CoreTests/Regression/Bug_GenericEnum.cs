using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Bugs
{
    public class GenericParent<T> : List<T>
    {
        public enum BookType
        {
            A,
            B,
        }

        public int Compare()
        {
            BookType a = BookType.A;
            if (a == BookType.A)
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
    }

    internal class Bug_GenericEnum : GeneralTestCaseBase
    {
        [UnitTest]
        public void compare_booktype()
        {
            var g = new GenericParent<int>();
            Assert.Equal(1, g.Compare());
        }
    }
}
