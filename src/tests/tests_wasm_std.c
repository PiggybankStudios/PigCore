/*
File:   tests_wasm_std.c
Author: Taylor Robbins
Date:   01\10\2025
Description: 
	** Contains a bunch of tests of the C Standard Library implementation we made for WebAssembly
*/

#if TARGET_IS_WASM

bool TestCaseInt_(const char* testCaseStr, int result, int expectedValue)
{
	if (result != expectedValue)
	{
		MyPrint("Test case failed!");
		MyPrint("Test \"%s\"", testCaseStr);
		MyPrint("Expected %d", expectedValue);
		MyPrint("Got %d", result);
		return false;
	}
	else { return true; }
}
#define TestCaseInt(testCase, expectedValue) do { numCases++; if (TestCaseInt_(#testCase, (testCase), (expectedValue))) { numCasesSucceeded++; } } while(0)

bool TestCaseFloat_(const char* testCaseStr, float result, float expectedValue)
{
	if (result != expectedValue && !(isnan(result) && isnan(expectedValue)))
	{
		MyPrint("Test case failed!");
		MyPrint("Test \"%s\"", testCaseStr);
		MyPrint("Expected %f", expectedValue);
		MyPrint("Got %f", result);
		return false;
	}
	else { return true; }
}
#define TestCaseFloat(testCase, expectedValue) do { if (TestCaseFloat_(#testCase, (testCase), (expectedValue))) { numCasesSucceeded++; } numCases++; } while(0)

bool TestCaseDouble_(const char* testCaseStr, double result, double expectedValue)
{
	if (result != expectedValue && !(isnan(result) && isnan(expectedValue)))
	{
		MyPrint("Test case failed!");
		MyPrint("Test \"%s\"", testCaseStr);
		MyPrint("Expected %lf", expectedValue);
		MyPrint("Got %lf", result);
		return false;
	}
	else { return true; }
}
#define TestCaseDouble(testCase, expectedValue) do { if (TestCaseDouble_(#testCase, (testCase), (expectedValue))) { numCasesSucceeded++; } numCases++; } while(0)

bool TestCasePrint_(const char* expectedStr, const char* formatStr, ...)
{
	char printBuffer[1024];
	va_list args;
	va_start(args, formatStr);
	int printResult = vsnprintf(&printBuffer[0], sizeof(printBuffer), formatStr, args);
	va_end(args);
	if (printResult < 0)
	{
		MyPrint("Test case failed!");
		MyPrint("Format \"%s\"", formatStr);
		MyPrint("Expected \"%s\"", expectedStr);
		MyPrint("Got Print Error %d", printResult);
		return false;
	}
	else
	{
		if (strncmp(&printBuffer[0], expectedStr, sizeof(printBuffer)) != 0)
		{
			MyPrint("Test case failed!");
			MyPrint("Format \"%s\"", formatStr);
			MyPrint("Expected \"%s\"", expectedStr);
			MyPrint("Got \"%s\"", &printBuffer[0]);
			return false;
		}
		else { return true; }
	}
}
#define TestCasePrint(expectedStr, formatStr, ...) do { if (TestCasePrint_((expectedStr), (formatStr), ##__VA_ARGS__)) { numCasesSucceeded++; } numCases++; } while(0)

