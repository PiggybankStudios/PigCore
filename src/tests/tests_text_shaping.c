/*
File:   tests_text_shaping.c
Author: Taylor Robbins
Date:   08\09\2026
Description: 
	** Holds some test code that exercises the kb_text_shape.h APIs
*/

#if (BUILD_WITH_SOKOL_GFX && BUILD_WITH_SOKOL_APP)

typedef plex TextShapingState TextShapingState;
plex TextShapingState
{
	kbts_shape_context* context;
};
static TextShapingState* textShaping = nullptr;

//Pre-declared in tests_main.c
void InitTextShapingTests()
{
	textShaping = AllocType(TextShapingState, stdHeap);
	NotNull(textShaping);
	ClearPointer(textShaping);
	textShaping->context = kbts_CreateShapeContext(/*AllocatorFunc*/ nullptr, /*AllocatorData*/ (void*)stdHeap);
}

//Pre-declared in tests_main.c
void UpdateTextShapingTests()
{
	NotNull(textShaping);
	
}

//Pre-declared in tests_main.c
void RenderTextShapingTests()
{
	NotNull(textShaping);
	
}

#endif //(BUILD_WITH_SOKOL_GFX && BUILD_WITH_SOKOL_APP)
