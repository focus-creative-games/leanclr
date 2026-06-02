using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Test
{
    public interface IFoo
    {
        int Calc();
    }
    
    public interface IBar<T>
    {
        T Sum(int a);
        int GetHashCode();
    }
    
    public interface IRun<T>
    {
        int Comput(T a);
        int GetHashCode();
    }
}
