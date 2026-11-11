using dnlib.DotNet;

namespace LeanAOT.GenerationPlan
{
    public static class AotProfileFingerprint
    {
        public static (string hash, uint bytes) Compute(MethodDef method)
        {
            if (method.Module is not ModuleDefMD module || !method.HasBody) return ("", 0);
            var reader = module.Metadata.PEImage.CreateReader(module.Metadata.PEImage.ToFileOffset(method.RVA));
            uint start = reader.Position;
            byte first = reader.ReadByte();
            uint bytes;
            if ((first & 3) == 2) bytes = (uint)(first >> 2);
            else if ((first & 3) == 3)
            {
                uint header = (uint)(reader.ReadByte() >> 4) * 4;
                reader.ReadUInt16();
                bytes = reader.ReadUInt32();
                if (header < 12) return ("", 0);
                reader.Position = start + header;
            }
            else return ("", 0);
            if (bytes > reader.Length - reader.Position) return ("", 0);
            ulong hash = 14695981039346656037UL;
            for (uint i = 0; i < bytes; ++i) hash = unchecked((hash ^ reader.ReadByte()) * 1099511628211UL);
            return (hash.ToString("X16"), bytes);
        }
    }
}
