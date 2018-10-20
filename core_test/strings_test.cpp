/**************************************************************************/
/*                                                                        */
/*                              WWIV Version 5.x                          */
/*           Copyright (C)2008-2017, WWIV Software Services                */
/*                                                                        */
/*    Licensed  under the  Apache License, Version  2.0 (the "License");  */
/*    you may not use this  file  except in compliance with the License.  */
/*    You may obtain a copy of the License at                             */
/*                                                                        */
/*                http://www.apache.org/licenses/LICENSE-2.0              */
/*                                                                        */
/*    Unless  required  by  applicable  law  or agreed to  in  writing,   */
/*    software  distributed  under  the  License  is  distributed on an   */
/*    "AS IS"  BASIS, WITHOUT  WARRANTIES  OR  CONDITIONS OF ANY  KIND,   */
/*    either  express  or implied.  See  the  License for  the specific   */
/*    language governing permissions and limitations under the License.   */
/*                                                                        */
/**************************************************************************/
#include "gtest/gtest.h"

#include <string>
#include <vector>

#include "core/strings.h"

using std::cout;
using std::endl;
using std::ostringstream;
using std::string;
using std::vector;

using namespace wwiv::strings;

TEST(StringsTest, StripColors) {
  EXPECT_EQ(string(""), stripcolors(string("")));
  EXPECT_EQ(string("|"), stripcolors(string("|")));
  EXPECT_EQ(string("|0"), stripcolors(string("|0")));
  EXPECT_EQ(string("12345"), stripcolors(string("12345")));
  EXPECT_EQ(string("abc"), stripcolors(string("abc")));
  EXPECT_EQ(string("1 abc"), stripcolors(string("\x031 abc")));
  EXPECT_EQ(string("\x03 abc"), stripcolors(string("\x03 abc")));
  EXPECT_EQ(string("abc"), stripcolors(string("|15abc")));
}

TEST(StringsTest, StripColors_AnsiSeq) {
  EXPECT_EQ(string(""), stripcolors(string("\x1b[0m")));
  EXPECT_EQ(string(""), stripcolors(string("\x1b[0;33;46;1m")));
  EXPECT_EQ(string("|"), stripcolors(string("|\x1b[0;33;46;1m")));
  EXPECT_EQ(string("abc"), stripcolors(string("|15\x1b[0;33;46;1mabc")));
  EXPECT_EQ(string("abc"),
            stripcolors(string("\x1b[0m|15\x1b[0;33;46;1ma\x1b[0mb\x1b[0mc\x1b[0m")));
}

TEST(StringsTest, StringColors_CharStarVersion) {
  EXPECT_STREQ("", stripcolors(""));
  EXPECT_STREQ("|", stripcolors("|"));
  EXPECT_STREQ("|0", stripcolors("|0"));
  EXPECT_STREQ("12345", stripcolors("12345"));
}

TEST(StringsTest, Properize) {
  EXPECT_EQ(string("Rushfan"), properize(string("rushfan")));
  EXPECT_EQ(string("Rushfan"), properize(string("rUSHFAN")));
  EXPECT_EQ(string(""), properize(string("")));
  EXPECT_EQ(string(" "), properize(string(" ")));
  EXPECT_EQ(string("-"), properize(string("-")));
  EXPECT_EQ(string("."), properize(string(".")));
  EXPECT_EQ(string("R"), properize(string("R")));
  EXPECT_EQ(string("R"), properize(string("r")));
  EXPECT_EQ(string("Ru"), properize(string("RU")));
  EXPECT_EQ(string("R.U"), properize(string("r.u")));
  EXPECT_EQ(string("R U"), properize(string("r u")));
  EXPECT_EQ(string("Rushfan"), properize(string("Rushfan")));
}

TEST(StringsTest, StringPrintf_Smoke) {
  static const string kRushfan = "rushfan";
  EXPECT_EQ(kRushfan, StringPrintf("%s%s", "rush", "fan"));
  EXPECT_EQ(kRushfan, StringPrintf("%s%c%c%c", "rush", 'f', 'a', 'n'));
}

TEST(StringsTest, StrCat_Smoke) {
  static const string kRushfan = "rushfan";
  EXPECT_EQ(kRushfan, StrCat("rush", "fan"));
  EXPECT_EQ(kRushfan, StrCat("ru", "sh", "fan"));
  EXPECT_EQ(kRushfan, StrCat("ru", "sh", "f", "an"));
  EXPECT_EQ(kRushfan, StrCat("r", "u", "sh", "f", "an"));
  EXPECT_EQ(kRushfan, StrCat("r", "u", "s", "h", "f", "an"));
}

