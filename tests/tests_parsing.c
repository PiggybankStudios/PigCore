/*
File:   tests_parsing.c
Author: Taylor Robbins
Date:   02\01\2025
Description:
	** Contains a bunch of test cases for the functions in misc_parsing.h
*/

#if 0

void TestParsingFunctions()
{
	Result parseError = Result_None;
	
	WriteLine_O("+==============================+");
	WriteLine_O("|          R32 Tests           |");
	WriteLine_O("+==============================+");
	r32 floatValue = 0; 
	#define TRY_PARSE_R32_TEST(parseString) if (!TryParseR32(StrLit(parseString), &floatValue, &parseError)) { PrintLine_E("TryParseR32(\"" parseString "\") failed: %s", GetResultStr(parseError)); } else { PrintLine_I("\"" parseString "\" -> %f", floatValue); }
	TRY_PARSE_R32_TEST("0.0f");
	TRY_PARSE_R32_TEST("0.1f");
	TRY_PARSE_R32_TEST("0.2");
	TRY_PARSE_R32_TEST("0.3");
	TRY_PARSE_R32_TEST("0.4000");
	TRY_PARSE_R32_TEST("0.105");
	TRY_PARSE_R32_TEST("1");
	TRY_PARSE_R32_TEST("100");
	TRY_PARSE_R32_TEST("-10");
	TRY_PARSE_R32_TEST(".5");
	TRY_PARSE_R32_TEST("-.1");
	TRY_PARSE_R32_TEST("-0.5628");
	TRY_PARSE_R32_TEST("10.5628");
	TRY_PARSE_R32_TEST("1e4");
	TRY_PARSE_R32_TEST("1e+4");
	TRY_PARSE_R32_TEST("1e-4");
	TRY_PARSE_R32_TEST("+8");
	TRY_PARSE_R32_TEST("1000");
	TRY_PARSE_R32_TEST("10000");
	TRY_PARSE_R32_TEST("100000");
	TRY_PARSE_R32_TEST("1000000");
	TRY_PARSE_R32_TEST("10000000");
	TRY_PARSE_R32_TEST("100000000");
	TRY_PARSE_R32_TEST("1000000000");
	TRY_PARSE_R32_TEST("10000000000");
	TRY_PARSE_R32_TEST("100000000000");
	TRY_PARSE_R32_TEST("1000000000000");
	TRY_PARSE_R32_TEST("10000000000000");
	TRY_PARSE_R32_TEST("100000000000000");
	TRY_PARSE_R32_TEST("1000000000000000");
	TRY_PARSE_R32_TEST("10000000000000000");
	TRY_PARSE_R32_TEST("");
	TRY_PARSE_R32_TEST("-");
	TRY_PARSE_R32_TEST("+");
	TRY_PARSE_R32_TEST(".");
	TRY_PARSE_R32_TEST("+-10");
	TRY_PARSE_R32_TEST("NaN");
	TRY_PARSE_R32_TEST("Inf");
	TRY_PARSE_R32_TEST("+Inf");
	TRY_PARSE_R32_TEST("Infinity");
	TRY_PARSE_R32_TEST("nan");
	TRY_PARSE_R32_TEST("ab");
	
	WriteLine_O("+==============================+");
	WriteLine_O("|          R64 Tests           |");
	WriteLine_O("+==============================+");
	r64 doubleValue = 0; 
	#define TRY_PARSE_R64_TEST(parseString) if (!TryParseR64(StrLit(parseString), &doubleValue, &parseError)) { PrintLine_E("TryParseR64(\"" parseString "\") failed: %s", GetResultStr(parseError)); } else { PrintLine_I("\"" parseString "\" -> %lf", doubleValue); }
	TRY_PARSE_R64_TEST("0.0f");
	TRY_PARSE_R64_TEST("0.1f");
	TRY_PARSE_R64_TEST("0.2");
	TRY_PARSE_R64_TEST("0.3");
	TRY_PARSE_R64_TEST("0.4000");
	TRY_PARSE_R64_TEST("0.105");
	TRY_PARSE_R64_TEST("1");
	TRY_PARSE_R64_TEST("100");
	TRY_PARSE_R64_TEST("-10");
	TRY_PARSE_R64_TEST(".5");
	TRY_PARSE_R64_TEST("-.1");
	TRY_PARSE_R64_TEST("-0.5628");
	TRY_PARSE_R64_TEST("10.5628");
	TRY_PARSE_R64_TEST("+8");
	TRY_PARSE_R64_TEST("1000");
	TRY_PARSE_R64_TEST("10000");
	TRY_PARSE_R64_TEST("100000");
	TRY_PARSE_R64_TEST("1000000");
	TRY_PARSE_R64_TEST("10000000");
	TRY_PARSE_R64_TEST("100000000");
	TRY_PARSE_R64_TEST("1000000000");
	TRY_PARSE_R64_TEST("10000000000");
	TRY_PARSE_R64_TEST("100000000000");
	TRY_PARSE_R64_TEST("1000000000000");
	TRY_PARSE_R64_TEST("10000000000000");
	TRY_PARSE_R64_TEST("100000000000000");
	TRY_PARSE_R64_TEST("1000000000000000");
	TRY_PARSE_R64_TEST("10000000000000000");
	TRY_PARSE_R64_TEST("10000000000000000.1");
	TRY_PARSE_R64_TEST("");
	TRY_PARSE_R64_TEST("-");
	TRY_PARSE_R64_TEST("+");
	TRY_PARSE_R64_TEST(".");
	TRY_PARSE_R64_TEST("+-10");
	TRY_PARSE_R64_TEST("NaN");
	TRY_PARSE_R64_TEST("Inf");
	TRY_PARSE_R64_TEST("+Inf");
	TRY_PARSE_R64_TEST("Infinity");
	TRY_PARSE_R64_TEST("nan");
	TRY_PARSE_R64_TEST("ab");
	
	WriteLine_O("+==============================+");
	WriteLine_O("|          U64 Tests           |");
	WriteLine_O("+==============================+");
	u64 valueU64 = 0; 
	#define TRY_PARSE_U64_TEST(parseString) if (!TryParseU64(StrLit(parseString), &valueU64, &parseError)) { PrintLine_E("TryParseU64(\"" parseString "\") failed: %s", GetResultStr(parseError)); } else { PrintLine_I("\"" parseString "\" -> %llu", valueU64); }
	TRY_PARSE_U64_TEST("1");
	TRY_PARSE_U64_TEST("100");
	TRY_PARSE_U64_TEST("100A");
	TRY_PARSE_U64_TEST("0xFF");
	TRY_PARSE_U64_TEST("0123456789");
	TRY_PARSE_U64_TEST("0xABCDEF");
	TRY_PARSE_U64_TEST("0xabcdef");
	TRY_PARSE_U64_TEST("0xF1");
	TRY_PARSE_U64_TEST("0b1001");
	TRY_PARSE_U64_TEST("0b1111");
	TRY_PARSE_U64_TEST("0b1119");
	TRY_PARSE_U64_TEST("+1");
	TRY_PARSE_U64_TEST("-10");
	TRY_PARSE_U64_TEST("");
	TRY_PARSE_U64_TEST("0x");
	TRY_PARSE_U64_TEST("10 20");
	TRY_PARSE_U64_TEST("0xFFFFFFFFFFFFFFFE");
	TRY_PARSE_U64_TEST("0xFFFFFFFFFFFFFFFF");
	TRY_PARSE_U64_TEST("18446744073709551615");
	TRY_PARSE_U64_TEST("18446744073709551616"); //TODO: Incorrectly Succeed
	TRY_PARSE_U64_TEST("18446744073709551617"); //TODO: Incorrectly Succeed
	TRY_PARSE_U64_TEST("18446744073709551618"); //TODO: Incorrectly Succeed
	TRY_PARSE_U64_TEST("18446744073709551619"); //TODO: Incorrectly Succeed
	TRY_PARSE_U64_TEST("18446744073709551620");
	TRY_PARSE_U64_TEST("18446744073709551621");
	TRY_PARSE_U64_TEST("18446744073709551622");
	TRY_PARSE_U64_TEST("18446744073709551623");
	TRY_PARSE_U64_TEST("18446744073709551624");
	TRY_PARSE_U64_TEST("18446744073709551625");
	TRY_PARSE_U64_TEST("18446744073709551626");
	TRY_PARSE_U64_TEST("18446744073709551627");
	TRY_PARSE_U64_TEST("18446744073709551628");
	TRY_PARSE_U64_TEST("18446744073709551629");
	TRY_PARSE_U64_TEST("18446744073710000000");
	
	WriteLine_O("+==============================+");
	WriteLine_O("|          U32 Tests           |");
	WriteLine_O("+==============================+");
	u32 valueU32 = 0; 
	#define TRY_PARSE_U32_TEST(parseString) if (!TryParseU32(StrLit(parseString), &valueU32, &parseError)) { PrintLine_E("TryParseU32(\"" parseString "\") failed: %s", GetResultStr(parseError)); } else { PrintLine_I("\"" parseString "\" -> %lu", valueU32); }
	TRY_PARSE_U32_TEST("10");
	TRY_PARSE_U32_TEST("123456789");
	TRY_PARSE_U32_TEST("0xABCDEF");
	TRY_PARSE_U32_TEST("0xabcdef");
	TRY_PARSE_U32_TEST("0xfF");
	TRY_PARSE_U32_TEST("4000000000");
	TRY_PARSE_U32_TEST("4294967295");
	TRY_PARSE_U32_TEST("4294967296");
	TRY_PARSE_U32_TEST("4294967297");
	TRY_PARSE_U32_TEST("4294967298");
	TRY_PARSE_U32_TEST("4294967299");
	TRY_PARSE_U32_TEST("4294967300");
	TRY_PARSE_U32_TEST("0xFFFFFFFF");
	TRY_PARSE_U32_TEST("0x100000000");
	TRY_PARSE_U32_TEST("0xFFFFFFFFF");
	TRY_PARSE_U32_TEST("0x10000000000000001");
	
	WriteLine_O("+==============================+");
	WriteLine_O("|          I64 Tests           |");
	WriteLine_O("+==============================+");
	i64 valueI64 = 0; 
	#define TRY_PARSE_I64_TEST(parseString) if (!TryParseI64(StrLit(parseString), &valueI64, &parseError)) { PrintLine_E("TryParseI64(\"" parseString "\") failed: %s", GetResultStr(parseError)); } else { PrintLine_I("\"" parseString "\" -> %lld", valueI64); }
	TRY_PARSE_I64_TEST("10");
	TRY_PARSE_I64_TEST("123456789");
	TRY_PARSE_I64_TEST("-100");
	TRY_PARSE_I64_TEST("-123456789");
	TRY_PARSE_I64_TEST("+10");
	TRY_PARSE_I64_TEST("-");
	TRY_PARSE_I64_TEST("+");
	TRY_PARSE_I64_TEST("+0");
	TRY_PARSE_I64_TEST("4000000000");
	TRY_PARSE_I64_TEST("9223372036854775807");
	TRY_PARSE_I64_TEST("-9223372036854775807");
	TRY_PARSE_I64_TEST("9223372036854775808");
	TRY_PARSE_I64_TEST("-9223372036854775808");
	TRY_PARSE_I64_TEST("-9223372036854775809");
	TRY_PARSE_I64_TEST("0x7FFFFFFFFFFFFFFF");
	TRY_PARSE_I64_TEST("0x8000000000000000");
	TRY_PARSE_I64_TEST("-0x8000000000000000");
	
	WriteLine_O("+==============================+");
	WriteLine_O("|          I32 Tests           |");
	WriteLine_O("+==============================+");
	i32 valueI32 = 0; 
	#define TRY_PARSE_I32_TEST(parseString) if (!TryParseI32(StrLit(parseString), &valueI32, &parseError)) { PrintLine_E("TryParseI32(\"" parseString "\") failed: %s", GetResultStr(parseError)); } else { PrintLine_I("\"" parseString "\" -> %ld", valueI32); }
	TRY_PARSE_I32_TEST("10");
	TRY_PARSE_I32_TEST("40000");
	TRY_PARSE_I32_TEST("-12");
	TRY_PARSE_I32_TEST("+-12");
	TRY_PARSE_I32_TEST("-");
	TRY_PARSE_I32_TEST("+");
	TRY_PARSE_I32_TEST("+45");
	TRY_PARSE_I32_TEST("0xFf");
	TRY_PARSE_I32_TEST("0x7FFFFFFF");
	TRY_PARSE_I32_TEST("0x80000000");
	TRY_PARSE_I32_TEST("2147483647");
	TRY_PARSE_I32_TEST("-2147483647");
	TRY_PARSE_I32_TEST("2147483648");
	TRY_PARSE_I32_TEST("-2147483648");
	TRY_PARSE_I32_TEST("-2147483649");
}

#endif
