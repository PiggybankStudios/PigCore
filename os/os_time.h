/*
File:   os_time.h
Author: Taylor Robbins
Date:   01\21\2025
Description:
	** Holds functions that help deal with timing values, like timestamp integers
	** or elapsed time between frames, or year\month\day\hour\min\sec format
*/

#ifndef _OS_TIME_H
#define _OS_TIME_H

#include "base/base_compiler_check.h"
#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"

typedef plex OsTime OsTime;
plex OsTime
{
	u64 msSinceStart;
	r32 msSinceStartRemainder;
	
	#if TARGET_IS_WINDOWS
	LARGE_INTEGER largeInteger;
	#elif TARGET_IS_LINUX
	struct timespec timeValue;
	#endif
};
#if TARGET_IS_WINDOWS
#define OsTime_Zero_Const { .msSinceStart=0, .msSinceStartRemainder=0.0f, .largeInteger={ .QuadPart=0 } }
#else
#define OsTime_Zero_Const { .msSinceStart=0, .msSinceStartRemainder=0.0f }
#endif
#define OsTime_Zero NEW_STRUCT(OsTime)OsTime_Zero_Const

#define NUM_US_PER_MS         1000
#define NUM_MS_PER_SECOND     1000
#define NUM_SEC_PER_MINUTE    60
#define NUM_MIN_PER_HOUR      60
#define NUM_HOUR_PER_DAY      24
#define NUM_DAYS_PER_WEEK     7
#define NUM_DAYS_PER_YEAR     365
#define NUM_DAYS_PER_4YEARS   (365*4 + 1)
#define UNIX_EPOCH_DATE       1970
#define MAX_DAYS_IN_MONTH     31

#define NUM_US_PER_SECOND ((u64)NUM_MS_PER_SECOND  * (u64)NUM_US_PER_MS)       //     1,000,000 (i32 required)
#define NUM_US_PER_MINUTE ((u64)NUM_SEC_PER_MINUTE * (u64)NUM_US_PER_SECOND)   //    60,000,000 (i32 required)
#define NUM_US_PER_HOUR   ((u64)NUM_MIN_PER_HOUR   * (u64)NUM_US_PER_MINUTE)   // 3,600,000,000 (u32 required)
#define NUM_US_PER_DAY    ((u64)NUM_HOUR_PER_DAY   * (u64)NUM_US_PER_HOUR)     //86,400,000,000 (i64 required)

#define NUM_MS_PER_MINUTE ((u64)NUM_SEC_PER_MINUTE * (u64)NUM_MS_PER_SECOND)   //        60,000 (u16 required)
#define NUM_MS_PER_HOUR   ((u64)NUM_MIN_PER_HOUR   * (u64)NUM_MS_PER_MINUTE)   //     3,600,000 (i32 required)
#define NUM_MS_PER_DAY    ((u64)NUM_HOUR_PER_DAY   * (u64)NUM_MS_PER_HOUR)     //    86,400,000 (i32 required)
#define NUM_MS_PER_WEEK   ((u64)NUM_DAYS_PER_WEEK  * (u64)NUM_MS_PER_DAY)      //   604,800,000 (i32 required)
#define NUM_MS_PER_YEAR   ((u64)NUM_DAYS_PER_YEAR  * (u64)NUM_MS_PER_DAY)      //31,536,000,000 (i64 required)

#define NUM_SEC_PER_HOUR   ((u64)NUM_MIN_PER_HOUR  * (u64)NUM_SEC_PER_MINUTE)  //     3,600 (i16 required)
#define NUM_SEC_PER_DAY    ((u64)NUM_HOUR_PER_DAY  * (u64)NUM_SEC_PER_HOUR)    //    86,400 (i32 required)
#define NUM_SEC_PER_WEEK   ((u64)NUM_DAYS_PER_WEEK * (u64)NUM_SEC_PER_DAY)     //   604,800 (i32 required)
#define NUM_SEC_PER_YEAR   ((u64)NUM_DAYS_PER_YEAR * (u64)NUM_SEC_PER_DAY)     //31,536,000 (i32 required)

#define NUM_MIN_PER_DAY     ((u64)NUM_HOUR_PER_DAY  * (u64)NUM_MIN_PER_HOUR)   //  1,440 (i16 required)
#define NUM_MIN_PER_WEEK    ((u64)NUM_DAYS_PER_WEEK * (u64)NUM_MIN_PER_DAY)    // 10,080 (i16 required)
#define NUM_MIN_PER_YEAR    ((u64)NUM_DAYS_PER_YEAR * (u64)NUM_MIN_PER_DAY)    //525,600 (i32 required)

//There are 30 years between 1970 and 2000, that's 7*4 + 2 which allows us to easily account for leap years during that time
#define SECS_BETWEEN_1970_AND_2000 (((u64)NUM_DAYS_PER_4YEARS * NUM_SEC_PER_DAY * 7) + ((u64)NUM_DAYS_PER_YEAR * NUM_SEC_PER_DAY * 2))

