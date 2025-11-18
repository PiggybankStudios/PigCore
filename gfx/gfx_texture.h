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
#include "lib/lib_sokol_gfx.h"
#include "lib/lib_tracy.h"

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
	TextureFlag_Mutable       = 0x80,
	TextureFlag_All = 0xFF,
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
		case TextureFlag_Mutable:       return "Mutable";
		default: return UNKNOWN_STR;
	}
}
#endif

typedef plex Texture Texture;
plex Texture
{
	Arena* arena;
	Result error;
	sg_image image;
	sg_sampler sampler;
	u8 flags;
	car
	{
		v2i size;
		plex { i32 Width, Height; };
	};
	uxx numPixels;
	uxx pixelSize;
	uxx totalSize;
	car { void* pixelsPntr; u8* pixelsU8; u32* pixelsU32; r32 pixelsR32; };
	Str8 name; //TODO: Do we want this in release builds? Maybe DEBUG only?
	#if DEBUG_BUILD
	Str8 filePath;
	#endif
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	void FreeTexture(Texture* texture);
	ImageData GenerateMipmapLayer(Arena* arena, ImageData upperLayer);
	Texture InitTexture(Arena* arena, Str8 name, v2i size, const void* pixelsPntr, u8 flags);
 	PIG_CORE_INLINE void SetTextureFilePath(Texture* texture, Str8 filePath);
	void UpdateTexturePart(Texture* texture, reci sourceRec, const void* pixelsPntr);
	PIG_CORE_INLINE void UpdateTexture(Texture* texture, const void* pixelsPntr);
	PIG_CORE_INLINE void BindTexture(sg_bindings* bindings, Texture* texture, uxx textureIndex);
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
		TracyCZoneN(funcZone, "FreeTexture", true);
		sg_destroy_image(texture->image);
		sg_destroy_sampler(texture->sampler);
		FreeStr8(texture->arena, &texture->name);
		#if DEBUG_BUILD
		FreeStr8(texture->arena, &texture->filePath);
		#endif
		if (texture->pixelsPntr != nullptr) { FreeMem(texture->arena, texture->pixelsPntr, texture->totalSize); }
		TracyCZoneEnd(funcZone);
	}
	ClearPointer(texture);
}

