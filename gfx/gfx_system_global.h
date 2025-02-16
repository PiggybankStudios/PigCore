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

#if BUILD_WITH_SOKOL_GFX

//TODO: Change __declspec stuff to work when compiling with Clang!
#if !PIG_CORE_IMPLEMENTATION
extern __declspec(dllimport) GfxSystem gfx;
#else
__declspec(dllexport) GfxSystem gfx = ZEROED;
#endif

//NOTE: You should call InitGfxSystem(arena, &gfx) like normal, no implicit alias is made for this function or FreeGfxSystem

#define FlushPipelineGen()  GfxSystem_FlushPipelineGen(&gfx)
#define FlushBindings()     GfxSystem_FlushBindings(&gfx)

#define BeginFrame(swapchain, screenSize, clearColor, clearDepth) GfxSystem_BeginFrame(&gfx, (swapchain), (screenSize), (clearColor), (clearDepth))
#define EndFrame()                                                GfxSystem_EndFrame(&gfx)

#define DrawVerticesEx(startVertexOrIndex, numVerticesOrIndices)  GfxSystem_DrawVerticesEx(&gfx, (startVertexOrIndex), (numVerticesOrIndices))
#define DrawVertices()                                            GfxSystem_DrawVertices(&gfx)

#define BindShader(shaderPntr)                        GfxSystem_BindShader(&gfx, (shaderPntr))
#define BindVertBuffer(bufferPntr)                    GfxSystem_BindVertBuffer(&gfx, (bufferPntr))
#define BindTextureAtIndex(texturePntr, textureIndex) GfxSystem_BindTextureAtIndex(&gfx, (texturePntr), (textureIndex))
#define BindTexture(texturePntr)                      GfxSystem_BindTexture(&gfx, (texturePntr))

#define BindFontEx(fontPntr, fontSize, fontStyleFlags) GfxSystem_BindFontEx(&gfx, (fontPntr), (fontSize), (fontStyleFlags))
#define BindFontAtSize(fontPntr, fontSize)             GfxSystem_BindFontAtSize(&gfx, (fontPntr), (fontSize))
#define BindFont(fontPntr)                             GfxSystem_BindFont(&gfx, (fontPntr))

#define SetClipRec(clipRec)                      GfxSystem_SetClipRec(&gfx, (clipRec))
#define DisableClipRec()                         GfxSystem_DisableClipRec(&gfx)
#define SetDepthTestEnabled(depthTestEnabled)    GfxSystem_SetDepthTestEnabled(&gfx, (depthTestEnabled))
#define SetDepthWriteEnabled(depthWriteEnabled)  GfxSystem_SetDepthWriteEnabled(&gfx, (depthWriteEnabled))
#define SetColorWriteEnabled(colorWriteEnabled)  GfxSystem_SetColorWriteEnabled(&gfx, (colorWriteEnabled))
#define SetCullingEnabled(cullingEnabled)        GfxSystem_SetCullingEnabled(&gfx, (cullingEnabled))

#define SetDepth(depth)  GfxSystem_SetDepth(&gfx, (depth))

#define SetProjectionMat(matrix)    GfxSystem_SetProjectionMat(&gfx, (matrix))
#define SetViewMat(matrix)          GfxSystem_SetViewMat(&gfx, (matrix))
#define SetWorldMat(matrix)         GfxSystem_SetWorldMat(&gfx, (matrix))
#define SetTintColorRaw(colorVec)   GfxSystem_SetTintColorRaw(&gfx, (colorVec))
#define SetTintColor(color)         GfxSystem_SetTintColor(&gfx, (color))
#define SetSourceRecRaw(rectangle)  GfxSystem_SetSourceRecRaw(&gfx, (rectangle))
#define SetSourceRec(rectangle)     GfxSystem_SetSourceRec(&gfx, (rectangle))

#define DrawTexturedRectangleEx(rectangle, color, texture, sourceRec)      GfxSystem_DrawTexturedRectangleEx(&gfx, (rectangle), (color), (texture), (sourceRec))
#define DrawTexturedRectangle(rectangle, color, texture)                   GfxSystem_DrawTexturedRectangle(&gfx, (rectangle), (color), (texture))
#define DrawRectangle(rectangle, color)                                    GfxSystem_DrawRectangle(&gfx, (rectangle), (color))
#define DrawRectangleOutlineEx(rectangle, borderThickness, color, outside) GfxSystem_DrawRectangleOutlineEx(&gfx, (rectangle), (borderThickness), (color), (outside))
#define DrawRectangleOutline(rectangle, borderThickness, color)            GfxSystem_DrawRectangleOutline(&gfx, (rectangle), (borderThickness), (color))
#define DrawTexturedObb2Ex(boundingBox, color, texture, sourceRec)         GfxSystem_DrawTexturedObb2Ex(&gfx, (boundingBox), (color), (texture), (sourceRec))
#define DrawTexturedObb2(boundingBox, color, texture)                      GfxSystem_DrawTexturedObb2(&gfx, (boundingBox), (color), (texture))
#define DrawObb2(boundingBox, color)                                       GfxSystem_DrawObb2(&gfx, (boundingBox), (color))

#define ClearDepthBuffer(clearDepth)  GfxSystem_ClearDepthBuffer(&gfx, (clearDepth))

//NOTE: Windows.h defines a DrawText define but we don't really need it so we #undef it here
#if TARGET_IS_WINDOWS && defined(DrawText)
#undef DrawText
#endif

#define DrawTextAtSize(fontSize, text, position, color) GfxSystem_DrawTextAtSize(&gfx, (fontSize), (text), (position), (color))
#define DrawTextBold(text, position, color)             GfxSystem_DrawTextBold(&gfx, (text), (position), (color))
#define DrawTextItalic(text, position, color)           GfxSystem_DrawTextItalic(&gfx, (text), (position), (color))
#define DrawText(text, position, color)                 GfxSystem_DrawText(&gfx, (text), (position), (color))

#endif //BUILD_WITH_SOKOL_GFX

#endif //  _GFX_SYSTEM_GLOBAL_H
