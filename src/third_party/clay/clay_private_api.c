/*
File:   clay_api.c
Author: Taylor Robbins
Date:   03\31\2025
Description: 
	** Holds the implementations for the private functions of clay
	** Was a piece of original clay.h (zlib LICENSE attached below)
*/

Str8 Clay__WriteStringToCharBuffer(charArray* buffer, Str8 string)
{
	for (uxx cIndex = 0; cIndex < string.length; cIndex++)
	{
		buffer->items[buffer->length + cIndex] = string.chars[cIndex];
	}
	buffer->length += string.length;
	return MakeStr8(string.length, buffer->items + buffer->length - string.length);
}

Clay_LayoutElement* Clay__GetOpenLayoutElement(void)
{
	Clay_Context* context = Clay_GetCurrentContext();
	return Clay_LayoutElementArray_Get(&context->layoutElements, i32Array_GetValue(&context->openLayoutElementStack, context->openLayoutElementStack.length - 1));
}
CLAY_DECOR u32 Clay__GetParentElementId(void)
{
	Clay_Context* context = Clay_GetCurrentContext();
	return Clay_LayoutElementArray_Get(&context->layoutElements, i32Array_GetValue(&context->openLayoutElementStack, context->openLayoutElementStack.length - 2))->id;
}

Clay_LayoutConfig* Clay__StoreLayoutConfig(Clay_LayoutConfig config) {  return Clay_GetCurrentContext()->booleanWarnings.maxElementsExceeded ? &CLAY_LAYOUT_DEFAULT : Clay__LayoutConfigArray_Add(&Clay_GetCurrentContext()->layoutConfigs, config); }
CLAY_DECOR Clay_TextElementConfig* Clay__StoreTextElementConfig(Clay_TextElementConfig config) {  return Clay_GetCurrentContext()->booleanWarnings.maxElementsExceeded ? &Clay_TextElementConfig_DEFAULT : Clay__TextElementConfigArray_Add(&Clay_GetCurrentContext()->textElementConfigs, config); }
Clay_ImageElementConfig* Clay__StoreImageElementConfig(Clay_ImageElementConfig config) {  return Clay_GetCurrentContext()->booleanWarnings.maxElementsExceeded ? &Clay_ImageElementConfig_DEFAULT : Clay__ImageElementConfigArray_Add(&Clay_GetCurrentContext()->imageElementConfigs, config); }
Clay_FloatingElementConfig* Clay__StoreFloatingElementConfig(Clay_FloatingElementConfig config) {  return Clay_GetCurrentContext()->booleanWarnings.maxElementsExceeded ? &Clay_FloatingElementConfig_DEFAULT : Clay__FloatingElementConfigArray_Add(&Clay_GetCurrentContext()->floatingElementConfigs, config); }
Clay_CustomElementConfig* Clay__StoreCustomElementConfig(Clay_CustomElementConfig config) {  return Clay_GetCurrentContext()->booleanWarnings.maxElementsExceeded ? &Clay_CustomElementConfig_DEFAULT : Clay__CustomElementConfigArray_Add(&Clay_GetCurrentContext()->customElementConfigs, config); }
Clay_ScrollElementConfig* Clay__StoreScrollElementConfig(Clay_ScrollElementConfig config) {  return Clay_GetCurrentContext()->booleanWarnings.maxElementsExceeded ? &Clay_ScrollElementConfig_DEFAULT : Clay__ScrollElementConfigArray_Add(&Clay_GetCurrentContext()->scrollElementConfigs, config); }
Clay_BorderElementConfig* Clay__StoreBorderElementConfig(Clay_BorderElementConfig config) {  return Clay_GetCurrentContext()->booleanWarnings.maxElementsExceeded ? &Clay_BorderElementConfig_DEFAULT : Clay__BorderElementConfigArray_Add(&Clay_GetCurrentContext()->borderElementConfigs, config); }
Clay_SharedElementConfig* Clay__StoreSharedElementConfig(Clay_SharedElementConfig config) {  return Clay_GetCurrentContext()->booleanWarnings.maxElementsExceeded ? &Clay_SharedElementConfig_DEFAULT : Clay__SharedElementConfigArray_Add(&Clay_GetCurrentContext()->sharedElementConfigs, config); }

Clay_ElementConfig Clay__AttachElementConfig(Clay_ElementConfigUnion config, Clay__ElementConfigType type)
{
	Clay_Context* context = Clay_GetCurrentContext();
	if (context->booleanWarnings.maxElementsExceeded) { return NEW_STRUCT(Clay_ElementConfig) ZEROED; }
	Clay_LayoutElement* openLayoutElement = Clay__GetOpenLayoutElement();
	openLayoutElement->elementConfigs.length++;
	return *Clay__ElementConfigArray_Add(&context->elementConfigs, NEW_STRUCT(Clay_ElementConfig) { .type = type, .config = config });
}

Clay_ElementConfigUnion Clay__FindElementConfigWithType(Clay_LayoutElement* element, Clay__ElementConfigType type)
{
	for (uxx cIndex = 0; cIndex < element->elementConfigs.length; cIndex++)
	{
		Clay_ElementConfig* config = Clay__ElementConfigArray_GetSlice(&element->elementConfigs, cIndex);
		if (config->type == type) { return config->config; }
	}
	return NEW_STRUCT(Clay_ElementConfigUnion) { NULL };
}

Clay_ElementId Clay__HashNumber(const u32 offset, const u32 seed)
{
	u32 hash = seed;
	hash += (offset + 48);
	hash += (hash << 10);
	hash ^= (hash >> 6);

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);
	return NEW_STRUCT(Clay_ElementId) { .id = hash + 1, .offset = offset, .baseId = seed, .stringId = Str8_Empty }; // Reserve the hash result of zero as "null id"
}

CLAY_DECOR Clay_ElementId Clay__HashString(Str8 key, const u32 offset, const u32 seed)
{
	u32 hash = 0;
	u32 base = seed;
	
	for (uxx cIndex = 0; cIndex < key.length; cIndex++) {
		base += key.chars[cIndex];
		base += (base << 10);
		base ^= (base >> 6);
	}
	hash = base;
	hash += offset;
	hash += (hash << 10);
	hash ^= (hash >> 6);
	
	hash += (hash << 3);
	base += (base << 3);
	hash ^= (hash >> 11);
	base ^= (base >> 11);
	hash += (hash << 15);
	base += (base << 15);
	return NEW_STRUCT(Clay_ElementId) { .id = hash + 1, .offset = offset, .baseId = base + 1, .stringId = key }; // Reserve the hash result of zero as "null id"
}

u32 Clay__HashTextWithConfig(Str8* text, Clay_TextElementConfig* config)
{
	u32 hash = 0;
	uintptr_t pointerAsNumber = (uintptr_t)text->chars;
	
	if (config->hashStringContents)
	{
		u32 maxLengthToHash = MinI32(text->length, 256);
		for (u32 i = 0; i < maxLengthToHash; i++)
		{
			hash += text->chars[i];
			hash += (hash << 10);
			hash ^= (hash >> 6);
		}
	}
	else
	{
		hash += pointerAsNumber;
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	
	hash += text->length;
	hash += (hash << 10);
	hash ^= (hash >> 6);
	
	hash += config->fontId;
	hash += (hash << 10);
	hash ^= (hash >> 6);
	
	hash += config->fontSize;
	hash += (hash << 10);
	hash ^= (hash >> 6);
	
	hash += config->lineHeight;
	hash += (hash << 10);
	hash ^= (hash >> 6);
	
	hash += config->letterSpacing;
	hash += (hash << 10);
	hash ^= (hash >> 6);
	
	hash += config->wrapMode;
	hash += (hash << 10);
	hash ^= (hash >> 6);
	
	//TODO: This should be changed so it's generic or maybe a callback. Directly accessing userData members inside Clay source means you can't change CLAY_TEXT_USERDATA_TYPE to anything besides ClayTextUserData
	if (Clay__HashTextUserData != nullptr)
	{
		hash = Clay__HashTextUserData(hash, config);
	}
	
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);
	return hash + 1; // Reserve the hash result of zero as "null id"
}

Clay__MeasuredWord* Clay__AddMeasuredWord(Clay__MeasuredWord word, Clay__MeasuredWord* previousWord)
{
	Clay_Context* context = Clay_GetCurrentContext();
	if (context->measuredWordsFreeList.length > 0)
	{
		i32 newItemIndex = i32Array_GetValue(&context->measuredWordsFreeList, context->measuredWordsFreeList.length - 1);
		context->measuredWordsFreeList.length--;
		Clay__MeasuredWordArray_Set(&context->measuredWords, (uxx)newItemIndex, word);
		previousWord->next = newItemIndex;
		return Clay__MeasuredWordArray_Get(&context->measuredWords, (uxx)newItemIndex);
	}
	else
	{
		previousWord->next = (i32)context->measuredWords.length;
		return Clay__MeasuredWordArray_Add(&context->measuredWords, word);
	}
}

Clay__MeasureTextCacheItem* Clay__MeasureTextCached(Str8* text, Clay_TextElementConfig* config)
{
	Clay_Context* context = Clay_GetCurrentContext();
	#ifndef CLAY_WASM
	if (!Clay__MeasureText)
	{
		if (!context->booleanWarnings.textMeasurementFunctionNotSet)
		{
			context->booleanWarnings.textMeasurementFunctionNotSet = true;
			context->errorHandler.errorHandlerFunction(NEW_STRUCT(Clay_ErrorData) {
				.errorType = CLAY_ERROR_TYPE_TEXT_MEASUREMENT_FUNCTION_NOT_PROVIDED,
				.errorText = CLAY_STRING("Clay's internal MeasureText function is null. You may have forgotten to call Clay_SetMeasureTextFunction(), or passed a NULL function pointer by mistake."),
				.userData = context->errorHandler.userData
			});
		}
		return &Clay__MeasureTextCacheItem_DEFAULT;
	}
	#endif
	u32 id = Clay__HashTextWithConfig(text, config);
	u32 hashBucket = id % (context->maxMeasureTextCacheWordCount / 32);
	i32 elementIndexPrevious = 0;
	i32 elementIndex = context->measureTextHashMap.items[hashBucket];
	while (elementIndex != 0)
	{
		Clay__MeasureTextCacheItem* hashEntry = Clay__MeasureTextCacheItemArray_Get(&context->measureTextHashMapInternal, (uxx)elementIndex);
		if (hashEntry->id == id)
		{
			hashEntry->generation = context->generation;
			return hashEntry;
		}
		// This element hasn't been seen in a few frames, delete the hash map item
		if (context->generation - hashEntry->generation > 2)
		{
			// Add all the measured words that were included in this measurement to the freelist
			i32 nextWordIndex = hashEntry->measuredWordsStartIndex;
			while (nextWordIndex != -1)
			{
				Clay__MeasuredWord* measuredWord = Clay__MeasuredWordArray_Get(&context->measuredWords, (uxx)nextWordIndex);
				i32Array_Add(&context->measuredWordsFreeList, (uxx)nextWordIndex);
				nextWordIndex = measuredWord->next;
			}
			
			i32 nextIndex = hashEntry->nextIndex;
			Clay__MeasureTextCacheItemArray_Set(&context->measureTextHashMapInternal, elementIndex, NEW_STRUCT(Clay__MeasureTextCacheItem) { .measuredWordsStartIndex = -1 });
			i32Array_Add(&context->measureTextHashMapInternalFreeList, elementIndex);
			if (elementIndexPrevious == 0)
			{
				context->measureTextHashMap.items[hashBucket] = nextIndex;
			}
			else
			{
				Clay__MeasureTextCacheItem* previousHashEntry = Clay__MeasureTextCacheItemArray_Get(&context->measureTextHashMapInternal, (uxx)elementIndexPrevious);
				previousHashEntry->nextIndex = nextIndex;
			}
			elementIndex = nextIndex;
		}
		else
		{
			elementIndexPrevious = elementIndex;
			elementIndex = hashEntry->nextIndex;
		}
	}
	
	i32 newItemIndex = 0;
	Clay__MeasureTextCacheItem newCacheItem = { .measuredWordsStartIndex = -1, .id = id, .generation = context->generation };
	Clay__MeasureTextCacheItem* measured = NULL;
	if (context->measureTextHashMapInternalFreeList.length > 0)
	{
		newItemIndex = i32Array_GetValue(&context->measureTextHashMapInternalFreeList, context->measureTextHashMapInternalFreeList.length - 1);
		context->measureTextHashMapInternalFreeList.length--;
		Clay__MeasureTextCacheItemArray_Set(&context->measureTextHashMapInternal, (uxx)newItemIndex, newCacheItem);
		measured = Clay__MeasureTextCacheItemArray_Get(&context->measureTextHashMapInternal, (uxx)newItemIndex);
	}
	else
	{
		if (context->measureTextHashMapInternal.length == context->measureTextHashMapInternal.allocLength - 1)
		{
			if (context->booleanWarnings.maxTextMeasureCacheExceeded)
			{
				context->errorHandler.errorHandlerFunction(NEW_STRUCT(Clay_ErrorData) {
					.errorType = CLAY_ERROR_TYPE_ELEMENTS_CAPACITY_EXCEEDED,
					.errorText = CLAY_STRING("Clay ran out of capacity while attempting to measure text elements. Try using Clay_SetMaxElementCount() with a higher value."),
					.userData = context->errorHandler.userData
				});
				context->booleanWarnings.maxTextMeasureCacheExceeded = true;
			}
			return &Clay__MeasureTextCacheItem_DEFAULT;
		}
		measured = Clay__MeasureTextCacheItemArray_Add(&context->measureTextHashMapInternal, newCacheItem);
		newItemIndex = context->measureTextHashMapInternal.length - 1;
	}
	
	uxx start = 0;
	uxx end = 0;
	r32 lineWidth = 0;
	r32 measuredWidth = 0;
	r32 measuredHeight = 0;
	r32 spaceWidth = Clay__MeasureText(Str8_Space, config, context->measureTextUserData).Width;
	Clay__MeasuredWord tempWord = { .next = -1 };
	Clay__MeasuredWord* previousWord = &tempWord;
	while (end < text->length)
	{
		if (context->measuredWords.length == context->measuredWords.allocLength - 1)
		{
			if (!context->booleanWarnings.maxTextMeasureCacheExceeded)
			{
				context->errorHandler.errorHandlerFunction(NEW_STRUCT(Clay_ErrorData) {
					.errorType = CLAY_ERROR_TYPE_TEXT_MEASUREMENT_CAPACITY_EXCEEDED,
					.errorText = CLAY_STRING("Clay has run out of space in it's internal text measurement cache. Try using Clay_SetMaxMeasureTextCacheWordCount() (default 16384, with 1 unit storing 1 measured word)."),
					.userData = context->errorHandler.userData
				});
				context->booleanWarnings.maxTextMeasureCacheExceeded = true;
			}
			return &Clay__MeasureTextCacheItem_DEFAULT;
		}
		char current = text->chars[end];
		if (current == ' ' || current == '\n')
		{
			uxx length = end - start;
			v2 dimensions = Clay__MeasureText(StrSliceLength(*text, start, length), config, context->measureTextUserData);
			measuredHeight = MaxR32(measuredHeight, dimensions.Height);
			if (current == ' ')
			{
				dimensions.Width += spaceWidth;
				previousWord = Clay__AddMeasuredWord(NEW_STRUCT(Clay__MeasuredWord) { .startOffset = start, .length = length + 1, .width = dimensions.Width, .next = -1 }, previousWord);
				lineWidth += dimensions.Width;
			}
			if (current == '\n')
			{
				if (length > 0)
				{
					previousWord = Clay__AddMeasuredWord(NEW_STRUCT(Clay__MeasuredWord) { .startOffset = start, .length = length, .width = dimensions.Width, .next = -1 }, previousWord);
				}
				previousWord = Clay__AddMeasuredWord(NEW_STRUCT(Clay__MeasuredWord) { .startOffset = end + 1, .length = 0, .width = 0, .next = -1 }, previousWord);
				lineWidth += dimensions.Width;
				measuredWidth = MaxR32(lineWidth, measuredWidth);
				measured->containsNewlines = true;
				lineWidth = 0;
			}
			start = end + 1;
		}
		end++;
	}
	if (end - start > 0)
	{
		v2 dimensions = Clay__MeasureText(StrSlice(*text, start, end), config, context->measureTextUserData);
		Clay__AddMeasuredWord(NEW_STRUCT(Clay__MeasuredWord) { .startOffset = start, .length = end - start, .width = dimensions.Width, .next = -1 }, previousWord);
		lineWidth += dimensions.Width;
		measuredHeight = MaxR32(measuredHeight, dimensions.Height);
	}
	measuredWidth = MaxR32(lineWidth, measuredWidth);
	
	measured->measuredWordsStartIndex = tempWord.next;
	measured->unwrappedDimensions.Width = measuredWidth;
	measured->unwrappedDimensions.Height = measuredHeight;
	
	if (elementIndexPrevious != 0)
	{
		Clay__MeasureTextCacheItemArray_Get(&context->measureTextHashMapInternal, (uxx)elementIndexPrevious)->nextIndex = newItemIndex;
	}
	else
	{
		context->measureTextHashMap.items[hashBucket] = newItemIndex;
	}
	return measured;
}

