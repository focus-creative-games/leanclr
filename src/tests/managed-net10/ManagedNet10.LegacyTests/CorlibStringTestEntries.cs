namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunCorlibString()
        {
            RunCorlibStringFirstThree();
            RunCorlibStringLastThree();
        }

        public static void RunCorlibStringFirstThree()
        {
            RunCorlibStringFirstTwo();
            RunCorlibStringIndexOf();
        }

        public static void RunCorlibStringFirstTwo()
        {
            RunCorlibStringCompareConcatContains();
            RunCorlibStringEndsEqualsInsert();
        }

        public static void RunCorlibStringEndsIndexOf()
        {
            RunCorlibStringEndsEqualsInsert();
            RunCorlibStringIndexOf();
        }

        public static void RunCorlibStringCompareThenEndsWith()
        {
            RunCorlibStringCompareConcatContains();
            RunCorlibStringEndsWith();
        }

        public static void RunCorlibStringCompareEndsNoThrowThenEquals1()
        {
            RunCorlibStringCompareConcatContains();
            RunCorlibStringMethods(
                "EndsWith_Match",
                "EndsWith_NoMatch",
                "EndsWith_StringComparison_OrdinalIgnoreCase",
                "EndsWith_StringComparison_Ordinal",
                "EndsWith_LongerSuffixThanString");
            RunCorlibStringMethods("Equals_Instance_ValueEqual");
        }

        public static void RunCorlibStringCompareEndsThrowThenEquals1()
        {
            RunCorlibStringCompareConcatContains();
            RunCorlibStringMethods("EndsWith_NullSuffix_Throws");
            RunCorlibStringMethods("Equals_Instance_ValueEqual");
        }

        public static void RunCorlibStringEndsThrowThenEquals1()
        {
            RunCorlibStringMethods("EndsWith_NullSuffix_Throws");
            RunCorlibStringMethods("Equals_Instance_ValueEqual");
        }

        public static void RunCorlibStringEndsThenEquals1()
        {
            RunCorlibStringEndsWith();
            RunCorlibStringMethods("Equals_Instance_ValueEqual");
        }

        public static void RunCorlibStringCompareThenEquals()
        {
            RunCorlibStringCompareConcatContains();
            RunCorlibStringEquals();
        }

        public static void RunCorlibStringCompareThenInsert()
        {
            RunCorlibStringCompareConcatContains();
            RunCorlibStringInsert();
        }

        public static void RunCorlibStringCompareThenEndsEquals()
        {
            RunCorlibStringCompareConcatContains();
            RunCorlibStringEndsWith();
            RunCorlibStringEquals();
        }

        public static void RunCorlibStringCompareEndsThenEqualsCore()
        {
            RunCorlibStringCompareConcatContains();
            RunCorlibStringEndsWith();
            RunCorlibStringEqualsCore();
        }

        public static void RunCorlibStringCompareEndsThenEquals1()
        {
            RunCorlibStringCompareEndsThenEqualsMethods(
                "Equals_Instance_ValueEqual");
        }

        public static void RunCorlibStringCompareEndsThenEquals2()
        {
            RunCorlibStringCompareEndsThenEqualsMethods(
                "Equals_Instance_ValueEqual",
                "Equals_Instance_NotEqual");
        }

        public static void RunCorlibStringCompareEndsThenEquals3()
        {
            RunCorlibStringCompareEndsThenEqualsMethods(
                "Equals_Instance_ValueEqual",
                "Equals_Instance_NotEqual",
                "Equals_Instance_Null");
        }

        public static void RunCorlibStringCompareEndsThenEquals4()
        {
            RunCorlibStringCompareEndsThenEqualsMethods(
                "Equals_Instance_ValueEqual",
                "Equals_Instance_NotEqual",
                "Equals_Instance_Null",
                "Equals_Static");
        }

        public static void RunCorlibStringCompareEndsThenEquals5()
        {
            RunCorlibStringCompareEndsThenEqualsMethods(
                "Equals_Instance_ValueEqual",
                "Equals_Instance_NotEqual",
                "Equals_Instance_Null",
                "Equals_Static",
                "Equals_StringComparison_OrdinalIgnoreCase");
        }

        public static void RunCorlibStringCompareEndsThenIntern()
        {
            RunCorlibStringCompareConcatContains();
            RunCorlibStringEndsWith();
            RunCorlibStringIntern();
        }

        public static void RunCorlibStringCompareThenEqualsInsert()
        {
            RunCorlibStringCompareConcatContains();
            RunCorlibStringEquals();
            RunCorlibStringInsert();
        }

        public static void RunCorlibStringLastThree()
        {
            RunCorlibStringLastIndexOfStartsWith();
            RunCorlibStringSubstringCharArrayLowerTrimSplit();
            RunCorlibStringRemoveReplace();
        }

        public static void RunCorlibStringCompareConcatContains()
        {
            RunCorlibStringMethods(
                "GetSceneNameTest",
                "Compare_EqualStrings_ReturnsZero",
                "Compare_LexicographicOrder",
                "Compare_PrefixAndLength",
                "Compare_WithStartIndexAndLength",
                "Compare_NullArguments",
                "Compare_CaseSensitiveDefault",
                "Compare_StringComparison_Ordinal",
                "Compare_StringComparison_OrdinalIgnoreCase",
                "Compare_StringComparison_WithIndexAndLength",
                "CompareOrdinal",
                "Compare_UnicodeAndSurrogates",
                "Concat_TwoStrings",
                "Concat_TwoStrings_NullTreatedAsEmpty",
                "Concat_ThreeAndFour",
                "Concat_ParamsStringArray",
                "Concat_ParamsObjectArray",
                "Concat_EmptyAndSingleChar",
                "Contains_Found",
                "Contains_NotFound",
                "Contains_EmptyValue",
                "Contains_NullValue_Throws",
                "Contains_OverlappingPattern");
        }

        public static void RunCorlibStringCompareOnlyThenEndsEquals1()
        {
            RunCorlibStringMethods(
                "GetSceneNameTest",
                "Compare_EqualStrings_ReturnsZero",
                "Compare_LexicographicOrder",
                "Compare_PrefixAndLength",
                "Compare_WithStartIndexAndLength",
                "Compare_NullArguments",
                "Compare_CaseSensitiveDefault",
                "Compare_StringComparison_Ordinal",
                "Compare_StringComparison_OrdinalIgnoreCase",
                "Compare_StringComparison_WithIndexAndLength",
                "CompareOrdinal",
                "Compare_UnicodeAndSurrogates");
            RunCorlibStringEndsWith();
            RunCorlibStringMethods("Equals_Instance_ValueEqual");
        }

        public static void RunCorlibStringCompareFirstHalfThenEndsEquals1()
        {
            RunCorlibStringMethods(
                "GetSceneNameTest",
                "Compare_EqualStrings_ReturnsZero",
                "Compare_LexicographicOrder",
                "Compare_PrefixAndLength",
                "Compare_WithStartIndexAndLength",
                "Compare_NullArguments");
            RunCorlibStringEndsWith();
            RunCorlibStringMethods("Equals_Instance_ValueEqual");
        }

        public static void RunCorlibStringCompareSecondHalfThenEndsEquals1()
        {
            RunCorlibStringMethods(
                "Compare_CaseSensitiveDefault",
                "Compare_StringComparison_Ordinal",
                "Compare_StringComparison_OrdinalIgnoreCase",
                "Compare_StringComparison_WithIndexAndLength",
                "CompareOrdinal",
                "Compare_UnicodeAndSurrogates");
            RunCorlibStringEndsWith();
            RunCorlibStringMethods("Equals_Instance_ValueEqual");
        }

        public static void RunCorlibStringGetSceneNameThenEndsEquals1()
        {
            RunCorlibStringCompareMethodThenEndsEquals1("GetSceneNameTest");
        }

        public static void RunCorlibStringCompareEqualThenEndsEquals1()
        {
            RunCorlibStringCompareMethodThenEndsEquals1("Compare_EqualStrings_ReturnsZero");
        }

        public static void RunCorlibStringCompareLexicographicThenEndsEquals1()
        {
            RunCorlibStringCompareMethodThenEndsEquals1("Compare_LexicographicOrder");
        }

        public static void RunCorlibStringComparePrefixThenEndsEquals1()
        {
            RunCorlibStringCompareMethodThenEndsEquals1("Compare_PrefixAndLength");
        }

        public static void RunCorlibStringCompareWithStartThenEndsEquals1()
        {
            RunCorlibStringCompareMethodThenEndsEquals1("Compare_WithStartIndexAndLength");
        }

        public static void RunCorlibStringCompareNullThenEndsEquals1()
        {
            RunCorlibStringCompareMethodThenEndsEquals1("Compare_NullArguments");
        }

        public static void RunCorlibStringCompareCaseDefaultThenEndsEquals1()
        {
            RunCorlibStringCompareMethodThenEndsEquals1("Compare_CaseSensitiveDefault");
        }

        public static void RunCorlibStringCompareOrdinalThenEndsEquals1()
        {
            RunCorlibStringCompareMethodThenEndsEquals1("Compare_StringComparison_Ordinal");
        }

        public static void RunCorlibStringCompareOrdinalIgnoreCaseThenEndsEquals1()
        {
            RunCorlibStringCompareMethodThenEndsEquals1("Compare_StringComparison_OrdinalIgnoreCase");
        }

        public static void RunCorlibStringCompareWithIndexLengthThenEndsEquals1()
        {
            RunCorlibStringCompareMethodThenEndsEquals1("Compare_StringComparison_WithIndexAndLength");
        }

        public static void RunCorlibStringCompareOrdinalMethodThenEndsEquals1()
        {
            RunCorlibStringCompareMethodThenEndsEquals1("CompareOrdinal");
        }

        public static void RunCorlibStringCompareUnicodeThenEndsEquals1()
        {
            RunCorlibStringCompareMethodThenEndsEquals1("Compare_UnicodeAndSurrogates");
        }

        public static void RunCorlibStringConcatContainsThenEndsEquals1()
        {
            RunCorlibStringMethods(
                "Concat_TwoStrings",
                "Concat_TwoStrings_NullTreatedAsEmpty",
                "Concat_ThreeAndFour",
                "Concat_ParamsStringArray",
                "Concat_ParamsObjectArray",
                "Concat_EmptyAndSingleChar",
                "Contains_Found",
                "Contains_NotFound",
                "Contains_EmptyValue",
                "Contains_NullValue_Throws",
                "Contains_OverlappingPattern");
            RunCorlibStringEndsWith();
            RunCorlibStringMethods("Equals_Instance_ValueEqual");
        }

        public static void RunCorlibStringEndsEqualsInsert()
        {
            RunCorlibStringMethods(
                "EndsWith_Match",
                "EndsWith_NoMatch",
                "EndsWith_StringComparison_OrdinalIgnoreCase",
                "EndsWith_StringComparison_Ordinal",
                "EndsWith_LongerSuffixThanString",
                "EndsWith_NullSuffix_Throws",
                "Equals_Instance_ValueEqual",
                "Equals_Instance_NotEqual",
                "Equals_Instance_Null",
                "Equals_Static",
                "Equals_StringComparison_OrdinalIgnoreCase",
                "Equals_Operator",
                "Equals_ReferenceEqualsInterned",
                "Insert_BeginningMiddleEnd",
                "Insert_EmptyValue",
                "Insert_IntoEmpty",
                "Insert_LongerValue",
                "Insert_NullValue_Throws",
                "Insert_IndexOutOfRange_Throws");
        }

        public static void RunCorlibStringEndsWith()
        {
            RunCorlibStringMethods(
                "EndsWith_Match",
                "EndsWith_NoMatch",
                "EndsWith_StringComparison_OrdinalIgnoreCase",
                "EndsWith_StringComparison_Ordinal",
                "EndsWith_LongerSuffixThanString",
                "EndsWith_NullSuffix_Throws");
        }

        public static void RunCorlibStringEquals()
        {
            RunCorlibStringEqualsCore();
            RunCorlibStringIntern();
        }

        public static void RunCorlibStringEqualsCore()
        {
            RunCorlibStringMethods(
                "Equals_Instance_ValueEqual",
                "Equals_Instance_NotEqual",
                "Equals_Instance_Null",
                "Equals_Static",
                "Equals_StringComparison_OrdinalIgnoreCase",
                "Equals_Operator");
        }

        public static void RunCorlibStringInsert()
        {
            RunCorlibStringMethods(
                "Insert_BeginningMiddleEnd",
                "Insert_EmptyValue",
                "Insert_IntoEmpty",
                "Insert_LongerValue",
                "Insert_NullValue_Throws",
                "Insert_IndexOutOfRange_Throws");
        }

        public static void RunCorlibStringInsertBeginningMiddleEnd()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_String), "Insert_BeginningMiddleEnd");
        }

        public static void RunCorlibStringInsertEmptyValue()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_String), "Insert_EmptyValue");
        }

        public static void RunCorlibStringInsertIntoEmpty()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_String), "Insert_IntoEmpty");
        }

        public static void RunCorlibStringInsertLongerValue()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_String), "Insert_LongerValue");
        }

        public static void RunCorlibStringInsertNullValueThrows()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_String), "Insert_NullValue_Throws");
        }

        public static void RunCorlibStringInsertIndexOutOfRangeThrows()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_String), "Insert_IndexOutOfRange_Throws");
        }

        public static void RunCorlibStringIndexOf()
        {
            RunCorlibStringMethods(
                "IndexOf_Char",
                "IndexOf_Char_WithStartIndex",
                "IndexOf_Char_WithStartIndexAndCount",
                "IndexOf_String",
                "IndexOf_String_WithStartIndex",
                "IndexOf_String_WithStartIndexAndCount",
                "IndexOf_EmptyString",
                "IndexOf_OverlappingMatches",
                "IndexOf_StringComparison_OrdinalIgnoreCase",
                "IndexOf_StringComparison_WithStartIndex",
                "IndexOf_NullNeedle_Throws",
                "IndexOf_InvalidStartIndexOrCount_Throws",
                "IndexOf_SurrogateChar");
        }

        public static void RunCorlibStringLastIndexOfStartsWith()
        {
            RunCorlibStringMethods(
                "LastIndexOf_Char",
                "LastIndexOf_Char_WithStartIndex",
                "LastIndexOf_Char_WithStartIndexAndCount",
                "LastIndexOf_String",
                "LastIndexOf_String_WithStartIndex",
                "LastIndexOf_String_WithStartIndexAndCount");

            LegacyTestRunner.RunMethod(typeof(CorlibStringNet10Semantics), "LastIndexOf_EmptyString");

            RunCorlibStringMethods(
                "LastIndexOf_OverlappingMatches",
                "LastIndexOf_StringComparison_OrdinalIgnoreCase",
                "LastIndexOf_StringComparison_WithStartIndex",
                "LastIndexOf_SceneNamePattern",
                "LastIndexOf_NullValue_Throws",
                "LastIndexOf_InvalidStartIndexOrCount_Throws",
                "StartsWith_Match",
                "StartsWith_NoMatch",
                "StartsWith_StringComparison_OrdinalIgnoreCase",
                "StartsWith_StringComparison_Ordinal",
                "StartsWith_LongerPrefixThanString",
                "StartsWith_NullPrefix_Throws");
        }

        public static void RunCorlibStringSubstringCharArrayLowerTrimSplit()
        {
            RunCorlibStringMethods(
                "Substring_FromStartIndex",
                "Substring_WithLength",
                "Substring_SingleCharAndEmptySource",
                "Substring_StartIndexOutOfRange_Throws",
                "Substring_LengthOutOfRange_Throws",
                "ToCharArray_FullString",
                "ToCharArray_Range",
                "ToCharArray_Empty",
                "ToCharArray_IsCopy",
                "ToCharArray_InvalidRange_Throws",
                "ToLower_Basic",
                "ToLower_AlreadyLowerOrEmpty",
                "ToLower_Invariant",
                "ToLower_DoesNotMutateOriginal",
                "Trim_WhitespaceBothEnds",
                "Trim_AllWhitespaceBecomesEmpty",
                "Split_MultipleSeparators",
                "Split_StringSeparators",
                "Split_RemoveEmptyEntries",
                "Split_NoneKeepsEmptyEntries",
                "Split_WithCountLimit",
                "Split_NullSeparatorArray_UsesWhitespace");
        }

        public static void RunCorlibStringRemoveReplace()
        {
            RunCorlibStringMethods(
                "Remove_FromStart",
                "Remove_FromMiddle",
                "Remove_FromEnd",
                "Remove_AllCharacters",
                "Remove_ZeroCount_NoChange",
                "Remove_StartIndexOutOfRange_Throws",
                "Remove_CountOutOfRange_Throws",
                "Replace_Char",
                "Replace_String",
                "Replace_String_AllOccurrences",
                "Replace_String_NoMatch_ReturnsSameContent",
                "Replace_String_NullOldValue_Throws",
                "Replace_String_NullNewValue_Allowed",
                "Replace_Char_DoesNotMutateOriginal");
        }

        public static void RunCorlibStringIntern()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_String), "Equals_ReferenceEqualsInterned");
        }

        public static void RunCorlibStringToLower()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_String), "ToLower_Basic");
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_String), "ToLower_Invariant");
        }

        public static void RunCorlibStringSplit()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_String), "Split_MultipleSeparators");
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_String), "Split_StringSeparators");
        }

        private static void RunCorlibStringMethods(params string[] methodNames)
        {
            for (int i = 0; i < methodNames.Length; i++)
            {
                LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_String), methodNames[i]);
            }
        }

        private static void RunCorlibStringCompareEndsThenEqualsMethods(params string[] methodNames)
        {
            RunCorlibStringCompareConcatContains();
            RunCorlibStringEndsWith();
            RunCorlibStringMethods(methodNames);
        }

        private static void RunCorlibStringCompareMethodThenEndsEquals1(string methodName)
        {
            RunCorlibStringMethods(methodName);
            RunCorlibStringEndsWith();
            RunCorlibStringMethods("Equals_Instance_ValueEqual");
        }
    }

    internal sealed class CorlibStringNet10Semantics
    {
        [UnitTest]
        public void LastIndexOf_EmptyString()
        {
            Assert.Equal(3, "abc".LastIndexOf(""));
            Assert.Equal(5, "hello".LastIndexOf(""));
            Assert.Equal(0, "".LastIndexOf(""));
            Assert.Equal(3, "abc".LastIndexOf("", 2));
        }
    }
}