TEST(StringsTest, StrCat_AlphaNumeric) {
  static const string kWoot = "w00t";
  EXPECT_EQ(kWoot, StrCat("w", 0, 0, "t"));
}

TEST(StringsTest, StringReplace_EntireString) {
  string s = "Hello";
  string world = "World";
  EXPECT_EQ(world, StringReplace(&s, "Hello", "World"));
  EXPECT_EQ(world, s);
}

TEST(StringsTest, StringReplace_PartialString) {
  string s = "Hello World";
  string expected = "World World";
  EXPECT_EQ(expected, StringReplace(&s, "Hello", "World"));
  EXPECT_EQ(expected, s);
}

TEST(StringsTest, StringReplace_NotFound) {
  string s = "Hello World";
  string expected(s);
  EXPECT_EQ(expected, StringReplace(&s, "Dude", "Where's my car"));
  EXPECT_EQ(expected, s);
}

TEST(StringsTest, SplitString_Basic) {
  const string s = "Hello World";
  vector<string> expected = {"Hello", "World"};
  vector<string> actual;
  SplitString(s, " ", &actual);
  EXPECT_EQ(expected, actual);
}

TEST(StringsTest, SplitString_BasicReturned) {
  const string s = "Hello World";
  vector<string> expected = {"Hello", "World"};
  vector<string> actual = SplitString(s, " ");
  EXPECT_EQ(expected, actual);
}

TEST(StringsTest, SplitString_ExtraSingleDelim) {
  const string s = "Hello   World";
  vector<string> expected = {"Hello", "World"};
  vector<string> actual;
  SplitString(s, " ", &actual);
  EXPECT_EQ(expected, actual);
}

TEST(StringsTest, SplitString_ExtraSingleDelim_NoSkipEmpty) {
  const string s = "Hello   World";
  vector<string> expected = {"Hello", "", "", "World"};
  vector<string> actual;
  SplitString(s, " ", false, &actual);
  EXPECT_EQ(expected, actual);
}

TEST(StringsTest, SplitString_TwoDelims) {
  const string s = "Hello\tWorld Everyone";
  vector<string> expected = {"Hello", "World", "Everyone"};
  vector<string> actual;
  SplitString(s, " \t", &actual);
  EXPECT_EQ(expected, actual);
}

TEST(StringsTest, SplitString_TwoDelimsBackToBack) {
  const string s = "Hello\t\tWorld  \t\t  Everyone";
  vector<string> expected = {"Hello", "World", "Everyone"};
  vector<string> actual;
  SplitString(s, " \t", &actual);
  EXPECT_EQ(expected, actual);
}

TEST(StringsTest, String_int16_t) {
  EXPECT_EQ(1234, to_number<int16_t>("1234"));
  EXPECT_EQ(0, to_number<int16_t>("0"));
  EXPECT_EQ(-1234, to_number<int16_t>("-1234"));

  EXPECT_EQ(std::numeric_limits<int16_t>::max(), to_number<int16_t>("999999"));
  EXPECT_EQ(std::numeric_limits<int16_t>::min(), to_number<int16_t>("-999999"));

  EXPECT_EQ(0, to_number<int16_t>(""));
  EXPECT_EQ(0, to_number<int16_t>("ASDF"));
}

TEST(StringsTest, String_uint16_t) {
  EXPECT_EQ(1234, to_number<uint16_t>("1234"));
  EXPECT_EQ(0, to_number<uint16_t>("0"));

  EXPECT_EQ(std::numeric_limits<uint16_t>::max(), to_number<uint16_t>("999999"));

  EXPECT_EQ(0, to_number<uint16_t>(""));
  EXPECT_EQ(0, to_number<uint16_t>("ASDF"));
}

TEST(StringsTest, String_unsigned_int) {
  EXPECT_EQ(1234u, to_number<unsigned int>("1234"));
  EXPECT_EQ(static_cast<unsigned int>(0), to_number<unsigned int>("0"));

  EXPECT_EQ(999999u, to_number<unsigned int>("999999"));

  EXPECT_EQ(0u, to_number<unsigned int>(""));
  EXPECT_EQ(0u, to_number<unsigned int>("ASDF"));
}

TEST(StringsTest, String_int) {
  EXPECT_EQ(1234, to_number<int>("1234"));
  EXPECT_EQ(0, to_number<int>("0"));

  EXPECT_EQ(999999, to_number<int>("999999"));
  EXPECT_EQ(-999999, to_number<int>("-999999"));

  EXPECT_EQ(0, to_number<int>(""));
  EXPECT_EQ(0, to_number<int>("ASDF"));
}