bool Clay__PointIsInsideRect(v2 point, rec rect)
{
	return point.X >= rect.X && point.X <= rect.X + rect.Width && point.Y >= rect.Y && point.Y <= rect.Y + rect.Height;
}

Clay_LayoutElementHashMapItem* Clay__AddHashMapItem(Clay_ElementId elementId, Clay_LayoutElement* layoutElement, u32 idAlias)
{
	Clay_Context* context = Clay_GetCurrentContext();
	if (context->layoutElementsHashMapInternal.length == context->layoutElementsHashMapInternal.allocLength - 1) { return NULL; }
	Clay_LayoutElementHashMapItem item = { .elementId = elementId, .layoutElement = layoutElement, .nextIndex = -1, .generation = context->generation + 1, .idAlias = idAlias };
	u32 hashBucket = elementId.id % context->layoutElementsHashMap.allocLength;
	i32 hashItemPrevious = -1;
	i32 hashItemIndex = context->layoutElementsHashMap.items[hashBucket];
	while (hashItemIndex != -1) // Just replace collision, not a big deal - leave it up to the end user
	{
		Clay_LayoutElementHashMapItem* hashItem = Clay__LayoutElementHashMapItemArray_Get(&context->layoutElementsHashMapInternal, (uxx)hashItemIndex);
		if (hashItem->elementId.id == elementId.id) // Collision - resolve based on generation
		{
			item.nextIndex = hashItem->nextIndex;
			if (hashItem->generation <= context->generation) // First collision - assume this is the "same" element
			{
				hashItem->elementId = elementId; // Make sure to copy this across. If the stringId reference has changed, we should update the hash item to use the new one.
				hashItem->generation = context->generation + 1;
				hashItem->layoutElement = layoutElement;
				hashItem->debugData->collision = false;
			}
			else // Multiple collisions this frame - two elements have the same ID
			{
				context->errorHandler.errorHandlerFunction(NEW_STRUCT(Clay_ErrorData) {
					.errorType = CLAY_ERROR_TYPE_DUPLICATE_ID,
					.errorText = CLAY_STRING("An element with this ID was already previously declared during this layout."),
					.userData = context->errorHandler.userData
				});
				if (context->debugModeEnabled) { hashItem->debugData->collision = true; }
			}
			return hashItem;
		}
		hashItemPrevious = hashItemIndex;
		hashItemIndex = hashItem->nextIndex;
	}
	Clay_LayoutElementHashMapItem* hashItem = Clay__LayoutElementHashMapItemArray_Add(&context->layoutElementsHashMapInternal, item);
	hashItem->debugData = Clay__DebugElementDataArray_Add(&context->debugElementData, NEW_STRUCT(Clay__DebugElementData) ZEROED);
	if (hashItemPrevious != -1)
	{
		Clay__LayoutElementHashMapItemArray_Get(&context->layoutElementsHashMapInternal, (uxx)hashItemPrevious)->nextIndex = (i32)context->layoutElementsHashMapInternal.length - 1;
	}
	else
	{
		context->layoutElementsHashMap.items[hashBucket] = (i32)context->layoutElementsHashMapInternal.length - 1;
	}
	return hashItem;
}

Clay_LayoutElementHashMapItem* Clay__GetHashMapItem(u32 id)
{
	Clay_Context* context = Clay_GetCurrentContext();
	u32 hashBucket = id % context->layoutElementsHashMap.allocLength;
	i32 elementIndex = context->layoutElementsHashMap.items[hashBucket];
	while (elementIndex != -1)
	{
		Clay_LayoutElementHashMapItem* hashEntry = Clay__LayoutElementHashMapItemArray_Get(&context->layoutElementsHashMapInternal, (uxx)elementIndex);
		if (hashEntry->elementId.id == id) { return hashEntry; }
		elementIndex = hashEntry->nextIndex;
	}
	return &Clay_LayoutElementHashMapItem_DEFAULT;
}

Clay_ElementId Clay__GenerateIdForAnonymousElement(Clay_LayoutElement* openLayoutElement)
{
	Clay_Context* context = Clay_GetCurrentContext();
	Clay_LayoutElement* parentElement = Clay_LayoutElementArray_Get(&context->layoutElements, i32Array_GetValue(&context->openLayoutElementStack, context->openLayoutElementStack.length - 2));
	Clay_ElementId elementId = Clay__HashNumber(parentElement->childrenOrTextContent.children.length, parentElement->id);
	openLayoutElement->id = elementId.id;
	Clay__AddHashMapItem(elementId, openLayoutElement, 0);
	Str8Array_Add(&context->layoutElementIdStrings, elementId.stringId);
	return elementId;
}

bool Clay__ElementHasConfig(Clay_LayoutElement* layoutElement, Clay__ElementConfigType type)
{
	for (uxx cIndex = 0; cIndex < layoutElement->elementConfigs.length; cIndex++)
	{
		if (Clay__ElementConfigArray_GetSlice(&layoutElement->elementConfigs, cIndex)->type == type) { return true; }
	}
	return false;
}

void Clay__UpdateAspectRatioBox(Clay_LayoutElement* layoutElement)
{
	for (uxx cIndex = 0; cIndex < layoutElement->elementConfigs.length; cIndex++)
	{
		Clay_ElementConfig* config = Clay__ElementConfigArray_GetSlice(&layoutElement->elementConfigs, cIndex);
		if (config->type == CLAY__ELEMENT_CONFIG_TYPE_IMAGE)
		{
			Clay_ImageElementConfig* imageConfig = config->config.imageElementConfig;
			if (imageConfig->sourceDimensions.Width == 0 || imageConfig->sourceDimensions.Height == 0) { break; }
			r32 aspect = imageConfig->sourceDimensions.Width / imageConfig->sourceDimensions.Height;
			if (layoutElement->dimensions.Width == 0 && layoutElement->dimensions.Height != 0)
			{
				layoutElement->dimensions.Width = layoutElement->dimensions.Height * aspect;
			}
			else if (layoutElement->dimensions.Width != 0 && layoutElement->dimensions.Height == 0)
			{
				layoutElement->dimensions.Height = layoutElement->dimensions.Height * (1 / aspect);
			}
			break;
		}
	}
}

CLAY_DECOR void Clay__CloseElement(void) {
	Clay_Context* context = Clay_GetCurrentContext();
	if (context->booleanWarnings.maxElementsExceeded) { return; }
	Clay_LayoutElement* openLayoutElement = Clay__GetOpenLayoutElement();
	Clay_LayoutConfig* layoutConfig = openLayoutElement->layoutConfig;
	bool elementHasScrollHorizontal = false;
	bool elementHasScrollVertical = false;
	for (uxx cIndex = 0; cIndex < openLayoutElement->elementConfigs.length; cIndex++)
	{
		Clay_ElementConfig* config = Clay__ElementConfigArray_GetSlice(&openLayoutElement->elementConfigs, cIndex);
		if (config->type == CLAY__ELEMENT_CONFIG_TYPE_SCROLL)
		{
			elementHasScrollHorizontal = config->config.scrollElementConfig->horizontal;
			elementHasScrollVertical = config->config.scrollElementConfig->vertical;
			context->openClipElementStack.length--;
			break;
		}
	}
	
	// Attach children to the current open element
	openLayoutElement->childrenOrTextContent.children.elements = &context->layoutElementChildren.items[context->layoutElementChildren.length];
	if (layoutConfig->layoutDirection == CLAY_LEFT_TO_RIGHT)
	{
		openLayoutElement->dimensions.Width = (r32)(layoutConfig->padding.left + layoutConfig->padding.right);
		for (uxx i = 0; i < openLayoutElement->childrenOrTextContent.children.length; i++)
		{
			i32 childIndex = i32Array_GetValue(&context->layoutElementChildrenBuffer, context->layoutElementChildrenBuffer.length - openLayoutElement->childrenOrTextContent.children.length + i);
			Clay_LayoutElement* child = Clay_LayoutElementArray_Get(&context->layoutElements, (uxx)childIndex);
			openLayoutElement->dimensions.Width += child->dimensions.Width;
			openLayoutElement->dimensions.Height = MaxR32(openLayoutElement->dimensions.Height, child->dimensions.Height + layoutConfig->padding.top + layoutConfig->padding.bottom);
			// Minimum size of child elements doesn't matter to scroll containers as they can shrink and hide their contents
			if (!elementHasScrollHorizontal)
			{
				openLayoutElement->minDimensions.Width += child->minDimensions.Width;
			}
			if (!elementHasScrollVertical)
			{
				openLayoutElement->minDimensions.Height = MaxR32(openLayoutElement->minDimensions.Height, child->minDimensions.Height + layoutConfig->padding.top + layoutConfig->padding.bottom);
			}
			i32Array_Add(&context->layoutElementChildren, childIndex);
		}
		r32 childGap = (r32)(MaxU16(openLayoutElement->childrenOrTextContent.children.length - 1, 0) * layoutConfig->childGap);
		openLayoutElement->dimensions.Width += childGap; // TODO this is technically a bug with childgap and scroll containers
		openLayoutElement->minDimensions.Width += childGap;
	}
	else if (layoutConfig->layoutDirection == CLAY_TOP_TO_BOTTOM)
	{
		openLayoutElement->dimensions.Height = (r32)(layoutConfig->padding.top + layoutConfig->padding.bottom);
		for (uxx i = 0; i < openLayoutElement->childrenOrTextContent.children.length; i++)
		{
			i32 childIndex = i32Array_GetValue(&context->layoutElementChildrenBuffer, context->layoutElementChildrenBuffer.length - openLayoutElement->childrenOrTextContent.children.length + i);
			Clay_LayoutElement* child = Clay_LayoutElementArray_Get(&context->layoutElements, (uxx)childIndex);
			openLayoutElement->dimensions.Height += child->dimensions.Height;
			openLayoutElement->dimensions.Width = MaxR32(openLayoutElement->dimensions.Width, child->dimensions.Width + layoutConfig->padding.left + layoutConfig->padding.right);
			// Minimum size of child elements doesn't matter to scroll containers as they can shrink and hide their contents
			if (!elementHasScrollVertical)
			{
				openLayoutElement->minDimensions.Height += child->minDimensions.Height;
			}
			if (!elementHasScrollHorizontal)
			{
				openLayoutElement->minDimensions.Width = MaxR32(openLayoutElement->minDimensions.Width, child->minDimensions.Width + layoutConfig->padding.left + layoutConfig->padding.right);
			}
			i32Array_Add(&context->layoutElementChildren, childIndex);
		}
		r32 childGap = (r32)(MaxU16(openLayoutElement->childrenOrTextContent.children.length - 1, 0) * layoutConfig->childGap);
		openLayoutElement->dimensions.Height += childGap; // TODO this is technically a bug with childgap and scroll containers
		openLayoutElement->minDimensions.Height += childGap;
	}
	
	context->layoutElementChildrenBuffer.length -= openLayoutElement->childrenOrTextContent.children.length;
	
	// Clamp element min and max width to the values configured in the layout
	if (layoutConfig->sizing.width.type != CLAY__SIZING_TYPE_PERCENT)
	{
		if (layoutConfig->sizing.width.size.minMax.max <= 0) // Set the max size if the user didn't specify, makes calculations easier
		{
			layoutConfig->sizing.width.size.minMax.max = HugeR32;
		}
		openLayoutElement->dimensions.Width = MinR32(MaxR32(openLayoutElement->dimensions.Width, layoutConfig->sizing.width.size.minMax.min), layoutConfig->sizing.width.size.minMax.max);
		openLayoutElement->minDimensions.Width = MinR32(MaxR32(openLayoutElement->minDimensions.Width, layoutConfig->sizing.width.size.minMax.min), layoutConfig->sizing.width.size.minMax.max);
	}
	else
	{
		openLayoutElement->dimensions.Width = 0;
	}
	
	// Clamp element min and max height to the values configured in the layout
	if (layoutConfig->sizing.height.type != CLAY__SIZING_TYPE_PERCENT)
	{
		if (layoutConfig->sizing.height.size.minMax.max <= 0) // Set the max size if the user didn't specify, makes calculations easier
		{
			layoutConfig->sizing.height.size.minMax.max = HugeR32;
		}
		openLayoutElement->dimensions.Height = MinR32(MaxR32(openLayoutElement->dimensions.Height, layoutConfig->sizing.height.size.minMax.min), layoutConfig->sizing.height.size.minMax.max);
		openLayoutElement->minDimensions.Height = MinR32(MaxR32(openLayoutElement->minDimensions.Height, layoutConfig->sizing.height.size.minMax.min), layoutConfig->sizing.height.size.minMax.max);
	}
	else
	{
		openLayoutElement->dimensions.Height = 0;
	}
	
	Clay__UpdateAspectRatioBox(openLayoutElement);
	
	bool elementIsFloating = Clay__ElementHasConfig(openLayoutElement, CLAY__ELEMENT_CONFIG_TYPE_FLOATING);
	
	// Close the currently open element
	i32 closingElementIndex = i32Array_RemoveSwapback(&context->openLayoutElementStack, (int)context->openLayoutElementStack.length - 1);
	openLayoutElement = Clay__GetOpenLayoutElement();
	
	if (!elementIsFloating && context->openLayoutElementStack.length > 1)
	{
		openLayoutElement->childrenOrTextContent.children.length++;
		i32Array_Add(&context->layoutElementChildrenBuffer, closingElementIndex);
	}
}

