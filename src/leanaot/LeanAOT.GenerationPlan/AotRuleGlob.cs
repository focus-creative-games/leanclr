using System.Text;
using System.Text.RegularExpressions;

namespace LeanAOT.GenerationPlan
{
    /// <summary>
    /// Glob subset for AOT rule XML: <c>*</c> (any length), <c>?</c> (one char). Used with <see cref="Regex"/>; patterns are anchored full match.
    /// </summary>
    internal static class AotRuleGlob
    {
        internal static bool IsMatch(string pattern, string text, bool ignoreCase)
        {
            if (pattern == null || text == null)
                return false;
            var rx = GlobToRegex(pattern, ignoreCase);
            return rx.IsMatch(text);
        }

        internal static Regex GlobToRegex(string pattern, bool ignoreCase)
        {
            var sb = new StringBuilder(pattern.Length + 16);
            sb.Append('^');
            foreach (var c in pattern)
            {
                switch (c)
                {
                    case '*':
                        sb.Append(".*");
                        break;
                    case '?':
                        sb.Append('.');
                        break;
                    case '\\':
                    case '.':
                    case '+':
                    case '(':
                    case ')':
                    case '[':
                    case ']':
                    case '{':
                    case '}':
                    case '^':
                    case '$':
                    case '|':
                        sb.Append('\\').Append(c);
                        break;
                    default:
                        sb.Append(c);
                        break;
                }
            }
            sb.Append('$');
            var opt = RegexOptions.CultureInvariant;
            if (ignoreCase)
                opt |= RegexOptions.IgnoreCase;
            return new Regex(sb.ToString(), opt);
        }

        internal static Regex CompileGlob(string pattern, bool ignoreCase) => GlobToRegex(pattern, ignoreCase);
    }
}
