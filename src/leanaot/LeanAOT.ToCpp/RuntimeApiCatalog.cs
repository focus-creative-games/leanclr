using System.Text.Json;

namespace LeanAOT.ToCpp
{
    public sealed class RuntimeApiEntry
    {
        public string Name { get; set; }

        public string Func { get; set; }

        public string Header { get; set; }
    }

    public sealed class RuntimeApiCatalog
    {
        private readonly IReadOnlyDictionary<string, RuntimeApiEntry> _icalls;
        private readonly IReadOnlyDictionary<string, RuntimeApiEntry> _intrinsics;
        private readonly IReadOnlyDictionary<string, RuntimeApiEntry> _icallsNewobj;
        private readonly IReadOnlyDictionary<string, RuntimeApiEntry> _intrinsicsNewobj;

        private RuntimeApiCatalog(
            IReadOnlyDictionary<string, RuntimeApiEntry> icalls,
            IReadOnlyDictionary<string, RuntimeApiEntry> intrinsics,
            IReadOnlyDictionary<string, RuntimeApiEntry> icallsNewobj,
            IReadOnlyDictionary<string, RuntimeApiEntry> intrinsicsNewobj)
        {
            _icalls = icalls;
            _intrinsics = intrinsics;
            _icallsNewobj = icallsNewobj;
            _intrinsicsNewobj = intrinsicsNewobj;
        }

        public int IcallCount => _icalls.Count;
        public int IntrinsicCount => _intrinsics.Count;
        public int IcallNewobjCount => _icallsNewobj.Count;
        public int IntrinsicNewobjCount => _intrinsicsNewobj.Count;

        public bool TryGetIcall(string name, out RuntimeApiEntry entry) => _icalls.TryGetValue(name, out entry);
        public bool TryGetIntrinsic(string name, out RuntimeApiEntry entry) => _intrinsics.TryGetValue(name, out entry);
        public bool TryGetIcallNewobj(string name, out RuntimeApiEntry entry) => _icallsNewobj.TryGetValue(name, out entry);
        public bool TryGetIntrinsicNewobj(string name, out RuntimeApiEntry entry) => _intrinsicsNewobj.TryGetValue(name, out entry);

        public static RuntimeApiCatalog LoadFromDirectory(string baseDirectory)
        {
            if (string.IsNullOrWhiteSpace(baseDirectory))
            {
                throw new ArgumentException("Base directory cannot be null or empty.", nameof(baseDirectory));
            }

            var options = new JsonSerializerOptions
            {
                PropertyNameCaseInsensitive = true
            };

            var icalls = LoadEntries(Path.Combine(baseDirectory, "icalls.json"), options);
            var intrinsics = LoadEntries(Path.Combine(baseDirectory, "intrinsics.json"), options);
            var icallsNewobj = LoadEntries(Path.Combine(baseDirectory, "icalls_newobj.json"), options);
            var intrinsicsNewobj = LoadEntries(Path.Combine(baseDirectory, "intrinsics_newobj.json"), options);

            return new RuntimeApiCatalog(icalls, intrinsics, icallsNewobj, intrinsicsNewobj);
        }

        private static IReadOnlyDictionary<string, RuntimeApiEntry> LoadEntries(string path, JsonSerializerOptions options)
        {
            if (!File.Exists(path))
            {
                throw new FileNotFoundException($"Runtime API config file not found: {path}", path);
            }

            var json = File.ReadAllText(path);
            var entries = JsonSerializer.Deserialize<List<RuntimeApiEntry>>(json, options);
            if (entries == null)
            {
                throw new InvalidDataException($"Failed to deserialize runtime API config file: {path}");
            }

            var map = new Dictionary<string, RuntimeApiEntry>(StringComparer.Ordinal);
            for (int i = 0; i < entries.Count; i++)
            {
                var entry = entries[i];
                if (entry == null || string.IsNullOrWhiteSpace(entry.Name) || string.IsNullOrWhiteSpace(entry.Func) || string.IsNullOrWhiteSpace(entry.Header))
                {
                    throw new InvalidDataException($"Invalid runtime API entry at index {i} in file: {path}");
                }

                if (!map.TryAdd(entry.Name, entry))
                {
                    throw new InvalidDataException($"Duplicate runtime API entry name '{entry.Name}' in file: {path}");
                }
            }

            return map;
        }
    }
}