bool Clay__MemCmp(const char* s1, const char* s2, i32 length);
#if !defined(CLAY_DISABLE_SIMD) && (defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64))
	bool Clay__MemCmp(const char* s1, const char* s2, i32 length)
	{
		while (length >= 16)
		{
			__m128i v1 = _mm_loadu_si128((const __m128i*)s1);
			__m128i v2 = _mm_loadu_si128((const __m128i*)s2);
			
			if (_mm_movemask_epi8(_mm_cmpeq_epi8(v1, v2)) != 0xFFFF) { return false; } // If any byte differs
			
			s1 += 16;
			s2 += 16;
			length -= 16;
		}
		
		// Handle remaining bytes
		while (length--)
		{
			if (*s1 != *s2) { return false; }
			s1++;
			s2++;
		}
		
		return true;
	}
#elif !defined(CLAY_DISABLE_SIMD) && defined(__aarch64__)
	bool Clay__MemCmp(const char* s1, const char* s2, i32 length)
	{
		while (length >= 16)
		{
			uint8x16_t v1 = vld1q_u8((const u8*)s1);
			uint8x16_t v2 = vld1q_u8((const u8*)s2);
			
			// Compare vectors
			if (vminvq_u32(vceqq_u8(v1, v2)) != 0xFFFFFFFF) { return false; } // If there's a difference
			
			s1 += 16;
			s2 += 16;
			length -= 16;
		}
		
		// Handle remaining bytes
		while (length--)
		{
			if (*s1 != *s2) { return false; }
			s1++;
			s2++;
		}
		
		return true;
	}
#else
	bool Clay__MemCmp(const char* s1, const char* s2, i32 length)
	{
		for (i32 i = 0; i < length; i++)
		{
			if (s1[i] != s2[i]) { return false; }
		}
		return true;
	}
#endif

CLAY_DECOR void Clay__OpenElement(void)
{
	Clay_Context* context = Clay_GetCurrentContext();
	if (context->layoutElements.length == context->layoutElements.allocLength - 1 || context->booleanWarnings.maxElementsExceeded)
	{
		context->booleanWarnings.maxElementsExceeded = true;
		return;
	}
	Clay_LayoutElement layoutElement = ZEROED;
	Clay_LayoutElementArray_Add(&context->layoutElements, layoutElement);
	i32Array_Add(&context->openLayoutElementStack, context->layoutElements.length - 1);
	if (context->openClipElementStack.length > 0)
	{
		i32Array_Set(&context->layoutElementClipElementIds, context->layoutElements.length - 1, i32Array_GetValue(&context->openClipElementStack, (int)context->openClipElementStack.length - 1));
	}
	else
	{
		i32Array_Set(&context->layoutElementClipElementIds, context->layoutElements.length - 1, 0);
	}
}

CLAY_DECOR void Clay__OpenTextElement(Str8 text, Clay_TextElementConfig* textConfig)
{
	Clay_Context* context = Clay_GetCurrentContext();
	if (context->layoutElements.length == context->layoutElements.allocLength - 1 || context->booleanWarnings.maxElementsExceeded)
	{
		context->booleanWarnings.maxElementsExceeded = true;
		return;
	}
	Clay_LayoutElement* parentElement = Clay__GetOpenLayoutElement();
	
	Clay_LayoutElement layoutElement = ZEROED;
	Clay_LayoutElement* textElement = Clay_LayoutElementArray_Add(&context->layoutElements, layoutElement);
	if (context->openClipElementStack.length > 0)
	{
		i32Array_Set(&context->layoutElementClipElementIds, context->layoutElements.length - 1, i32Array_GetValue(&context->openClipElementStack, (int)context->openClipElementStack.length - 1));
	}
	else
	{
		i32Array_Set(&context->layoutElementClipElementIds, context->layoutElements.length - 1, 0);
	}
	
	i32Array_Add(&context->layoutElementChildrenBuffer, context->layoutElements.length - 1);
	Clay__MeasureTextCacheItem* textMeasured = Clay__MeasureTextCached(&text, textConfig);
	Clay_ElementId elementId = Clay__HashNumber(parentElement->childrenOrTextContent.children.length, parentElement->id);
	textElement->id = elementId.id;
	Clay__AddHashMapItem(elementId, textElement, 0);
	Str8Array_Add(&context->layoutElementIdStrings, elementId.stringId);
	v2 textDimensions = MakeV2(
		textMeasured->unwrappedDimensions.Width,
		textConfig->lineHeight > 0 ? (r32)textConfig->lineHeight : textMeasured->unwrappedDimensions.Height
	);
	textElement->dimensions = textDimensions;
	textElement->minDimensions = MakeV2(textMeasured->unwrappedDimensions.Height, textDimensions.Height); // TODO not sure this is the best way to decide min width for text
	textElement->childrenOrTextContent.textElementData = Clay__TextElementDataArray_Add(&context->textElementData, NEW_STRUCT(Clay__TextElementData) { .text = text, .preferredDimensions = textMeasured->unwrappedDimensions, .elementIndex = context->layoutElements.length - 1 });
	textElement->elementConfigs = NEW_STRUCT(Clay__ElementConfigArraySlice) {
		.length = 1,
		.items = Clay__ElementConfigArray_Add(&context->elementConfigs, NEW_STRUCT(Clay_ElementConfig) { .type = CLAY__ELEMENT_CONFIG_TYPE_TEXT, .config = { .textElementConfig = textConfig }})
	};
	textElement->layoutConfig = &CLAY_LAYOUT_DEFAULT;
	parentElement->childrenOrTextContent.children.length++;
}

Clay_ElementId Clay__AttachId(Clay_ElementId elementId)
{
	Clay_Context* context = Clay_GetCurrentContext();
	if (context->booleanWarnings.maxElementsExceeded)
	{
		return Clay_ElementId_DEFAULT;
	}
	Clay_LayoutElement* openLayoutElement = Clay__GetOpenLayoutElement();
	u32 idAlias = openLayoutElement->id;
	openLayoutElement->id = elementId.id;
	Clay__AddHashMapItem(elementId, openLayoutElement, idAlias);
	Str8Array_Add(&context->layoutElementIdStrings, elementId.stringId);
	return elementId;
}