TEST(StringsTest, String_int8_t) {
  EXPECT_EQ(std::numeric_limits<int8_t>::max(), to_number<int8_t>("1234"));
  EXPECT_EQ(0, to_number<int8_t>("0"));
  EXPECT_EQ(std::numeric_limits<int8_t>::min(), to_number<int8_t>("-1234"));

  EXPECT_EQ(std::numeric_limits<int8_t>::max(), to_number<int8_t>("999999"));
  EXPECT_EQ(std::numeric_limits<int8_t>::min(), to_number<int8_t>("-999999"));

  EXPECT_EQ(0, to_number<int8_t>(""));
  EXPECT_EQ(0, to_number<int8_t>("ASDF"));
}

TEST(StringsTest, String_uint8_t) {
  EXPECT_EQ(12, to_number<uint8_t>("12"));
  EXPECT_EQ(255, to_number<uint8_t>("255"));
  EXPECT_EQ(0, to_number<uint8_t>("0"));

  EXPECT_EQ(std::numeric_limits<uint8_t>::max(), to_number<uint8_t>("999999"));

  EXPECT_EQ(0, to_number<uint8_t>(""));
  EXPECT_EQ(0, to_number<uint8_t>("ASDF"));
}

TEST(StringsTest, StringRemoveWhitespace_NoSpace) {
  string s("HelloWorld");
  string expected(s);
  StringRemoveWhitespace(&s);
  EXPECT_EQ(expected, s);
}

TEST(StringsTest, StringRemoveWhitespace_InnerSpace) {
  string expected("HelloWorld");
  string s("Hello World");
  StringRemoveWhitespace(&s);
  EXPECT_EQ(expected, s);
}

TEST(StringsTest, StringRemoveWhitespace_Trailing) {
  string expected("HelloWorld");
  string s("Hello World  ");
  StringRemoveWhitespace(&s);
  EXPECT_EQ(expected, s);
}

TEST(StringsTest, StringRemoveWhitespace_Leading) {
  string expected("HelloWorld");
  string s("  Hello World");
  StringRemoveWhitespace(&s);
  EXPECT_EQ(expected, s);
}

TEST(StringsTest, StartsWith) {
  EXPECT_TRUE(starts_with("--foo", "--"));
  EXPECT_TRUE(starts_with("asdf", "a"));
  EXPECT_TRUE(starts_with("asdf", "as"));
  EXPECT_TRUE(starts_with("asdf", "asd"));
  EXPECT_TRUE(starts_with("asdf", "asdf"));
  EXPECT_FALSE(starts_with("asdf", "asf"));
  EXPECT_FALSE(starts_with("asdf", "asdfe"));
}

TEST(StringsTest, EndssWith) {
  EXPECT_TRUE(ends_with("--foo", "foo"));
  EXPECT_TRUE(ends_with("asdf", "f"));
  EXPECT_TRUE(ends_with("asdf", "df"));
  EXPECT_TRUE(ends_with("asdf", "sdf"));
  EXPECT_TRUE(ends_with("asdf", "asdf"));
  EXPECT_FALSE(ends_with("asdf", "adf"));
  EXPECT_FALSE(ends_with("asdf", "easdf"));
}

TEST(StringsTest, StringJustify_Left) {
  string a("a");
  StringJustify(&a, 2, ' ', JustificationType::LEFT);
  EXPECT_EQ("a ", a);

  string b("b");
  StringJustify(&b, 2, ' ', JustificationType::LEFT);
  EXPECT_EQ("b ", b);
}

TEST(StringsTest, StringJustify_LeftOtherChar) {
  string a("a");
  StringJustify(&a, 2, '.', JustificationType::LEFT);
  EXPECT_EQ("a.", a);

  string b("b");
  StringJustify(&b, 2, '.', JustificationType::LEFT);
  EXPECT_EQ("b.", b);
}

TEST(StringsTest, StringJustify_Right) {
  string a("a");
  StringJustify(&a, 2, ' ', JustificationType::RIGHT);
  EXPECT_EQ(" a", a);

  string b("b");
  StringJustify(&b, 2, ' ', JustificationType::RIGHT);
  EXPECT_EQ(" b", b);
}

TEST(StringsTest, StringJustify_RightOtherChar) {
  string a("a");
  StringJustify(&a, 2, '.', JustificationType::RIGHT);
  EXPECT_EQ(".a", a);

  string b("b");
  StringJustify(&b, 2, '.', JustificationType::RIGHT);
  EXPECT_EQ(".b", b);
}