void RunMathTestCases()
{
	int numCases = 0;
	int numCasesSucceeded = 0;
	
	TestCaseFloat(fminf(-13.1234f, 1.2f), -13.1234f);
	TestCaseFloat(fminf(1.2f, -13.1234f), -13.1234f);
	TestCaseFloat(fminf(1.2f, 0.0f), 0.0f);
	TestCaseFloat(fminf(1.2f, INFINITY), 1.2f);
	TestCaseFloat(fminf(1.2f, -INFINITY), -INFINITY);
	TestCaseFloat(fminf(1.2f, NAN), 1.2f);
	TestCaseFloat(fminf(1.20000001f, 1.2f), 1.2f);
	
	TestCaseDouble(fmin(-13.1234, 1.2), -13.1234);
	TestCaseDouble(fmin(1.2, -13.1234), -13.1234);
	TestCaseDouble(fmin(1.2, 0.0), 0.0);
	TestCaseDouble(fmin(1.2, INFINITY), 1.2);
	TestCaseDouble(fmin(1.2, -INFINITY), -INFINITY);
	TestCaseDouble(fmin(1.2, NAN), 1.2);
	TestCaseDouble(fmin(1.20000001, 1.2), 1.2);
	
	TestCaseFloat(fmaxf(-13.1234f, 1.2f), 1.2f);
	TestCaseFloat(fmaxf(1.2f, -13.1234f), 1.2f);
	TestCaseFloat(fmaxf(1.2f, 0.0f), 1.2f);
	TestCaseFloat(fmaxf(1.2f, INFINITY), INFINITY);
	TestCaseFloat(fmaxf(1.2f, -INFINITY), 1.2f);
	TestCaseFloat(fmaxf(1.2f, NAN), 1.2f);
	TestCaseFloat(fmaxf(1.20000001f, 1.2f), 1.20000001f);
	
	TestCaseDouble(fmax(-13.1234, 1.2), 1.2);
	TestCaseDouble(fmax(1.2, -13.1234), 1.2);
	TestCaseDouble(fmax(1.2, 0.0), 1.2);
	TestCaseDouble(fmax(1.2, INFINITY), INFINITY);
	TestCaseDouble(fmax(1.2, -INFINITY), 1.2);
	TestCaseDouble(fmax(1.2, NAN), 1.2);
	TestCaseDouble(fmax(1.20000001, 1.2), 1.20000001);
	
	TestCaseFloat(fmodf(5.0f, 3.0f), 2.0f);
	TestCaseFloat(fmodf(-13.1234f, 1.0f), -0.12339973449707031f); //very close to -0.1234
	TestCaseFloat(fmodf(5.24f, 1.33f), 1.2499996423721313f); //very close to 1.25
	TestCaseFloat(fmodf(INFINITY, 1.0f), NAN);
	TestCaseFloat(fmodf(1.2f, 0.0f), NAN);
	
	TestCaseDouble(fmod(5.0, 3.0), 2.0);
	TestCaseDouble(fmod(-13.1234, 1.0), -0.12340000000000018); //very close to -0.1234
	TestCaseDouble(fmod(5.24, 1.33), 1.25);
	TestCaseDouble(fmod(INFINITY, 1.0), NAN);
	TestCaseDouble(fmod(1.2, 0.0), NAN);
	
	TestCaseFloat(fabs(-13.1234f), 13.1234f);
	TestCaseFloat(fabs(13.1234f), 13.1234f);
	TestCaseFloat(fabs(-INFINITY), INFINITY);
	TestCaseFloat(fabs(INFINITY), INFINITY);
	TestCaseFloat(fabs(NAN), NAN);
	TestCaseFloat(fabs(0.0f), 0.0f);
	TestCaseFloat(fabs(-1000000.0001f), 1000000.0001f);
	
	TestCaseDouble(fabs(-13.1234), 13.1234);
	TestCaseDouble(fabs(13.1234), 13.1234);
	TestCaseDouble(fabs(-INFINITY), INFINITY);
	TestCaseDouble(fabs(INFINITY), INFINITY);
	TestCaseDouble(fabs(NAN), NAN);
	TestCaseDouble(fabs(0.0), 0.0);
	TestCaseDouble(fabs(-1000000.0001), 1000000.0001);
	
	TestCaseFloat(roundf(1.24f), 1.0f);
	TestCaseFloat(roundf(1.0f), 1.0f);
	TestCaseFloat(roundf(0.7f), 1.0f);
	TestCaseFloat(roundf(-1.24f), -1.0f);
	TestCaseFloat(roundf(-1.0f), -1.0f);
	TestCaseFloat(roundf(-0.7f), -1.0f);
	TestCaseFloat(roundf(0.5f), 1.0f);
	TestCaseFloat(roundf(0.5f - FLT_EPSILON), 0.0f);
	TestCaseFloat(roundf(-0.5f), -1.0f);
	TestCaseFloat(roundf(-0.5f + FLT_EPSILON), 0.0f);
	TestCaseFloat(roundf(INFINITY), INFINITY);
	TestCaseFloat(roundf(-INFINITY), -INFINITY);
	TestCaseFloat(roundf(NAN), NAN);
	
	TestCaseDouble(round(1.24), 1.0);
	TestCaseDouble(round(1.0), 1.0);
	TestCaseDouble(round(0.7), 1.0);
	TestCaseDouble(round(-1.24), -1.0);
	TestCaseDouble(round(-1.0), -1.0);
	TestCaseDouble(round(-0.7), -1.0);
	TestCaseDouble(round(0.5), 1.0);
	TestCaseDouble(round(0.5 - DBL_EPSILON), 0.0);
	TestCaseDouble(round(-0.5), -1.0);
	TestCaseDouble(round(-0.5 + DBL_EPSILON), 0.0);
	TestCaseDouble(round(INFINITY), INFINITY);
	TestCaseDouble(round(-INFINITY), -INFINITY);
	TestCaseDouble(round(NAN), NAN);
	
	TestCaseFloat(floorf(1.0f), 1.0f);
	TestCaseFloat(floorf(2.0f), 2.0f);
	TestCaseFloat(floorf(1.001f), 1.0f);
	TestCaseFloat(floorf(1.24f), 1.0f);
	TestCaseFloat(floorf(1.99f), 1.0f);
	TestCaseFloat(floorf(-1.0f), -1.0f);
	TestCaseFloat(floorf(-2.0f), -2.0f);
	TestCaseFloat(floorf(-1.001f), -2.0f);
	TestCaseFloat(floorf(-1.24f), -2.0f);
	TestCaseFloat(floorf(-1.99f), -2.0f);
	TestCaseFloat(floorf(1024.5f), 1024.0f);
	TestCaseFloat(floorf(INFINITY), INFINITY);
	TestCaseFloat(floorf(-INFINITY), -INFINITY);
	TestCaseFloat(floorf(NAN), NAN);
	
	TestCaseDouble(floor(1.0), 1.0);
	TestCaseDouble(floor(2.0), 2.0);
	TestCaseDouble(floor(1.001), 1.0);
	TestCaseDouble(floor(1.24), 1.0);
	TestCaseDouble(floor(1.99), 1.0);
	TestCaseDouble(floor(-1.0), -1.0);
	TestCaseDouble(floor(-2.0), -2.0);
	TestCaseDouble(floor(-1.001), -2.0);
	TestCaseDouble(floor(-1.24), -2.0);
	TestCaseDouble(floor(-1.99), -2.0);
	TestCaseDouble(floor(1024.5), 1024.0);
	TestCaseDouble(floor(INFINITY), INFINITY);
	TestCaseDouble(floor(-INFINITY), -INFINITY);
	TestCaseDouble(floor(NAN), NAN);
	
	TestCaseFloat(ceilf(1.0f), 1.0f);
	TestCaseFloat(ceilf(2.0f), 2.0f);
	TestCaseFloat(ceilf(1.001f), 2.0f);
	TestCaseFloat(ceilf(1.24f), 2.0f);
	TestCaseFloat(ceilf(1.99f), 2.0f);
	TestCaseFloat(ceilf(-1.0f), -1.0f);
	TestCaseFloat(ceilf(-2.0f), -2.0f);
	TestCaseFloat(ceilf(-1.001f), -1.0f);
	TestCaseFloat(ceilf(-1.24f), -1.0f);
	TestCaseFloat(ceilf(-1.99f), -1.0f);
	TestCaseFloat(ceilf(1024.5f), 1025.0f);
	TestCaseFloat(ceilf(INFINITY), INFINITY);
	TestCaseFloat(ceilf(-INFINITY), -INFINITY);
	TestCaseFloat(ceilf(NAN), NAN);
	
	TestCaseDouble(ceil(1.0), 1.0);
	TestCaseDouble(ceil(2.0), 2.0);
	TestCaseDouble(ceil(1.001), 2.0);
	TestCaseDouble(ceil(1.24), 2.0);
	TestCaseDouble(ceil(1.99), 2.0);
	TestCaseDouble(ceil(-1.0), -1.0);
	TestCaseDouble(ceil(-2.0), -2.0);
	TestCaseDouble(ceil(-1.001), -1.0);
	TestCaseDouble(ceil(-1.24), -1.0);
	TestCaseDouble(ceil(-1.99), -1.0);
	TestCaseDouble(ceil(1024.5), 1025.0);
	TestCaseDouble(ceil(INFINITY), INFINITY);
	TestCaseDouble(ceil(-INFINITY), -INFINITY);
	TestCaseDouble(ceil(NAN), NAN);
	
	TestCaseFloat(scalbnf(1.0f, 1), 2.0f); //scalbn(a, b) = a * 2^b
	TestCaseFloat(scalbnf(1.0f, 4), 16.0f);
	TestCaseFloat(scalbnf(4.0f, 1), 8.0f);
	TestCaseFloat(scalbnf(3.14f, 0), 3.14f);
	TestCaseFloat(scalbnf(1.0f, -1), 0.5f);
	TestCaseFloat(scalbnf(1.0f, -2), 0.25f);
	TestCaseFloat(scalbnf(INFINITY, 2), INFINITY);
	TestCaseFloat(scalbnf(NAN, 2), NAN);
	
	TestCaseDouble(scalbn(1.0, 1), 2.0);
	TestCaseDouble(scalbn(1.0, 4), 16.0);
	TestCaseDouble(scalbn(4.0, 1), 8.0);
	TestCaseDouble(scalbn(3.14, 0), 3.14);
	TestCaseDouble(scalbn(1.0, -1), 0.5);
	TestCaseDouble(scalbn(1.0, -2), 0.25);
	TestCaseDouble(scalbn(INFINITY, 2), INFINITY);
	TestCaseDouble(scalbn(NAN, 2), NAN);
	
	TestCaseFloat(sqrtf(0.0f), 0.0f);
	TestCaseFloat(sqrtf(1.0f), 1.0f);
	TestCaseFloat(sqrtf(0.25f), 0.5f);
	TestCaseFloat(sqrtf(4.0f), 2.0f);
	TestCaseFloat(sqrtf(2.0f), 1.4142135381698608f);
	TestCaseFloat(sqrtf(-1.0f), NAN);
	TestCaseFloat(sqrtf(100.0f), 10.0f);
	TestCaseFloat(sqrtf(101.0f), 10.049875259399414f);
	TestCaseFloat(sqrtf(INFINITY), INFINITY);
	TestCaseFloat(sqrtf(-INFINITY), NAN);
	TestCaseFloat(sqrtf(NAN), NAN);
	
	TestCaseDouble(sqrt(0.0), 0.0);
	TestCaseDouble(sqrt(1.0), 1.0);
	TestCaseDouble(sqrt(0.25), 0.5);
	TestCaseDouble(sqrt(4.0), 2.0);
	TestCaseDouble(sqrt(2.0), 1.4142135623730951);
	TestCaseDouble(sqrt(-1.0), NAN);
	TestCaseDouble(sqrt(100.0), 10.0);
	TestCaseDouble(sqrt(101.0), 10.04987562112089);
	TestCaseDouble(sqrt(INFINITY), INFINITY);
	TestCaseDouble(sqrt(-INFINITY), NAN);
	TestCaseDouble(sqrt(NAN), NAN);
	
	TestCaseFloat(cbrtf(0.0f), 0.0f);
	TestCaseFloat(cbrtf(1.0f), 1.0f);
	TestCaseFloat(cbrtf(0.125f), 0.5f);
	TestCaseFloat(cbrtf(-0.125f), -0.5f);
	TestCaseFloat(cbrtf(8.0f), 2.0f);
	TestCaseFloat(cbrtf(2.0f), 1.2599210739135742f);
	TestCaseFloat(cbrtf(-1.0f), -1.0f);
	TestCaseFloat(cbrtf(1000.0f), 10.0f);
	TestCaseFloat(cbrtf(1001.0f), 10.003332138061523f);
	TestCaseFloat(cbrtf(INFINITY), INFINITY);
	TestCaseFloat(cbrtf(-INFINITY), -INFINITY);
	TestCaseFloat(cbrtf(NAN), NAN);
	
	TestCaseDouble(cbrt(0.0), 0.0);
	TestCaseDouble(cbrt(1.0), 1.0);
	TestCaseDouble(cbrt(0.125), 0.5);
	TestCaseDouble(cbrt(-0.125), -0.5);
	TestCaseDouble(cbrt(8.0), 2.0);
	TestCaseDouble(cbrt(2.0), 1.2599210498948732);
	TestCaseDouble(cbrt(-1.0), -1.0);
	TestCaseDouble(cbrt(1000.0), 10.0);
	TestCaseDouble(cbrt(1001.0), 10.003332222839095);
	TestCaseDouble(cbrt(INFINITY), INFINITY);
	TestCaseDouble(cbrt(-INFINITY), -INFINITY);
	TestCaseDouble(cbrt(NAN), NAN);
	
	TestCaseFloat(sinf(0.0f), 0.0f);
	TestCaseFloat(sinf(0.5f), 0.4794255495071411f);
	TestCaseFloat(sinf(1.0f), 0.8414709568023682f);
	TestCaseFloat(sinf(M_PI), -8.742277657347586e-8f); //very close to 0
	TestCaseFloat(sinf(2*M_PI), 1.7484555314695172e-7f); //very close to 0
	TestCaseFloat(sinf(M_PI_2), 1.0f);
	TestCaseFloat(sinf(3*M_PI_2), -1.0f);
	
	TestCaseDouble(sin(0.0), 0.0);
	TestCaseDouble(sin(0.5), 0.479425538604203);
	TestCaseDouble(sin(1.0), 0.8414709848078965);
	TestCaseDouble(sin(M_PI), 1.2246467991473532e-16); //very close to 0
	TestCaseDouble(sin(2*M_PI), -2.4492935982947064e-16); //very close to 0
	TestCaseDouble(sin(M_PI_2), 1.0);
	TestCaseDouble(sin(3*M_PI_2), -1.0);
	
	TestCaseFloat(cosf(0.0f), 1.0f);
	TestCaseFloat(cosf(0.5f), 0.8775825500488281f);
	TestCaseFloat(cosf(1.0f), 0.5403022766113281f);
	TestCaseFloat(cosf(M_PI), -1.0f);
	TestCaseFloat(cosf(2*M_PI), 1.0f);
	TestCaseFloat(cosf(M_PI_2), -4.371138828673793e-8f); //very close to 0
	TestCaseFloat(cosf(3*M_PI_2), 1.1924880638503055e-8f); //very close to 0
	
	TestCaseDouble(cos(0.0), 1.0);
	TestCaseDouble(cos(0.5), 0.8775825618903728);
	TestCaseDouble(cos(1.0), 0.5403023058681398);
	TestCaseDouble(cos(M_PI), -1.0);
	TestCaseDouble(cos(2*M_PI), 1.0);
	TestCaseDouble(cos(M_PI_2), 6.123233995736766e-17); //very close to 0
	TestCaseDouble(cos(3*M_PI_2), -1.8369701987210297e-16); //very close to 0
	
	TestCaseFloat(tanf(0.0f), 0.0f);
	TestCaseFloat(tanf(0.5f), 0.5463024973869324f);
	TestCaseFloat(tanf(1.0f), 1.5574077367782593f);
	TestCaseFloat(tanf(M_PI), 8.742278367890322e-8f); //very close to 0
	TestCaseFloat(tanf(2*M_PI), 1.7484556735780643e-7f); //very close to 0
	TestCaseFloat(tanf(M_PI_2 - FLT_EPSILON), 13245402.0f); //very large
	TestCaseFloat(tanf(M_PI_2 + FLT_EPSILON), -6137956.0f); //very large (negative)
	
	TestCaseDouble(tan(0.0), 0.0);
	TestCaseDouble(tan(0.5f), 0.5463024898437905);
	TestCaseDouble(tan(1.0), 1.5574077246549023);
	TestCaseDouble(tan(M_PI), -1.2246467991473532e-16); //very close to 0
	TestCaseDouble(tan(2*M_PI), -2.4492935982947064e-16); //very close to 0
	TestCaseDouble(tan(M_PI_2 - DBL_EPSILON), 3530114321217157.5); //very large
	TestCaseDouble(tan(M_PI_2 + DBL_EPSILON), -6218431163823738.0); //very large (negative)
	
	TestCaseFloat(asinf(0.0f), 0.0f);
	TestCaseFloat(asinf(0.1f), 0.1001674234867096f);
	TestCaseFloat(asinf(0.5f), 0.5235987901687622f);
	TestCaseFloat(asinf(1.0f), 1.5707963705062866f);
	TestCaseFloat(asinf(-1.0f), -1.5707963705062866f);
	TestCaseFloat(asinf(1.1f), NAN);
	TestCaseFloat(asinf(-1.1f), NAN);
	
	TestCaseDouble(asin(0.0), 0.0);
	TestCaseDouble(asin(0.1), 0.1001674211615598);
	TestCaseDouble(asin(0.5), 0.5235987755982989);
	TestCaseDouble(asin(1.0), 1.5707963267948966);
	TestCaseDouble(asin(-1.0), -1.5707963267948966);
	TestCaseDouble(asin(1.1), NAN);
	TestCaseDouble(asin(-1.1), NAN);
	
	TestCaseFloat(acosf(0.0f), 1.570796251296997f);
	TestCaseFloat(acosf(0.1f), 1.4706288576126099f);
	TestCaseFloat(acosf(0.5f), 1.0471975803375244f);
	TestCaseFloat(acosf(1.0f), 0.0f);
	TestCaseFloat(acosf(-1.0f), 3.141592502593994f);
	TestCaseFloat(acosf(1.1f), NAN);
	TestCaseFloat(acosf(-1.1f), NAN);
	
	TestCaseDouble(acos(0.0), 1.5707963267948966);
	TestCaseDouble(acos(0.1), 1.4706289056333368);
	TestCaseDouble(acos(0.5), 1.0471975511965979);
	TestCaseDouble(acos(1.0), 0.0);
	TestCaseDouble(acos(-1.0), 3.141592653589793);
	TestCaseDouble(acos(1.1), NAN);
	TestCaseDouble(acos(-1.1), NAN);
	
	TestCaseFloat(atanf(0.0f), 0.0f);
	TestCaseFloat(atanf(0.1f), 0.09966865181922913f);
	TestCaseFloat(atanf(0.5f), 0.46364760398864746f);
	TestCaseFloat(atanf(1.0f), 0.7853981852531433f);
	TestCaseFloat(atanf(-1.0f), -0.7853981852531433f);
	TestCaseFloat(atanf(10.0f), 1.4711276292800903f);
	TestCaseFloat(atanf(-10.0f), -1.4711276292800903f);
	
	TestCaseDouble(atan(0.0), 0.0);
	TestCaseDouble(atan(0.1), 0.09966865249116204);
	TestCaseDouble(atan(0.5), 0.4636476090008061);
	TestCaseDouble(atan(1.0), 0.7853981633974483);
	TestCaseDouble(atan(-1.0), -0.7853981633974483);
	TestCaseDouble(atan(10.0), 1.4711276743037347);
	TestCaseDouble(atan(-10.0), -1.4711276743037347);
	
	TestCaseFloat(atan2f(0.0f, 1.0f), 0.0f);
	TestCaseFloat(atan2f(1.0f, 10.0f), 0.09966865181922913f);
	TestCaseFloat(atan2f(1.0f, 2.0f), 0.46364760398864746f);
	TestCaseFloat(atan2f(1.0f, 1.0f), 0.7853981852531433f);
	TestCaseFloat(atan2f(-1.0f, 1.0f), -0.7853981852531433f);
	TestCaseFloat(atan2f(100.0f, 10.0f), 1.4711276292800903f);
	TestCaseFloat(atan2f(-100.0f, 10.0f), -1.4711276292800903f);
	
	TestCaseDouble(atan2(0.0, 1.0), 0.0);
	TestCaseDouble(atan2(1.0, 10.0), 0.09966865249116204);
	TestCaseDouble(atan2(1.0, 2.0), 0.4636476090008061);
	TestCaseDouble(atan2(1.0, 1.0), 0.7853981633974483);
	TestCaseDouble(atan2(-1.0, 1.0), -0.7853981633974483);
	TestCaseDouble(atan2(100.0, 10.0), 1.4711276743037347);
	TestCaseDouble(atan2(-100.0, 10.0), -1.4711276743037347);
	
	TestCaseFloat(powf(2.0f, 2.0f), 4.0f);
	TestCaseFloat(powf(2.0f, 3.0f), 8.0f);
	TestCaseFloat(powf(4.0f, 0.5f), 2.0f);
	TestCaseFloat(powf(2.0f, 0.5f), sqrtf(2.0f));
	TestCaseFloat(powf(1.1f, 2.0f), 1.2100000381469727f);
	TestCaseFloat(powf(2.0f, 1.2f), 2.2973968982696533f);
	TestCaseFloat(powf(2.0f, 1.8f), 3.4822020530700684f);
	TestCaseFloat(powf(2.0f, 1.99f), 3.972369909286499f);
	TestCaseFloat(powf(2.0f, 3.001f), 8.005546569824219f);
	TestCaseFloat(powf(2.0f, 0.0f), 1.0f);
	TestCaseFloat(powf(2.0f, INFINITY), INFINITY);
	TestCaseFloat(powf(2.0f, NAN), NAN);
	TestCaseFloat(powf(NAN, NAN), NAN);
	TestCaseFloat(powf(NAN, 2.0f), NAN);
	TestCaseFloat(powf(INFINITY, 2.0f), INFINITY);
	TestCaseFloat(powf(-INFINITY, 2.0f), INFINITY);
	TestCaseFloat(powf(-INFINITY, 3.0f), -INFINITY);
	
	TestCaseDouble(pow(2.0, 2.0), 4.0);
	TestCaseDouble(pow(2.0, 3.0), 8.0);
	TestCaseDouble(pow(4.0, 0.5), 2.0);
	TestCaseDouble(pow(2.0, 0.5), sqrt(2.0));
	TestCaseDouble(pow(1.1f, 2.0), 1.210000052452088);
	TestCaseDouble(pow(2.0, 1.2), 2.2973967099940698);
	TestCaseDouble(pow(2.0, 1.8), 3.4822022531844965);
	TestCaseDouble(pow(2.0, 1.99), 3.9723699817481437);
	TestCaseDouble(pow(2.0, 3.001), 8.005547099700644);
	TestCaseDouble(pow(2.0, 0.0), 1.0);
	TestCaseDouble(pow(2.0, INFINITY), INFINITY);
	TestCaseDouble(pow(2.0, NAN), NAN);
	TestCaseDouble(pow(NAN, NAN), NAN);
	TestCaseDouble(pow(NAN, 2.0), NAN);
	TestCaseDouble(pow(INFINITY, 2.0), INFINITY);
	TestCaseDouble(pow(-INFINITY, 2.0), INFINITY);
	TestCaseDouble(pow(-INFINITY, 3.0), -INFINITY);
	
	TestCaseFloat(logf(1.0f), 0.0f);
	TestCaseFloat(logf(2.0f), 0.6931471824645996f);
	TestCaseFloat(logf(0.0f), -INFINITY);
	TestCaseFloat(logf(-1.0f), NAN);
	TestCaseFloat(logf(1.2f), 0.18232159316539764f);
	TestCaseFloat(logf(100.0f), 4.605170249938965f);
	
	TestCaseDouble(log(1.0), 0.0);
	TestCaseDouble(log(2.0), 0.6931471805599453);
	TestCaseDouble(log(0.0), -INFINITY);
	TestCaseDouble(log(-1.0), NAN);
	TestCaseDouble(log(1.2), 0.1823215567939546);
	TestCaseDouble(log(100.0), 4.605170185988092);
	
	TestCaseFloat(log2f(1.0f), 0.0f);
	TestCaseFloat(log2f(2.0f), 1.0f);
	TestCaseFloat(log2f(0.0f), -INFINITY);
	TestCaseFloat(log2f(-1.0f), NAN);
	TestCaseFloat(log2f(1.2f),  0.263034462928772f);
	TestCaseFloat(log2f(100.0f), 6.643856048583984f);
	
	TestCaseDouble(log2(1.0), 0.0);
	TestCaseDouble(log2(2.0), 1.0);
	TestCaseDouble(log2(0.0), -INFINITY);
	TestCaseDouble(log2(-1.0), NAN);
	TestCaseDouble(log2(1.2),  0.2630344058337938);
	TestCaseDouble(log2(100.0), 6.643856189774724);
	
	TestCaseFloat(log10f(1.0f), 0.0f);
	TestCaseFloat(log10f(2.0f), 0.3010300099849701f);
	TestCaseFloat(log10f(10.0f), 1.0f);
	TestCaseFloat(log10f(0.0f), -INFINITY);
	TestCaseFloat(log10f(-1.0f), NAN);
	TestCaseFloat(log10f(1.2f), 0.0791812613606453f);
	TestCaseFloat(log10f(100.0f), 2.0f);
	TestCaseFloat(log10f(124.0f), 2.093421697616577f);
	
	TestCaseDouble(log10(1.0), 0.0);
	TestCaseDouble(log10(2.0), 0.3010299956639812);
	TestCaseDouble(log10(10.0), 1.0);
	TestCaseDouble(log10(0.0), -INFINITY);
	TestCaseDouble(log10(-1.0), NAN);
	TestCaseDouble(log10(1.2), 0.07918124604762482);
	TestCaseDouble(log10(100.0), 2.0);
	TestCaseDouble(log10(124.0), 2.093421685162235);
	
	TestCaseFloat(ldexpf(1.0f, 1), 2.0f); //ldexp is equivalent to scalbn
	TestCaseFloat(ldexpf(1.0f, 4), 16.0f);
	TestCaseFloat(ldexpf(4.0f, 1), 8.0f);
	TestCaseFloat(ldexpf(3.14f, 0), 3.14f);
	TestCaseFloat(ldexpf(1.0f, -1), 0.5f);
	TestCaseFloat(ldexpf(1.0f, -2), 0.25f);
	TestCaseFloat(ldexpf(INFINITY, 2), INFINITY);
	TestCaseFloat(ldexpf(NAN, 2), NAN);
	
	TestCaseDouble(ldexp(1.0, 1), 2.0);
	TestCaseDouble(ldexp(1.0, 4), 16.0);
	TestCaseDouble(ldexp(4.0, 1), 8.0);
	TestCaseDouble(ldexp(3.14, 0), 3.14);
	TestCaseDouble(ldexp(1.0, -1), 0.5);
	TestCaseDouble(ldexp(1.0, -2), 0.25);
	TestCaseDouble(ldexp(INFINITY, 2), INFINITY);
	TestCaseDouble(ldexp(NAN, 2), NAN);
	
	TestCaseFloat(expf(1.0f), M_E);
	TestCaseFloat(expf(2.0f), 7.389056205749512f);
	TestCaseFloat(expf(1.2f), 3.3201169967651367f);
	TestCaseFloat(expf(0.0f), 1.0f);
	TestCaseFloat(expf(-1.0f), 0.3678794503211975f);
	TestCaseFloat(expf(10.0f), 22026.46484375f);
	TestCaseFloat(expf(-10.0f), 0.00004539993096841499f);
	
	TestCaseDouble(exp(1.0), M_E);
	TestCaseDouble(exp(2.0), 7.38905609893065);
	TestCaseDouble(exp(1.2), 3.3201169227365472);
	TestCaseDouble(exp(0.0), 1.0);
	TestCaseDouble(exp(-1.0), 0.36787944117144233);
	TestCaseDouble(exp(10.0), 22026.465794806718);
	TestCaseDouble(exp(-10.0), 0.000045399929762484854);
	
	TestCaseFloat(copysignf(112.0f, -1.0f), -112.0f);
	TestCaseFloat(copysignf(112.0f, -INFINITY), -112.0f);
	TestCaseFloat(copysignf(0.0f, -1.0f), -0.0f);
	TestCaseFloat(copysignf(INFINITY, -1.0f), -INFINITY);
	TestCaseFloat(copysignf(-INFINITY, 0.0f), INFINITY);
	TestCaseFloat(copysignf(NAN, -1.0f), NAN);
	TestCaseFloat(copysignf(112.0f, NAN), 112.0f);
	
	TestCaseDouble(copysign(112.0, -1.0), -112.0);
	TestCaseDouble(copysign(112.0, -INFINITY), -112.0);
	TestCaseDouble(copysign(0.0, -1.0), -0.0);
	TestCaseDouble(copysign(INFINITY, -1.0), -INFINITY);
	TestCaseDouble(copysign(-INFINITY, 0.0), INFINITY);
	TestCaseDouble(copysign(NAN, -1.0), NAN);
	TestCaseDouble(copysign(112.0, NAN), 112.0);
	
	//TODO: Re-enable once atof is implemented!
	// TestCaseFloat(atof("1.0"), 1.0f);
	// TestCaseFloat(atof("-1"), -1.0f);
	// TestCaseFloat(atof("*5"), 0.0f); //invalid input
	// TestCaseFloat(atof("5*"), 0.0f); //invalid input
	// TestCaseFloat(atof("3.1415926"), 3.1415926f);
	// TestCaseFloat(atof("3.14159265358"), 3.1415927410125732f);
	// TestCaseFloat(atof("-3.14159265358"), -3.1415927410125732f);
	
	if (numCasesSucceeded == numCases)
	{
		MyPrint("All Math Tests Succeeded: %d", numCases);
	}
	else
	{
		MyPrint("Some Math Tests Failed: %d", numCases);
		MyPrint("Successes: %d", numCasesSucceeded);
		MyPrint("Failures: %d", numCases - numCasesSucceeded);
	}
}

