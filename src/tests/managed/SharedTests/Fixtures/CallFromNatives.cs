using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AOTDefs
{
    public abstract class  NativeBase
    {
        public abstract void Call();

        public abstract int Call(int x);
    }

    public class CallFromNatives
    {
        public static void Call(NativeBase native)
        {
            native.Call();
            native.Call();
            native.Call();
            native.Call();
            native.Call();
            native.Call();
            native.Call();
            native.Call();
            native.Call();
            native.Call();
        }

        public static int Call(NativeBase native, int x)
        {
            native.Call(x);
            native.Call(x);
            native.Call(x);
            native.Call(x);
            native.Call(x);
            native.Call(x);
            native.Call(x);
            native.Call(x);
            native.Call(x);
            return native.Call(x);
        }
    }

    
}
