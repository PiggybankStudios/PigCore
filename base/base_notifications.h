/*
File:   base_notifications.h
Author: Taylor Robbins
Date:   11\10\2025
Description:
	** Notifications are similar to debug output in that they are a way for use to send formatted strings to the user
	** In graphical applications the notification is displayed with a UI element that slides in (like a "toast"),
	** often in the bottom-left or bottom-right and then disappears after a period of time
	** (or when too many notifications are coming in and the notificiation queue gets too tall)
	** In non-graphical applications, or before the queue is set up, the notification just acts like a debug log
	** NOTE: Since the implementation of the routing functions depends on a few
	** things like mem_arena.h that want to be able to do notifications, this file
	** is split into a regular header (this one) and an implementation (base_notifications_impl.h).
	** That way mem_arena.h can #include this file without a circular dependency.
	** This does require the application to include base_notifications_impl.h if it
	** #includes any file that #includes this one.
*/

#ifndef _BASE_NOTIFICATIONS_H
#define _BASE_NOTIFICATIONS_H

#include "base/base_compiler_check.h"
#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_dbg_level.h"

#ifndef NOTIFICATION_ERRORS_ON_FORMAT_FAILURE
#define NOTIFICATION_ERRORS_ON_FORMAT_FAILURE 1
#endif

#ifndef ENABLE_NOTIFICATION_LEVEL_DEBUG
#define ENABLE_NOTIFICATION_LEVEL_DEBUG    DEBUG_BUILD
#endif
#ifndef ENABLE_NOTIFICATION_LEVEL_REGULAR
#define ENABLE_NOTIFICATION_LEVEL_REGULAR  1
#endif
#ifndef ENABLE_NOTIFICATION_LEVEL_INFO
#define ENABLE_NOTIFICATION_LEVEL_INFO     1
#endif
#ifndef ENABLE_NOTIFICATION_LEVEL_NOTIFY //TODO: This should be renamed! It was an okay word when we were just talking about debug output, but now that we have "notifications" it's confusing
#define ENABLE_NOTIFICATION_LEVEL_NOTIFY   1
#endif
#ifndef ENABLE_NOTIFICATION_LEVEL_OTHER
#define ENABLE_NOTIFICATION_LEVEL_OTHER    1
#endif
#ifndef ENABLE_NOTIFICATION_LEVEL_WARNING
#define ENABLE_NOTIFICATION_LEVEL_WARNING  1
#endif
#ifndef ENABLE_NOTIFICATION_LEVEL_ERROR
#define ENABLE_NOTIFICATION_LEVEL_ERROR    1
#endif

#if PIG_CORE_IMPLEMENTATION
PEXP void NotificationRouter(const char* filePath, u32 lineNumber, const char* funcName, DbgLevel level, const char* message);
PEXP void NotificationRouterPrint(const char* filePath, u32 lineNumber, const char* funcName, DbgLevel level, uxx printBufferLength, char* printBuffer, const char* formatString, ...);
#else
void NotificationRouter(const char* filePath, u32 lineNumber, const char* funcName, DbgLevel level, const char* message);
void NotificationRouterPrint(const char* filePath, u32 lineNumber, const char* funcName, DbgLevel level, uxx printBufferLength, char* printBuffer, const char* formatString, ...);
#endif

#define NotifyAt(level, message)                                   NotificationRouter     (__FILE__, __LINE__, __func__, (level), (message))
#define NotifyPrintAt(level, formatString, ...)                    NotificationRouterPrint(__FILE__, __LINE__, __func__, (level), 0, nullptr, (formatString), ##__VA_ARGS__)
#define NotifyBufferPrintAt(level, arrayBuffer, formatString, ...) NotificationRouterPrint(__FILE__, __LINE__, __func__, (level), ArrayCount(arrayBuffer), (arrayBuffer), (formatString), ##__VA_ARGS__)

#if ENABLE_NOTIFICATION_LEVEL_DEBUG
#define Notify_D(message)                                   NotificationRouter     (__FILE__, __LINE__, __func__, DbgLevel_Debug, (message))
#define NotifyPrint_D(formatString, ...)                    NotificationRouterPrint(__FILE__, __LINE__, __func__, DbgLevel_Debug, 0, nullptr, (formatString), ##__VA_ARGS__)
#define NotifyBufferPrint_D(arrayBuffer, formatString, ...) NotificationRouterPrint(__FILE__, __LINE__, __func__, DbgLevel_Debug, ArrayCount(arrayBuffer), (arrayBuffer), (formatString), ##__VA_ARGS__)
#else
#define Notify_D(message)                                   //nothing
#define NotifyPrint_D(formatString, ...)                    //nothing
#define NotifyBufferPrint_D(arrayBuffer, formatString, ...) //nothing
#endif

