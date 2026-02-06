/*
File:   base_notifications_impl.h
Author: Taylor Robbins
Date:   11\10\2025
*/

#ifndef _BASE_NOTIFICATIONS_IMPL_H
#define _BASE_NOTIFICATIONS_IMPL_H

#include "base/base_defines_check.h"
#include "base/base_compiler_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_dbg_level.h"
#include "base/base_assert.h"
#include "base/base_debug_output.h"
#include "base/base_notifications.h"
#include "std/std_printf.h"
#include "struct/struct_string.h"
#include "mem/mem_arena.h"
#include "mem/mem_scratch.h"
#include "misc/misc_printing.h"
#include "ui/ui_clay_notifications.h"

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	#if NOTIFICATION_QUEUE_AVAILABLE
	PIG_CORE_INLINE void SetGlobalNotificationQueue(NotificationQueue* queue);
	#endif //NOTIFICATION_QUEUE_AVAILABLE
	//NOTE: NotificationRouter and NotificationRouterPrint are pre-declared in base_notifications.h
	#if NOTIFICATION_QUEUE_AVAILABLE
	bool DismissNotification(NotificationQueue* queue, u64 programTime, bool dismissAll);
	#endif
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

#if NOTIFICATION_QUEUE_AVAILABLE
THREAD_LOCAL NotificationQueue* GlobalNotificationQueue = nullptr;
PEXPI void SetGlobalNotificationQueue(NotificationQueue* queue)
{
	GlobalNotificationQueue = queue;
}
#endif //NOTIFICATION_QUEUE_AVAILABLE

PEXP void NotificationRouter(const char* filePath, u32 lineNumber, const char* funcName, DbgLevel level, u64 duration, const char* message)
{
	if ((level == DbgLevel_Debug   && ENABLE_NOTIFICATION_LEVEL_DEBUG)   ||
		(level == DbgLevel_Regular && ENABLE_NOTIFICATION_LEVEL_REGULAR) ||
		(level == DbgLevel_Info    && ENABLE_NOTIFICATION_LEVEL_INFO)    ||
		(level == DbgLevel_Notify  && ENABLE_NOTIFICATION_LEVEL_NOTIFY)  ||
		(level == DbgLevel_Other   && ENABLE_NOTIFICATION_LEVEL_OTHER)   ||
		(level == DbgLevel_Warning && ENABLE_NOTIFICATION_LEVEL_WARNING) ||
		(level == DbgLevel_Error   && ENABLE_NOTIFICATION_LEVEL_ERROR)   ||
		level == DbgLevel_None || level >= DbgLevel_Count)
	{
		DebugOutputRouter(filePath, lineNumber, funcName, level, true, true, message);
		
		bool sentToQueue = false;
		#if NOTIFICATION_QUEUE_AVAILABLE
		if (GlobalNotificationQueue != nullptr && GlobalNotificationQueue->arena != nullptr)
		{
			Notification* newNotification = AddNotificationToQueue(GlobalNotificationQueue, level, MakeStr8Nt(message));
			NotNull(newNotification);
			newNotification->duration = (duration != 0) ? duration : DEFAULT_NOTIFICATION_TIME;
			sentToQueue = true;
		}
		#endif //NOTIFICATION_QUEUE_AVAILABLE
		
		// Error notifications are treated as breakpoints when no graphical display is available (only in debug builds)
		#if DEBUG_BUILD
		if (!sentToQueue && level >= DbgLevel_Error) { MyBreakMsg("Error notification!"); }
		#else
		UNUSED(sentToQueue);
		#endif
	}
}

