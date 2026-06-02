using System;

namespace CorlibTests.InternalCall
{
    /// <summary>
    /// Comprehensive tests for System.String instance and static methods.
    /// Targets .NET Framework 4.7.2 / mscorlib semantics.
    /// </summary>
    internal class TC_System_String : TestCaseBase
    {
        public static int GetSceneName(string sceneName)
        {
            return sceneName.LastIndexOf(".unity");
        }

        #region Existing / integration

        [UnitTest]
        public void GetSceneNameTest()
        {
            string sceneAssetName = "MainScene.unity";
            int sceneNamePosition = GetSceneName(sceneAssetName);
            Assert.Equal(9, sceneNamePosition);
        }

        #endregion

        #region Compare

        [UnitTest]
        public void Compare_EqualStrings_ReturnsZero()
        {
            Assert.Equal(0, string.Compare("abc", "abc"));
            Assert.Equal(0, string.Compare("", ""));
            Assert.Equal(0, string.Compare("a", "a"));
        }

        [UnitTest]
        public void Compare_LexicographicOrder()
        {
            Assert.IsTrue(string.Compare("a", "b") < 0);
            Assert.IsTrue(string.Compare("b", "a") > 0);
            Assert.IsTrue(string.Compare("abc", "abd") < 0);
            Assert.IsTrue(string.Compare("abd", "abc") > 0);
        }

        [UnitTest]
        public void Compare_PrefixAndLength()
        {
            Assert.IsTrue(string.Compare("abc", "abcd") < 0);
            Assert.IsTrue(string.Compare("abcd", "abc") > 0);
            Assert.Equal(0, string.Compare("abc", 0, "abcd", 0, 3));
            Assert.Equal(0, string.Compare("abcd", 0, "abc", 0, 3));
        }

        [UnitTest]
        public void Compare_WithStartIndexAndLength()
        {
            Assert.Equal(0, string.Compare("xxabc", 2, "yyabc", 2, 3));
            Assert.IsTrue(string.Compare("xxabc", 2, "yyabd", 2, 3) < 0);
            Assert.IsTrue(string.Compare("xxabd", 2, "yyabc", 2, 3) > 0);
        }

        [UnitTest]
        public void Compare_NullArguments()
        {
            Assert.Equal(0, string.Compare(null, null));
            Assert.IsTrue(string.Compare(null, "a") < 0);
            Assert.IsTrue(string.Compare("a", null) > 0);
            Assert.IsTrue(string.Compare(null, "") < 0);
            Assert.IsTrue(string.Compare("", null) > 0);
        }

        [UnitTest]
        public void Compare_CaseSensitiveDefault()
        {
            Assert.IsTrue(string.Compare("a", "A") != 0);
            Assert.IsTrue(string.Compare("abc", "ABC") != 0);
        }

        [UnitTest]
        public void Compare_StringComparison_Ordinal()
        {
            Assert.Equal(0, string.Compare("a", "a", StringComparison.Ordinal));
            Assert.IsTrue(string.Compare("a", "A", StringComparison.Ordinal) > 0);
            Assert.IsTrue(string.Compare("A", "a", StringComparison.Ordinal) < 0);
        }

        [UnitTest]
        public void Compare_StringComparison_OrdinalIgnoreCase()
        {
            Assert.Equal(0, string.Compare("abc", "ABC", StringComparison.OrdinalIgnoreCase));
            Assert.Equal(0, string.Compare("AbC", "aBc", StringComparison.OrdinalIgnoreCase));
            Assert.IsTrue(string.Compare("a", "b", StringComparison.OrdinalIgnoreCase) < 0);
        }

        [UnitTest]
        public void Compare_StringComparison_WithIndexAndLength()
        {
            Assert.Equal(0, string.Compare("xxabc", 2, "yyabc", 2, 3, StringComparison.Ordinal));
            Assert.Equal(0, string.Compare("xxAbC", 2, "yyabc", 2, 3, StringComparison.OrdinalIgnoreCase));
        }

        [UnitTest]
        public void CompareOrdinal()
        {
            Assert.Equal(0, string.CompareOrdinal("abc", "abc"));
            Assert.IsTrue(string.CompareOrdinal("a", "b") < 0);
            Assert.IsTrue(string.CompareOrdinal("a", "A") > 0);
            Assert.Equal(0, string.CompareOrdinal("abc", 0, "abcd", 0, 3));
        }