#if ENABLE_NOTIFICATION_LEVEL_REGULAR
#define Notify_R(message)                                   NotificationRouter     (__FILE__, __LINE__, __func__, DbgLevel_Regular, (message))
#define NotifyPrint_R(formatString, ...)                    NotificationRouterPrint(__FILE__, __LINE__, __func__, DbgLevel_Regular, 0, nullptr, (formatString), ##__VA_ARGS__)
#define NotifyBufferPrint_R(arrayBuffer, formatString, ...) NotificationRouterPrint(__FILE__, __LINE__, __func__, DbgLevel_Regular, ArrayCount(arrayBuffer), (arrayBuffer), (formatString), ##__VA_ARGS__)
#else
#define Notify_R(message)                                   //nothing
#define NotifyPrint_R(formatString, ...)                    //nothing
#define NotifyBufferPrint_R(arrayBuffer, formatString, ...) //nothing
#endif

#if ENABLE_NOTIFICATION_LEVEL_INFO
#define Notify_I(message)                                   NotificationRouter     (__FILE__, __LINE__, __func__, DbgLevel_Info, (message))
#define NotifyPrint_I(formatString, ...)                    NotificationRouterPrint(__FILE__, __LINE__, __func__, DbgLevel_Info, 0, nullptr, (formatString), ##__VA_ARGS__)
#define NotifyBufferPrint_I(arrayBuffer, formatString, ...) NotificationRouterPrint(__FILE__, __LINE__, __func__, DbgLevel_Info, ArrayCount(arrayBuffer), (arrayBuffer), (formatString), ##__VA_ARGS__)
#else
#define Notify_I(message)                                   //nothing
#define NotifyPrint_I(formatString, ...)                    //nothing
#define NotifyBufferPrint_I(arrayBuffer, formatString, ...) //nothing
#endif

#if ENABLE_NOTIFICATION_LEVEL_NOTIFY
#define Notify_N(message)                                   NotificationRouter     (__FILE__, __LINE__, __func__, DbgLevel_Notify, (message))
#define NotifyPrint_N(formatString, ...)                    NotificationRouterPrint(__FILE__, __LINE__, __func__, DbgLevel_Notify, 0, nullptr, (formatString), ##__VA_ARGS__)
#define NotifyBufferPrint_N(arrayBuffer, formatString, ...) NotificationRouterPrint(__FILE__, __LINE__, __func__, DbgLevel_Notify, ArrayCount(arrayBuffer), (arrayBuffer), (formatString), ##__VA_ARGS__)
#else
#define Notify_N(message)                                   //nothing
#define NotifyPrint_N(formatString, ...)                    //nothing
#define NotifyBufferPrint_N(arrayBuffer, formatString, ...) //nothing
#endif

#if ENABLE_NOTIFICATION_LEVEL_OTHER
#define Notify_O(message)                                   NotificationRouter     (__FILE__, __LINE__, __func__, DbgLevel_Other, (message))
#define NotifyPrint_O(formatString, ...)                    NotificationRouterPrint(__FILE__, __LINE__, __func__, DbgLevel_Other, 0, nullptr, (formatString), ##__VA_ARGS__)
#define NotifyBufferPrint_O(arrayBuffer, formatString, ...) NotificationRouterPrint(__FILE__, __LINE__, __func__, DbgLevel_Other, ArrayCount(arrayBuffer), (arrayBuffer), (formatString), ##__VA_ARGS__)
#else
#define Notify_O(message)                                   //nothing
#define NotifyPrint_O(formatString, ...)                    //nothing
#define NotifyBufferPrint_O(arrayBuffer, formatString, ...) //nothing
#endif

#if ENABLE_NOTIFICATION_LEVEL_WARNING
#define Notify_W(message)                                   NotificationRouter     (__FILE__, __LINE__, __func__, DbgLevel_Warning, (message))
#define NotifyPrint_W(formatString, ...)                    NotificationRouterPrint(__FILE__, __LINE__, __func__, DbgLevel_Warning, 0, nullptr, (formatString), ##__VA_ARGS__)
#define NotifyBufferPrint_W(arrayBuffer, formatString, ...) NotificationRouterPrint(__FILE__, __LINE__, __func__, DbgLevel_Warning, ArrayCount(arrayBuffer), (arrayBuffer), (formatString), ##__VA_ARGS__)
#else
#define Notify_W(message)                                   //nothing
#define NotifyPrint_W(formatString, ...)                    //nothing
#define NotifyBufferPrint_W(arrayBuffer, formatString, ...) //nothing
#endif

#if ENABLE_NOTIFICATION_LEVEL_ERROR
#define Notify_E(message)                                   NotificationRouter     (__FILE__, __LINE__, __func__, DbgLevel_Error, (message))
#define NotifyPrint_E(formatString, ...)                    NotificationRouterPrint(__FILE__, __LINE__, __func__, DbgLevel_Error, 0, nullptr, (formatString), ##__VA_ARGS__)
#define NotifyBufferPrint_E(arrayBuffer, formatString, ...) NotificationRouterPrint(__FILE__, __LINE__, __func__, DbgLevel_Error, ArrayCount(arrayBuffer), (arrayBuffer), (formatString), ##__VA_ARGS__)
#else
#define Notify_E(message)                                   //nothing
#define NotifyPrint_E(formatString, ...)                    //nothing
#define NotifyBufferPrint_E(arrayBuffer, formatString, ...) //nothing
#endif

#endif //  _BASE_NOTIFICATIONS_H