//TODO: Measure performance of this mipmap generator! Possibly spend time making it faster
//NOTE: For a 1569x998 texture:
//      Desktop Gamma: 2ms for 784x499, 0.6ms for 392x249, 0.1ms for 196x124, etc. (3.6ms total)
//      Laptop Linear: 60ms for 784x499, 12ms for 392x249, 3ms for 196x124, etc. (75ms total)
//      Laptop Gamma: 1.5ms for 784x499, 0.4ms for 392x249, 0.1ms for 196x124, etc. (2ms total)
//NOTE: 
PEXP ImageData GenerateMipmapLayer(Arena* arena, ImageData upperLayer)
{
	TracyCZoneN(_funcZone, "GenerateMipmapLayer", true);
	NotNull(arena);
	Assert(upperLayer.size.Width >= 2 && upperLayer.size.Height >= 2);
	ScratchBegin1(scratch, arena);
	
	TracyCZoneN(linearConversion, "LinearConversion", true);
	uxx upperLayerNumPixels = (uxx)(upperLayer.size.Width * upperLayer.size.Height);
	Colorf* upperLayerLinear = AllocArray(Colorf, scratch, upperLayerNumPixels);
	for (uxx pIndex = 0; pIndex < upperLayerNumPixels; pIndex++)
	{
		r32 floatR = (r32)((upperLayer.pixels[pIndex] >> 16) & 0xFF) / 255.0f;
		r32 floatG = (r32)((upperLayer.pixels[pIndex] >>  8) & 0xFF) / 255.0f;
		r32 floatB = (r32)((upperLayer.pixels[pIndex] >>  0) & 0xFF) / 255.0f;
		r32 floatA = (r32)((upperLayer.pixels[pIndex] >> 24) & 0xFF) / 255.0f;
		upperLayerLinear[pIndex].R = TO_LINEAR_FROM_GAMMA_R32_FAST(floatR);
		upperLayerLinear[pIndex].G = TO_LINEAR_FROM_GAMMA_R32_FAST(floatG);
		upperLayerLinear[pIndex].B = TO_LINEAR_FROM_GAMMA_R32_FAST(floatB);
		upperLayerLinear[pIndex].A = floatA;
	}
	TracyCZoneEnd(linearConversion);
	
	ImageData result = ZEROED;
	result.size = MakeV2i(upperLayer.size.Width/2, upperLayer.size.Height/2);
	result.numPixels = (uxx)(result.size.Width * result.size.Height);
	result.pixels = AllocArray(u32, arena, result.numPixels);
	NotNull(result.pixels);
	for (i32 yOffset = 0; yOffset < result.size.Height; yOffset++)
	{
		for (i32 xOffset = 0; xOffset < result.size.Width; xOffset++)
		{
			Color32* outPixel = (Color32*)&result.pixels[INDEX_FROM_COORD2D(xOffset, yOffset, result.size.Width, result.size.Height)];
			v2i upperPos = MakeV2i(xOffset*2, yOffset*2);
			if (upperPos.X >= upperLayer.size.Width) { upperPos.X = upperLayer.size.Width-1; }
			if (upperPos.Y >= upperLayer.size.Height) { upperPos.Y = upperLayer.size.Height-1; }
			float* inRow0 = (float*)(&upperLayerLinear[INDEX_FROM_COORD2D(upperPos.X, upperPos.Y, upperLayer.size.Width, upperLayer.size.Height)]);
			float* inRow1 = (float*)(&upperLayerLinear[INDEX_FROM_COORD2D(upperPos.X, upperPos.Y+1, upperLayer.size.Width, upperLayer.size.Height)]);
			r32 floatR = (inRow0[0] + inRow0[4] + inRow1[0] + inRow1[4]) / 4.0f;
			r32 floatG = (inRow0[1] + inRow0[5] + inRow1[1] + inRow1[5]) / 4.0f;
			r32 floatB = (inRow0[2] + inRow0[6] + inRow1[2] + inRow1[6]) / 4.0f;
			r32 floatA = (inRow0[3] + inRow0[7] + inRow1[3] + inRow1[7]) / 4.0f;
			outPixel->r = (u8)(TO_GAMMA_FROM_LINEAR_R32_FAST(floatR) * 255.0f);
			outPixel->g = (u8)(TO_GAMMA_FROM_LINEAR_R32_FAST(floatG) * 255.0f);
			outPixel->b = (u8)(TO_GAMMA_FROM_LINEAR_R32_FAST(floatB) * 255.0f);
			outPixel->a = (u8)(                             (floatA) * 255.0f);
		}
	}
	
	ScratchEnd(scratch);
	TracyCZoneEnd(_funcZone);
	return result;
}

