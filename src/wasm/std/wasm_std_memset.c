/*
File:   wasm_std_memset.c
Author: Taylor Robbins
Date:   01\10\2025
Description: 
	** Holds implementations of functions like memset(), memcpy(), memmove(), etc.
*/

#define WASM_STD_USE_BUILTIN_MEMSET  1
#define WASM_STD_USE_BUILTIN_MEMCPY  1
#define WASM_STD_USE_BUILTIN_MEMMOVE 1

#define WORD_CONTAINS_ZERO(word) ((word)-((size_t)-1/UCHAR_MAX) & ~(word) & (((size_t)-1/UCHAR_MAX) * (UCHAR_MAX/2+1)))

#define MAX(left, right) ((left) > (right) ? (left) : (right))
#define MIN(left, right) ((left) < (right) ? (left) : (right))

typedef size_t __attribute__((__may_alias__)) a_size_t;
#define SIZEOF_A_SIZE_T sizeof(a_size_t)
typedef uint32_t __attribute__((__may_alias__)) a_u32;
typedef uint64_t __attribute__((__may_alias__)) a_u64;

void* _memset(void* pntr, int value, size_t numBytes)
{
	#if WASM_STD_USE_BUILTIN_MEMSET
	// Basically converts to memory.fill instruction
	return __builtin_memset(pntr, value, numBytes);
	#else
	{
		//Modified implementation from Musl Lib-C
		unsigned char* charPntr = pntr;
		size_t alignment;
		
		// Fill head and tail with minimal branching. Each
		// conditional ensures that all the subsequently used
		// offsets are well-defined and in the pntr region.
		
		if (!numBytes) { return pntr; }
		charPntr[0] = value;
		charPntr[numBytes-1] = value;
		if (numBytes <= 2) { return pntr; }
		charPntr[1] = value;
		charPntr[2] = value;
		charPntr[numBytes-2] = value;
		charPntr[numBytes-3] = value;
		if (numBytes <= 6) { return pntr; }
		charPntr[3] = value;
		charPntr[numBytes-4] = value;
		if (numBytes <= 8) { return pntr; }
		
		// Advance pointer to align it at a 4-byte boundary,
		// and truncate numBytes to a multiple of 4. The previous code
		// already took care of any head/tail that get cut off
		// by the alignment.
		alignment = (-(uintptr_t)charPntr & 3);
		charPntr += alignment;
		numBytes -= alignment;
		numBytes &= -4;
		
		a_u32 value32 = ((a_u32) - 1) / 255 * (unsigned char)value;
		
		// In preparation to copy 32 bytes at a time, aligned on
		// an 8-byte bounary, fill head/tail up to 28 bytes each.
		// As in the initial byte-based head/tail fill, each
		// conditional below ensures that the subsequent offsets
		// are valid (e.g. !(numBytes<=24) implies numBytes>=28).
		*(a_u32*)(charPntr + 0) = value32;
		*(a_u32*)(charPntr + numBytes - 4) = value32;
		if (numBytes <= 8) { return pntr; }
		*(a_u32*)(charPntr + 4) = value32;
		*(a_u32*)(charPntr + 8) = value32;
		*(a_u32*)(charPntr + numBytes - 12) = value32;
		*(a_u32*)(charPntr + numBytes - 8) = value32;
		if (numBytes <= 24) { return pntr; }
		*(a_u32*)(charPntr + 12) = value32;
		*(a_u32*)(charPntr + 16) = value32;
		*(a_u32*)(charPntr + 20) = value32;
		*(a_u32*)(charPntr + 24) = value32;
		*(a_u32*)(charPntr + numBytes - 28) = value32;
		*(a_u32*)(charPntr + numBytes - 24) = value32;
		*(a_u32*)(charPntr + numBytes - 20) = value32;
		*(a_u32*)(charPntr + numBytes - 16) = value32;
		
		// Align to a multiple of 8 so we can fill 64 bits at a time,
		// and avoid writing the same bytes twice as much as is
		// practical without introducing additional branching.
		alignment = 24 + ((uintptr_t)charPntr & 4);
		charPntr += alignment;
		numBytes -= alignment;
		
		// If this loop is reached, 28 tail bytes have already been
		// filled, so any remainder when numBytes drops below 32 can be
		// safely ignored.
		a_u64 value64 = (value32 | ((a_u64)value32 << 32));
		for (; numBytes >= 32; numBytes -= 32, charPntr += 32)
		{
			*(a_u64*)(charPntr + 0) = value64;
			*(a_u64*)(charPntr + 8) = value64;
			*(a_u64*)(charPntr + 16) = value64;
			*(a_u64*)(charPntr + 24) = value64;
		}
		
		return pntr;
	}
	#endif //WASM_STD_USE_BUILTIN_MEMSET
}