        [UnitTest]
        public void Compare_UnicodeAndSurrogates()
        {
            string s1 = "\uD800\uDC00"; // surrogate pair U+10000
            string s2 = "\uD800\uDC01";
            Assert.IsTrue(string.Compare(s1, s2, StringComparison.Ordinal) < 0);
            Assert.Equal(0, string.Compare(s1, s1, StringComparison.Ordinal));
        }

        #endregion

        #region Concat

        [UnitTest]
        public void Concat_TwoStrings()
        {
            Assert.Equal("ab", string.Concat("a", "b"));
            Assert.Equal("hello", string.Concat("hel", "lo"));
        }

        [UnitTest]
        public void Concat_TwoStrings_NullTreatedAsEmpty()
        {
            Assert.Equal("a", string.Concat(null, "a"));
            Assert.Equal("a", string.Concat("a", null));
            Assert.Equal("", string.Concat(null, null));
        }

        [UnitTest]
        public void Concat_ThreeAndFour()
        {
            Assert.Equal("abcd", string.Concat("a", "b", "c", "d"));
            Assert.Equal("xyz", string.Concat("x", "y", "z"));
        }

        [UnitTest]
        public void Concat_ParamsStringArray()
        {
            Assert.Equal("abc", string.Concat(new string[] { "a", "b", "c" }));
            Assert.Equal("ac", string.Concat(new string[] { "a", null, "c" }));
            Assert.Equal("", string.Concat(new string[0]));
            Assert.Equal("solo", string.Concat(new string[] { "solo" }));
        }

        [UnitTest]
        public void Concat_ParamsObjectArray()
        {
            Assert.Equal("123", string.Concat(new object[] { 1, 2, 3 }));
            Assert.Equal("aTrue", string.Concat(new object[] { "a", true }));
            Assert.Equal("x", string.Concat(new object[] { "x" }));
            Assert.Equal("", string.Concat(new object[] { null }));
            Assert.Equal("ab", string.Concat(new object[] { "a", null, "b" }));
        }

        [UnitTest]
        public void Concat_EmptyAndSingleChar()
        {
            Assert.Equal("", string.Concat("", ""));
            Assert.Equal("!!", string.Concat("!", "!"));
        }

        #endregion

        #region Contains

        [UnitTest]
        public void Contains_Found()
        {
            Assert.IsTrue("hello".Contains("ell"));
            Assert.IsTrue("hello".Contains("hello"));
            Assert.IsTrue("hello".Contains("h"));
            Assert.IsTrue("hello".Contains("o"));
        }

        [UnitTest]
        public void Contains_NotFound()
        {
            Assert.IsFalse("hello".Contains("world"));
            Assert.IsFalse("hello".Contains("Hello"));
            Assert.IsFalse("".Contains("a"));
            Assert.IsFalse("a".Contains("ab"));
        }

        [UnitTest]
        public void Contains_EmptyValue()
        {
            Assert.IsTrue("".Contains(""));
            Assert.IsTrue("abc".Contains(""));
        }

        [UnitTest]
        public void Contains_NullValue_Throws()
        {
            Assert.ExpectException<ArgumentNullException>(() => "abc".Contains(null));
        }

        [UnitTest]
        public void Contains_OverlappingPattern()
        {
            Assert.IsTrue("aaaa".Contains("aaa"));
            Assert.IsTrue("ababab".Contains("aba"));
        }

        #endregion

        #region EndsWith

        [UnitTest]
        public void EndsWith_Match()
        {
            Assert.IsTrue("hello".EndsWith("lo"));
            Assert.IsTrue("hello".EndsWith("hello"));
            Assert.IsTrue("a".EndsWith("a"));
            Assert.IsTrue("".EndsWith(""));
        }

        [UnitTest]
        public void EndsWith_NoMatch()
        {
            Assert.IsFalse("hello".EndsWith("hell"));
            Assert.IsFalse("hello".EndsWith("Hello"));
            Assert.IsFalse("hello".EndsWith("hello!"));
            Assert.IsFalse("ab".EndsWith("abc"));
        }

        [UnitTest]
        public void EndsWith_StringComparison_OrdinalIgnoreCase()
        {
            Assert.IsTrue("hello".EndsWith("LO", StringComparison.OrdinalIgnoreCase));
            Assert.IsTrue("Hello".EndsWith("lo", StringComparison.OrdinalIgnoreCase));
            Assert.IsFalse("hello".EndsWith("LA", StringComparison.OrdinalIgnoreCase));
        }

        [UnitTest]
        public void EndsWith_StringComparison_Ordinal()
        {
            Assert.IsTrue("hello".EndsWith("lo", StringComparison.Ordinal));
            Assert.IsFalse("hello".EndsWith("LO", StringComparison.Ordinal));
        }

