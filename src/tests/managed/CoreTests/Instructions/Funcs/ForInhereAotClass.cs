namespace Tests.Instruments.Funcs
{
    public class ForInhereAotClass : IFoo
    {
        public int a;

        public int b;

        public ForInhereAotClass(int a, int b)
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

        public int Show2()
        {
            return 2;
        }

        public override string ToString()
        {
            return a.ToString();
        }

        public int Calc()
        {
            return 1;
        }

        public static int Default()
        {
            return 10;
        }

        public int CallGeneric<T>(T x)
        {
            return 100;
        }

        public static object CallStaticGeneric<T>(T x)
        {
            return x;
        }
    }

}
