/*
File:   gfx_system_global.h
Author: Taylor Robbins
Date:   01\30\2025
Description: 
	** This file defines a single global GfxSystem named "gfx" and makes a bunch
	** of macros that implicitly reference that global when calling GfxSystem functions
	** NOTE: This file is NOT included in gfx_all.h and is explitly opt-in since not all
	** applications want a non-thread-local global variable tied in to all their rendering calls
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

#define FlushPipelineGen()                        FlushSystemPipelineGen(&gfx)
#define FlushBindings()                           FlushSystemBindings(&gfx)

#define BeginFrame(clearColor, clearDepth)        BeginSystemFrame(&gfx, (clearColor), (clearDepth))
#define EndFrame()                                EndSystemFrame(&gfx)

#define DrawVerticesEx(startVertex, numVertices)  DrawSystemVerticesEx(&gfx, (startVertex), (numVertices))
#define DrawVertices()                            DrawSystemVertices(&gfx)

#define BindShader(shaderPntr)                    BindSystemShader(&gfx, (shaderPntr))
#define BindVertBuffer(bufferPntr)                BindSystemVertBuffer(&gfx, (bufferPntr))
#define BindTexture(texturePntr)                  BindSystemTexture(&gfx, (texturePntr))

#define SetProjectionMat(matrix)                  SetSystemProjectionMat(&gfx, (matrix))
#define SetViewMat(matrix)                        SetSystemViewMat(&gfx, (matrix))
#define SetWorldMat(matrix)                       SetSystemWorldMat(&gfx, (matrix))
#define SetTintColorRaw(colorVec)                 SetSystemTintColorRaw(&gfx, (colorVec))
#define SetTintColor(color)                       SetSystemTintColor(&gfx, (color))
#define SetSourceRecRaw(rectangle)                SetSystemSourceRecRaw(&gfx, (rectangle))
#define SetSourceRec(rectangle)                   SetSystemSourceRec(&gfx, (rectangle))

#endif //BUILD_WITH_SOKOL

#endif //  _GFX_SYSTEM_GLOBAL_H