#define WIN32_FILETIME_SEC_OFFSET           11644473600ULL //11,644,473,600 seconds between Jan 1st 1601 and Jan 1st 1970

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	u64 OsGetCurrentTimestampEx(bool offsetToLocal, i64* timezoneOffsetOut, bool* timezoneDoesDstOut); //, Arena* timezoneNameArena, Str8* timezoneNameOut);
	PIG_CORE_INLINE u64 OsGetCurrentTimestamp(bool offsetToLocal);
	PIG_CORE_INLINE u64 OsTimeDiffMsU64(OsTime start, OsTime end, r32* remainderOut);
	PIG_CORE_INLINE r32 OsTimeDiffMsR32(OsTime start, OsTime end);
	PIG_CORE_INLINE OsTime OsGetTime();
	PIG_CORE_INLINE void OsMarkStartTime();
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

static bool OsProgramStartTimeMarked = false;
static OsTime OsProgramStartTime = OsTime_Zero_Const;

#if TARGET_IS_WINDOWS
static bool GotQpcFrequency = false;
static LARGE_INTEGER QpcFrequency;
static void GetQpcFrequencyIfNeeded()
{
	if (!GotQpcFrequency)
	{
		QueryPerformanceFrequency(&QpcFrequency);
		GotQpcFrequency = true;
	}
}
#endif

//TODO: Re-add the following output but in a cross file or some kind since it relies on ConvertUcs2StrToUtf8: Arena* timezoneNameArena, Str8* timezoneNameOut
PEXP u64 OsGetCurrentTimestampEx(bool offsetToLocal, i64* timezoneOffsetOut, bool* timezoneDoesDstOut) 
{
	// if (timezoneNameOut != nullptr) { NotNull(timezoneNameArena); }
	u64 result = 0;
	
	#if TARGET_IS_WINDOWS
	{
		if (offsetToLocal)
		{
			u64 unixTimestamp = OsGetCurrentTimestampEx(false, nullptr, nullptr);
			TIME_ZONE_INFORMATION timezoneInfo = ZEROED;
			DWORD timezoneResult = GetTimeZoneInformation(&timezoneInfo);
			DebugAssertMsg(timezoneResult != TIME_ZONE_ID_INVALID, "GetTimeZoneInformation failed and gave TIME_ZONE_ID_INVALID");
			DEBUG_USED(timezoneResult);
			i64 localTimezoneOffset = -((i64)timezoneInfo.Bias * NUM_SEC_PER_MINUTE);
			SetOptionalOutPntr(timezoneOffsetOut, localTimezoneOffset);
			result = unixTimestamp + localTimezoneOffset;
			bool localTimezoneDoesDst = (timezoneInfo.DaylightBias != 0); //TODO: It's possible that DaylightBias isn't -60 minutes. Should we handle that?
			SetOptionalOutPntr(timezoneDoesDstOut, localTimezoneDoesDst);
			// if (timezoneNameOut != nullptr)
			// {
			// 	*timezoneNameOut = ConvertUcs2StrToUtf8(timezoneNameArena, MakeStr16Nt(&timezoneInfo.StandardName[0]), false);
			// }
		}
		else
		{
			//TODO: We could use the higher accuracy from this file time to determine sub-second timestamp values
			FILETIME systemFileTime = ZEROED;
			GetSystemTimeAsFileTime(&systemFileTime);
			ULARGE_INTEGER systemLargeIntegerTime = ZEROED;
			systemLargeIntegerTime.HighPart = systemFileTime.dwHighDateTime;
			systemLargeIntegerTime.LowPart = systemFileTime.dwLowDateTime;
			//NOTE: FILETIME value is number of 100-nanosecond intervals since Jan 1st 1601 UTC
			//      We want number of seconds since Jan 1st 1970 UTC so divide by 10,000,000 and subtract off 369 years
			result = (u64)(systemLargeIntegerTime.QuadPart/10000000ULL);
			if (result >= WIN32_FILETIME_SEC_OFFSET) { result -= WIN32_FILETIME_SEC_OFFSET; }
		}
	}
	#elif (TARGET_IS_LINUX || TARGET_IS_OSX || TARGET_IS_ANDROID)
	{
		if (offsetToLocal)
		{
			time_t utcTime = time(NULL);
			plex tm localTime = ZEROED;
			plex tm* localTimeResult = localtime_r(&utcTime, &localTime); //TODO: This might be a POSIX-only thing? Maybe we should use localtime() instead?
			Assert(localTimeResult == &localTime);
			long timezoneOffset = localTime.tm_gmtoff;
			SetOptionalOutPntr(timezoneOffsetOut, (i64)timezoneOffset);
			// PrintLine_D("Timezone: \"%s\"", localTime.tm_zone);
			SetOptionalOutPntr(timezoneDoesDstOut, localTime.tm_isdst); //TODO: This is not right, it's just saying whether DST currently is in effect, not if the timezone will do DST!
			result = (u64)((i64)utcTime + timezoneOffset);
		}
		else
		{
			plex timeval currentTimeVal = ZEROED;
			int getTimeResult = gettimeofday(&currentTimeVal, NULL);
			UNUSED(getTimeResult); //TODO: Should we check this result?
			result = (u64)currentTimeVal.tv_sec;
			//TODO: tv_usec is also available from currentTimeVal
		}
	}
	#elif TARGET_IS_ORCA
	{
		if (offsetToLocal)
		{
			//TODO: Implement me!
			AssertMsg(false, "Orca does not currently have a way to get local time!");
		}
		else
		{
			r64 clockTime = oc_clock_time(OC_CLOCK_DATE);
			result = (u64)RoundR64i(AbsR64(clockTime));
		}
	}
	#elif TARGET_IS_PLAYDATE
	{
		if (offsetToLocal)
		{
			i32 timezoneOffset = pd->system->getTimezoneOffset();
			u64 unixTimestamp = OsGetCurrentTimestampEx(false, nullptr, nullptr);
			SetOptionalOutPntr(timezoneOffsetOut, timezoneOffset);
			SetOptionalOutPntr(timezoneDoesDstOut, true);
			return unixTimestamp + timezoneOffset;
		}
		else
		{
			unsigned int milliseconds = 0;
			unsigned int secondsSince2000 = pd->system->getSecondsSinceEpoch(&milliseconds);
			return (u64)secondsSince2000 + SECS_BETWEEN_1970_AND_2000;
		}
	}
	#else
	AssertMsg(false, "OsGetCurrentTimestampEx does not support the current platform yet!")
	#endif
	
	return result;
}
PEXPI u64 OsGetCurrentTimestamp(bool offsetToLocal) { return OsGetCurrentTimestampEx(offsetToLocal, nullptr, nullptr); }