        [UnitTest]
        public void EndsWith_LongerSuffixThanString()
        {
            Assert.IsFalse("a".EndsWith("ab"));
            Assert.IsFalse("".EndsWith("a"));
        }

        [UnitTest]
        public void EndsWith_NullSuffix_Throws()
        {
            Assert.ExpectException<ArgumentNullException>(() => "abc".EndsWith(null));
        }

        #endregion

        #region Equals

        [UnitTest]
        public void Equals_Instance_ValueEqual()
        {
            string a = "test";
            string b = string.Concat("te", "st");
            Assert.IsTrue(a.Equals(b));
            Assert.IsTrue(b.Equals(a));
            Assert.IsTrue(a.Equals("test"));
        }

        [UnitTest]
        public void Equals_Instance_NotEqual()
        {
            Assert.IsFalse("abc".Equals("abd"));
            Assert.IsFalse("abc".Equals("ABC"));
            Assert.IsFalse("".Equals(" "));
        }

        [UnitTest]
        public void Equals_Instance_Null()
        {
            string s = "x";
            Assert.IsFalse(s.Equals(null));
            Assert.IsFalse("".Equals(null));
        }

        [UnitTest]
        public void Equals_Static()
        {
            Assert.IsTrue(string.Equals("a", "a"));
            Assert.IsFalse(string.Equals("a", "b"));
            Assert.IsTrue(string.Equals(null, null));
            Assert.IsFalse(string.Equals(null, "a"));
            Assert.IsFalse(string.Equals("a", null));
        }

        [UnitTest]
        public void Equals_StringComparison_OrdinalIgnoreCase()
        {
            Assert.IsTrue(string.Equals("abc", "ABC", StringComparison.OrdinalIgnoreCase));
            Assert.IsFalse(string.Equals("abc", "abd", StringComparison.OrdinalIgnoreCase));
            Assert.IsTrue(string.Equals(null, null, StringComparison.Ordinal));
            Assert.IsFalse(string.Equals(null, "a", StringComparison.Ordinal));
        }

        [UnitTest]
        public void Equals_Operator()
        {
            Assert.IsTrue("a" == "a");
            Assert.IsFalse("a" == "b");
            Assert.IsTrue(null == (string)null);
            Assert.IsFalse("a" == null);
        }

        [UnitTest]
        public void Equals_ReferenceEqualsInterned()
        {
            string a = string.Intern("intern_me");
            string b = string.Intern("intern_me");
            Assert.IsTrue(object.ReferenceEquals(a, b));
            Assert.IsTrue(a.Equals(b));
        }

        #endregion

        #region Insert

        [UnitTest]
        public void Insert_BeginningMiddleEnd()
        {
            Assert.Equal("Xabc", "abc".Insert(0, "X"));
            Assert.Equal("aXbc", "abc".Insert(1, "X"));
            Assert.Equal("abcX", "abc".Insert(3, "X"));
        }

        [UnitTest]
        public void Insert_EmptyValue()
        {
            Assert.Equal("abc", "abc".Insert(1, ""));
            Assert.Equal("abc", "abc".Insert(0, ""));
            Assert.Equal("abc", "abc".Insert(3, ""));
        }

        [UnitTest]
        public void Insert_IntoEmpty()
        {
            Assert.Equal("hello", "".Insert(0, "hello"));
        }

        [UnitTest]
        public void Insert_LongerValue()
        {
            Assert.Equal("fiprex", "fix".Insert(2, "pre"));
            Assert.Equal("0123", "03".Insert(1, "12"));
            Assert.Equal("world", "ld".Insert(0, "wor"));
        }

        [UnitTest]
        public void Insert_NullValue_Throws()
        {
            Assert.ExpectException<ArgumentNullException>(() => "abc".Insert(0, null));
        }

