using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AOTDefs
{
    public class EnumFieldAttribute : Attribute
    {
        public EnumFieldAttribute(AOT_Enum_int x)
        {
            X = x;
        }

        public AOT_Enum_int X { get; }
    }
}
