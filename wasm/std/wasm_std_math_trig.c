/*
File:   wasm_std_math_trig.c
Author: Taylor Robbins
Date:   01\13\2025
Description: 
	** Holds implementation for trigonometric functions like cos(), sin(), tan(), atan(), etc.
*/

// Small multiples of pi/2 rounded to double precision.
static const double
	pio2_1x = 1*M_PI_2, // 0x3FF921FB, 0x54442D18
	pio2_2x = 2*M_PI_2, // 0x400921FB, 0x54442D18
	pio2_3x = 3*M_PI_2, // 0x4012D97C, 0x7F3321D2
	pio2_4x = 4*M_PI_2; // 0x401921FB, 0x54442D18

float sinf(float value)
{
	//Modified implementation from Musl Lib-C
	double result;
	uint32_t invValue;
	int temp, sign;
	
	GET_FLOAT_WORD(invValue, value);
	sign = (invValue >> 31);
	invValue &= 0x7FFFFFFF;
	
	if (invValue <= 0x3F490FDA) // |value| ~<= pi/4
	{
		if (invValue < 0x39800000) // |value| < 2**-12
		{
			// raise inexact if value!=0 and underflow if subnormal
			FORCE_EVAL((invValue < 0x00800000) ? value / 0x1p120F : value + 0x1p120F);
			return value;
		}
		return __sindf(value);
	}
	if (invValue <= 0x407B53D1) // |value| ~<= 5*pi/4
	{
		if (invValue <= 0x4016CBE3) // |value| ~<= 3pi/4
		{
			if (sign) { return -__cosdf(value + pio2_1x); }
			else { return __cosdf(value - pio2_1x); }
		}
		return __sindf(sign ? -(value + pio2_2x) : -(value - pio2_2x));
	}
	if (invValue <= 0x40E231D5) // |value| ~<= 9*pi/4
	{
		if (invValue <= 0x40AFEDDF) // |value| ~<= 7*pi/4
		{
			if (sign) { return __cosdf(value + pio2_3x); }
			else { return -__cosdf(value - pio2_3x); }
		}
		return __sindf(sign ? value + pio2_4x : value - pio2_4x);
	}
	
	// sin(Inf or NaN) is NaN
	if (invValue >= 0x7F800000) { return value - value; }
	
	// general argument reduction needed
	temp = __rem_pio2f(value, &result);
	switch (temp & 3)
	{
		case 0:  return  __sindf(result);
		case 1:  return  __cosdf(result);
		case 2:  return  __sindf(-result);
		default: return -__cosdf(result);
	}
}
double sin(double value)
{
	//Modified implementation from Musl Lib-C
	double result[2];
	uint32_t highWord;
	unsigned n; //TODO: give this a better name
	
	// High word of value.
	GET_HIGH_WORD(highWord, value);
	highWord &= 0x7FFFFFFF;
	
	// |value| ~< pi/4
	if (highWord <= 0x3FE921FB)
	{
		if (highWord < 0x3e500000) // |value| < 2**-26
		{
			// raise inexact if value != 0 and underflow if subnorma
			FORCE_EVAL((highWord < 0x00100000) ? value / 0x1p120f : value + 0x1p120f);
			return value;
		}
		return __sin(value, 0.0, 0);
	}
	
	// sin(Inf or NaN) is NaN
	if (highWord >= 0x7ff00000) { return value - value; }
	
	// argument reduction needed
	n = __rem_pio2(value, result);
	switch (n&3)
	{
		case 0:  return  __sin(result[0], result[1], 1);
		case 1:  return  __cos(result[0], result[1]);
		case 2:  return -__sin(result[0], result[1], 1);
		default: return -__cos(result[0], result[1]);
	}
}

