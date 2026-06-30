/*
File:   wasm_std_math_pow.c
Author: Taylor Robbins
Date:   01\13\2025
Description: 
	** Holds implementation for power related functions like pow(), log(), exp(), ldexp(), etc.
*/

float powf(float base, float exponent)
{
	//Modified implementation from Musl Lib-C
	uint32_t signBias = 0;
	uint32_t baseInt, exponentInt;
	
	baseInt = asuint(base);
	exponentInt = asuint(exponent);
	if (predict_false(baseInt - 0x00800000 >= 0x7F800000 - 0x00800000 || zeroinfnan32(exponentInt)))
	{
		// Either (base < 0x1p-126 or inf or nan) or (exponent is 0 or inf or nan).
		if (predict_false(zeroinfnan32(exponentInt)))
		{
			if (2 * exponentInt == 0) { return 1.0f; }
			if (baseInt == 0x3F800000) { return 1.0f; }
			if (2 * baseInt > 2u * 0x7F800000 || 2 * exponentInt > 2u * 0x7F800000) { return base + exponent; }
			if (2 * baseInt == 2 * 0x3F800000) { return 1.0f; }
			if ((2 * baseInt < 2 * 0x3F800000) == !(exponentInt & 0x80000000)) { return 0.0f; } // |base|<1 && exponent==inf or |base|>1 && exponent==-inf.
			return exponent * exponent;
		}
		if (predict_false(zeroinfnan32(baseInt)))
		{
			float_t baseSquared = base * base;
			if (baseInt & 0x80000000 && checkint32(exponentInt) == 1) { baseSquared = -baseSquared; }
			// Without the barrier some versions of clang hoist the 1/baseSquared and
			// thus division by zero exception can be signaled spuriously.
			return exponentInt & 0x80000000 ? fp_barrierf(1 / baseSquared) : baseSquared;
		}
		// base and exponent are non-zero finite.
		if (baseInt & 0x80000000)
		{
			// Finite base < 0.
			int exponentType = checkint32(exponentInt);
			if (exponentType == 0) { return __math_invalidf(base); }
			if (exponentType == 1) { signBias = exp2inline_SIGN_BIAS; }
			baseInt &= 0x7FFFFFFF;
		}
		if (baseInt < 0x00800000)
		{
			// Normalize subnormal base so exponent becomes negative.
			baseInt = asuint(base * 0x1p23F);
			baseInt &= 0x7FFFFFFF;
			baseInt -= 23 << 23;
		}
	}
	double_t logBase = log2_inline(baseInt);
	double_t exponentLogBase = exponent * logBase; // cannot overflow, exponent is single prec.
	if (predict_false((asuint64(exponentLogBase) >> 47 & 0xFFFF) >= asuint64(126.0 * POWF_SCALE) >> 47))
	{
		// |exponent*log(base)| >= 126.
		if (exponentLogBase > 0x1.FFFFFFFD1D571p+6 * POWF_SCALE) { return __math_oflowf(signBias); }
		if (exponentLogBase <= -150.0 * POWF_SCALE) { return __math_uflowf(signBias); }
	}
	return exp2_inline(exponentLogBase, signBias);
}
double pow(double base, double exponent)
{
	//Modified implementation from Musl Lib-C
	uint32_t signBias = 0;
	uint64_t baseInt, exponentInt;
	uint32_t baseTop12, exponentTop12;
	
	baseInt = asuint64(base);
	exponentInt = asuint64(exponent);
	baseTop12 = top12(base);
	exponentTop12 = top12(exponent);
	if (predict_false(baseTop12 - 0x001 >= 0x7FF - 0x001 || (exponentTop12 & 0x7FF) - 0x3BE >= 0x43E - 0x3BE))
	{
		// Note: if |exponent| > 1075 * ln2 * 2^53 ~= 0x1.749p62 then pow(base,exponent) = inf/0
		// and if |exponent| < 2^-54 / 1075 ~= 0x1.e7b6p-65 then pow(base,exponent) = +-1.
		// Special cases: (base < 0x1p-126 or inf or nan) or
		// (|exponent| < 0x1p-65 or |exponent| >= 0x1p63 or nan).
		if (predict_false(zeroinfnan64(exponentInt)))
		{
			if (2 * exponentInt == 0) { return 1.0; }
			if (baseInt == asuint64(1.0)) { return 1.0; }
			if (2 * baseInt > 2 * asuint64(INFINITY) || 2 * exponentInt > 2 * asuint64(INFINITY)) { return base + exponent; }
			if (2 * baseInt == 2 * asuint64(1.0)) { return 1.0; }
			if ((2 * baseInt < 2 * asuint64(1.0)) == !(exponentInt >> 63)) { return 0.0; } // |base|<1 && exponent==inf or |base|>1 && exponent==-inf.
			return exponent * exponent;
		}
		if (predict_false(zeroinfnan64(baseInt)))
		{
			double_t xSquared = base * base;
			if (baseInt >> 63 && checkint64(exponentInt) == 1) { xSquared = -xSquared; }
			// Without the barrier some versions of clang hoist the 1/xSquared and
			// thus division by zero exception can be signaled spuriously.
			return (exponentInt >> 63) ? fp_barrier(1 / xSquared) : xSquared;
		}
		// Here base and exponent are non-zero finite.
		if (baseInt >> 63)
		{
			// Finite base < 0.
			int exponentType = checkint64(exponentInt);
			if (exponentType == 0) { return __math_invalid(base); }
			if (exponentType == 1) { signBias = expinline_SIGN_BIAS; }
			baseInt &= 0x7FFFFFFFFFFFFFFF;
			baseTop12 &= 0x7FF;
		}
		if ((exponentTop12 & 0X7FF) - 0X3BE >= 0X43E - 0X3BE)
		{
			// Note: signBias == 0 here because exponent is not odd.
			if (baseInt == asuint64(1.0)) { return 1.0; }
			if ((exponentTop12 & 0x7FF) < 0x3BE)
			{
				// |exponent| < 2^-65, base^exponent ~= 1 + exponent*log(base).
				return 1.0;
			}
			return (baseInt > asuint64(1.0)) == (exponentTop12 < 0x800)
				? __math_oflow(0)
				: __math_uflow(0);
		}
		if (baseTop12 == 0)
		{
			// Normalize subnormal base so exponent becomes negative.
			baseInt = asuint64(base * 0x1p52);
			baseInt &= 0x7FFFFFFFFFFFFFFF;
			baseInt -= (52ULL << 52);
		}
	}
	
	double_t baseLow1;
	double_t baseHigh1 = log_inline(baseInt, &baseLow1);
	double_t exponentHigh1, exponentLow1;
	double_t exponentHigh2 = asdouble(exponentInt & (-1ULL << 27));
	double_t exponentLow2 = exponent - exponentHigh2;
	double_t baseHigh2 = asdouble(asuint64(baseHigh1) & (-1ULL << 27));
	double_t baseLow2 = baseHigh1 - baseHigh2 + baseLow1;
	exponentHigh1 = exponentHigh2 * baseHigh2;
	exponentLow1 = exponentLow2 * baseHigh2 + exponent * baseLow2; // |exponentLow1| < |exponentHigh1| * 2^-25.
	return exp_inline(exponentHigh1, exponentLow1, signBias);
}

