//  File:   math.h
//  Author: Taylor Robbins
//  Date:   01\13\2025

#ifndef _MATH_H
#define _MATH_H

#include <internal/wasm_std_common.h>

//NOTE: clang was shadowing our intrinsics implementations (like floor, ceil, scalbnf, sqrt, etc.) with it's builtin ones,
// we've decided to route through our functions using macros and functions prefixed with _.
// Even if they route to builtins at the end of the day, this gives us the ability
// to test our implementation and the control to route these however we want.

// +--------------------------------------------------------------+
// |                           Defines                            |
// +--------------------------------------------------------------+
#define NAN       __builtin_nanf("")
#define INFINITY  __builtin_inff()

#define FP_NAN       0
#define FP_INFINITE  1
#define FP_ZERO      2
#define FP_SUBNORMAL 3
#define FP_NORMAL    4

#define M_E             2.7182818284590452354   // e
#define M_LOG2E         1.4426950408889634074   // log_2 e
#define M_LOG10E        0.43429448190325182765  // log_10 e
#define M_LN2           0.69314718055994530942  // log_e 2
#define M_LN10          2.30258509299404568402  // log_e 10
#define M_PI            3.14159265358979323846  // pi
#define M_PI_2          1.57079632679489661923  // pi/2
#define M_PI_4          0.78539816339744830962  // pi/4
#define M_1_PI          0.31830988618379067154  // 1/pi
#define M_2_PI          0.63661977236758134308  // 2/pi
#define M_2_SQRTPI      1.12837916709551257390  // 2/sqrt(pi)
#define M_SQRT2         1.41421356237309504880  // sqrt(2)
#define M_SQRT1_2       0.70710678118654752440  // 1/sqrt(2)

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define isinf(value) (sizeof(value) == sizeof(float)            \
	? (__FLOAT_BITS((float)value) & 0x7fffffff) == 0x7f800000   \
	: (__DOUBLE_BITS((double)value) & -1ULL>>1) == 0x7ffULL<<52 \
)

#define isnan(value) (sizeof(value) == sizeof(float)           \
	? (__FLOAT_BITS((float)value) & 0x7fffffff) > 0x7f800000   \
	: (__DOUBLE_BITS((double)value) & -1ULL>>1) > 0x7ffULL<<52 \
)

#define signbit(value) (sizeof(value) == sizeof(float) \
	? (int)(__FLOAT_BITS((float)value)>>31)            \
	: (int)(__DOUBLE_BITS((double)value)>>63)          \
)

#define isnormal(value) (sizeof(value) == sizeof(float)             \
	? ((__FLOAT_BITS(value)+0x00800000) & 0x7fffffff) >= 0x01000000 \
	: ((__DOUBLE_BITS(value)+(1ULL<<52)) & -1ULL>>1) >= 1ULL<<53    \
)

#define fpclassify(value) (sizeof(value) == sizeof(float) \
	? __fpclassifyf(value)                                \
	: __fpclassify(value)                                 \
)

#define FORCE_EVAL(value) do {                                     \
	if (sizeof(value) == sizeof(float)) { fp_force_evalf(value); } \
	else { fp_force_eval(value); }                                 \
} while(0)

#define asuint(value)   ((union { float    _value; uint32_t _integer; }){ value })._integer
#define asfloat(value)  ((union { uint32_t _value; float    _float;   }){ value })._float
#define asuint64(value) ((union { double   _value; uint64_t _integer; }){ value })._integer
#define asdouble(value) ((union { uint64_t _value; double   _double;  }){ value })._double

