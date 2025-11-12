/*
File:   clay_public_api.c
Author: Taylor Robbins
Date:   03\31\2025
Description: 
	** Holds the implementations for the private functions of clay
	** Was a piece of original clay.h (zlib LICENSE attached below)
*/

#ifndef CLAY_WASM
CLAY_DECOR void Clay_SetMeasureTextFunction(ClayMeasureText_f* measureTextFunction, CLAY_MEASURE_USERDATA_TYPE userData)
{
	Clay_Context* context = Clay_GetCurrentContext();
	Clay__MeasureText = measureTextFunction;
	context->measureTextUserData = userData;
}
CLAY_DECOR void Clay_SetHashTextUserDataFunction(ClayHashTextUserData_f* hashTextUserDataFunction)
{
	Clay__HashTextUserData = hashTextUserDataFunction;
}
CLAY_DECOR void Clay_SetQueryScrollOffsetFunction(v2 (*queryScrollOffsetFunction)(u32 elementId, CLAY_QUERYSCROLL_USERDATA_TYPE userData), CLAY_QUERYSCROLL_USERDATA_TYPE userData)
{
	Clay_Context* context = Clay_GetCurrentContext();
	Clay__QueryScrollOffset = queryScrollOffsetFunction;
	context->queryScrollOffsetUserData = userData;
}
#endif

WASM_EXPORT("Clay_SetLayoutDimensions")
CLAY_DECOR void Clay_SetLayoutDimensions(v2 dimensions)
{
	Clay_GetCurrentContext()->layoutDimensions = dimensions;
}

WASM_EXPORT("Clay_SetPointerState")
CLAY_DECOR void Clay_SetPointerState(v2 position, bool isPointerDown)
{
	Clay_Context* context = Clay_GetCurrentContext();
	if (context->booleanWarnings.maxElementsExceeded) { return; }
	context->pointerInfo.position = position;
	context->pointerOverIds.length = 0;
	i32Array dfsBuffer = context->layoutElementChildrenBuffer;
	for (i32 rootIndex = context->layoutElementTreeRoots.length - 1; rootIndex >= 0; --rootIndex)
	{
		dfsBuffer.length = 0;
		Clay__LayoutElementTreeRoot* root = Clay__LayoutElementTreeRootArray_Get(&context->layoutElementTreeRoots, rootIndex);
		i32Array_Add(&dfsBuffer, (i32)root->layoutElementIndex);
		context->treeNodeVisited.items[0] = false;
		bool found = false;
		while (dfsBuffer.length > 0)
		{
			if (context->treeNodeVisited.items[dfsBuffer.length - 1])
			{
				dfsBuffer.length--;
				continue;
			}
			context->treeNodeVisited.items[dfsBuffer.length - 1] = true;
			Clay_LayoutElement* currentElement = Clay_LayoutElementArray_Get(&context->layoutElements, i32Array_GetValue(&dfsBuffer, (int)dfsBuffer.length - 1));
			Clay_LayoutElementHashMapItem* mapItem = Clay__GetHashMapItem(currentElement->id); // TODO think of a way around this, maybe the fact that it's essentially a binary tree limits the cost, but the worst case is not great
			rec elementBox = mapItem->boundingBox;
			elementBox.X -= root->pointerOffset.X;
			elementBox.Y -= root->pointerOffset.Y;
			if (mapItem)
			{
				if ((Clay__PointIsInsideRect(position, elementBox)))
				{
					if (mapItem->onHoverFunction)
					{
						mapItem->onHoverFunction(mapItem->elementId, context->pointerInfo, mapItem->hoverFunctionUserData);
					}
					Clay__ElementIdArray_Add(&context->pointerOverIds, mapItem->elementId);
					found = true;
					
					if (mapItem->idAlias != 0)
					{
						Clay__ElementIdArray_Add(&context->pointerOverIds, NEW_STRUCT(Clay_ElementId) { .id = mapItem->idAlias });
					}
				}
				if (Clay__ElementHasConfig(currentElement, CLAY__ELEMENT_CONFIG_TYPE_TEXT))
				{
					dfsBuffer.length--;
					continue;
				}
				for (i32 i = currentElement->childrenOrTextContent.children.length - 1; i >= 0; --i)
				{
					i32Array_Add(&dfsBuffer, currentElement->childrenOrTextContent.children.elements[i]);
					context->treeNodeVisited.items[dfsBuffer.length - 1] = false; // TODO needs to be ranged checked
				}
			}
			else
			{
				dfsBuffer.length--;
			}
		}
		
		Clay_LayoutElement* rootElement = Clay_LayoutElementArray_Get(&context->layoutElements, root->layoutElementIndex);
		if (found && Clay__ElementHasConfig(rootElement, CLAY__ELEMENT_CONFIG_TYPE_FLOATING) &&
			Clay__FindElementConfigWithType(rootElement, CLAY__ELEMENT_CONFIG_TYPE_FLOATING).floatingElementConfig->pointerCaptureMode == CLAY_POINTER_CAPTURE_MODE_CAPTURE)
		{
			break;
		}
	}
	
	if (isPointerDown)
	{
		if (context->pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME)
		{
			context->pointerInfo.state = CLAY_POINTER_DATA_PRESSED;
		}
		else if (context->pointerInfo.state != CLAY_POINTER_DATA_PRESSED)
		{
			context->pointerInfo.state = CLAY_POINTER_DATA_PRESSED_THIS_FRAME;
		}
	} 
	else
	{
		if (context->pointerInfo.state == CLAY_POINTER_DATA_RELEASED_THIS_FRAME)
		{
			context->pointerInfo.state = CLAY_POINTER_DATA_RELEASED;
		}
		else if (context->pointerInfo.state != CLAY_POINTER_DATA_RELEASED)
		{
			context->pointerInfo.state = CLAY_POINTER_DATA_RELEASED_THIS_FRAME;
		}
	}
}

