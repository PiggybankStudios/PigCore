/*
File:   cross_file_and_sprite_sheet.h
Author: Taylor Robbins
Date:   11\18\2025
*/

#ifndef _CROSS_FILE_AND_SPRITE_SHEET_H
#define _CROSS_FILE_AND_SPRITE_SHEET_H

//NOTE: Intentionally no includes here

#if BUILD_WITH_SOKOL_GFX

#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE SpriteSheet LoadSpriteSheet(Arena* arena, Str8 debugName, Str8 filePath, bool tryLoadMetaFile);
#endif

#if PIG_CORE_IMPLEMENTATION

PEXPI SpriteSheet LoadSpriteSheet(Arena* arena, Str8 debugName, Str8 filePath, bool tryLoadMetaFile)
{
	ScratchBegin1(scratch, arena);
	SpriteSheet result = ZEROED;
	
	Slice imageFileContents = Slice_Empty;
	if (OsReadBinFile(filePath, scratch, &imageFileContents))
	{
		Slice metaFileContents = Slice_Empty;
		if (tryLoadMetaFile)
		{
			Str8 fileExt = GetFileExtPart(filePath, /*includeSubExtensions*/ false, /*includeLeadingPeriod*/ true);
			FilePath filePathWithoutExt = StrSlice(filePath, 0, filePath.length - fileExt.length);
			FilePath metaFilePath = JoinStringsInArena(scratch, filePathWithoutExt, StrLit(".meta"), false);
			OsReadTextFile(metaFilePath, scratch, &metaFileContents); //ignore success/failure
		}
		
		result = InitSpriteSheet(arena, debugName, filePath, imageFileContents, metaFileContents);
	}
	else { result.error = Result_FailedToReadFile; }
	
	ScratchEnd(scratch);
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_SOKOL_GFX

#endif //  _CROSS_FILE_AND_SPRITE_SHEET_H
