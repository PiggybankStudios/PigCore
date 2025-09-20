/*
File:   os_font.h
Author: Taylor Robbins
Date:   02\05\2025
Description:
	** Contains functions that help us ask the operating system for font files like .ttf
*/

#ifndef _OS_FONT_H
#define _OS_FONT_H

#include "base/base_compiler_check.h"
#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_assert.h"
#include "std/std_memset.h"
#include "misc/misc_result.h"
#include "mem/mem_arena.h"
#include "mem/mem_scratch.h"
#include "struct/struct_string.h"
#if TARGET_IS_LINUX
#include "os/os_file.h"
#endif

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	Result OsReadPlatformFont(Arena* arena, Str8 fontName, i32 fontSize, bool bold, bool italic, Slice* fileContentsOut);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

#if TARGET_IS_LINUX
static FcConfig* fontConfig = nullptr;
#endif

PEXP Result OsReadPlatformFont(Arena* arena, Str8 fontName, i32 fontSize, bool bold, bool italic, Slice* fileContentsOut)
{
	NotNull(arena);
	NotNull(fileContentsOut);
	NotNullStr(fontName);
	Result result = Result_None;
	#if TARGET_IS_WINDOWS
	{
		ScratchBegin1(scratch, arena);
		NotNullStr(fontName);
		Str8 fontNameNt = AllocStrAndCopy(scratch, fontName.length, fontName.chars, true);
		
		//TODO: Call EnumFontFamiliesEx first to determine which font name exactly we will be getting
		
		//NOTE: This comes from wingdi.h (included by Windows.h) and resides in Gdi32.dll
		HFONT fontHandle = CreateFontA(
			fontSize,                   //height
			0,                          //width
			0,                          //escapement
			0,                          //orientation
			bold ? FW_BOLD : FW_NORMAL, //weight
			italic,                     //italic
			false,                      //underline TODO: Should we ask for this?
			false,                      //strikeout TODO: Should we ask for this?
			ANSI_CHARSET,               //charset
			OUT_TT_ONLY_PRECIS,         //output precision (only give us ttf type fonts)
			CLIP_DEFAULT_PRECIS,        //clipping precision
			DEFAULT_QUALITY,            //quality
			DEFAULT_PITCH|FF_DONTCARE,  //pitch and family
			fontNameNt.chars            //Face Name
		);
		if (fontHandle == NULL)
		{
			// PrintLine_E("Failed to find font with name \"%s\"", fontNameNt.chars);
			ScratchEnd(scratch);
			return Result_NotFound;
		}
		
		HWND windowHandle = GetForegroundWindow();
		if (windowHandle == NULL)
		{
			BOOL deleteResult = DeleteObject(fontHandle);
			Assert(deleteResult != 0);
			ScratchEnd(scratch);
			return Result_NoForegroundWindow;
		}
		
		HDC winDeviceContext = GetDC(windowHandle);
		if (winDeviceContext == NULL)
		{
			BOOL deleteResult = DeleteObject(fontHandle);
			Assert(deleteResult != 0);
			ScratchEnd(scratch);
			return Result_NoDeviceContext;
		}
		
		HGDIOBJ selectResult = SelectObject(winDeviceContext, fontHandle);
		if (selectResult == NULL)
		{
			// PrintLine_E("Failed to select platform font after creation! \"%s\"", fontNameNt.pntr);
			BOOL deleteResult = DeleteObject(fontHandle);
			Assert(deleteResult != 0);
			ScratchEnd(scratch);
			return Result_Failure; //TODO: Should we give this failure a better name?
		}
		
		DWORD getFontDataResult = GetFontData(
			winDeviceContext, //hardware device context
			0,                //table
			0,                //offset
			nullptr,          //buffer
			0                 //buffer size
		);
		if (getFontDataResult == GDI_ERROR)
		{
			// PrintLine_E("Failed to get/measure platform font contents for font \"%s\"", fontNameNt.pntr);
			BOOL deleteResult = DeleteObject(fontHandle);
			Assert(deleteResult != 0);
			ScratchEnd(scratch);
			return Result_FailedToReadFile;
		}
		if (getFontDataResult <= 0)
		{
			// PrintLine_E("There was no file contents for font \"%s\"", fontNameNt.pntr);
			BOOL deleteResult = DeleteObject(fontHandle);
			Assert(deleteResult != 0);
			ScratchEnd(scratch);
			return Result_EmptyFile;
		}
		
		fileContentsOut->length = (uxx)getFontDataResult;
		fileContentsOut->bytes = (u8*)AllocMem(arena, fileContentsOut->length);
		if (fileContentsOut->bytes == nullptr)
		{
			// PrintLine_E("Failed to allocate space to store font file data for font \"%s\": %llu bytes", fontNameNt.pntr, fontDataSize);
			BOOL deleteResult = DeleteObject(fontHandle);
			Assert(deleteResult != 0);
			ScratchEnd(scratch);
			return Result_FailedToAllocateMemory;
		}
		
		getFontDataResult = GetFontData(
			winDeviceContext,              //hardware device context
			0,                             //table
			0,                             //offset
			fileContentsOut->bytes,        //buffer
			(DWORD)fileContentsOut->length //buffer size
		);
		Assert(getFontDataResult == (DWORD)fileContentsOut->length);
		
		BOOL deleteResult = DeleteObject(fontHandle);
		Assert(deleteResult != 0);
		
		result = Result_Success;
		ScratchEnd(scratch);
	}
	#elif TARGET_IS_LINUX
	{
		UNUSED(fontSize);
		ScratchBegin1(scratch, arena);
		
		if (fontConfig == nullptr)
		{
			fontConfig = FcInitLoadConfigAndFonts();
			NotNull(fontConfig);
		}
		
		NotNullStr(fontName);
		Str8 fontSearchStr = PrintInArenaStr(scratch, "%.*s%s%s", StrPrint(fontName), bold ? ":bold" : "", italic ? ":italic" : "");
		FcPattern* searchPattern = FcNameParse((FcChar8*)fontSearchStr.chars);
		NotNull(searchPattern);
		
		FcConfigSubstitute(fontConfig, searchPattern, FcMatchPattern);
		FcDefaultSubstitute(searchPattern);
		
		FcFontSet* fontSet = FcFontSetCreate();
		FcObjectSet* objectSet = FcObjectSetBuild(FC_FAMILY, FC_STYLE, FC_FILE, nullptr);
		
		FcResult fcResult;
		FcFontSet* fontPatterns = FcFontSort(fontConfig, searchPattern, FcTrue, 0, &fcResult);
		NotNull(fontPatterns);
		if (fontPatterns->nfont <= 0)
		{
			//TODO: Do we need to cleanup fontSet, objectSet?
			FcFontSetSortDestroy(fontPatterns);
			FcPatternDestroy(searchPattern);
			FcObjectSetDestroy(objectSet);
			FcFontSetDestroy(fontSet);
			ScratchEnd(scratch);
			return Result_NotFound;
		}
		
		FcPattern* fontPattern = FcFontRenderPrepare(fontConfig, searchPattern, fontPatterns->fonts[0]);
		NotNull(fontPattern);
		FcFontSetAdd(fontSet, fontPattern);
		Assert(fontSet->nfont > 0);
		
		FcFontSetSortDestroy(fontPatterns);
		FcPatternDestroy(searchPattern);
		
		FcPattern* font = FcPatternFilter(fontSet->fonts[0], objectSet);
		NotNull(font);
		FcValue fontValue;
		FcPatternGet(font, FC_FILE, 0, &fontValue);
		NotNull(fontValue.u.f);
		FilePath fontFilePath = AllocStrAndCopyNt(scratch, fontValue.u.f, false);
		
		FcPatternDestroy(font);
		FcFontSetDestroy(fontSet);
		FcObjectSetDestroy(objectSet);
		
		Slice fontFileContents = Slice_Empty;
		bool readSuccess = OsReadBinFile(fontFilePath, arena, &fontFileContents);
		ScratchEnd(scratch);
		if (!readSuccess) { return Result_FailedToReadFile; }
		else if (IsEmptyStr(fontFileContents)) { return Result_EmptyFile; }
		
		*fileContentsOut = fontFileContents;
		result = Result_Success;
	}
	#elif TARGET_IS_ANDROID
	{
		UNUSED(fontSize);
		UNUSED(bold);
		UNUSED(italic);
		ScratchBegin1(scratch, arena);
		FilePath fontPath = JoinStringsInArena3(scratch, FilePathLit("/system/fonts/"), fontName, StrLit(".ttf"), true);
		if (OsDoesFileExist(fontPath))
		{
			if (OsReadBinFile(fontPath, arena, fileContentsOut))
			{
				result = Result_Success;
			}
			else { result = Result_FailedToReadFile; }
		}
		else
		{
			PrintLine_W("Couldn't find \"%.*s\"", StrPrint(fontPath));
			result = Result_NotFound;
		}
		ScratchEnd(scratch);
	}
	#else
	AssertMsg(false, "OsReadPlatformFont does not support the current platform yet!");
	result = Result_NotImplemented;
	#endif
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _OS_FONT_H

#if defined(_OS_FONT_H) && defined(_GFX_FONT_H)
#include "cross/cross_os_font_and_gfx_font.h"
#endif
