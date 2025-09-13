/*
File:   misc_hash.h
Author: Taylor Robbins
Date:   01\15\2025
Description:
	** Contains functions that perform various hash algorithms
*/

#ifndef _MISC_HASH_H
#define _MISC_HASH_H

#include "base/base_compiler_check.h"
#include "base/base_defines_check.h"
#include "base/base_typedefs.h"

#define FNV_HASH_BASE_U64   0xcbf29ce484222325ULL //= DEC(14,695,981,039,346,656,037)
#define FNV_HASH_PRIME_U64  0x00000100000001b3ULL //= DEC(1,099,511,628,211)
#define FNV_HASH_BASE_U32   0x811C9DC5U //= DEC(2,166,136,261)
#define FNV_HASH_PRIME_U32  0x01000193U //= DEC(16,777,619)

//NOTE: Apple's Clang does not support certain x86 instruction restrictions that meowhash relies on!
#if TARGET_IS_WASM || TARGET_IS_PLAYDATE || (TARGET_IS_OSX && COMPILER_IS_CLANG)
#define MEOW_HASH_AVAILABLE 0
#else
#define MEOW_HASH_AVAILABLE 1
#endif

typedef car Hash128 Hash128;
car Hash128
{
	u32 parts[4];
	u64 halves[2];
	plex { u32 part0; u32 part1; u32 part2; u32 part3; };
	plex { u32 first; u32 second; u32 third; u32 fourth; };
	plex { u64 left; u64 right; };
	plex { u64 lower; u64 upper; };
	plex { u64 half1; u64 half2; };
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE bool AreEqualHash128(Hash128 left, Hash128 right);
	u64 FnvHashU64Ex(const void* bufferPntr, u64 numBytes, u64 startingState);
	PIG_CORE_INLINE u64 FnvHashU64(const void* bufferPntr, u64 numBytes);
	u32 FnvHashU32Ex(const void* bufferPntr, u64 numBytes, u32 startingState);
	PIG_CORE_INLINE u32 FnvHashU32(const void* bufferPntr, u64 numBytes);
	PIG_CORE_INLINE u16 FnvHashU16(const void* bufferPntr, u64 numBytes);
	PIG_CORE_INLINE u8 FnvHashU8(const void* bufferPntr, u64 numBytes);
	#if MEOW_HASH_AVAILABLE
	Hash128 MeowHash128(const void* bufferPntr, u64 numBytes);
	u64 MeowHashU64(const void* bufferPntr, u64 numBytes);
	u32 MeowHashU32(const void* bufferPntr, u64 numBytes);
	u16 MeowHashU16(const void* bufferPntr, u64 numBytes);
	u8 MeowHashU8(const void* bufferPntr, u64 numBytes);
	#endif //MEOW_HASH_AVAILABLE
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
// See https://www.cantorsparadise.com/cantor-pairing-function-e213a8a89c2b
#define CantorPair(x, y) ((((x) + (y))*((x) + (y) + 1) + (y)) / 2)
#define CantorPairV(vector) CantorPair((vector).X, (vector).Y)
//NOTE: CantorTriplet seems to perform worse than FnvHashV3 when used for a hash table, even for tightly packed values, like our test with all values between (0,0,0) and (10,10,10)
#define CantorTriplet(x, y, z) (((CantorPair((x), (y)) + (z))*(CantorPair((x), (y)) + (z) + 1) + (z)) / 2)
#define CantorTripletV(vector) CantorTriplet((vector).X, (vector).Y, (vector).Z)

#define FnvHashV3(x, y, z) ((((((FNV_HASH_BASE_U32 ^ (u32)(x)) * FNV_HASH_PRIME_U32) ^ (u32)(y)) * FNV_HASH_PRIME_U32) ^ (u32)(z)) * FNV_HASH_PRIME_U32)

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI bool AreEqualHash128(Hash128 left, Hash128 right) { return (left.half1 == right.half1 && left.half2 == right.half2); }

// +--------------------------------------------------------------+
// |                      FNV Hash Algorithm                      |
// +--------------------------------------------------------------+
//FNV=Fowler-Noll-Vo "https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function"
//This is a 64-bit FNV-1a hash function:
PEXP u64 FnvHashU64Ex(const void* bufferPntr, u64 numBytes, u64 startingState)
{
	const u8* bytePntr = (const u8*)bufferPntr;
	u64 result = startingState;
	for (u64 bIndex = 0; bIndex < numBytes; bIndex++)
	{
		result = result ^ bytePntr[bIndex];
		result = result * FNV_HASH_PRIME_U64;
	}
	return result;
}
PEXPI u64 FnvHashU64(const void* bufferPntr, u64 numBytes) { return FnvHashU64Ex(bufferPntr, numBytes, FNV_HASH_BASE_U64); }
PEXP u32 FnvHashU32Ex(const void* bufferPntr, u64 numBytes, u32 startingState)
{
	const u8* bytePntr = (const u8*)bufferPntr;
	u32 result = startingState;
	for (u64 bIndex = 0; bIndex < numBytes; bIndex++)
	{
		result = result ^ bytePntr[bIndex];
		result = result * FNV_HASH_PRIME_U32;
	}
	return result;
}
PEXPI u32 FnvHashU32(const void* bufferPntr, u64 numBytes) { return FnvHashU32Ex(bufferPntr, numBytes, FNV_HASH_BASE_U32); }
PEXPI u16 FnvHashU16(const void* bufferPntr, u64 numBytes) { return (u16)FnvHashU32(bufferPntr, numBytes); }
PEXPI u8 FnvHashU8(const void* bufferPntr, u64 numBytes) { return (u8)FnvHashU32(bufferPntr, numBytes); }

// +--------------------------------------------------------------+
// |                     meow_hash Algorithm                      |
// +--------------------------------------------------------------+
//NOTE: When compiling for WASM or Playdate we are getting: fatal error: 'x86intrin.h' file not found
#if MEOW_HASH_AVAILABLE

#include "third_party/meow_hash/meow_hash_x64_aesni.h"

PEXP Hash128 MeowHash128(const void* bufferPntr, u64 numBytes)
{
	Assert(numBytes == 0 || bufferPntr != nullptr);
	meow_u128 hash128 = MeowHash(MeowDefaultSeed, numBytes, (void*)bufferPntr);
	Hash128 result;
	result.halves[0] = MeowU64From(hash128, 0);
	result.halves[1] = MeowU64From(hash128, 1);
	return result;
}
PEXP u64 MeowHashU64(const void* bufferPntr, u64 numBytes)
{
	Assert(numBytes == 0 || bufferPntr != nullptr);
	meow_u128 hash128 = MeowHash(MeowDefaultSeed, numBytes, (void*)bufferPntr);
	return MeowU64From(hash128, 0);
}
PEXP u32 MeowHashU32(const void* bufferPntr, u64 numBytes)
{
	Assert(numBytes == 0 || bufferPntr != nullptr);
	meow_u128 hash128 = MeowHash(MeowDefaultSeed, numBytes, (void*)bufferPntr);
	return MeowU32From(hash128, 0);
}
PEXP u16 MeowHashU16(const void* bufferPntr, u64 numBytes)
{
	Assert(numBytes == 0 || bufferPntr != nullptr);
	meow_u128 hash128 = MeowHash(MeowDefaultSeed, numBytes, (void*)bufferPntr);
	return (u16)MeowU32From(hash128, 0);
}
PEXP u8 MeowHashU8(const void* bufferPntr, u64 numBytes)
{
	Assert(numBytes == 0 || bufferPntr != nullptr);
	meow_u128 hash128 = MeowHash(MeowDefaultSeed, numBytes, (void*)bufferPntr);
	return (u8)MeowU32From(hash128, 0);
}

#endif //MEOW_HASH_AVAILABLE

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _MISC_HASH_H

#if defined(_MISC_HASH_H) && defined(_STRUCT_STRING_H)
#include "cross/cross_string_and_hash.h"
#endif