WASM_EXPORT("Clay_Initialize")
CLAY_DECOR Clay_Context* Clay_Initialize(Arena* arena, v2 layoutDimensions, Clay_ErrorHandler errorHandler)
{
	Clay_Context* context = AllocType(Clay_Context, arena);
	if (context == nullptr) { return nullptr; }
	// DEFAULTS
	Clay_Context* oldContext = Clay_GetCurrentContext();
	*context = NEW_STRUCT(Clay_Context) {
		.maxElementCount = oldContext ? oldContext->maxElementCount : Clay__defaultMaxElementCount,
		.maxMeasureTextCacheWordCount = oldContext ? oldContext->maxMeasureTextCacheWordCount : Clay__defaultMaxMeasureTextWordCacheCount,
		.errorHandler = errorHandler.errorHandlerFunction ? errorHandler : NEW_STRUCT(Clay_ErrorHandler) { Clay__ErrorHandlerFunctionDefault, 0 },
		.layoutDimensions = layoutDimensions,
		.internalArena = arena,
	};
	Clay_SetCurrentContext(context);
	Clay__InitializePersistentMemory(context);
	Clay__InitializeEphemeralMemory(context);
	for (uxx iIndex = 0; iIndex < context->layoutElementsHashMap.allocLength; ++iIndex)
	{
		context->layoutElementsHashMap.items[iIndex] = -1;
	}
	for (uxx iIndex = 0; iIndex < context->measureTextHashMap.allocLength; ++iIndex)
	{
		context->measureTextHashMap.items[iIndex] = 0;
	}
	context->measureTextHashMapInternal.length = 1; // Reserve the 0 value to mean "no next element"
	context->layoutDimensions = layoutDimensions;
	return context;
}

WASM_EXPORT("Clay_GetCurrentContext")
CLAY_DECOR Clay_Context* Clay_GetCurrentContext(void)
{
	return Clay__currentContext;
}

WASM_EXPORT("Clay_SetCurrentContext")
CLAY_DECOR void Clay_SetCurrentContext(Clay_Context* context)
{
	Clay__currentContext = context;
}

