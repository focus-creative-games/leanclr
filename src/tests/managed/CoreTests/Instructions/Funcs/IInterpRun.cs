namespace Tests.Instruments.Funcs
{
    public interface IInterpRun<in T>
    {
        int Comput(T a);
    }

}