int memcmp(const void* left, const void* right, size_t numBytes)
{
	//Modified implementation from Musl Lib-C
	const unsigned char *leftByte = left, *rightByte = right;
	for (; (numBytes && *leftByte == *rightByte); numBytes--, leftByte++, rightByte++);
	return (numBytes ? *leftByte - *rightByte : 0);
}

void* _memcpy(void* dest, const void* source, size_t numBytes)
{
	#if WASM_STD_USE_BUILTIN_MEMCPY
	// Basically converts to memory.copy instruction
	return __builtin_memcpy(dest, source, numBytes);
	#else
	{
		//Modified implementation from Musl Lib-C
		unsigned char* destCharPntr = dest;
		const unsigned char* sourceCharPntr = source;
		
		uint32_t wVarU32, xVarU32;
		
		for (; (uintptr_t)sourceCharPntr % 4 && numBytes; numBytes--)
		{
			*destCharPntr++ = *sourceCharPntr++;
		}
		
		if ((uintptr_t)destCharPntr % 4 == 0)
		{
			for (; numBytes >= 16; sourceCharPntr += 16, destCharPntr += 16, numBytes -= 16)
			{
				*(a_u32*)(destCharPntr + 0) = *(a_u32*)(sourceCharPntr + 0);
				*(a_u32*)(destCharPntr + 4) = *(a_u32*)(sourceCharPntr + 4);
				*(a_u32*)(destCharPntr + 8) = *(a_u32*)(sourceCharPntr + 8);
				*(a_u32*)(destCharPntr + 12) = *(a_u32*)(sourceCharPntr + 12);
			}
			if (numBytes & 8)
			{
				*(a_u32*)(destCharPntr + 0) = *(a_u32*)(sourceCharPntr + 0);
				*(a_u32*)(destCharPntr + 4) = *(a_u32*)(sourceCharPntr + 4);
				destCharPntr += 8;
				sourceCharPntr += 8;
			}
			if (numBytes & 4)
			{
				*(a_u32*)(destCharPntr+0) = *(a_u32*)(sourceCharPntr + 0);
				destCharPntr += 4;
				sourceCharPntr += 4;
			}
			if (numBytes & 2)
			{
				*destCharPntr++ = *sourceCharPntr++;
				*destCharPntr++ = *sourceCharPntr++;
			}
			if (numBytes & 1)
			{
				*destCharPntr = *sourceCharPntr;
			}
			return dest;
		}
		
		if (numBytes >= 32)
		{
			switch ((uintptr_t)destCharPntr % 4)
			{
				case 1:
				{
					wVarU32 = *(a_u32*)sourceCharPntr;
					*destCharPntr++ = *sourceCharPntr++;
					*destCharPntr++ = *sourceCharPntr++;
					*destCharPntr++ = *sourceCharPntr++;
					numBytes -= 3;
					for (; numBytes >= 17; sourceCharPntr += 16, destCharPntr += 16, numBytes -= 16)
					{
						xVarU32 = *(a_u32*)(sourceCharPntr + 1);
						*(a_u32*)(destCharPntr + 0) = ((wVarU32 >> 24) | (xVarU32 << 8));
						wVarU32 = *(a_u32*)(sourceCharPntr + 5);
						*(a_u32*)(destCharPntr + 4) = ((xVarU32 >> 24) | (wVarU32 << 8));
						xVarU32 = *(a_u32*)(sourceCharPntr + 9);
						*(a_u32*)(destCharPntr + 8) = ((wVarU32 >> 24) | (xVarU32 << 8));
						wVarU32 = *(a_u32*)(sourceCharPntr + 13);
						*(a_u32*)(destCharPntr + 12) = ((xVarU32 >> 24) | (wVarU32 << 8));
					}
				} break;
				case 2:
				{
					wVarU32 = *(a_u32*)sourceCharPntr;
					*destCharPntr++ = *sourceCharPntr++;
					*destCharPntr++ = *sourceCharPntr++;
					numBytes -= 2;
					for (; numBytes >= 18; sourceCharPntr += 16, destCharPntr += 16, numBytes -= 16)
					{
						xVarU32 = *(a_u32*)(sourceCharPntr + 2);
						*(a_u32*)(destCharPntr + 0) = ((wVarU32 >> 16) | (xVarU32 << 16));
						wVarU32 = *(a_u32*)(sourceCharPntr + 6);
						*(a_u32*)(destCharPntr + 4) = ((xVarU32 >> 16) | (wVarU32 << 16));
						xVarU32 = *(a_u32*)(sourceCharPntr + 10);
						*(a_u32*)(destCharPntr + 8) = ((wVarU32 >> 16) | (xVarU32 << 16));
						wVarU32 = *(a_u32*)(sourceCharPntr + 14);
						*(a_u32*)(destCharPntr + 12) = ((xVarU32 >> 16) | (wVarU32 << 16));
					}
				} break;
				case 3:
				{
					wVarU32 = *(a_u32*)sourceCharPntr;
					*destCharPntr++ = *sourceCharPntr++;
					numBytes -= 1;
					for (; numBytes >= 19; sourceCharPntr += 16, destCharPntr += 16, numBytes -= 16)
					{
						xVarU32 = *(a_u32*)(sourceCharPntr + 3);
						*(a_u32*)(destCharPntr + 0) = ((wVarU32 >> 8) | (xVarU32 << 24));
						wVarU32 = *(a_u32*)(sourceCharPntr + 7);
						*(a_u32*)(destCharPntr + 4) = ((xVarU32 >> 8) | (wVarU32 << 24));
						xVarU32 = *(a_u32*)(sourceCharPntr + 11);
						*(a_u32*)(destCharPntr + 8) = ((wVarU32 >> 8) | (xVarU32 << 24));
						wVarU32 = *(a_u32*)(sourceCharPntr + 15);
						*(a_u32*)(destCharPntr + 12) = ((xVarU32 >> 8) | (wVarU32 << 24));
					}
				} break;
			}
		}
		if (numBytes & 16)
		{
			*destCharPntr++ = *sourceCharPntr++;
			*destCharPntr++ = *sourceCharPntr++;
			*destCharPntr++ = *sourceCharPntr++;
			*destCharPntr++ = *sourceCharPntr++;
			*destCharPntr++ = *sourceCharPntr++;
			*destCharPntr++ = *sourceCharPntr++;
			*destCharPntr++ = *sourceCharPntr++;
			*destCharPntr++ = *sourceCharPntr++;
			*destCharPntr++ = *sourceCharPntr++;
			*destCharPntr++ = *sourceCharPntr++;
			*destCharPntr++ = *sourceCharPntr++;
			*destCharPntr++ = *sourceCharPntr++;
			*destCharPntr++ = *sourceCharPntr++;
			*destCharPntr++ = *sourceCharPntr++;
			*destCharPntr++ = *sourceCharPntr++;
			*destCharPntr++ = *sourceCharPntr++;
		}
		if (numBytes & 8)
		{
			*destCharPntr++ = *sourceCharPntr++;
			*destCharPntr++ = *sourceCharPntr++;
			*destCharPntr++ = *sourceCharPntr++;
			*destCharPntr++ = *sourceCharPntr++;
			*destCharPntr++ = *sourceCharPntr++;
			*destCharPntr++ = *sourceCharPntr++;
			*destCharPntr++ = *sourceCharPntr++;
			*destCharPntr++ = *sourceCharPntr++;
		}
		if (numBytes & 4)
		{
			*destCharPntr++ = *sourceCharPntr++;
			*destCharPntr++ = *sourceCharPntr++;
			*destCharPntr++ = *sourceCharPntr++;
			*destCharPntr++ = *sourceCharPntr++;
		}
		if (numBytes & 2)
		{
			*destCharPntr++ = *sourceCharPntr++;
			*destCharPntr++ = *sourceCharPntr++;
		}
		if (numBytes & 1)
		{
			*destCharPntr = *sourceCharPntr;
		}
		
		return dest;
	}
	#endif //WASM_STD_USE_BUILTIN_MEMCPY
}