WASM_EXPORT("Clay_UpdateScrollContainers")
CLAY_DECOR bool Clay_UpdateScrollContainers(bool enableDragScrolling, v2 scrollDelta, r32 deltaTime)
{
	Clay_Context* context = Clay_GetCurrentContext();
	bool isAutoScrollingOccurring = false;
	bool isPointerActive = enableDragScrolling && (context->pointerInfo.state == CLAY_POINTER_DATA_PRESSED || context->pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME);
	// Don't apply scroll events to ancestors of the inner element
	uxx highestPriorityElementIndex = UINTXX_MAX;
	Clay__ScrollContainerDataInternal* highestPriorityScrollData = nullptr;
	for (uxx sIndex = 0; sIndex < context->scrollContainerDatas.length; sIndex++)
	{
		Clay__ScrollContainerDataInternal* scrollData = Clay__ScrollContainerDataInternalArray_Get(&context->scrollContainerDatas, sIndex);
		if (!scrollData->openThisFrame)
		{
			Clay__ScrollContainerDataInternalArray_RemoveSwapback(&context->scrollContainerDatas, sIndex);
			continue;
		}
		scrollData->openThisFrame = false;
		Clay_LayoutElementHashMapItem* hashMapItem = Clay__GetHashMapItem(scrollData->elementId);
		// Element isn't rendered this frame but scroll offset has been retained
		if (!hashMapItem)
		{
			Clay__ScrollContainerDataInternalArray_RemoveSwapback(&context->scrollContainerDatas, sIndex);
			continue;
		}
		
		// Touch / click is released
		if (!isPointerActive && scrollData->pointerScrollActive)
		{
			r32 xDiff = scrollData->scrollTarget.X - scrollData->scrollOrigin.X;
			if (xDiff < -10 || xDiff > 10)
			{
				scrollData->scrollMomentum.X = (scrollData->scrollTarget.X - scrollData->scrollOrigin.X) / (scrollData->momentumTime * 25);
			}
			r32 yDiff = scrollData->scrollTarget.Y - scrollData->scrollOrigin.Y;
			if (yDiff < -10 || yDiff > 10)
			{
				scrollData->scrollMomentum.Y = (scrollData->scrollTarget.Y - scrollData->scrollOrigin.Y) / (scrollData->momentumTime * 25);
			}
			scrollData->pointerScrollActive = false;
			
			scrollData->pointerOrigin = V2_Zero;
			scrollData->scrollOrigin = V2_Zero;
			scrollData->momentumTime = 0;
		}
		
		bool scrollMomentumOccurring = (scrollData->scrollMomentum.X != 0 || scrollData->scrollMomentum.Y != 0);
		if (scrollMomentumOccurring) { isAutoScrollingOccurring = true; }
		
		// Apply existing momentum
		scrollData->scrollTarget.X += scrollData->scrollMomentum.X;
		scrollData->scrollMomentum.X *= 0.95f;
		bool scrollOccurred = scrollDelta.X != 0 || scrollDelta.Y != 0;
		if ((scrollData->scrollMomentum.X > -0.1f && scrollData->scrollMomentum.X < 0.1f) || scrollOccurred)
		{
			scrollData->scrollMomentum.X = 0;
		}
		scrollData->scrollTarget.X = MinR32(MaxR32(scrollData->scrollTarget.X, -(MaxR32(scrollData->contentSize.Width - scrollData->layoutElement->dimensions.Width, 0))), 0);
		
		scrollData->scrollTarget.Y += scrollData->scrollMomentum.Y;
		scrollData->scrollMomentum.Y *= 0.95f;
		if ((scrollData->scrollMomentum.Y > -0.1f && scrollData->scrollMomentum.Y < 0.1f) || scrollOccurred)
		{
			scrollData->scrollMomentum.Y = 0;
		}
		scrollData->scrollTarget.Y = MinR32(MaxR32(scrollData->scrollTarget.Y, -(MaxR32(scrollData->contentSize.Height - scrollData->layoutElement->dimensions.Height, 0))), 0);
		
		// Update scrollPosition to scrollTarget with scrollLag taken into account
		if (scrollData->scrollLag == 0 || scrollMomentumOccurring || isPointerActive)
		{
			scrollData->scrollPosition = scrollData->scrollTarget;
		}
		else
		{
			v2 targetDelta = MakeV2(
				scrollData->scrollTarget.X - scrollData->scrollPosition.X,
				scrollData->scrollTarget.Y - scrollData->scrollPosition.Y
			);
			r32 targetDistanceSquared = (targetDelta.X * targetDelta.X) + (targetDelta.Y * targetDelta.Y);
			if (targetDistanceSquared >= 1.0f)
			{
				//TODO: We should do the proper framerate independent calculation here!
				scrollData->scrollPosition.X += targetDelta.X / scrollData->scrollLag;
				scrollData->scrollPosition.Y += targetDelta.Y / scrollData->scrollLag;
				isAutoScrollingOccurring = true;
			}
			else
			{
				scrollData->scrollPosition = scrollData->scrollTarget;
			}
		}
		
		for (uxx pIndex = 0; pIndex < context->pointerOverIds.length; ++pIndex) // TODO n & m are small here but this being n*m gives me the creeps
		{
			if (scrollData->layoutElement->id == Clay__ElementIdArray_Get(&context->pointerOverIds, pIndex)->id)
			{
				highestPriorityElementIndex = pIndex;
				highestPriorityScrollData = scrollData;
			}
		}
	}
	
	if (highestPriorityElementIndex < UINTXX_MAX && highestPriorityScrollData)
	{
		Clay_LayoutElement* scrollElement = highestPriorityScrollData->layoutElement;
		Clay_ScrollElementConfig* scrollConfig = Clay__FindElementConfigWithType(scrollElement, CLAY__ELEMENT_CONFIG_TYPE_SCROLL).scrollElementConfig;
		highestPriorityScrollData->scrollLag = scrollConfig->scrollLag;
		bool canScrollVertically = scrollConfig->vertical && highestPriorityScrollData->contentSize.Height > scrollElement->dimensions.Height;
		bool canScrollHorizontally = scrollConfig->horizontal && highestPriorityScrollData->contentSize.Width > scrollElement->dimensions.Width;
		// Handle wheel scroll
		if (canScrollVertically)
		{
			highestPriorityScrollData->scrollTarget.Y = highestPriorityScrollData->scrollTarget.Y + scrollDelta.Y * 10;
		}
		if (canScrollHorizontally)
		{
			highestPriorityScrollData->scrollTarget.X = highestPriorityScrollData->scrollTarget.X + scrollDelta.X * 10;
		}
		// Handle click / touch scroll
		if (isPointerActive)
		{
			highestPriorityScrollData->scrollMomentum = V2_Zero;
			if (!highestPriorityScrollData->pointerScrollActive)
			{
				highestPriorityScrollData->pointerOrigin = context->pointerInfo.position;
				highestPriorityScrollData->scrollOrigin = highestPriorityScrollData->scrollTarget;
				highestPriorityScrollData->pointerScrollActive = true;
			}
			else
			{
				r32 scrollDeltaX = 0, scrollDeltaY = 0;
				if (canScrollHorizontally)
				{
					r32 oldXScrollPosition = highestPriorityScrollData->scrollTarget.X;
					highestPriorityScrollData->scrollTarget.X = highestPriorityScrollData->scrollOrigin.X + (context->pointerInfo.position.X - highestPriorityScrollData->pointerOrigin.X);
					highestPriorityScrollData->scrollTarget.X = MaxR32(MinR32(highestPriorityScrollData->scrollTarget.X, 0), -(highestPriorityScrollData->contentSize.Width - highestPriorityScrollData->boundingBox.Width));
					scrollDeltaX = highestPriorityScrollData->scrollTarget.X - oldXScrollPosition;
				}
				if (canScrollVertically)
				{
					r32 oldYScrollPosition = highestPriorityScrollData->scrollTarget.Y;
					highestPriorityScrollData->scrollTarget.Y = highestPriorityScrollData->scrollOrigin.Y + (context->pointerInfo.position.Y - highestPriorityScrollData->pointerOrigin.Y);
					highestPriorityScrollData->scrollTarget.Y = MaxR32(MinR32(highestPriorityScrollData->scrollTarget.Y, 0), -(highestPriorityScrollData->contentSize.Height - highestPriorityScrollData->boundingBox.Height));
					scrollDeltaY = highestPriorityScrollData->scrollTarget.Y - oldYScrollPosition;
				}
				if (scrollDeltaX > -0.1f && scrollDeltaX < 0.1f && scrollDeltaY > -0.1f && scrollDeltaY < 0.1f && highestPriorityScrollData->momentumTime > 0.15f)
				{
					highestPriorityScrollData->momentumTime = 0;
					highestPriorityScrollData->pointerOrigin = context->pointerInfo.position;
					highestPriorityScrollData->scrollOrigin = highestPriorityScrollData->scrollTarget;
				}
				else
				{
					 highestPriorityScrollData->momentumTime += deltaTime;
				}
			}
		}
		// Clamp any changes to scroll position to the maximum size of the contents
		if (canScrollVertically)
		{
			highestPriorityScrollData->scrollPosition.Y = MaxR32(MinR32(highestPriorityScrollData->scrollPosition.Y, 0), -(highestPriorityScrollData->contentSize.Height - scrollElement->dimensions.Height));
			highestPriorityScrollData->scrollTarget.Y = MaxR32(MinR32(highestPriorityScrollData->scrollTarget.Y, 0), -(highestPriorityScrollData->contentSize.Height - scrollElement->dimensions.Height));
		}
		if (canScrollHorizontally)
		{
			highestPriorityScrollData->scrollPosition.X = MaxR32(MinR32(highestPriorityScrollData->scrollPosition.X, 0), -(highestPriorityScrollData->contentSize.Width - scrollElement->dimensions.Width));
			highestPriorityScrollData->scrollTarget.X = MaxR32(MinR32(highestPriorityScrollData->scrollTarget.X, 0), -(highestPriorityScrollData->contentSize.Width - scrollElement->dimensions.Width));
		}
		
		//If no scrollLag, or currently scrolling with touch, immediately move scrollPosition to scrollTarget rather than waiting for next frame
		if (highestPriorityScrollData->scrollLag == 0 || isPointerActive)
		{
			highestPriorityScrollData->scrollPosition = highestPriorityScrollData->scrollTarget;
		}
	}
	return isAutoScrollingOccurring;
}

