/*
File:   clay_debug.c
Author: Taylor Robbins
Date:   03\31\2025
Description: 
	** Holds code that renders the debug panel of Clay (using Clay)
	** Was a piece of original clay.h (zlib LICENSE attached below)
*/

Clay__DebugElementConfigTypeLabelConfig Clay__DebugGetElementConfigTypeLabel(Clay__ElementConfigType type)
{
	switch (type)
	{
		case CLAY__ELEMENT_CONFIG_TYPE_SHARED:   return NEW_STRUCT(Clay__DebugElementConfigTypeLabelConfig) { CLAY_STRING("Shared"),   {.valueU32=0xFFF38630} }; //(243, 134, 48, 255)
		case CLAY__ELEMENT_CONFIG_TYPE_TEXT:     return NEW_STRUCT(Clay__DebugElementConfigTypeLabelConfig) { CLAY_STRING("Text"),     {.valueU32=0xFF69D2E7} }; //(105, 210, 231, 255)
		case CLAY__ELEMENT_CONFIG_TYPE_IMAGE:    return NEW_STRUCT(Clay__DebugElementConfigTypeLabelConfig) { CLAY_STRING("Image"),    {.valueU32=0xFF79BD9A} }; //(121, 189, 154, 255)
		case CLAY__ELEMENT_CONFIG_TYPE_FLOATING: return NEW_STRUCT(Clay__DebugElementConfigTypeLabelConfig) { CLAY_STRING("Floating"), {.valueU32=0xFFFA6900} }; //(250, 105, 0, 255)
		case CLAY__ELEMENT_CONFIG_TYPE_SCROLL:   return NEW_STRUCT(Clay__DebugElementConfigTypeLabelConfig) { CLAY_STRING("Scroll"),   {.valueU32=0xFFF2C45A} }; //(242, 196, 90, 255)
		case CLAY__ELEMENT_CONFIG_TYPE_BORDER:   return NEW_STRUCT(Clay__DebugElementConfigTypeLabelConfig) { CLAY_STRING("Border"),   {.valueU32=0xFF6C5B7B} }; //(108, 91, 123, 255)
		case CLAY__ELEMENT_CONFIG_TYPE_CUSTOM:   return NEW_STRUCT(Clay__DebugElementConfigTypeLabelConfig) { CLAY_STRING("Custom"),   {.valueU32=0xFF0B486B} }; //(11, 72, 107, 255)
		default: break;
	}
	return NEW_STRUCT(Clay__DebugElementConfigTypeLabelConfig) { CLAY_STRING("Error"), {.valueU32=0xFF000000} };
}