void RunStringTestCases()
{
	int numCases = 0;
	int numCasesSucceeded = 0;
	uint8_t testArray[128];
	uint8_t testArray2[128];
	
	memset(&testArray[0], 'A', sizeof(testArray));
	memset(&testArray[10], 'b', sizeof(testArray) - 10);
	testArray[126] = 0;
	testArray[127] = 0;
	memmove(&testArray[0], &testArray[8], 4);
	// for (int bIndex = 0; bIndex < sizeof(testArray); bIndex++) { MyPrint("[%d] = %d", bIndex, testArray[bIndex]); }
	
	//TODO: Add test cases for memset
	
	//TODO: Add test cases for memcpy
	
	TestCaseInt(memcmp(&testArray[0], &testArray[0], 5), 0);
	TestCaseInt(memcmp(&testArray[0], &testArray[5], 5), 33);
	TestCaseInt(memcmp(&testArray[5], &testArray[0], 5), -33);
	
	//TODO: Add test cases for memmove
	
	strcpy((char*)&testArray2[0], (char*)&testArray[0]);
	// for (int bIndex = 0; bIndex < (int)sizeof(testArray2); bIndex++) { MyPrint("[%d] = %d", bIndex, testArray2[bIndex]); }
	TestCaseInt(testArray2[0], 'A');
	TestCaseInt(testArray2[1], 'A');
	TestCaseInt(testArray2[2], 'b');
	TestCaseInt(testArray2[3], 'b');
	TestCaseInt(testArray2[4], 'A');
	TestCaseInt(testArray2[10], 'b');
	TestCaseInt(testArray2[126], '\0');
	
	//TODO: Add test cases for strstr
	// char* substr1 = strstr((char*)&testArray[0], "AA");
	// MyPrint("substr1: %d", (int)(substr1 - (char*)&testArray[0]));
	// char* substr2 = strstr((char*)&testArray[0], "bbAA");
	// MyPrint("substr2: %d", (int)(substr2 - (char*)&testArray[0]));
	// char* substr3 = strstr((char*)&testArray[0], "AAAA");
	// MyPrint("substr3: %d", (int)(substr3 - (char*)&testArray[0]));
	// char* substr4 = strstr((char*)&testArray[0], "bbbb");
	// MyPrint("substr4: %d", (int)(substr4 - (char*)&testArray[0]));
	// char* substr5 = strstr((char*)&testArray[0], "b");
	// MyPrint("substr5: %d", (int)(substr5 - (char*)&testArray[0]));
	// char* substr6 = strstr((char*)&testArray[0], "AAbbAAAAAAbb");
	// MyPrint("substr6: %d", (int)(substr6 - (char*)&testArray[0]));
	
	TestCaseInt(strcmp((char*)&testArray[0], (char*)&testArray[0]), 0);
	TestCaseInt(strcmp((char*)&testArray[0], (char*)&testArray[1]), -33);
	TestCaseInt(strcmp((char*)&testArray[1], (char*)&testArray[0]), 33);
	
	TestCaseInt(strncmp((char*)&testArray[0], (char*)&testArray[0], 5), 0);
	TestCaseInt(strncmp((char*)&testArray[0], (char*)&testArray[5], 5), 33);
	TestCaseInt(strncmp((char*)&testArray[5], (char*)&testArray[0], 5), -33);
	
	TestCaseInt(strlen((char*)&testArray[0]), 126);
	
	if (numCasesSucceeded == numCases)
	{
		MyPrint("All String Tests Succeeded: %d", numCases);
	}
	else
	{
		MyPrint("Some String Tests Failed: %d", numCases);
		MyPrint("Successes: %d", numCasesSucceeded);
		MyPrint("Failures: %d", numCases - numCasesSucceeded);
	}
}