WASM_EXPORT("Clay_BeginLayout")
CLAY_DECOR void Clay_BeginLayout(void)
{
	Clay_Context* context = Clay_GetCurrentContext();
	Clay__FreeEphemeralMemory(context);
	Clay__InitializeEphemeralMemory(context);
	context->generation++;
	context->dynamicElementIndex = 0;
	// Set up the root container that covers the entire window
	v2 rootDimensions = MakeV2(context->layoutDimensions.Width, context->layoutDimensions.Height);
	if (context->debugModeEnabled) { rootDimensions.Width -= (r32)Clay__debugViewWidth; }
	context->booleanWarnings = NEW_STRUCT(Clay_BooleanWarnings) ZEROED;
	Clay__OpenElement();
	Clay__ConfigureOpenElement(NEW_STRUCT(Clay_ElementDeclaration) {
		.id = CLAY_ID("Clay__RootContainer"),
		.layout = { .sizing = {CLAY_SIZING_FIXED((rootDimensions.Width)), CLAY_SIZING_FIXED(rootDimensions.Height)} }
	});
	i32Array_Add(&context->openLayoutElementStack, 0);
	Clay__LayoutElementTreeRootArray_Add(&context->layoutElementTreeRoots, NEW_STRUCT(Clay__LayoutElementTreeRoot) { .layoutElementIndex = 0 });
}

