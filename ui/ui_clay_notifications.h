/*
File:   ui_clay_notifications.h
Author: Taylor Robbins
Date:   11\07\2025
Description:
	** This is the graphical side of the notification queue when using Clay as the layout engine and sokol_gfx.h
	** The NotificationQueue can be used as the output for base_notifications.h in graphical applications, otherwise notifications are treated as regular debug output
*/

//TODO: Notifications with '\n' character cause incorrect behavior because clay is handling them even when wrapMode=CLAY_TEXT_WRAP_NONE on the text element

#ifndef _UI_CLAY_NOTIFICATIONS_H
#define _UI_CLAY_NOTIFICATIONS_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_dbg_level.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "misc/misc_easing.h"
#include "struct/struct_vectors.h"
#include "struct/struct_rectangles.h"
#include "struct/struct_string.h"
#include "struct/struct_color.h"
#include "mem/mem_arena.h"
#include "struct/struct_var_array.h"
#include "ui/ui_clay.h"
#include "ui/ui_clay_widget_context.h"
#include "lib/lib_sokol_gfx.h"
#include "gfx/gfx_font.h"
#include "gfx/gfx_texture.h"

#if BUILD_WITH_SOKOL_GFX && BUILD_WITH_CLAY
#define NOTIFICATION_QUEUE_AVAILABLE 1
#else
#define NOTIFICATION_QUEUE_AVAILABLE 0
#endif

#if NOTIFICATION_QUEUE_AVAILABLE

#define MAX_NOTIFICATIONS                  16
#define DEFAULT_NOTIFICATION_TIME          5*1000 //ms
#define NOTIFICATION_BETWEEN_MARGIN        3 //px
#define NOTIFICATION_PADDING               8 //px
#define NOTIFICATION_MOVE_LAG              3 //divisor
#define NOTIFICATION_APPEAR_ANIM_TIME      200 //ms
#define NOTIFICATION_DISAPPEAR_ANIM_TIME   300 //ms
#define NOTIFICATION_SCREEN_MARGIN_RIGHT   4 //px
#define NOTIFICATION_SCREEN_MARGIN_BOTTOM  4 //px
#define NOTIFICATION_MAX_WIDTH_PERCENT     0.50f //percent of screen width
#define NOTIFICATION_AUTO_DISMISS_SCREEN_HEIGHT_PERCENT 0.5f //percent of screen height

typedef plex Notification Notification;
plex Notification
{
	Arena* arena;
	u64 id;
	u64 spawnTime;
	u64 duration;
	Str8 messageStr;
	r32 currentOffsetY;
	r32 gotoOffsetY;
	TextMeasure textMeasure;
	r32 textMeasureWrapWidth;
	r32 textMeasureLineHeight;
	DbgLevel level;
};

typedef plex NotificationIcon NotificationIcon;
plex NotificationIcon
{
	DbgLevel level;
	Texture* texture;
	r32 scale;
	rec sourceRec;
	Color32 color;
};

typedef plex NotificationQueue NotificationQueue;
plex NotificationQueue
{
	Arena* arena;
	u64 nextId;
	VarArray notifications; //Notification
	v2i prevScreenSize;
	u64 currentProgramTime;
	NotificationIcon icons[DbgLevel_Count];
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void FreeNotification(Notification* notification);
	PIG_CORE_INLINE void FreeNotificationQueue(NotificationQueue* queue);
	PIG_CORE_INLINE void InitNotificationQueue(Arena* arena, NotificationQueue* queueOut);
	PIG_CORE_INLINE void SetNotificationIconEx(NotificationQueue* queue, DbgLevel level, Texture* texture, r32 scale, Color32 color, rec sourceRec);
	PIG_CORE_INLINE void SetNotificationIcon(NotificationQueue* queue, DbgLevel level, Texture* texture, r32 scale, Color32 color);
	Notification* AddNotificationToQueue(NotificationQueue* queue, DbgLevel level, Str8 message);
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
	queueOut->currentProgramTime = 0;
	for (uxx lIndex = 0; lIndex < DbgLevel_Count; lIndex++) { queueOut->icons[lIndex].level = (DbgLevel)lIndex; }
}