int CompareU32(const void* left, const void* right)
{
	uint32_t* leftU32 = (uint32_t*)left;
	uint32_t* rightU32 = (uint32_t*)right;
	if (*leftU32 == *rightU32) { return 0; }
	return ((*leftU32 > *rightU32) ? 1 : -1);
}

void RunStdLibTestCases()
{
	int numCases = 0;
	int numCasesSucceeded = 0;
	
	TestCaseInt(abs(17), 17);
	TestCaseInt(abs(-17), 17);
	TestCaseInt(abs(0), 0);
	TestCaseInt(abs(INT_MAX), INT_MAX);
	TestCaseInt(abs(INT_MIN+1), INT_MAX);
	
	// TestCaseDouble(atof(" "), 0.0);
	// TestCaseDouble(atof("0.5"), 0.5);
	// TestCaseDouble(atof("1.2"), 1.2);
	// TestCaseDouble(atof("+1.2"), 1.2);
	// TestCaseDouble(atof("-1.2"), -1.2);
	// TestCaseDouble(atof("0.0001"), 0.0001);
	// TestCaseDouble(atof("1.23456789"), 1.2345678900000001);
	// TestCaseDouble(atof("-10000.5"), -10000.5);
	// TestCaseDouble(atof("--1"), 0.0);
	// TestCaseDouble(atof("-+1"), 0.0);
	// TestCaseDouble(atof("+-1"), 0.0);
	// TestCaseDouble(atof("1-"), 0.0);
	// TestCaseDouble(atof("1.0-"), 0.0);
	// TestCaseDouble(atof("1.0+"), 0.0);
	// TestCaseDouble(atof("-0.0"), 0.0);
	// TestCaseDouble(atof("1j"), 0.0);
	// TestCaseDouble(atof("1^.2"), 0.0);
	// TestCaseDouble(atof("1..2"), 0.0);
	// TestCaseDouble(atof("    1.2"), 1.2);
	// TestCaseDouble(atof("1    .2"), 0.0);
	// TestCaseDouble(atof("1.    2"), 0.0);
	// TestCaseDouble(atof("1.2    "), 1.2);
	// TestCaseDouble(atof("3.14159265358979323846"), 3.14159265358979323846);
	// TestCaseDouble(atof("3.1e2"), 3.1e2);
	// TestCaseDouble(atof("3.1e-2"), 0.031000000923872003); //pretty close to 0.031
	// TestCaseDouble(atof("1.23e-20"), 0.000000000000000000012300003665686174); //pretty close to 0.0...0123
	
	if (numCasesSucceeded == numCases)
	{
		MyPrint("All StdLib Tests Succeeded: %d", numCases);
	}
	else
	{
		MyPrint("Some StdLib Tests Failed: %d", numCases);
		MyPrint("Successes: %d", numCasesSucceeded);
		MyPrint("Failures: %d", numCases - numCasesSucceeded);
	}
}