        [UnitTest]
        public void Insert_IndexOutOfRange_Throws()
        {
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "abc".Insert(-1, "x"));
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "abc".Insert(4, "x"));
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "".Insert(1, "x"));
        }

        #endregion

        #region IndexOf

        [UnitTest]
        public void IndexOf_Char()
        {
            Assert.Equal(0, "abc".IndexOf('a'));
            Assert.Equal(2, "abc".IndexOf('c'));
            Assert.Equal(-1, "abc".IndexOf('z'));
            Assert.Equal(0, "aaa".IndexOf('a'));
        }

        [UnitTest]
        public void IndexOf_Char_WithStartIndex()
        {
            Assert.Equal(3, "abca".IndexOf('a', 1));
            Assert.Equal(-1, "abc".IndexOf('a', 3));
            Assert.Equal(3, "abca".IndexOf('a', 2));
        }

        [UnitTest]
        public void IndexOf_Char_WithStartIndexAndCount()
        {
            Assert.Equal(2, "abaca".IndexOf('a', 1, 3));
            Assert.Equal(2, "abaca".IndexOf('a', 1, 2));
            Assert.Equal(2, "abaca".IndexOf('a', 2, 3));
        }

        [UnitTest]
        public void IndexOf_String()
        {
            Assert.Equal(1, "banana".IndexOf("ana"));
            Assert.Equal(0, "banana".IndexOf("ban"));
            Assert.Equal(-1, "banana".IndexOf("xyz"));
            Assert.Equal(0, "hello".IndexOf("hello"));
        }

        [UnitTest]
        public void IndexOf_String_WithStartIndex()
        {
            Assert.Equal(3, "banana".IndexOf("ana", 2));
            Assert.Equal(-1, "banana".IndexOf("ban", 1));
            Assert.Equal(1, "banana".IndexOf("ana", 1));
        }

        [UnitTest]
        public void IndexOf_String_WithStartIndexAndCount()
        {
            Assert.Equal(1, "banana".IndexOf("ana", 1, 4));
            Assert.Equal(-1, "banana".IndexOf("ana", 1, 2));
            Assert.Equal(2, "banana".IndexOf("na", 2, 3));
        }

        [UnitTest]
        public void IndexOf_EmptyString()
        {
            Assert.Equal(0, "abc".IndexOf(""));
            Assert.Equal(1, "abc".IndexOf("", 1));
            Assert.Equal(3, "abc".IndexOf("", 3));
            Assert.Equal(0, "".IndexOf(""));
        }

        [UnitTest]
        public void IndexOf_OverlappingMatches()
        {
            Assert.Equal(0, "aaaa".IndexOf("aa"));
            Assert.Equal(0, "aaaa".IndexOf("aa", 0));
            Assert.Equal(1, "aaaa".IndexOf("aa", 1));
        }

        [UnitTest]
        public void IndexOf_StringComparison_OrdinalIgnoreCase()
        {
            Assert.Equal(0, "Hello".IndexOf("he", StringComparison.OrdinalIgnoreCase));
            Assert.Equal(2, "Hello".IndexOf("LL", StringComparison.OrdinalIgnoreCase));
            Assert.Equal(-1, "Hello".IndexOf("xyz", StringComparison.OrdinalIgnoreCase));
        }

        [UnitTest]
        public void IndexOf_StringComparison_WithStartIndex()
        {
            Assert.Equal(4, "Hello World".IndexOf("O", 4, StringComparison.OrdinalIgnoreCase));
            Assert.Equal(-1, "Hello".IndexOf("H", 1, StringComparison.Ordinal));
        }

        [UnitTest]
        public void IndexOf_NullNeedle_Throws()
        {
            Assert.ExpectException<ArgumentNullException>(() => "abc".IndexOf((string)null));
        }

        [UnitTest]
        public void IndexOf_InvalidStartIndexOrCount_Throws()
        {
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "abc".IndexOf('a', -1));
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "abc".IndexOf('a', 4));
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "abc".IndexOf('a', 1, -1));
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "abc".IndexOf('a', 2, 2));
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "abc".IndexOf("a", -1));
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "abc".IndexOf("a", 4));
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "abc".IndexOf("a", 1, -1));
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "abc".IndexOf("a", 2, 2));
        }

        [UnitTest]
        public void IndexOf_SurrogateChar()
        {
            string s = "a\uD800\uDC00b";
            Assert.Equal(1, s.IndexOf('\uD800'));
            Assert.Equal(2, s.IndexOf('\uDC00'));
        }

        #endregion

        #region LastIndexOf

        [UnitTest]
        public void LastIndexOf_Char()
        {
            Assert.Equal(4, "abaca".LastIndexOf('a'));
            Assert.Equal(2, "abc".LastIndexOf('c'));
            Assert.Equal(-1, "abc".LastIndexOf('z'));
        }

        [UnitTest]
        public void LastIndexOf_Char_WithStartIndex()
        {
            Assert.Equal(2, "abaca".LastIndexOf('a', 3));
            Assert.Equal(0, "abaca".LastIndexOf('a', 1));
            Assert.Equal(-1, "abc".LastIndexOf('c', 1));
        }

        [UnitTest]
        public void LastIndexOf_Char_WithStartIndexAndCount()
        {
            Assert.Equal(2, "abaca".LastIndexOf('a', 3, 3));
            Assert.Equal(0, "abaca".LastIndexOf('a', 1, 2));
            Assert.Equal(-1, "abaca".LastIndexOf('a', 1, 1));
        }

        [UnitTest]
        public void LastIndexOf_String()
        {
            Assert.Equal(3, "banana".LastIndexOf("ana"));
            Assert.Equal(0, "banana".LastIndexOf("ban"));
            Assert.Equal(-1, "banana".LastIndexOf("xyz"));
        }

        [UnitTest]
        public void LastIndexOf_String_WithStartIndex()
        {
            Assert.Equal(-1, "banana".LastIndexOf("ana", 2));
            Assert.Equal(0, "banana".LastIndexOf("ban", 2));
            Assert.Equal(-1, "banana".LastIndexOf("ana", 0));
        }

        [UnitTest]
        public void LastIndexOf_String_WithStartIndexAndCount()
        {
            Assert.Equal(1, "banana".LastIndexOf("ana", 4, 4));
            Assert.Equal(-1, "banana".LastIndexOf("ana", 0, 1));
            Assert.Equal(-1, "banana".LastIndexOf("na", 4, 2));
        }

        [UnitTest]
        public void LastIndexOf_EmptyString()
        {
            Assert.Equal(2, "abc".LastIndexOf(""));
            Assert.Equal(4, "hello".LastIndexOf(""));
            Assert.Equal(0, "".LastIndexOf(""));
            Assert.Equal(2, "abc".LastIndexOf("", 2));
        }

        [UnitTest]
        public void LastIndexOf_OverlappingMatches()
        {
            Assert.Equal(2, "aaaa".LastIndexOf("aa"));
            Assert.Equal(1, "aaaa".LastIndexOf("aa", 2));
        }

        [UnitTest]
        public void LastIndexOf_StringComparison_OrdinalIgnoreCase()
        {
            Assert.Equal(7, "Hello World".LastIndexOf("O", StringComparison.OrdinalIgnoreCase));
            Assert.Equal(2, "Hello".LastIndexOf("LL", StringComparison.OrdinalIgnoreCase));
            Assert.Equal(-1, "Hello".LastIndexOf("xyz", StringComparison.OrdinalIgnoreCase));
        }

        [UnitTest]
        public void LastIndexOf_StringComparison_WithStartIndex()
        {
            Assert.Equal(0, "Hello World".LastIndexOf("Hello", 4, StringComparison.OrdinalIgnoreCase));
            Assert.Equal(-1, "Hello".LastIndexOf("o", 2, StringComparison.OrdinalIgnoreCase));
        }

        [UnitTest]
        public void LastIndexOf_SceneNamePattern()
        {
            Assert.Equal(9, "MainScene.unity".LastIndexOf(".unity"));
            Assert.Equal(-1, "MainScene.unity".LastIndexOf(".scene"));
            Assert.Equal(9, "a.unity.b.unity".LastIndexOf(".unity"));
        }

        [UnitTest]
        public void LastIndexOf_NullValue_Throws()
        {
            Assert.ExpectException<ArgumentNullException>(() => "abc".LastIndexOf((string)null));
        }

        [UnitTest]
        public void LastIndexOf_InvalidStartIndexOrCount_Throws()
        {
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "abc".LastIndexOf('a', -1));
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "abc".LastIndexOf('a', 3));
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "abc".LastIndexOf('a', 1, -1));
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "abc".LastIndexOf('a', 1, 3));
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "abc".LastIndexOf("a", -1));
            //Assert.ExpectException<ArgumentOutOfRangeException>(() => "abc".LastIndexOf("a", 3));
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "abc".LastIndexOf("a", 1, -1));
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "abc".LastIndexOf("a", 1, 3));
        }

        #endregion

        #region StartsWith

        [UnitTest]
        public void StartsWith_Match()
        {
            Assert.IsTrue("hello".StartsWith("he"));
            Assert.IsTrue("hello".StartsWith("hello"));
            Assert.IsTrue("a".StartsWith("a"));
            Assert.IsTrue("".StartsWith(""));
        }

        [UnitTest]
        public void StartsWith_NoMatch()
        {
            Assert.IsFalse("hello".StartsWith("lo"));
            Assert.IsFalse("hello".StartsWith("Hello"));
            Assert.IsFalse("hello".StartsWith("hello!"));
            Assert.IsFalse("ab".StartsWith("abc"));
        }

        [UnitTest]
        public void StartsWith_StringComparison_OrdinalIgnoreCase()
        {
            Assert.IsTrue("hello".StartsWith("HE", StringComparison.OrdinalIgnoreCase));
            Assert.IsTrue("Hello".StartsWith("he", StringComparison.OrdinalIgnoreCase));
            Assert.IsFalse("hello".StartsWith("LA", StringComparison.OrdinalIgnoreCase));
        }

        [UnitTest]
        public void StartsWith_StringComparison_Ordinal()
        {
            Assert.IsTrue("hello".StartsWith("he", StringComparison.Ordinal));
            Assert.IsFalse("hello".StartsWith("HE", StringComparison.Ordinal));
        }

        [UnitTest]
        public void StartsWith_LongerPrefixThanString()
        {
            Assert.IsFalse("a".StartsWith("ab"));
            Assert.IsFalse("".StartsWith("a"));
        }

        [UnitTest]
        public void StartsWith_NullPrefix_Throws()
        {
            Assert.ExpectException<ArgumentNullException>(() => "abc".StartsWith(null));
        }

        #endregion

        #region Substring

        [UnitTest]
        public void Substring_FromStartIndex()
        {
            Assert.Equal("bc", "abc".Substring(1));
            Assert.Equal("abc", "abc".Substring(0));
            Assert.Equal("", "abc".Substring(3));
            Assert.Equal("lo", "hello".Substring(3));
        }

        [UnitTest]
        public void Substring_WithLength()
        {
            Assert.Equal("b", "abc".Substring(1, 1));
            Assert.Equal("bc", "abc".Substring(1, 2));
            Assert.Equal("", "abc".Substring(2, 0));
            Assert.Equal("abc", "abc".Substring(0, 3));
        }

        [UnitTest]
        public void Substring_SingleCharAndEmptySource()
        {
            Assert.Equal("x", "x".Substring(0));
            Assert.Equal("", "x".Substring(1));
            Assert.Equal("", "".Substring(0));
        }

        [UnitTest]
        public void Substring_StartIndexOutOfRange_Throws()
        {
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "abc".Substring(-1));
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "abc".Substring(4));
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "".Substring(1));
        }

        [UnitTest]
        public void Substring_LengthOutOfRange_Throws()
        {
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "abc".Substring(1, -1));
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "abc".Substring(1, 3));
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "abc".Substring(2, 2));
        }

        #endregion

        #region ToCharArray

        [UnitTest]
        public void ToCharArray_FullString()
        {
            char[] chars = "abc".ToCharArray();
            Assert.Equal(3, chars.Length);
            Assert.Equal('a', chars[0]);
            Assert.Equal('b', chars[1]);
            Assert.Equal('c', chars[2]);
        }

        [UnitTest]
        public void ToCharArray_Range()
        {
            char[] chars = "hello".ToCharArray(1, 3);
            Assert.Equal(3, chars.Length);
            Assert.Equal('e', chars[0]);
            Assert.Equal('l', chars[1]);
            Assert.Equal('l', chars[2]);
        }

        [UnitTest]
        public void ToCharArray_Empty()
        {
            char[] empty = "".ToCharArray();
            Assert.Equal(0, empty.Length);
            char[] slice = "a".ToCharArray(1, 0);
            Assert.Equal(0, slice.Length);
        }

        [UnitTest]
        public void ToCharArray_IsCopy()
        {
            char[] chars = "abc".ToCharArray();
            chars[0] = 'z';
            Assert.Equal('a', "abc"[0]);
        }

        [UnitTest]
        public void ToCharArray_InvalidRange_Throws()
        {
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "abc".ToCharArray(-1, 1));
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "abc".ToCharArray(4, 0));
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "abc".ToCharArray(1, 3));
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "abc".ToCharArray(2, 2));
        }

        #endregion

        #region ToLower

        [UnitTest]
        public void ToLower_Basic()
        {
            Assert.Equal("abc", "ABC".ToLower());
            Assert.Equal("hello world", "Hello World".ToLower());
            Assert.Equal("abc123", "AbC123".ToLower());
        }

        [UnitTest]
        public void ToLower_AlreadyLowerOrEmpty()
        {
            Assert.Equal("abc", "abc".ToLower());
            Assert.Equal("", "".ToLower());
        }

        [UnitTest]
        public void ToLower_Invariant()
        {
            Assert.Equal("abc", "ABC".ToLowerInvariant());
            Assert.Equal("hello", "HELLO".ToLowerInvariant());
            Assert.Equal("", "".ToLowerInvariant());
        }

        [UnitTest]
        public void ToLower_DoesNotMutateOriginal()
        {
            string original = "ABC";
            string lowered = original.ToLower();
            Assert.Equal("ABC", original);
            Assert.Equal("abc", lowered);
        }

        #endregion

        #region Trim

        [UnitTest]
        public void Trim_WhitespaceBothEnds()
        {
            Assert.Equal("abc", "  abc  ".Trim());
            Assert.Equal("abc", "\tabc\n".Trim());
            Assert.Equal("a b c", "  a b c  ".Trim());
        }

