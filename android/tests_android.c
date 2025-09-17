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

#define PrintLine_V(message, ...) ((void)__android_log_print(ANDROID_LOG_VERBOSE, "pigcore", message "\n", ##__VA_ARGS__))
#define PrintLine_D(message, ...) ((void)__android_log_print(ANDROID_LOG_DEBUG,   "pigcore", message "\n", ##__VA_ARGS__))
#define PrintLine_I(message, ...) ((void)__android_log_print(ANDROID_LOG_INFO,    "pigcore", message "\n", ##__VA_ARGS__))
#define PrintLine_W(message, ...) ((void)__android_log_print(ANDROID_LOG_WARN,    "pigcore", message "\n", ##__VA_ARGS__))
#define PrintLine_E(message, ...) ((void)__android_log_print(ANDROID_LOG_ERROR,   "pigcore", message "\n", ##__VA_ARGS__))
#define PrintLine_F(message, ...) ((void)__android_log_print(ANDROID_LOG_FATAL,   "pigcore", message "\n", ##__VA_ARGS__))
#define WriteLine_V(message)      PrintLine_V(message)
#define WriteLine_D(message)      PrintLine_D(message)
#define WriteLine_I(message)      PrintLine_I(message)
#define WriteLine_W(message)      PrintLine_W(message)
#define WriteLine_E(message)      PrintLine_E(message)
#define WriteLine_F(message)      PrintLine_F(message)

bool initialized = false;
EGLDisplay display = {0};
EGLConfig config = {0};
EGLSurface surface = {0};
EGLContext context = {0};
int frameIndex = 0;

// Example rendering function
bool draw_frame(struct android_app* app)
{
	if (!initialized && app->window != nullptr)
	{
		PrintLine_I("Initializing...");
		display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
		eglInitialize(display, nullptr, nullptr);
		const EGLint eglConfigAttribs[] = {
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
			EGL_BLUE_SIZE, 8,
			EGL_GREEN_SIZE, 8,
			EGL_RED_SIZE, 8,
			EGL_ALPHA_SIZE, 8,
			EGL_DEPTH_SIZE, 0,
			EGL_NONE
		};
		EGLConfig configs[8];
		GLint numConfigs = 0;
		eglChooseConfig(display, eglConfigAttribs, &configs[0], 8, &numConfigs);
		// PrintLine_I("Got %d matching config%s", numConfigs, (numConfigs == 1) ? "" : "s");
		if (numConfigs == 0) { PrintLine_E("Failed to find any surface configs matching our requested attributes!"); return false; }
		config = configs[0];
		EGLint eglFormat = 0;
	    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &eglFormat);
	    ANativeWindow_setBuffersGeometry(app->window, 100, 200, eglFormat);
		surface = eglCreateWindowSurface(display, config, app->window, nullptr);
		if (surface == EGL_NO_SURFACE)
		{
			PrintLine_E("Failed to create surface!");
			return false;
		}
		const EGLint eglContextAttribs[] = {
			EGL_CONTEXT_CLIENT_VERSION, 2,
			EGL_NONE
		};
		context = eglCreateContext(display, config, nullptr, eglContextAttribs);
		eglMakeCurrent(display, surface, surface, context);
		
		PrintLine_I("Done initializing!");
		initialized = true;
	}
	
	if (initialized)
	{
		glClearColor(0.5f, (float)(frameIndex%60)/59.0f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		// Draw your "buttons", text, etc. using OpenGL
		// render_button(button_x, button_y, button_width, button_height);
		// render_text("Hello World", text_x, text_y);
		
		eglSwapBuffers(display, surface);
		frameIndex++;
	}
	
	return true;
}

void android_main(struct android_app* app)
{
	WriteLine_I("android_main() is running...");
	//Suppresses link-time dead code removal of stuff in android_native_app_glue.c
	(void)ANativeActivity_onCreate;
	
	//NOTE: app->window is probably nullptr at this point. We need to wait until it becomes filled before initializing GLES
	initialized = false;
	
	// +==============================+
	// |        Main Game Loop        |
	// +==============================+
	bool shouldContinue = true;
	while (shouldContinue)
	{
		// +==============================+
		// |        Event Polling         |
		// +==============================+
		int pollResult = 0;
		int events = 0;
		struct android_poll_source* source = nullptr;
		while ((pollResult = ALooper_pollOnce(0, nullptr, &events, (void**)&source)) >= 0)
		{
			if (source != nullptr) { source->process(app, source); }
			if (app->destroyRequested != 0) { break; }
		}
		if (pollResult == ALOOPER_POLL_ERROR) { WriteLine_E("ALooper returned a POLL_ERROR error!"); }
		if (app->destroyRequested != 0) { WriteLine_E("destroyRequested"); break; }
		
		shouldContinue = draw_frame(app);
		if (!shouldContinue) { WriteLine_E("exiting"); break; }
	}
}


// JNINativeInterface
JNIEXPORT jstring JNICALL Java_com_piggybank_pigcore_tests_MainActivity_MyCFunction(JNIEnv* env, jobject this)
{
	WriteLine_I("Hello Android from MyCFunction() in C!!");
	return (*env)->NewStringUTF(env, "String from MyCFunction()");
}