float logf(float value)
{
	//Modified implementation from Musl Lib-C
	double_t zVar, rVar, rVarSquared, result, yVar, cInverse, logc;
	uint32_t valueInt, zVarInt, temp;
	int vVar, index;

	valueInt = asuint(value);
	// Fix sign of zero with downward rounding when value==1.
	if (predict_false(valueInt == 0x3F800000)) { return 0; }
	if (predict_false(valueInt - 0x00800000 >= 0x7F800000 - 0x00800000))
	{
		// value < 0x1p-126 or inf or nan.
		if (valueInt * 2 == 0) { return __math_divzerof(1); }
		if (valueInt == 0x7F800000) { return value; } // log(inf) == inf.
		if ((valueInt & 0x80000000) || valueInt * 2 >= 0xFF000000) { return __math_invalidf(value); }
		// value is subnormal, normalize it.
		valueInt = asuint(value * 0x1p23F);
		valueInt -= 23 << 23;
	}

	// value = 2^vVar zVar; where zVar is in range [OFF,2*OFF] and exact.
	// The range is split into N subintervals.
	// The ith subinterval contains zVar and c is near its center.
	temp = valueInt - logf_OFF;
	index = (temp >> (23 - LOGF_TABLE_BITS)) % logf_N;
	vVar = ((int32_t)temp >> 23); // arithmetic shift
	zVarInt = valueInt - (temp & 0x1FF << 23);
	cInverse = logf_T[index].invc;
	logc = logf_T[index].logc;
	zVar = (double_t)asfloat(zVarInt);

	// log(value) = log1p(zVar/c-1) + log(c) + vVar*Ln2
	rVar = (zVar * cInverse) - 1;
	yVar = logc + ((double_t)vVar * logf_Ln2);

	// Pipelined polynomial evaluation to approximate log1p(rVar).
	rVarSquared = (rVar * rVar);
	result = (logf_A[1] * rVar) + logf_A[2];
	result = (logf_A[0] * rVarSquared) + result;
	result = (result * rVarSquared) + (yVar + rVar);
	return eval_as_float(result);
}
double log(double value)
{
	//Modified implementation from Musl Lib-C
	double_t wVar, zVar, rVar, rVarSquared, rVarCubed, result, cInverse, logc, vVarDelta, resultHigh, resultLow;
	uint64_t valueInt, zVarInt, tmp;
	uint32_t top;
	int vVar, index;
	
	valueInt = asuint64(value);
	top = top16(value);
	if (predict_false(valueInt - asuint64(1.0 - 0x1p-4) < asuint64(1.0 + 0x1.09p-4) - asuint64(1.0 - 0x1p-4)))
	{
		// Handle close to 1.0 inputs separately.
		// Fix sign of zero with downward rounding when value==1.
		if (predict_false(valueInt == asuint64(1.0))) { return 0; }
		rVar = value - 1.0;
		rVarSquared = rVar * rVar;
		rVarCubed = rVar * rVarSquared;
		result = rVarCubed *
		    (log_B[1] + rVar * log_B[2] + rVarSquared * log_B[3] +
		     rVarCubed * (log_B[4] + rVar * log_B[5] + rVarSquared * log_B[6] +
			   rVarCubed * (log_B[7] + rVar * log_B[8] + rVarSquared * log_B[9] + rVarCubed * log_B[10])));
		// Worst-case error is around 0.507 ULP.
		wVar = rVar * 0x1p27;
		double_t rhi = rVar + wVar - wVar;
		double_t rlo = rVar - rhi;
		wVar = rhi * rhi * log_B[0]; // log_B[0] == -0.5.
		resultHigh = rVar + wVar;
		resultLow = rVar - resultHigh + wVar;
		resultLow += log_B[0] * rlo * (rhi + rVar);
		result += resultLow;
		result += resultHigh;
		return eval_as_double(result);
	}
	if (predict_false(top - 0x0010 >= 0x7FF0 - 0x0010))
	{
		// value < 0x1p-1022 or inf or nan.
		if (valueInt * 2 == 0) { return __math_divzero(1); }
		if (valueInt == asuint64(INFINITY)) { return value; } // log(inf) == inf.
		if ((top & 0x8000) || (top & 0x7FF0) == 0x7FF0) { return __math_invalid(value); }
		// value is subnormal, normalize it.
		valueInt = asuint64(value * 0x1p52);
		valueInt -= 52ULL << 52;
	}
	
	// value = 2^vVar zVar; where zVar is in range [OFF,2*OFF) and exact.
	// The range is split into N subintervals.
	// The ith subinterval contains zVar and c is near its center.
	tmp = valueInt - log_OFF;
	index = (tmp >> (52 - LOG_TABLE_BITS)) % log_N;
	vVar = (int64_t)tmp >> 52; /* arithmetic shift */
	zVarInt = valueInt - (tmp & 0xfffULL << 52);
	cInverse = log_T[index].invc;
	logc = log_T[index].logc;
	zVar = asdouble(zVarInt);
	
	// log(value) = log1p(zVar/c-1) + log(c) + vVar*Ln2.
	// rVar ~= zVar/c - 1, |rVar| < 1/(2*N).
	// rounding error: 0x1p-55/N + 0x1p-66.
	rVar = (zVar - log_T2[index].chi - log_T2[index].clo) * cInverse;
	vVarDelta = (double_t)vVar;
	
	// resultHigh + resultLow = rVar + log(c) + vVar*Ln2.
	wVar = vVarDelta * log_Ln2hi + logc;
	resultHigh = wVar + rVar;
	resultLow = wVar - resultHigh + rVar + vVarDelta * log_Ln2lo;
	
	// log(value) = resultLow + (log1p(rVar) - rVar) + resultHigh.
	rVarSquared = rVar * rVar; // rounding error: 0x1p-54/N^2.
	// Worst case error if |result| > 0x1p-5:
	// 0.5 + 4.13/N + abs-poly-error*2^57 ULP (+ 0.002 ULP without fma)
	// Worst case error if |result| > 0x1p-4:
	// 0.5 + 2.06/N + abs-poly-error*2^56 ULP (+ 0.001 ULP without fma).
	result = resultLow + (rVarSquared * log_A[0]) +
	    rVar * rVarSquared * (log_A[1] + rVar * log_A[2] + rVarSquared * (log_A[3] + rVar * log_A[4])) + resultHigh;
	return eval_as_double(result);
}