void RunStdioTestCases()
{
	int numCases = 0;
	int numCasesSucceeded = 0;
	
	TestCasePrint("1234", "%d", 1234);
	TestCasePrint("1234", "%i", 1234);
	TestCasePrint("1234", "%u", 1234);
	TestCasePrint("2322", "%o", 1234);
	TestCasePrint("Hello World!", "%s %s", "Hello", "World!");
	TestCasePrint("We've done 5 cases so far...", "We've done %d case%s so far...", numCases, (numCases == 1) ? "" : "s");
	TestCasePrint("Hell", "%.4s", "Hello World!");
	TestCasePrint("Hello W", "%.*s", 7, "Hello World!");
	TestCasePrint("1.234000", "%f", 1.234f);
	TestCasePrint("1.23", "%.2f", 1.234f);
	TestCasePrint("1.234", "%g", 1.234f);
	TestCasePrint("1.234000e+00", "%e", 1.234f);
	TestCasePrint("1.234000e+20", "%e", 1.234e20);
	TestCasePrint("123400000000000000000.000000", "%f", 1.234e20);
	TestCasePrint("1.234e+20", "%g", 1.234e20);
	TestCasePrint("1.234568", "%f", 1.23456789123); //TODO: %f doesn't seem to be support lots of numbers after the period? Can we format doubles reliably?
	TestCasePrint("1.23457", "%g", 1.23456789123);
	TestCasePrint("1.234567891230000", "%.15f", 1.23456789123);
	TestCasePrint("1.23456789123", "%.15g", 1.23456789123);
	TestCasePrint("1.2345678912356779", "%.150g", 1.234567891235678);
	TestCasePrint("1", "%.0f", 1.234f);
	TestCasePrint("123,456,789", "%'d", 123456789);
	TestCasePrint("ff802abc", "%x", 0xFF802ABC);
	TestCasePrint("FF802ABC", "%X", 0xFF802ABC);
	TestCasePrint("11223344FF802ABC", "%llX", 0x11223344FF802ABCULL);
	TestCasePrint("FF802ABC", "%X", 0x11223344FF802ABCULL);
	TestCasePrint("The letter 'X'", "The letter \'%c\'", 0x58);
	TestCasePrint("1,234,605,619,290,319,548", "%'llu", 0x11223344FF802ABCULL);
	TestCasePrint("-81,684,114,683,472,572", "%'lld", -0x01223344FF802ABCLL);
	
	if (numCasesSucceeded == numCases)
	{
		MyPrint("All StdLib Tests Succeeded: %d", numCases);
	}
	else
	{
		MyPrint("Some StdLib Tests Failed: %d", numCases);
		MyPrint("Successes: %d", numCasesSucceeded);
		MyPrint("Failures: %d", numCases - numCasesSucceeded);
	}
}

void RunWasmStdTests()
{
	RunMathTestCases();
	RunStringTestCases();
	RunStdLibTestCases();
	RunStdioTestCases();
}

#endif //TARGET_IS_WASM