PEXP Texture InitTexture(Arena* arena, Str8 name, v2i size, const void* pixelsPntr, u8 flags)
{
	NotNull(arena);
	NotNullStr(name);
	Assert(size.Width > 0 && size.Height > 0);
	NotNull(pixelsPntr);
	TracyCZoneN(funcZone, "InitTexture", true);
	ScratchBegin1(scratch, arena);
	
	Texture result = ZEROED;
	result.arena = arena;
	result.size = size;
	result.flags = flags;
	
	if (IsFlagSet(flags, TextureFlag_NoAlpha) && !IsFlagSet(flags, TextureFlag_SingleChannel))
	{
		TracyCZoneN(_SingleChannelConversion, "SingleChannelConversion", true);
		uxx inputPixelSize = IsFlagSet(flags, TextureFlag_IsHdr) ? sizeof(r32)*3 : sizeof(u8)*3;
		uxx inputTotalSize = inputPixelSize * size.Width * size.Height;
		uxx alphaChannelSize = IsFlagSet(flags, TextureFlag_IsHdr) ? sizeof(r32) : sizeof(u8);
		uxx newInputSize = inputTotalSize + (alphaChannelSize * size.Width * size.Height);
		u8* newPixels = AllocArray(u8, scratch, newInputSize);
		if (newPixels == nullptr)
		{
			result.error = Result_FailedToAllocateMemory;
			ScratchEnd(scratch);
			TracyCZoneEnd(_SingleChannelConversion);
			TracyCZoneEnd(funcZone);
			return result;
		}
		const u8* readPntr = (const u8*)pixelsPntr;
		u8* writePntr = newPixels;
		for (uxx yIndex = 0; yIndex < (uxx)size.Height; yIndex++)
		{
			for (uxx xIndex = 0; xIndex <= (uxx)size.Width; xIndex++)
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
		TracyCZoneEnd(_SingleChannelConversion);
	}
	
	result.numPixels = (uxx)(size.Width * size.Height);
	result.pixelSize = IsFlagSet(flags, TextureFlag_IsHdr)
		? (IsFlagSet(flags, TextureFlag_SingleChannel) ? sizeof(r32)*1 : sizeof(r32)*4)
		: (IsFlagSet(flags, TextureFlag_SingleChannel) ? sizeof(u8)*1 : sizeof(u8)*4);
	result.totalSize = result.numPixels * result.pixelSize;
	
	if (IsFlagSet(flags, TextureFlag_HasCopy))
	{
		result.pixelsU8 = (u8*)AllocMem(arena, result.totalSize);
		if (result.pixelsU8 == nullptr)
		{
			result.error = Result_FailedToAllocateMemory;
			ScratchEnd(scratch);
			TracyCZoneEnd(funcZone);
			return result;
		}
		MyMemCopy(result.pixelsU8, pixelsPntr, result.totalSize);
	}
	
	if (!IsEmptyStr(name)) { result.name = AllocStr8(arena, name); NotNull(result.name.chars); }
	
	//SEE https://github.com/floooh/sokol/issues/102 AND https://github.com/Deins/sokol/tree/soft_gen_mipmaps
	sg_range pixelsRange = (sg_range){ pixelsPntr, result.totalSize };
	sg_range* mipmapRanges = nullptr;
	uxx numMipLevels = 0;
	if (!IsFlagSet(flags, TextureFlag_NoMipmaps))
	{
		ImageData baseImageData = ZEROED;
		baseImageData.size = size;
		baseImageData.numPixels = (uxx)(size.Width * size.Height);
		baseImageData.pixels = (u32*)pixelsPntr;
		
		numMipLevels = MinI32(SG_MAX_MIPMAPS-1, FloorR32i(Log2R32((r32)MinI32(size.Width, size.Height))));
		mipmapRanges = AllocArray(sg_range, scratch, numMipLevels);
		NotNull(mipmapRanges);
		ImageData* mipmapImageDatas = AllocArray(ImageData, scratch, numMipLevels);
		NotNull(mipmapImageDatas);
		for (uxx mIndex = 0; mIndex < numMipLevels; mIndex++)
		{
			ImageData upperLayer = (mIndex > 0) ? mipmapImageDatas[mIndex-1] : baseImageData;
			mipmapImageDatas[mIndex] = GenerateMipmapLayer(scratch, upperLayer);
			// PrintLine_D("Generated mipmap[%llu] %dx%d (from %dx%d)", mIndex, mipmapImageDatas[mIndex].size.Width, mipmapImageDatas[mIndex].size.Height, upperLayer.size.Width, upperLayer.size.Height);
			mipmapRanges[mIndex] = (sg_range){ mipmapImageDatas[mIndex].pixels, mipmapImageDatas[mIndex].numPixels * result.pixelSize };
		}
	}
	
	sg_image_desc imageDesc = ZEROED;
	imageDesc.type = SG_IMAGETYPE_2D;
	imageDesc.usage = IsFlagSet(flags, TextureFlag_Mutable) ? SG_USAGE_DYNAMIC : SG_USAGE_IMMUTABLE;
	imageDesc.width = size.Width;
	imageDesc.height = size.Height;
	imageDesc.num_mipmaps = (int)(1 + numMipLevels);
	imageDesc.pixel_format = IsFlagSet(flags, TextureFlag_IsHdr)
		? (IsFlagSet(flags, TextureFlag_SingleChannel) ? SG_PIXELFORMAT_R32F : SG_PIXELFORMAT_RGBA32F)
		: (IsFlagSet(flags, TextureFlag_SingleChannel) ? SG_PIXELFORMAT_R8 : SG_PIXELFORMAT_RGBA8);
	if (!IsFlagSet(flags, TextureFlag_Mutable))
	{
		imageDesc.data.subimage[0][0] = pixelsRange;
		if (!IsFlagSet(flags, TextureFlag_NoMipmaps) && numMipLevels > 0)
		{
			for (uxx mIndex = 0; mIndex < numMipLevels; mIndex++) { imageDesc.data.subimage[0][1+mIndex] = mipmapRanges[mIndex]; }
		}
	}
	Str8 nameNt = AllocStrAndCopy(scratch, name.length, name.chars, true); NotNull(nameNt.chars); //allocate to ensure null-term char
	imageDesc.label = nameNt.chars;
	
	TracyCZoneN(_MakeImage, "sg_make_image", true);
	result.image = sg_make_image(&imageDesc);
	TracyCZoneEnd(_MakeImage);
	if (result.image.id == SG_INVALID_ID)
	{
		FreeStr8(arena, &result.name);
		if (result.pixelsPntr != nullptr) { FreeMem(arena, result.pixelsPntr, result.totalSize); }
		result.error = Result_SokolError;
		ScratchEnd(scratch);
		TracyCZoneEnd(funcZone);
		return result;
	}
	
	if (IsFlagSet(flags, TextureFlag_Mutable))
	{
		sg_image_data imageData = ZEROED;
		imageData.subimage[0][0] = pixelsRange;
		if (!IsFlagSet(flags, TextureFlag_NoMipmaps) && numMipLevels > 0)
		{
			for (uxx mIndex = 0; mIndex < numMipLevels; mIndex++) { imageData.subimage[0][1+mIndex] = mipmapRanges[mIndex]; }
		}
		sg_update_image(result.image, &imageData);
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
	
	TracyCZoneN(_MakeSampler, "sg_make_sampler", true);
	result.sampler = sg_make_sampler(&samplerDesc);
	TracyCZoneEnd(_MakeSampler);
	if (result.sampler.id == SG_INVALID_ID)
	{
		sg_destroy_image(result.image);
		FreeStr8(arena, &result.name);
		if (result.pixelsPntr != nullptr) { FreeMem(arena, result.pixelsPntr, result.totalSize); }
		result.error = Result_SokolError;
		ScratchEnd(scratch);
		TracyCZoneEnd(funcZone);
		return result;
	}
	
	ScratchEnd(scratch);
	result.error = Result_Success;
	TracyCZoneEnd(funcZone);
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

PEXP void UpdateTexturePart(Texture* texture, reci sourceRec, const void* pixelsPntr)
{
	NotNull(texture);
	NotNull(texture->arena);
	NotNull(texture->pixelsPntr);
	Assert(sourceRec.Width >= 0 && sourceRec.Height >= 0);
	Assert(sourceRec.X >= 0 && sourceRec.Y >= 0);
	Assert(sourceRec.X + sourceRec.Width <= texture->Width && sourceRec.Y + sourceRec.Height <= texture->Height);
	if (sourceRec.Width == 0 || sourceRec.Height == 0) { return; }
	NotNull(pixelsPntr);
	
	ImageData newImageData = ZEROED;
	if (IsFlagSet(texture->flags, TextureFlag_HasCopy))
	{
		for (uxx rowIndex = 0; rowIndex < (uxx)sourceRec.Height; rowIndex++)
		{
			const u8* sourceRow = &((u8*)pixelsPntr)[INDEX_FROM_COORD2D(0, rowIndex, sourceRec.Width, sourceRec.Height) * texture->pixelSize];
			u8* destRow = &texture->pixelsU8[INDEX_FROM_COORD2D(sourceRec.X + 0, sourceRec.Y + rowIndex, texture->Width, texture->Height) * texture->pixelSize];
			MyMemCopy(destRow, sourceRow, sourceRec.Width * texture->pixelSize);
		}
		newImageData = MakeImageData(texture->size, texture->pixelsPntr);
	}
	else
	{
		Assert(sourceRec.X == 0 && sourceRec.Y == 0 && sourceRec.Width == texture->Width && sourceRec.Height == texture->Height);
		newImageData = MakeImageData(texture->size, (u32*)pixelsPntr);
	}
	
	if (IsFlagSet(texture->flags, TextureFlag_Mutable))
	{
		Assert(IsFlagSet(texture->flags, TextureFlag_NoMipmaps));
		sg_image_data sokolImageData = ZEROED;
		sokolImageData.subimage[0][0] = (sg_range){ texture->pixelsPntr, texture->totalSize };
		sg_update_image(texture->image, &sokolImageData);
	}
	else
	{
		Texture newTexture = InitTexture(texture->arena, texture->name, newImageData.size, newImageData.pixels, texture->flags);
		FreeTexture(texture);
		MyMemCopy(texture, &newTexture, sizeof(Texture));
	}
}

PEXPI void UpdateTexture(Texture* texture, const void* pixelsPntr)
{
	UpdateTexturePart(texture, MakeReci(0, 0, texture->Width, texture->Height), pixelsPntr);
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
