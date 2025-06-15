/*
File:   tests_playdate.c
Author: Taylor Robbins
Date:   06\15\2025
Description: 
	** Holds the PlaydateUpdateCallback which does some simple tests of the
	** Playdate API to make sure things are working
*/

#if TARGET_IS_PLAYDATE

int PlaydateUpdateCallback(void* userData)
{
	pd->graphics->clear(kColorBlack);
	u64 timestamp = OsGetCurrentTimestamp(false);
	char printBuffer[64];
	int printResult = MyBufferPrintf(printBuffer, ArrayCount(printBuffer), "%llu", timestamp);
	pd->graphics->setDrawMode(kDrawModeFillWhite);
	pd->graphics->drawText(printBuffer, printResult, kUTF8Encoding, 10, 10);
	return 1;
}

#endif //TARGET_IS_PLAYDATE
