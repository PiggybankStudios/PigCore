/*
File:   tests_android.c
Author: Taylor Robbins
Date:   09\15\2025
Description: 
	** A simple C program to test compilation to Android
*/

#include "android_native_app_glue.c"

#include <jni.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <android/log.h>
#include <android/looper.h>

#include <GLES2/gl2.h>        // OpenGL ES 2.0
#include <GLES2/gl2ext.h>     // Extensions
#include <EGL/egl.h>          // EGL for context management

#define nullptr ((void*)0)

#define PrintLine_I(...) ((void)__android_log_print(ANDROID_LOG_INFO, "threaded_app", __VA_ARGS__))
#define PrintLine_E(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "threaded_app", __VA_ARGS__))

#if 0
void HandleAndroidCmd(struct android_app* app, int32_t cmd)
{
	switch (cmd)
	{
		case APP_CMD_INPUT_CHANGED:        PrintLine_I("Got command: INPUT_CHANGED\n");        break; //TODO: Implement me!
		case APP_CMD_INIT_WINDOW:          PrintLine_I("Got command: INIT_WINDOW\n");          break; //TODO: Implement me!
		case APP_CMD_TERM_WINDOW:          PrintLine_I("Got command: TERM_WINDOW\n");          break; //TODO: Implement me!
		case APP_CMD_WINDOW_RESIZED:       PrintLine_I("Got command: WINDOW_RESIZED\n");       break; //TODO: Implement me!
		case APP_CMD_WINDOW_REDRAW_NEEDED: PrintLine_I("Got command: WINDOW_REDRAW_NEEDED\n"); break; //TODO: Implement me!
		case APP_CMD_CONTENT_RECT_CHANGED: PrintLine_I("Got command: CONTENT_RECT_CHANGED\n"); break; //TODO: Implement me!
		case APP_CMD_GAINED_FOCUS:         PrintLine_I("Got command: GAINED_FOCUS\n");         break; //TODO: Implement me!
		case APP_CMD_LOST_FOCUS:           PrintLine_I("Got command: LOST_FOCUS\n");           break; //TODO: Implement me!
		case APP_CMD_CONFIG_CHANGED:       PrintLine_I("Got command: CONFIG_CHANGED\n");       break; //TODO: Implement me!
		case APP_CMD_LOW_MEMORY:           PrintLine_I("Got command: LOW_MEMORY\n");           break; //TODO: Implement me!
		case APP_CMD_START:                PrintLine_I("Got command: START\n");                break; //TODO: Implement me!
		case APP_CMD_RESUME:               PrintLine_I("Got command: RESUME\n");               break; //TODO: Implement me!
		case APP_CMD_SAVE_STATE:           PrintLine_I("Got command: SAVE_STATE\n");           break; //TODO: Implement me!
		case APP_CMD_PAUSE:                PrintLine_I("Got command: PAUSE\n");                break; //TODO: Implement me!
		case APP_CMD_STOP:                 PrintLine_I("Got command: STOP\n");                 break; //TODO: Implement me!
		case APP_CMD_DESTROY:              PrintLine_I("Got command: DESTROY\n");              break; //TODO: Implement me!
		default:                           PrintLine_I("Got unknown command: %d", cmd);        break;
	}
}
#endif

// Example rendering function
void draw_frame()
{
	glClear(GL_COLOR_BUFFER_BIT);
	
	// Draw your "buttons", text, etc. using OpenGL
	// render_button(button_x, button_y, button_width, button_height);
	// render_text("Hello World", text_x, text_y);
	
	// eglSwapBuffers(display, surface);
}

void android_main(struct android_app* app)
{
	PrintLine_I("android_main() is running...\n");
	// app_dummy(); //Suppresses link-time dead code removal of stuff in android_native_app_glue.c
	(void)ANativeActivity_onCreate;
	
	// app->onAppCmd = HandleAndroidCmd;
	
	// if (init_display(app) == 0) { PrintLine_E("Failed to init_display!\n"); return; }
	
	// +==============================+
	// |        Main Game Loop        |
	// +==============================+
	while (true)
	{
		// +==============================+
		// |        Event Polling         |
		// +==============================+
		int pollResult = 0;
		int events = 0;
		struct android_poll_source* source = nullptr;
		while ((pollResult = ALooper_pollOnce(0, nullptr, &events, (void**)&source)) >= 0)
		{
			if (source != nullptr)
			{
				source->process(app, source);
			}
			
			if (app->destroyRequested != 0) { break; }
		}
		if (pollResult == ALOOPER_POLL_ERROR) { PrintLine_E("ALooper returned a POLL_ERROR error!"); }
		if (app->destroyRequested != 0) { break; }
		
		draw_frame();
	}
}


// JNINativeInterface
JNIEXPORT jstring JNICALL Java_com_piggybank_sputnik_MainActivity_MyCFunction(JNIEnv* env, jobject this)
{
	PrintLine_I("Hello Android from MyCFunction() in C!!\n");
	return (*env)->NewStringUTF(env, "String from MyCFunction()");
}