float cosf(float value)
{
	//Modified implementation from Musl Lib-C
	double result;
	uint32_t valueWord;
	unsigned n, sign; //TODO: give this a better name

	GET_FLOAT_WORD(valueWord, value);
	sign = valueWord >> 31;
	valueWord &= 0x7FFFFFFF;

	if (valueWord <= 0x3F490FDA) // |value| ~<= pi/4
	{
		if (valueWord < 0x39800000) // |value| < 2**-12
		{
			// raise inexact if value != 0
			FORCE_EVAL(value + 0x1p120F);
			return 1.0f;
		}
		return __cosdf(value);
	}
	if (valueWord <= 0x407B53D1) // |value| ~<= 5*pi/4
	{
		if (valueWord > 0x4016CBE3) // |value|  ~> 3*pi/4
		{
			return -__cosdf(sign ? value+pio2_2x : value-pio2_2x);
		}
		else
		{
			if (sign) { return __sindf(value + pio2_1x); }
			else { return __sindf(pio2_1x - value); }
		}
	}
	if (valueWord <= 0x40E231D5) // |value| ~<= 9*pi/4
	{
		if (valueWord > 0x40AFEDDF) // |value| ~> 7*pi/4
		{
			return __cosdf(sign ? value+pio2_4x : value-pio2_4x);
		}
		else
		{
			if (sign) { return __sindf(-value - pio2_3x); }
			else { return __sindf(value - pio2_3x); }
		}
	}

	// cos(Inf or NaN) is NaN
	if (valueWord >= 0x7F800000) { return value-value; }

	// general argument reduction needed
	n = __rem_pio2f(value, &result);
	switch (n & 3)
	{
		case 0:  return  __cosdf(result);
		case 1:  return  __sindf(-result);
		case 2:  return -__cosdf(result);
		default: return  __sindf(result);
	}
}
double cos(double value)
{
	//Modified implementation from Musl Lib-C
	double result[2];
	uint32_t highWord;
	unsigned n; //TODO: give this a better name

	GET_HIGH_WORD(highWord, value);
	highWord &= 0x7FFFFFFF;

	// |value| ~< pi/4
	if (highWord <= 0x3FE921FB)
	{
		if (highWord < 0x3E46A09E) // |value| < 2**-27 * sqrt(2)
		{
			// raise inexact if value!=0
			FORCE_EVAL(value + 0x1p120F);
			return 1.0;
		}
		return __cos(value, 0);
	}

	// cos(Inf or NaN) is NaN
	if (highWord >= 0x7FF00000) { return (value - value); }

	// argument reduction
	n = __rem_pio2(value, result);
	switch (n & 3)
	{
		case 0: return  __cos(result[0], result[1]);
		case 1: return -__sin(result[0], result[1], 1);
		case 2: return -__cos(result[0], result[1]);
		default: return  __sin(result[0], result[1], 1);
	}
}

float tanf(float value)
{
	//Modified implementation from Musl Lib-C
	double result;
	uint32_t valueWord;
	unsigned n, sign; //TODO: give this a better name
	
	GET_FLOAT_WORD(valueWord, value);
	sign = (valueWord >> 31);
	valueWord &= 0x7FFFFFFF;
	
	if (valueWord <= 0x3F490FDA) // |value| ~<= pi/4
	{
		if (valueWord < 0x39800000) // |value| < 2**-12
		{
			// raise inexact if value!=0 and underflow if subnormal
			FORCE_EVAL((valueWord < 0x00800000) ? value / 0x1p120F : value + 0x1p120F);
			return value;
		}
		return __tandf(value, 0);
	}
	if (valueWord <= 0x407B53D1) // |value| ~<= 5*pi/4
	{
		if (valueWord <= 0x4016CBE3) // |value| ~<= 3pi/4
		{
			return __tandf((sign ? value + pio2_1x : value - pio2_1x), 1);
		}
		else
		{
			return __tandf((sign ? value + pio2_2x : value - pio2_2x), 0);
		}
	}
	if (valueWord <= 0x40E231D5) // |value| ~<= 9*pi/4
	{
		if (valueWord <= 0x40AFEDDF) // |value| ~<= 7*pi/4
		{
			return __tandf((sign ? value + pio2_3x : value - pio2_3x), 1);
		}
		else
		{
			return __tandf((sign ? value + pio2_4x : value - pio2_4x), 0);
		}
	}
	
	// tan(Inf or NaN) is NaN
	if (valueWord >= 0x7F800000) { return (value - value); }
	
	// argument reduction
	n = __rem_pio2f(value, &result);
	return __tandf(result, (n & 1));
}
double tan(double value)
{
	//Modified implementation from Musl Lib-C
	double result[2];
	uint32_t highWord;
	unsigned n; //TODO: give this a better name

	GET_HIGH_WORD(highWord, value);
	highWord &= 0x7FFFFFFF;

	// |value| ~< pi/4
	if (highWord <= 0x3FE921FB)
	{
		if (highWord < 0x3E400000) // |value| < 2**-27
		{
			// raise inexact if value!=0 and underflow if subnormal
			FORCE_EVAL((highWord < 0x00100000) ? value / 0x1p120F : value + 0x1p120F);
			return value;
		}
		return __tan(value, 0.0, 0);
	}

	// tan(Inf or NaN) is NaN
	if (highWord >= 0x7FF00000) { return (value - value); }

	// argument reduction
	n = __rem_pio2(value, result);
	return __tan(result[0], result[1], (n & 1));
}

