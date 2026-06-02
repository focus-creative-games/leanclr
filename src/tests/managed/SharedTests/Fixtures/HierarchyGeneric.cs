using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AOTDefs
{
    public interface IHG
    {
        int Load(int x);
    }

    public class HierarchyGeneric<A> : IHG
    {
        public int Load(int x)
        {
            return x;
        }
    }



    public class HierarchyGeneric2<B> : HierarchyGeneric<B>
    {

    }
}