TEST(StringsTest, StringJustify_LongerString) {
  string a("aaa");
  StringJustify(&a, 2, ' ', JustificationType::LEFT);
  EXPECT_EQ("aa", a);

  string b("bbb");
  StringJustify(&b, 2, ' ', JustificationType::RIGHT);
  EXPECT_EQ("bb", b);
}

TEST(StringsTest, StringTrim) {
  string a = " a ";
  StringTrim(&a);
  EXPECT_EQ("a", a);

  string b = "b";
  StringTrim(&b);
  EXPECT_EQ("b", b);
}

TEST(StringsTest, StringTrimBegin) {
  string a = " a ";
  StringTrimBegin(&a);
  EXPECT_EQ("a ", a);
}

TEST(StringsTest, StringTrimEnd) {
  string a = " a ";
  StringTrimEnd(&a);
  EXPECT_EQ(" a", a);
}

TEST(StringsTest, StringUpperCase) {
  string a = "aB";
  StringUpperCase(&a);
  EXPECT_EQ("AB", a);
}

TEST(StringsTest, StringLowerCase) {
  string a = "aB";
  StringLowerCase(&a);
  EXPECT_EQ("ab", a);
}

TEST(StringsTest, StringRemoveWhitespace_charstar) {
  char s[81];
  strcpy(s, " h e l l o ");
  EXPECT_STREQ("hello", StringRemoveWhitespace(s));
  EXPECT_STREQ("hello", s);
}

TEST(StringsTest, StringRemoveWhitespace_str) {
  string s = " h e l l o ";
  StringRemoveWhitespace(&s);
  EXPECT_STREQ("hello", s.c_str());
}

TEST(StringsTest, StringRemoveChar) { EXPECT_STREQ("he", StringRemoveChar("hello world", 'l')); }

TEST(StringsTest, IEQuals_charstar) {
  EXPECT_TRUE(iequals("foo", "foo"));
  EXPECT_FALSE(iequals("foo", "fo"));
  EXPECT_FALSE(iequals("fo", "foo"));
  EXPECT_FALSE(iequals("", "foo"));
  EXPECT_FALSE(iequals("foo", ""));
}

TEST(StringsTest, IEQuals) {
  EXPECT_TRUE(iequals(string("foo"), string("foo")));
  EXPECT_FALSE(iequals(string("foo"), string("fo")));
  EXPECT_FALSE(iequals(string("fo"), string("foo")));
  EXPECT_FALSE(iequals(string(""), string("foo")));
  EXPECT_FALSE(iequals(string("foo"), string("")));
}

TEST(StringsTest, SizeWithoutColors) {
  EXPECT_EQ(1u, size_without_colors("a"));
  EXPECT_EQ(1u, size_without_colors("|#1a"));
  EXPECT_EQ(1u, size_without_colors("|09a"));
  EXPECT_EQ(1u, size_without_colors("|17|10a"));
}

TEST(StringsTest, SizeWithoutColors_AnsiStr) {
  EXPECT_EQ(0u, size_without_colors("\x1b[0m"));
  EXPECT_EQ(0u, size_without_colors("\x1b[0;33;46;1m"));
  EXPECT_EQ(1u, size_without_colors("|\x1b[0;33;46;1m"));
  EXPECT_EQ(3u, size_without_colors("|15\x1b[0;33;46;1mabc"));
  EXPECT_EQ(3u, size_without_colors("\x1b[0m|15\x1b[0;33;46;1ma\x1b[0mb\x1b[0mc\x1b[0m"));
}

TEST(StringsTest, TrimToSizeIgnoreColors) {
  EXPECT_EQ("a", trim_to_size_ignore_colors("a", 1));
  EXPECT_EQ("|#5a", trim_to_size_ignore_colors("|#5a", 1));
  EXPECT_EQ("|09|16a", trim_to_size_ignore_colors("|09|16a", 1));
  EXPECT_EQ("|09|16a|09", trim_to_size_ignore_colors("|09|16a|09", 1));
  EXPECT_EQ("|09|16a", trim_to_size_ignore_colors("|09|16aa|09", 1));
}

TEST(StringsTest, PadTo) { 
  auto result = pad_to("a", 2);
  EXPECT_EQ(result, "a ");
}

TEST(StringsTest, PadToPad) {
  auto result = pad_to("a", 'x', 2);
  EXPECT_EQ(result, "ax");
}

TEST(StringsTest, LPadTo) {
  auto result = lpad_to("a", 2);
  EXPECT_EQ(result, " a");
}

TEST(StringsTest, LPadToPad) {
  auto result = lpad_to("a", 'x', 2);
  EXPECT_EQ(result, "xa");
}