CLAY_DECOR void Clay__ConfigureOpenElement(const Clay_ElementDeclaration declaration)
{
	Clay_Context* context = Clay_GetCurrentContext();
	Clay_LayoutElement* openLayoutElement = Clay__GetOpenLayoutElement();
	openLayoutElement->layoutConfig = Clay__StoreLayoutConfig(declaration.layout);
	if ((declaration.layout.sizing.width.type == CLAY__SIZING_TYPE_PERCENT && declaration.layout.sizing.width.size.percent > 1) ||
		(declaration.layout.sizing.height.type == CLAY__SIZING_TYPE_PERCENT && declaration.layout.sizing.height.size.percent > 1))
	{
		context->errorHandler.errorHandlerFunction(NEW_STRUCT(Clay_ErrorData) {
			.errorType = CLAY_ERROR_TYPE_PERCENTAGE_OVER_1,
			.errorText = CLAY_STRING("An element was configured with CLAY_SIZING_PERCENT, but the provided percentage value was over 1.0. Clay expects a value between 0 and 1, i.e. 20% is 0.2."),
			.userData = context->errorHandler.userData
		});
	}
	
	Clay_ElementId openLayoutElementId = declaration.id;
	
	openLayoutElement->elementConfigs.items = &context->elementConfigs.items[context->elementConfigs.length];
	Clay_SharedElementConfig* sharedConfig = NULL;
	if (declaration.backgroundColor.a > 0)
	{
		sharedConfig = Clay__StoreSharedElementConfig(NEW_STRUCT(Clay_SharedElementConfig) { .backgroundColor = declaration.backgroundColor });
		Clay__AttachElementConfig(NEW_STRUCT(Clay_ElementConfigUnion) { .sharedElementConfig = sharedConfig }, CLAY__ELEMENT_CONFIG_TYPE_SHARED);
	}
	if (!Clay__MemCmp((char*)(&declaration.cornerRadius), (char*)(&Clay__CornerRadius_DEFAULT), sizeof(Clay_CornerRadius)))
	{
		if (sharedConfig)
		{
			sharedConfig->cornerRadius = declaration.cornerRadius;
		}
		else
		{
			sharedConfig = Clay__StoreSharedElementConfig(NEW_STRUCT(Clay_SharedElementConfig) { .cornerRadius = declaration.cornerRadius });
			Clay__AttachElementConfig(NEW_STRUCT(Clay_ElementConfigUnion) { .sharedElementConfig = sharedConfig }, CLAY__ELEMENT_CONFIG_TYPE_SHARED);
		}
	}
	CLAY_ELEMENT_USERDATA_TYPE zeroUserDataType = ZEROED;
	if (!Clay__MemCmp((char*)(&declaration.userData), (char*)(&zeroUserDataType), sizeof(CLAY_ELEMENT_USERDATA_TYPE)))
	{
		if (sharedConfig)
		{
			sharedConfig->userData = declaration.userData;
		}
		else
		{
			sharedConfig = Clay__StoreSharedElementConfig(NEW_STRUCT(Clay_SharedElementConfig) { .userData = declaration.userData });
			Clay__AttachElementConfig(NEW_STRUCT(Clay_ElementConfigUnion) { .sharedElementConfig = sharedConfig }, CLAY__ELEMENT_CONFIG_TYPE_SHARED);
		}
	}
	CLAY_IMAGEDATA_TYPE zeroImageDataType = ZEROED;
	if (!Clay__MemCmp((char*)(&declaration.image.imageData), (char*)(&zeroImageDataType), sizeof(CLAY_IMAGEDATA_TYPE)))
	{
		Clay__AttachElementConfig(NEW_STRUCT(Clay_ElementConfigUnion) { .imageElementConfig = Clay__StoreImageElementConfig(declaration.image) }, CLAY__ELEMENT_CONFIG_TYPE_IMAGE);
		i32Array_Add(&context->imageElementPointers, context->layoutElements.length - 1);
	}
	if (declaration.floating.attachTo != CLAY_ATTACH_TO_NONE)
	{
		Clay_FloatingElementConfig floatingConfig = declaration.floating;
		// This looks dodgy but because of the auto generated root element the depth of the tree will always be at least 2 here
		Clay_LayoutElement* hierarchicalParent = Clay_LayoutElementArray_Get(&context->layoutElements, i32Array_GetValue(&context->openLayoutElementStack, context->openLayoutElementStack.length - 2));
		if (hierarchicalParent)
		{
			u32 clipElementId = 0;
			if (declaration.floating.attachTo == CLAY_ATTACH_TO_PARENT)
			{
				// Attach to the element's direct hierarchical parent
				floatingConfig.parentId = hierarchicalParent->id;
				if (context->openClipElementStack.length > 0)
				{
					clipElementId = i32Array_GetValue(&context->openClipElementStack, (int)context->openClipElementStack.length - 1);
				}
			}
			else if (declaration.floating.attachTo == CLAY_ATTACH_TO_ELEMENT_WITH_ID)
			{
				Clay_LayoutElementHashMapItem* parentItem = Clay__GetHashMapItem(floatingConfig.parentId);
				if (!parentItem)
				{
					context->errorHandler.errorHandlerFunction(NEW_STRUCT(Clay_ErrorData) {
						.errorType = CLAY_ERROR_TYPE_FLOATING_CONTAINER_PARENT_NOT_FOUND,
						.errorText = CLAY_STRING("A floating element was declared with a parentId, but no element with that ID was found."),
						.userData = context->errorHandler.userData
					});
				}
				else
				{
					clipElementId = i32Array_GetValue(&context->layoutElementClipElementIds, parentItem->layoutElement - context->layoutElements.items);
				}
			}
			else if (declaration.floating.attachTo == CLAY_ATTACH_TO_ROOT)
			{
				floatingConfig.parentId = Clay__HashString(CLAY_STRING("Clay__RootContainer"), 0, 0).id;
			}
			if (!openLayoutElementId.id)
			{
				openLayoutElementId = Clay__HashString(CLAY_STRING("Clay__FloatingContainer"), context->layoutElementTreeRoots.length, 0);
			}
			Clay__LayoutElementTreeRootArray_Add(&context->layoutElementTreeRoots, NEW_STRUCT(Clay__LayoutElementTreeRoot) {
				.layoutElementIndex = i32Array_GetValue(&context->openLayoutElementStack, context->openLayoutElementStack.length - 1),
				.parentId = floatingConfig.parentId,
				.clipElementId = clipElementId,
				.zIndex = floatingConfig.zIndex,
			});
			Clay__AttachElementConfig(NEW_STRUCT(Clay_ElementConfigUnion) { .floatingElementConfig = Clay__StoreFloatingElementConfig(declaration.floating) }, CLAY__ELEMENT_CONFIG_TYPE_FLOATING);
		}
	}
	if (declaration.custom.customData)
	{
		Clay__AttachElementConfig(NEW_STRUCT(Clay_ElementConfigUnion) { .customElementConfig = Clay__StoreCustomElementConfig(declaration.custom) }, CLAY__ELEMENT_CONFIG_TYPE_CUSTOM);
	}
	
	if (openLayoutElementId.id != 0)
	{
		Clay__AttachId(openLayoutElementId);
	}
	else if (openLayoutElement->id == 0)
	{
		openLayoutElementId = Clay__GenerateIdForAnonymousElement(openLayoutElement);
	}
	
	if (declaration.scroll.horizontal | declaration.scroll.vertical)
	{
		Clay__AttachElementConfig(NEW_STRUCT(Clay_ElementConfigUnion) { .scrollElementConfig = Clay__StoreScrollElementConfig(declaration.scroll) }, CLAY__ELEMENT_CONFIG_TYPE_SCROLL);
		i32Array_Add(&context->openClipElementStack, (int)openLayoutElement->id);
		// Retrieve or create cached data to track scroll position across frames
		Clay__ScrollContainerDataInternal* scrollOffset = nullptr;
		for (uxx cIndex = 0; cIndex < context->scrollContainerDatas.length; cIndex++)
		{
			Clay__ScrollContainerDataInternal* mapping = Clay__ScrollContainerDataInternalArray_Get(&context->scrollContainerDatas, cIndex);
			if (openLayoutElement->id == mapping->elementId)
			{
				scrollOffset = mapping;
				scrollOffset->layoutElement = openLayoutElement;
				scrollOffset->openThisFrame = true;
				scrollOffset->scrollLag = declaration.scroll.scrollLag;
			}
		}
		if (!scrollOffset)
		{
			scrollOffset = Clay__ScrollContainerDataInternalArray_Add(&context->scrollContainerDatas, NEW_STRUCT(Clay__ScrollContainerDataInternal){.layoutElement = openLayoutElement, .scrollOrigin = FillV2(-1), .elementId = openLayoutElement->id, .scrollLag = declaration.scroll.scrollLag, .openThisFrame = true});
		}
		if (context->externalScrollHandlingEnabled)
		{
			scrollOffset->scrollTarget = Clay__QueryScrollOffset(scrollOffset->elementId, context->queryScrollOffsetUserData);
			scrollOffset->scrollPosition = scrollOffset->scrollTarget;
		}
	}
	if (!Clay__MemCmp((char*)(&declaration.border.width), (char*)(&Clay__BorderWidth_DEFAULT), sizeof(Clay_BorderWidth)))
	{
		Clay__AttachElementConfig(NEW_STRUCT(Clay_ElementConfigUnion) { .borderElementConfig = Clay__StoreBorderElementConfig(declaration.border) }, CLAY__ELEMENT_CONFIG_TYPE_BORDER);
	}
	
	if (declaration.tooltip.text.length > 0 && Clay__RegisterTooltip != nullptr)
	{
		Clay__RegisterTooltip(&declaration, context->registerTooltipUserData);
	}
}

//TODO: We really should push all these arenas over to use a Scratch arena
void Clay__InitializeEphemeralMemory(Clay_Context* context)
{
	i32 maxElementCount = context->maxElementCount;
	// Ephemeral Memory - reset every frame
	
	context->layoutElementChildrenBuffer = i32Array_Init(maxElementCount, context->internalArena);
	context->layoutElements = Clay_LayoutElementArray_Init(maxElementCount, context->internalArena);
	context->warnings = Clay__WarningArray_Init(100, context->internalArena);
	
	context->layoutConfigs = Clay__LayoutConfigArray_Init(maxElementCount, context->internalArena);
	context->elementConfigs = Clay__ElementConfigArray_Init(maxElementCount, context->internalArena);
	context->textElementConfigs = Clay__TextElementConfigArray_Init(maxElementCount, context->internalArena);
	context->imageElementConfigs = Clay__ImageElementConfigArray_Init(maxElementCount, context->internalArena);
	context->floatingElementConfigs = Clay__FloatingElementConfigArray_Init(maxElementCount, context->internalArena);
	context->scrollElementConfigs = Clay__ScrollElementConfigArray_Init(maxElementCount, context->internalArena);
	context->customElementConfigs = Clay__CustomElementConfigArray_Init(maxElementCount, context->internalArena);
	context->borderElementConfigs = Clay__BorderElementConfigArray_Init(maxElementCount, context->internalArena);
	context->sharedElementConfigs = Clay__SharedElementConfigArray_Init(maxElementCount, context->internalArena);
	
	context->layoutElementIdStrings = Str8Array_Init(maxElementCount, context->internalArena);
	context->wrappedTextLines = Clay__WrappedTextLineArray_Init(maxElementCount, context->internalArena);
	context->layoutElementTreeNodeArray1 = Clay__LayoutElementTreeNodeArray_Init(maxElementCount, context->internalArena);
	context->layoutElementTreeRoots = Clay__LayoutElementTreeRootArray_Init(maxElementCount, context->internalArena);
	context->layoutElementChildren = i32Array_Init(maxElementCount, context->internalArena);
	context->openLayoutElementStack = i32Array_Init(maxElementCount, context->internalArena);
	context->textElementData = Clay__TextElementDataArray_Init(maxElementCount, context->internalArena);
	context->imageElementPointers = i32Array_Init(maxElementCount, context->internalArena);
	context->renderCommands = Clay_RenderCommandArray_Init(maxElementCount, context->internalArena);
	context->treeNodeVisited = boolArray_Init(maxElementCount, context->internalArena);
	context->treeNodeVisited.length = context->treeNodeVisited.allocLength; // This array is accessed directly rather than behaving as a list
	context->openClipElementStack = i32Array_Init(maxElementCount, context->internalArena);
	context->reusableElementIndexBuffer = i32Array_Init(maxElementCount, context->internalArena);
	context->layoutElementClipElementIds = i32Array_Init(maxElementCount, context->internalArena);
	context->dynamicStringData = charArray_Init(maxElementCount, context->internalArena);
}

void Clay__FreeEphemeralMemory(Clay_Context* context)
{
	i32Array_Free(&context->layoutElementChildrenBuffer, context->internalArena);
	Clay_LayoutElementArray_Free(&context->layoutElements, context->internalArena);
	Clay__WarningArray_Free(&context->warnings, context->internalArena);
	
	Clay__LayoutConfigArray_Free(&context->layoutConfigs, context->internalArena);
	Clay__ElementConfigArray_Free(&context->elementConfigs, context->internalArena);
	Clay__TextElementConfigArray_Free(&context->textElementConfigs, context->internalArena);
	Clay__ImageElementConfigArray_Free(&context->imageElementConfigs, context->internalArena);
	Clay__FloatingElementConfigArray_Free(&context->floatingElementConfigs, context->internalArena);
	Clay__ScrollElementConfigArray_Free(&context->scrollElementConfigs, context->internalArena);
	Clay__CustomElementConfigArray_Free(&context->customElementConfigs, context->internalArena);
	Clay__BorderElementConfigArray_Free(&context->borderElementConfigs, context->internalArena);
	Clay__SharedElementConfigArray_Free(&context->sharedElementConfigs, context->internalArena);
	
	Str8Array_Free(&context->layoutElementIdStrings, context->internalArena);
	Clay__WrappedTextLineArray_Free(&context->wrappedTextLines, context->internalArena);
	Clay__LayoutElementTreeNodeArray_Free(&context->layoutElementTreeNodeArray1, context->internalArena);
	Clay__LayoutElementTreeRootArray_Free(&context->layoutElementTreeRoots, context->internalArena);
	i32Array_Free(&context->layoutElementChildren, context->internalArena);
	i32Array_Free(&context->openLayoutElementStack, context->internalArena);
	Clay__TextElementDataArray_Free(&context->textElementData, context->internalArena);
	i32Array_Free(&context->imageElementPointers, context->internalArena);
	Clay_RenderCommandArray_Free(&context->renderCommands, context->internalArena);
	boolArray_Free(&context->treeNodeVisited, context->internalArena);
	i32Array_Free(&context->openClipElementStack, context->internalArena);
	i32Array_Free(&context->reusableElementIndexBuffer, context->internalArena);
	i32Array_Free(&context->layoutElementClipElementIds, context->internalArena);
	charArray_Free(&context->dynamicStringData, context->internalArena);
}

void Clay__InitializePersistentMemory(Clay_Context* context)
{
	// Persistent memory - initialized once and not reset
	i32 maxElementCount = context->maxElementCount;
	i32 maxMeasureTextCacheWordCount = context->maxMeasureTextCacheWordCount;
	
	context->scrollContainerDatas = Clay__ScrollContainerDataInternalArray_Init(10, context->internalArena);
	context->layoutElementsHashMapInternal = Clay__LayoutElementHashMapItemArray_Init(maxElementCount, context->internalArena);
	context->layoutElementsHashMap = i32Array_Init(maxElementCount, context->internalArena);
	context->measureTextHashMapInternal = Clay__MeasureTextCacheItemArray_Init(maxElementCount, context->internalArena);
	context->measureTextHashMapInternalFreeList = i32Array_Init(maxElementCount, context->internalArena);
	context->measuredWordsFreeList = i32Array_Init(maxMeasureTextCacheWordCount, context->internalArena);
	context->measureTextHashMap = i32Array_Init(maxElementCount, context->internalArena);
	context->measuredWords = Clay__MeasuredWordArray_Init(maxMeasureTextCacheWordCount, context->internalArena);
	context->pointerOverIds = Clay__ElementIdArray_Init(maxElementCount, context->internalArena);
	context->debugElementData = Clay__DebugElementDataArray_Init(maxElementCount, context->internalArena);
}

void Clay__CompressChildrenAlongAxis(bool xAxis, r32 totalSizeToDistribute, i32Array resizableContainerBuffer)
{
	Clay_Context* context = Clay_GetCurrentContext();
	i32Array largestContainers = context->openClipElementStack;
	
	while (totalSizeToDistribute > 0.1)
	{
		largestContainers.length = 0;
		r32 largestSize = 0;
		r32 targetSize = 0;
		for (uxx cIndex = 0; cIndex < resizableContainerBuffer.length; ++cIndex)
		{
			Clay_LayoutElement* childElement = Clay_LayoutElementArray_Get(&context->layoutElements, i32Array_GetValue(&resizableContainerBuffer, cIndex));
			r32 childSize = xAxis ? childElement->dimensions.Width : childElement->dimensions.Height;
			if ((childSize - largestSize) < 0.1 && (childSize - largestSize) > -0.1)
			{
				i32Array_Add(&largestContainers, i32Array_GetValue(&resizableContainerBuffer, cIndex));
			}
			else if (childSize > largestSize)
			{
				targetSize = largestSize;
				largestSize = childSize;
				largestContainers.length = 0;
				i32Array_Add(&largestContainers, i32Array_GetValue(&resizableContainerBuffer, cIndex));
			}
			else if (childSize > targetSize)
			{
				targetSize = childSize;
			}
		}
		
		if (largestContainers.length == 0) { return; }
		
		targetSize = MaxR32(targetSize, (largestSize * largestContainers.length) - totalSizeToDistribute) / largestContainers.length;
		
		for (uxx childOffset = 0; childOffset < largestContainers.length; childOffset++)
		{
			i32 childIndex = i32Array_GetValue(&largestContainers, childOffset);
			Clay_LayoutElement* childElement = Clay_LayoutElementArray_Get(&context->layoutElements, childIndex);
			r32* childSize = xAxis ? &childElement->dimensions.Width : &childElement->dimensions.Height;
			r32 childMinSize = xAxis ? childElement->minDimensions.Width : childElement->minDimensions.Height;
			r32 oldChildSize = *childSize;
			*childSize = MaxR32(childMinSize, targetSize);
			totalSizeToDistribute -= (oldChildSize - *childSize);
			if (*childSize == childMinSize)
			{
				for (uxx cIndex = 0; cIndex < resizableContainerBuffer.length; cIndex++)
				{
					if (i32Array_GetValue(&resizableContainerBuffer, cIndex) == childIndex)
					{
						i32Array_RemoveSwapback(&resizableContainerBuffer, cIndex);
						break;
					}
				}
			}
		}
	}
}

