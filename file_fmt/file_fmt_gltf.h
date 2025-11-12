/*
File:   file_fmt_gltf.h
Author: Taylor Robbins
Date:   02\13\2025
Description:
	** Holds functions that help us parse the glTF (and glB) file format that stores 3D Models
*/

//https://kcoley.github.io/glTF/specification/2.0/figures/gltfOverview-2.0.0a.png

#ifndef _FILE_FMT_GLTF_H
#define _FILE_FMT_GLTF_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "os/os_path.h"
#include "misc/misc_result.h"
#include "mem/mem_arena.h"
#include "mem/mem_scratch.h"
#include "struct/struct_string.h"
#include "struct/struct_model_data.h"
#include "gfx/gfx_image_loading.h"

#if !TARGET_IS_PLAYDATE && !TARGET_IS_WEB //TODO: cgltf.h relies on fopen and a few other stdlib functions that are not available on the Playdate and in the Web

#if PIG_CORE_IMPLEMENTATION
#define CGLTF_IMPLEMENTATION
#endif
#include "third_party/cgltf/cgltf.h"

#define CGLTF_READ_FILE_FUNC_DEF(functionName) cgltf_result functionName(const cgltf_memory_options* memoryOptions, const cgltf_file_options* fileOptions, const char* path, cgltf_size* size, void** data)
typedef CGLTF_READ_FILE_FUNC_DEF(CgltfReadFile_f);
#define CGLTF_RELEASE_FILE_FUNC_DEF(functionName) void functionName(const cgltf_memory_options* memoryOptions, const cgltf_file_options* fileOptions, void* data)
typedef CGLTF_RELEASE_FILE_FUNC_DEF(CgltfReleaseFile_f);

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE Result GetResultFromCgltfResult(cgltf_result cgltfResult);
	Result TryParseGltfFileEx(Slice fileContents, Arena* arena, ModelData* modelDataOut, CgltfReadFile_f* readFileFunc, CgltfReleaseFile_f* releaseFileFunc);
	PIG_CORE_INLINE Result TryParseGltfFile(Slice fileContents, Arena* arena, ModelData* modelDataOut);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

static void* cgltfAllocFunc(void* userPntr, cgltf_size numBytes)
{
	NotNull(userPntr);
	Arena* arena = (Arena*)userPntr;
	return AllocMem(arena, (uxx)numBytes);
}
static void cgltfFreeFunc(void* userPntr, void* allocPntr)
{
	NotNull(userPntr);
	Arena* arena = (Arena*)userPntr;
	if (CanArenaFree(arena)) { FreeMem(arena, allocPntr, 0); }
}

// +==============================+
// |    cgltfReadFileStubFunc     |
// +==============================+
// cgltf_result cgltfReadFileStubFunc(const cgltf_memory_options* memoryOptions, const cgltf_file_options* fileOptions, const char* path, cgltf_size* size, void** data)
static CGLTF_READ_FILE_FUNC_DEF(cgltfReadFileStubFunc)
{
	UNUSED(memoryOptions);
	UNUSED(fileOptions);
	UNUSED(size);
	UNUSED(data);
	PrintLine_E("glTF tried to load external file \"%s\" when ReadFileStubFunc was in use", path);
	return cgltf_result_file_not_found;
}
// +==============================+
// |   cgltfReleaseFileStubFunc   |
// +==============================+
// void cgltfReleaseFileStubFunc(const cgltf_memory_options* memoryOptions, const cgltf_file_options* fileOptions, void* data)
static CGLTF_RELEASE_FILE_FUNC_DEF(cgltfReleaseFileStubFunc)
{
	UNUSED(memoryOptions);
	UNUSED(fileOptions);
	PrintLine_E("glTF tried to free external file data %p when ReleaseFileStubFunc was in use", data);
}

PEXPI Result GetResultFromCgltfResult(cgltf_result cgltfResult)
{
	switch (cgltfResult)
	{
		case cgltf_result_success:         return Result_Success;
		case cgltf_result_data_too_short:  return Result_TooShort;
		case cgltf_result_unknown_format:  return Result_MissingFileHeader;
		case cgltf_result_invalid_json:    return Result_InvalidSyntax;
		case cgltf_result_invalid_gltf:    return Result_ParsingFailure;
		case cgltf_result_invalid_options: return Result_InvalidOptions;
		case cgltf_result_file_not_found:  return Result_FileNotFound;
		case cgltf_result_io_error:        return Result_FailedToReadFile;
		case cgltf_result_out_of_memory:   return Result_FailedToAllocateMemory;
		case cgltf_result_legacy_gltf:     return Result_Deprecated;
		default: return Result_Failure;
	}
}

static const char* GetGltfPrimitiveTypeStr(cgltf_primitive_type type)
{
	switch (type)
	{
		case cgltf_primitive_type_invalid: return "invalid";
		case cgltf_primitive_type_points: return "points";
		case cgltf_primitive_type_lines: return "lines";
		case cgltf_primitive_type_line_loop: return "line_loop";
		case cgltf_primitive_type_line_strip: return "line_strip";
		case cgltf_primitive_type_triangles: return "triangles";
		case cgltf_primitive_type_triangle_strip: return "triangle_strip";
		case cgltf_primitive_type_triangle_fan: return "triangle_fan";
		default: return UNKNOWN_STR;
	}
}
static const char* GetGltfAttributeTypeStr(cgltf_attribute_type type)
{
	switch (type)
	{
		case cgltf_attribute_type_invalid: return "invalid";
		case cgltf_attribute_type_position: return "position";
		case cgltf_attribute_type_normal: return "normal";
		case cgltf_attribute_type_tangent: return "tangent";
		case cgltf_attribute_type_texcoord: return "texcoord";
		case cgltf_attribute_type_color: return "color";
		case cgltf_attribute_type_joints: return "joints";
		case cgltf_attribute_type_weights: return "weights";
		case cgltf_attribute_type_custom: return "custom";
		default: return UNKNOWN_STR;
	}
}
static const char* GetGltfComponentTypeStr(cgltf_component_type type)
{
	switch (type)
	{
		case cgltf_component_type_invalid: return "invalid";
		case cgltf_component_type_r_8: return "i8";
		case cgltf_component_type_r_8u: return "u8";
		case cgltf_component_type_r_16: return "i32";
		case cgltf_component_type_r_16u: return "u16";
		case cgltf_component_type_r_32u: return "u32";
		case cgltf_component_type_r_32f: return "r32";
		default: return UNKNOWN_STR;
	}
}

