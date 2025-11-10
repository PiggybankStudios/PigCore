/*
File:   ui_clay_notifications.h
Author: Taylor Robbins
Date:   11\07\2025
*/

#ifndef _UI_CLAY_NOTIFICATIONS_H
#define _UI_CLAY_NOTIFICATIONS_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_dbg_level.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "struct/struct_string.h"
#include "mem/mem_arena.h"
#include "struct/struct_var_array.h"
#include "ui/ui_clay.h"
#include "lib/lib_sokol_gfx.h"

#if BUILD_WITH_SOKOL_GFX && BUILD_WITH_CLAY

#define MAX_NOTIFICATIONS                  16
#define DEFAULT_NOTIFICATION_TIME          5*1000 //ms
#define NOTIFICATION_BETWEEN_MARGIN        3 //px
#define NOTIFICATION_PADDING               8 //px
#define NOTIFICATION_MOVE_LAG              3 //divisor
#define NOTIFICATION_APPEAR_ANIM_TIME      200 //ms
#define NOTIFICATION_DISAPPEAR_ANIM_TIME   300 //ms
#define NOTIFICATION_SCREEN_MARGIN_RIGHT   4 //px
#define NOTIFICATION_SCREEN_MARGIN_BOTTOM  4 //px
#define NOTIFICATION_ICON_SIZE             32 //px
#define NOTIFICATION_AUTO_DISMISS_SCREEN_HEIGHT_PERCENT 0.5f //percent of screen height

typedef struct Notification Notification;
struct Notification
{
	Arena* arena;
	u64 id;
	u64 spawnTime;
	u64 duration;
	Str8 messageStr;
	r32 currentOffsetY;
	r32 gotoOffsetY;
	DbgLevel level;
};

