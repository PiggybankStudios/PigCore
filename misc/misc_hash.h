/*
File:   misc_hash.h
Author: Taylor Robbins
Date:   01\15\2025
Description:
	** Contains functions that perform various hash algorithms
*/

#ifndef _MISC_HASH_H
#define _MISC_HASH_H

#include "base/base_typedefs.h"

typedef union Hash128 Hash128;
union Hash128
{
	u32 parts[4];
	u64 halves[2];
	struct { u32 part0; u32 part1; u32 part2; u32 part3; };
	struct { u32 first; u32 second; u32 third; u32 fourth; };
	struct { u64 left; u64 right; };
	struct { u64 lower; u64 upper; };
	struct { u64 half1; u64 half2; };
};

static inline bool AreEqualHash128(Hash128 left, Hash128 right) { return (left.half1 == right.half1 && left.half2 == right.half2); }

// +--------------------------------------------------------------+
// |                      FNV Hash Algorithm                      |
// +--------------------------------------------------------------+
//FNV=Fowler-Noll-Vo "https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function"
//This is a 64-bit FNV-1a hash function:

#define FNV_HASH_BASE_U64   0xcbf29ce484222325ULL //= DEC(14695981039346656037)
#define FNV_HASH_PRIME_U64  0x100000001b3ULL      //= DEC(1099511628211)

u64 FnvHashU64Ex(const void* bufferPntr, u64 numBytes, u64 startingState)
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
u64 FnvHashU64(const void* bufferPntr, u64 numBytes) { return FnvHashU64Ex(bufferPntr, numBytes, FNV_HASH_BASE_U64); }

u32 FnvHashU32(const void* bufferPntr, u64 numBytes)
{
	return (u32)FnvHashU64(bufferPntr, numBytes);
}

u16 FnvHashU16(const void* bufferPntr, u64 numBytes)
{
	return (u16)FnvHashU64(bufferPntr, numBytes);
}

u8 FnvHashU8(const void* bufferPntr, u64 numBytes)
{
	return (u8)FnvHashU64(bufferPntr, numBytes);
}

// +--------------------------------------------------------------+
// |                     meow_hash Algorithm                      |
// +--------------------------------------------------------------+
//NOTE: When compiling for WASM we are getting: fatal error: 'x86intrin.h' file not found
#if !TARGET_IS_WASM
#define MEOW_HASH_AVAILABLE 1
#include "third_party/meow_hash/meow_hash_x64_aesni.h"

Hash128 MeowHash128(const void* bufferPntr, u64 numBytes)
{
	Assert(numBytes == 0 || bufferPntr != nullptr);
	meow_u128 hash128 = MeowHash(MeowDefaultSeed, numBytes, (void*)bufferPntr);
	Hash128 result;
	result.halves[0] = MeowU64From(hash128, 0);
	result.halves[1] = MeowU64From(hash128, 1);
	return result;
}
u64 MeowHashU64(const void* bufferPntr, u64 numBytes)
{
	Assert(numBytes == 0 || bufferPntr != nullptr);
	meow_u128 hash128 = MeowHash(MeowDefaultSeed, numBytes, (void*)bufferPntr);
	return MeowU64From(hash128, 0);
}
u32 MeowHashU32(const void* bufferPntr, u64 numBytes)
{
	Assert(numBytes == 0 || bufferPntr != nullptr);
	meow_u128 hash128 = MeowHash(MeowDefaultSeed, numBytes, (void*)bufferPntr);
	return MeowU32From(hash128, 0);
}
u16 MeowHashU16(const void* bufferPntr, u64 numBytes)
{
	Assert(numBytes == 0 || bufferPntr != nullptr);
	meow_u128 hash128 = MeowHash(MeowDefaultSeed, numBytes, (void*)bufferPntr);
	return (u16)MeowU32From(hash128, 0);
}
u8 MeowHashU8(const void* bufferPntr, u64 numBytes)
{
	Assert(numBytes == 0 || bufferPntr != nullptr);
	meow_u128 hash128 = MeowHash(MeowDefaultSeed, numBytes, (void*)bufferPntr);
	return (u8)MeowU32From(hash128, 0);
}

#else
#define MEOW_HASH_AVAILABLE 0
#endif //!TARGET_IS_WASM

#endif //  _MISC_HASH_H

#if defined(_MISC_HASH_H) && defined(_STRUCT_STRING_H)
#include "cross/cross_string_and_hash.h"
#endif