float asinf(float value)
{
	//Modified implementation from Musl Lib-C
	double sqrtZ;
	float zVar;
	uint32_t wordValue, unsignedValue;
	
	GET_FLOAT_WORD(wordValue, value);
	unsignedValue = wordValue & 0x7FFFFFFF;
	if (unsignedValue >= 0x3F800000) // |value| >= 1
	{
		if (unsignedValue == 0x3F800000) // |value| == 1
		{
			return (value * pio2) + 0x1p-120f;  // asin(+-1) = +-pi/2 with inexact
		}
		return 0 / (value - value);  // asin(|value|>1) is NaN
	}
	if (unsignedValue < 0x3f000000) // |value| < 0.5
	{
		// if 0x1p-126 <= |value| < 0x1p-12, avoid raising underflow
		if (unsignedValue < 0x39800000 && unsignedValue >= 0x00800000) { return value; }
		return value + value * asinf_helper(value * value);
	}
	// 1 > |value| >= 0.5
	zVar = (1 - fabsf(value)) * 0.5f;
	sqrtZ = sqrt(zVar);
	value = pio2 - 2 * (sqrtZ + (sqrtZ * asinf_helper(zVar)));
	if (wordValue >> 31) { return -value; }
	return value;
}
double asin(double value)
{
	//Modified implementation from Musl Lib-C
	double zVar, rVar, sVar;
	uint32_t highWord, valueUnsigned;
	
	GET_HIGH_WORD(highWord, value);
	valueUnsigned = (highWord & 0x7FFFFFFF);
	// |value| >= 1 or nan
	if (valueUnsigned >= 0x3FF00000)
	{
		uint32_t lowWord;
		GET_LOW_WORD(lowWord, value);
		if (((valueUnsigned - 0x3FF00000) | lowWord) == 0)
		{
			// asin(1) = +-pi/2 with inexact
			return (value * pio2d_hi) + 0x1p-120F;
		}
		return 0 / (value - value);
	}
	// |value| < 0.5
	if (valueUnsigned < 0x3FE00000)
	{
		// if 0x1p-1022 <= |value| < 0x1p-26, avoid raising underflow
		if (valueUnsigned < 0x3E500000 && valueUnsigned >= 0x00100000) { return value; }
		return value + (value * asin_helper(value * value));
	}
	// 1 > |value| >= 0.5
	zVar = (1 - fabs(value)) * 0.5;
	sVar = sqrt(zVar);
	rVar = asin_helper(zVar);
	if (valueUnsigned >= 0x3FEF3333) // if |value| > 0.975
	{
		value = pio2d_hi - (2 * (sVar + (sVar * rVar)) - pio2d_lo);
	}
	else
	{
		double fVar, cVar;
		// fVar+cVar = sqrt(zVar)
		fVar = sVar;
		SET_LOW_WORD(fVar, 0);
		cVar = (zVar - (fVar * fVar)) / (sVar + fVar);
		value = (0.5 * pio2d_hi) - ((2 * sVar * rVar) - (pio2d_lo - (2 * cVar)) - ((0.5 * pio2d_hi) - (2 * fVar)));
	}
	if (highWord >> 31) { return -value; }
	return value;
}