float log2f(float value)
{
	//Modified implementation from Musl Lib-C
	double_t zVar, rVar, rVarSquared, pVar, result, yVar, vInverse, logc;
	uint32_t valueInt, zVarInt, top, tmp;
	int k, i;
	
	valueInt = asuint(value);
	// Fix sign of zero with downward rounding when value==1.
	if (predict_false(valueInt == 0x3F800000)) { return 0; }
	if (predict_false(valueInt - 0x00800000 >= 0x7F800000 - 0x00800000))
	{
		// value < 0x1p-126 or inf or nan.
		if (valueInt * 2 == 0) { return __math_divzerof(1); }
		if (valueInt == 0x7F800000) { return value; } // log2(inf) == inf.
		if ((valueInt & 0x80000000) || valueInt * 2 >= 0xFF000000) { return __math_invalidf(value); }
		// value is subnormal, normalize it.
		valueInt = asuint(value * 0x1p23f);
		valueInt -= (23 << 23);
	}
	
	// value = 2^k zVar; where zVar is in range [OFF,2*OFF] and exact.
	// The range is split into N subintervals.
	// The ith subinterval contains zVar and c is near its center.
	tmp = valueInt - log2f_OFF;
	i = (tmp >> (23 - LOG2F_TABLE_BITS)) % log2f_N;
	top = (tmp & 0xFF800000);
	zVarInt = valueInt - top;
	k = (int32_t)tmp >> 23; // arithmetic shift
	vInverse = log2f_T[i].invc;
	logc = log2f_T[i].logc;
	zVar = (double_t)asfloat(zVarInt);
	
	// log2(value) = log1p(z/c-1)/ln2 + log2(c) + k
	rVar = zVar * vInverse - 1;
	yVar = logc + (double_t)k;
	
	// Pipelined polynomial evaluation to approximate log1p(rVar)/ln2.
	rVarSquared = rVar * rVar;
	result = log2f_A[1] * rVar + log2f_A[2];
	result = log2f_A[0] * rVarSquared + result;
	pVar = log2f_A[3] * rVar + yVar;
	result = result * rVarSquared + pVar;
	return eval_as_float(result);
}
double log2(double value)
{
	//Modified implementation from Musl Lib-C
	double_t zVar, rVar, rVarSquared, rVarQuad, result, cInverse, logc, tempDouble, resultHigh, resultLow, tVar1, tVar2, tVar3, polyValue;
	uint64_t valueInt, iz, tmp;
	uint32_t top;
	int temp, index;
	
	valueInt = asuint64(value);
	top = top16(value);
	if (predict_false(valueInt - asuint64(1.0 - 0x1.5B51p-5) < asuint64(1.0 + 0x1.6AB2p-5) - asuint64(1.0 - 0x1.5B51p-5)))
	{
		// Handle close to 1.0 inputs separately.
		// Fix sign of zero with downward rounding when value==1.
		if (predict_false(valueInt == asuint64(1.0))) { return 0; }
		rVar = value - 1.0;
		double_t rVarHigh, rVarLow;
		rVarHigh = asdouble(asuint64(rVar) & -1ULL << 32);
		rVarLow = rVar - rVarHigh;
		resultHigh = (rVarHigh * log2_InvLn2hi);
		resultLow = (rVarLow * log2_InvLn2hi) + (rVar * log2_InvLn2lo);
		rVarSquared = rVar * rVar; // rounding error: 0x1p-62.
		rVarQuad = rVarSquared * rVarSquared;
		// Worst-case error is less than 0.54 ULP (0.55 ULP without fma).
		polyValue = rVarSquared * (log2_B[0] + (rVar * log2_B[1]));
		result = resultHigh + polyValue;
		resultLow += resultHigh - result + polyValue;
		resultLow += rVarQuad * (log2_B[2] + rVar * log2_B[3] + rVarSquared * (log2_B[4] + rVar * log2_B[5]) +
			    rVarQuad * (log2_B[6] + rVar * log2_B[7] + rVarSquared * (log2_B[8] + rVar * log2_B[9])));
		result += resultLow;
		return eval_as_double(result);
	}
	
	if (predict_false(top - 0x0010 >= 0x7FF0 - 0x0010))
	{
		// value < 0x1p-1022 or inf or nan.
		if (valueInt * 2 == 0) { return __math_divzero(1); }
		if (valueInt == asuint64(INFINITY)) { return value; } // log(inf) == inf.
		if ((top & 0x8000) || (top & 0x7FF0) == 0x7FF0) { return __math_invalid(value); }
		// value is subnormal, normalize it.
		valueInt = asuint64(value * 0x1p52);
		valueInt -= 52ULL << 52;
	}
	
	// value = 2^temp zVar; where zVar is in range [OFF,2*OFF) and exact.
	// The range is split into N subintervals.
	// The ith subinterval contains zVar and c is near its center.
	tmp = valueInt - log2_OFF;
	index = (tmp >> (52 - LOG2_TABLE_BITS)) % log2_N;
	temp = (int64_t)tmp >> 52; // arithmetic shift
	iz = valueInt - (tmp & 0xfffULL << 52);
	cInverse = log2_T[index].invc;
	logc = log2_T[index].logc;
	zVar = asdouble(iz);
	tempDouble = (double_t)temp;
	
	// log2(value) = log2(zVar/c) + log2(c) + temp.
	// rVar ~= zVar/c - 1, |rVar| < 1/(2*N).
	double_t rVarHigh, rVarLow;
	// rounding error: 0x1p-55/N + 0x1p-65.
	rVar = (zVar - log2_T2[index].chi - log2_T2[index].clo) * cInverse;
	rVarHigh = asdouble(asuint64(rVar) & -1ULL << 32);
	rVarLow = rVar - rVarHigh;
	tVar1 = rVarHigh * log2_InvLn2hi;
	tVar2 = rVarLow * log2_InvLn2hi + rVar * log2_InvLn2lo;
	
	// resultHigh + resultLow = rVar/ln2 + log2(c) + temp.
	tVar3 = tempDouble + logc;
	resultHigh = tVar3 + tVar1;
	resultLow = tVar3 - resultHigh + tVar1 + tVar2;
	
	// log2(rVar+1) = rVar/ln2 + rVar^2*poly(rVar).
	// Evaluation is optimized assuming superscalar pipelined execution.
	rVarSquared = rVar * rVar; // rounding error: 0x1p-54/N^2.
	rVarQuad = rVarSquared * rVarSquared;
	// Worst-case error if |result| > 0x1p-4: 0.547 ULP (0.550 ULP without fma).
	// ~ 0.5 + 2/N/ln2 + abs-poly-error*0x1p56 ULP (+ 0.003 ULP without fma).
	polyValue = log2_A[0] + rVar * log2_A[1] + rVarSquared * (log2_A[2] + rVar * log2_A[3]) + rVarQuad * (log2_A[4] + rVar * log2_A[5]);
	result = resultLow + (rVarSquared * polyValue) + resultHigh;
	return eval_as_double(result);
}

