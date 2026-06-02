namespace Tests.Instruments.Funcs
{
    public class ForInherenGenericInterpClass : IInterpBar<object>, IInterpRun<string>
    {
        public int x;

        public ForInherenGenericInterpClass(int x)
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