void* _memmove(void* dest, const void* source, size_t numBytes)
{
	#if WASM_STD_USE_BUILTIN_MEMMOVE
	return __builtin_memmove(dest, source, numBytes);
	#else
	char* destCharPntr = dest;
	const char* sourceCharPntr = source;
	
	if (destCharPntr == sourceCharPntr) { return destCharPntr; }
	if ((uintptr_t)sourceCharPntr - (uintptr_t)destCharPntr - numBytes <= -2 * numBytes)
	{
		return memcpy(destCharPntr, sourceCharPntr, numBytes);
	}
	
	if (destCharPntr < sourceCharPntr)
	{
		if ((uintptr_t)sourceCharPntr % SIZEOF_A_SIZE_T == (uintptr_t)destCharPntr % SIZEOF_A_SIZE_T)
		{
			while ((uintptr_t)destCharPntr % SIZEOF_A_SIZE_T)
			{
				if (!numBytes--) { return dest; }
				*destCharPntr++ = *sourceCharPntr++;
			}
			for (; numBytes >= SIZEOF_A_SIZE_T; numBytes -= SIZEOF_A_SIZE_T, destCharPntr += SIZEOF_A_SIZE_T, sourceCharPntr += SIZEOF_A_SIZE_T)
			{
				*(a_size_t*)destCharPntr = *(a_size_t*)sourceCharPntr;
			}
		}
		for (; numBytes; numBytes--) { *destCharPntr++ = *sourceCharPntr++; }
	}
	else
	{
		if (((uintptr_t)sourceCharPntr % SIZEOF_A_SIZE_T) == ((uintptr_t)destCharPntr % SIZEOF_A_SIZE_T))
		{
			while ((uintptr_t)(destCharPntr + numBytes) % SIZEOF_A_SIZE_T)
			{
				if (!numBytes--) { return dest; }
				destCharPntr[numBytes] = sourceCharPntr[numBytes];
			}
			while (numBytes >= SIZEOF_A_SIZE_T)
			{
				numBytes -= SIZEOF_A_SIZE_T;
				*(a_size_t*)(destCharPntr+numBytes) = *(a_size_t*)(sourceCharPntr + numBytes);
			}
		}
		while (numBytes)
		{
			numBytes--;
			destCharPntr[numBytes] = sourceCharPntr[numBytes];
		}
	}
	
	return dest;
	#endif //WASM_STD_USE_BUILTIN_MEMMOVE
}