float log10f(float value)
{
	//Modified implementation from Musl Lib-C
	union { float value; uint32_t integer; } valueUnion = { value };
	float_t hfsq, valueSubOne, sVar, sVarSquared, sVarQuad, tVar1, tVar2, tSum, vVarOriginal, highFloat, lowFloat;
	uint32_t valueInt;
	int vVar;
	
	valueInt = valueUnion.integer;
	vVar = 0;
	if (valueInt < 0x00800000 || valueInt>>31) // value < 2**-126
	{
		if (valueInt<<1 == 0) { return -1 / (value * value); } // log(+-0)=-inf
		if (valueInt>>31) { return (value - value) / 0.0f; } // log(-#) = NaN
		// subnormal number, scale up value
		vVar -= 25;
		value *= 0x1p25F;
		valueUnion.value = value;
		valueInt = valueUnion.integer;
	}
	else if (valueInt >= 0x7F800000) { return value; }
	else if (valueInt == 0x3F800000) { return 0; }
	
	// reduce value into [sqrt(2)/2, sqrt(2)]
	valueInt += 0x3F800000 - 0x3F3504F3;
	vVar += (int)(valueInt>>23) - 0x7F;
	valueInt = (valueInt & 0x007FFFFF) + 0x3F3504F3;
	valueUnion.integer = valueInt;
	value = valueUnion.value;
	
	valueSubOne = value - 1.0f;
	sVar = valueSubOne / (2.0f + valueSubOne);
	sVarSquared = sVar * sVar;
	sVarQuad = sVarSquared * sVarSquared;
	tVar1= sVarQuad * (Lg2 + (sVarQuad * Lg4));
	tVar2= sVarSquared * (Lg1 + (sVarQuad * Lg3));
	tSum = tVar2 + tVar1;
	hfsq = 0.5f * valueSubOne * valueSubOne;
	
	highFloat = valueSubOne - hfsq;
	valueUnion.value = highFloat;
	valueUnion.integer &= 0xFFFFF000;
	highFloat = valueUnion.value;
	lowFloat = valueSubOne - highFloat - hfsq + (sVar * (hfsq + tSum));
	vVarOriginal = vVar;
	return (vVarOriginal * log10_2lo) + ((lowFloat + highFloat) * ivln10lo) + (lowFloat * ivln10hi) + (highFloat * ivln10hi) + (vVarOriginal * log10_2hi);
}
double log10(double value)
{
	//Modified implementation from Musl Lib-C
	union { double value; uint64_t integer; } valueUnion = { value };
	double_t hfsq, valueSubOne, sVar, sVarSquared, sVarQuad, tVar1, tVar2, tSum, vVarOriginal, result, highDouble, lowDouble, resultHigh, resultLow;
	uint32_t valueUpperWord;
	int vVar;
	
	valueUpperWord = (valueUnion.integer >> 32);
	vVar = 0;
	if (valueUpperWord < 0x00100000 || (valueUpperWord >> 31))
	{
		if ((valueUnion.integer << 1) == 0) { return -1 / (value * value); } // log(+-0)=-inf
		if (valueUpperWord >> 31) { return (value - value) / 0.0; } // log(-#) = NaN
		// subnormal number, scale value up
		vVar -= 54;
		value *= 0x1p54;
		valueUnion.value = value;
		valueUpperWord = (valueUnion.integer >> 32);
	}
	else if (valueUpperWord >= 0x7FF00000) { return value; }
	else if (valueUpperWord == 0x3FF00000 && (valueUnion.integer << 32) == 0) { return 0; }
	
	// reduce value into [sqrt(2)/2, sqrt(2)]
	valueUpperWord += 0x3FF00000 - 0x3FE6A09E;
	vVar += (int)(valueUpperWord >> 20) - 0x3FF;
	valueUpperWord = (valueUpperWord & 0x000FFFFF) + 0x3FE6A09E;
	valueUnion.integer = (uint64_t)valueUpperWord << 32 | (valueUnion.integer & 0xFFFFFFFF);
	value = valueUnion.value;
	
	valueSubOne = value - 1.0;
	hfsq = 0.5 * valueSubOne * valueSubOne;
	sVar = valueSubOne / (2.0 + valueSubOne);
	sVarSquared = sVar * sVar;
	sVarQuad = sVarSquared * sVarSquared;
	tVar1 = sVarQuad * (Lg2d + sVarQuad * (Lg4d + sVarQuad * Lg6d));
	tVar2 = sVarSquared * (Lg1d + sVarQuad * (Lg3d + sVarQuad * (Lg5d + sVarQuad * Lg7d)));
	tSum = tVar2 + tVar1;
	
	// See log2.c for details.
	// highDouble+lowDouble = valueSubOne - hfsq + sVar*(hfsq+tSum) ~ log(1+valueSubOne)
	highDouble = valueSubOne - hfsq;
	valueUnion.value = highDouble;
	valueUnion.integer &= (uint64_t)-1 << 32;
	highDouble = valueUnion.value;
	lowDouble = valueSubOne - highDouble - hfsq + (sVar * (hfsq + tSum));
	
	// resultHigh+resultLow ~ log10(1+valueSubOne) + vVar*log10(2)
	resultHigh = highDouble * ivln10hid;
	vVarOriginal = vVar;
	result = vVarOriginal * log10_2hid;
	resultLow = vVarOriginal * log10_2lod + (lowDouble + highDouble) * ivln10lod + lowDouble * ivln10hid;
	
	// Extra precision in for adding result is not strictly needed
	// since there is no very large cancellation near value = sqrt(2) or
	// value = 1/sqrt(2), but we do it anyway since it costs little on CPUs
	// with some parallelism and it reduces the error for many args.
	sVarQuad = result + resultHigh;
	resultLow += (result - sVarQuad) + resultHigh;
	resultHigh = sVarQuad;
	
	return resultLow + resultHigh;
}

