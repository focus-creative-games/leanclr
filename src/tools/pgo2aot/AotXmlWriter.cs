using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Xml.Linq;

namespace Pgo2Aot
{
    public static class AotXmlWriter
    {
        public static void Write(string outputPath, IReadOnlyList<ProfileEntry> entries, ulong minSiteCalls = 64)
        {
            var grouped = entries
                .GroupBy(entry => entry.Assembly, StringComparer.OrdinalIgnoreCase)
                .OrderBy(group => group.Key, StringComparer.OrdinalIgnoreCase);

            var root = new XElement("aot");
            root.Add(new XAttribute("selected-method-count", entries.Count),
                new XAttribute("selected-il-bytes", entries.Sum(e => (decimal)e.IlBytes)), new XAttribute("receiver-policy", "bounded-reuse-v1"),
                new XAttribute("min-site-calls", minSiteCalls));
            foreach (IGrouping<string, ProfileEntry> assemblyGroup in grouped)
            {
                var assemblyElement = new XElement("assembly", new XAttribute("name", assemblyGroup.Key));
                foreach (ProfileEntry entry in assemblyGroup.OrderBy(item => item.Signature, StringComparer.Ordinal))
                {
                    var method = new XElement("method", new XAttribute("signature", entry.Signature),
                        new XAttribute("calls", entry.Calls), new XAttribute("cost", entry.Cost));
                    if (!string.IsNullOrEmpty(entry.IlHash))
                    {
                        method.Add(new XAttribute("il-hash", entry.IlHash), new XAttribute("il-bytes", entry.IlBytes));
                        int words = 0;
                        foreach (var site in entry.InterfaceSites.OrderByDescending(s => s.Calls).ThenBy(s => s.IlOffset))
                        {
                            int capacity = ReceiverPolicy.Capacity(site, minSiteCalls);
                            if (capacity == 0 || words + capacity * 3 > 12) continue;
                            words += capacity * 3;
                            method.Add(new XElement("interface-cache", new XAttribute("il-offset", site.IlOffset),
                                new XAttribute("entries", capacity), new XAttribute("calls", site.Calls)));
                        }
                    }
                    assemblyElement.Add(method);
                }

                root.Add(assemblyElement);
            }

            var document = new XDocument(new XDeclaration("1.0", "utf-8", null), root);
            string directory = Path.GetDirectoryName(outputPath);
            if (!string.IsNullOrEmpty(directory))
            {
                Directory.CreateDirectory(directory);
            }

            document.Save(outputPath);
        }
    }
}
