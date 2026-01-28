/*
File:   cross_shader_and_texture.h
Author: Taylor Robbins
Date:   01\30\2025
*/

#ifndef _CROSS_SHADER_AND_TEXTURE_H
#define _CROSS_SHADER_AND_TEXTURE_H

#if BUILD_WITH_SOKOL_GFX

#if !PIG_CORE_IMPLEMENTATION
	bool BindTextureInShaderAtIndex(sg_bindings* bindings, Shader* shader, Texture* texture, uxx viewIndex, uxx samplerIndex);
	PIG_CORE_INLINE bool BindTextureInShaderByName(sg_bindings* bindings, Shader* shader, Texture* texture, Str8 shaderViewName, Str8 shaderSamplerName);
#endif

#if PIG_CORE_IMPLEMENTATION

PEXP bool BindTextureInShaderAtIndex(sg_bindings* bindings, Shader* shader, Texture* texture, uxx viewIndex, uxx samplerIndex)
{
	NotNull(bindings);
	NotNull(shader);
	NotNull(texture);
	Assert(viewIndex < SG_MAX_VIEW_BINDSLOTS);
	Assert(samplerIndex < SG_MAX_SAMPLER_BINDSLOTS);
	bool result = false;
	for (uxx vIndex = 0; vIndex < shader->numViews; vIndex++)
	{
		ShaderView* view = &shader->views[vIndex];
		if (view->index == viewIndex)
		{
			// PrintLine_D("Binding View %d at index %llu", texture->view.id, view->index);
			bindings->views[view->index] = texture->view;
			if (view->sizeUniformIndex < shader->numUniforms)
			{
				ShaderUniform* sizeUniform = &shader->uniforms[view->sizeUniformIndex];
				Assert(sizeUniform->size == sizeof(v2));
				Assert(sizeUniform->blockIndex < MAX_NUM_SHADER_UNIFORM_BLOCKS);
				ShaderUniformBlock* uniformBlock = &shader->uniformBlocks[sizeUniform->blockIndex];
				NotNull(uniformBlock->value.bytes);
				Assert(sizeUniform->offset + sizeUniform->size <= uniformBlock->value.length);
				v2 textureSizef = ToV2Fromi(texture->size);
				MyMemCopy(&uniformBlock->value.bytes[sizeUniform->offset], &textureSizef, sizeof(v2));
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
			// PrintLine_D("Binding Sampler %d at index %llu", texture->sampler.id, sampler->index);
			bindings->samplers[sampler->index] = texture->sampler;
			result = true;
		}
	}
	return result;
}

PEXPI bool BindTextureInShaderByName(sg_bindings* bindings, Shader* shader, Texture* texture, Str8 shaderViewName, Str8 shaderSamplerName)
{
	uxx viewIndex = shader->numViews;
	uxx samplerIndex = shader->numSamplers;
	for (uxx vIndex = 0; vIndex < shader->numViews; vIndex++)
	{
		if (StrExactEquals(shader->views[vIndex].name, shaderViewName)) { viewIndex = shader->views[vIndex].index; break; }
	}
	for (uxx sIndex = 0; sIndex < shader->numSamplers; sIndex++)
	{
		if (StrExactEquals(shader->samplers[sIndex].name, shaderSamplerName)) { samplerIndex = shader->samplers[sIndex].index; break; }
	}
	return BindTextureInShaderAtIndex(bindings, shader, texture, viewIndex, samplerIndex);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_SOKOL_GFX

#endif //  _CROSS_SHADER_AND_TEXTURE_H