float _scalbnf(float value, int power)
{
	//Modified implementation from Musl Lib-C
	union { float value; uint32_t integer; } valueUnion;
	float_t result = value;
	
	if (power > 127)
	{
		result *= 0x1p127F;
		power -= 127;
		if (power > 127)
		{
			result *= 0x1p127F;
			power -= 127;
			if (power > 127) { power = 127; }
		}
	}
	else if (power < -126)
	{
		result *= 0x1p-126F * 0x1p24F;
		power += 126 - 24;
		if (power < -126)
		{
			result *= 0x1p-126F * 0x1p24F;
			power += 126 - 24;
			if (power < -126) { power = -126; }
		}
	}
	valueUnion.integer = ((uint32_t)(0x7F + power) << 23);
	result = result * valueUnion.value;
	return result;
}
double _scalbn(double value, int power)
{
	//Modified implementation from Musl Lib-C
	union { double value; uint64_t integer; } valueUnion;
	double_t result = value;
	
	if (power > 1023)
	{
		result *= 0x1p1023;
		power -= 1023;
		if (power > 1023)
		{
			result *= 0x1p1023;
			power -= 1023;
			if (power > 1023) { power = 1023; }
		}
	}
	else if (power < -1022)
	{
		// make sure final power < -53 to avoid double
		// rounding in the subnormal range
		result *= 0x1p-1022 * 0x1p53;
		power += 1022 - 53;
		if (power < -1022)
		{
			result *= 0x1p-1022 * 0x1p53;
			power += 1022 - 53;
			if (power < -1022) { power = -1022; }
		}
	}
	valueUnion.integer = ((uint64_t)(0x3FF + power) << 52);
	result = result * valueUnion.value;
	return result;
}

float ldexpf(float value, int exponent)
{
	return scalbnf(value, exponent);
}
double ldexp(double value, int exponent)
{
	return scalbn(value, exponent);
}