#if IL2CPP_ONLY
        [UnitTest]
        public void Trim_NoWhitespaceUnchanged()
        {
            Assert.Equal("abc", " abc ".Trim());
            Assert.Equal("abc", "  abc".TrimStart());
            Assert.Equal("abc", "abc  ".TrimEnd());
        }
#endif

        [UnitTest]
        public void Trim_AllWhitespaceBecomesEmpty()
        {
            Assert.Equal("", "   ".Trim());
            Assert.Equal("", "\t\n".Trim());
        }


#if IL2CPP_ONLY
        [UnitTest]
        public void Trim_EmptyString()
        {
            Assert.Equal("", "".Trim());
            Assert.Equal("", "".TrimStart());
            Assert.Equal("", "".TrimEnd());
        }

        [UnitTest]
        public void Trim_CustomTrimChars()
        {
            Assert.Equal("abc", "xxabcxx".Trim('x'));
            Assert.Equal("bc", "aabcaa".Trim('a'));
            Assert.Equal("hello", "...hello...".Trim('.'));
        }
#endif


#if IL2CPP_ONLY
        [UnitTest]
        public void TrimStart_CustomChars()
        {
            Assert.Equal("bc", "aabc".TrimStart('a'));
            Assert.Equal("hello", "...hello".TrimStart('.'));
            Assert.Equal("abc", "  abc".TrimStart());
        }

        [UnitTest]
        public void TrimEnd_CustomChars()
        {
            Assert.Equal("ab", "abxx".TrimEnd('x'));
            Assert.Equal("hello", "hello...".TrimEnd('.'));
            Assert.Equal("abc", "abc  ".TrimEnd());
        }
        //[UnitTest]
        //public void Trim_NullTrimChars_Throws()
        //{
        //    Assert.ExpectException<ArgumentNullException>(() => "abc".Trim(null));
        //    Assert.ExpectException<ArgumentNullException>(() => "abc".TrimStart(null));
        //    Assert.ExpectException<ArgumentNullException>(() => "abc".TrimEnd(null));
        //}