typedef struct NotificationQueue NotificationQueue;
struct NotificationQueue
{
	Arena* arena;
	u64 nextId;
	VarArray notifications; //Notification
	v2i prevScreenSize;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void FreeNotification(Notification* notification);
	PIG_CORE_INLINE void FreeNotificationQueue(NotificationQueue* queue);
	PIG_CORE_INLINE void InitNotificationQueue(Arena* arena, NotificationQueue* queueOut);
	Notification* AddNotificationToQueue(NotificationQueue* queue, DbgLevel level, Str8 message, u64 programTime);
	void DoUiNotificationQueue(UiWidgetContext* context, NotificationQueue* queue, PigFont* font, r32 fontSize, u8 fontStyle, v2i screenSize);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI void FreeNotification(Notification* notification)
{
	NotNull(notification);
	if (notification->arena != nullptr)
	{
		FreeStr8(notification->arena, &notification->messageStr);
	}
	ClearPointer(notification);
}

PEXPI void FreeNotificationQueue(NotificationQueue* queue)
{
	VarArrayLoop(&queue->notifications, nIndex)
	{
		VarArrayLoopGet(Notification, notification, &queue->notifications, nIndex);
		FreeNotification(notification);
	}
	FreeVarArray(&queue->notifications);
	ClearPointer(queue);
}

PEXPI void InitNotificationQueue(Arena* arena, NotificationQueue* queueOut)
{
	NotNull(arena);
	NotNull(queueOut);
	ClearPointer(queueOut);
	queueOut->arena = arena;
	InitVarArray(Notification, &queueOut->notifications, arena);
	queueOut->nextId = 1;
}

PEXP Notification* AddNotificationToQueue(NotificationQueue* queue, DbgLevel level, Str8 message, u64 programTime)
{
	NotNull(queue);
	NotNull(queue->arena);
	NotNullStr(message);
	
	//TODO: Check for existing duplicates of this message. Move them to the bottom and increment their repetition count!
	
	if (queue->notifications.length >= MAX_NOTIFICATIONS)
	{
		Notification* oldestNotification = VarArrayGetLast(Notification, &queue->notifications);
		FreeNotification(oldestNotification);
		VarArrayRemoveLast(Notification, &queue->notifications);
	}
	
	Notification* newNotification = VarArrayInsert(Notification, &queue->notifications, 0);
	NotNull(newNotification);
	ClearPointer(newNotification);
	newNotification->arena = queue->arena;
	newNotification->id = queue->nextId;
	queue->nextId++;
	newNotification->messageStr = AllocStr8(queue->arena, message);
	newNotification->spawnTime = programTime;
	newNotification->duration = DEFAULT_NOTIFICATION_TIME;
	newNotification->level = level;
	
	return newNotification;
}

PEXP void DoUiNotificationQueue(UiWidgetContext* context, NotificationQueue* queue, PigFont* font, r32 fontSize, u8 fontStyle, v2i screenSize)
{
	NotNull(context);
	NotNull(queue);
	NotNull(queue->arena);
	NotNull(font);
	
	u16 fontId = GetClayUIRendererFontId(context->renderer, font, fontStyle);
	bool screenSizeChanged = (!AreEqualV2i(queue->prevScreenSize, screenSize));
	
	rec prevNotificationDrawRec = Rec_Zero;
	VarArrayLoop(&queue->notifications, nIndex)
	{
		VarArrayLoopGet(Notification, notification, &queue->notifications, nIndex);
		
		// If we're halfway up the screen, auto-dismiss the notification by shortening the duration
		if (notification->gotoOffsetY >= screenSize.Height * (r32)NOTIFICATION_AUTO_DISMISS_SCREEN_HEIGHT_PERCENT)
		{
			u64 currentTime = TimeSinceBy(context->programTime, notification->spawnTime);
			if (currentTime < notification->duration - NOTIFICATION_DISAPPEAR_ANIM_TIME)
			{
				notification->duration = currentTime + NOTIFICATION_DISAPPEAR_ANIM_TIME;
			}
		}
		
		if (TimeSinceBy(context->programTime, notification->spawnTime) >= notification->duration)
		{
			FreeNotification(notification);
			VarArrayRemoveAt(Notification, &queue->notifications, nIndex);
			nIndex--;
			continue;
		}
	}
	
	VarArrayLoop(&queue->notifications, nIndex)
	{
		VarArrayLoopGet(Notification, notification, &queue->notifications, nIndex);
		Str8 notificationIdStr = ScratchPrintStr("Notification%llu", (u64)notification->id);
		ClayId notificationId = ToClayId(notificationIdStr);
		rec notificationDrawRec = GetClayElementDrawRec(notificationId);
		bool isSizeKnown = (notificationDrawRec.Width > 0);
		
		if (!screenSizeChanged && prevNotificationDrawRec.Width > 0 && prevNotificationDrawRec.Height > 0)
		{
			notification->gotoOffsetY = ((r32)screenSize.Height - UISCALE_R32(context->uiScale, NOTIFICATION_SCREEN_MARGIN_BOTTOM)) - (prevNotificationDrawRec.Y - UISCALE_R32(context->uiScale, NOTIFICATION_BETWEEN_MARGIN));
		}
		prevNotificationDrawRec = notificationDrawRec;
		
		r32 offsetDiff = (notification->gotoOffsetY - notification->currentOffsetY);
		if (AbsR32(offsetDiff) >= 1.0f) { notification->currentOffsetY += offsetDiff / NOTIFICATION_MOVE_LAG; }
		else { notification->currentOffsetY = notification->gotoOffsetY; }
		
		u64 spawnAnimTime = TimeSinceBy(context->programTime, notification->spawnTime);
		r32 appearAnimAmount = 1.0f;
		r32 disappearAnimAmount = 0.0f;
		if (spawnAnimTime > notification->duration - NOTIFICATION_DISAPPEAR_ANIM_TIME)
		{
			disappearAnimAmount = (r32)(spawnAnimTime - (notification->duration - NOTIFICATION_DISAPPEAR_ANIM_TIME)) / (r32)NOTIFICATION_DISAPPEAR_ANIM_TIME;
		}
		else if (spawnAnimTime < NOTIFICATION_APPEAR_ANIM_TIME)
		{
			appearAnimAmount = (r32)spawnAnimTime / (r32)NOTIFICATION_APPEAR_ANIM_TIME;
		}
		
		Color32 backgroundColor = ColorWithAlpha(MonokaiGray2, 1.0f - disappearAnimAmount);
		Color32 textColor = ColorWithAlpha(MonokaiWhite, 1.0f - disappearAnimAmount);
		Color32 borderColor = ColorWithAlpha(MonokaiLightGray, 1.0f - disappearAnimAmount);
		Color32 iconColor = MonokaiWhite;
		v2 offset = NewV2(
			-UISCALE_R32(context->uiScale, NOTIFICATION_SCREEN_MARGIN_RIGHT),
			-UISCALE_R32(context->uiScale, NOTIFICATION_SCREEN_MARGIN_BOTTOM) - notification->currentOffsetY
		);
		if (appearAnimAmount < 1.0f && isSizeKnown)
		{
			offset.X += notificationDrawRec.Width * EaseExponentialIn(1.0f - appearAnimAmount);
		}
		#if 1
		Texture* iconTexture = nullptr;
		#else
		AppIcon appIcon = GetAppIconForNotificationLevel(notification->level, &iconColor);
		Texture* iconTexture = (appIcon != AppIcon_None) ? &app->icons[appIcon] : nullptr;
		#endif
		
		CLAY({ .id = notificationId,
			.layout = {
				.sizing = { .width = CLAY_SIZING_FIT(0, screenSize.Width*0.75f), .height = CLAY_SIZING_FIT(0) },
				.padding = CLAY_PADDING_ALL(UISCALE_U16(context->uiScale, NOTIFICATION_PADDING)),
				.childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER },
			},
			.floating = {
				.zIndex = 102,
				.offset = offset,
				.attachTo = CLAY_ATTACH_TO_PARENT,
				.pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_CAPTURE,
				.attachPoints = {
					.parent = CLAY_ATTACH_POINT_RIGHT_BOTTOM,
					.element = (isSizeKnown ? CLAY_ATTACH_POINT_RIGHT_BOTTOM : CLAY_ATTACH_POINT_LEFT_BOTTOM),
				},
			},
			.backgroundColor = backgroundColor,
			.cornerRadius = CLAY_CORNER_RADIUS(UISCALE_R32(context->uiScale, 8)),
			.border = { .width = CLAY_BORDER_OUTSIDE(UISCALE_BORDER(context->uiScale, 2)), .color = borderColor },
		})
		{
			CLAY({ .layout={ .layoutDirection=CLAY_LEFT_TO_RIGHT, .childGap=UISCALE_U16(context->uiScale, 5), .childAlignment={ .y=CLAY_ALIGN_Y_CENTER } } })
			{
				// if (iconTexture != nullptr)
				// {
				// 	CLAY_ICON(iconTexture, FillV2(UISCALE_R32(context->uiScale, NOTIFICATION_ICON_SIZE)), ColorWithAlpha(iconColor, 1.0f - disappearAnimAmount));
				// }
				
				CLAY_TEXT(
					notification->messageStr,
					CLAY_TEXT_CONFIG({
						.fontId = fontId,
						.fontSize = (u16)fontSize,
						.textColor = textColor,
					})
				);
			}
		}
	}
	
	queue->prevScreenSize = screenSize;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_SOKOL_GFX && BUILD_WITH_CLAY

#endif //  _UI_CLAY_NOTIFICATIONS_H