float expf(float value)
{
	uint32_t abstop;
	uint64_t ki, t;
	double_t kd, xd, z, r, r2, y, s;
	
	xd = (double_t)value;
	abstop = top12(value) & 0x7FF;
	if (predict_false(abstop >= top12(88.0f)))
	{
		// |value| >= 88 or value is nan.
		if (asuint(value) == asuint(-INFINITY)) { return 0.0f; }
		if (abstop >= top12(INFINITY)) { return value + value; }
		if (value > 0x1.62E42Ep6F) { return __math_oflowf(0); } // value > log(0x1p128) ~= 88.72
		if (value < -0x1.9FE368p6F) { return __math_uflowf(0); } // value < log(0x1p-150) ~= -103.97
	}
	
	// value*N/Ln2 = k + r with r in [-1/2, 1/2] and int k.
	z = exp2f_InvLn2N * xd;
	
	// Round and convert z to int, the result is in [-150*N, 128*N] and
	// ideally ties-to-even rule is used, otherwise the magnitude of r
	// can be bigger which gives larger approximation error.
	kd = eval_as_double(z + exp2f_SHIFT);
	ki = asuint64(kd);
	kd -= exp2f_SHIFT;
	r = z - kd;
	
	// exp(value) = 2^(k/N) * 2^(r/N) ~= s * (C0*r^3 + C1*r^2 + C2*r + 1)
	t = exp2f_T[ki % EXP2F_N];
	t += ki << (52 - EXP2F_TABLE_BITS);
	s = asdouble(t);
	z = exp2f_C[0] * r + exp2f_C[1];
	r2 = r * r;
	y = exp2f_C[2] * r + 1;
	y = z * r2 + y;
	y = y * s;
	return eval_as_float(y);
}
double exp(double value)
{
	uint32_t abstop;
	uint64_t ki, idx, top, sbits;
	double_t kd, z, r, r2, scale, tail, tmp;
	
	abstop = top12(value) & 0x7ff;
	if (predict_false(abstop - top12(0x1p-54) >= top12(512.0) - top12(0x1p-54))) {
		if (abstop - top12(0x1p-54) >= 0x80000000)
			// Avoid spurious underflow for tiny value.
			// Note: 0 is common input.
			return 1.0 + value;
		if (abstop >= top12(1024.0)) {
			if (asuint64(value) == asuint64(-INFINITY))
				return 0.0;
			if (abstop >= top12(INFINITY))
				return 1.0 + value;
			if (asuint64(value) >> 63)
				return __math_uflow(0);
			else
				return __math_oflow(0);
		}
		// Large value is special cased below.
		abstop = 0;
	}
	
	// exp(value) = 2^(k/N) * exp(r), with exp(r) in [2^(-1/2N),2^(1/2N)].
	// value = ln2/N*k + r, with int k and r in [-ln2/2N, ln2/2N].
	z = exp_InvLn2N * value;
	// z - kd is in [-1, 1] in non-nearest rounding modes.
	kd = eval_as_double(z + exp_Shift);
	ki = asuint64(kd);
	kd -= exp_Shift;
	r = value + kd * exp_NegLn2hiN + kd * exp_NegLn2loN;
	// 2^(k/N) ~= scale * (1 + tail).
	idx = 2 * (ki % exp_N);
	top = ki << (52 - EXP_TABLE_BITS);
	tail = asdouble(exp_T[idx]);
	// This is only a valid scale when -1023*N < k < 1024*N.
	sbits = exp_T[idx + 1] + top;
	// exp(value) = 2^(k/N) * exp(r) ~= scale + scale * (tail + exp(r) - 1).
	// Evaluation is optimized assuming superscalar pipelined execution.
	r2 = r * r;
	// Without fma the worst case error is 0.25/N ulp larger.
	// Worst case error is less than 0.5+1.11/N+(abs poly error * 2^53) ulp.
	tmp = tail + r + r2 * (exp_C2 + r * exp_C3) + r2 * r2 * (exp_C4 + r * exp_C5);
	if (predict_false(abstop == 0)) { return exp_specialcase(tmp, sbits, ki); }
	scale = asdouble(sbits);
	/* Note: tmp == 0 or |tmp| > 2^-200 and scale > 2^-739, so there
	   is no spurious underflow here even without fma.  */
	return eval_as_double(scale + scale * tmp);
}