WASM_EXPORT("Clay_EndLayout")
CLAY_DECOR Clay_RenderCommandArray Clay_EndLayout(void)
{
	Clay_Context* context = Clay_GetCurrentContext();
	Clay__CloseElement();
	bool elementsExceededBeforeDebugView = context->booleanWarnings.maxElementsExceeded;
	if (context->debugModeEnabled && !elementsExceededBeforeDebugView)
	{
		context->warningsEnabled = false;
		Clay__RenderDebugView();
		context->warningsEnabled = true;
	}
	if (context->booleanWarnings.maxElementsExceeded)
	{
		Str8 message;
		if (!elementsExceededBeforeDebugView)
		{
			message = StrLit("Clay Error: Layout elements exceeded Clay__maxElementCount after adding the debug-view to the layout.");
		}
		else
		{
			message = StrLit("Clay Error: Layout elements exceeded Clay__maxElementCount");
		}
		Clay__AddRenderCommand(NEW_STRUCT(Clay_RenderCommand ) {
			.boundingBox = MakeRec(context->layoutDimensions.Width / 2 - 59 * 4, context->layoutDimensions.Height / 2, 0, 0),
			.renderData = { .text = { .stringContents = message, .textColor = {.valueU32=0xFFFF0000}, .fontSize = 16 } }, //(255, 0, 0, 255)
			.commandType = CLAY_RENDER_COMMAND_TYPE_TEXT
		});
	}
	else
	{
		Clay__CalculateFinalLayout();
	}
	return context->renderCommands;
}