PEXPI void SetNotificationIconEx(NotificationQueue* queue, DbgLevel level, Texture* texture, r32 scale, Color32 color, rec sourceRec)
{
	Assert(level < DbgLevel_Count);
	queue->icons[level].texture = texture;
	queue->icons[level].scale = scale;
	queue->icons[level].sourceRec = sourceRec;
	if (sourceRec.X == 0 && sourceRec.Y == 0 && sourceRec.Width == 0 && sourceRec.Height == 0 && texture != nullptr)
	{
		queue->icons[level].sourceRec = NewRec(0, 0, (r32)texture->Width, (r32)texture->Height);
	}
	queue->icons[level].color = color;
}
PEXPI void SetNotificationIcon(NotificationQueue* queue, DbgLevel level, Texture* texture, r32 scale, Color32 color)
{
	SetNotificationIconEx(queue, level, texture, scale, color, NEW_STRUCT(rec)Rec_Zero_Const);
}

PEXP Notification* AddNotificationToQueue(NotificationQueue* queue, DbgLevel level, Str8 message)
{
	NotNull(queue);
	NotNull(queue->arena);
	NotNullStr(message);
	
	//TODO: Check for existing duplicates of this message. Move them to the bottom and increment their repetition count!
	
	if (queue->notifications.length >= MAX_NOTIFICATIONS)
	{
		Notification* oldestNotification = VarArrayGetLast(Notification, &queue->notifications);
		// PrintLine_D("Removing notification (limit reached) \"%.*s\"", StrPrint(oldestNotification->messageStr));
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
	newNotification->spawnTime = queue->currentProgramTime;
	newNotification->duration = DEFAULT_NOTIFICATION_TIME;
	newNotification->level = level;
	
	return newNotification;
}

PEXP void DoUiNotificationQueue(UiWidgetContext* context, NotificationQueue* queue, PigFont* font, r32 fontSize, u8 fontStyle, v2i screenSize)
{
	NotNull(context);
	NotNull(context->uiArena);
	NotNull(context->renderer);
	NotNull(queue);
	NotNull(queue->arena);
	NotNull(font);
	queue->currentProgramTime = context->programTime;
	
	u16 fontId = GetClayUIRendererFontId(context->renderer, font, fontStyle);
	bool screenSizeChanged = (!AreEqualV2i(queue->prevScreenSize, screenSize));
	r32 lineHeight = GetFontLineHeight(font, fontSize, fontStyle);
	r32 wrapWidth = (screenSize.Width * NOTIFICATION_MAX_WIDTH_PERCENT) - (r32)(2*UISCALE_U16(context->uiScale, NOTIFICATION_PADDING));
	
	VarArrayLoop(&queue->notifications, nIndex)
	{
		VarArrayLoopGet(Notification, notification, &queue->notifications, nIndex);
		
		// If we're halfway up the screen, auto-dismiss the notification by shortening the duration
		if (notification->gotoOffsetY >= screenSize.Height * (r32)NOTIFICATION_AUTO_DISMISS_SCREEN_HEIGHT_PERCENT)
		{
			// PrintLine_D("Dismissing notification (height limit) \"%.*s\"", StrPrint(notification->messageStr));
			u64 currentTime = TimeSinceBy(context->programTime, notification->spawnTime);
			if (currentTime < notification->duration - NOTIFICATION_DISAPPEAR_ANIM_TIME)
			{
				notification->duration = currentTime + NOTIFICATION_DISAPPEAR_ANIM_TIME;
			}
		}
		
		if (TimeSinceBy(context->programTime, notification->spawnTime) >= notification->duration)
		{
			// PrintLine_D("Dismissing notification (timeout) \"%.*s\"", StrPrint(notification->messageStr));
			FreeNotification(notification);
			VarArrayRemoveAt(Notification, &queue->notifications, nIndex);
			nIndex--;
			continue;
		}
		
		if (!AreSimilarR32(wrapWidth, notification->textMeasureWrapWidth, DEFAULT_R32_TOLERANCE) ||
			!AreSimilarR32(lineHeight, notification->textMeasureLineHeight, DEFAULT_R32_TOLERANCE) ||
			(notification->textMeasure.Width == 0 && notification->textMeasure.Height == 0 && !IsEmptyStr(notification->messageStr)))
		{
			notification->textMeasure = MeasureTextEx(font, fontSize, fontStyle, false, wrapWidth, notification->messageStr);
			notification->textMeasureWrapWidth = wrapWidth;
			notification->textMeasureLineHeight = lineHeight;
		}
	}
	
	rec prevNotificationDrawRec = Rec_Zero;
	VarArrayLoop(&queue->notifications, nIndex)
	{
		VarArrayLoopGet(Notification, notification, &queue->notifications, nIndex);
		ClayId notificationId = ToClayIdPrint(context->uiArena, "Notification%llu", (u64)notification->id);
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
		
		Color32 backgroundColor = ColorWithAlpha(MonokaiDarkGray, 1.0f - disappearAnimAmount);
		Color32 textColor = ColorWithAlpha(MonokaiWhite, 1.0f - disappearAnimAmount);
		Color32 borderColor = ColorWithAlpha(MonokaiLightGray, 1.0f - disappearAnimAmount);
		v2 offset = NewV2(
			-UISCALE_R32(context->uiScale, NOTIFICATION_SCREEN_MARGIN_RIGHT),
			-UISCALE_R32(context->uiScale, NOTIFICATION_SCREEN_MARGIN_BOTTOM) - notification->currentOffsetY
		);
		if (appearAnimAmount < 1.0f && isSizeKnown)
		{
			offset.X += notificationDrawRec.Width * EaseExponentialIn(1.0f - appearAnimAmount);
		}
		NotificationIcon* icon = (notification->level < DbgLevel_Count) ? &queue->icons[notification->level] : nullptr;
		Texture* iconTexture = (icon != nullptr) ? icon->texture : nullptr;
		Color32 iconColor = (icon != nullptr) ? icon->color : MonokaiWhite;
		
		CLAY({ .id = notificationId,
			.layout = {
				.sizing = { .width = CLAY_SIZING_FIT(0), .height = CLAY_SIZING_FIT(0) },
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
				if (iconTexture != nullptr)
				{
					CLAY({
						.layout = {
							.sizing = {
								.width = CLAY_SIZING_FIXED(UISCALE_R32(context->uiScale, icon->sourceRec.Width * icon->scale)),
								.height = CLAY_SIZING_FIXED(UISCALE_R32(context->uiScale, icon->sourceRec.Height * icon->scale)),
							},
						},
						.image = {
							.imageData = iconTexture,
							.sourceDimensions = ToV2Fromi(iconTexture->size),
						},
						.backgroundColor = ColorWithAlpha(iconColor, 1.0f - disappearAnimAmount),
						.userData = { .imageSourceRec = icon->sourceRec },
					}) {}
				}
				
				CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_FIXED(notification->textMeasure.visualRec.Width), .height = CLAY_SIZING_FIXED(notification->textMeasure.visualRec.Height) } } })
				{
					CLAY_TEXT(
						AllocStr8(context->uiArena, notification->messageStr),
						CLAY_TEXT_CONFIG({
							.fontId = fontId,
							.fontSize = (u16)fontSize,
							.textColor = textColor,
							.wrapMode = CLAY_TEXT_WRAP_NONE,
							.textAlignment = CLAY_TEXT_ALIGN_LEFT,
							.userData = { .wrapWidth = wrapWidth },
						})
					);
				}
			}
		}
	}
	
	queue->prevScreenSize = screenSize;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //NOTIFICATION_QUEUE_AVAILABLE

#endif //  _UI_CLAY_NOTIFICATIONS_H
