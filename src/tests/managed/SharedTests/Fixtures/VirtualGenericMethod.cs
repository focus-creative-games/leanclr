using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AOTDefs
{
    public interface IVG
    {
        string Foo<T>(T x);

        T Bar<T>(T a);
    }

    public abstract class VirtualGenericMethodBase
    {
        public virtual string Sum<T>(T a)
        {
            return a.ToString();
        }

        public virtual T Run<T>(T a, T b)
        {
            return a;
        }
    }

    public class VirtualGenericMethod : VirtualGenericMethodBase, IVG
    {
        public override string Sum<T>(T a)
        {
            return "#" + a.ToString();
        }

        public override T Run<T>(T a, T b)
        {
            return b;
        }

        public string Foo<T>(T x)
        {
            return "$" + x.ToString();
        }

        public T Bar<T>(T a)
        {
            return a;
        }
    }

    public static class RefVirtualGenericMethod
    {
        public static void Ref1()
        {
            IVG a = new VirtualGenericMethod();
            a.Foo<float>(1f);
            a.Bar<long>(1);

            VirtualGenericMethodBase b = new VirtualGenericMethod();
            b.Sum<long>(2);
            b.Run<float>(1f, 2f);
        }
    }
}