WASM_EXPORT("Clay_GetElementId")
CLAY_DECOR Clay_ElementId Clay_GetElementId(Str8 idString)
{
	return Clay__HashString(idString, 0, 0);
}

WASM_EXPORT("Clay_GetElementIdWithIndex")
CLAY_DECOR Clay_ElementId Clay_GetElementIdWithIndex(Str8 idString, u32 index)
{
	return Clay__HashString(idString, index, 0);
}

CLAY_DECOR bool Clay_Hovered(void)
{
	Clay_Context* context = Clay_GetCurrentContext();
	if (context->booleanWarnings.maxElementsExceeded) { return false; }
	Clay_LayoutElement* openLayoutElement = Clay__GetOpenLayoutElement();
	// If the element has no id attached at this point, we need to generate one
	if (openLayoutElement->id == 0)
	{
		Clay__GenerateIdForAnonymousElement(openLayoutElement);
	}
	for (uxx iIndex = 0; iIndex < context->pointerOverIds.length; ++iIndex)
	{
		if (Clay__ElementIdArray_Get(&context->pointerOverIds, iIndex)->id == openLayoutElement->id)
		{
			return true;
		}
	}
	return false;
}

CLAY_DECOR void Clay_OnHover(void (*onHoverFunction)(Clay_ElementId elementId, Clay_PointerData pointerInfo, CLAY_ONHOVER_USERDATA_TYPE userData), CLAY_ONHOVER_USERDATA_TYPE userData)
{
	Clay_Context* context = Clay_GetCurrentContext();
	if (context->booleanWarnings.maxElementsExceeded) { return; }
	Clay_LayoutElement* openLayoutElement = Clay__GetOpenLayoutElement();
	if (openLayoutElement->id == 0) { Clay__GenerateIdForAnonymousElement(openLayoutElement); }
	Clay_LayoutElementHashMapItem* hashMapItem = Clay__GetHashMapItem(openLayoutElement->id);
	hashMapItem->onHoverFunction = onHoverFunction;
	hashMapItem->hoverFunctionUserData = userData;
}

WASM_EXPORT("Clay_PointerOver")
CLAY_DECOR bool Clay_PointerOver(Clay_ElementId elementId) // TODO return priority for separating multiple results
{
	Clay_Context* context = Clay_GetCurrentContext();
	for (uxx iIndex = 0; iIndex < context->pointerOverIds.length; ++iIndex)
	{
		if (Clay__ElementIdArray_Get(&context->pointerOverIds, iIndex)->id == elementId.id)
		{
			return true;
		}
	}
	return false;
}

WASM_EXPORT("Clay_GetScrollContainerData")
CLAY_DECOR Clay_ScrollContainerData Clay_GetScrollContainerData(Clay_ElementId id, bool getConfig)
{
	Clay_Context* context = Clay_GetCurrentContext();
	for (uxx sIndex = 0; sIndex < context->scrollContainerDatas.length; ++sIndex)
	{
		Clay__ScrollContainerDataInternal* scrollContainerData = Clay__ScrollContainerDataInternalArray_Get(&context->scrollContainerDatas, sIndex);
		if (scrollContainerData->elementId == id.id)
		{
			//TODO: Remove the usage of layoutElement, and don't return scrollConfig? Or make sure that layoutElement comes from this frame before passing it back to the calling code?
			Clay_ScrollElementConfig* scrollConfig = getConfig ? Clay__FindElementConfigWithType(scrollContainerData->layoutElement, CLAY__ELEMENT_CONFIG_TYPE_SCROLL).scrollElementConfig : nullptr;
			if (!getConfig || scrollConfig != nullptr)
			{
				return NEW_STRUCT(Clay_ScrollContainerData) {
					.scrollTarget = &scrollContainerData->scrollTarget,
					.scrollPosition = &scrollContainerData->scrollPosition,
					.scrollContainerDimensions = MakeV2(scrollContainerData->boundingBox.Width, scrollContainerData->boundingBox.Height),
					.contentDimensions = scrollContainerData->contentSize,
					.config = (scrollConfig != nullptr ? *scrollConfig : NEW_STRUCT(Clay_ScrollElementConfig) ZEROED),
					.found = true
				};
			}
		}
	}
	return NEW_STRUCT(Clay_ScrollContainerData) ZEROED;
}

