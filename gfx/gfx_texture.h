/*
File:   gfx_texture.h
Author: Taylor Robbins
Date:   01\30\2025
*/

#ifndef _GFX_TEXTURE_H
#define _GFX_TEXTURE_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "struct/struct_vectors.h"
#include "mem/mem_arena.h"
#include "mem/mem_scratch.h"
#include "struct/struct_string.h"
#include "misc/misc_result.h"
#include "misc/misc_sokol_gfx_include.h"

#if BUILD_WITH_SOKOL_GFX

#ifndef SOKOL_GFX_INCLUDED
#error Somehow sokol_gfx.h was not included properly before gfx_texture.h!
#endif

typedef enum TextureFlag TextureFlag;
enum TextureFlag
{
	TextureFlag_None = 0x00,
	TextureFlag_IsPixelated   = 0x01,
	TextureFlag_IsRepeating   = 0x02,
	TextureFlag_NoAlpha       = 0x04,
	TextureFlag_IsHdr         = 0x08,
	TextureFlag_SingleChannel = 0x10,
	TextureFlag_HasCopy       = 0x20,
	TextureFlag_NoMipmaps     = 0x40,
	TextureFlag_All = 0x3F,
};
#if !PIG_CORE_IMPLEMENTATION
const char* GetTextureFlagStr(TextureFlag enumValue);
#else
const char* GetTextureFlagStr(TextureFlag enumValue)
{
	switch (enumValue)
	{
		case TextureFlag_None:          return "None";
		case TextureFlag_IsPixelated:   return "IsPixelated";
		case TextureFlag_IsRepeating:   return "IsRepeating";
		case TextureFlag_NoAlpha:       return "HasAlpha";
		case TextureFlag_IsHdr:         return "IsHdr";
		case TextureFlag_SingleChannel: return "SingleChannel";
		case TextureFlag_HasCopy:       return "HasCopy";
		case TextureFlag_NoMipmaps:     return "NoMipmaps";
		default: return UNKNOWN_STR;
	}
}
#endif