//NOTE: sqrt is one of the few functions we can use the __builtin version of
#if 1
float sqrtf(float value)  { return __builtin_sqrtf(value); }
double sqrt(double value) { return __builtin_sqrt(value);  }
#else
float sqrtf(float value)
{
	//Modified implementation from Musl Lib-C
	uint32_t valueInt, mVar, mVar1, mVar0, even, ey;
	
	jsPrintString("Called sqrtf!");
	valueInt = asuint(value);
	if (predict_false(valueInt - 0x00800000 >= 0x7F800000 - 0x00800000))
	{
		// value < 0x1p-126 or inf or nan.
		if (valueInt * 2 == 0) { return value; }
		if (valueInt == 0x7F800000) { return value; }
		if (valueInt > 0x7F800000) { return __math_invalidf(value); }
		// value is subnormal, normalize it.
		valueInt = asuint(value * 0x1p23f);
		valueInt -= (23 << 23);
	}
	
	// value = 4^e mVar; with int e and mVar in [1, 4).
	even = (valueInt & 0x00800000);
	mVar1 = ((valueInt << 8) | 0x80000000);
	mVar0 = ((valueInt << 7) & 0x7FFFFFFF);
	mVar = (even ? mVar0 : mVar1);
	
	// 2^e is the exponent part of the return value.
	ey = (valueInt >> 1);
	ey += (0x3F800000 >> 1);
	ey &= 0x7F800000;
	
	// compute rVar ~ 1/sqrt(mVar), sVar ~ sqrt(mVar) with 2 goldschmidt iterations.
	static const uint32_t three = 0xC0000000;
	uint32_t rVar, sVar, dVar, uVar, iVar;
	iVar = (valueInt >> 17) % 128;
	rVar = (uint32_t)__rsqrt_table[iVar] << 16;
	// |rVar*sqrt(mVar) - 1| < 0x1p-8
	sVar = MultiplyU32Overflow(mVar, rVar);
	// |sVar/sqrt(mVar) - 1| < 0x1p-8
	dVar = MultiplyU32Overflow(sVar, rVar);
	uVar = three - dVar;
	rVar = MultiplyU32Overflow(rVar, uVar) << 1;
	// |rVar*sqrt(mVar) - 1| < 0x1.7bp-16
	sVar = MultiplyU32Overflow(sVar, uVar) << 1;
	// |sVar/sqrt(mVar) - 1| < 0x1.7bp-16
	dVar = MultiplyU32Overflow(sVar, rVar);
	uVar = three - dVar;
	sVar = MultiplyU32Overflow(sVar, uVar);
	// -0x1.03p-28 < sVar/sqrt(mVar) - 1 < 0x1.fp-31
	sVar = (sVar - 1)>>6;
	// sVar < sqrt(mVar) < sVar + 0x1.08p-23
	
	// compute nearest rounded result. 
	uint32_t dVar0, dVar1, dVar2;
	float result, tVar;
	dVar0 = (mVar << 16) - (sVar * sVar);
	dVar1 = sVar - dVar0;
	dVar2 = dVar1 + sVar + 1;
	sVar += (dVar1 >> 31);
	sVar &= 0x007FFFFF;
	sVar |= ey;
	result = asfloat(sVar);
	// handle rounding and inexact exception.
	uint32_t tiny = (predict_false(dVar2 == 0) ? 0 : 0x01000000);
	tiny |= ((dVar1 ^ dVar2) & 0x80000000);
	tVar = asfloat(tiny);
	result = eval_as_float(result + tVar);
	return result;
}
double sqrt(double value)
{
	//Modified implementation from Musl Lib-C
	uint64_t valueInt, top, mVar;
	
	// special case handling.
	valueInt = asuint64(value);
	top = (valueInt >> 52);
	if (predict_false((top - 0x001) >= (0x7FF - 0x001)))
	{
		// value < 0x1p-1022 or inf or nan.
		if (valueInt * 2 == 0) { return value; }
		if (valueInt == 0x7FF0000000000000) { return value; }
		if (valueInt > 0x7FF0000000000000) { return __math_invalid(value); }
		// value is subnormal, normalize it.
		valueInt = asuint64(value * 0x1p52);
		top = valueInt >> 52;
		top -= 52;
	}
	
	// argument reduction:
	// value = 4^e mVar; with integer e, and mVar in [1, 4)
	// mVar: fixed point representation [2.62]
	// 2^e is the exponent part of the result.
	int even = (top & 1);
	mVar = ((valueInt << 11) | 0x8000000000000000);
	if (even) { mVar >>= 1; }
	top = ((top + 0x3FF) >> 1);
	
	/* approximate rVar ~ 1/sqrt(mVar) and sVar ~ sqrt(mVar) when mVar in [1,4)

	   initial estimate:
	   7bit table lookup (1bit exponent and 6bit significand).

	   iterative approximation:
	   using 2 goldschmidt iterations with 32bit int arithmetics
	   and a final iteration with 64bit int arithmetics.

	   details:

	   the relative error (e = r0 sqrt(mVar)-1) of a linear estimate
	   (r0 = a mVar + b) is |e| < 0.085955 ~ 0x1.6p-4 at best,
	   a table lookup is faster and needs one less iteration
	   6 bit lookup table (128b) gives |e| < 0x1.f9p-8
	   7 bit lookup table (256b) gives |e| < 0x1.fdp-9
	   for single and double prec 6bit is enough but for quad
	   prec 7bit is needed (or modified iterations). to avoid
	   one more iteration >=13bit table would be needed (16k).

	   a newton-raphson iteration for rVar is
	     w = rVar*rVar
	     uVar = 3 - mVar*w
	     rVar = rVar*uVar/2
	   can use a goldschmidt iteration for sVar at the end or
	     sVar = mVar*rVar

	   first goldschmidt iteration is
	     sVar = mVar*rVar
	     uVar = 3 - sVar*rVar
	     rVar = rVar*uVar/2
	     sVar = sVar*uVar/2
	   next goldschmidt iteration is
	     uVar = 3 - sVar*rVar
	     rVar = rVar*uVar/2
	     sVar = sVar*uVar/2
	   and at the end rVar is not computed only sVar.

	   they use the same amount of operations and converge at the
	   same quadratic rate, i.e. if
	     r1 sqrt(mVar) - 1 = e, then
	     r2 sqrt(mVar) - 1 = -3/2 e^2 - 1/2 e^3
	   the advantage of goldschmidt is that the mul for sVar and rVar
	   are independent (computed in parallel), however it is not
	   "self synchronizing": it only uses the input mVar in the
	   first iteration so rounding errors accumulate. at the end
	   or when switching to larger precision arithmetics rounding
	   errors dominate so the first iteration should be used.

	   the fixed point representations are
	     mVar: 2.30 rVar: 0.32, sVar: 2.30, dVar: 2.30, uVar: 2.30, three: 2.30
	   and after switching to 64 bit
	     mVar: 2.62 rVar: 0.64, sVar: 2.62, dVar: 2.62, uVar: 2.62, three: 2.62
	*/
	
	static const uint64_t three = 0xC0000000;
	uint64_t rVar, sVar, dVar, uVar, tableIndex;
	
	tableIndex = ((valueInt >> 46) % 128);
	rVar = ((uint32_t)__rsqrt_table[tableIndex] << 16);
	// |rVar sqrt(mVar) - 1| < 0x1.fdp-9
	sVar = MultiplyU32Overflow((mVar >> 32), rVar);
	// |sVar/sqrt(mVar) - 1| < 0x1.fdp-9
	dVar = MultiplyU32Overflow(sVar, rVar);
	uVar = three - dVar;
	rVar = (MultiplyU32Overflow(rVar, uVar) << 1);
	// |rVar sqrt(mVar) - 1| < 0x1.7bp-16
	sVar = (MultiplyU32Overflow(sVar, uVar) << 1);
	// |sVar/sqrt(mVar) - 1| < 0x1.7bp-16
	dVar = MultiplyU32Overflow(sVar, rVar);
	uVar = three - dVar;
	rVar = (MultiplyU32Overflow(rVar, uVar) << 1);
	// |rVar sqrt(mVar) - 1| < 0x1.3704p-29 (measured worst-case)
	rVar = (rVar << 32);
	sVar = MultiplyU64Overflow(mVar, rVar);
	dVar = MultiplyU64Overflow(sVar, rVar);
	uVar = (three << 32) - dVar;
	sVar = MultiplyU64Overflow(sVar, uVar);  // repr: 3.61
	// -0x1p-57 < sVar - sqrt(mVar) < 0x1.8001p-61
	sVar = ((sVar - 2) >> 9); // repr: 12.52
	// -0x1.09p-52 < sVar - sqrt(mVar) < -0x1.fffcp-63
	
	// sVar < sqrt(mVar) < sVar + 0x1.09p-52,
	// compute nearest rounded result:
	// the nearest result to 52 bits is either sVar or sVar+0x1p-52,
	// we can decide by comparing (2^52 sVar + 0.5)^2 to 2^104 mVar.
	uint64_t dVar0, dVar1, dVar2;
	double result, tVar;
	dVar0 = (mVar << 42) - (sVar * sVar);
	dVar1 = sVar - dVar0;
	dVar2 = dVar1 + sVar + 1;
	sVar += (dVar1 >> 63);
	sVar &= 0x000FFFFFFFFFFFFF;
	sVar |= (top << 52);
	result = asdouble(sVar);
	// handle rounding modes and inexact exception:
	// only (sVar+1)^2 == 2^42 mVar case is exact otherwise
	// add a tiny value to cause the fenv effects.
	uint64_t tiny = (predict_false(dVar2==0) ? 0 : 0x0010000000000000);
	tiny |= ((dVar1 ^ dVar2) & 0x8000000000000000);
	tVar = asdouble(tiny);
	result = eval_as_double(result + tVar);
	return result;
}
#endif

