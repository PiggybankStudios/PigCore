/*
File:   wasm_std_math_basic.c
Author: Taylor Robbins
Date:   01\13\2025
Description: 
	** Holds implementations for "basic" math functions exposed in math.h
	** like round(), fmod(), floor(), etc.
	** Many of these support using the __builtin version so we opt for that wherever possible
*/

float copysignf(float  magnitude, float  sign) { return __builtin_copysignf(magnitude, sign); }
double copysign(double magnitude, double sign) { return __builtin_copysign(magnitude,  sign); }

float fminf(float value1, float value2)   { return __builtin_fminf(value1, value2); }
double fmin(double value1, double value2) { return __builtin_fmin(value1,  value2); }
float fmaxf(float value1, float value2)   { return __builtin_fmaxf(value1, value2); }
double fmax(double value1, double value2) { return __builtin_fmax(value1,  value2); }

float fabsf(float value) { return __builtin_fabsf(value); }
double fabs(double value) { return __builtin_fabs(value); }

float fmodf(float numer, float denom) { return __builtin_fmodf(numer, denom); }
double fmod(double numer, double denom) { return __builtin_fmod(numer, denom); }

float _floorf(float value) { return __builtin_floorf(value); }
double _floor(double value) { return __builtin_floor(value); }
float _ceilf(float value) { return __builtin_ceilf(value); }
double _ceil(double value) { return __builtin_ceil(value); }

float roundf(float value)
{
	//Modified implementation from Musl Lib-C
	union { float value; uint32_t integer; } valueUnion = { value };
	int valueExponent = ((valueUnion.integer >> 23) & 0xFF);
	float_t result;
	
	if (valueExponent >= 0x7F+23) { return value; }
	if (valueUnion.integer >> 31) { value = -value; }
	if (valueExponent < 0x7F-1)
	{
		//raise inexact if value!=0 
		FORCE_EVAL(value + tointf);
		return 0*valueUnion.value;
	}
	result = value + tointf - tointf - value;
	if (result > 0.5f) { result = result + value - 1; }
	else if (result <= -0.5f) { result = result + value + 1; }
	else { result = result + value; }
	if (valueUnion.integer >> 31) { result = -result; }
	return result;
}
double round(double value)
{
	//Modified implementation from Musl Lib-C
	union { double value; uint64_t integer; } valueUnion = { value };
	int valueExponent = ((valueUnion.integer >> 52) & 0x7FF);
	double_t result;
	
	if (valueExponent >= 0x3FF+52) { return value; }
	if (valueUnion.integer >> 63) { value = -value; }
	if (valueExponent < 0x3FF-1)
	{
		//raise inexact if value!=0 
		FORCE_EVAL(value + tointd);
		return 0 * valueUnion.value;
	}
	result = value + tointd - tointd - value;
	if (result > 0.5) { result = result + value - 1; }
	else if (result <= -0.5) { result = result + value + 1; }
	else { result = result + value; }
	if (valueUnion.integer >> 63) { result = -result; }
	return result;
}