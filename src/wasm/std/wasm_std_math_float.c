/*
File:   wasm_std_math_float.c
Author: Taylor Robbins
Date:   01\13\2025
Description: 
	** Holds implementations for various float related functions exposed by math.h
	** like __FLOAT_BITS, eval_as_float, __fpclassifyf, etc.
*/

int __fpclassifyf(float value)
{
	//Modified implementation from Musl Lib-C
	union { float value; uint32_t integer; } valueUnion = { value };
	int exponent = (valueUnion.integer >> 23) & 0xFF;
	if (exponent == 0) { return (valueUnion.integer << 1) ? FP_SUBNORMAL : FP_ZERO; }
	if (exponent == 0xFF) { return (valueUnion.integer << 9) ? FP_NAN : FP_INFINITE; }
	return FP_NORMAL;
}
int __fpclassify(double value)
{
	//Modified implementation from Musl Lib-C
	union { double value; uint64_t integer; } valueUnion = { value };
	int e = (valueUnion.integer >> 52) & 0x7FF;
	if (!e) { return (valueUnion.integer << 1) ? FP_SUBNORMAL : FP_ZERO; }
	if (e==0x7FF) { return (valueUnion.integer << 12) ? FP_NAN : FP_INFINITE; }
	return FP_NORMAL;
}

unsigned __FLOAT_BITS(float value)
{
	//Modified implementation from Musl Lib-C
	union { float value; unsigned integer; } valueUnion = { value };
	return valueUnion.integer;
}
unsigned long long __DOUBLE_BITS(double value)
{
	//Modified implementation from Musl Lib-C
	union { double value; unsigned long long integer; } valueUnion = { value };
	return valueUnion.integer;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
void fp_force_evalf(float value)
{
	//Modified implementation from Musl Lib-C
	volatile float volatileValue;
	volatileValue = value;
}
void fp_force_eval(double value)
{
	//Modified implementation from Musl Lib-C
	volatile double volatileValue;
	volatileValue = value;
}
#pragma clang diagnostic pop

float __math_invalidf(float value)
{
	//Modified implementation from Musl Lib-C
	return (value - value) / (value - value);
}
double __math_invalid(double value)
{
	//Modified implementation from Musl Lib-C
	return (value - value) / (value - value);
}

float eval_as_float(float x)
{
	//Modified implementation from Musl Lib-C
	float  y = x;
	return y;
}
double eval_as_double(double x)
{
	//Modified implementation from Musl Lib-C
	double y = x;
	return y;
}