float acosf(float value)
{
	//Modified implementation from Musl Lib-C
	float zVar, wVar, sVar, cVar, df;
	uint32_t valueWord, ix;
	
	GET_FLOAT_WORD(valueWord, value);
	ix = valueWord & 0x7FFFFFFF;
	// |value| >= 1 or nan
	if (ix >= 0x3F800000)
	{
		if (ix == 0x3F800000)
		{
			if (valueWord >> 31) { return (2 * pio2_hi) + 0x1p-120F; }
			return 0;
		}
		return 0 / (value - value);
	}
	// |value| < 0.5
	if (ix < 0x3F000000)
	{
		if (ix <= 0x32800000) // |value| < 2**-26
		{
			return pio2_hi + 0x1p-120F;
		}
		return pio2_hi - (value - (pio2_lo - value * acosf_helper(value * value)));
	}
	// value < -0.5
	if (valueWord >> 31)
	{
		zVar = (1 + value) * 0.5f;
		sVar = sqrtf(zVar);
		wVar = (acosf_helper(zVar) * sVar) - pio2_lo;
		return 2 * (pio2_hi - (sVar + wVar));
	}
	// value > 0.5
	zVar = (1 - value) * 0.5f;
	sVar = sqrtf(zVar);
	GET_FLOAT_WORD(valueWord, sVar);
	SET_FLOAT_WORD(df, (valueWord & 0xFFFFF000));
	cVar = (zVar - (df * df)) / (sVar + df);
	wVar = (acosf_helper(zVar) * sVar) + cVar;
	return 2 * (df + wVar);
}
double acos(double value)
{
	//Modified implementation from Musl Lib-C
	double zVar, wVar, sVar, cVar, df;
	uint32_t highWord, valueUnsigned;
	
	GET_HIGH_WORD(highWord, value);
	valueUnsigned = (highWord & 0x7FFFFFFF);
	// |value| >= 1 or nan
	if (valueUnsigned >= 0x3FF00000)
	{
		uint32_t lowWord;

		GET_LOW_WORD(lowWord, value);
		if (((valueUnsigned - 0x3FF00000) | lowWord) == 0)
		{
			// acos(1)=0, acos(-1)=pi
			if (highWord >> 31) { return (2 * pio2d_hi) + 0x1p-120f; }
			return 0;
		}
		return 0 / (value - value);
	}
	// |value| < 0.5
	if (valueUnsigned < 0x3FE00000)
	{
		if (valueUnsigned <= 0x3C600000)  // |value| < 2**-57
		{
			return pio2d_hi + 0x1p-120f;
		}
		return pio2d_hi - (value - (pio2d_lo - (value * acos_helper(value * value))));
	}
	// value < -0.5
	if (highWord >> 31)
	{
		zVar = (1.0 + value) * 0.5;
		sVar = sqrt(zVar);
		wVar = (acos_helper(zVar) * sVar) - pio2d_lo;
		return 2 * (pio2d_hi - (sVar + wVar));
	}
	// value > 0.5
	zVar = (1.0 - value) * 0.5;
	sVar = sqrt(zVar);
	df = sVar;
	SET_LOW_WORD(df, 0);
	cVar = (zVar - (df * df)) / (sVar + df);
	wVar = (acos_helper(zVar) * sVar) + cVar;
	return 2 * (df + wVar);
}