float _cbrtf(float value)
{
	//Modified implementation from Musl Lib-C
	double_t tVarCubed, tVar;
	union { float value; uint32_t integer; } valueUnion = { value };
	uint32_t valueUnsigned = (valueUnion.integer & 0x7FFFFFFF);
	
	if (valueUnsigned >= 0x7F800000) { return (value + value); } // cbrt(NaN,INF) is itself
	
	// rough cbrt to 5 bits
	if (valueUnsigned < 0x00800000) // zero or subnormal?
	{
		if (valueUnsigned == 0) { return value; } // cbrt(+-0) is itself
		valueUnion.value = (value * 0x1p24F);
		valueUnsigned = (valueUnion.integer & 0x7FFFFFFF);
		valueUnsigned = (valueUnsigned / 3) + B2;
	}
	else { valueUnsigned = (valueUnsigned / 3) + B1; }
	valueUnion.integer &= 0x80000000;
	valueUnion.integer |= valueUnsigned;
	
	// First step Newton iteration (solving t*t-value/t == 0) to 16 bits.  In
	// double precision so that its terms can be arranged for efficiency
	// without causing overflow or underflow.
	tVar = valueUnion.value;
	tVarCubed = (tVar * tVar * tVar);
	tVar = tVar * ((double_t)value + value + tVarCubed) / (value + tVarCubed + tVarCubed);
	
	// Second step Newton iteration to 47 bits.  In double precision for
	// efficiency and accuracy.
	tVarCubed = (tVar * tVar * tVar);
	tVar = tVar * ((double_t)value + value + tVarCubed) / (value + tVarCubed + tVarCubed);
	
	// rounding to 24 bits is perfect in round-to-nearest mode
	return tVar;
}
double _cbrt(double value)
{
	//Modified implementation from Musl Lib-C
	union { double value; uint64_t integer; } valueUnion = { value };
	double_t rVar, sVar, tVar, wVar;
	uint32_t upperUnsigned = ((valueUnion.integer >> 32) & 0x7FFFFFFF);
	
	if (upperUnsigned >= 0x7FF00000) { return (value + value); } // cbrt(NaN,INF) is itself
	
	// Rough cbrt to 5 bits:
	//    cbrt(2**e*(1+m) ~= 2**(e/3)*(1+(e%3+m)/3)
	// where e is integral and >= 0, m is real and in [0, 1), and "/" and
	// "%" are integer division and modulus with rounding towards minus
	// infinity.  The RHS is always >= the LHS and has a maximum relative
	// error of about 1 in 16.  Adding a bias of -0.03306235651 to the
	// (e%3+m)/3 term reduces the error to about 1 in 32. With the IEEE
	// floating point representation, for finite positive normal values,
	// ordinary integer divison of the value in bits magically gives
	// almost exactly the RHS of the above provided we first subtract the
	// exponent bias (1023 for doubles) and later add it back.  We do the
	// subtraction virtually to keep e >= 0 so that ordinary integer
	// division rounds towards minus infinity; this is also efficient.
	if (upperUnsigned < 0x00100000) // zero or subnormal?
	{
		valueUnion.value = value * 0x1p54;
		upperUnsigned = ((valueUnion.integer >> 32) & 0x7FFFFFFF);
		if (upperUnsigned == 0) { return value; } // cbrt(0) is itself
		upperUnsigned = upperUnsigned/3 + Bd2;
	}
	else { upperUnsigned = upperUnsigned/3 + Bd1; }
	valueUnion.integer &= (1ULL << 63);
	valueUnion.integer |= (uint64_t)upperUnsigned << 32;
	tVar = valueUnion.value;
	
	// New cbrt to 23 bits:
	//    cbrt(value) = tVar*cbrt(value/tVar**3) ~= tVar*P(tVar**3/value)
	// where P(rVar) is a polynomial of degree 4 that approximates 1/cbrt(rVar)
	// to within 2**-23.5 when |rVar - 1| < 1/10.  The rough approximation
	// has produced tVar such than |tVar/cbrt(value) - 1| ~< 1/32, and cubing this
	// gives us bounds for rVar = tVar**3/value.
	//
	// Try to optimize for parallel evaluation as in __tanf.c.
	rVar = (tVar * tVar) * (tVar / value);
	tVar = tVar * ((P0 + (rVar * (P1 + (rVar * P2)))) + ((rVar * rVar) * rVar) * (P3 + (rVar * P4)));
	
	// Round tVar away from zero to 23 bits (sloppily except for ensuring that
	// the result is larger in magnitude than cbrt(value) but not much more than
	// 2 23-bit ulps larger).  With rounding towards zero, the error bound
	// would be ~5/6 instead of ~4/6.  With a maximum error of 2 23-bit ulps
	// in the rounded tVar, the infinite-precision error in the Newton
	// approximation barely affects third digit in the final error
	// 0.667; the error in the rounded tVar can be up to about 3 23-bit ulps
	// before the final error is larger than 0.667 ulps.
	valueUnion.value = tVar;
	valueUnion.integer = ((valueUnion.integer + 0x80000000) & 0xFFFFFFFFC0000000ULL);
	tVar = valueUnion.value;
	
	// one step Newton iteration to 53 bits with error < 0.667 ulps
	sVar = tVar * tVar;         // tVar*tVar is exact
	rVar = value / sVar;         // error <= 0.5 ulps; |rVar| < |tVar|
	wVar = tVar + tVar;         // tVar+tVar is exact
	rVar = (rVar - tVar) / (wVar + rVar); // rVar-tVar is exact; wVar+rVar ~= 3*tVar
	tVar = tVar + (tVar * rVar);       // error <= 0.5 + 0.5/3 + epsilon
	return tVar;
}