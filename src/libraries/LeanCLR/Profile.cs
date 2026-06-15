using System.Runtime.CompilerServices;

namespace LeanCLR
{
    public static class Profile
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void ResetPeriodStats();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void EndPeriodStats();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void ResetGlobalStats();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern string GetPeriodStats();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern string GetGlobalStats();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern string GetPeriodStatsJson();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void ExportPeriodStatsJson(string path);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern string GetGlobalStatsJson();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void ExportGlobalStatsJson(string path);
    }
}
