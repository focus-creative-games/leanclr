using System.Runtime.InteropServices;

namespace CoreTests
{
    public class CustomPInvoke
    {
        [DllImport("CustomNativeLib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Add(int a, int b);
    }
}
