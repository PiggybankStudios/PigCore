/*
File:   cross_imgui_and_gfx_system.h
Author: Taylor Robbins
Date:   02\21\2025
*/

#ifndef _CROSS_IMGUI_AND_GFX_SYSTEM_H
#define _CROSS_IMGUI_AND_GFX_SYSTEM_H

//NOTE: Intentionally no includes here

#if BUILD_WITH_IMGUI

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	void GfxSystem_ImguiBeginFrame(GfxSystem* system, ImguiUI* imgui);
	void GfxSystem_RenderImDrawData(GfxSystem* system, Arena* vertBufferArena, VertBuffer* vertBuffer, ImDrawData* drawData);
	void GfxSystem_ImguiEndFrame(GfxSystem* system, ImguiUI* imgui);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

//NOTE: You should fill/handle imgui->io inputs/outputs like DeltaTime, WantSetMousePos, WantTextInput BEFORE calling this function!
PEXP void GfxSystem_ImguiBeginFrame(GfxSystem* system, ImguiUI* imgui)
{
	NotNull(system);
	Assert(system->frameStarted);
	NotNull(imgui);
	NotNull(imgui->context);
	NotNull(imgui->io);
	Assert(!imgui->frameStarted);
	
	igSetCurrentContext(imgui->context);
	
	imgui->io->DisplaySize = ToImVec2(ToV2Fromi(system->screenSize));
	imgui->io->DisplayFramebufferScale = ToImVec2(FillV2(1.0f)); //TODO: Fill this with real info!
	
	igNewFrame();
	imgui->frameStarted = true;
}

