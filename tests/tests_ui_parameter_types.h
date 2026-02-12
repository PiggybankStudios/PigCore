/*
File:   tests_ui_parameter_types.h
Author: Taylor Robbins
Date:   02\11\2026
*/

#ifndef _TESTS_UI_PARAMETER_TYPES_H
#define _TESTS_UI_PARAMETER_TYPES_H

//TODO: This is a temporary file while we test out things. This solution doesn't work if PigCore is getting compiled separately as a dll unless the dll_main.c #includes this file!

#if BUILD_WITH_PIG_UI
typedef plex TestsUiRendererParams TestsUiRendererParams;
plex TestsUiRendererParams
{
	bool isGradient;
	Color32 gradientColorStart;
	Color32 gradientColorEnd;
	Dir2 gradientDir;
};
#define UiRendererParameters TestsUiRendererParams
typedef plex TestsUiThemerParams TestsUiThemerParams;
plex TestsUiThemerParams
{
	bool isButton;
	bool isBackground;
};
#define UiThemerParameters TestsUiThemerParams
#endif

#endif //  _TESTS_UI_PARAMETER_TYPES_H
