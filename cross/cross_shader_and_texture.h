/*
File:   cross_shader_and_texture.h
Author: Taylor Robbins
Date:   01\30\2025
*/

#ifndef _CROSS_SHADER_AND_TEXTURE_H
#define _CROSS_SHADER_AND_TEXTURE_H

#if BUILD_WITH_SOKOL

#if !PIG_CORE_IMPLEMENTATION
	bool BindTextureAtIndex(sg_bindings* bindings, Shader* shader, Texture* texture, uxx imageIndex, uxx samplerIndex);
	PIG_CORE_INLINE bool BindTextureByName(sg_bindings* bindings, Shader* shader, Texture* texture, Str8 shaderImageName, Str8 shaderSamplerName);
#endif

#if PIG_CORE_IMPLEMENTATION

PEXP bool BindTextureAtIndex(sg_bindings* bindings, Shader* shader, Texture* texture, uxx imageIndex, uxx samplerIndex)
{
	NotNull(bindings);
	NotNull(shader);
	NotNull(texture);
	Assert(imageIndex < SG_MAX_IMAGE_BINDSLOTS);
	Assert(samplerIndex < SG_MAX_SAMPLER_BINDSLOTS);
	bool result = false;
	for (uxx iIndex = 0; iIndex < shader->numImages; iIndex++)
	{
		ShaderImage* image = &shader->images[iIndex];
		if (image->index == imageIndex)
		{
			bindings->images[image->index] = texture->image;
			if (image->sizeUniformIndex < shader->numUniforms)
			{
				ShaderUniform* sizeUniform = &shader->uniforms[image->sizeUniformIndex];
				Assert(sizeUniform->size == sizeof(v2));
				Assert(sizeUniform->blockIndex < MAX_NUM_SHADER_UNIFORM_BLOCKS);
				ShaderUniformBlock* uniformBlock = &shader->uniformBlocks[sizeUniform->blockIndex];
				NotNull(uniformBlock->value.bytes);
				Assert(sizeUniform->offset + sizeUniform->size <= uniformBlock->value.length);
				v2 imageSize = ToV2Fromi(texture->size);
				MyMemCopy(&uniformBlock->value.bytes[sizeUniform->offset], &imageSize, sizeof(v2));
				uniformBlock->valueChanged = true;
			}
			result = true;
		}
	}
	for (uxx sIndex = 0; sIndex < shader->numSamplers; sIndex++)
	{
		ShaderSampler* sampler = &shader->samplers[sIndex];
		if (sampler->index == samplerIndex)
		{
			bindings->samplers[sampler->index] = texture->sampler;
			result = true;
		}
	}
	return result;
}

PEXPI bool BindTextureByName(sg_bindings* bindings, Shader* shader, Texture* texture, Str8 shaderImageName, Str8 shaderSamplerName)
{
	uxx imageIndex = shader->numImages;
	uxx samplerIndex = shader->numSamplers;
	for (uxx iIndex = 0; iIndex < shader->numImages; iIndex++)
	{
		if (StrExactEquals(shader->images[iIndex].name, shaderImageName)) { imageIndex = shader->images[iIndex].index; break; }
	}
	for (uxx sIndex = 0; sIndex < shader->numImages; sIndex++)
	{
		if (StrExactEquals(shader->samplers[sIndex].name, shaderSamplerName)) { samplerIndex = shader->samplers[sIndex].index; break; }
	}
	return BindTextureAtIndex(bindings, shader, texture, imageIndex, samplerIndex);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_SOKOL

#endif //  _CROSS_SHADER_AND_TEXTURE_H