void Clay__SizeContainersAlongAxis(bool xAxis)
{
	Clay_Context* context = Clay_GetCurrentContext();
	i32Array bfsBuffer = context->layoutElementChildrenBuffer;
	i32Array resizableContainerBuffer = context->openLayoutElementStack;
	for (uxx rootIndex = 0; rootIndex < context->layoutElementTreeRoots.length; ++rootIndex)
	{
		bfsBuffer.length = 0;
		Clay__LayoutElementTreeRoot* root = Clay__LayoutElementTreeRootArray_Get(&context->layoutElementTreeRoots, rootIndex);
		Clay_LayoutElement* rootElement = Clay_LayoutElementArray_Get(&context->layoutElements, (int)root->layoutElementIndex);
		i32Array_Add(&bfsBuffer, (i32)root->layoutElementIndex);
		
		// Size floating containers to their parents
		if (Clay__ElementHasConfig(rootElement, CLAY__ELEMENT_CONFIG_TYPE_FLOATING))
		{
			Clay_FloatingElementConfig* floatingElementConfig = Clay__FindElementConfigWithType(rootElement, CLAY__ELEMENT_CONFIG_TYPE_FLOATING).floatingElementConfig;
			Clay_LayoutElementHashMapItem* parentItem = Clay__GetHashMapItem(floatingElementConfig->parentId);
			if (parentItem && parentItem != &Clay_LayoutElementHashMapItem_DEFAULT)
			{
				Clay_LayoutElement* parentLayoutElement = parentItem->layoutElement;
				if (rootElement->layoutConfig->sizing.width.type == CLAY__SIZING_TYPE_GROW)
				{
					rootElement->dimensions.Width = parentLayoutElement->dimensions.Width;
				}
				if (rootElement->layoutConfig->sizing.height.type == CLAY__SIZING_TYPE_GROW)
				{
					rootElement->dimensions.Height = parentLayoutElement->dimensions.Height;
				}
			}
		}
		
		rootElement->dimensions.Width = MinR32(MaxR32(rootElement->dimensions.Width, rootElement->layoutConfig->sizing.width.size.minMax.min), rootElement->layoutConfig->sizing.width.size.minMax.max);
		rootElement->dimensions.Height = MinR32(MaxR32(rootElement->dimensions.Height, rootElement->layoutConfig->sizing.height.size.minMax.min), rootElement->layoutConfig->sizing.height.size.minMax.max);
		
		for (uxx cIndex = 0; cIndex < bfsBuffer.length; ++cIndex)
		{
			i32 parentIndex = i32Array_GetValue(&bfsBuffer, cIndex);
			Clay_LayoutElement* parent = Clay_LayoutElementArray_Get(&context->layoutElements, parentIndex);
			Clay_LayoutConfig* parentStyleConfig = parent->layoutConfig;
			i32 growContainerCount = 0;
			r32 parentSize = xAxis ? parent->dimensions.Width : parent->dimensions.Height;
			r32 parentPadding = (r32)(xAxis ? (parent->layoutConfig->padding.left + parent->layoutConfig->padding.right) : (parent->layoutConfig->padding.top + parent->layoutConfig->padding.bottom));
			r32 innerContentSize = 0, growContainerContentSize = 0, totalPaddingAndChildGaps = parentPadding;
			bool sizingAlongAxis = (xAxis && parentStyleConfig->layoutDirection == CLAY_LEFT_TO_RIGHT) || (!xAxis && parentStyleConfig->layoutDirection == CLAY_TOP_TO_BOTTOM);
			resizableContainerBuffer.length = 0;
			r32 parentChildGap = parentStyleConfig->childGap;
			
			for (i32 childOffset = 0; childOffset < parent->childrenOrTextContent.children.length; childOffset++)
			{
				i32 childElementIndex = parent->childrenOrTextContent.children.elements[childOffset];
				Clay_LayoutElement* childElement = Clay_LayoutElementArray_Get(&context->layoutElements, childElementIndex);
				Clay_SizingAxis childSizing = xAxis ? childElement->layoutConfig->sizing.width : childElement->layoutConfig->sizing.height;
				r32 childSize = xAxis ? childElement->dimensions.Width : childElement->dimensions.Height;
				
				if (!Clay__ElementHasConfig(childElement, CLAY__ELEMENT_CONFIG_TYPE_TEXT) && childElement->childrenOrTextContent.children.length > 0)
				{
					i32Array_Add(&bfsBuffer, childElementIndex);
				}
				
				if (childSizing.type != CLAY__SIZING_TYPE_PERCENT
					&& childSizing.type != CLAY__SIZING_TYPE_FIXED
					&& (!Clay__ElementHasConfig(childElement, CLAY__ELEMENT_CONFIG_TYPE_TEXT) ||
						(Clay__FindElementConfigWithType(childElement, CLAY__ELEMENT_CONFIG_TYPE_TEXT).textElementConfig->wrapMode == CLAY_TEXT_WRAP_WORDS) ||
						(Clay__FindElementConfigWithType(childElement, CLAY__ELEMENT_CONFIG_TYPE_TEXT).textElementConfig->textAlignment == CLAY_TEXT_ALIGN_SHRINK)) // todo too many loops
					&& (xAxis || !Clay__ElementHasConfig(childElement, CLAY__ELEMENT_CONFIG_TYPE_IMAGE)))
				{
					i32Array_Add(&resizableContainerBuffer, childElementIndex);
				}
				
				if (sizingAlongAxis)
				{
					innerContentSize += (childSizing.type == CLAY__SIZING_TYPE_PERCENT ? 0 : childSize);
					if (childSizing.type == CLAY__SIZING_TYPE_GROW)
					{
						growContainerContentSize += childSize;
						growContainerCount++;
					}
					if (childOffset > 0)
					{
						innerContentSize += parentChildGap; // For children after index 0, the childAxisOffset is the gap from the previous child
						totalPaddingAndChildGaps += parentChildGap;
					}
				}
				else
				{
					innerContentSize = MaxR32(childSize, innerContentSize);
				}
			}
			
			// Expand percentage containers to size
			for (i32 childOffset = 0; childOffset < parent->childrenOrTextContent.children.length; childOffset++)
			{
				i32 childElementIndex = parent->childrenOrTextContent.children.elements[childOffset];
				Clay_LayoutElement* childElement = Clay_LayoutElementArray_Get(&context->layoutElements, childElementIndex);
				Clay_SizingAxis childSizing = xAxis ? childElement->layoutConfig->sizing.width : childElement->layoutConfig->sizing.height;
				r32* childSize = xAxis ? &childElement->dimensions.Width : &childElement->dimensions.Height;
				if (childSizing.type == CLAY__SIZING_TYPE_PERCENT)
				{
					*childSize = (parentSize - totalPaddingAndChildGaps) * childSizing.size.percent;
					if (sizingAlongAxis) { innerContentSize += *childSize; }
					Clay__UpdateAspectRatioBox(childElement);
				}
			}
			
			if (sizingAlongAxis)
			{
				r32 sizeToDistribute = parentSize - parentPadding - innerContentSize;
				// The content is too large, compress the children as much as possible
				if (sizeToDistribute < 0)
				{
					// If the parent can scroll in the axis direction in this direction, don't compress children, just leave them alone
					Clay_ScrollElementConfig* scrollElementConfig = Clay__FindElementConfigWithType(parent, CLAY__ELEMENT_CONFIG_TYPE_SCROLL).scrollElementConfig;
					if (scrollElementConfig)
					{
						if (((xAxis && scrollElementConfig->horizontal) || (!xAxis && scrollElementConfig->vertical))) { continue; }
					}
					// Scrolling containers preferentially compress before others
					Clay__CompressChildrenAlongAxis(xAxis, -sizeToDistribute, resizableContainerBuffer);
				// The content is too small, allow SIZING_GROW containers to expand
				}
				else if (sizeToDistribute > 0 && growContainerCount > 0)
				{
					r32 targetSize = (sizeToDistribute + growContainerContentSize) / (r32)growContainerCount;
					for (ixx childOffset = 0; (uxx)childOffset < resizableContainerBuffer.length; childOffset++)
					{
						Clay_LayoutElement* childElement = Clay_LayoutElementArray_Get(&context->layoutElements, i32Array_GetValue(&resizableContainerBuffer, (uxx)childOffset));
						Clay_SizingAxis childSizing = xAxis ? childElement->layoutConfig->sizing.width : childElement->layoutConfig->sizing.height;
						if (childSizing.type == CLAY__SIZING_TYPE_GROW)
						{
							r32* childSize = xAxis ? &childElement->dimensions.Width : &childElement->dimensions.Height;
							r32* minSize = xAxis ? &childElement->minDimensions.Width : &childElement->minDimensions.Height;
							if (targetSize < *minSize)
							{
								growContainerContentSize -= *minSize;
								i32Array_RemoveSwapback(&resizableContainerBuffer, (uxx)childOffset);
								growContainerCount--;
								targetSize = (sizeToDistribute + growContainerContentSize) / (r32)growContainerCount;
								childOffset = -1;
								continue;
							}
							*childSize = targetSize;
						}
					}
				}
			// Sizing along the non layout axis ("off axis")
			}
			else
			{
				for (uxx childOffset = 0; childOffset < resizableContainerBuffer.length; childOffset++)
				{
					Clay_LayoutElement* childElement = Clay_LayoutElementArray_Get(&context->layoutElements, i32Array_GetValue(&resizableContainerBuffer, childOffset));
					Clay_SizingAxis childSizing = xAxis ? childElement->layoutConfig->sizing.width : childElement->layoutConfig->sizing.height;
					r32* childSize = xAxis ? &childElement->dimensions.Width : &childElement->dimensions.Height;
					
					if (!xAxis && Clay__ElementHasConfig(childElement, CLAY__ELEMENT_CONFIG_TYPE_IMAGE)) { continue; } // Currently we don't support resizing aspect ratio images on the Y axis because it would break the ratio
					
					// If we're laying out the children of a scroll panel, grow containers expand to the height of the inner content, not the outer container
					r32 maxSize = parentSize - parentPadding;
					if (Clay__ElementHasConfig(parent, CLAY__ELEMENT_CONFIG_TYPE_SCROLL))
					{
						Clay_ScrollElementConfig* scrollElementConfig = Clay__FindElementConfigWithType(parent, CLAY__ELEMENT_CONFIG_TYPE_SCROLL).scrollElementConfig;
						if (((xAxis && scrollElementConfig->horizontal) || (!xAxis && scrollElementConfig->vertical)))
						{
							maxSize = MaxR32(maxSize, innerContentSize);
						}
					}
					if (childSizing.type == CLAY__SIZING_TYPE_FIT)
					{
						*childSize = MaxR32(childSizing.size.minMax.min, MinR32(*childSize, maxSize));
					}
					else if (childSizing.type == CLAY__SIZING_TYPE_GROW)
					{
						*childSize = MinR32(maxSize, childSizing.size.minMax.max);
					}
				}
			}
		}
	}
}

Str8 Clay__IntToString(i32 integer)
{
	if (integer == 0) { return StrLit("0"); }
	Clay_Context* context = Clay_GetCurrentContext();
	char* chars = (char*)(context->dynamicStringData.items + context->dynamicStringData.length);
	i32 length = 0;
	i32 sign = integer;
	
	if (integer < 0) { integer = -integer; }
	while (integer > 0)
	{
		chars[length++] = (char)(integer % 10 + '0');
		integer /= 10;
	}
	
	if (sign < 0) { chars[length++] = '-'; }
	
	// Reverse the string to get the correct order
	for (i32 j = 0, k = length - 1; j < k; j++, k--)
	{
		char temp = chars[j];
		chars[j] = chars[k];
		chars[k] = temp;
	}
	context->dynamicStringData.length += length;
	return MakeStr8(length, chars);
}

void Clay__AddRenderCommand(Clay_RenderCommand renderCommand)
{
	Clay_Context* context = Clay_GetCurrentContext();
	if (context->renderCommands.length < context->renderCommands.allocLength - 1)
	{
		Clay_RenderCommandArray_Add(&context->renderCommands, renderCommand);
	}
	else
	{
		if (!context->booleanWarnings.maxRenderCommandsExceeded)
		{
			context->booleanWarnings.maxRenderCommandsExceeded = true;
			context->errorHandler.errorHandlerFunction(NEW_STRUCT(Clay_ErrorData) {
				.errorType = CLAY_ERROR_TYPE_ELEMENTS_CAPACITY_EXCEEDED,
				.errorText = CLAY_STRING("Clay ran out of capacity while attempting to create render commands. This is usually caused by a large amount of wrapping text elements while close to the max element capacity. Try using Clay_SetMaxElementCount() with a higher value."),
				.userData = context->errorHandler.userData
			});
		}
	}
}

bool Clay__ElementIsOffscreen(rec* boundingBox)
{
	Clay_Context* context = Clay_GetCurrentContext();
	if (context->disableCulling) { return false; }
	
	return (
		(boundingBox->X > (r32)context->layoutDimensions.Width) ||
		(boundingBox->Y > (r32)context->layoutDimensions.Height) ||
		(boundingBox->X + boundingBox->Width < 0) ||
		(boundingBox->Y + boundingBox->Height < 0));
}

