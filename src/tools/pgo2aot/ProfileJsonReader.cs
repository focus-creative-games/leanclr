using System.Text.Json;

namespace Pgo2Aot;

public static class ProfileJsonReader
{
    private static readonly JsonSerializerOptions s_jsonOptions = new()
    {
        PropertyNameCaseInsensitive = true,
    };

    public static IReadOnlyList<ProfileEntry> LoadAndMerge(IEnumerable<string> inputPaths)
    {
        var merged = new Dictionary<string, ProfileEntry>(StringComparer.Ordinal);

        foreach (string inputPath in inputPaths)
        {
            string fullPath = Path.GetFullPath(inputPath);
            if (!File.Exists(fullPath))
            {
                throw new FileNotFoundException($"Profile JSON not found: {fullPath}", fullPath);
            }

            string json = File.ReadAllText(fullPath);
            ProfileEntry[]? entries = JsonSerializer.Deserialize<ProfileEntry[]>(json, s_jsonOptions);
            if (entries == null)
            {
                throw new InvalidDataException($"Profile JSON is empty or invalid: {fullPath}");
            }

            foreach (ProfileEntry entry in entries)
            {
                if (string.IsNullOrWhiteSpace(entry.Assembly))
                {
                    throw new InvalidDataException($"Profile entry missing assembly in {fullPath}");
                }

                if (string.IsNullOrWhiteSpace(entry.Signature))
                {
                    throw new InvalidDataException($"Profile entry missing signature in {fullPath}");
                }

                string key = $"{entry.Assembly}\0{entry.Signature}";
                if (merged.TryGetValue(key, out ProfileEntry? existing))
                {
                    merged[key] = new ProfileEntry
                    {
                        Assembly = entry.Assembly,
                        Signature = entry.Signature,
                        Calls = Math.Max(existing.Calls, entry.Calls),
                        Cost = Math.Max(existing.Cost, entry.Cost),
                    };
                }
                else
                {
                    merged[key] = entry;
                }
            }
        }

        return merged.Values.ToList();
    }
}
