namespace Tests.Instruments.Funcs
{
    public class ForInhereInterpClass : IInterpFoo
    {
        public int a;

        public int b;

        public ForInhereInterpClass(int a, int b)
        {
            this.a = a;
            this.b = b;
        }

        public void Inc()
        {
            a += 1;
        }

        public int Show()
        {
            return a;
        }

        public override int GetHashCode()
        {
            return a;
        }

        public int Calc()
        {
            return a + 1;
        }

        public static int Default()
        {
            return 10;
        }
    }

}
