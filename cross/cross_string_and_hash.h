/*
File:   cross_string_and_hash.h
Author: Taylor Robbins
Date:   01\15\2025
*/

#ifndef _CROSS_STRING_AND_HASH_H
#define _CROSS_STRING_AND_HASH_H

u8 FnvHashStrU8(Str8 string)
{
	return FnvHashU8(string.pntr, string.length);
}
u16 FnvHashStrU16(Str8 string)
{
	return FnvHashU16(string.pntr, string.length);
}
u32 FnvHashStrU32(Str8 string)
{
	return FnvHashU32(string.pntr, string.length);
}
u64 FnvHashStrU64(Str8 string)
{
	return FnvHashU64(string.pntr, string.length);
}

#if MEOW_HASH_AVAILABLE
u8 MeowHashStrU8(Str8 string)
{
	return MeowHashU8(string.pntr, string.length);
}
u16 MeowHashStrU16(Str8 string)
{
	return MeowHashU16(string.pntr, string.length);
}
u32 MeowHashStrU32(Str8 string)
{
	return MeowHashU32(string.pntr, string.length);
}
u64 MeowHashStrU64(Str8 string)
{
	return MeowHashU64(string.pntr, string.length);
}
Hash128 MeowHashStr128(Str8 string)
{
	return MeowHash128(string.pntr, string.length);
}
#endif

#endif //  _CROSS_STRING_AND_HASH_H