float atanf(float value)
{
	//Modified implementation from Musl Lib-C
	float_t wVar, sVar1, sVar2, zVar;
	uint32_t valueUnsigned, sign;
	int index;
	
	GET_FLOAT_WORD(valueUnsigned, value);
	sign = valueUnsigned>>31;
	valueUnsigned &= 0x7FFFFFFF;
	if (valueUnsigned >= 0x4C800000) // if |value| >= 2**26
	{
		if (isnan(value)) { return value; }
		zVar = atanhi[3] + 0x1p-120f;
		return (sign ? -zVar : zVar);
	}
	if (valueUnsigned < 0x3EE00000) // |value| < 0.4375
	{
		if (valueUnsigned < 0x39800000) // |value| < 2**-12
		{
			if (valueUnsigned < 0x00800000)
			{
				// raise underflow for subnormal value
				FORCE_EVAL(value*value);
			}
			return value;
		}
		index = -1;
	}
	else
	{
		value = fabsf(value);
		if (valueUnsigned < 0x3f980000) // |value| < 1.1875
		{
			if (valueUnsigned < 0x3f300000) //  7/16 <= |value| < 11/16
			{
				index = 0;
				value = (2.0f*value - 1.0f)/(2.0f + value);
			}
			else // 11/16 <= |value| < 19/16
			{
				index = 1;
				value = (value - 1.0f)/(value + 1.0f);
			}
		}
		else
		{
			if (valueUnsigned < 0x401c0000) // |value| < 2.4375
			{
				index = 2;
				value = (value - 1.5f)/(1.0f + 1.5f*value);
			}
			else // 2.4375 <= |value| < 2**26
			{
				index = 3;
				value = -1.0f/value;
			}
		}
	}
	// end of argument reduction
	zVar = value * value;
	wVar = zVar * zVar;
	// break sum from i=0 to 10 aT[i]zVar**(i+1) into odd and even poly
	sVar1 = zVar * (aT[0] + wVar * (aT[2] + wVar * aT[4]));
	sVar2 = wVar * (aT[1] + wVar * aT[3]);
	if (index < 0) { return value - value * (sVar1 + sVar2); }
	zVar = atanhi[index] - ((value * (sVar1 + sVar2) - atanlo[index]) - value);
	return (sign ? -zVar : zVar);
}
double atan(double value)
{
	//Modified implementation from Musl Lib-C
	double_t quad, sVar1, sVar2, square;
	uint32_t valueUnsigned, sign;
	int index;
	
	GET_HIGH_WORD(valueUnsigned, value);
	sign = (valueUnsigned >> 31);
	valueUnsigned &= 0x7FFFFFFF;
	if (valueUnsigned >= 0x44100000) // if |value| >= 2^66
	{
		if (isnan(value)) { return value; }
		square = atanhid[3] + 0x1p-120F;
		return (sign ? -square : square);
	}
	if (valueUnsigned < 0x3FDC0000) // |value| < 0.4375
	{
		if (valueUnsigned < 0x3E400000) // |value| < 2^-27
		{
			if (valueUnsigned < 0x00100000)
			{
				// raise underflow for subnormal value
				FORCE_EVAL((float)value);
			}
			return value;
		}
		index = -1;
	}
	else
	{
		value = fabs(value);
		if (valueUnsigned < 0x3FF30000) // |value| < 1.1875
		{
			if (valueUnsigned < 0x3FE60000) //  7/16 <= |value| < 11/16
			{
				index = 0;
				value = ((2.0 * value) - 1.0) / (2.0 + value);
			}
			else // 11/16 <= |value| < 19/16
			{
				index = 1;
				value = (value - 1.0) / (value + 1.0);
			}
		}
		else
		{
			if (valueUnsigned < 0x40038000) // |value| < 2.4375
			{
				index = 2;
				value = (value - 1.5) / (1.0 + (1.5 * value));
			}
			else // 2.4375 <= |value| < 2^66
			{
				index = 3;
				value = -1.0 / value;
			}
		}
	}
	// end of argument reduction
	square = (value * value);
	quad = (square * square);
	// break sum from i=0 to 10 aTd[i]square**(i+1) into odd and even poly
	sVar1 = square * (aTd[0] + quad * (aTd[2] + quad * (aTd[4] + quad * (aTd[6] + quad * (aTd[8] + quad * aTd[10])))));
	sVar2 = quad * (aTd[1] + quad * (aTd[3] + quad * (aTd[5] + quad * (aTd[7] + quad * aTd[9]))));
	if (index < 0) { return value - value * (sVar1 + sVar2); }
	square = atanhid[index] - (value * (sVar1 + sVar2) - atanlod[index] - value);
	return (sign ? -square : square);
}

