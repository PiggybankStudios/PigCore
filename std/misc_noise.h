/*
File:   misc_noise.h
Author: Taylor Robbins
Date:   01\15\2025
*/

#ifndef _MISC_NOISE_H
#define _MISC_NOISE_H

#include "base/base_typedefs.h"
#include "struct/struct_vectors.h"
#include "std/std_trig.h"
#include "std/std_basic_math.h"
#include "misc/misc_random.h"

//TODO: Enable me and fix me up once we have the rec type!
#if 0
v2 GetBlueNoisePosInRec(u64 seed, rec bounds, u64 numOfPositions, u64 index)
{
	Assert(numOfPositions > 0);
	Assert(numOfPositions <= 0x7FFFFFFF); //TODO: Honestly this just breaks down for really large numbers because numRows*numCols can be bigger than u32 max
	if (bounds.width == 0 || bounds.height == 0) { return bounds.topLeft; }
	if (index >= numOfPositions) { index = (index % numOfPositions); }
	RandomSeries_t tempSeries;
	CreateRandomSeries(&tempSeries);
	SeedRandomSeriesU64(&tempSeries, seed);
	StepRandomSeries(&tempSeries, index*2);
	
	r32 aspectRatio = bounds.width / bounds.height;
	v2i arraySize = Vec2i_Zero;
	if (aspectRatio >= 1.0f) //wide bounds
	{
		u32 numRows = (u32)FloorR32i(SqrtR32((r32)numOfPositions));
		if (numRows < 1) { numRows = 1; }
		while (numRows > 1 && (((r32)numOfPositions / (r32)(numRows-1)) / (r32)(numRows-1)) < aspectRatio)
		{
			numRows--;
		}
		arraySize = NewVec2i(CeilR32i((r32)numOfPositions / (r32)numRows), (i32)numRows);
	}
	else //tall bounds
	{
		u32 numColumns = (u32)FloorR32i(SqrtR32((r32)numOfPositions));
		if (numColumns < 1) { numColumns = 1; }
		while (numColumns > 1 && ((r32)(numColumns-1) / ((r32)numOfPositions / (r32)(numColumns-1))) > aspectRatio)
		{
			numColumns--;
		}
		arraySize = NewVec2i((i32)numColumns, CeilR32i((r32)numOfPositions / (r32)numColumns));
	}
	
	u32 actualIndex = (u32)index;
	i32 numEmptySpots = (arraySize.width * arraySize.height) - (i32)numOfPositions;
	if (numEmptySpots > 0)
	{
		i32 skipPeriod = CeilR32i((r32)(arraySize.width * arraySize.height) / (r32)numEmptySpots);
		// if (index == 0) { MyLibPrintLine_D("%d emptySpots, %d skipPeriod", numEmptySpots, skipPeriod); }
		if (skipPeriod > 0) { actualIndex += (u32)((index + skipPeriod/2) / skipPeriod); }
	}
	
	// MyLibPrintLine_D("Array Size for %.1fx%.1f box and %u points: (%d, %d)", bounds.width, bounds.height, numOfPositions, arraySize.width, arraySize.height);
	// return bounds.topLeft + bounds.size/2;
	
	v2 cellSize = Vec2Divide(bounds.size, ToVec2(arraySize));
	v2i gridPos = NewVec2i((i32)(actualIndex % arraySize.width), (i32)(actualIndex / arraySize.width));
	v2 offset = Vec2Multiply(ToVec2(gridPos), cellSize) + cellSize/2;
	
	v2 jitter = NewVec2(GetRandR32(&tempSeries)*2 - 1.0f, GetRandR32(&tempSeries)*2 - 0.5f);
	jitter = Vec2Multiply(jitter, cellSize/4);
	offset += jitter;
	
	return bounds.topLeft + offset;
}
#endif

#endif //  _MISC_NOISE_H
