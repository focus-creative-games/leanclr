namespace Tests.Instruments.Funcs
{
    public struct ForInherenGenericInterpValue : IInterpBar<object>, IInterpRun<string>
    {
        public int x;

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