// Returns row count
Clay__RenderDebugLayoutData Clay__RenderDebugLayoutElementsList(i32 initialRootsLength, i32 highlightedRowIndex)
{
	Clay_Context* context = Clay_GetCurrentContext();
	i32Array dfsBuffer = context->reusableElementIndexBuffer;
	Clay__DebugView_ScrollViewItemLayoutConfig = NEW_STRUCT(Clay_LayoutConfig) { .sizing = { .height = CLAY_SIZING_FIXED(CLAY__DEBUGVIEW_ROW_HEIGHT) }, .childGap = 6, .childAlignment = { .y = CLAY_ALIGN_Y_CENTER }};
	Clay__RenderDebugLayoutData layoutData = ZEROED;
	
	u32 highlightedElementId = 0;
	
	for (i32 rootIndex = 0; rootIndex < initialRootsLength; ++rootIndex)
	{
		dfsBuffer.length = 0;
		Clay__LayoutElementTreeRoot* root = Clay__LayoutElementTreeRootArray_Get(&context->layoutElementTreeRoots, rootIndex);
		i32Array_Add(&dfsBuffer, (i32)root->layoutElementIndex);
		context->treeNodeVisited.items[0] = false;
		if (rootIndex > 0)
		{
			CLAY({ .id = CLAY_IDI("Clay__DebugView_EmptyRowOuter", rootIndex), .layout = { .sizing = {.width = CLAY_SIZING_GROW(0)}, .padding = {CLAY__DEBUGVIEW_INDENT_WIDTH / 2, 0, 0, 0} } })
			{
				CLAY({ .id = CLAY_IDI("Clay__DebugView_EmptyRow", rootIndex), .layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED((r32)CLAY__DEBUGVIEW_ROW_HEIGHT) }}, .border = { .color = CLAY__DEBUGVIEW_COLOR_3, .width = { .top = 1 } } }) {}
			}
			layoutData.rowCount++;
		}
		while (dfsBuffer.length > 0)
		{
			i32 currentElementIndex = i32Array_GetValue(&dfsBuffer, (int)dfsBuffer.length - 1);
			Clay_LayoutElement* currentElement = Clay_LayoutElementArray_Get(&context->layoutElements, (int)currentElementIndex);
			if (context->treeNodeVisited.items[dfsBuffer.length - 1])
			{
				if (!Clay__ElementHasConfig(currentElement, CLAY__ELEMENT_CONFIG_TYPE_TEXT) && currentElement->childrenOrTextContent.children.length > 0)
				{
					Clay__CloseElement();
					Clay__CloseElement();
					Clay__CloseElement();
				}
				dfsBuffer.length--;
				continue;
			}
			
			if (highlightedRowIndex == layoutData.rowCount)
			{
				if (context->pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME)
				{
					context->debugSelectedElementId = currentElement->id;
				}
				highlightedElementId = currentElement->id;
			}
			
			context->treeNodeVisited.items[dfsBuffer.length - 1] = true;
			Clay_LayoutElementHashMapItem* currentElementData = Clay__GetHashMapItem(currentElement->id);
			bool offscreen = Clay__ElementIsOffscreen(&currentElementData->boundingBox);
			if (context->debugSelectedElementId == currentElement->id)
			{
				layoutData.selectedElementRowIndex = layoutData.rowCount;
			}
			CLAY({ .id = CLAY_IDI("Clay__DebugView_ElementOuter", currentElement->id), .layout = Clay__DebugView_ScrollViewItemLayoutConfig })
			{
				// Collapse icon / button
				if (!(Clay__ElementHasConfig(currentElement, CLAY__ELEMENT_CONFIG_TYPE_TEXT) || currentElement->childrenOrTextContent.children.length == 0))
				{
					CLAY({
						.id = CLAY_IDI("Clay__DebugView_CollapseElement", currentElement->id),
						.layout = { .sizing = {CLAY_SIZING_FIXED(16), CLAY_SIZING_FIXED(16)}, .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER} },
						.cornerRadius = CLAY_CORNER_RADIUS(4),
						.border = { .color = CLAY__DEBUGVIEW_COLOR_3, .width = {1, 1, 1, 1, 0} },
					})
					{
						CLAY_TEXT((currentElementData && currentElementData->debugData->collapsed) ? CLAY_STRING("+") : CLAY_STRING("-"), CLAY_TEXT_CONFIG({ .textColor = CLAY__DEBUGVIEW_COLOR_4, .fontSize = 16 }));
					}
				}
				else
				{ // Square dot for empty containers
					CLAY({ .layout = { .sizing = {CLAY_SIZING_FIXED(16), CLAY_SIZING_FIXED(16)}, .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER } } })
					{
						CLAY({ .layout = { .sizing = {CLAY_SIZING_FIXED(8), CLAY_SIZING_FIXED(8)} }, .backgroundColor = CLAY__DEBUGVIEW_COLOR_3, .cornerRadius = CLAY_CORNER_RADIUS(2) }) {}
					}
				}
				// Collisions and offscreen info
				if (currentElementData)
				{
					if (currentElementData->debugData->collision)
					{
						CLAY({ .layout = { .padding = { 8, 8, 2, 2 }}, .border = { .color = {.valueU32=0xFFB19308}, .width = {1, 1, 1, 1, 0} } }) //(177, 147, 8, 255)
						{
							CLAY_TEXT(CLAY_STRING("Duplicate ID"), CLAY_TEXT_CONFIG({ .textColor = CLAY__DEBUGVIEW_COLOR_3, .fontSize = 16 }));
						}
					}
					if (offscreen)
					{
						CLAY({ .layout = { .padding = { 8, 8, 2, 2 } }, .border = {  .color = CLAY__DEBUGVIEW_COLOR_3, .width = { 1, 1, 1, 1, 0} } })
						{
							CLAY_TEXT(CLAY_STRING("Offscreen"), CLAY_TEXT_CONFIG({ .textColor = CLAY__DEBUGVIEW_COLOR_3, .fontSize = 16 }));
						}
					}
				}
				Str8 idString = context->layoutElementIdStrings.items[currentElementIndex];
				if (idString.length > 0)
				{
					CLAY_TEXT(idString, offscreen ? CLAY_TEXT_CONFIG({ .textColor = CLAY__DEBUGVIEW_COLOR_3, .fontSize = 16 }) : &Clay__DebugView_TextNameConfig);
				}
				for (i32 elementConfigIndex = 0; elementConfigIndex < currentElement->elementConfigs.length; ++elementConfigIndex)
				{
					Clay_ElementConfig* elementConfig = Clay__ElementConfigArray_GetSlice(&currentElement->elementConfigs, elementConfigIndex);
					if (elementConfig->type == CLAY__ELEMENT_CONFIG_TYPE_SHARED)
					{
						Color32 labelColor = {.valueU32=0x5AF38630}; //(243, 134, 48, 90)
						labelColor.a = 90;
						Color32 backgroundColor = elementConfig->config.sharedElementConfig->backgroundColor;
						Clay_CornerRadius radius = elementConfig->config.sharedElementConfig->cornerRadius;
						if (backgroundColor.a > 0)
						{
							CLAY({ .layout = { .padding = { 8, 8, 2, 2 } }, .backgroundColor = labelColor, .cornerRadius = CLAY_CORNER_RADIUS(4), .border = { .color = labelColor, .width = { 1, 1, 1, 1, 0} } })
							{
								CLAY_TEXT(CLAY_STRING("Color"), CLAY_TEXT_CONFIG({ .textColor = offscreen ? CLAY__DEBUGVIEW_COLOR_3 : CLAY__DEBUGVIEW_COLOR_4, .fontSize = 16 }));
							}
						}
						if (radius.bottomLeft > 0)
						{
							CLAY({ .layout = { .padding = { 8, 8, 2, 2 } }, .backgroundColor = labelColor, .cornerRadius = CLAY_CORNER_RADIUS(4), .border = { .color = labelColor, .width = { 1, 1, 1, 1, 0 } } })
							{
								CLAY_TEXT(CLAY_STRING("Radius"), CLAY_TEXT_CONFIG({ .textColor = offscreen ? CLAY__DEBUGVIEW_COLOR_3 : CLAY__DEBUGVIEW_COLOR_4, .fontSize = 16 }));
							}
						}
						continue;
					}
					Clay__DebugElementConfigTypeLabelConfig config = Clay__DebugGetElementConfigTypeLabel(elementConfig->type);
					Color32 backgroundColor = config.color;
					backgroundColor.a = 90;
					CLAY({ .layout = { .padding = { 8, 8, 2, 2 } }, .backgroundColor = backgroundColor, .cornerRadius = CLAY_CORNER_RADIUS(4), .border = { .color = config.color, .width = { 1, 1, 1, 1, 0 } } })
					{
						CLAY_TEXT(config.label, CLAY_TEXT_CONFIG({ .textColor = offscreen ? CLAY__DEBUGVIEW_COLOR_3 : CLAY__DEBUGVIEW_COLOR_4, .fontSize = 16 }));
					}
				}
			}
			
			// Render the text contents below the element as a non-interactive row
			if (Clay__ElementHasConfig(currentElement, CLAY__ELEMENT_CONFIG_TYPE_TEXT))
			{
				layoutData.rowCount++;
				Clay__TextElementData* textElementData = currentElement->childrenOrTextContent.textElementData;
				Clay_TextElementConfig* rawTextConfig = offscreen ? CLAY_TEXT_CONFIG({ .textColor = CLAY__DEBUGVIEW_COLOR_3, .fontSize = 16 }) : &Clay__DebugView_TextNameConfig;
				CLAY({ .layout = { .sizing = { .height = CLAY_SIZING_FIXED(CLAY__DEBUGVIEW_ROW_HEIGHT)}, .childAlignment = { .y = CLAY_ALIGN_Y_CENTER } } })
				{
					CLAY({ .layout = { .sizing = {.width = CLAY_SIZING_FIXED(CLAY__DEBUGVIEW_INDENT_WIDTH + 16) } } }) {}
					CLAY_TEXT(CLAY_STRING("\""), rawTextConfig);
					CLAY_TEXT(textElementData->text.length > 40 ? (NEW_STRUCT(Str8) { .length = 40, .chars = textElementData->text.chars }) : textElementData->text, rawTextConfig);
					if (textElementData->text.length > 40) { CLAY_TEXT(CLAY_STRING("..."), rawTextConfig); }
					CLAY_TEXT(CLAY_STRING("\""), rawTextConfig);
				}
			}
			else if (currentElement->childrenOrTextContent.children.length > 0)
			{
				Clay__OpenElement();
				Clay__ConfigureOpenElement(NEW_STRUCT(Clay_ElementDeclaration) { .layout = { .padding = { .left = 8 } } });
				Clay__OpenElement();
				Clay__ConfigureOpenElement(NEW_STRUCT(Clay_ElementDeclaration) { .layout = { .padding = { .left = CLAY__DEBUGVIEW_INDENT_WIDTH }}, .border = { .color = CLAY__DEBUGVIEW_COLOR_3, .width = { .left = 1 } }});
				Clay__OpenElement();
				Clay__ConfigureOpenElement(NEW_STRUCT(Clay_ElementDeclaration) { .layout = { .layoutDirection = CLAY_TOP_TO_BOTTOM } });
			}
			
			layoutData.rowCount++;
			if (!(Clay__ElementHasConfig(currentElement, CLAY__ELEMENT_CONFIG_TYPE_TEXT) || (currentElementData && currentElementData->debugData->collapsed)))
			{
				for (i32 i = currentElement->childrenOrTextContent.children.length - 1; i >= 0; --i)
				{
					i32Array_Add(&dfsBuffer, currentElement->childrenOrTextContent.children.elements[i]);
					context->treeNodeVisited.items[dfsBuffer.length - 1] = false; // TODO needs to be ranged checked
				}
			}
		}
	}
	
	if (context->pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME)
	{
		Clay_ElementId collapseButtonId = Clay__HashString(CLAY_STRING("Clay__DebugView_CollapseElement"), 0, 0);
		for (i32 i = (int)context->pointerOverIds.length - 1; i >= 0; i--)
		{
			Clay_ElementId* elementId = Clay__ElementIdArray_Get(&context->pointerOverIds, i);
			if (elementId->baseId == collapseButtonId.baseId)
			{
				Clay_LayoutElementHashMapItem* highlightedItem = Clay__GetHashMapItem(elementId->offset);
				highlightedItem->debugData->collapsed = !highlightedItem->debugData->collapsed;
				break;
			}
		}
	}
	
	if (highlightedElementId)
	{
		CLAY({ .id = CLAY_ID("Clay__DebugView_ElementHighlight"), .layout = { .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)} }, .floating = { .parentId = highlightedElementId, .zIndex = 32767, .pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH, .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID } })
		{
			CLAY({ .id = CLAY_ID("Clay__DebugView_ElementHighlightRectangle"), .layout = { .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)} }, .backgroundColor = Clay__debugViewHighlightColor }) {}
		}
	}
	return layoutData;
}

