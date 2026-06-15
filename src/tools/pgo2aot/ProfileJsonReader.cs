using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
#if UNITY_EDITOR
using UnityEngine;
#else
using System.Text.Json;
#endif

namespace Pgo2Aot
{
    public static class ProfileJsonReader
    {
#if !UNITY_EDITOR
        private static readonly JsonSerializerOptions s_jsonOptions = new JsonSerializerOptions
        {
            PropertyNameCaseInsensitive = true,
        };
#endif

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
                ProfileEntry[] entries = ParseProfileJson(json, fullPath);

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
                    if (merged.TryGetValue(key, out ProfileEntry existing))
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

        private static ProfileEntry[] ParseProfileJson(string json, string sourcePath)
        {
#if UNITY_EDITOR
            return ParseProfileJsonUnity(json, sourcePath);
#else
            ProfileEntry[] entries = JsonSerializer.Deserialize<ProfileEntry[]>(json, s_jsonOptions);
            if (entries == null)
            {
                throw new InvalidDataException($"Profile JSON is empty or invalid: {sourcePath}");
            }

            return entries;
#endif
        }

#if UNITY_EDITOR
        private static ProfileEntry[] ParseProfileJsonUnity(string json, string sourcePath)
        {
            string trimmed = json.Trim();
            if (trimmed.Length == 0)
            {
                throw new InvalidDataException($"Profile JSON is empty: {sourcePath}");
            }

            if (trimmed == "[]")
            {
                return Array.Empty<ProfileEntry>();
            }

            if (!trimmed.StartsWith("[") || !trimmed.EndsWith("]"))
            {
                throw new InvalidDataException($"Profile JSON must be a top-level array: {sourcePath}");
            }

            string wrapped = "{\"items\":" + trimmed + "}";
            ProfileEntryJsonListDto dto = JsonUtility.FromJson<ProfileEntryJsonListDto>(wrapped);
            if (dto == null || dto.items == null || dto.items.Length == 0)
            {
                throw new InvalidDataException($"Profile JSON is empty or invalid: {sourcePath}");
            }

            var entries = new ProfileEntry[dto.items.Length];
            for (int i = 0; i < dto.items.Length; i++)
            {
                entries[i] = dto.items[i].ToProfileEntry();
            }

            return entries;
        }

        [Serializable]
        private sealed class ProfileEntryJsonDto
        {
            public string assembly;
            public string signature;
            public long calls;
            public long cost;

            public ProfileEntry ToProfileEntry()
            {
                return new ProfileEntry
                {
                    Assembly = assembly ?? string.Empty,
                    Signature = signature ?? string.Empty,
                    Calls = calls > 0 ? (ulong)calls : 0,
                    Cost = cost > 0 ? (ulong)cost : 0,
                };
            }
        }

        [Serializable]
        private sealed class ProfileEntryJsonListDto
        {
            public ProfileEntryJsonDto[] items;
        }
#endif
    }
}