PEXP void GfxSystem_RenderImDrawData(GfxSystem* system, Arena* vertBufferArena, VertBuffer* vertBuffer, const ImDrawData* drawData)
{
	NotNull(system);
	Assert(system->frameStarted);
	NotNull(vertBufferArena);
	NotNull(vertBuffer);
	NotNull(drawData);
	
	if (drawData->TotalVtxCount == 0) { return; } //nothing to draw
	
	// +==============================+
	// | Update Vertices and Indices  |
	// +==============================+
	{
		ScratchBegin1(scratch, vertBufferArena);
		
		Vertex2D* scratchVertices = AllocArray(Vertex2D, scratch, drawData->TotalVtxCount);
		NotNull(scratchVertices);
		u16* scratchIndices = AllocArray(u16, scratch, drawData->TotalIdxCount);
		NotNull(scratchIndices);
		uxx vertexOffset = 0;
		uxx indexOffset = 0;
		
		for (int lIndex = 0; lIndex < drawData->CmdListsCount; lIndex++)
		{
			const ImDrawList* cmdDrawList = drawData->CmdLists.Data[lIndex];
			if (cmdDrawList->VtxBuffer.Size > 0)
			{
				DebugAssert(vertexOffset + cmdDrawList->VtxBuffer.Size <= drawData->TotalVtxCount);
				Vertex2D* verticesPntr = &scratchVertices[vertexOffset];
				vertexOffset += cmdDrawList->VtxBuffer.Size;
				for (int vIndex = 0; vIndex < cmdDrawList->VtxBuffer.Size; vIndex++)
				{
					const ImDrawVert* imVert = &cmdDrawList->VtxBuffer.Data[vIndex];
					verticesPntr[vIndex].position = ToV2FromImgui(imVert->pos);
					verticesPntr[vIndex].texCoord = ToV2FromImgui(imVert->uv);
					verticesPntr[vIndex].color = ToV4rFromColor32(MakeColorU32(
						(imVert->col & 0xFF00FF00) |
						((imVert->col & 0x00FF0000) >> 16) |
						((imVert->col & 0x000000FF) << 16)
					));
				}
			}
			
			if (cmdDrawList->IdxBuffer.Size > 0)
			{
				DebugAssert(indexOffset + cmdDrawList->IdxBuffer.Size <= drawData->TotalIdxCount);
				u16* indicesPntr = &scratchIndices[indexOffset];
				indexOffset += cmdDrawList->IdxBuffer.Size;
				MyMemCopy(indicesPntr, cmdDrawList->IdxBuffer.Data, sizeof(u16) * cmdDrawList->IdxBuffer.Size);
			}
		}
		
		//Resize/Create vertBuffer if needed
		if (vertBuffer->arena == nullptr ||
			vertBuffer->numVertices < drawData->TotalVtxCount ||
			vertBuffer->numIndices < drawData->TotalIdxCount)
		{
			if (vertBuffer->arena != nullptr) { FreeVertBuffer(vertBuffer); }
			*vertBuffer = InitVertBuffer2D(vertBufferArena, StrLit("ImGuiVertBuffer"), VertBufferUsage_Streaming, drawData->TotalVtxCount, nullptr, false);
			Assert(vertBuffer->error == Result_Success);
			AddIndicesToVertBufferU16(vertBuffer, drawData->TotalIdxCount, nullptr, false);
			ChangeVerticesInVertBuffer2D(vertBuffer, drawData->TotalVtxCount, scratchVertices);
			ChangeIndicesInVertBufferU16(vertBuffer, drawData->TotalIdxCount, scratchIndices);
		}
		else
		{
			ChangeVerticesInVertBuffer2D(vertBuffer, drawData->TotalVtxCount, scratchVertices);
			ChangeIndicesInVertBufferU16(vertBuffer, drawData->TotalIdxCount, scratchIndices);
		}
		
		ScratchEnd(scratch);
	}
	
	
	// +==============================+
	// |     Execute the Commands     |
	// +==============================+
	{
		reci oldClipRec = system->state.clipRec;
		bool oldCullingEnabled = system->state.cullingEnabled;
		GfxSystem_BindVertBuffer(system, vertBuffer);
		GfxSystem_SetCullingEnabled(system, false);
		uxx vertexOffset = 0;
		uxx indexOffset = 0;
		for (int lIndex = 0; lIndex < drawData->CmdListsCount; lIndex++)
		{
			const ImDrawList* cmdDrawList = drawData->CmdLists.Data[lIndex];
			for (int cIndex = 0; cIndex < cmdDrawList->CmdBuffer.Size; cIndex++)
			{
				const ImDrawCmd* cmd = &cmdDrawList->CmdBuffer.Data[cIndex];
				if (cmd->UserCallback == ImDrawCallback_ResetRenderState)
				{
					Assert(false); //TODO: Implement me!
				}
				else if (cmd->UserCallback != nullptr)
				{
					cmd->UserCallback(cmdDrawList, cmd);
				}
				else
				{
					GfxSystem_SetWorldMat(system, Mat4_Identity);
					GfxSystem_SetClipRec(system, ToReciFromf(MakeRec(cmd->ClipRect.x, cmd->ClipRect.y, cmd->ClipRect.z - cmd->ClipRect.x, cmd->ClipRect.w - cmd->ClipRect.y)));
					Texture* texture = (Texture*)ImDrawCmd_GetTexID((ImDrawCmd*)cmd);
					GfxSystem_BindTexture(system, texture);
					GfxSystem_SetSourceRec(system, MakeRecV(V2_Zero, (texture != nullptr) ? ToV2Fromi(texture->size) : V2_One));
					GfxSystem_SetTintColorRaw(system, FillV4r(1));
					GfxSystem_SetVertexOffset(system, sizeof(Vertex2D) * (vertexOffset + cmd->VtxOffset));
					GfxSystem_DrawVerticesEx(system, indexOffset + cmd->IdxOffset, cmd->ElemCount);
				}
			}
			vertexOffset += cmdDrawList->VtxBuffer.Size;
			indexOffset += cmdDrawList->IdxBuffer.Size;
		}
		
		GfxSystem_SetVertexOffset(system, 0);
		GfxSystem_SetClipRec(system, oldClipRec);
		GfxSystem_SetCullingEnabled(system, oldCullingEnabled);
	}
}

PEXP void GfxSystem_ImguiEndFrame(GfxSystem* system, ImguiUI* imgui)
{
	NotNull(system);
	Assert(system->frameStarted);
	NotNull(imgui);
	NotNull(imgui->context);
	Assert(imgui->frameStarted);
	
	igRender();
	imgui->frameStarted = false;
	
	ImDrawData* imDrawData = igGetDrawData();
	GfxSystem_RenderImDrawData(system, imgui->arena, &imgui->vertBuffer, imDrawData);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_IMGUI

#endif //  _CROSS_IMGUI_AND_GFX_SYSTEM_H
