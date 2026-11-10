using System;
using System.Diagnostics;
using System.IO;
public static class HostServicesFixture
{
    public static int Run(IntPtr handle)
    {
        if(DateTime.UtcNow.Ticks != 621355968000000000L + 1234567890L) return 1;
        if(Stopwatch.GetTimestamp() != 9876543210L) return 2;
        if(Environment.TickCount != 987654) return 3;
        using(var stream=new FileStream(handle,FileAccess.ReadWrite,true,16))
        {
            if(stream.Length != 3) return 4;
            if(stream.ReadByte() != 65) return 5;
            stream.Position=0;
            stream.WriteByte(90);
            stream.Flush();
            stream.Position=0;
            if(stream.ReadByte()!=90) return 6;
        }
        if(Guid.NewGuid() == Guid.Empty) return 7;
        return 0;
    }
    public static int EntropyFailure()
    {
        try { Guid.NewGuid(); return 1; }
        catch(InvalidOperationException) { return 0; }
    }
}
