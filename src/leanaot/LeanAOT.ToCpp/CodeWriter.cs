using System.Diagnostics;
using System.Text;

namespace LeanAOT.ToCpp
{

    public interface ICodeBlock
    {
        public int CodeSize { get; }

        public int LineCount { get; }

        void CollectLines(List<string> lines);
    }

    public class CodeThrunkWriter : ICodeBlock
    {
        private static readonly NLog.Logger s_logger = NLog.LogManager.GetCurrentClassLogger();

        private readonly string _name;
        private readonly CodeThunkZone _parent;

        private int _codeSize;

        public string Name => _name;

        public int CodeSize => _codeSize;

        public int LineCount => _lines.Count;

        private readonly List<string> _lines = new List<string>();

        private int _indentLevel;
        private string _indentStr;

        public CodeThrunkWriter(string name, CodeThunkZone parent)
        {
            _name = name;
            _parent = parent;
            _indentLevel = 0;
            _indentStr = string.Empty;
        }

        public void IncreaseIndent()
        {
            _indentLevel++;
            _indentStr = new string(' ', _indentLevel * 4);
        }

        public void DecreaseIndent()
        {
            _indentLevel--;
            if (_indentLevel < 0)
                _indentLevel = 0;
            _indentStr = new string(' ', _indentLevel * 4);
        }

        public void SetIndent(int level)
        {
            _indentLevel = level;
            if (_indentLevel < 0)
                _indentLevel = 0;
            _indentStr = new string(' ', _indentLevel * 4);
        }

        public void BeginBlock()
        {
            AddLine("{");
            IncreaseIndent();
        }

        public void EndBlock()
        {
            DecreaseIndent();
            AddLine("}");
        }

        public void AddLine(string line)
        {
            string lineWithIndent = _indentStr + line;
            _lines.Add(lineWithIndent);
            _codeSize += lineWithIndent.Length + 1;
        }

        public void AddLines(IEnumerable<string> lines)
        {
            foreach (string line in lines)
            {
                AddLine(line);
            }
        }

        public void AddLine()
        {
            _lines.Add(string.Empty);
            _codeSize += 1;
        }

        public void AddLineIgnoreIndent(string line)
        {
            _lines.Add(line);
            _codeSize += line.Length + 1;
        }

        public void CollectLines(List<string> lines)
        {
            lines.AddRange(_lines);
        }
    }

    public class CodeThunkZone : ICodeBlock
    {
        private readonly string _name;
        private readonly CodeWriter _parent;
        private readonly List<CodeThrunkWriter> _segments = new List<CodeThrunkWriter>();
        private Dictionary<string, CodeThrunkWriter> _segmentMap = new Dictionary<string, CodeThrunkWriter>();

        public string Name => _name;
        public int CodeSize => _segments.Sum(s => s.CodeSize);
        public int LineCount => _segments.Sum(s => s.LineCount);


        public CodeThunkZone(string name, CodeWriter parent)
        {
            _name = name;
            _parent = parent;
        }

        public CodeThrunkWriter CreateThunk(string segmentName)
        {
            if (_segments.Count > 30)
            {
                throw new Exception($"Too many segments in thunk zone '{_name}'. Consider splitting into multiple zones. Current count: {_segments.Count}");
            }
            Debug.Assert(_segmentMap.ContainsKey(segmentName) == false, "Segment with the same file name already exists.");
            var segment = new CodeThrunkWriter(segmentName, this);
            _segments.Add(segment);
            _segmentMap.Add(segmentName, segment);
            return segment;
        }

        public void CollectLines(List<string> lines)
        {
            foreach (var segment in _segments)
            {
                segment.CollectLines(lines);
            }
        }

        public void MarkAsArchived()
        {
            _parent.MarkAsArchived(this);
        }
    }

    public class CodeWriter
    {
        private static readonly NLog.Logger s_logger = NLog.LogManager.GetCurrentClassLogger();

        private readonly string _fileName;

        public int CodeSize => _achivedCodeSize + _unachiveSegments.Sum(s => s.CodeSize);

        public int LineCount => _achivedLineCount + _unachiveSegments.Sum(s => s.LineCount);

        private readonly List<CodeThunkZone> _segments = new List<CodeThunkZone>();

        private readonly HashSet<CodeThunkZone> _unachiveSegments = new HashSet<CodeThunkZone>();


        private int _achivedLineCount;
        private int _achivedCodeSize;


        public CodeWriter(string fileName)
        {
            _fileName = fileName;
        }

        public CodeThunkZone CreateThunkCollection(string segmentName)
        {
            if (_unachiveSegments.Count > 30)
            {
                throw new Exception($"Too many unarchived segments in code file '{_fileName}'. Do you forget to archive some segments? Current count: {_unachiveSegments.Count}");
            }
            var segment = new CodeThunkZone(segmentName, this);
            _segments.Add(segment);
            _unachiveSegments.Add(segment);
            return segment;
        }

        public void MarkAsArchived(CodeThunkZone thunkCollection)
        {
            Debug.Assert(_unachiveSegments.Contains(thunkCollection), "The provided thunk is not in the unachived segments.");
            _unachiveSegments.Remove(thunkCollection);
            _achivedLineCount += thunkCollection.LineCount;
            _achivedCodeSize += thunkCollection.CodeSize;
        }

        void CollectLines(List<string> lines)
        {
            foreach (var segment in _segments)
            {
                segment.CollectLines(lines);
            }
        }

        public void Save()
        {
            var lines = new List<string>(LineCount);
            CollectLines(lines);
            File.WriteAllLines(_fileName, lines, new UTF8Encoding(false));
            s_logger.Info($"Saved code file: {_fileName}");
        }
    }
}