PEXPI u64 OsTimeDiffMsU64(OsTime start, OsTime end, r32* remainderOut)
{
	u64 result = 0;
	
	#if TARGET_IS_WINDOWS
	{
		if (end.largeInteger.QuadPart >= start.largeInteger.QuadPart)
		{
			i64 value = (end.largeInteger.QuadPart - start.largeInteger.QuadPart);
			i64 numer = Billion(1);
			GetQpcFrequencyIfNeeded();
			i64 denom = QpcFrequency.QuadPart;
			i64 quantity = value / denom;
			i64 remainder = value % denom;
			u64 now = (quantity * numer) + (remainder * numer / denom);
			result = (u64)FloorR64i((r64)now / 1000000.0);
			SetOptionalOutPntr(remainderOut, (r32)FractionalPartR64((r64)now / 1000000.0));
		}
	}
	#elif TARGET_IS_LINUX
	{
		if (end.timeValue.tv_sec > start.timeValue.tv_sec ||
			(end.timeValue.tv_sec == start.timeValue.tv_sec && end.timeValue.tv_nsec >= start.timeValue.tv_nsec))
		{
			result = (end.timeValue.tv_sec - start.timeValue.tv_sec)*NUM_MS_PER_SECOND;
			if (end.timeValue.tv_nsec >= start.timeValue.tv_nsec) { result += (end.timeValue.tv_nsec - start.timeValue.tv_nsec)/Million(1); }
		}
	}
	// #elif TARGET_IS_OSX
	//TODO: Implement me!
	// #elif TARGET_IS_ANDROID
	//TODO: Implement me!
	#else
	AssertMsg(false, "OsTimeDiffMsU64 does not support the current platform yet!")
	#endif
	
	return result;
}
PEXPI r32 OsTimeDiffMsR32(OsTime start, OsTime end)
{
	r32 remainder = 0.0f;
	u64 result = OsTimeDiffMsU64(start, end, &remainder);
	return (r32)result + remainder;
}

PEXPI OsTime OsGetTime()
{
	OsTime result = OsTime_Zero;
	
	#if TARGET_IS_WINDOWS
	{
		QueryPerformanceCounter(&result.largeInteger);
	}
	#elif TARGET_IS_LINUX
	{
		
		clock_gettime(CLOCK_MONOTONIC, &result.timeValue);
	}
	// #elif TARGET_IS_OSX
	// //TODO: Implement me!
	// #elif TARGET_IS_ANDROID
	// //TODO: Implement me!
	// #elif TARGET_IS_WASM
	// //TODO: Implement me!
	// #elif TARGET_IS_PLAYDATE
	// //TODO: Implement me!
	#else
	AssertMsg(false, "OsGetTime does not support the current platform yet!")
	#endif
	
	if (OsProgramStartTimeMarked)
	{
		result.msSinceStart = OsTimeDiffMsU64(OsProgramStartTime, result, &result.msSinceStartRemainder);
	}
	return result;
}

PEXPI void OsMarkStartTime()
{
	OsProgramStartTime = OsGetTime();
	OsProgramStartTimeMarked = true;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _OS_TIME_H
