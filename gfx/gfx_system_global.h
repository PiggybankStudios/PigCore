/*
File:   gfx_system_global.h
Author: Taylor Robbins
Date:   01\30\2025
Description: 
	** This file defines a single global GfxSystem named "gfx" and makes shortened
	** alias macros that implicitly pass this global gfx as the first parameter
	** This might seem simply and possibly ugly but it allows for very smooth flow when
	** doing single-threaded rendering in an application. We don't have to keep typing
	** &gfx all over the place. We also basically never have more than one GfxSystem
	** instance so the only case where we don't want this file is when we are doing
	** threaded rendering or we want to be more careful about globals for some reason.
	**
	** NOTE: This file is NOT included in gfx_all.h and is explitly opt-in since not all
	** applications want a non-thread-local global variable tied in to all their rendering calls.
	** This file can be included in PigCore.dll if PIG_CORE_DLL_INCLUDE_GFX_SYSTEM_GLOBAL is enabled
Example:
Without this file:
	BeginSystemFrame(&gfx, MonokaiBack, 1.0f);
	{
		BindSystemShader(&gfx, &main2dShader);
		BindSystemTexture(&gfx, &gradientTexture);
		SetSystemProjectionMat(&gfx, MakeScaleYMat4(-1.0f));
		SetSystemViewMat(&gfx, Mat4_Identity);
		SetSystemSourceRec(&gfx, NewV4(0, 0, (r32)gradientTexture.Width, (r32)gradientTexture.Height));
		mat4 worldMat = Mat4_Identity;
		TransformMat4(&worldMat, MakeScaleXYZMat4(recSize.Width, recSize.Height, 1.0f));
		TransformMat4(&worldMat, MakeTranslateXYZMat4(recPos.X, recPos.Y, 0.0f));
		SetSystemWorldMat(&gfx, worldMat);
		SetSystemTintColor(&gfx, color);
		BindSystemVertBuffer(&gfx, &squareBuffer);
		DrawSystemVertices(&gfx, );
	}
	EndSystemFrame(&gfx, );
With this file:
	BeginFrame(MonokaiBack, 1.0f);
	{
		BindShader(&main2dShader);
		BindTexture(&gradientTexture);
		SetProjectionMat(MakeScaleYMat4(-1.0f));
		SetViewMat(Mat4_Identity);
		SetSourceRec(NewV4(0, 0, (r32)gradientTexture.Width, (r32)gradientTexture.Height));
		mat4 worldMat = Mat4_Identity;
		TransformMat4(&worldMat, MakeScaleXYZMat4(recSize.Width, recSize.Height, 1.0f));
		TransformMat4(&worldMat, MakeTranslateXYZMat4(recPos.X, recPos.Y, 0.0f));
		SetWorldMat(worldMat);
		SetTintColor(color);
		BindVertBuffer(&squareBuffer);
		DrawVertices();
	}
	EndFrame();
*/

#ifndef _GFX_SYSTEM_GLOBAL_H
#define _GFX_SYSTEM_GLOBAL_H

#include "base/base_defines_check.h"
#include "gfx/gfx_system.h"

#if BUILD_WITH_SOKOL

#if !PIG_CORE_IMPLEMENTATION
extern GfxSystem gfx;
#else
GfxSystem gfx = ZEROED;
#endif

//NOTE: You should call InitGfxSystem(arena, &gfx) like normal, no implicit alias is made for this function or FreeGfxSystem

#define FlushPipelineGen()                            FlushSystemPipelineGen(&gfx)
#define FlushBindings()                               FlushSystemBindings(&gfx)

#define BeginFrame(swapchain, clearColor, clearDepth) BeginSystemFrame(&gfx, (swapchain), (clearColor), (clearDepth))
#define EndFrame()                                    EndSystemFrame(&gfx)

#define DrawVerticesEx(startVertex, numVertices)      DrawSystemVerticesEx(&gfx, (startVertex), (numVertices))
#define DrawVertices()                                DrawSystemVertices(&gfx)

#define BindShader(shaderPntr)                        BindSystemShader(&gfx, (shaderPntr))
#define BindVertBuffer(bufferPntr)                    BindSystemVertBuffer(&gfx, (bufferPntr))
#define BindTexture(texturePntr)                      BindSystemTexture(&gfx, (texturePntr))

#define SetProjectionMat(matrix)                      SetSystemProjectionMat(&gfx, (matrix))
#define SetViewMat(matrix)                            SetSystemViewMat(&gfx, (matrix))
#define SetWorldMat(matrix)                           SetSystemWorldMat(&gfx, (matrix))
#define SetTintColorRaw(colorVec)                     SetSystemTintColorRaw(&gfx, (colorVec))
#define SetTintColor(color)                           SetSystemTintColor(&gfx, (color))
#define SetSourceRecRaw(rectangle)                    SetSystemSourceRecRaw(&gfx, (rectangle))
#define SetSourceRec(rectangle)                       SetSystemSourceRec(&gfx, (rectangle))

#endif //BUILD_WITH_SOKOL

#endif //  _GFX_SYSTEM_GLOBAL_H