void Clay__RenderDebugLayoutSizing(Clay_SizingAxis sizing, Clay_TextElementConfig* infoTextConfig)
{
	Str8 sizingLabel = CLAY_STRING("GROW");
	if (sizing.type == CLAY__SIZING_TYPE_FIT) { sizingLabel = CLAY_STRING("FIT"); }
	else if (sizing.type == CLAY__SIZING_TYPE_PERCENT) { sizingLabel = CLAY_STRING("PERCENT"); }
	CLAY_TEXT(sizingLabel, infoTextConfig);
	if (sizing.type == CLAY__SIZING_TYPE_GROW || sizing.type == CLAY__SIZING_TYPE_FIT)
	{
		CLAY_TEXT(CLAY_STRING("("), infoTextConfig);
		if (sizing.size.minMax.min != 0)
		{
			CLAY_TEXT(CLAY_STRING("min: "), infoTextConfig);
			CLAY_TEXT(Clay__IntToString(sizing.size.minMax.min), infoTextConfig);
			if (sizing.size.minMax.max != HugeR32) { CLAY_TEXT(CLAY_STRING(", "), infoTextConfig); }
		}
		if (sizing.size.minMax.max != HugeR32)
		{
			CLAY_TEXT(CLAY_STRING("max: "), infoTextConfig);
			CLAY_TEXT(Clay__IntToString(sizing.size.minMax.max), infoTextConfig);
		}
		CLAY_TEXT(CLAY_STRING(")"), infoTextConfig);
	}
}

void Clay__RenderDebugViewElementConfigHeader(Str8 elementId, Clay__ElementConfigType type)
{
	Clay__DebugElementConfigTypeLabelConfig config = Clay__DebugGetElementConfigTypeLabel(type);
	Color32 backgroundColor = config.color;
	backgroundColor.a = 90;
	CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_GROW(0) }, .padding = CLAY_PADDING_ALL(CLAY__DEBUGVIEW_OUTER_PADDING), .childAlignment = { .y = CLAY_ALIGN_Y_CENTER } } })
	{
		CLAY({ .layout = { .padding = { 8, 8, 2, 2 } }, .backgroundColor = backgroundColor, .cornerRadius = CLAY_CORNER_RADIUS(4), .border = { .color = config.color, .width = { 1, 1, 1, 1, 0 } } })
		{
			CLAY_TEXT(config.label, CLAY_TEXT_CONFIG({ .textColor = CLAY__DEBUGVIEW_COLOR_4, .fontSize = 16 }));
		}
		CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_GROW(0) } } }) {}
		CLAY_TEXT(elementId, CLAY_TEXT_CONFIG({ .textColor = CLAY__DEBUGVIEW_COLOR_3, .fontSize = 16, .wrapMode = CLAY_TEXT_WRAP_NONE }));
	}
}