void Clay__CalculateFinalLayout(void)
{
	Clay_Context* context = Clay_GetCurrentContext();
	// Calculate sizing along the X axis
	Clay__SizeContainersAlongAxis(true);
	
	// Wrap text
	for (uxx textElementIndex = 0; textElementIndex < context->textElementData.length; ++textElementIndex)
	{
		Clay__TextElementData* textElementData = Clay__TextElementDataArray_Get(&context->textElementData, textElementIndex);
		textElementData->wrappedLines = NEW_STRUCT(Clay__WrappedTextLineArraySlice) { .length = 0, .items = &context->wrappedTextLines.items[context->wrappedTextLines.length] };
		Clay_LayoutElement* containerElement = Clay_LayoutElementArray_Get(&context->layoutElements, (int)textElementData->elementIndex);
		Clay_TextElementConfig* textConfig = Clay__FindElementConfigWithType(containerElement, CLAY__ELEMENT_CONFIG_TYPE_TEXT).textElementConfig;
		Clay__MeasureTextCacheItem* measureTextCacheItem = Clay__MeasureTextCached(&textElementData->text, textConfig);
		bool considerNewLines = (textConfig->wrapMode == CLAY_TEXT_WRAP_NEWLINES || textConfig->wrapMode == CLAY_TEXT_WRAP_WORDS);
		bool considerMaxWidth = (textConfig->wrapMode == CLAY_TEXT_WRAP_WORDS && textConfig->textAlignment != CLAY_TEXT_ALIGN_SHRINK);
		r32 lineWidth = 0;
		r32 lineHeight = textConfig->lineHeight > 0 ? (r32)textConfig->lineHeight : textElementData->preferredDimensions.Height;
		i32 lineLengthChars = 0;
		i32 lineStartOffset = 0;
		if (!measureTextCacheItem->containsNewlines && textElementData->preferredDimensions.Width <= containerElement->dimensions.Width)
		{
			Clay__WrappedTextLineArray_Add(&context->wrappedTextLines, NEW_STRUCT(Clay__WrappedTextLine) { containerElement->dimensions,  textElementData->text });
			textElementData->wrappedLines.length++;
			continue;
		}
		r32 spaceWidth = Clay__MeasureText(Str8_Space, textConfig, context->measureTextUserData).Width;
		i32 wordIndex = measureTextCacheItem->measuredWordsStartIndex;
		while (wordIndex != -1)
		{
			if (context->wrappedTextLines.length > context->wrappedTextLines.allocLength - 1) { break; }
			Clay__MeasuredWord* measuredWord = Clay__MeasuredWordArray_Get(&context->measuredWords, wordIndex);
			// Only word on the line is too large, just render it anyway
			if (lineLengthChars == 0 && lineWidth + measuredWord->width > containerElement->dimensions.Width && considerMaxWidth)
			{
				Clay__WrappedTextLineArray_Add(&context->wrappedTextLines, NEW_STRUCT(Clay__WrappedTextLine) { MakeV2(measuredWord->width, lineHeight), { .length = measuredWord->length, .chars = &textElementData->text.chars[measuredWord->startOffset] } });
				textElementData->wrappedLines.length++;
				wordIndex = measuredWord->next;
				lineStartOffset = measuredWord->startOffset + measuredWord->length;
			}
			// measuredWord->length == 0 means a newline character
			else if ((measuredWord->length == 0 && considerNewLines) || (lineWidth + measuredWord->width > containerElement->dimensions.Width && considerMaxWidth))
			{
				// Wrapped text lines list has overflowed, just render out the line
				bool finalCharIsSpace = textElementData->text.chars[lineStartOffset + lineLengthChars - 1] == ' ';
				Clay__WrappedTextLineArray_Add(&context->wrappedTextLines, NEW_STRUCT(Clay__WrappedTextLine) { MakeV2(lineWidth + (finalCharIsSpace ? -spaceWidth : 0), lineHeight), { .length = lineLengthChars + (finalCharIsSpace ? -1 : 0), .chars = &textElementData->text.chars[lineStartOffset] } });
				textElementData->wrappedLines.length++;
				if (lineLengthChars == 0 || measuredWord->length == 0) { wordIndex = measuredWord->next; }
				lineWidth = 0;
				lineLengthChars = 0;
				lineStartOffset = measuredWord->startOffset;
			}
			else
			{
				lineWidth += measuredWord->width;
				lineLengthChars += measuredWord->length;
				wordIndex = measuredWord->next;
			}
		}
		if (lineLengthChars > 0)
		{
			Clay__WrappedTextLineArray_Add(&context->wrappedTextLines, NEW_STRUCT(Clay__WrappedTextLine) { MakeV2(lineWidth, lineHeight), {.length = lineLengthChars, .chars = &textElementData->text.chars[lineStartOffset] } });
			textElementData->wrappedLines.length++;
		}
		containerElement->dimensions.Height = lineHeight * (r32)textElementData->wrappedLines.length;
	}
	
	// Scale vertical image heights according to aspect ratio
	for (uxx pIndex = 0; pIndex < context->imageElementPointers.length; ++pIndex)
	{
		Clay_LayoutElement* imageElement = Clay_LayoutElementArray_Get(&context->layoutElements, i32Array_GetValue(&context->imageElementPointers, pIndex));
		Clay_ImageElementConfig* config = Clay__FindElementConfigWithType(imageElement, CLAY__ELEMENT_CONFIG_TYPE_IMAGE).imageElementConfig;
		imageElement->dimensions.Height = (config->sourceDimensions.Height / MaxR32(config->sourceDimensions.Width, 1)) * imageElement->dimensions.Width;
	}
	
	// Propagate effect of text wrapping, image aspect scaling etc. on height of parents
	Clay__LayoutElementTreeNodeArray dfsBuffer = context->layoutElementTreeNodeArray1;
	dfsBuffer.length = 0;
	for (uxx rIndex = 0; rIndex < context->layoutElementTreeRoots.length; ++rIndex)
	{
		Clay__LayoutElementTreeRoot* root = Clay__LayoutElementTreeRootArray_Get(&context->layoutElementTreeRoots, rIndex);
		context->treeNodeVisited.items[dfsBuffer.length] = false;
		Clay__LayoutElementTreeNodeArray_Add(&dfsBuffer, NEW_STRUCT(Clay__LayoutElementTreeNode) { .layoutElement = Clay_LayoutElementArray_Get(&context->layoutElements, (int)root->layoutElementIndex) });
	}
	while (dfsBuffer.length > 0)
	{
		Clay__LayoutElementTreeNode* currentElementTreeNode = Clay__LayoutElementTreeNodeArray_Get(&dfsBuffer, (int)dfsBuffer.length - 1);
		Clay_LayoutElement* currentElement = currentElementTreeNode->layoutElement;
		if (!context->treeNodeVisited.items[dfsBuffer.length - 1])
		{
			context->treeNodeVisited.items[dfsBuffer.length - 1] = true;
			// If the element has no children or is the container for a text element, don't bother inspecting it
			if (Clay__ElementHasConfig(currentElement, CLAY__ELEMENT_CONFIG_TYPE_TEXT) || currentElement->childrenOrTextContent.children.length == 0)
			{
				dfsBuffer.length--;
				continue;
			}
			// Add the children to the DFS buffer (needs to be pushed in reverse so that stack traversal is in correct layout order)
			for (i32 i = 0; i < currentElement->childrenOrTextContent.children.length; i++)
			{
				context->treeNodeVisited.items[dfsBuffer.length] = false;
				Clay__LayoutElementTreeNodeArray_Add(&dfsBuffer, NEW_STRUCT(Clay__LayoutElementTreeNode) { .layoutElement = Clay_LayoutElementArray_Get(&context->layoutElements, currentElement->childrenOrTextContent.children.elements[i]) });
			}
			continue;
		}
		dfsBuffer.length--;
		
		// DFS node has been visited, this is on the way back up to the root
		Clay_LayoutConfig* layoutConfig = currentElement->layoutConfig;
		if (layoutConfig->layoutDirection == CLAY_LEFT_TO_RIGHT)
		{
			// Resize any parent containers that have grown in height along their non layout axis
			for (i32 j = 0; j < currentElement->childrenOrTextContent.children.length; ++j)
			{
				Clay_LayoutElement* childElement = Clay_LayoutElementArray_Get(&context->layoutElements, currentElement->childrenOrTextContent.children.elements[j]);
				r32 childHeightWithPadding = MaxR32(childElement->dimensions.Height + layoutConfig->padding.top + layoutConfig->padding.bottom, currentElement->dimensions.Height);
				currentElement->dimensions.Height = MinR32(MaxR32(childHeightWithPadding, layoutConfig->sizing.height.size.minMax.min), layoutConfig->sizing.height.size.minMax.max);
			}
		}
		else if (layoutConfig->layoutDirection == CLAY_TOP_TO_BOTTOM)
		{
			// Resizing along the layout axis
			r32 contentHeight = (r32)(layoutConfig->padding.top + layoutConfig->padding.bottom);
			for (i32 j = 0; j < currentElement->childrenOrTextContent.children.length; ++j)
			{
				Clay_LayoutElement* childElement = Clay_LayoutElementArray_Get(&context->layoutElements, currentElement->childrenOrTextContent.children.elements[j]);
				contentHeight += childElement->dimensions.Height;
			}
			contentHeight += (r32)(MaxU16(currentElement->childrenOrTextContent.children.length - 1, 0) * layoutConfig->childGap);
			currentElement->dimensions.Height = MinR32(MaxR32(contentHeight, layoutConfig->sizing.height.size.minMax.min), layoutConfig->sizing.height.size.minMax.max);
		}
	}
	
	// Calculate sizing along the Y axis
	Clay__SizeContainersAlongAxis(false);
	
	// Sort tree roots by z-index
	uxx sortMax = context->layoutElementTreeRoots.length - 1;
	while (sortMax > 0)
	{ // todo dumb bubble sort
		for (uxx sIndex = 0; sIndex < sortMax; ++sIndex)
		{
			Clay__LayoutElementTreeRoot current = *Clay__LayoutElementTreeRootArray_Get(&context->layoutElementTreeRoots, sIndex);
			Clay__LayoutElementTreeRoot next = *Clay__LayoutElementTreeRootArray_Get(&context->layoutElementTreeRoots, sIndex + 1);
			if (next.zIndex < current.zIndex)
			{
				Clay__LayoutElementTreeRootArray_Set(&context->layoutElementTreeRoots, sIndex, next);
				Clay__LayoutElementTreeRootArray_Set(&context->layoutElementTreeRoots, sIndex + 1, current);
			}
		}
		sortMax--;
	}
	
	// Calculate final positions and generate render commands
	context->renderCommands.length = 0;
	dfsBuffer.length = 0;
	for (uxx rootIndex = 0; rootIndex < context->layoutElementTreeRoots.length; ++rootIndex)
	{
		dfsBuffer.length = 0;
		Clay__LayoutElementTreeRoot* root = Clay__LayoutElementTreeRootArray_Get(&context->layoutElementTreeRoots, rootIndex);
		Clay_LayoutElement* rootElement = Clay_LayoutElementArray_Get(&context->layoutElements, (int)root->layoutElementIndex);
		v2 rootPosition = ZEROED;
		Clay_LayoutElementHashMapItem* parentHashMapItem = Clay__GetHashMapItem(root->parentId);
		// Position root floating containers
		if (Clay__ElementHasConfig(rootElement, CLAY__ELEMENT_CONFIG_TYPE_FLOATING) && parentHashMapItem)
		{
			Clay_FloatingElementConfig* config = Clay__FindElementConfigWithType(rootElement, CLAY__ELEMENT_CONFIG_TYPE_FLOATING).floatingElementConfig;
			v2 rootDimensions = rootElement->dimensions;
			rec parentBoundingBox = parentHashMapItem->boundingBox;
			// Set X position
			v2 targetAttachPosition = ZEROED;
			switch (config->attachPoints.parent)
			{
				case CLAY_ATTACH_POINT_LEFT_TOP:
				case CLAY_ATTACH_POINT_LEFT_CENTER:
				case CLAY_ATTACH_POINT_LEFT_BOTTOM: targetAttachPosition.X = parentBoundingBox.X; break;
				case CLAY_ATTACH_POINT_CENTER_TOP:
				case CLAY_ATTACH_POINT_CENTER_CENTER:
				case CLAY_ATTACH_POINT_CENTER_BOTTOM: targetAttachPosition.X = parentBoundingBox.X + (parentBoundingBox.Width / 2); break;
				case CLAY_ATTACH_POINT_RIGHT_TOP:
				case CLAY_ATTACH_POINT_RIGHT_CENTER:
				case CLAY_ATTACH_POINT_RIGHT_BOTTOM: targetAttachPosition.X = parentBoundingBox.X + parentBoundingBox.Width; break;
			}
			switch (config->attachPoints.element)
			{
				case CLAY_ATTACH_POINT_LEFT_TOP:
				case CLAY_ATTACH_POINT_LEFT_CENTER:
				case CLAY_ATTACH_POINT_LEFT_BOTTOM: break;
				case CLAY_ATTACH_POINT_CENTER_TOP:
				case CLAY_ATTACH_POINT_CENTER_CENTER:
				case CLAY_ATTACH_POINT_CENTER_BOTTOM: targetAttachPosition.X -= (rootDimensions.Width / 2); break;
				case CLAY_ATTACH_POINT_RIGHT_TOP:
				case CLAY_ATTACH_POINT_RIGHT_CENTER:
				case CLAY_ATTACH_POINT_RIGHT_BOTTOM: targetAttachPosition.X -= rootDimensions.Width; break;
			}
			switch (config->attachPoints.parent) // I know I could merge the x and y switch statements, but this is easier to read
			{
				case CLAY_ATTACH_POINT_LEFT_TOP:
				case CLAY_ATTACH_POINT_RIGHT_TOP:
				case CLAY_ATTACH_POINT_CENTER_TOP: targetAttachPosition.Y = parentBoundingBox.Y; break;
				case CLAY_ATTACH_POINT_LEFT_CENTER:
				case CLAY_ATTACH_POINT_CENTER_CENTER:
				case CLAY_ATTACH_POINT_RIGHT_CENTER: targetAttachPosition.Y = parentBoundingBox.Y + (parentBoundingBox.Height / 2); break;
				case CLAY_ATTACH_POINT_LEFT_BOTTOM:
				case CLAY_ATTACH_POINT_CENTER_BOTTOM:
				case CLAY_ATTACH_POINT_RIGHT_BOTTOM: targetAttachPosition.Y = parentBoundingBox.Y + parentBoundingBox.Height; break;
			}
			switch (config->attachPoints.element)
			{
				case CLAY_ATTACH_POINT_LEFT_TOP:
				case CLAY_ATTACH_POINT_RIGHT_TOP:
				case CLAY_ATTACH_POINT_CENTER_TOP: break;
				case CLAY_ATTACH_POINT_LEFT_CENTER:
				case CLAY_ATTACH_POINT_CENTER_CENTER:
				case CLAY_ATTACH_POINT_RIGHT_CENTER: targetAttachPosition.Y -= (rootDimensions.Height / 2); break;
				case CLAY_ATTACH_POINT_LEFT_BOTTOM:
				case CLAY_ATTACH_POINT_CENTER_BOTTOM:
				case CLAY_ATTACH_POINT_RIGHT_BOTTOM: targetAttachPosition.Y -= rootDimensions.Height; break;
			}
			targetAttachPosition.X += config->offset.X;
			targetAttachPosition.Y += config->offset.Y;
			rootPosition = targetAttachPosition;
		}
		if (root->clipElementId)
		{
			Clay_LayoutElementHashMapItem* clipHashMapItem = Clay__GetHashMapItem(root->clipElementId);
			if (clipHashMapItem)
			{
				// Floating elements that are attached to scrolling contents won't be correctly positioned if external scroll handling is enabled, fix here
				if (context->externalScrollHandlingEnabled)
				{
					Clay_ScrollElementConfig* scrollConfig = Clay__FindElementConfigWithType(clipHashMapItem->layoutElement, CLAY__ELEMENT_CONFIG_TYPE_SCROLL).scrollElementConfig;
					for (uxx sIndex = 0; sIndex < context->scrollContainerDatas.length; sIndex++)
					{
						Clay__ScrollContainerDataInternal* mapping = Clay__ScrollContainerDataInternalArray_Get(&context->scrollContainerDatas, sIndex);
						if (mapping->layoutElement == clipHashMapItem->layoutElement)
						{
							root->pointerOffset = mapping->scrollPosition;
							if (scrollConfig->horizontal) { rootPosition.X += mapping->scrollPosition.X; }
							if (scrollConfig->vertical) { rootPosition.Y += mapping->scrollPosition.X; }
							break;
						}
					}
				}
				Clay__AddRenderCommand(NEW_STRUCT(Clay_RenderCommand) {
					.boundingBox = clipHashMapItem->boundingBox,
					.userData = ZEROED,
					.id = Clay__HashNumber(rootElement->id, rootElement->childrenOrTextContent.children.length + 10).id, // TODO need a better strategy for managing derived ids
					.zIndex = root->zIndex,
					.commandType = CLAY_RENDER_COMMAND_TYPE_SCISSOR_START,
				});
			}
		}
		Clay__LayoutElementTreeNodeArray_Add(&dfsBuffer, NEW_STRUCT(Clay__LayoutElementTreeNode) { .layoutElement = rootElement, .position = rootPosition, .nextChildOffset = { .X = (r32)rootElement->layoutConfig->padding.left, .Y = (r32)rootElement->layoutConfig->padding.top } });
		
		context->treeNodeVisited.items[0] = false;
		while (dfsBuffer.length > 0)
		{
			Clay__LayoutElementTreeNode* currentElementTreeNode = Clay__LayoutElementTreeNodeArray_Get(&dfsBuffer, (int)dfsBuffer.length - 1);
			Clay_LayoutElement* currentElement = currentElementTreeNode->layoutElement;
			Clay_LayoutConfig* layoutConfig = currentElement->layoutConfig;
			v2 scrollOffset = ZEROED;
			
			// This will only be run a single time for each element in downwards DFS order
			if (!context->treeNodeVisited.items[dfsBuffer.length - 1])
			{
				context->treeNodeVisited.items[dfsBuffer.length - 1] = true;
				
				rec currentElementBoundingBox = MakeRec(currentElementTreeNode->position.X, currentElementTreeNode->position.Y, currentElement->dimensions.Width, currentElement->dimensions.Height);
				if (Clay__ElementHasConfig(currentElement, CLAY__ELEMENT_CONFIG_TYPE_FLOATING))
				{
					Clay_FloatingElementConfig* floatingElementConfig = Clay__FindElementConfigWithType(currentElement, CLAY__ELEMENT_CONFIG_TYPE_FLOATING).floatingElementConfig;
					v2 expand = floatingElementConfig->expand;
					currentElementBoundingBox.X -= expand.Width;
					currentElementBoundingBox.Width += expand.Width * 2;
					currentElementBoundingBox.Y -= expand.Height;
					currentElementBoundingBox.Height += expand.Height * 2;
				}
				
				Clay__ScrollContainerDataInternal* scrollContainerData = nullptr;
				// Apply scroll offsets to container
				if (Clay__ElementHasConfig(currentElement, CLAY__ELEMENT_CONFIG_TYPE_SCROLL))
				{
					Clay_ScrollElementConfig* scrollConfig = Clay__FindElementConfigWithType(currentElement, CLAY__ELEMENT_CONFIG_TYPE_SCROLL).scrollElementConfig;
					
					// This linear scan could theoretically be slow under very strange conditions, but I can't imagine a real UI with more than a few 10's of scroll containers
					for (uxx sIndex = 0; sIndex < context->scrollContainerDatas.length; sIndex++)
					{
						Clay__ScrollContainerDataInternal* mapping = Clay__ScrollContainerDataInternalArray_Get(&context->scrollContainerDatas, sIndex);
						if (mapping->layoutElement == currentElement)
						{
							scrollContainerData = mapping;
							mapping->boundingBox = currentElementBoundingBox;
							if (scrollConfig->horizontal) { scrollOffset.X = mapping->scrollPosition.X; }
							if (scrollConfig->vertical) { scrollOffset.Y = mapping->scrollPosition.Y; }
							if (context->externalScrollHandlingEnabled) { scrollOffset = V2_Zero; }
							break;
						}
					}
				}
				
				Clay_LayoutElementHashMapItem* hashMapItem = Clay__GetHashMapItem(currentElement->id);
				if (hashMapItem)
				{
					hashMapItem->boundingBox = currentElementBoundingBox;
					if (hashMapItem->idAlias)
					{
						Clay_LayoutElementHashMapItem* hashMapItemAlias = Clay__GetHashMapItem(hashMapItem->idAlias);
						if (hashMapItemAlias)
						{
							hashMapItemAlias->boundingBox = currentElementBoundingBox;
						}
					}
				}
				
				uxx sortedConfigIndexes[20];
				for (uxx elementConfigIndex = 0; elementConfigIndex < currentElement->elementConfigs.length; ++elementConfigIndex)
				{
					sortedConfigIndexes[elementConfigIndex] = elementConfigIndex;
				}
				sortMax = (currentElement->elementConfigs.length > 0) ? currentElement->elementConfigs.length - 1 : 0;
				while (sortMax > 0) // todo dumb bubble sort
				{
					for (uxx sIndex = 0; sIndex < sortMax; ++sIndex)
					{
						uxx current = sortedConfigIndexes[sIndex];
						uxx next = sortedConfigIndexes[sIndex + 1];
						Clay__ElementConfigType currentType = Clay__ElementConfigArray_GetSlice(&currentElement->elementConfigs, current)->type;
						Clay__ElementConfigType nextType = Clay__ElementConfigArray_GetSlice(&currentElement->elementConfigs, next)->type;
						if (nextType == CLAY__ELEMENT_CONFIG_TYPE_SCROLL || currentType == CLAY__ELEMENT_CONFIG_TYPE_BORDER)
						{
							sortedConfigIndexes[sIndex] = next;
							sortedConfigIndexes[sIndex + 1] = current;
						}
					}
					sortMax--;
				}
				
				bool emitRectangle = false;
				// Create the render commands for this element
				Clay_SharedElementConfig* sharedConfig = Clay__FindElementConfigWithType(currentElement, CLAY__ELEMENT_CONFIG_TYPE_SHARED).sharedElementConfig;
				if (sharedConfig && sharedConfig->backgroundColor.a > 0) { emitRectangle = true; }
				else if (!sharedConfig)
				{
					emitRectangle = false;
					sharedConfig = &Clay_SharedElementConfig_DEFAULT;
				}
				for (uxx elementConfigIndex = 0; elementConfigIndex < currentElement->elementConfigs.length; ++elementConfigIndex)
				{
					Clay_ElementConfig* elementConfig = Clay__ElementConfigArray_GetSlice(&currentElement->elementConfigs, sortedConfigIndexes[elementConfigIndex]);
					Clay_RenderCommand renderCommand = {
						.boundingBox = currentElementBoundingBox,
						.userData = sharedConfig->userData,
						.id = currentElement->id,
					};
					
					bool offscreen = Clay__ElementIsOffscreen(&currentElementBoundingBox);
					// Culling - Don't bother to generate render commands for rectangles entirely outside the screen - this won't stop their children from being rendered if they overflow
					bool shouldRender = !offscreen;
					switch (elementConfig->type)
					{
						case CLAY__ELEMENT_CONFIG_TYPE_FLOATING:
						case CLAY__ELEMENT_CONFIG_TYPE_SHARED:
						case CLAY__ELEMENT_CONFIG_TYPE_BORDER:
						{
							shouldRender = false;
							break;
						}
						case CLAY__ELEMENT_CONFIG_TYPE_SCROLL:
						{
							renderCommand.commandType = CLAY_RENDER_COMMAND_TYPE_SCISSOR_START;
							renderCommand.renderData = NEW_STRUCT(Clay_RenderData) {
								.scroll = {
									.horizontal = elementConfig->config.scrollElementConfig->horizontal,
									.vertical = elementConfig->config.scrollElementConfig->vertical,
									.scrollLag = elementConfig->config.scrollElementConfig->scrollLag,
								}
							};
							break;
						}
						case CLAY__ELEMENT_CONFIG_TYPE_IMAGE:
						{
							renderCommand.commandType = CLAY_RENDER_COMMAND_TYPE_IMAGE;
							renderCommand.renderData = NEW_STRUCT(Clay_RenderData) {
								.image = {
									.backgroundColor = sharedConfig->backgroundColor,
									.cornerRadius = sharedConfig->cornerRadius,
									.sourceDimensions = elementConfig->config.imageElementConfig->sourceDimensions,
									.imageData = elementConfig->config.imageElementConfig->imageData,
							   }
							};
							emitRectangle = false;
							break;
						}
						case CLAY__ELEMENT_CONFIG_TYPE_TEXT:
						{
							if (!shouldRender) { break; }
							shouldRender = false;
							Clay_ElementConfigUnion configUnion = elementConfig->config;
							Clay_TextElementConfig* textElementConfig = configUnion.textElementConfig;
							r32 naturalLineHeight = currentElement->childrenOrTextContent.textElementData->preferredDimensions.Height;
							r32 finalLineHeight = textElementConfig->lineHeight > 0 ? (r32)textElementConfig->lineHeight : naturalLineHeight;
							r32 lineHeightOffset = (finalLineHeight - naturalLineHeight) / 2;
							r32 yPosition = lineHeightOffset;
							for (uxx lineIndex = 0; lineIndex < currentElement->childrenOrTextContent.textElementData->wrappedLines.length; ++lineIndex)
							{
								Clay__WrappedTextLine* wrappedLine = Clay__WrappedTextLineArray_GetSlice(&currentElement->childrenOrTextContent.textElementData->wrappedLines, lineIndex);
								if (wrappedLine->line.length == 0)
								{
									yPosition += finalLineHeight;
									continue;
								}
								r32 offset = (currentElementBoundingBox.Width - wrappedLine->dimensions.Width);
								if (textElementConfig->textAlignment == CLAY_TEXT_ALIGN_LEFT || textElementConfig->textAlignment == CLAY_TEXT_ALIGN_SHRINK)
								{
									offset = 0;
								}
								if (textElementConfig->textAlignment == CLAY_TEXT_ALIGN_CENTER)
								{
									offset /= 2;
								}
								rec boundingBox = MakeRec(
									currentElementBoundingBox.X + offset,
									currentElementBoundingBox.Y + yPosition,
									wrappedLine->dimensions.Width,
									wrappedLine->dimensions.Height
								);
								if (textElementConfig->textAlignment == CLAY_TEXT_ALIGN_SHRINK && boundingBox.Width > currentElementBoundingBox.Width)
								{
									boundingBox.Width = currentElementBoundingBox.Width;
								}
								Clay__AddRenderCommand(NEW_STRUCT(Clay_RenderCommand) {
									.boundingBox = boundingBox,
									.renderData = { .text = {
										.stringContents = wrappedLine->line,
										.textColor = textElementConfig->textColor,
										.fontId = textElementConfig->fontId,
										.fontSize = textElementConfig->fontSize,
										.letterSpacing = textElementConfig->letterSpacing,
										.lineHeight = textElementConfig->lineHeight,
										.userData = textElementConfig->userData,
									}},
									.userData = sharedConfig->userData,
									.id = Clay__HashNumber(lineIndex, currentElement->id).id,
									.zIndex = root->zIndex,
									.commandType = CLAY_RENDER_COMMAND_TYPE_TEXT,
								});
								yPosition += finalLineHeight;
								
								if (!context->disableCulling && (currentElementBoundingBox.Y + yPosition > context->layoutDimensions.Height)) { break; }
							}
							break;
						}
						case CLAY__ELEMENT_CONFIG_TYPE_CUSTOM:
						{
							renderCommand.commandType = CLAY_RENDER_COMMAND_TYPE_CUSTOM;
							renderCommand.renderData = NEW_STRUCT(Clay_RenderData) {
								.custom = {
									.backgroundColor = sharedConfig->backgroundColor,
									.cornerRadius = sharedConfig->cornerRadius,
									.customData = elementConfig->config.customElementConfig->customData,
								}
							};
							emitRectangle = false;
							break;
						}
						default: break;
					}
					if (shouldRender)
					{
						Clay__AddRenderCommand(renderCommand);
					}
					if (offscreen)
					{
						// NOTE: You may be tempted to try an early return / continue if an element is off screen. Why bother calculating layout for its children, right?
						// Unfortunately, a FLOATING_CONTAINER may be defined that attaches to a child or grandchild of this element, which is large enough to still
						// be on screen, even if this element isn't. That depends on this element and it's children being laid out correctly (even if they are entirely off screen)
					}
				}
				
				if (emitRectangle)
				{
					Clay__AddRenderCommand(NEW_STRUCT(Clay_RenderCommand) {
						.boundingBox = currentElementBoundingBox,
						.renderData = { .rectangle = {
								.backgroundColor = sharedConfig->backgroundColor,
								.cornerRadius = sharedConfig->cornerRadius,
						}},
						.userData = sharedConfig->userData,
						.id = currentElement->id,
						.zIndex = root->zIndex,
						.commandType = CLAY_RENDER_COMMAND_TYPE_RECTANGLE,
					});
				}
				
				// Setup initial on-axis alignment
				if (!Clay__ElementHasConfig(currentElementTreeNode->layoutElement, CLAY__ELEMENT_CONFIG_TYPE_TEXT))
				{
					v2 contentSize = V2_Zero;
					if (layoutConfig->layoutDirection == CLAY_LEFT_TO_RIGHT)
					{
						for (i32 i = 0; i < currentElement->childrenOrTextContent.children.length; ++i)
						{
							Clay_LayoutElement* childElement = Clay_LayoutElementArray_Get(&context->layoutElements, currentElement->childrenOrTextContent.children.elements[i]);
							contentSize.Width += childElement->dimensions.Width;
							contentSize.Height = MaxR32(contentSize.Height, childElement->dimensions.Height);
						}
						contentSize.Width += (r32)(MaxU16(currentElement->childrenOrTextContent.children.length - 1, 0) * layoutConfig->childGap);
						r32 extraSpace = currentElement->dimensions.Width - (r32)(layoutConfig->padding.left + layoutConfig->padding.right) - contentSize.Width;
						switch (layoutConfig->childAlignment.x)
						{
							case CLAY_ALIGN_X_LEFT: extraSpace = 0; break;
							case CLAY_ALIGN_X_CENTER: extraSpace /= 2; break;
							default: break;
						}
						currentElementTreeNode->nextChildOffset.X += extraSpace;
					}
					else
					{
						for (i32 i = 0; i < currentElement->childrenOrTextContent.children.length; ++i)
						{
							Clay_LayoutElement* childElement = Clay_LayoutElementArray_Get(&context->layoutElements, currentElement->childrenOrTextContent.children.elements[i]);
							contentSize.Width = MaxR32(contentSize.Width, childElement->dimensions.Width);
							contentSize.Height += childElement->dimensions.Height;
						}
						contentSize.Height += (r32)(MaxU16(currentElement->childrenOrTextContent.children.length - 1, 0) * layoutConfig->childGap);
						r32 extraSpace = currentElement->dimensions.Height - (r32)(layoutConfig->padding.top + layoutConfig->padding.bottom) - contentSize.Height;
						switch (layoutConfig->childAlignment.y)
						{
							case CLAY_ALIGN_Y_TOP: extraSpace = 0; break;
							case CLAY_ALIGN_Y_CENTER: extraSpace /= 2; break;
							default: break;
						}
						currentElementTreeNode->nextChildOffset.Y += extraSpace;
					}
					
					if (scrollContainerData)
					{
						scrollContainerData->contentSize = MakeV2(contentSize.Width + (r32)(layoutConfig->padding.left + layoutConfig->padding.right), contentSize.Height + (r32)(layoutConfig->padding.top + layoutConfig->padding.bottom));
					}
				}
			}
			else
			{
				// DFS is returning upwards backwards
				bool closeScrollElement = false;
				Clay_ScrollElementConfig* scrollConfig = Clay__FindElementConfigWithType(currentElement, CLAY__ELEMENT_CONFIG_TYPE_SCROLL).scrollElementConfig;
				if (scrollConfig) {
					closeScrollElement = true;
					for (uxx sIndex = 0; sIndex < context->scrollContainerDatas.length; sIndex++)
					{
						Clay__ScrollContainerDataInternal* mapping = Clay__ScrollContainerDataInternalArray_Get(&context->scrollContainerDatas, sIndex);
						if (mapping->layoutElement == currentElement)
						{
							if (scrollConfig->horizontal) { scrollOffset.X = mapping->scrollPosition.X; }
							if (scrollConfig->vertical) { scrollOffset.Y = mapping->scrollPosition.Y; }
							if (context->externalScrollHandlingEnabled) { scrollOffset = V2_Zero; }
							break;
						}
					}
				}
				
				if (Clay__ElementHasConfig(currentElement, CLAY__ELEMENT_CONFIG_TYPE_BORDER))
				{
					Clay_LayoutElementHashMapItem* currentElementData = Clay__GetHashMapItem(currentElement->id);
					rec currentElementBoundingBox = currentElementData->boundingBox;
					
					// Culling - Don't bother to generate render commands for rectangles entirely outside the screen - this won't stop their children from being rendered if they overflow
					if (!Clay__ElementIsOffscreen(&currentElementBoundingBox))
					{
						Clay_SharedElementConfig* sharedConfig = Clay__ElementHasConfig(currentElement, CLAY__ELEMENT_CONFIG_TYPE_SHARED) ? Clay__FindElementConfigWithType(currentElement, CLAY__ELEMENT_CONFIG_TYPE_SHARED).sharedElementConfig : &Clay_SharedElementConfig_DEFAULT;
						Clay_BorderElementConfig* borderConfig = Clay__FindElementConfigWithType(currentElement, CLAY__ELEMENT_CONFIG_TYPE_BORDER).borderElementConfig;
						Clay_RenderCommand renderCommand = {
								.boundingBox = currentElementBoundingBox,
								.renderData = { .border = {
									.color = borderConfig->color,
									.cornerRadius = sharedConfig->cornerRadius,
									.width = borderConfig->width
								}},
								.userData = sharedConfig->userData,
								.id = Clay__HashNumber(currentElement->id, currentElement->childrenOrTextContent.children.length).id,
								.commandType = CLAY_RENDER_COMMAND_TYPE_BORDER,
						};
						Clay__AddRenderCommand(renderCommand);
						if (borderConfig->width.betweenChildren > 0 && borderConfig->color.a > 0)
						{
							r32 halfGap = layoutConfig->childGap / 2;
							v2 borderOffset = MakeV2((r32)layoutConfig->padding.left - halfGap, (r32)layoutConfig->padding.top - halfGap);
							if (layoutConfig->layoutDirection == CLAY_LEFT_TO_RIGHT)
							{
								for (i32 i = 0; i < currentElement->childrenOrTextContent.children.length; ++i)
								{
									Clay_LayoutElement* childElement = Clay_LayoutElementArray_Get(&context->layoutElements, currentElement->childrenOrTextContent.children.elements[i]);
									if (i > 0)
									{
										Clay__AddRenderCommand(NEW_STRUCT(Clay_RenderCommand) {
											.boundingBox = MakeRec(currentElementBoundingBox.X + borderOffset.X + scrollOffset.X, currentElementBoundingBox.Y + scrollOffset.Y, (r32)borderConfig->width.betweenChildren, currentElement->dimensions.Height),
											.renderData = { .rectangle = {
												.backgroundColor = borderConfig->color,
											} },
											.userData = sharedConfig->userData,
											.id = Clay__HashNumber(currentElement->id, currentElement->childrenOrTextContent.children.length + 1 + i).id,
											.commandType = CLAY_RENDER_COMMAND_TYPE_RECTANGLE,
										});
									}
									borderOffset.X += (childElement->dimensions.Width + (r32)layoutConfig->childGap);
								}
							}
							else
							{
								for (i32 i = 0; i < currentElement->childrenOrTextContent.children.length; ++i)
								{
									Clay_LayoutElement* childElement = Clay_LayoutElementArray_Get(&context->layoutElements, currentElement->childrenOrTextContent.children.elements[i]);
									if (i > 0)
									{
										Clay__AddRenderCommand(NEW_STRUCT(Clay_RenderCommand) {
											.boundingBox = MakeRec(currentElementBoundingBox.X + scrollOffset.X, currentElementBoundingBox.Y + borderOffset.Y + scrollOffset.Y, currentElement->dimensions.Width, (r32)borderConfig->width.betweenChildren),
											.renderData = { .rectangle = {
													.backgroundColor = borderConfig->color,
											} },
											.userData = sharedConfig->userData,
											.id = Clay__HashNumber(currentElement->id, currentElement->childrenOrTextContent.children.length + 1 + i).id,
											.commandType = CLAY_RENDER_COMMAND_TYPE_RECTANGLE,
										});
									}
									borderOffset.Y += (childElement->dimensions.Height + (r32)layoutConfig->childGap);
								}
							}
						}
					}
				}
				// This exists because the scissor needs to end _after_ borders between elements
				if (closeScrollElement)
				{
					Clay__AddRenderCommand(NEW_STRUCT(Clay_RenderCommand) {
						.id = Clay__HashNumber(currentElement->id, rootElement->childrenOrTextContent.children.length + 11).id,
						.commandType = CLAY_RENDER_COMMAND_TYPE_SCISSOR_END,
					});
				}
				
				dfsBuffer.length--;
				continue;
			}
			
			// Add children to the DFS buffer
			if (!Clay__ElementHasConfig(currentElement, CLAY__ELEMENT_CONFIG_TYPE_TEXT))
			{
				dfsBuffer.length += currentElement->childrenOrTextContent.children.length;
				for (i32 i = 0; i < currentElement->childrenOrTextContent.children.length; ++i)
				{
					Clay_LayoutElement* childElement = Clay_LayoutElementArray_Get(&context->layoutElements, currentElement->childrenOrTextContent.children.elements[i]);
					// Alignment along non layout axis
					if (layoutConfig->layoutDirection == CLAY_LEFT_TO_RIGHT)
					{
						currentElementTreeNode->nextChildOffset.Y = currentElement->layoutConfig->padding.top;
						r32 whiteSpaceAroundChild = currentElement->dimensions.Height - (r32)(layoutConfig->padding.top + layoutConfig->padding.bottom) - childElement->dimensions.Height;
						switch (layoutConfig->childAlignment.y)
						{
							case CLAY_ALIGN_Y_TOP: break;
							case CLAY_ALIGN_Y_CENTER: currentElementTreeNode->nextChildOffset.Y += whiteSpaceAroundChild / 2; break;
							case CLAY_ALIGN_Y_BOTTOM: currentElementTreeNode->nextChildOffset.Y += whiteSpaceAroundChild; break;
						}
					}
					else
					{
						currentElementTreeNode->nextChildOffset.X = currentElement->layoutConfig->padding.left;
						r32 whiteSpaceAroundChild = currentElement->dimensions.Width - (r32)(layoutConfig->padding.left + layoutConfig->padding.right) - childElement->dimensions.Width;
						switch (layoutConfig->childAlignment.x)
						{
							case CLAY_ALIGN_X_LEFT: break;
							case CLAY_ALIGN_X_CENTER: currentElementTreeNode->nextChildOffset.X += whiteSpaceAroundChild / 2; break;
							case CLAY_ALIGN_X_RIGHT: currentElementTreeNode->nextChildOffset.X += whiteSpaceAroundChild; break;
						}
					}
					
					v2 childPosition = MakeV2(
						currentElementTreeNode->position.X + currentElementTreeNode->nextChildOffset.X + scrollOffset.X,
						currentElementTreeNode->position.Y + currentElementTreeNode->nextChildOffset.Y + scrollOffset.Y
					);
					
					// DFS buffer elements need to be added in reverse because stack traversal happens backwards
					u32 newNodeIndex = dfsBuffer.length - 1 - i;
					dfsBuffer.items[newNodeIndex] = NEW_STRUCT(Clay__LayoutElementTreeNode) {
						.layoutElement = childElement,
						.position = childPosition,
						.nextChildOffset = { .X = (r32)childElement->layoutConfig->padding.left, .Y = (r32)childElement->layoutConfig->padding.top },
					};
					context->treeNodeVisited.items[newNodeIndex] = false;
					
					// Update parent offsets
					if (layoutConfig->layoutDirection == CLAY_LEFT_TO_RIGHT)
					{
						currentElementTreeNode->nextChildOffset.X += childElement->dimensions.Width + (r32)layoutConfig->childGap;
					}
					else
					{
						currentElementTreeNode->nextChildOffset.Y += childElement->dimensions.Height + (r32)layoutConfig->childGap;
					}
				}
			}
		}
		
		if (root->clipElementId)
		{
			Clay__AddRenderCommand(NEW_STRUCT(Clay_RenderCommand) { .id = Clay__HashNumber(rootElement->id, rootElement->childrenOrTextContent.children.length + 11).id, .commandType = CLAY_RENDER_COMMAND_TYPE_SCISSOR_END });
		}
	}
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
