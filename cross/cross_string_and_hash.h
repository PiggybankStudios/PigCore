/*
File:   cross_string_and_hash.h
Author: Taylor Robbins
Date:   01\15\2025
*/

#ifndef _CROSS_STRING_AND_HASH_H
#define _CROSS_STRING_AND_HASH_H

//NOTE: Intentionally no includes here

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE u8 FnvHashStrU8(Str8 string);
	PIG_CORE_INLINE u16 FnvHashStrU16(Str8 string);
	PIG_CORE_INLINE u32 FnvHashStrU32(Str8 string);
	PIG_CORE_INLINE u64 FnvHashStrU64(Str8 string);
	PIG_CORE_INLINE u8 MeowHashStrU8(Str8 string);
	PIG_CORE_INLINE u16 MeowHashStrU16(Str8 string);
	PIG_CORE_INLINE u32 MeowHashStrU32(Str8 string);
	PIG_CORE_INLINE u64 MeowHashStrU64(Str8 string);
	PIG_CORE_INLINE Hash128 MeowHashStr128(Str8 string);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI u8 FnvHashStrU8(Str8 string)
{
	return FnvHashU8(string.pntr, string.length);
}
PEXPI u16 FnvHashStrU16(Str8 string)
{
	return FnvHashU16(string.pntr, string.length);
}
PEXPI u32 FnvHashStrU32(Str8 string)
{
	return FnvHashU32(string.pntr, string.length);
}
PEXPI u64 FnvHashStrU64(Str8 string)
{
	return FnvHashU64(string.pntr, string.length);
}

#if MEOW_HASH_AVAILABLE
PEXPI u8 MeowHashStrU8(Str8 string)
{
	return MeowHashU8(string.pntr, string.length);
}
PEXPI u16 MeowHashStrU16(Str8 string)
{
	return MeowHashU16(string.pntr, string.length);
}
PEXPI u32 MeowHashStrU32(Str8 string)
{
	return MeowHashU32(string.pntr, string.length);
}
PEXPI u64 MeowHashStrU64(Str8 string)
{
	return MeowHashU64(string.pntr, string.length);
}
PEXPI Hash128 MeowHashStr128(Str8 string)
{
	return MeowHash128(string.pntr, string.length);
}
#endif //MEOW_HASH_AVAILABLE

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_STRING_AND_HASH_H