void Clay__RenderDebugViewColor(Color32 color, Clay_TextElementConfig* textConfig)
{
	CLAY({ .layout = { .childAlignment = {.y = CLAY_ALIGN_Y_CENTER} } })
	{
		CLAY_TEXT(CLAY_STRING("{ r: "), textConfig);
		CLAY_TEXT(Clay__IntToString(color.r), textConfig);
		CLAY_TEXT(CLAY_STRING(", g: "), textConfig);
		CLAY_TEXT(Clay__IntToString(color.g), textConfig);
		CLAY_TEXT(CLAY_STRING(", b: "), textConfig);
		CLAY_TEXT(Clay__IntToString(color.b), textConfig);
		CLAY_TEXT(CLAY_STRING(", a: "), textConfig);
		CLAY_TEXT(Clay__IntToString(color.a), textConfig);
		CLAY_TEXT(CLAY_STRING(" }"), textConfig);
		CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_FIXED(10) } } }) {}
		CLAY({ .layout = { .sizing = { CLAY_SIZING_FIXED(CLAY__DEBUGVIEW_ROW_HEIGHT - 8), CLAY_SIZING_FIXED(CLAY__DEBUGVIEW_ROW_HEIGHT - 8)} }, .backgroundColor = color, .cornerRadius = CLAY_CORNER_RADIUS(4), .border = { .color = CLAY__DEBUGVIEW_COLOR_4, .width = { 1, 1, 1, 1, 0 } } }) {}
	}
}

void Clay__RenderDebugViewCornerRadius(Clay_CornerRadius cornerRadius, Clay_TextElementConfig* textConfig)
{
	CLAY({ .layout = { .childAlignment = {.y = CLAY_ALIGN_Y_CENTER} } })
	{
		CLAY_TEXT(CLAY_STRING("{ topLeft: "), textConfig);
		CLAY_TEXT(Clay__IntToString(cornerRadius.topLeft), textConfig);
		CLAY_TEXT(CLAY_STRING(", topRight: "), textConfig);
		CLAY_TEXT(Clay__IntToString(cornerRadius.topRight), textConfig);
		CLAY_TEXT(CLAY_STRING(", bottomLeft: "), textConfig);
		CLAY_TEXT(Clay__IntToString(cornerRadius.bottomLeft), textConfig);
		CLAY_TEXT(CLAY_STRING(", bottomRight: "), textConfig);
		CLAY_TEXT(Clay__IntToString(cornerRadius.bottomRight), textConfig);
		CLAY_TEXT(CLAY_STRING(" }"), textConfig);
	}
}

void HandleDebugViewCloseButtonInteraction(Clay_ElementId elementId, Clay_PointerData pointerInfo, CLAY_ONHOVER_USERDATA_TYPE userData)
{
	Clay_Context* context = Clay_GetCurrentContext();
	(void) elementId; (void) pointerInfo; (void) userData;
	if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) { context->debugModeEnabled = false; }
}

