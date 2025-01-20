/*
File:   misc_random.h
Author: Taylor Robbins
Date:   01\05\2025
Description:
	** Holds functions that help us keep track of and define random number sequence algorithms
Wikipedia Link: https://en.wikipedia.org/wiki/List_of_random_number_generators
Other Page: https://peteroupc.github.io/random.html#Existing_RNG_APIs_in_Programming_Languages
*/

#ifndef _MISC_RANDOM_H
#define _MISC_RANDOM_H

#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"

#ifndef DEFAULT_RANDOM_SERIES_TYPE
#define DEFAULT_RANDOM_SERIES_TYPE RandomSeriesType_LinearCongruential64
#endif

#define RAND_FLOAT_PRECISION_R32 8000000UL //8 million
#define RAND_FLOAT_PRECISION_R64 400000000000000ULL //400 trillion

// +--------------------------------------------------------------+
// |                  Type/Structure Definitions                  |
// +--------------------------------------------------------------+
typedef enum RandomSeriesType RandomSeriesType;
enum RandomSeriesType
{
	RandomSeriesType_None = 0,
	RandomSeriesType_Fixed,
	RandomSeriesType_Incremental,
	RandomSeriesType_LinearCongruential32,   //LCG32
	RandomSeriesType_LinearCongruential64,   //LCG64
	RandomSeriesType_PermutedCongruential64, //PCG64
	RandomSeriesType_XoroShiro128,           //XS128
	RandomSeriesType_NumTypes,
};
#if !PIG_CORE_IMPLEMENTATION
const char* GetRandomSeriesTypeStr(RandomSeriesType enumValue);
const char* GetRandomSeriesTypeAcronymStr(RandomSeriesType enumValue);
#else
PEXP const char* GetRandomSeriesTypeStr(RandomSeriesType enumValue)
{
	switch (enumValue)
	{
		case RandomSeriesType_None:                   return "None";
		case RandomSeriesType_Fixed:                  return "Fixed";
		case RandomSeriesType_Incremental:            return "Incremental";
		case RandomSeriesType_LinearCongruential32:   return "LinearCongruential32";
		case RandomSeriesType_LinearCongruential64:   return "LinearCongruential64";
		case RandomSeriesType_PermutedCongruential64: return "PermutedCongruential64";
		case RandomSeriesType_XoroShiro128:           return "XoroShiro128";
		default: return UNKNOWN_STR;
	}
}
PEXP const char* GetRandomSeriesTypeAcronymStr(RandomSeriesType enumValue)
{
	switch (enumValue)
	{
		case RandomSeriesType_None:                   return "None";
		case RandomSeriesType_Fixed:                  return "Fixed";
		case RandomSeriesType_Incremental:            return "Inc";
		case RandomSeriesType_LinearCongruential32:   return "LCG32";
		case RandomSeriesType_LinearCongruential64:   return "LCG64";
		case RandomSeriesType_PermutedCongruential64: return "PGC64";
		case RandomSeriesType_XoroShiro128:           return "XS128";
		default: return UNKNOWN_STR;
	}
}
#endif