WASM_EXPORT("Clay_GetElementData")
CLAY_DECOR Clay_ElementData Clay_GetElementData(Clay_ElementId id)
{
	Clay_LayoutElementHashMapItem* item = Clay__GetHashMapItem(id.id);
	if(item == &Clay_LayoutElementHashMapItem_DEFAULT)
	{
		return NEW_STRUCT(Clay_ElementData) ZEROED;
	}
	
	return NEW_STRUCT(Clay_ElementData){
		.boundingBox = item->boundingBox,
		.found = true
	};
}

WASM_EXPORT("Clay_SetDebugModeEnabled")
CLAY_DECOR void Clay_SetDebugModeEnabled(bool enabled)
{
	Clay_Context* context = Clay_GetCurrentContext();
	context->debugModeEnabled = enabled;
}

WASM_EXPORT("Clay_IsDebugModeEnabled")
CLAY_DECOR bool Clay_IsDebugModeEnabled(void)
{
	Clay_Context* context = Clay_GetCurrentContext();
	return context->debugModeEnabled;
}

WASM_EXPORT("Clay_SetCullingEnabled")
CLAY_DECOR void Clay_SetCullingEnabled(bool enabled)
{
	Clay_Context* context = Clay_GetCurrentContext();
	context->disableCulling = !enabled;
}

WASM_EXPORT("Clay_SetExternalScrollHandlingEnabled")
void Clay_SetExternalScrollHandlingEnabled(bool enabled)
{
	Clay_Context* context = Clay_GetCurrentContext();
	context->externalScrollHandlingEnabled = enabled;
}

WASM_EXPORT("Clay_GetMaxElementCount")
CLAY_DECOR i32 Clay_GetMaxElementCount(void)
{
	Clay_Context* context = Clay_GetCurrentContext();
	return context->maxElementCount;
}

WASM_EXPORT("Clay_SetMaxElementCount")
CLAY_DECOR void Clay_SetMaxElementCount(i32 maxElementCount)
{
	Clay_Context* context = Clay_GetCurrentContext();
	if (context)
	{
		context->maxElementCount = maxElementCount;
	}
	else
	{
		Clay__defaultMaxElementCount = maxElementCount; // TODO: Fix this
		Clay__defaultMaxMeasureTextWordCacheCount = maxElementCount * 2;
	}
}

WASM_EXPORT("Clay_GetMaxMeasureTextCacheWordCount")
CLAY_DECOR i32 Clay_GetMaxMeasureTextCacheWordCount(void)
{
	Clay_Context* context = Clay_GetCurrentContext();
	return context->maxMeasureTextCacheWordCount;
}

WASM_EXPORT("Clay_SetMaxMeasureTextCacheWordCount")
CLAY_DECOR void Clay_SetMaxMeasureTextCacheWordCount(i32 maxMeasureTextCacheWordCount)
{
	Clay_Context* context = Clay_GetCurrentContext();
	if (context)
	{
		Clay__currentContext->maxMeasureTextCacheWordCount = maxMeasureTextCacheWordCount;
	}
	else
	{
		Clay__defaultMaxMeasureTextWordCacheCount = maxMeasureTextCacheWordCount; // TODO: Fix this
	}
}

WASM_EXPORT("Clay_ResetMeasureTextCache")
CLAY_DECOR void Clay_ResetMeasureTextCache(void)
{
	Clay_Context* context = Clay_GetCurrentContext();
	context->measureTextHashMapInternal.length = 0;
	context->measureTextHashMapInternalFreeList.length = 0;
	context->measureTextHashMap.length = 0;
	context->measuredWords.length = 0;
	context->measuredWordsFreeList.length = 0;
	
	for (uxx mIndex = 0; mIndex < context->measureTextHashMap.allocLength; ++mIndex)
	{
		context->measureTextHashMap.items[mIndex] = 0;
	}
	context->measureTextHashMapInternal.length = 1; // Reserve the 0 value to mean "no next element"
}

/*
LICENSE
zlib/libpng license

Copyright (c) 2024 Nic Barker

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the
use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

	1. The origin of this software must not be misrepresented; you must not
	claim that you wrote the original software. If you use this software in a
	product, an acknowledgment in the product documentation would be
	appreciated but is not required.

	2. Altered source versions must be plainly marked as such, and must not
	be misrepresented as being the original software.

	3. This notice may not be removed or altered from any source
	distribution.
*/
