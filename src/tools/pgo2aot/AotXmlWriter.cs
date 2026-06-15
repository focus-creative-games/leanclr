using System.Xml.Linq;

namespace Pgo2Aot;

public static class AotXmlWriter
{
    public static void Write(string outputPath, IReadOnlyList<ProfileEntry> entries)
    {
        var grouped = entries
            .GroupBy(entry => entry.Assembly, StringComparer.OrdinalIgnoreCase)
            .OrderBy(group => group.Key, StringComparer.OrdinalIgnoreCase);

        var root = new XElement("aot");
        foreach (IGrouping<string, ProfileEntry> assemblyGroup in grouped)
        {
            var assemblyElement = new XElement("assembly", new XAttribute("name", assemblyGroup.Key));
            foreach (ProfileEntry entry in assemblyGroup.OrderBy(item => item.Signature, StringComparer.Ordinal))
            {
                assemblyElement.Add(new XElement("method", new XAttribute("signature", entry.Signature)));
            }

            root.Add(assemblyElement);
        }

        var document = new XDocument(new XDeclaration("1.0", "utf-8", null), root);
        string? directory = Path.GetDirectoryName(outputPath);
        if (!string.IsNullOrEmpty(directory))
        {
            Directory.CreateDirectory(directory);
        }

        document.Save(outputPath);
    }
}
