namespace Tests.Instruments.Funcs
{
    public class ForInherenGenericAotClass : IBar<object>, IRun<string>
    {
        public int x;

        public ForInherenGenericAotClass(int x)
        {
            this.x = x;
        }

        public void Inc()
        {
            x += 1;
        }

        public object Sum(int a)
        {
            return x;
        }

        public int Comput(string a)
        {
            return x;
        }

        public override int GetHashCode()
        {
            return x;
        }
    }

}
