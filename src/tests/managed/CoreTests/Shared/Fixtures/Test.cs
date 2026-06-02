using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;



public class TestAotRef
{
    public void CallRef(ref int a)
    {
        a = 5;
    }

    public void CallIn(in int b)
    {

    }

    public void CallOut(out int b)
    {
        b = 15;
    }
}

public class MainT
{
    public virtual void Log()
    {
    }

    public static void Foo<T>()
    {
        T[,] arr = new T[2, 3];
        T x = arr[0, 0];
        arr[0, 0] = default;
        ref T y = ref arr[1, 1];
        y = default;
    }

    public static void Show()
    {
        Foo<float>();
        var arr = new List<sbyte>();
    }
}