int strcmp(const char* left, const char* right)
{
	//Modified implementation from Musl Lib-C
	for (; *left == *right && *left; left++, right++);
	return (*(unsigned char*)left - *(unsigned char*)right);
}

char* strcpy(char* dest, const char* source)
{
	a_size_t* destWord;
	const a_size_t* sourceWord;
	if ((uintptr_t)source % sizeof(size_t) == (uintptr_t)dest % sizeof(size_t))
	{
		for (; (uintptr_t)source % sizeof(size_t); source++, dest++)
		{
			if (!(*dest=*source)) { return dest; }
		}
		destWord = (void*)dest;
		sourceWord = (const void*)source;
		for (; !WORD_CONTAINS_ZERO(*sourceWord); *destWord++ = *sourceWord++) { }
		dest=(void*)destWord;
		source=(const void*)sourceWord;
	}
	for (; (*dest=*source); source++, dest++) { }

	return dest;
}

//TODO: Add strstr implementation!

int strncmp(const char* left, const char* right, size_t numBytes)
{
	//Modified implementation from Musl Lib-C
	const unsigned char* leftPntr = (void*)left;
	const unsigned char* rightPntr = (void*)right;
	if (!numBytes--) { return 0; }
	for (; (*leftPntr && *rightPntr && numBytes && *leftPntr == *rightPntr); leftPntr++, rightPntr++, numBytes--);
	return *leftPntr - *rightPntr;
}

size_t strlen(const char* str)
{
	//Modified implementation from Musl Lib-C
	const char* startPntr = str;
	const a_size_t* wordPntr;
	//Walk 1 character at a time until we are size_t aligned
	for (; ((uintptr_t)str % sizeof(size_t)) != 0; str++)
	{
		if (!*str) { return str-startPntr; }
	}
	//Check size_t bytes at once with WORD_CONTAINS_ZERO
	for (wordPntr = (const void *)str; !WORD_CONTAINS_ZERO(*wordPntr); wordPntr++) { }
	str = (const void *)wordPntr;
	//Walk 0-3 bytes until we find the 0 in the word
	for (; *str; str++) { }
	return (str - startPntr);
}
