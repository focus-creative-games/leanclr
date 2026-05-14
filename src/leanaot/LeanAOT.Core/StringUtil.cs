using System.Text;

namespace LeanAOT.Core
{

    public static class StringUtil
    {
        public static string EscapeCppString(string value)
        {
            var sb = new StringBuilder();
            foreach (char ch in value)
            {
                switch (ch)
                {
                case '\\': sb.Append("\\\\"); break;
                case '"': sb.Append("\\\""); break;
                case '\n': sb.Append("\\n"); break;
                case '\r': sb.Append("\\r"); break;
                case '\t': sb.Append("\\t"); break;
                default: sb.Append(ch); break;
                }
            }
            return sb.ToString();
        }

        public static string StandardizedDllName(string value)
        {
            var sb = new StringBuilder();
            foreach (char ch in value)
            {
                if (char.IsLetterOrDigit(ch) || ch == '_')
                {
                    sb.Append(ch);
                }
                else
                {
                    sb.Append('_');
                }
            }
            return sb.ToString();
        }
    }
}