#define EXTRACT_WORDS(highWord, lowWord, value) do { uint64_t __u = asuint64(value); (highWord) = (__u >> 32); (lowWord) = (uint32_t)__u; } while (0)
#define INSERT_WORDS(doubleVar, highWord, lowWord) do { (doubleVar) = asdouble(((uint64_t)(highWord)<<32) | (uint32_t)(lowWord)); } while (0)
#define GET_HIGH_WORD(wordVar, value) do { (wordVar) = asuint64(value) >> 32; } while (0)
#define GET_LOW_WORD(wordVar, value)  do { (wordVar) = (uint32_t)asuint64(value); } while (0)
#define SET_HIGH_WORD(doubleVar, wordValue) INSERT_WORDS(doubleVar, wordValue, (uint32_t)asuint64(doubleVar))
#define SET_LOW_WORD(doubleVar, wordValue)  INSERT_WORDS(doubleVar, asuint64(doubleVar)>>32, wordValue)
#define GET_FLOAT_WORD(wordVar, value)  do { (wordVar)  = asuint(value);  } while (0)
#define SET_FLOAT_WORD(floatVar, value) do { (floatVar) = asfloat(value); } while (0)

// Helps static branch prediction so hot path can be better optimized.
#define predict_true(condition) __builtin_expect(!!(condition), 1)
#define predict_false(condition) __builtin_expect(condition, 0)

CONDITIONAL_EXTERN_C_START

// +--------------------------------------------------------------+
// |                       Helper Functions                       |
// +--------------------------------------------------------------+
int __fpclassifyf(float value);
int __fpclassify(double value);

unsigned __FLOAT_BITS(float value);
unsigned long long __DOUBLE_BITS(double value);

void fp_force_evalf(float value);
void fp_force_eval(double value);

float __math_invalidf(float value);
double __math_invalid(double value);

float eval_as_float(float x);
double eval_as_double(double x);

float __math_divzerof(uint32_t sign);
double __math_divzero(uint32_t sign);

// Top 16 bits of a double.
uint32_t top16(double x);
// Top 12 bits of a double (sign and exponent bits).
uint32_t top12(double value);
uint32_t top12f(float x);

float fp_barrierf(float x);
double fp_barrier(double x);

// +--------------------------------------------------------------+
// |                          Functions                           |
// +--------------------------------------------------------------+
float  fminf(float value1, float value2);
double fmin(double value1, double value2);

float  fmaxf(float value1, float value2);
double fmax(double value1, double value2);

float  fabsf(float value);
double fabs(double value);
// TODO: long double fabsl(long double value);

float  fmodf(float numer, float denom);
double fmod(double numer, double denom);
// TODO: long double fmodl(long double numer, long double denom);

float  roundf(float value);
double round(double value);

float  _floorf(float value);
double _floor(double value);
#define floorf(value) _floorf(value)
#define floor(value)  _floor(value)

float  _ceilf(float value);
double _ceil(double value);
#define ceilf(value) _ceilf(value)
#define ceil(value)  _ceil(value)

float  _scalbnf(float value, int power);
double _scalbn(double value, int power);
// TODO: long double _scalbnl(long double value, int power);
#define scalbnf(value, power) _scalbnf(value, power)
#define scalbn(value, power)  _scalbn(value, power)

float  sqrtf(float value);
double sqrt(double value);

float  _cbrtf(float value);
double _cbrt(double value);
#define cbrtf(value) _cbrtf(value)
#define cbrt(value)  _cbrt(value)

float  sinf(float value);
double sin(double value);

float  cosf(float value);
double cos(double value);

float  tanf(float value);
double tan(double value);

float  asinf(float value);
double asin(double value);

float  acosf(float value);
double acos(double value);

float  atanf(float value);
double atan(double value);

float  atan2f(float numer, float denom);
double atan2(double numer, double denom);

float  powf(float value, float exponent);
double pow(double value, double exponent);

float  logf(float value);
double log(double value);

float  log2f(float value);
double log2(double value);

float  log10f(float value);
double log10(double x);

float  ldexpf(float value, int exponent);
double ldexp(double value, int exponent);

float  expf(float value);
double exp(double value);

float  copysignf(float magnitude, float sign);
double copysign(double magnitude, double sign);
// TODO: long double copysignl(long double magnitude, long double sign);

CONDITIONAL_EXTERN_C_END

#endif //  _MATH_H