typedef struct RandomSeries RandomSeries;
struct RandomSeries
{
	RandomSeriesType type;
	bool seeded;
	u64 state;
	u64 state128[2]; //for 128-bit RNGs
	u64 defaultIncrement;
	u64 generationCount; //how many numbers have been generated since the series was seeded
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	void InitRandomSeriesEx(RandomSeries* series, RandomSeriesType type, u64 defaultIncrement);
	PIG_CORE_INLINE void InitRandomSeries(RandomSeries* series, RandomSeriesType type);
	PIG_CORE_INLINE void InitRandomSeriesDefault(RandomSeries* series);
	PIG_CORE_INLINE void SeedRandomSeriesU32(RandomSeries* series, u32 seed);
	PIG_CORE_INLINE void SeedRandomSeriesU64(RandomSeries* series, u64 seed);
	PIG_CORE_INLINE void SeedRandomSeriesU128(RandomSeries* series, u64 seed1, u64 seed2);
	PIG_CORE_INLINE u64 XS128_rotl(const u64 x, int k);
	void StepRandomSeries(RandomSeries* series, u64 numberOfSteps);
	PIG_CORE_INLINE u8 GetRandU8(RandomSeries* series);
	PIG_CORE_INLINE u8 GetRandU8Range(RandomSeries* series, u8 min, u8 max);
	PIG_CORE_INLINE u16 GetRandU16(RandomSeries* series);
	PIG_CORE_INLINE u16 GetRandU16Range(RandomSeries* series, u16 min, u16 max);
	PIG_CORE_INLINE u32 GetRandU32(RandomSeries* series);
	PIG_CORE_INLINE u32 GetRandU32Range(RandomSeries* series, u32 min, u32 max);
	PIG_CORE_INLINE u64 GetRandU64(RandomSeries* series);
	PIG_CORE_INLINE u64 GetRandU64Range(RandomSeries* series, u64 min, u64 max);
	PIG_CORE_INLINE i8 GetRandI8(RandomSeries* series);
	PIG_CORE_INLINE i8 GetRandI8Range(RandomSeries* series, i8 min, i8 max);
	PIG_CORE_INLINE i16 GetRandI16(RandomSeries* series);
	PIG_CORE_INLINE i16 GetRandI16Range(RandomSeries* series, i16 min, i16 max);
	PIG_CORE_INLINE i32 GetRandI32(RandomSeries* series);
	PIG_CORE_INLINE i32 GetRandI32Range(RandomSeries* series, i32 min, i32 max);
	PIG_CORE_INLINE i64 GetRandI64(RandomSeries* series);
	PIG_CORE_INLINE r32 GetRandR32(RandomSeries* series);
	PIG_CORE_INLINE r32 GetRandR32Range(RandomSeries* series, r32 min, r32 max);
	PIG_CORE_INLINE r64 GetRandR64(RandomSeries* series);
	PIG_CORE_INLINE r64 GetRandR64Range(RandomSeries* series, r64 min, r64 max);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                           Creation                           |
// +--------------------------------------------------------------+
PEXP void InitRandomSeriesEx(RandomSeries* series, RandomSeriesType type, u64 defaultIncrement)
{
	NotNull(series);
	ClearPointer(series);
	series->seeded = false;
	series->type = type;
	series->state = 0;
	series->defaultIncrement = defaultIncrement;
	series->generationCount = 0;
}
PEXPI void InitRandomSeries(RandomSeries* series, RandomSeriesType type) { InitRandomSeriesEx(series, type, 1); }
PEXPI void InitRandomSeriesDefault(RandomSeries* series) { InitRandomSeriesEx(series, DEFAULT_RANDOM_SERIES_TYPE, 1); }

// +--------------------------------------------------------------+
// |                             Seed                             |
// +--------------------------------------------------------------+
PEXPI void SeedRandomSeriesU32(RandomSeries* series, u32 seed)
{
	NotNull(series); //TODO: Convert to NotNull_?
	series->state = (u64)seed;
	series->state128[0] = (u64)seed;
	series->state128[1] = (u64)seed;
	series->generationCount = 0;
	series->seeded = true;
}
PEXPI void SeedRandomSeriesU64(RandomSeries* series, u64 seed)
{
	NotNull(series); //TODO: Convert to NotNull_?
	series->state = seed;
	series->state128[0] = seed;
	series->state128[1] = seed;
	series->generationCount = 0;
	series->seeded = true;
}
PEXPI void SeedRandomSeriesU128(RandomSeries* series, u64 seed1, u64 seed2)
{
	NotNull(series); //TODO: Convert to NotNull_?
	series->state = (seed1 ^ seed2);
	series->state128[0] = seed1;
	series->state128[1] = seed2;
	series->generationCount = 0;
	series->seeded = true;
}

// +--------------------------------------------------------------+
// |                       Helper Functions                       |
// +--------------------------------------------------------------+
//TODO: Can this function be inline tagged?
PEXPI u64 XS128_rotl(const u64 x, int k)
{
	return (x << k) | (x >> (64 - k));
}

// +--------------------------------------------------------------+
// |                             Step                             |
// +--------------------------------------------------------------+
PEXP void StepRandomSeries(RandomSeries* series, u64 numberOfSteps)
{
	NotNull(series); //TODO: Convert to NotNull_?
	Assert(series->type < RandomSeriesType_NumTypes); //TODO: Change to Assert_?
	Assert(series->seeded); //TODO: Change to Assert_?
	
	switch (series->type)
	{
		case RandomSeriesType_Fixed:
		{
			//Fixed doesn't do anything, it just keeps a fixed number
		} break;
		case RandomSeriesType_Incremental:
		{
			if (series->state < UINT64_MAX - numberOfSteps) { series->state += numberOfSteps; }
			else { series->state = numberOfSteps - (UINT64_MAX - numberOfSteps - series->state); }
			series->generationCount += numberOfSteps;
		} break;
		case RandomSeriesType_LinearCongruential32:
		{
			//n(x+1) = n(x) * A + C modulo M
			for (u64 sIndex = 0; sIndex < numberOfSteps; sIndex++)
			{
				u64 newState = (series->state * 1103515245ULL + 12345ULL) & 0xFFFFFFFFFFFFFFFFULL;
				series->state = newState;
			}
			series->generationCount += numberOfSteps;
		} break;
		case RandomSeriesType_LinearCongruential64:
		{
			//n(x+1) = n(x) * A + C modulo M
			//Values taken from https://nuclear.llnl.gov/CNP/rng/rngman/node4.html
			for (u64 sIndex = 0; sIndex < numberOfSteps; sIndex++)
			{
				u64 newState = (series->state * 2862933555777941757ULL + 3037000493ULL) & 0xFFFFFFFFFFFFFFFFULL;
				series->state = newState;
			}
			series->generationCount += numberOfSteps;
		} break;
		case RandomSeriesType_PermutedCongruential64:
		{
			//This code comes from BYP on Handmade Network
			for (u64 sIndex = 0; sIndex < numberOfSteps; sIndex++)
			{
				#define PCG_INC 1
				u64 newState = ((series->state * 6364136223846793005ULL) + (PCG_INC|1));
				newState = (newState ^ (newState >> 22)) >> (22 + (newState >> 61));
				series->state = newState;
			}
			series->generationCount += numberOfSteps;
		} break;
		case RandomSeriesType_XoroShiro128:
		{
			// https://xoroshiro.di.unimi.it/xoroshiro128plusplus.c
			//TODO: There are jump() and long_jump() functions we could use
			u64 s0 = series->state128[0];
			u64 s1 = series->state128[1];
			series->state = XS128_rotl(s0 + s1, 17) + s0;
			s1 ^= s0;
			series->state128[0] = XS128_rotl(s0, 49) ^ s1 ^ (s1 << 21); // a, b
			series->state128[1] = XS128_rotl(s1, 28); // c
		} break;
		default: Assert(false); break; //TODO: Change to Assert_?
	}
}

// +--------------------------------------------------------------+
// |                             Get                              |
// +--------------------------------------------------------------+
PEXPI u64 GetRandU64(RandomSeries* series);

//NOTE: Max values are not inclusive in all these functions (except float, but that generally doesn't matter)

//TODO: Are the modulo operators here going to effect the distribution of the random series?
//      Also do the floating point ideas of "precision" actually work like I want? Can we perform floating point
//      math up in those ranges? Maybe we should modulo into a conservative estimate of r32/r64 precision
PEXPI u8 GetRandU8(RandomSeries* series)
{
	u64 randU64 = GetRandU64(series);
	//TODO: Probably should just bit-mask instead of modulo for perf reasons
	return (u8)(randU64 % 256);
}
PEXPI u8 GetRandU8Range(RandomSeries* series, u8 min, u8 max)
{
	if (max < min) { SwapVariables(u8, min, max); }
	if (min == max) { return min; }
	u64 randU64 = GetRandU64(series);
	return (u8)((randU64 % (max-min)) + min);
}

PEXPI u16 GetRandU16(RandomSeries* series)
{
	u64 randU64 = GetRandU64(series);
	return (u16)(randU64 % 65536);
}
PEXPI u16 GetRandU16Range(RandomSeries* series, u16 min, u16 max)
{
	if (max < min) { SwapVariables(u16, min, max); }
	if (min == max) { return min; }
	u64 randU64 = GetRandU64(series);
	return (u16)((randU64 % (max-min)) + min);
}

PEXPI u32 GetRandU32(RandomSeries* series)
{
	NotNull(series); //TODO: Convert to NotNull_?
	StepRandomSeries(series, series->defaultIncrement);
	return (u32)(series->state % UINT32_MAX);
}
PEXPI u32 GetRandU32Range(RandomSeries* series, u32 min, u32 max)
{
	NotNull(series); //TODO: Convert to NotNull_?
	if (max < min) { SwapVariables(u32, min, max); }
	if (min == max) { return min; }
	u32 result = GetRandU32(series);
	result = (result % (max - min)) + min;
	return result;
}

//TODO: For some reason we are getting these unreachable code warnings in this code??
#if COMPILER_IS_MSVC
#pragma warning(push)
#pragma warning(disable: 4702) //warning C4702: unreachable code
#endif //COMPILER_IS_MSVC

PEXPI u64 GetRandU64(RandomSeries* series) //pre-declared a little ways above since all other unsigned GetRands rely on this one
{
	NotNull(series); //TODO: Convert to NotNull_?
	StepRandomSeries(series, series->defaultIncrement);
	return series->state;
}
PEXPI u64 GetRandU64Range(RandomSeries* series, u64 min, u64 max)
{
	NotNull(series); //TODO: Convert to NotNull_?
	if (max < min) { SwapVariables(u64, min, max); }
	if (min == max) { return min; }
	u64 result = GetRandU64(series);
	result = (result % (max - min)) + min;
	return result;
}

PEXPI i8 GetRandI8(RandomSeries* series)
{
	return ReinterpretCastU8ToI8(GetRandU8(series));
	u64 randU64 = GetRandU64(series);
	return (i8)((randU64 % 256) - 128);
}
PEXPI i8 GetRandI8Range(RandomSeries* series, i8 min, i8 max)
{
	if (max < min) { SwapVariables(i8, min, max); }
	if (min == max) { return min; }
	u64 randU64 = GetRandU64(series);
	return (i8)((i32)(randU64 % (u64)(max-min)) + min);
}

PEXPI i16 GetRandI16(RandomSeries* series)
{
	return ReinterpretCastU16ToI16(GetRandU16(series));
	u64 randU64 = GetRandU64(series);
	return (i16)((randU64 % 256) - 1216);
}
PEXPI i16 GetRandI16Range(RandomSeries* series, i16 min, i16 max)
{
	if (max < min) { SwapVariables(i16, min, max); }
	if (min == max) { return min; }
	u64 randU64 = GetRandU64(series);
	return (i16)((i32)(randU64 % (u64)(max-min)) + min);
}

#if COMPILER_IS_MSVC
#pragma warning(pop)
#endif

PEXPI i32 GetRandI32(RandomSeries* series)
{
	return ReinterpretCastU32ToI32(GetRandU32(series));
}
PEXPI i32 GetRandI32Range(RandomSeries* series, i32 min, i32 max)
{
	if (max < min) { SwapVariables(i32, min, max); }
	if (min == max) { return min; }
	u64 randU64 = GetRandU64(series);
	return (i32)((i64)(randU64 % (u64)(max-min)) + min);
}

PEXPI i64 GetRandI64(RandomSeries* series)
{
	return ReinterpretCastU64ToI64(GetRandU64(series));
}
//TODO: If we can find a better way to do the signed modulos that don't require a larger number space to accomplish then we should implement GetRandI64

PEXPI r32 GetRandR32(RandomSeries* series)
{
	NotNull(series); //TODO: Convert to NotNull_?
	StepRandomSeries(series, series->defaultIncrement);
	u64 integerRandom = (series->state % RAND_FLOAT_PRECISION_R32);
	r32 result = (r32)integerRandom / (r32)RAND_FLOAT_PRECISION_R32;
	return result;
}
PEXPI r32 GetRandR32Range(RandomSeries* series, r32 min, r32 max)
{
	if (max < min) { SwapVariables(r32, min, max); }
	return (GetRandR32(series) * (max - min)) + min;
}

PEXPI r64 GetRandR64(RandomSeries* series)
{
	NotNull(series); //TODO: Convert to NotNull_?
	StepRandomSeries(series, series->defaultIncrement);
	u64 integerRandom = (series->state % RAND_FLOAT_PRECISION_R64);
	r64 result = (r64)integerRandom / (r64)RAND_FLOAT_PRECISION_R64;
	return result;
}
PEXPI r64 GetRandR64Range(RandomSeries* series, r64 min, r64 max)
{
	if (max < min) { SwapVariables(r64, min, max); }
	return (GetRandR64(series) * (max - min)) + min;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _MISC_RANDOM_H
