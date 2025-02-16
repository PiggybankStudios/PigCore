/*
File:   struct_model_data.h
Author: Taylor Robbins
Date:   02\14\2025
Description:
	** Holds the ModelData struct which is the format that all 3D Model file
	** format parsers produce. This struct is often then transformed into a game
	** specific format depending on the games needs
*/

#ifndef _STRUCT_MODEL_DATA_H
#define _STRUCT_MODEL_DATA_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_assert.h"
#include "std/std_memset.h"
#include "mem/mem_arena.h"
#include "struct/struct_var_array.h"
#include "struct/struct_vectors.h"
#include "struct/struct_quaternion.h"
#include "struct/struct_rectangles.h"
#include "struct/struct_matrices.h"
#include "struct/struct_image_data.h"
#include "gfx/gfx_vertices.h"

typedef struct ModelDataTransform ModelDataTransform;
struct ModelDataTransform
{
	v3 position;
	v3 scale;
	quat rotation;
};

typedef struct ModelDataPart ModelDataPart;
struct ModelDataPart
{
	uxx index;
	Str8 name;
	ModelDataTransform transform;
	box bounds;
	VarArray vertices; //Vertex3D TODO: We probably want to support loading other vertex attributes eventually (like multiple texCoords? Or Tangent vector?)
	VarArray indices; //i32
	uxx materialIndex;
};

typedef struct ModelDataMaterial ModelDataMaterial;
struct ModelDataMaterial
{
	uxx index;
	Str8 name;
	
	v4r albedoFactor;
	r32 roughnessFactor;
	r32 metallicFactor;
	r32 ambientOcclusionFactor;
	
	uxx albedoTextureIndex;
	uxx metallicRoughnessTextureIndex;
	uxx normalTextureIndex;
	uxx ambientOcclusionTextureIndex;
};

typedef struct ModelDataTexture ModelDataTexture;
struct ModelDataTexture
{
	uxx index;
	Str8 name;
	Slice imageFileContents;
	ImageData imageData;
};

typedef struct ModelData ModelData;
struct ModelData
{
	Arena* arena;
	VarArray parts; //ModelDataPart
	VarArray materials; //ModelDataMaterial
	VarArray textures; //ModelDataTexture
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void FreeModelDataPart(Arena* arena, ModelDataPart* part);
	PIG_CORE_INLINE void FreeModelDataMaterial(Arena* arena, ModelDataMaterial* material);
	PIG_CORE_INLINE void FreeModelData(ModelData* modelData);
	ModelData InitModelData(Arena* arena);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI void FreeModelDataPart(Arena* arena, ModelDataPart* part)
{
	NotNull(arena);
	NotNull(part);
	FreeStr8(arena, &part->name);
	FreeVarArray(&part->vertices);
	FreeVarArray(&part->indices);
	ClearPointer(part);
}
PEXPI void FreeModelDataMaterial(Arena* arena, ModelDataMaterial* material)
{
	NotNull(arena);
	NotNull(material);
	FreeStr8(arena, &material->name);
	ClearPointer(material);
}
PEXPI void FreeModelDataTexture(Arena* arena, ModelDataTexture* texture)
{
	NotNull(arena);
	NotNull(texture);
	FreeStr8(arena, &texture->name);
	FreeStr8(arena, &texture->imageFileContents);
	FreeImageData(arena, &texture->imageData);
	ClearPointer(texture);
}
PEXPI void FreeModelData(ModelData* modelData)
{
	NotNull(modelData);
	if (modelData->arena != nullptr)
	{
		VarArrayLoop(&modelData->parts, pIndex)
		{
			VarArrayLoopGet(ModelDataPart, part, &modelData->parts, pIndex);
			FreeModelDataPart(modelData->arena, part);
		}
		FreeVarArray(&modelData->parts);
		VarArrayLoop(&modelData->materials, mIndex)
		{
			VarArrayLoopGet(ModelDataMaterial, material, &modelData->materials, mIndex);
			FreeModelDataMaterial(modelData->arena, material);
		}
		FreeVarArray(&modelData->materials);
	}
	ClearPointer(modelData);
}

PEXP ModelData InitModelData(Arena* arena)
{
	ModelData result = ZEROED;
	result.arena = arena;
	InitVarArray(ModelDataPart, &result.parts, arena);
	InitVarArray(ModelDataMaterial, &result.materials, arena);
	InitVarArray(ModelDataTexture, &result.textures, arena);
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _STRUCT_MODEL_DATA_H