#endif

#endregion

#region Split

#if IL2CPP_ONLY
        [UnitTest]
        public void Split_SingleChar()
        {
            string[] parts = "a,b,c".Split(',');
            Assert.Equal(3, parts.Length);
            Assert.Equal("a", parts[0]);
            Assert.Equal("b", parts[1]);
            Assert.Equal("c", parts[2]);
        }
#endif

        [UnitTest]
        public void Split_MultipleSeparators()
        {
            string[] parts = "a,b;c".Split(',', ';');
            Assert.Equal(3, parts.Length);
            Assert.Equal("a", parts[0]);
            Assert.Equal("b", parts[1]);
            Assert.Equal("c", parts[2]);
        }

        [UnitTest]
        public void Split_StringSeparators()
        {
            string[] parts = "one<->two<->three".Split(new string[] { "<->" }, StringSplitOptions.None);
            Assert.Equal(3, parts.Length);
            Assert.Equal("one", parts[0]);
            Assert.Equal("two", parts[1]);
            Assert.Equal("three", parts[2]);
        }

        [UnitTest]
        public void Split_RemoveEmptyEntries()
        {
            string[] parts = "a,,b,".Split(new char[] { ',' }, StringSplitOptions.RemoveEmptyEntries);
            Assert.Equal(2, parts.Length);
            Assert.Equal("a", parts[0]);
            Assert.Equal("b", parts[1]);
        }

        [UnitTest]
        public void Split_NoneKeepsEmptyEntries()
        {
            string[] parts = "a,,b".Split(new char[] { ',' }, StringSplitOptions.None);
            Assert.Equal(3, parts.Length);
            Assert.Equal("a", parts[0]);
            Assert.Equal("", parts[1]);
            Assert.Equal("b", parts[2]);
        }

        [UnitTest]
        public void Split_WithCountLimit()
        {
            string[] parts = "a,b,c,d".Split(new char[] { ',' }, 2, StringSplitOptions.None);
            Assert.Equal(2, parts.Length);
            Assert.Equal("a", parts[0]);
            Assert.Equal("b,c,d", parts[1]);
        }

        [UnitTest]
        public void Split_NullSeparatorArray_UsesWhitespace()
        {
            string[] parts = "a b\tc".Split((char[])null);
            Assert.Equal(3, parts.Length);
            Assert.Equal("a", parts[0]);
            Assert.Equal("b", parts[1]);
            Assert.Equal("c", parts[2]);
        }

