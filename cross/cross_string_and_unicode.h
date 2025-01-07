/*
File:   cross_string_and_unicode.h
Author: Taylor Robbins
Date:   01\06\2025
Description:
	** Extensions to the Unicode and Str8 APIs that use both together
*/

#ifndef _CROSS_STRING_AND_UNICODE_H
#define _CROSS_STRING_AND_UNICODE_H

//NOTE: Intentionally no includes here

u8 GetCodepointForUtf8Str(Str8 str, uxx index, u32* codepointOut)
{
	Assert(index <= str.length);
	return GetCodepointForUtf8(str.length - index, str.chars + index, codepointOut);
}

bool DoesStrContainMultibyteUtf8Characters(Str8 str)
{
	for (uxx bIndex = 0; bIndex < str.length; bIndex++)
	{
		if (GetCodepointForUtf8Str(str, bIndex, nullptr) > 0) { return true; }
	}
	return false;
}

#endif //  _CROSS_STRING_AND_UNICODE_H
