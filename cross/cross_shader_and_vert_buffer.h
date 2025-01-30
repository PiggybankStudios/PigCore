/*
File:   cross_shader_and_vert_buffer.h
Author: Taylor Robbins
Date:   01\29\2025
*/

#ifndef _CROSS_SHADER_AND_VERT_BUFFER_H
#define _CROSS_SHADER_AND_VERT_BUFFER_H

#if BUILD_WITH_SOKOL

#if !PIG_CORE_IMPLEMENTATION
	void FillPipelineDescLayout(sg_pipeline_desc* pipelineDesc, const Shader* shader, const VertBuffer* bufferRef);
#endif

#if PIG_CORE_IMPLEMENTATION

//NOTE: For now this assumes we have one buffer slot for the pipeline. You cand bind different buffers
//      but they all have to have the same vertex format as the buffer passed here
PEXP void FillPipelineDescLayout(sg_pipeline_desc* pipelineDesc, const Shader* shader, const VertBuffer* bufferRef)
{
	NotNull(shader);
	NotNull(bufferRef);
	NotNull(pipelineDesc);
	Assert(bufferRef->handle.id != SG_INVALID_ID);
	Assert(bufferRef->numAttributes > 0 && bufferRef->numAttributes < MAX_NUM_VERT_ATTRIBUTES);
	pipelineDesc->shader = shader->handle;
	pipelineDesc->layout.buffers[0].stride = (int)bufferRef->vertexSize;
	for (uxx bufferAttribIndex = 0; bufferAttribIndex < bufferRef->numAttributes; bufferAttribIndex++)
	{
		const VertAttribute* bufferAttrib = &bufferRef->attributes[bufferAttribIndex];
		for (uxx shaderAttribIndex = 0; shaderAttribIndex < shader->numAttributes; shaderAttribIndex++)
		{
			const ShaderAttribute* shaderAttrib = &shader->attributes[shaderAttribIndex];
			if (bufferAttrib->type == shaderAttrib->type)
			{
				sg_vertex_format sokolFormat = SG_VERTEXFORMAT_INVALID;
				switch (bufferAttrib->size)
				{
					case sizeof(r32)*1: sokolFormat = SG_VERTEXFORMAT_FLOAT; break;
					case sizeof(r32)*2: sokolFormat = SG_VERTEXFORMAT_FLOAT2; break;
					case sizeof(r32)*3: sokolFormat = SG_VERTEXFORMAT_FLOAT3; break;
					case sizeof(r32)*4: sokolFormat = SG_VERTEXFORMAT_FLOAT4; break;
					default: DebugAssertMsg(false, "Unhandled attribute size in FillPipelineDescLayout!"); break;
				}
				pipelineDesc->layout.attrs[shaderAttrib->index].buffer_index = 0;
				pipelineDesc->layout.attrs[shaderAttrib->index].format = sokolFormat;
				pipelineDesc->layout.attrs[shaderAttrib->index].offset = bufferAttrib->offset;
			}
		}
	}
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_SOKOL

#endif //  _CROSS_SHADER_AND_VERT_BUFFER_H
