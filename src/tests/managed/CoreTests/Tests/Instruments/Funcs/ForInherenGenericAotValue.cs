namespace Tests.Instruments.Funcs
{
    public struct ForInherenGenericAotValue : IBar<object>, IRun<string>
    {
        public int x;

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

        public override string ToString()
        {
            return x.ToString();
        }
    }

}
