/*
File:   tools_str_array.h
Author: Taylor Robbins
Date:   06\20\2025
*/

#ifndef _TOOLS_STR_ARRAY_H
#define _TOOLS_STR_ARRAY_H

typedef plex StrArray StrArray;
plex StrArray
{
	uxx length;
	uxx allocLength;
	Str8* strings;
};

void FreeStrArray(StrArray* array)
{
	for (uxx sIndex = 0; sIndex < array->length; sIndex++)
	{
		if (array->strings[sIndex].chars != nullptr) { free(array->strings[sIndex].chars); }
	}
	if (array->strings != nullptr) { free(array->strings); array->strings = nullptr; }
	array->length = 0;
	array->allocLength = 0;
}

Str8* AddStr(StrArray* array, Str8 newString)
{
	if (array->length >= array->allocLength)
	{
		uxx newAllocLength = array->allocLength;
		if (newAllocLength < 8) { newAllocLength = 8; }
		else { newAllocLength = newAllocLength*2; }
		Str8* newAllocSpace = (Str8*)malloc(sizeof(Str8) * newAllocLength);
		if (array->length > 0) { memcpy(newAllocSpace, array->strings, sizeof(Str8) * array->length); }
		if (array->strings != nullptr) { free(array->strings); }
		array->strings = newAllocSpace;
		array->allocLength = newAllocLength;
	}
	
	Str8* result = &array->strings[array->length];
	array->length++;
	*result = CopyStr8(newString, false);
	return result;
}

Str8* InsertStr(StrArray* array, Str8 newString, uxx insertIndex)
{
	Str8 strAtEnd = *AddStr(array, newString);
	if (insertIndex < array->length)
	{
		memmove(&array->strings[insertIndex+1], &array->strings[insertIndex], (array->length-1) - insertIndex);
		array->strings[insertIndex] = strAtEnd;
	}
	return &array->strings[insertIndex];
}

void RemoveStrAtIndex(StrArray* array, uxx index)
{
	assert(index < array->length);
	if (array->strings[index].chars != nullptr) { free(array->strings[index].chars); }
	if (index < array->length-1)
	{
		memmove(&array->strings[index], &array->strings[index+1], sizeof(Str8) * (array->length - (index+1)));
	}
	array->length--;
}

uxx FindStr(const StrArray* array, Str8 targetStr)
{
	for (uxx sIndex = 0; sIndex < array->length; sIndex++)
	{
		if (StrExactEquals(array->strings[sIndex], targetStr))
		{
			return sIndex;
		}
	}
	return array->length;
}
bool ContainsStr(const StrArray* array, Str8 targetStr)
{
	return (FindStr(array, targetStr) < array->length);
}

bool RemoveStr(StrArray* array, Str8 targetStr)
{
	uxx index = FindStr(array, targetStr);
	if (index >= array->length) { return false; }
	else
	{
		RemoveStrAtIndex(array, index);
		return true;
	}
}

#endif //  _TOOLS_STR_ARRAY_H