float atan2f(float numer, float denom)
{
	//Modified implementation from Musl Lib-C
	float zVar;
	uint32_t index, denomWord, numerWord;

	if (isnan(denom) || isnan(numer)) { return (denom + numer); }
	GET_FLOAT_WORD(denomWord, denom);
	GET_FLOAT_WORD(numerWord, numer);
	if (denomWord == 0x3F800000) { return atanf(numer); } // denom=1.0
	index = ((numerWord >> 31) & 1) | ((denomWord >> 30) & 2);  // 2*sign(denom)+sign(numer)
	denomWord &= 0x7FFFFFFF;
	numerWord &= 0x7FFFFFFF;

	// when numer = 0
	if (numerWord == 0)
	{
		switch (index)
		{
			case 0:
			case 1: return numer; // atan(+-0,+anything)=+-0
			case 2: return  pi;   // atan(+0,-anything) = pi
			case 3: return -pi;   // atan(-0,-anything) =-pi
		}
	}
	// when denom = 0
	if (denomWord == 0) { return (index & 1) ? -pi/2 : pi/2; }
	// when denom is INF
	if (denomWord == 0x7F800000)
	{
		if (numerWord == 0x7F800000)
		{
			switch (index)
			{
				case 0: return  pi/4;   // atan(+INF,+INF)
				case 1: return -pi/4;   // atan(-INF,+INF)
				case 2: return 3*pi/4;  // atan(+INF,-INF)
				case 3: return -3*pi/4; // atan(-INF,-INF)
			}
		}
		else
		{
			switch (index)
			{
				case 0: return  0.0f;    // atan(+...,+INF)
				case 1: return -0.0f;    // atan(-...,+INF)
				case 2: return  pi; // atan(+...,-INF)
				case 3: return -pi; // atan(-...,-INF)
			}
		}
	}
	// |numer/denom| > 0x1p26
	if (denomWord + (26 << 23) < numerWord || numerWord == 0x7F800000)
	{
		return (index &1) ? -pi/2 : pi/2;
	}

	// zVar = atan(|numer/denom|) with correct underflow
	if ((index & 2) && numerWord + (26 << 23) < denomWord)  //|numer/denom| < 0x1p-26, denom < 0
	{
		zVar = 0.0;
	}
	else
	{
		zVar = atanf(fabsf(numer / denom));
	}
	switch (index)
	{
		case 0:  return zVar;              // atan(+,+)
		case 1:  return -zVar;             // atan(-,+)
		case 2:  return pi - (zVar-pi_lo); // atan(+,-)
		default: return (zVar-pi_lo) - pi; // atan(-,-)
	}
}
double atan2(double numer, double denom)
{
	//Modified implementation from Musl Lib-C
	double zVar;
	uint32_t index, denomLow, numerLow, denomHigh, numerHigh;
	
	if (isnan(denom) || isnan(numer)) { return denom+numer; }
	EXTRACT_WORDS(denomHigh, denomLow, denom);
	EXTRACT_WORDS(numerHigh, numerLow, numer);
	if ((denomHigh-0x3FF00000 | denomLow) == 0) { return atan(numer); } // denom = 1.0
	index = ((numerHigh >> 31) & 1) | ((denomHigh >> 30) & 2); // 2*sign(denom)+sign(numer)
	denomHigh = (denomHigh & 0x7FFFFFFF);
	numerHigh = (numerHigh & 0x7FFFFFFF);
	
	// when numer = 0
	if ((numerHigh|numerLow) == 0)
	{
		switch(index)
		{
			case 0:
			case 1: return numer;   // atan(+-0,+anything)=+-0
			case 2: return  pi; // atan(+0,-anything) = pi
			case 3: return -pi; // atan(-0,-anything) =-pi
		}
	}
	// when denom = 0
	if ((denomHigh|denomLow) == 0) { return (index & 1) ? -pi/2 : pi/2; }
	// when denom is INF
	if (denomHigh == 0x7FF00000)
	{
		if (numerHigh == 0x7FF00000)
		{
			switch(index)
			{
				case 0: return  pi/4;   // atan(+INF,+INF)
				case 1: return -pi/4;   // atan(-INF,+INF)
				case 2: return  3*pi/4; // atan(+INF,-INF)
				case 3: return -3*pi/4; // atan(-INF,-INF)
			}
		}
		else
		{
			switch(index)
			{
				case 0: return  0.0; // atan(+...,+INF)
				case 1: return -0.0; // atan(-...,+INF)
				case 2: return  pi;  // atan(+...,-INF)
				case 3: return -pi;  // atan(-...,-INF)
			}
		}
	}
	// |numer/denom| > 0x1p64
	if (denomHigh+(64<<20) < numerHigh || numerHigh == 0x7FF00000) { return (index & 1) ? -pi/2 : pi/2; }
	
	// zVar = atan(|numer/denom|) without spurious underflow
	if ((index & 2) && numerHigh + (64 << 20) < denomHigh)  // |numer/denom| < 0x1p-64, denom<0
	{
		zVar = 0;
	}
	else
	{
		zVar = atan(fabs(numer / denom));
	}
	switch (index)
	{
		case 0:  return zVar;                // atan(+,+)
		case 1:  return -zVar;               // atan(-,+)
		case 2:  return pi - (zVar - pi_lo); // atan(+,-)
		default: return (zVar - pi_lo) - pi; // atan(-,-)
	}
}