static ModelDataTransform CummulativeTransformForCgltfNode(cgltf_node* node)
{
	ModelDataTransform result = ZEROED;
	result.scale = V3_One;
	result.rotation = Quat_Identity;
	uxx numParents = 0;
	cgltf_node* root = node;
	while (root->parent != nullptr) { root = root->parent; numParents++; }
	for (uxx pIndex = 0; pIndex <= numParents; pIndex++)
	{
		cgltf_node* ancestor = node;
		for (uxx walkIndex = 0; walkIndex < pIndex; walkIndex++) { ancestor = ancestor->parent; }
		result.position = Add(result.position, MakeV3(ancestor->translation[0], ancestor->translation[1], ancestor->translation[2]));
		//TODO: We should handle negative scale as reversing the indices order??
		result.scale = Mul(result.scale, MakeV3(AbsR32(ancestor->scale[0]), AbsR32(ancestor->scale[1]), AbsR32(ancestor->scale[2])));
		result.rotation = MulQuat(result.rotation, MakeQuat(ancestor->rotation[0], ancestor->rotation[1], ancestor->rotation[2], ancestor->rotation[3]));
	}
	return result;
}

static cgltf_attribute* FindCgltfAttributeOfType(uxx numAttributes, cgltf_attribute* attributes, cgltf_attribute_type type)
{
	for (uxx aIndex = 0; aIndex < numAttributes; aIndex++)
	{
		if (attributes[aIndex].type == type) { return &attributes[aIndex]; }
	}
	return nullptr;
}

