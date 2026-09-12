using System.Buffers.Binary;

namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static unsafe void RunNet10SpanBinaryPrimitives()
        {
            byte[] data = new byte[] { 0x01, 0x00, 0x62, 0x04, 0x00, 0x00 };
            fixed (byte* pData = data)
            {
                System.ReadOnlySpan<byte> blob = new System.ReadOnlySpan<byte>(pData, data.Length);
                Assert.Equal(6, blob.Length);
                Assert.Equal(1, (int)blob[0]);
                Assert.Equal(0, (int)blob[1]);

                System.ReadOnlySpan<byte> prolog = blob.Slice(0, 2);
                Assert.Equal(2, prolog.Length);
                Assert.Equal(1, (int)prolog[0]);
                Assert.Equal(0, (int)prolog[1]);
                Assert.Equal(1, (int)BinaryPrimitives.ReadUInt16LittleEndian(prolog));

                System.ReadOnlySpan<byte> arg = blob.Slice(2, 2);
                Assert.Equal(2, arg.Length);
                Assert.Equal(0x62, (int)arg[0]);
                Assert.Equal(0x04, (int)arg[1]);
                Assert.Equal(1122, (int)BinaryPrimitives.ReadUInt16LittleEndian(arg));

                Assert.Equal(0, (int)BinaryPrimitives.ReadUInt16LittleEndian(blob.Slice(4, 2)));
            }
        }
    }
}