typedef struct Texture Texture;
struct Texture
{
	Arena* arena;
	Result error;
	sg_image image;
	sg_sampler sampler;
	u8 flags;
	union
	{
		v2i size;
		struct { i32 Width, Height; };
	};
	uxx numPixels;
	uxx pixelSize;
	uxx totalSize;
	union { void* pixelsPntr; u8* pixelsU8; u32* pixelsU32; r32 pixelsR32; };
	Str8 name;
	#if DEBUG_BUILD
	Str8 filePath;
	#endif
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	void FreeTexture(Texture* texture);
	Texture InitTexture(Arena* arena, Str8 name, v2i size, const void* pixelsPntr, u8 flags);
 	PIG_CORE_INLINE void SetTextureFilePath(Texture* texture, Str8 filePath);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP void FreeTexture(Texture* texture)
{
	NotNull(texture);
	if (texture->arena != nullptr)
	{
		sg_destroy_image(texture->image);
		sg_destroy_sampler(texture->sampler);
		FreeStr8(texture->arena, &texture->name);
		#if DEBUG_BUILD
		FreeStr8(texture->arena, &texture->filePath);
		#endif
		if (texture->pixelsPntr != nullptr) { FreeMem(texture->arena, texture->pixelsPntr, texture->totalSize); }
	}
	ClearPointer(texture);
}

PEXP Texture InitTexture(Arena* arena, Str8 name, v2i size, const void* pixelsPntr, u8 flags)
{
	NotNull(arena);
	NotNullStr(name);
	Assert(size.Width > 0 && size.Height > 0);
	NotNull(pixelsPntr);
	ScratchBegin1(scratch, arena);
	
	Texture result = ZEROED;
	result.arena = arena;
	result.size = size;
	result.flags = flags;
	
	if (IsFlagSet(flags, TextureFlag_NoAlpha) && !IsFlagSet(flags, TextureFlag_SingleChannel))
	{
		uxx inputPixelSize = IsFlagSet(flags, TextureFlag_IsHdr) ? sizeof(r32)*3 : sizeof(u8)*3;
		uxx inputTotalSize = inputPixelSize * size.Width * size.Height;
		uxx alphaChannelSize = IsFlagSet(flags, TextureFlag_IsHdr) ? sizeof(r32) : sizeof(u8);
		uxx newInputSize = inputTotalSize + (alphaChannelSize * size.Width * size.Height);
		u8* newPixels = AllocArray(u8, scratch, newInputSize);
		if (newPixels == nullptr) { result.error = Result_FailedToAllocateMemory; ScratchEnd(scratch); return result; }
		const u8* readPntr = (const u8*)pixelsPntr;
		u8* writePntr = newPixels;
		for (uxx yIndex = 0; yIndex < (uxx)size.Height; yIndex++)
		{
			for (uxx xIndex = 0; xIndex < (uxx)size.Width; xIndex++)
			{
				MyMemCopy(writePntr, readPntr, inputPixelSize);
				readPntr += inputPixelSize;
				writePntr += inputPixelSize;
				if (IsFlagSet(flags, TextureFlag_IsHdr)) { *((r32*)writePntr) = 1.0f; }
				else { *((u8*)writePntr) = 255; }
				writePntr += alphaChannelSize;
			}
		}
		pixelsPntr = newPixels;
	}
	
	result.numPixels = (uxx)(size.Width * size.Height);
	result.pixelSize = IsFlagSet(flags, TextureFlag_IsHdr)
		? (IsFlagSet(flags, TextureFlag_SingleChannel) ? sizeof(r32)*1 : sizeof(r32)*4)
		: (IsFlagSet(flags, TextureFlag_SingleChannel) ? sizeof(u8)*1 : sizeof(u8)*4);
	result.totalSize = result.numPixels * result.pixelSize;
	
	if (IsFlagSet(flags, TextureFlag_HasCopy))
	{
		result.pixelsU8 = AllocArray(u8, arena, result.totalSize);
		if (result.pixelsU8 == nullptr)
		{
			result.error = Result_FailedToAllocateMemory;
			ScratchEnd(scratch);
			return result;
		}
	}
	
	if (!IsEmptyStr(name)) { result.name = AllocStr8(arena, name); NotNull(result.name.chars); }
	
	//TODO: How do we handle creating mipmaps when !IsFlagSet(flags, TextureFlag_NoMipmaps)
	
	sg_image_desc imageDesc = ZEROED;
	imageDesc.type = SG_IMAGETYPE_2D;
	imageDesc.usage = SG_USAGE_IMMUTABLE;
	imageDesc.width = size.Width;
	imageDesc.height = size.Height;
	imageDesc.num_mipmaps = 1; //TODO: Will change once we generate mipmaps
	imageDesc.pixel_format = IsFlagSet(flags, TextureFlag_IsHdr)
		? (IsFlagSet(flags, TextureFlag_SingleChannel) ? SG_PIXELFORMAT_R32F : SG_PIXELFORMAT_RGBA32F)
		: (IsFlagSet(flags, TextureFlag_SingleChannel) ? SG_PIXELFORMAT_R8 : SG_PIXELFORMAT_RGBA8);
	imageDesc.data.subimage[0][0] = (sg_range){ pixelsPntr, result.totalSize };
	Str8 nameNt = AllocStrAndCopy(scratch, name.length, name.chars, true); NotNull(nameNt.chars); //allocate to ensure null-term char
	imageDesc.label = nameNt.chars;
	
	result.image = sg_make_image(&imageDesc);
	if (result.image.id == SG_INVALID_ID)
	{
		FreeStr8(arena, &result.name);
		if (result.pixelsPntr != nullptr) { FreeMem(arena, result.pixelsPntr, result.totalSize); }
		result.error = Result_SokolError;
		ScratchEnd(scratch);
		return result;
	}
	
	sg_sampler_desc samplerDesc = ZEROED;
	Str8 sampleNameNt = JoinStringsInArena(scratch, name, StrLit("_sampler"), true); NotNull(sampleNameNt.chars);
	samplerDesc.label = sampleNameNt.chars;
	samplerDesc.min_filter = IsFlagSet(flags, TextureFlag_IsPixelated) ? SG_FILTER_NEAREST : SG_FILTER_LINEAR;
	samplerDesc.mag_filter = IsFlagSet(flags, TextureFlag_IsPixelated) ? SG_FILTER_NEAREST : SG_FILTER_LINEAR;
	samplerDesc.mipmap_filter = IsFlagSet(flags, TextureFlag_IsPixelated) ? SG_FILTER_NEAREST : SG_FILTER_LINEAR;
	samplerDesc.wrap_u = IsFlagSet(flags, TextureFlag_IsRepeating) ? SG_WRAP_REPEAT : SG_WRAP_CLAMP_TO_EDGE;
	samplerDesc.wrap_v = IsFlagSet(flags, TextureFlag_IsRepeating) ? SG_WRAP_REPEAT : SG_WRAP_CLAMP_TO_EDGE;
	// samplerDesc.wrap_w = IsFlagSet(flags, TextureFlag_IsRepeating) ? SG_WRAP_REPEAT : SG_WRAP_CLAMP_TO_EDGE;
	
	result.sampler = sg_make_sampler(&samplerDesc);
	if (result.sampler.id == SG_INVALID_ID)
	{
		sg_destroy_image(result.image);
		FreeStr8(arena, &result.name);
		if (result.pixelsPntr != nullptr) { FreeMem(arena, result.pixelsPntr, result.totalSize); }
		result.error = Result_SokolError;
		ScratchEnd(scratch);
		return result;
	}
	
	ScratchEnd(scratch);
	result.error = Result_Success;
	return result;
}

PEXPI void SetTextureFilePath(Texture* texture, Str8 filePath)
{
	NotNull(texture);
	NotNull(texture->arena);
	NotNullStr(filePath);
	#if DEBUG_BUILD
	FreeStr8(texture->arena, &texture->filePath);
	if (!IsEmptyStr(filePath))
	{
		texture->filePath = AllocStr8(texture->arena, filePath);
		NotNull(texture->filePath.chars);
	}
	#else
	UNUSED(texture);
	UNUSED(filePath);
	#endif
}

PEXPI void BindTexture(sg_bindings* bindings, Texture* texture, uxx textureIndex)
{
	NotNull(bindings);
	NotNull(texture);
	Assert(texture->image.id != SG_INVALID_ID);
	Assert(texture->sampler.id != SG_INVALID_ID);
	bindings->images[textureIndex] = texture->image;
	bindings->samplers[textureIndex] = texture->sampler;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_SOKOL_GFX

#endif //  _GFX_TEXTURE_H

#if defined(_GFX_SHADER_H) && defined(_GFX_TEXTURE_H)
#include "cross/cross_shader_and_texture.h"
#endif