//TODO: Once we have ProcessLog, we should do a lot more checks on the data and output warnings for likely unwanted scenarios (like missing textures or material properties)
PEXP Result TryParseGltfFileEx(Slice fileContents, Arena* arena, ModelData* modelDataOut, CgltfReadFile_f* readFileFunc, CgltfReleaseFile_f* releaseFileFunc, Str8 gltfDir)
{
	NotNull(arena);
	NotNull(modelDataOut);
	NotNullStr(fileContents);
	NotNull(readFileFunc);
	NotNull(releaseFileFunc);
	ScratchBegin1(scratch, arena);
	
	cgltf_options parseOptions = ZEROED;
	parseOptions.memory.alloc_func = cgltfAllocFunc;
	parseOptions.memory.free_func = cgltfFreeFunc;
	parseOptions.memory.user_data = scratch;
	parseOptions.file.read = readFileFunc;
	parseOptions.file.release = releaseFileFunc;
	parseOptions.file.user_data = scratch;
	
	uxx scratchUsedBefore = scratch->used;
	UNUSED(scratchUsedBefore);
	
	cgltf_data* parseData = nullptr;
	cgltf_result parseResultCgltf = cgltf_parse(&parseOptions, fileContents.bytes, (size_t)fileContents.length, &parseData);
	Result parseResult = GetResultFromCgltfResult(parseResultCgltf);
	if (parseResult != Result_Success)
	{
		ScratchEnd(scratch);
		return parseResult;
	}
	
	
	//NOTE: If this returns cgltf_result_file_not_found it's likely that the glTF/glB
	// was not exported with the buffer data embedded and TryParseGltfFile doesn't do file access
	Str8 gltfDirNt = AllocStrAndCopy(scratch, gltfDir.length, gltfDir.chars, true);
	cgltf_result loadBuffersResultCgltf = cgltf_load_buffers(&parseOptions, parseData, gltfDirNt.chars);
	Result loadBuffersResult = GetResultFromCgltfResult(loadBuffersResultCgltf);
	if (loadBuffersResult != Result_Success)
	{
		ScratchEnd(scratch);
		return loadBuffersResult;
	}
	
	uxx scratchUsedAfter = scratch->used;
	UNUSED(scratchUsedAfter);
	
	ModelData modelData = InitModelData(arena);
	
	// +==============================+
	// |       Extract Textures       |
	// +==============================+
	VarArrayExpand(&modelData.textures, (uxx)parseData->textures_count);
	for (uxx tIndex = 0; tIndex < (uxx)parseData->textures_count; tIndex++)
	{
		cgltf_texture* parsedTexture = &parseData->textures[tIndex];
		ModelDataTexture* newTexture = VarArrayAdd(ModelDataTexture, &modelData.textures);
		NotNull(newTexture);
		ClearPointer(newTexture);
		newTexture->index = modelData.textures.length-1;
		newTexture->name = AllocStr8Nt(arena, parsedTexture->name);
		NotNull(parsedTexture->image);
		Str8 imageName = MakeStr8Nt(parsedTexture->image->name);
		if (IsEmptyStr(newTexture->name) && !IsEmptyStr(imageName)) { newTexture->name = AllocStr8(arena, imageName); }
		if (parsedTexture->image->buffer_view == nullptr)
		{
			if (readFileFunc != nullptr)
			{
				Assert(StrExactEndsWith(gltfDir, StrLit("/")) || StrExactEndsWith(gltfDir, StrLit("\\")));
				uxx scratchMark = ArenaGetMark(scratch);
				FilePath imageFilePathNt = JoinStringsInArena(scratch, gltfDir, MakeStr8Nt(parsedTexture->image->uri), true);
				cgltf_size dataSize = 0;
				void* dataPntr = nullptr;
				cgltf_result readFileResult = readFileFunc(&parseOptions.memory, &parseOptions.file, imageFilePathNt.chars, &dataSize, &dataPntr);
				if (readFileResult != cgltf_result_success || dataSize == 0 || dataPntr == nullptr)
				{
					FreeModelData(&modelData);
					ScratchEnd(scratch);
					return GetResultFromCgltfResult(readFileResult);
				}
				newTexture->imageFileContents.length = (uxx)dataSize;
				newTexture->imageFileContents.bytes = (u8*)AllocMem(arena, newTexture->imageFileContents.length);
				NotNull(newTexture->imageFileContents.bytes);
				MyMemCopy(newTexture->imageFileContents.bytes, dataPntr, (uxx)dataSize);
				ArenaResetToMark(scratch, scratchMark);
			}
		}
		else
		{
			NotNull(parsedTexture->image->buffer_view);
			NotNull(parsedTexture->image->buffer_view->buffer);
			NotNull(parsedTexture->image->buffer_view->buffer->data);
			//TODO: Should we somehow handle the parsedTexture->image->uri?
			Slice imageFileContents = MakeSlice(
				(uxx)parsedTexture->image->buffer_view->size,
				((u8*)parsedTexture->image->buffer_view->buffer->data) + parsedTexture->image->buffer_view->offset
			);
			newTexture->imageFileContents = AllocStr8(arena, imageFileContents);
			NotNull(newTexture->imageFileContents.bytes);
		}
		#if PIG_CORE_TRY_PARSE_IMAGE_AVAILABLE
		if (!IsEmptyStr(newTexture->imageFileContents))
		{
			Result parseImageResult = TryParseImageFile(newTexture->imageFileContents, arena, &newTexture->imageData);
			if (parseImageResult != Result_Success)
			{
				FreeModelData(&modelData);
				ScratchEnd(scratch);
				return parseImageResult;
			}
		}
		#endif
		UNUSED(parsedTexture->sampler); //TODO: Add support for sampler information like min and mag filter!
	}
	
	// +==============================+
	// |      Extract Materials       |
	// +==============================+
	VarArrayExpand(&modelData.materials, (uxx)parseData->materials_count);
	for (uxx mIndex = 0; mIndex < (uxx)parseData->materials_count; mIndex++)
	{
		cgltf_material* parsedMaterial = &parseData->materials[mIndex];
		ModelDataMaterial* material = VarArrayAdd(ModelDataMaterial, &modelData.materials);
		NotNull(material);
		ClearPointer(material);
		material->index = modelData.materials.length-1;
		material->name = AllocStr8Nt(arena, parsedMaterial->name);
		
		if (parsedMaterial->has_pbr_metallic_roughness)
		{
			material->albedoFactor = MakeV4r(parsedMaterial->pbr_metallic_roughness.base_color_factor[0], parsedMaterial->pbr_metallic_roughness.base_color_factor[1], parsedMaterial->pbr_metallic_roughness.base_color_factor[2], parsedMaterial->pbr_metallic_roughness.base_color_factor[3]);
			material->metallicFactor = parsedMaterial->pbr_metallic_roughness.metallic_factor;
			material->roughnessFactor = parsedMaterial->pbr_metallic_roughness.roughness_factor;
			if (parsedMaterial->pbr_metallic_roughness.base_color_texture.texture != nullptr)
			{
				Assert(parsedMaterial->pbr_metallic_roughness.base_color_texture.texture >= parseData->textures);
				Assert(parsedMaterial->pbr_metallic_roughness.base_color_texture.texture < &parseData->textures[parseData->textures_count]);
				material->albedoTextureIndex = (uxx)(parsedMaterial->pbr_metallic_roughness.base_color_texture.texture - parseData->textures);
			}
			else { material->albedoTextureIndex = UINTXX_MAX; }
			if (parsedMaterial->pbr_metallic_roughness.metallic_roughness_texture.texture != nullptr)
			{
				Assert(parsedMaterial->pbr_metallic_roughness.metallic_roughness_texture.texture >= parseData->textures);
				Assert(parsedMaterial->pbr_metallic_roughness.metallic_roughness_texture.texture < &parseData->textures[parseData->textures_count]);
				material->metallicRoughnessTextureIndex = (uxx)(parsedMaterial->pbr_metallic_roughness.metallic_roughness_texture.texture - parseData->textures);
			}
			else { material->metallicRoughnessTextureIndex = UINTXX_MAX; }
		}
		else
		{
			material->albedoFactor = FillV4r(1);
			material->metallicFactor = 0.0f;
			material->roughnessFactor = 0.5f;
			material->albedoTextureIndex = UINTXX_MAX;
			material->metallicRoughnessTextureIndex = UINTXX_MAX;
		}
		
		if (parsedMaterial->normal_texture.texture != nullptr)
		{
			Assert(parsedMaterial->normal_texture.texture >= parseData->textures);
			Assert(parsedMaterial->normal_texture.texture < &parseData->textures[parseData->textures_count]);
			material->normalTextureIndex = (uxx)(parsedMaterial->normal_texture.texture - parseData->textures);
		}
		else { material->normalTextureIndex = UINTXX_MAX; }
		
		//NOTE: We are currently plugging the AO texture into the specular input in the Principled BSDF in Blender!
		if (parsedMaterial->has_specular)
		{
			//TODO: Should we pay attention to specular_color_factor?
			material->ambientOcclusionFactor = parsedMaterial->specular.specular_factor;
			if (parsedMaterial->specular.specular_color_texture.texture != nullptr)
			{
				Assert(parsedMaterial->specular.specular_color_texture.texture >= parseData->textures);
				Assert(parsedMaterial->specular.specular_color_texture.texture < &parseData->textures[parseData->textures_count]);
				material->ambientOcclusionTextureIndex = (uxx)(parsedMaterial->specular.specular_color_texture.texture - parseData->textures);
			}
			else { material->ambientOcclusionTextureIndex = UINTXX_MAX; }
		}
		else
		{
			material->ambientOcclusionFactor = 1.0f;
			material->ambientOcclusionTextureIndex = UINTXX_MAX;
		}
		
		PrintLine_D("Material[%llu]: albedo=%llu metallicRoughness=%llu normal=%llu occlusion=%llu",
			(u64)mIndex,
			(u64)material->albedoTextureIndex,
			(u64)material->metallicRoughnessTextureIndex,
			(u64)material->normalTextureIndex,
			(u64)material->ambientOcclusionTextureIndex
		);
	}
	
	// +==============================+
	// |    Extract Parts (Meshes)    |
	// +==============================+
	uxx numMeshNodes = 0;
	for (uxx nIndex = 0; nIndex < (uxx)parseData->nodes_count; nIndex++) { if (parseData->nodes[nIndex].mesh != nullptr) { numMeshNodes++; } }
	
	VarArrayExpand(&modelData.parts, numMeshNodes);
	for (uxx nIndex = 0; nIndex < (uxx)parseData->nodes_count; nIndex++)
	{
		cgltf_node* parsedNode = &parseData->nodes[nIndex];
		if (parsedNode->mesh != nullptr)
		{
			ModelDataPart* part = VarArrayAdd(ModelDataPart, &modelData.parts);
			NotNull(part);
			ClearPointer(part);
			part->index = modelData.parts.length-1;
			part->name = AllocStr8Nt(arena, parsedNode->name);
			part->transform = CummulativeTransformForCgltfNode(parsedNode);
			part->materialIndex = UINTXX_MAX;
			//TODO: Fill out bounds!
			uxx numVerticesTotal = 0;
			uxx numIndicesTotal = 0;
			for (uxx pIndex = 0; pIndex < (uxx)parsedNode->mesh->primitives_count; pIndex++)
			{
				cgltf_primitive* primitive = &parsedNode->mesh->primitives[pIndex];
				cgltf_attribute* positionAttrib = FindCgltfAttributeOfType((uxx)primitive->attributes_count, primitive->attributes, cgltf_attribute_type_position);
				NotNull(positionAttrib); //TODO: Can this ever happen?
				NotNull(positionAttrib->data); //TODO: Can this ever happen?
				numVerticesTotal += (uxx)positionAttrib->data->count;
				numIndicesTotal += (uxx)((primitive->indices != nullptr) ? primitive->indices->count : 0);
			}
			InitVarArrayWithInitial(Vertex3D, &part->vertices, arena, numVerticesTotal);
			InitVarArrayWithInitial(i32, &part->indices, arena, numIndicesTotal);
			for (uxx pIndex = 0; pIndex < (uxx)parsedNode->mesh->primitives_count; pIndex++)
			{
				cgltf_primitive* primitive = &parsedNode->mesh->primitives[pIndex];
				
				uxx primitiveMaterialIndex = UINTXX_MAX;
				if (primitive->material != nullptr)
				{
					Assert(primitive->material >= &parseData->materials[0]);
					Assert(primitive->material < &parseData->materials[parseData->materials_count]);
					primitiveMaterialIndex = (uxx)(primitive->material - &parseData->materials[0]);
				}
				if (pIndex == 0) { part->materialIndex = primitiveMaterialIndex; }
				else if (part->materialIndex != primitiveMaterialIndex)
				{
					PrintLine_E("We don't support single meshes with multiple materials right now! Node[%llu] \"%s\" primitive[%llu] has material %llu not %llu",
						(u64)nIndex, parsedNode->name,
						(u64)pIndex,
						primitiveMaterialIndex, part->materialIndex
					);
				}
				
				cgltf_attribute* positionAttrib = FindCgltfAttributeOfType((uxx)primitive->attributes_count, primitive->attributes, cgltf_attribute_type_position);
				cgltf_attribute* normalAttrib = FindCgltfAttributeOfType((uxx)primitive->attributes_count, primitive->attributes, cgltf_attribute_type_normal);
				cgltf_attribute* texCoordAttrib = FindCgltfAttributeOfType((uxx)primitive->attributes_count, primitive->attributes, cgltf_attribute_type_texcoord);
				cgltf_attribute* colorAttrib = FindCgltfAttributeOfType((uxx)primitive->attributes_count, primitive->attributes, cgltf_attribute_type_color);
				//TODO: Look for cgltf_attribute_type_tangent?
				//TODO: Look for cgltf_attribute_type_joints?
				//TODO: Look for cgltf_attribute_type_weights?
				NotNull(positionAttrib); //TODO: Can this ever happen?
				i32 indicesOffset = (i32)part->vertices.length;
				uxx numVerticesInPrimitive = (uxx)positionAttrib->data->count;
				Vertex3D* newVertices = VarArrayAddMulti(Vertex3D, &part->vertices, numVerticesInPrimitive);
				NotNull(newVertices);
				for (uxx vIndex = 0; vIndex < numVerticesInPrimitive; vIndex++)
				{
					Vertex3D* newVertex = &newVertices[vIndex];
					
					NotNull(positionAttrib->data);
					NotNull(positionAttrib->data->buffer_view);
					NotNull(positionAttrib->data->buffer_view->buffer);
					NotNull(positionAttrib->data->buffer_view->buffer->data);
					r32* positionDataPntr = (r32*)&((u8*)positionAttrib->data->buffer_view->buffer->data)[
						positionAttrib->data->buffer_view->offset +
						(vIndex * positionAttrib->data->stride) +
						positionAttrib->data->offset
					];
					newVertex->position = MakeV3(positionDataPntr[0], positionDataPntr[1], positionDataPntr[2]);
					
					if (normalAttrib != nullptr)
					{
						NotNull(normalAttrib->data);
						Assert(normalAttrib->data->count == numVerticesInPrimitive);
						NotNull(normalAttrib->data->buffer_view);
						NotNull(normalAttrib->data->buffer_view->buffer);
						NotNull(normalAttrib->data->buffer_view->buffer->data);
						r32* normalDataPntr = (r32*)&((u8*)normalAttrib->data->buffer_view->buffer->data)[
							normalAttrib->data->buffer_view->offset +
							(vIndex * normalAttrib->data->stride) +
							normalAttrib->data->offset
						];
						newVertex->normal = MakeV3(normalDataPntr[0], normalDataPntr[1], normalDataPntr[2]);
					}
					else { newVertex->normal = V3_Up; }
					if (texCoordAttrib != nullptr)
					{
						NotNull(texCoordAttrib->data);
						Assert(texCoordAttrib->data->count == numVerticesInPrimitive);
						NotNull(texCoordAttrib->data->buffer_view);
						NotNull(texCoordAttrib->data->buffer_view->buffer);
						NotNull(texCoordAttrib->data->buffer_view->buffer->data);
						r32* texCoordDataPntr = (r32*)&((u8*)texCoordAttrib->data->buffer_view->buffer->data)[
							texCoordAttrib->data->buffer_view->offset +
							(vIndex * texCoordAttrib->data->stride) +
							texCoordAttrib->data->offset
						];
						newVertex->texCoord = MakeV2(texCoordDataPntr[0], texCoordDataPntr[1]);
					}
					else { newVertex->texCoord = V2_Zero; }
					if (colorAttrib != nullptr)
					{
						NotNull(colorAttrib->data);
						Assert(colorAttrib->data->count == numVerticesInPrimitive);
						NotNull(colorAttrib->data->buffer_view);
						NotNull(colorAttrib->data->buffer_view->buffer);
						NotNull(colorAttrib->data->buffer_view->buffer->data);
						r32* colorDataPntr = (r32*)&((u8*)colorAttrib->data->buffer_view->buffer->data)[
							colorAttrib->data->buffer_view->offset +
							(vIndex * colorAttrib->data->stride) +
							colorAttrib->data->offset
						];
						newVertex->color = MakeV4r(colorDataPntr[0], colorDataPntr[1], colorDataPntr[2], colorDataPntr[3]);
					}
					else { newVertex->color = FillV4r(1); }
				}
				
				if (primitive->indices != nullptr)
				{
					Assert(primitive->indices->type == cgltf_type_scalar);
					uxx numIndicesInPrimitive = (uxx)primitive->indices->count;
					i32* newIndices = VarArrayAddMulti(i32, &part->indices, numIndicesInPrimitive);
					for (uxx iIndex = 0; iIndex < numIndicesInPrimitive; iIndex++)
					{
						NotNull(primitive->indices->buffer_view);
						NotNull(primitive->indices->buffer_view->buffer);
						NotNull(primitive->indices->buffer_view->buffer->data);
						u8* indexDataPntr = &((u8*)primitive->indices->buffer_view->buffer->data)[
							primitive->indices->buffer_view->offset +
							(iIndex * primitive->indices->stride) +
							primitive->indices->offset
						];
						switch (primitive->indices->component_type)
						{
							case cgltf_component_type_r_8:   newIndices[iIndex] = indicesOffset + (i32)(*((i8*)indexDataPntr));  break;
							case cgltf_component_type_r_8u:  newIndices[iIndex] = indicesOffset + (i32)(*((u8*)indexDataPntr));  break;
							case cgltf_component_type_r_16:  newIndices[iIndex] = indicesOffset + (i32)(*((i16*)indexDataPntr)); break;
							case cgltf_component_type_r_16u: newIndices[iIndex] = indicesOffset + (i32)(*((u16*)indexDataPntr)); break;
							case cgltf_component_type_r_32u: newIndices[iIndex] = indicesOffset + (i32)(*((u32*)indexDataPntr)); break;
							case cgltf_component_type_r_32f: newIndices[iIndex] = indicesOffset + (i32)(*((r32*)indexDataPntr)); break;
							default: AssertMsg(false, "Indices are declared as unsupported component_type!"); break;
						}
					}
				}
				
			}
		}
	}
	
	// +====================================+
	// | Debug Printout of cgltf Structure  |
	// +====================================+
	#if 0
	{
		PrintLine_D("Used %llu bytes from scratch", scratchUsedAfter - scratchUsedBefore);
		
		if (parseData->file_type == cgltf_file_type_glb) { WriteLine_D("Type was Binary!"); }
		else if (parseData->file_type == cgltf_file_type_gltf) { WriteLine_D("Type was Text!"); }
		else if (parseData->file_type == cgltf_file_type_invalid) { WriteLine_D("Type was Invalid!"); }
		else { WriteLine_D("Type was Unknown!"); }
		PrintLine_D("file_data: %p (fileContents.bytes = %p)", parseData->file_data, fileContents.bytes);
		PrintLine_D("asset.copyright = %s", parseData->asset.copyright);
		PrintLine_D("asset.generator = %s", parseData->asset.generator);
		PrintLine_D("asset.version = %s", parseData->asset.version);
		PrintLine_D("asset.min_version = %s", parseData->asset.min_version);
		PrintLine_D("asset.extras = %llu-%llu %p", (u64)parseData->asset.extras.start_offset, (u64)parseData->asset.extras.end_offset, parseData->asset.extras.data);
		PrintLine_D("asset.extensions: [%llu]", (u64)parseData->asset.extensions_count);
		for (uxx eIndex = 0; eIndex < (uxx)parseData->asset.extensions_count; eIndex++)
		{
			PrintLine_D("\textension[%llu]: \"%s\" %p", (u64)eIndex, parseData->asset.extensions[eIndex].name, parseData->asset.extensions[eIndex].data);
		}
		
		PrintLine_N("Meshes: [%llu]", (u64)parseData->meshes_count);
		for (uxx mIndex = 0; mIndex < (uxx)parseData->meshes_count; mIndex++)
		{
			cgltf_mesh* mesh = &parseData->meshes[mIndex];
			PrintLine_D("\tMesh[%llu]: \"%s\" %llu primitive%s %llu weight%s %llu target name%s %llu extension%s",
				(u64)mIndex, mesh->name,
				(u64)mesh->primitives_count, Plural(mesh->primitives_count, "s"),
				(u64)mesh->weights_count, Plural(mesh->weights_count, "s"),
				(u64)mesh->target_names_count, Plural(mesh->target_names_count, "s"),
				(u64)mesh->extensions_count, Plural(mesh->extensions_count, "s")
			);
			// TODO: cgltf_float* weights;
			// TODO: char** target_names;
			// TODO: cgltf_extras extras;
			// TODO: cgltf_extension* extensions;
			for (uxx pIndex = 0; pIndex < (uxx)mesh->primitives_count; pIndex++)
			{
				cgltf_primitive* primitive = &mesh->primitives[pIndex];
				PrintLine_D("\t\tPrimitive[%llu]: %s %llu attribute%s %llu target%s %llu material mapping%s %llu extension%s",
					(u64)pIndex, GetGltfPrimitiveTypeStr(primitive->type),
					(u64)primitive->attributes_count, Plural(primitive->attributes_count, "s"),
					(u64)primitive->targets_count, Plural(primitive->targets_count, "s"),
					(u64)primitive->mappings_count, Plural(primitive->mappings_count, "s"),
					(u64)primitive->extensions_count, Plural(primitive->extensions_count, "s")
				);
				if (primitive->indices != nullptr)
				{
					PrintLine_D("\t\t\tIndices: %s offset=%llu count=%llu stride=%llu%s",
						GetGltfComponentTypeStr(primitive->indices->component_type),
						(u64)primitive->indices->offset,
						(u64)primitive->indices->count,
						(u64)primitive->indices->stride,
						primitive->indices->is_sparse ? " (Sparse)" : ""
					);
				}
				// TODO: cgltf_material* material;
				// TODO: cgltf_morph_target* targets;
				// TODO: cgltf_extras extras;
				// TODO: cgltf_bool has_draco_mesh_compression;
				// TODO: cgltf_draco_mesh_compression draco_mesh_compression;
				// TODO: cgltf_material_mapping* mappings;
				// TODO: cgltf_extension* extensions;
				for (uxx aIndex = 0; aIndex < (uxx)primitive->attributes_count; aIndex++)
				{
					cgltf_attribute* attribute = &primitive->attributes[aIndex];
					PrintLine_D("\t\t\tAttribute[%llu]: \"%s\" %s index=%llu",
						(u64)aIndex,
						attribute->name,
						GetGltfAttributeTypeStr(attribute->type),
						(u64)attribute->index
					);
					if (attribute->data != nullptr)
					{
						NotNull(attribute->data->buffer_view);
						NotNull(attribute->data->buffer_view->buffer);
						//TODO: Maybe we should look into attribute->data->buffer_view?
						PrintLine_D("\t\t\t\tAccessor: %s offset=%llu count=%llu stride=%llu%s",
							GetGltfComponentTypeStr(attribute->data->component_type),
							(u64)attribute->data->offset,
							(u64)attribute->data->count,
							(u64)attribute->data->stride,
							attribute->data->is_sparse ? " (Sparse)" : ""
						);
						PrintLine_D("\t\t\t\tBufferView: %s \"%s\" offset=%llu size=%llu stride=%llu",
							(attribute->data->buffer_view->type == cgltf_buffer_view_type_vertices) ? "vertices" : "indices",
							attribute->data->buffer_view->name,
							(u64)attribute->data->buffer_view->offset,
							(u64)attribute->data->buffer_view->size,
							(u64)attribute->data->buffer_view->stride
						);
						PrintLine_D("\t\t\t\tBuffer: \"%s\" %p size=%llu data=%p",
							attribute->data->buffer_view->buffer->name,
							attribute->data->buffer_view->buffer,
							(u64)attribute->data->buffer_view->buffer->size,
							attribute->data->buffer_view->buffer->data
						);
					}
				}
			}
		}
		
		PrintLine_N("Materials: [%llu]", (u64)parseData->materials_count);
		for (uxx mIndex = 0; mIndex < (uxx)parseData->materials_count; mIndex++)
		{
			cgltf_material* material = &parseData->materials[mIndex];
			PrintLine_D("\tMaterial[%llu]: \"%s\"%s%s%s%s%s%s%s%s%s%s%s%s%s",
				(u64)mIndex, material->name,
				material->has_pbr_metallic_roughness ? " (PbrMetallicRoughness)" : "",
				material->has_pbr_specular_glossiness ? " (PbrSpecularGlossiness)" : "",
				material->has_clearcoat ? " (clearcoat)" : "",
				material->has_transmission ? " (transmission)" : "",
				material->has_volume ? " (volume)" : "",
				material->has_ior ? " (ior)" : "",
				material->has_specular ? " (specular)" : "",
				material->has_sheen ? " (sheen)" : "",
				material->has_emissive_strength ? " (emissive_strength)" : "",
				material->has_iridescence ? " (iridescence)" : "",
				material->has_diffuse_transmission ? " (diffuse_transmission)" : "",
				material->has_anisotropy ? " (anisotropy)" : "",
				material->has_dispersion ? " (dispersion)" : ""
			);
			if (material->has_pbr_metallic_roughness)
			{
				PrintLine_D("\t\tPbrMetallicRoughness: metallic=%f%s roughness=%f%s baseColor=(%g, %g, %g, %g)%s",
					material->pbr_metallic_roughness.metallic_factor,
					(material->pbr_metallic_roughness.metallic_roughness_texture.texture != nullptr) ? "*Texture" : "",
					material->pbr_metallic_roughness.roughness_factor,
					(material->pbr_metallic_roughness.metallic_roughness_texture.texture != nullptr) ? "*Texture" : "",
					material->pbr_metallic_roughness.base_color_factor[0], material->pbr_metallic_roughness.base_color_factor[1], material->pbr_metallic_roughness.base_color_factor[2], material->pbr_metallic_roughness.base_color_factor[3],
					(material->pbr_metallic_roughness.base_color_texture.texture != nullptr) ? "*Texture" : ""
				);
			}
			if (material->has_pbr_specular_glossiness)
			{
				PrintLine_D("\t\tPbrSpecularGloss: diffuse=(%g, %g, %g, %g)%s specular=(%g, %g, %g)%s glossiness=%f%s",
					material->pbr_specular_glossiness.diffuse_factor[0], material->pbr_specular_glossiness.diffuse_factor[1], material->pbr_specular_glossiness.diffuse_factor[2], material->pbr_specular_glossiness.diffuse_factor[3],
					(material->pbr_specular_glossiness.diffuse_texture.texture != nullptr) ? "*Texture" : "",
					material->pbr_specular_glossiness.specular_factor[0], material->pbr_specular_glossiness.specular_factor[1], material->pbr_specular_glossiness.specular_factor[2],
					(material->pbr_specular_glossiness.specular_glossiness_texture.texture != nullptr) ? "*Texture" : "",
					material->pbr_specular_glossiness.glossiness_factor,
					(material->pbr_specular_glossiness.specular_glossiness_texture.texture != nullptr) ? "*Texture" : ""
				);
			}
			if (material->has_specular)
			{
				PrintLine_D("\t\tSpecular: specular=%f%s specular_color=(%g, %g, %g)%s",
					material->specular.specular_factor,
					(material->specular.specular_texture.texture != nullptr) ? "*Texture" : "",
					material->specular.specular_color_factor[0], material->specular.specular_color_factor[1], material->specular.specular_color_factor[2],
					(material->specular.specular_color_texture.texture != nullptr) ? "*Texture" : ""
				);
			}
			// TODO: cgltf_clearcoat clearcoat;
			// TODO: cgltf_ior ior;
			// TODO: cgltf_sheen sheen;
			// TODO: cgltf_transmission transmission;
			// TODO: cgltf_volume volume;
			// TODO: cgltf_emissive_strength emissive_strength;
			// TODO: cgltf_iridescence iridescence;
			// TODO: cgltf_diffuse_transmission diffuse_transmission;
			// TODO: cgltf_anisotropy anisotropy;
			// TODO: cgltf_dispersion dispersion;
			// TODO: cgltf_texture_view normal_texture;
			// TODO: cgltf_texture_view occlusion_texture;
			// TODO: cgltf_texture_view emissive_texture;
			// TODO: cgltf_float emissive_factor[3];
			// TODO: cgltf_alpha_mode alpha_mode;
			// TODO: cgltf_float alpha_cutoff;
			// TODO: cgltf_bool double_sided;
			// TODO: cgltf_bool unlit;
			// TODO: cgltf_extras extras;
			// TODO: cgltf_size extensions_count;
			// TODO: cgltf_extension* extensions;
		}
		
		PrintLine_N("Accessors: [%llu]", (u64)parseData->accessors_count);
		for (uxx aIndex = 0; aIndex < (uxx)parseData->accessors_count; aIndex++)
		{
			cgltf_accessor* accessor = &parseData->accessors[aIndex];
			UNUSED(accessor);
			//TODO: Implement me!
		}
		
		PrintLine_N("Buffer Views: [%llu]", (u64)parseData->buffer_views_count);
		for (uxx bvIndex = 0; bvIndex < (uxx)parseData->buffer_views_count; bvIndex++)
		{
			cgltf_buffer_view* bufferView = &parseData->buffer_views[bvIndex];
			UNUSED(bufferView);
			//TODO: Implement me!
		}
		
		PrintLine_N("Buffers: [%llu]", (u64)parseData->buffers_count);
		for (uxx bIndex = 0; bIndex < (uxx)parseData->buffers_count; bIndex++)
		{
			cgltf_buffer* buffer = &parseData->buffers[bIndex];
			Str8 uriStr = StrLit(buffer->uri);
			Str8 uriShortened = StrSliceMaxLength(uriStr, 0, 32);
			PrintLine_D("\tBuffer[%llu]: \"%s\" %llu bytes %p uri=\"%.*s%s\" %llu extension%s",
				(u64)bIndex, buffer->name,
				(u64)buffer->size,
				buffer->data,
				StrPrint(uriShortened), (uriShortened.length < uriStr.length) ? "..." : "",
				(u64)buffer->extensions_count, Plural(buffer->extensions_count, "s")
			);
			// TODO: cgltf_data_free_method data_free_method;
			// TODO: cgltf_extras extras;
			// TODO: cgltf_extension* extensions;
		}
		
		PrintLine_N("Images: [%llu]", (u64)parseData->images_count);
		for (uxx iIndex = 0; iIndex < (uxx)parseData->images_count; iIndex++)
		{
			cgltf_image* image = &parseData->images[iIndex];
			UNUSED(image);
			//TODO: Implement me!
		}
		
		PrintLine_N("Textures: [%llu]", (u64)parseData->textures_count);
		for (uxx tIndex = 0; tIndex < (uxx)parseData->textures_count; tIndex++)
		{
			cgltf_texture* texture = &parseData->textures[tIndex];
			PrintLine_D("\tTexture[%llu]: \"%s\" image=%p sampler=%p %llu extension%s%s%s",
				(u64)tIndex, texture->name,
				texture->image, texture->sampler,
				texture->extensions_count, Plural(texture->extensions_count, "s"),
				texture->has_basisu ? " (Has Basisu)" : "",
				texture->has_webp ? " (Has webp)" : ""
			);
			// TODO: cgltf_bool has_basisu;
			// TODO: cgltf_bool has_webp;
			// TODO: cgltf_extras extras;
			// TODO: cgltf_extension* extensions;
			if (texture->image != nullptr)
			{
				Str8 uriStr = StrLit(texture->image->uri);
				Str8 uriShortened = StrSliceMaxLength(uriStr, 0, 32);
				// TODO: cgltf_extras extras;
				// TODO: cgltf_extension* extensions;
				if (texture->image->buffer_view != nullptr)
				{
					NotNull(texture->image->buffer_view->buffer);
					PrintLine_D("\t\tImage: \"%s\" uri=\"%.*s%s\" mimeType=%s buffer=\"%s\" offset=%llu size=%llu stride=%llu %llu extension%s",
						texture->image->name,
						StrPrint(uriShortened), (uriShortened.length < uriStr.length) ? "..." : "",
						texture->image->mime_type,
						texture->image->buffer_view->buffer->name,
						(u64)texture->image->buffer_view->offset,
						(u64)texture->image->buffer_view->size,
						(u64)texture->image->buffer_view->stride,
						(u64)texture->image->extensions_count, Plural(texture->image->extensions_count, "s")
					);
				}
				else
				{
					PrintLine_D("\t\tImage: \"%s\" uri=\"%.*s%s\" mimeType=%s %llu extension%s",
						texture->image->name,
						StrPrint(uriShortened), (uriShortened.length < uriStr.length) ? "..." : "",
						texture->image->mime_type,
						(u64)texture->image->extensions_count, Plural(texture->image->extensions_count, "s")
					);
				}
			}
		}
		
		PrintLine_N("Samplers: [%llu]", (u64)parseData->samplers_count);
		for (uxx sIndex = 0; sIndex < (uxx)parseData->samplers_count; sIndex++)
		{
			cgltf_sampler* sampler = &parseData->samplers[sIndex];
			UNUSED(sampler);
			//TODO: Implement me!
		}
		
		PrintLine_N("Skins: [%llu]", (u64)parseData->skins_count);
		for (uxx sIndex = 0; sIndex < (uxx)parseData->skins_count; sIndex++)
		{
			cgltf_skin* skin = &parseData->skins[sIndex];
			UNUSED(skin);
			//TODO: Implement me!
		}
		
		PrintLine_N("Cameras: [%llu]", (u64)parseData->cameras_count);
		for (uxx cIndex = 0; cIndex < (uxx)parseData->cameras_count; cIndex++)
		{
			cgltf_camera* camera = &parseData->cameras[cIndex];
			UNUSED(camera);
			//TODO: Implement me!
		}
		
		PrintLine_N("Lights: [%llu]", (u64)parseData->lights_count);
		for (uxx lIndex = 0; lIndex < (uxx)parseData->lights_count; lIndex++)
		{
			cgltf_light* light = &parseData->lights[lIndex];
			UNUSED(light);
			//TODO: Implement me!
		}
		
		PrintLine_N("Nodes: [%llu]", (u64)parseData->nodes_count);
		for (uxx nIndex = 0; nIndex < (uxx)parseData->nodes_count; nIndex++)
		{
			cgltf_node* node = &parseData->nodes[nIndex];
			PrintLine_D("\tNode[%llu]: \"%s\" parent=\"%s\" %llu child%s%s%s%s%s",
				(u64)nIndex, node->name,
				(node->parent != nullptr) ? node->parent->name : "[none]",
				(u64)node->children_count, Plural(node->children_count, "ren"),
				(node->camera != nullptr) ? " (Camera)" : "",
				(node->mesh != nullptr) ? " (Mesh)" : "",
				(node->light != nullptr) ? " (Light)" : "",
				(node->skin != nullptr) ? " (Skin)" : ""
			);
			// TODO: cgltf_node** children;
			// TODO: cgltf_float* weights;
			// TODO: cgltf_size weights_count;
			// TODO: cgltf_bool has_translation;
			// TODO: cgltf_bool has_rotation;
			// TODO: cgltf_bool has_scale;
			// TODO: cgltf_bool has_matrix;
			// TODO: cgltf_float translation[3];
			// TODO: cgltf_float rotation[4];
			// TODO: cgltf_float scale[3];
			// TODO: cgltf_float matrix[16];
			// TODO: cgltf_extras extras;
			// TODO: cgltf_bool has_mesh_gpu_instancing;
			// TODO: cgltf_mesh_gpu_instancing mesh_gpu_instancing;
			// TODO: cgltf_size extensions_count;
			// TODO: cgltf_extension* extensions;
		}
		
		PrintLine_N("Scenes: [%llu]", (u64)parseData->scenes_count);
		for (uxx sIndex = 0; sIndex < (uxx)parseData->scenes_count; sIndex++)
		{
			cgltf_scene* scene = &parseData->scenes[sIndex];
			UNUSED(scene);
			//TODO: Implement me!
		}
		
		PrintLine_N("Animations: [%llu]", (u64)parseData->animations_count);
		for (uxx aIndex = 0; aIndex < (uxx)parseData->animations_count; aIndex++)
		{
			cgltf_animation* animation = &parseData->animations[aIndex];
			UNUSED(animation);
			//TODO: Implement me!
		}
		
		PrintLine_N("Material Variants: [%llu]", (u64)parseData->variants_count);
		for (uxx vIndex = 0; vIndex < (uxx)parseData->variants_count; vIndex++)
		{
			cgltf_material_variant* materialVariant = &parseData->variants[vIndex];
			UNUSED(materialVariant);
			//TODO: Implement me!
		}
		
		PrintLine_N("Data Extensions: [%llu]", (u64)parseData->data_extensions_count);
		for (uxx eIndex = 0; eIndex < (uxx)parseData->data_extensions_count; eIndex++)
		{
			cgltf_extension* dataExtension = &parseData->data_extensions[eIndex];
			UNUSED(dataExtension);
			//TODO: Implement me!
		}
		
		PrintLine_N("Extensions Used: [%llu]", (u64)parseData->extensions_used_count);
		for (uxx eIndex = 0; eIndex < (uxx)parseData->extensions_used_count; eIndex++)
		{
			char* usedExtensionName = parseData->extensions_used[eIndex];
			PrintLine_D("\tUsed Extension[%llu]: \"%s\"", (u64)eIndex, usedExtensionName);
		}
		
		PrintLine_N("Extensions Required: [%llu]", (u64)parseData->extensions_required_count);
		for (uxx eIndex = 0; eIndex < (uxx)parseData->extensions_required_count; eIndex++)
		{
			char* requiredExtensionName = parseData->extensions_required[eIndex];
			PrintLine_D("\tRequired Extension[%llu]: \"%s\"", (u64)eIndex, requiredExtensionName);
		}
		
		// TODO: cgltf_scene* scene;
		// TODO: cgltf_extras extras;
		// TODO: const char* json;
		// TODO: cgltf_size json_size;
		// TODO: const void* bin;
		// TODO: cgltf_size bin_size;
		// TODO: cgltf_memory_options memory;
		// TODO: cgltf_file_options file;
	}
	#endif
	
	ScratchEnd(scratch);
	MyMemCopy(modelDataOut, &modelData, sizeof(ModelData));
	return Result_Success;
}

PEXPI Result TryParseGltfFile(Slice fileContents, Arena* arena, ModelData* modelDataOut)
{
	return TryParseGltfFileEx(fileContents, arena, modelDataOut, cgltfReadFileStubFunc, cgltfReleaseFileStubFunc, Str8_Empty);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //!TARGET_IS_PLAYDATE && !TARGET_IS_WEB

#endif //  _FILE_FMT_GLTF_H

#if defined(_FILE_FMT_GLTF_H) && defined(_OS_FILE_H)
#include "cross/cross_gltf_and_os_file.h"
#endif
