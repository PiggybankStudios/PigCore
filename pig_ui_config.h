/*
File:   pig_ui_config.h
Author: Taylor Robbins
Date:   06\27\2026
*/

#ifndef _PIG_UI_CONFIG_H
#define _PIG_UI_CONFIG_H

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

#endif //  _PIG_UI_CONFIG_H
