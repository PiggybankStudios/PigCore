/*
File:   struct_color.h
Author: Taylor Robbins
Date:   01\05\2025
Description:
	** Holds the Color struct which is a 32-bit value that has 4 8-bit channels
	** The order of the channels 
*/

#ifndef _STRUCT_COLOR_H
#define _STRUCT_COLOR_H

typedef struct Color Color;
struct Color
{
	u32 valueU32;
	struct { u8 channel[4]; };
	struct { u8 b, g, r, a; };
	struct { u8 blue, green, red, alpha; };
};

//NOTE: This is BGRA order in memory, or in hex that's 0xAARRGGBB
Color NewColorU32(u32 valueU32)
{
	Color result;
	result.valueU32 = valueU32;
	return result;
}
Color NewColor(u8 red, u8 green, u8 blue, u8 alpha)
{
	Color result;
	result.red = red;
	result.green = green;
	result.blue = blue;
	result.alpha = alpha;
	return result;
}

#endif //  _STRUCT_COLOR_H