#if IL2CPP_ONLY
        [UnitTest]
        public void Split_NoSeparatorMatch_ReturnsWholeString()
        {
            string[] parts = "abc".Split(',');
            Assert.Equal(1, parts.Length);
            Assert.Equal("abc", parts[0]);
        }

        [UnitTest]
        public void Split_EmptyString()
        {
            string[] parts = "".Split(',');
            Assert.Equal(1, parts.Length);
            Assert.Equal("", parts[0]);
        }

        [UnitTest]
        public void Split_ConsecutiveSeparators()
        {
            string[] parts = "a||b".Split('|');
            Assert.Equal(3, parts.Length);
            Assert.Equal("a", parts[0]);
            Assert.Equal("", parts[1]);
            Assert.Equal("b", parts[2]);
        }
#endif

#endregion

        #region Remove

        [UnitTest]
        public void Remove_FromStart()
        {
            Assert.Equal("bc", "abc".Remove(0, 1));
            Assert.Equal("", "a".Remove(0, 1));
        }

        [UnitTest]
        public void Remove_FromMiddle()
        {
            Assert.Equal("ac", "abc".Remove(1, 1));
            Assert.Equal("ae", "abcde".Remove(1, 3));
        }

        [UnitTest]
        public void Remove_FromEnd()
        {
            Assert.Equal("ab", "abc".Remove(2, 1));
            Assert.Equal("hello", "hello!".Remove(5, 1));
        }

        [UnitTest]
        public void Remove_AllCharacters()
        {
            Assert.Equal("", "abc".Remove(0, 3));
        }

        [UnitTest]
        public void Remove_ZeroCount_NoChange()
        {
            Assert.Equal("abc", "abc".Remove(1, 0));
            Assert.Equal("abc", "abc".Remove(0, 0));
        }

        [UnitTest]
        public void Remove_StartIndexOutOfRange_Throws()
        {
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "abc".Remove(-1, 1));
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "abc".Remove(4, 0));
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "".Remove(0, 1));
        }

        [UnitTest]
        public void Remove_CountOutOfRange_Throws()
        {
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "abc".Remove(0, -1));
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "abc".Remove(0, 4));
            Assert.ExpectException<ArgumentOutOfRangeException>(() => "abc".Remove(2, 2));
        }

        #endregion

        #region Replace

        [UnitTest]
        public void Replace_Char()
        {
            Assert.Equal("cbc", "abc".Replace('a', 'c'));
            Assert.Equal("aba", "abc".Replace('c', 'a'));
            Assert.Equal("a c", "a c".Replace('x', 'y'));
        }

        [UnitTest]
        public void Replace_String()
        {
            Assert.Equal("foo bar foo", "foo baz foo".Replace("baz", "bar"));
            Assert.Equal("xx", "abab".Replace("ab", "x"));
            Assert.Equal("", "abc".Replace("abc", ""));
        }

        [UnitTest]
        public void Replace_String_AllOccurrences()
        {
            Assert.Equal("***", "aaa".Replace("a", "*"));
            Assert.Equal("1-2-3", "1,2,3".Replace(",", "-"));
        }

        [UnitTest]
        public void Replace_String_NoMatch_ReturnsSameContent()
        {
            string original = "hello";
            string result = original.Replace("xyz", "abc");
            Assert.Equal("hello", result);
        }

        [UnitTest]
        public void Replace_String_NullOldValue_Throws()
        {
            Assert.ExpectException<ArgumentNullException>(() => "abc".Replace(null, "x"));
        }

        [UnitTest]
        public void Replace_String_NullNewValue_Allowed()
        {
            Assert.Equal("bc", "abc".Replace("a", null));
        }

        [UnitTest]
        public void Replace_Char_DoesNotMutateOriginal()
        {
            string original = "abc";
            string replaced = original.Replace('a', 'z');
            Assert.Equal("abc", original);
            Assert.Equal("zbc", replaced);
        }

        #endregion
    }
}
