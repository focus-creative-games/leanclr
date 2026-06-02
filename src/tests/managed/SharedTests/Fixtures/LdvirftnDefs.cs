using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AOTDefs
{
    public interface Ldvirftn_IMath
    {
        int Add(int x);
    }

    public class Ldvirftn_A : Ldvirftn_IMath
    {
        public int x;

        public virtual int Add(int a)
        {
            this.x += a;
            return this.x;
        }

        public virtual int Foo()
        {
            return 1;
        }
    }

    public class Ldvirftn_B : Ldvirftn_A
    {
        public override int Add(int a)
        {
            this.x += a * 2;
            return this.x;
        }

        public override int Foo()
        {
            return 2;
        }
    }

    public struct Ldvirftn_S : Ldvirftn_IMath
    {
        public int x;

        public int Add(int a)
        {
            this.x += a;
            return this.x;
        }
    }
}