void Clay__RenderDebugView(void)
{
	Clay_Context* context = Clay_GetCurrentContext();
	Clay_ElementId closeButtonId = Clay__HashString(CLAY_STRING("Clay__DebugViewTopHeaderCloseButtonOuter"), 0, 0);
	if (context->pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME)
	{
		for (i32 i = 0; i < context->pointerOverIds.length; ++i)
		{
			Clay_ElementId* elementId = Clay__ElementIdArray_Get(&context->pointerOverIds, i);
			if (elementId->id == closeButtonId.id)
			{
				context->debugModeEnabled = false;
				return;
			}
		}
	}
	
	u32 initialRootsLength = context->layoutElementTreeRoots.length;
	u32 initialElementsLength = context->layoutElements.length;
	Clay_TextElementConfig* infoTextConfig = CLAY_TEXT_CONFIG({ .textColor = CLAY__DEBUGVIEW_COLOR_4, .fontSize = 16, .wrapMode = CLAY_TEXT_WRAP_NONE });
	Clay_TextElementConfig* infoTitleConfig = CLAY_TEXT_CONFIG({ .textColor = CLAY__DEBUGVIEW_COLOR_3, .fontSize = 16, .wrapMode = CLAY_TEXT_WRAP_NONE });
	Clay_ElementId scrollId = Clay__HashString(CLAY_STRING("Clay__DebugViewOuterScrollPane"), 0, 0);
	r32 scrollYOffset = 0;
	bool pointerInDebugView = context->pointerInfo.position.Y < context->layoutDimensions.Height - 300;
	for (i32 i = 0; i < context->scrollContainerDatas.length; ++i)
	{
		Clay__ScrollContainerDataInternal* scrollContainerData = Clay__ScrollContainerDataInternalArray_Get(&context->scrollContainerDatas, i);
		if (scrollContainerData->elementId == scrollId.id)
		{
			if (!context->externalScrollHandlingEnabled)
			{
				scrollYOffset = scrollContainerData->scrollPosition.Y;
			}
			else
			{
				pointerInDebugView = context->pointerInfo.position.Y + scrollContainerData->scrollPosition.Y < context->layoutDimensions.Height - 300;
			}
			break;
		}
	}
	i32 highlightedRow = pointerInDebugView
			? (i32)((context->pointerInfo.position.Y - scrollYOffset) / (r32)CLAY__DEBUGVIEW_ROW_HEIGHT) - 1
			: -1;
	if (context->pointerInfo.position.X < context->layoutDimensions.Width - (r32)Clay__debugViewWidth)
	{
		highlightedRow = -1;
	}
	Clay__RenderDebugLayoutData layoutData = ZEROED;
	CLAY({ .id = CLAY_ID("Clay__DebugView"),
		 .layout = { .sizing = { CLAY_SIZING_FIXED((r32)Clay__debugViewWidth) , CLAY_SIZING_FIXED(context->layoutDimensions.Height) }, .layoutDirection = CLAY_TOP_TO_BOTTOM },
		.floating = { .zIndex = 32765, .attachPoints = { .element = CLAY_ATTACH_POINT_LEFT_CENTER, .parent = CLAY_ATTACH_POINT_RIGHT_CENTER }, .attachTo = CLAY_ATTACH_TO_ROOT },
		.border = { .color = CLAY__DEBUGVIEW_COLOR_3, .width = { .bottom = 1 } }
	})
	{
		CLAY({ .layout = { .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(CLAY__DEBUGVIEW_ROW_HEIGHT)}, .padding = {CLAY__DEBUGVIEW_OUTER_PADDING, CLAY__DEBUGVIEW_OUTER_PADDING, 0, 0 }, .childAlignment = {.y = CLAY_ALIGN_Y_CENTER} }, .backgroundColor = CLAY__DEBUGVIEW_COLOR_2 })
		{
			CLAY_TEXT(CLAY_STRING("Clay Debug Tools"), infoTextConfig);
			CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_GROW(0) } } }) {}
			// Close button
			CLAY({
				.layout = { .sizing = {CLAY_SIZING_FIXED(CLAY__DEBUGVIEW_ROW_HEIGHT - 10), CLAY_SIZING_FIXED(CLAY__DEBUGVIEW_ROW_HEIGHT - 10)}, .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER} },
				.backgroundColor = {.valueU32=0x50D95B43}, //(217, 91, 67, 80)
				.cornerRadius = CLAY_CORNER_RADIUS(4),
				.border = { .color = {.valueU32=0xFFD95B43}, .width = { 1, 1, 1, 1, 0 } }, //(217, 91, 67, 255)
			})
			{
				CLAY_ONHOVER_USERDATA_TYPE zeroUserDataType = ZEROED;
				Clay_OnHover(HandleDebugViewCloseButtonInteraction, zeroUserDataType);
				CLAY_TEXT(CLAY_STRING("x"), CLAY_TEXT_CONFIG({ .textColor = CLAY__DEBUGVIEW_COLOR_4, .fontSize = 16 }));
			}
		}
		CLAY({ .layout = { .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(1)} }, .backgroundColor = CLAY__DEBUGVIEW_COLOR_3 } ) {}
		CLAY({ .id = scrollId, .layout = { .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)} }, .scroll = { .horizontal = true, .vertical = true } })
		{
			CLAY({ .layout = { .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)}, .layoutDirection = CLAY_TOP_TO_BOTTOM }, .backgroundColor = ((initialElementsLength + initialRootsLength) & 1) == 0 ? CLAY__DEBUGVIEW_COLOR_2 : CLAY__DEBUGVIEW_COLOR_1 })
			{
				Clay_ElementId panelContentsId = Clay__HashString(CLAY_STRING("Clay__DebugViewPaneOuter"), 0, 0);
				// Element list
				CLAY({ .id = panelContentsId, .layout = { .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)} }, .floating = { .zIndex = 32766, .pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH, .attachTo = CLAY_ATTACH_TO_PARENT } })
				{
					CLAY({ .layout = { .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)}, .padding = { CLAY__DEBUGVIEW_OUTER_PADDING, CLAY__DEBUGVIEW_OUTER_PADDING, 0, 0 }, .layoutDirection = CLAY_TOP_TO_BOTTOM } })
					{
						layoutData = Clay__RenderDebugLayoutElementsList((i32)initialRootsLength, highlightedRow);
					}
				}
				r32 contentWidth = Clay__GetHashMapItem(panelContentsId.id)->layoutElement->dimensions.Width;
				CLAY({ .layout = { .sizing = {.width = CLAY_SIZING_FIXED(contentWidth) }, .layoutDirection = CLAY_TOP_TO_BOTTOM } }) {}
				for (i32 i = 0; i < layoutData.rowCount; i++)
				{
					Color32 rowColor = (i & 1) == 0 ? CLAY__DEBUGVIEW_COLOR_2 : CLAY__DEBUGVIEW_COLOR_1;
					if (i == layoutData.selectedElementRowIndex)
					{
						rowColor = CLAY__DEBUGVIEW_COLOR_SELECTED_ROW;
					}
					if (i == highlightedRow)
					{
						rowColor.r *= 1.25f;
						rowColor.g *= 1.25f;
						rowColor.b *= 1.25f;
					}
					CLAY({ .layout = { .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(CLAY__DEBUGVIEW_ROW_HEIGHT)}, .layoutDirection = CLAY_TOP_TO_BOTTOM }, .backgroundColor = rowColor } ) {}
				}
			}
		}
		CLAY({ .layout = { .sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(1)} }, .backgroundColor = CLAY__DEBUGVIEW_COLOR_3 }) {}
		if (context->debugSelectedElementId != 0)
		{
			Clay_LayoutElementHashMapItem* selectedItem = Clay__GetHashMapItem(context->debugSelectedElementId);
			CLAY({
				.layout = { .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(300)}, .layoutDirection = CLAY_TOP_TO_BOTTOM },
				.backgroundColor = CLAY__DEBUGVIEW_COLOR_2 ,
				.scroll = { .vertical = true },
				.border = { .color = CLAY__DEBUGVIEW_COLOR_3, .width = { .betweenChildren = 1 } }
			})
			{
				CLAY({ .layout = { .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(CLAY__DEBUGVIEW_ROW_HEIGHT + 8)}, .padding = {CLAY__DEBUGVIEW_OUTER_PADDING, CLAY__DEBUGVIEW_OUTER_PADDING, 0, 0 }, .childAlignment = {.y = CLAY_ALIGN_Y_CENTER} } })
				{
					CLAY_TEXT(CLAY_STRING("Layout Config"), infoTextConfig);
					CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_GROW(0) } } }) {}
					if (selectedItem->elementId.stringId.length != 0)
					{
						CLAY_TEXT(selectedItem->elementId.stringId, infoTitleConfig);
						if (selectedItem->elementId.offset != 0)
						{
							CLAY_TEXT(CLAY_STRING(" ("), infoTitleConfig);
							CLAY_TEXT(Clay__IntToString(selectedItem->elementId.offset), infoTitleConfig);
							CLAY_TEXT(CLAY_STRING(")"), infoTitleConfig);
						}
					}
				}
				Clay_Padding attributeConfigPadding = {CLAY__DEBUGVIEW_OUTER_PADDING, CLAY__DEBUGVIEW_OUTER_PADDING, 8, 8};
				// Clay_LayoutConfig debug info
				CLAY({ .layout = { .padding = attributeConfigPadding, .childGap = 8, .layoutDirection = CLAY_TOP_TO_BOTTOM } })
				{
					// .boundingBox
					CLAY_TEXT(CLAY_STRING("Bounding Box"), infoTitleConfig);
					CLAY({ .layout = { .layoutDirection = CLAY_LEFT_TO_RIGHT } })
					{
						CLAY_TEXT(CLAY_STRING("{ x: "), infoTextConfig);
						CLAY_TEXT(Clay__IntToString(selectedItem->boundingBox.X), infoTextConfig);
						CLAY_TEXT(CLAY_STRING(", y: "), infoTextConfig);
						CLAY_TEXT(Clay__IntToString(selectedItem->boundingBox.Y), infoTextConfig);
						CLAY_TEXT(CLAY_STRING(", width: "), infoTextConfig);
						CLAY_TEXT(Clay__IntToString(selectedItem->boundingBox.Width), infoTextConfig);
						CLAY_TEXT(CLAY_STRING(", height: "), infoTextConfig);
						CLAY_TEXT(Clay__IntToString(selectedItem->boundingBox.Height), infoTextConfig);
						CLAY_TEXT(CLAY_STRING(" }"), infoTextConfig);
					}
					// .layoutDirection
					CLAY_TEXT(CLAY_STRING("Layout Direction"), infoTitleConfig);
					Clay_LayoutConfig* layoutConfig = selectedItem->layoutElement->layoutConfig;
					CLAY_TEXT(layoutConfig->layoutDirection == CLAY_TOP_TO_BOTTOM ? CLAY_STRING("TOP_TO_BOTTOM") : CLAY_STRING("LEFT_TO_RIGHT"), infoTextConfig);
					// .sizing
					CLAY_TEXT(CLAY_STRING("Sizing"), infoTitleConfig);
					CLAY({ .layout = { .layoutDirection = CLAY_LEFT_TO_RIGHT } })
					{
						CLAY_TEXT(CLAY_STRING("width: "), infoTextConfig);
						Clay__RenderDebugLayoutSizing(layoutConfig->sizing.width, infoTextConfig);
					}
					CLAY({ .layout = { .layoutDirection = CLAY_LEFT_TO_RIGHT } })
					{
						CLAY_TEXT(CLAY_STRING("height: "), infoTextConfig);
						Clay__RenderDebugLayoutSizing(layoutConfig->sizing.height, infoTextConfig);
					}
					// .padding
					CLAY_TEXT(CLAY_STRING("Padding"), infoTitleConfig);
					CLAY({ .id = CLAY_ID("Clay__DebugViewElementInfoPadding") })
					{
						CLAY_TEXT(CLAY_STRING("{ left: "), infoTextConfig);
						CLAY_TEXT(Clay__IntToString(layoutConfig->padding.left), infoTextConfig);
						CLAY_TEXT(CLAY_STRING(", right: "), infoTextConfig);
						CLAY_TEXT(Clay__IntToString(layoutConfig->padding.right), infoTextConfig);
						CLAY_TEXT(CLAY_STRING(", top: "), infoTextConfig);
						CLAY_TEXT(Clay__IntToString(layoutConfig->padding.top), infoTextConfig);
						CLAY_TEXT(CLAY_STRING(", bottom: "), infoTextConfig);
						CLAY_TEXT(Clay__IntToString(layoutConfig->padding.bottom), infoTextConfig);
						CLAY_TEXT(CLAY_STRING(" }"), infoTextConfig);
					}
					// .childGap
					CLAY_TEXT(CLAY_STRING("Child Gap"), infoTitleConfig);
					CLAY_TEXT(Clay__IntToString(layoutConfig->childGap), infoTextConfig);
					// .childAlignment
					CLAY_TEXT(CLAY_STRING("Child Alignment"), infoTitleConfig);
					CLAY({ .layout = { .layoutDirection = CLAY_LEFT_TO_RIGHT } })
					{
						CLAY_TEXT(CLAY_STRING("{ x: "), infoTextConfig);
						Str8 alignX = CLAY_STRING("LEFT");
						if (layoutConfig->childAlignment.x == CLAY_ALIGN_X_CENTER) { alignX = CLAY_STRING("CENTER"); }
						else if (layoutConfig->childAlignment.x == CLAY_ALIGN_X_RIGHT) { alignX = CLAY_STRING("RIGHT"); }
						CLAY_TEXT(alignX, infoTextConfig);
						CLAY_TEXT(CLAY_STRING(", y: "), infoTextConfig);
						Str8 alignY = CLAY_STRING("TOP");
						if (layoutConfig->childAlignment.y == CLAY_ALIGN_Y_CENTER) { alignY = CLAY_STRING("CENTER"); }
						else if (layoutConfig->childAlignment.y == CLAY_ALIGN_Y_BOTTOM) { alignY = CLAY_STRING("BOTTOM"); }
						CLAY_TEXT(alignY, infoTextConfig);
						CLAY_TEXT(CLAY_STRING(" }"), infoTextConfig);
					}
				}
				for (i32 elementConfigIndex = 0; elementConfigIndex < selectedItem->layoutElement->elementConfigs.length; ++elementConfigIndex)
				{
					Clay_ElementConfig* elementConfig = Clay__ElementConfigArray_GetSlice(&selectedItem->layoutElement->elementConfigs, elementConfigIndex);
					Clay__RenderDebugViewElementConfigHeader(selectedItem->elementId.stringId, elementConfig->type);
					switch (elementConfig->type)
					{
						case CLAY__ELEMENT_CONFIG_TYPE_SHARED:
						{
							Clay_SharedElementConfig* sharedConfig = elementConfig->config.sharedElementConfig;
							CLAY({ .layout = { .padding = attributeConfigPadding, .childGap = 8, .layoutDirection = CLAY_TOP_TO_BOTTOM }})
							{
								// .backgroundColor
								CLAY_TEXT(CLAY_STRING("Background Color"), infoTitleConfig);
								Clay__RenderDebugViewColor(sharedConfig->backgroundColor, infoTextConfig);
								// .cornerRadius
								CLAY_TEXT(CLAY_STRING("Corner Radius"), infoTitleConfig);
								Clay__RenderDebugViewCornerRadius(sharedConfig->cornerRadius, infoTextConfig);
							}
							break;
						}
						case CLAY__ELEMENT_CONFIG_TYPE_TEXT:
						{
							Clay_TextElementConfig* textConfig = elementConfig->config.textElementConfig;
							CLAY({ .layout = { .padding = attributeConfigPadding, .childGap = 8, .layoutDirection = CLAY_TOP_TO_BOTTOM } })
							{
								// .fontSize
								CLAY_TEXT(CLAY_STRING("Font Size"), infoTitleConfig);
								CLAY_TEXT(Clay__IntToString(textConfig->fontSize), infoTextConfig);
								// .fontId
								CLAY_TEXT(CLAY_STRING("Font ID"), infoTitleConfig);
								CLAY_TEXT(Clay__IntToString(textConfig->fontId), infoTextConfig);
								// .lineHeight
								CLAY_TEXT(CLAY_STRING("Line Height"), infoTitleConfig);
								CLAY_TEXT(textConfig->lineHeight == 0 ? CLAY_STRING("auto") : Clay__IntToString(textConfig->lineHeight), infoTextConfig);
								// .letterSpacing
								CLAY_TEXT(CLAY_STRING("Letter Spacing"), infoTitleConfig);
								CLAY_TEXT(Clay__IntToString(textConfig->letterSpacing), infoTextConfig);
								// .wrapMode
								CLAY_TEXT(CLAY_STRING("Wrap Mode"), infoTitleConfig);
								Str8 wrapMode = CLAY_STRING("WORDS");
								if (textConfig->wrapMode == CLAY_TEXT_WRAP_NONE) { wrapMode = CLAY_STRING("NONE"); }
								else if (textConfig->wrapMode == CLAY_TEXT_WRAP_NEWLINES) { wrapMode = CLAY_STRING("NEWLINES"); }
								CLAY_TEXT(wrapMode, infoTextConfig);
								// .textAlignment
								CLAY_TEXT(CLAY_STRING("Text Alignment"), infoTitleConfig);
								Str8 textAlignment = CLAY_STRING("LEFT");
								if (textConfig->textAlignment == CLAY_TEXT_ALIGN_CENTER) { textAlignment = CLAY_STRING("CENTER"); }
								else if (textConfig->textAlignment == CLAY_TEXT_ALIGN_RIGHT) { textAlignment = CLAY_STRING("RIGHT"); }
								CLAY_TEXT(textAlignment, infoTextConfig);
								// .textColor
								CLAY_TEXT(CLAY_STRING("Text Color"), infoTitleConfig);
								Clay__RenderDebugViewColor(textConfig->textColor, infoTextConfig);
							}
							break;
						}
						case CLAY__ELEMENT_CONFIG_TYPE_IMAGE:
						{
							Clay_ImageElementConfig* imageConfig = elementConfig->config.imageElementConfig;
							CLAY({ .id = CLAY_ID("Clay__DebugViewElementInfoImageBody"), .layout = { .padding = attributeConfigPadding, .childGap = 8, .layoutDirection = CLAY_TOP_TO_BOTTOM } })
							{
								// .sourceDimensions
								CLAY_TEXT(CLAY_STRING("Source Dimensions"), infoTitleConfig);
								CLAY({ .id = CLAY_ID("Clay__DebugViewElementInfoImageDimensions") })
								{
									CLAY_TEXT(CLAY_STRING("{ width: "), infoTextConfig);
									CLAY_TEXT(Clay__IntToString(imageConfig->sourceDimensions.Width), infoTextConfig);
									CLAY_TEXT(CLAY_STRING(", height: "), infoTextConfig);
									CLAY_TEXT(Clay__IntToString(imageConfig->sourceDimensions.Height), infoTextConfig);
									CLAY_TEXT(CLAY_STRING(" }"), infoTextConfig);
								}
								// Image Preview
								CLAY_TEXT(CLAY_STRING("Preview"), infoTitleConfig);
								CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_GROW(0, imageConfig->sourceDimensions.Width) }}, .image = *imageConfig }) {}
							}
							break;
						}
						case CLAY__ELEMENT_CONFIG_TYPE_SCROLL:
						{
							Clay_ScrollElementConfig* scrollConfig = elementConfig->config.scrollElementConfig;
							CLAY({ .layout = { .padding = attributeConfigPadding, .childGap = 8, .layoutDirection = CLAY_TOP_TO_BOTTOM } })
							{
								// .vertical
								CLAY_TEXT(CLAY_STRING("Vertical"), infoTitleConfig);
								CLAY_TEXT(scrollConfig->vertical ? CLAY_STRING("true") : CLAY_STRING("false") , infoTextConfig);
								// .horizontal
								CLAY_TEXT(CLAY_STRING("Horizontal"), infoTitleConfig);
								CLAY_TEXT(scrollConfig->horizontal ? CLAY_STRING("true") : CLAY_STRING("false") , infoTextConfig);
							}
							break;
						}
						case CLAY__ELEMENT_CONFIG_TYPE_FLOATING:
						{
							Clay_FloatingElementConfig* floatingConfig = elementConfig->config.floatingElementConfig;
							CLAY({ .layout = { .padding = attributeConfigPadding, .childGap = 8, .layoutDirection = CLAY_TOP_TO_BOTTOM } })
							{
								// .offset
								CLAY_TEXT(CLAY_STRING("Offset"), infoTitleConfig);
								CLAY({ .layout = { .layoutDirection = CLAY_LEFT_TO_RIGHT } })
								{
									CLAY_TEXT(CLAY_STRING("{ x: "), infoTextConfig);
									CLAY_TEXT(Clay__IntToString(floatingConfig->offset.X), infoTextConfig);
									CLAY_TEXT(CLAY_STRING(", y: "), infoTextConfig);
									CLAY_TEXT(Clay__IntToString(floatingConfig->offset.Y), infoTextConfig);
									CLAY_TEXT(CLAY_STRING(" }"), infoTextConfig);
								}
								// .expand
								CLAY_TEXT(CLAY_STRING("Expand"), infoTitleConfig);
								CLAY({ .layout = { .layoutDirection = CLAY_LEFT_TO_RIGHT } })
								{
									CLAY_TEXT(CLAY_STRING("{ width: "), infoTextConfig);
									CLAY_TEXT(Clay__IntToString(floatingConfig->expand.Width), infoTextConfig);
									CLAY_TEXT(CLAY_STRING(", height: "), infoTextConfig);
									CLAY_TEXT(Clay__IntToString(floatingConfig->expand.Height), infoTextConfig);
									CLAY_TEXT(CLAY_STRING(" }"), infoTextConfig);
								}
								// .zIndex
								CLAY_TEXT(CLAY_STRING("z-index"), infoTitleConfig);
								CLAY_TEXT(Clay__IntToString(floatingConfig->zIndex), infoTextConfig);
								// .parentId
								CLAY_TEXT(CLAY_STRING("Parent"), infoTitleConfig);
								Clay_LayoutElementHashMapItem* hashItem = Clay__GetHashMapItem(floatingConfig->parentId);
								CLAY_TEXT(hashItem->elementId.stringId, infoTextConfig);
							}
							break;
						}
						case CLAY__ELEMENT_CONFIG_TYPE_BORDER:
						{
							Clay_BorderElementConfig* borderConfig = elementConfig->config.borderElementConfig;
							CLAY({ .id = CLAY_ID("Clay__DebugViewElementInfoBorderBody"), .layout = { .padding = attributeConfigPadding, .childGap = 8, .layoutDirection = CLAY_TOP_TO_BOTTOM } })
							{
								CLAY_TEXT(CLAY_STRING("Border Widths"), infoTitleConfig);
								CLAY({ .layout = { .layoutDirection = CLAY_LEFT_TO_RIGHT } })
								{
									CLAY_TEXT(CLAY_STRING("{ left: "), infoTextConfig);
									CLAY_TEXT(Clay__IntToString(borderConfig->width.left), infoTextConfig);
									CLAY_TEXT(CLAY_STRING(", right: "), infoTextConfig);
									CLAY_TEXT(Clay__IntToString(borderConfig->width.right), infoTextConfig);
									CLAY_TEXT(CLAY_STRING(", top: "), infoTextConfig);
									CLAY_TEXT(Clay__IntToString(borderConfig->width.top), infoTextConfig);
									CLAY_TEXT(CLAY_STRING(", bottom: "), infoTextConfig);
									CLAY_TEXT(Clay__IntToString(borderConfig->width.bottom), infoTextConfig);
									CLAY_TEXT(CLAY_STRING(" }"), infoTextConfig);
								}
								// .textColor
								CLAY_TEXT(CLAY_STRING("Border Color"), infoTitleConfig);
								Clay__RenderDebugViewColor(borderConfig->color, infoTextConfig);
							}
							break;
						}
						case CLAY__ELEMENT_CONFIG_TYPE_CUSTOM:
						default: break;
					}
				}
			}
		}
		else
		{
			CLAY({ .id = CLAY_ID("Clay__DebugViewWarningsScrollPane"), .layout = { .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(300)}, .childGap = 6, .layoutDirection = CLAY_TOP_TO_BOTTOM }, .backgroundColor = CLAY__DEBUGVIEW_COLOR_2, .scroll = { .horizontal = true, .vertical = true } })
			{
				Clay_TextElementConfig* warningConfig = CLAY_TEXT_CONFIG({ .textColor = CLAY__DEBUGVIEW_COLOR_4, .fontSize = 16, .wrapMode = CLAY_TEXT_WRAP_NONE });
				CLAY({ .id = CLAY_ID("Clay__DebugViewWarningItemHeader"), .layout = { .sizing = {.height = CLAY_SIZING_FIXED(CLAY__DEBUGVIEW_ROW_HEIGHT)}, .padding = {CLAY__DEBUGVIEW_OUTER_PADDING, CLAY__DEBUGVIEW_OUTER_PADDING, 0, 0 }, .childGap = 8, .childAlignment = {.y = CLAY_ALIGN_Y_CENTER} } })
				{
					CLAY_TEXT(CLAY_STRING("Warnings"), warningConfig);
				}
				CLAY({ .id = CLAY_ID("Clay__DebugViewWarningsTopBorder"), .layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(1)} }, .backgroundColor = {.valueU32=0xFFC8C8C8} }) {} //(200, 200, 200, 255)
				i32 previousWarningsLength = context->warnings.length;
				for (i32 i = 0; i < previousWarningsLength; i++)
				{
					Clay__Warning warning = context->warnings.items[i];
					CLAY({ .id = CLAY_IDI("Clay__DebugViewWarningItem", i), .layout = { .sizing = {.height = CLAY_SIZING_FIXED(CLAY__DEBUGVIEW_ROW_HEIGHT)}, .padding = {CLAY__DEBUGVIEW_OUTER_PADDING, CLAY__DEBUGVIEW_OUTER_PADDING, 0, 0 }, .childGap = 8, .childAlignment = {.y = CLAY_ALIGN_Y_CENTER} } })
					{
						CLAY_TEXT(warning.baseMessage, warningConfig);
						if (warning.dynamicMessage.length > 0)
						{
							CLAY_TEXT(warning.dynamicMessage, warningConfig);
						}
					}
				}
			}
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