PEXP void NotificationRouterPrint(const char* filePath, u32 lineNumber, const char* funcName, DbgLevel level, u64 duration, uxx printBufferLength, char* printBuffer, const char* formatString, ...)
{
	if ((level == DbgLevel_Debug   && ENABLE_NOTIFICATION_LEVEL_DEBUG)   ||
		(level == DbgLevel_Regular && ENABLE_NOTIFICATION_LEVEL_REGULAR) ||
		(level == DbgLevel_Info    && ENABLE_NOTIFICATION_LEVEL_INFO)    ||
		(level == DbgLevel_Notify  && ENABLE_NOTIFICATION_LEVEL_NOTIFY)  ||
		(level == DbgLevel_Other   && ENABLE_NOTIFICATION_LEVEL_OTHER)   ||
		(level == DbgLevel_Warning && ENABLE_NOTIFICATION_LEVEL_WARNING) ||
		(level == DbgLevel_Error   && ENABLE_NOTIFICATION_LEVEL_ERROR)   ||
		level == DbgLevel_None || level >= DbgLevel_Count)
	{
		// When doing multi-threaded applications we may decide to use a buffer allocated on the stack rather than reaching for a Scratch arena (the thread may not have initialized any scratch arenas)
		if (printBufferLength > 0 && printBuffer != nullptr)
		{
			va_list args;
			va_start(args, formatString);
			int printResult = MyVaListPrintf(printBuffer, printBufferLength, formatString, args);
			va_end(args);
			if (printResult >= 0)
			{
				printBuffer[printBufferLength-1] = '\0';
				NotificationRouter(filePath, lineNumber, funcName, level, duration, printBuffer);
			}
			#if NOTIFICATION_ERRORS_ON_FORMAT_FAILURE
			else
			{
				NotificationRouter(filePath, lineNumber, funcName, DbgLevel_Error, duration, "NOTIFICATION_BUFFER_PRINT_FAILED! FORMAT:");
				NotificationRouter(filePath, lineNumber, funcName, DbgLevel_Error, duration, formatString);
			}
			#endif //NOTIFICATION_ERRORS_ON_FORMAT_FAILURE
		}
		else
		{
			ScratchBegin(scratch);
			if (scratch != nullptr)
			{
				PrintInArenaVa(scratch, messageStr, messageLength, formatString);
				if (messageLength >= 0 && (messageStr != nullptr || messageLength == 0))
				{
					NotificationRouter(filePath, lineNumber, funcName, level, duration, messageStr);
				}
				#if NOTIFICATION_ERRORS_ON_FORMAT_FAILURE
				else
				{
					NotificationRouter(filePath, lineNumber, funcName, DbgLevel_Error, duration, "NOTIFICATION_SCRATCH_PRINT_FAILED! FORMAT:");
					NotificationRouter(filePath, lineNumber, funcName, DbgLevel_Error, duration, formatString);
				}
				#endif //NOTIFICATION_ERRORS_ON_FORMAT_FAILURE
			}
			else
			{
				NotificationRouter(filePath, lineNumber, funcName, DbgLevel_Error, duration, "NO_SCRATCH_FOR_NOTIFICATION_PRINT! FORMAT:");
				NotificationRouter(filePath, lineNumber, funcName, DbgLevel_Error, duration, formatString);
			}
			ScratchEnd(scratch);
		}
	}
}

#if NOTIFICATION_QUEUE_AVAILABLE
PEXP bool DismissNotification(NotificationQueue* queue, u64 programTime, bool dismissAll)
{
	NotNull(queue);
	bool closedNotification = false;
	VarArrayLoop(&queue->notifications, nIndex)
	{
		VarArrayLoopGet(Notification, notification, &queue->notifications, nIndex);
		u64 spawnAnimTime = TimeSinceBy(programTime, notification->spawnTime);
		if (spawnAnimTime < notification->duration - NOTIFICATION_DISAPPEAR_ANIM_TIME)
		{
			// PrintLine_D("Dismissing notification[%llu] %llu", nIndex, notification->id);
			notification->duration = (spawnAnimTime + NOTIFICATION_DISAPPEAR_ANIM_TIME);
			closedNotification = true;
			if (!dismissAll) { break; }
		}
	}
	return closedNotification;
}
#endif //NOTIFICATION_QUEUE_AVAILABLE

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _BASE_NOTIFICATIONS_IMPL_H
