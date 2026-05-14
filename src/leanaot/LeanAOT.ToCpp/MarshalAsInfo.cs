using System.Runtime.InteropServices;

namespace LeanAOT.ToCpp
{
    public class MarshalAsInfo
    {
        public UnmanagedType UnmanagedType { get; set; }
        public UnmanagedType ArrayElementType { get; set; }
        public uint ParamIndex { get; set; }
        public uint ElementCount { get; set; }
    }
}