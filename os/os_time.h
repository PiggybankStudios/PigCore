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
#include "base/base_assert.h"

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

#define WIN32_FILETIME_SEC_OFFSET           11644473600ULL //11,644,473,600 seconds between Jan 1st 1601 and Jan 1st 1970

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	u64 OsGetCurrentTimestampEx(bool local, i64* timezoneOffsetOut, bool* timezoneDoesDstOut, Arena* timezoneNameArena, Str8* timezoneNameOut);
	PIG_CORE_INLINE u64 OsGetCurrentTimestamp(bool local);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

//TODO: Re-add the following output but in a cross file or some kind since it relies on ConvertUcs2StrToUtf8: Arena* timezoneNameArena, Str8* timezoneNameOut
PEXP u64 OsGetCurrentTimestampEx(bool local, i64* timezoneOffsetOut, bool* timezoneDoesDstOut) 
{
	// if (timezoneNameOut != nullptr) { NotNull(timezoneNameArena); }
	u64 result = 0;
	
	#if TARGET_IS_WINDOWS
	{
		if (local)
		{
			u64 unixTimestamp = OsGetCurrentTimestampEx(false, nullptr, nullptr);
			TIME_ZONE_INFORMATION timezoneInfo = ZEROED;
			DWORD timezoneResult = GetTimeZoneInformation(&timezoneInfo);
			DebugAssertMsg(timezoneResult != TIME_ZONE_ID_INVALID, "GetTimeZoneInformation failed and gave TIME_ZONE_ID_INVALID");
			DEBUG_USED(timezoneResult);
			i64 localTimezoneOffset = -((i64)timezoneInfo.Bias * NUM_SEC_PER_MINUTE);
			if (timezoneOffsetOut != nullptr) { *timezoneOffsetOut = localTimezoneOffset; }
			result = unixTimestamp + localTimezoneOffset;
			bool localTimezoneDoesDst = (timezoneInfo.DaylightBias != 0); //TODO: It's possible that DaylightBias isn't -60 minutes. Should we handle that?
			if (timezoneDoesDstOut != nullptr) { *timezoneDoesDstOut = localTimezoneDoesDst; }
			// if (timezoneNameOut != nullptr)
			// {
			// 	*timezoneNameOut = ConvertUcs2StrToUtf8(timezoneNameArena, Str16Lit(&timezoneInfo.StandardName[0]), false);
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
	#else
	AssertMsg(false, "OsGetCurrentTimestampEx does not support the current platform yet!")
	#endif
	
	return result;
}
PEXPI u64 OsGetCurrentTimestamp(bool local) { return OsGetCurrentTimestampEx(local, nullptr, nullptr); }

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _OS_TIME_H
