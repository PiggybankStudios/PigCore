/*
File:   tests_android.c
Author: Taylor Robbins
Date:   09\15\2025
Description: 
	** A simple C program to test compilation to Android
*/

//TODO: Implement os_process_info.h for TARGET_IS_ANDROID
//TODO: Implement os_clipboard.h for TARGET_IS_ANDROID
//TODO: Implement os_file_dialog.h for TARGET_IS_ANDROID
//TODO: Implement os_font.h for TARGET_IS_ANDROID
//TODO: Implement os_http.h for TARGET_IS_ANDROID
//TODO: Implement os_time.h for TARGET_IS_ANDROID

#if TARGET_IS_ANDROID

#include "android_native_app_glue.c"

struct android_app* androidApp = nullptr;
bool initialized = false;
EGLDisplay display = {0};
EGLConfig config = {0};
EGLSurface surface = {0};
EGLContext context = {0};
int frameIndex = 0;
Color32 backgroundColor = {.valueU32=White_Value};

// Example rendering function
bool draw_frame()
{
	NotNull(androidApp);
	if (!initialized && androidApp->window != nullptr)
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
		Assert(numConfigs > 0);
		config = configs[0];
		EGLint eglFormat = 0;
	    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &eglFormat);
	    ANativeWindow_setBuffersGeometry(androidApp->window, 0, 0, eglFormat);
		surface = eglCreateWindowSurface(display, config, androidApp->window, nullptr);
		Assert(surface != EGL_NO_SURFACE);
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
		backgroundColor.r = (u8)ClampI32((i32)backgroundColor.r + GetRandI32Range(mainRandom, -1, 2), 0, 255);
		backgroundColor.g = (u8)ClampI32((i32)backgroundColor.g + GetRandI32Range(mainRandom, -1, 2), 0, 255);
		backgroundColor.b = (u8)ClampI32((i32)backgroundColor.b + GetRandI32Range(mainRandom, -1, 2), 0, 255);
		
		glClearColor(backgroundColor.r/255.0f, backgroundColor.g/255.0f, backgroundColor.b/255.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		// Draw your "buttons", text, etc. using OpenGL
		// render_button(button_x, button_y, button_width, button_height);
		// render_text("Hello World", text_x, text_y);
		
		eglSwapBuffers(display, surface);
		frameIndex++;
	}
	
	return true;
}

void DoAndroidTests()
{
	NotNull(androidApp);
	//Suppresses link-time dead code removal of stuff in android_native_app_glue.c
	(void)ANativeActivity_onCreate;
	
	// +==============================+
	// |          Initialize          |
	// +==============================+
	{
		ScratchBegin(scratch);
		ScratchBegin1(scratch1, scratch);
		ScratchBegin2(scratch2, scratch, scratch1);
		// PrintLine_I("Scratch: %p %llu/%llu/%llu - Scratch1: %p %llu/%llu/%llu - Scratch2: %p %llu/%llu/%llu",
		// 	scratch, scratch->committed, scratch->used, scratch->size,
		// 	scratch1, scratch1->committed, scratch1->used, scratch1->size,
		// 	scratch2, scratch2->committed, scratch2->used, scratch2->size
		// );
		
		FilePath settingsSavePath = OsGetSettingsSavePath(scratch, Str8_Empty, Str8_Empty, true);
		PrintLine_W("settingsSavePath: \"%.*s\"", StrPrint(settingsSavePath));
		
		// struct mallinfo info = mallinfo();
		// size_t heap_size = info.uordblks + info.hblkhd;
		// PrintLine_I("heap_size: %zu", heap_size); //Got 17,754,416 from Pixel_3a_API_34 AVD
		
		// void* testAlloc1 = malloc(1024);
		// PrintLine_I("testAlloc1: %p", testAlloc1);
		// void* testAlloc2 = malloc(1024);
		// PrintLine_I("testAlloc2: %p", testAlloc2);
		
		#if 0
		{
			PrintLine_I("androidApp->userData:           %p", androidApp->userData); //nullptr
			PrintLine_I("androidApp->onAppCmd:           %p", androidApp->onAppCmd); //nullptr
			PrintLine_I("androidApp->onInputEvent:       %p", androidApp->onInputEvent); //nullptr
			PrintLine_I("androidApp->activity:           %p", androidApp->activity); //0x7deaec762990
			PrintLine_I("androidApp->config:             %p", androidApp->config); //0x7deb2c773dc0
			PrintLine_I("androidApp->savedState:         %p", androidApp->savedState); //nullptr
			PrintLine_I("androidApp->savedStateSize:     %zu", androidApp->savedStateSize); //0
			PrintLine_I("androidApp->looper:             %p", androidApp->looper); //0x7dec4c757490
			PrintLine_I("androidApp->inputQueue:         %p", androidApp->inputQueue); //nullptr
			PrintLine_I("androidApp->window:             %p", androidApp->window); //nullptr
			PrintLine_I("androidApp->contentRect:        (%d, %d, %d, %d)", androidApp->contentRect.left, androidApp->contentRect.top, androidApp->contentRect.right, androidApp->contentRect.bottom); //(0, 0, 0, 0)
			PrintLine_I("androidApp->activityState:      %d", androidApp->activityState); //0
			PrintLine_I("androidApp->destroyRequested:   %d", androidApp->destroyRequested); //0
			PrintLine_I("androidApp->msgread:            %d", androidApp->msgread); //75
			PrintLine_I("androidApp->msgwrite:           %d", androidApp->msgwrite); //76
			PrintLine_I("androidApp->thread:             %d", androidApp->thread); //537992432
			PrintLine_I("androidApp->running:            %d", androidApp->running); //1
			PrintLine_I("androidApp->stateSaved:         %d", androidApp->stateSaved); //0
			PrintLine_I("androidApp->destroyed:          %d", androidApp->destroyed); //0
			PrintLine_I("androidApp->redrawNeeded:       %d", androidApp->redrawNeeded); //0
			PrintLine_I("androidApp->pendingInputQueue:  %p", androidApp->pendingInputQueue); //nullptr
			PrintLine_I("androidApp->pendingWindow:      %p", androidApp->pendingWindow); //nullptr
			PrintLine_I("androidApp->pendingContentRect: (%d, %d, %d, %d)", androidApp->pendingContentRect.left, androidApp->pendingContentRect.top, androidApp->pendingContentRect.right, androidApp->pendingContentRect.bottom); //(0, 0, 0, 0)
			// PrintLine_I("androidApp->mutex: %p", androidApp->mutex); //pthread_mutex_t
			// PrintLine_I("androidApp->cond: %p", androidApp->cond); //pthread_cond_t
			// PrintLine_I("androidApp->cmdPollSource: %p", androidApp->cmdPollSource); //struct android_poll_source
			// PrintLine_I("androidApp->inputPollSource: %p", androidApp->inputPollSource); //struct android_poll_source
		}
		#endif
		
		#if 0
		{
			JNIEnv* env = nullptr;
			(*androidApp->activity->vm)->AttachCurrentThread(androidApp->activity->vm, &env, NULL);
			PrintLine_I("env: %p", env);
			
			jclass activityClass = (*env)->GetObjectClass(env, androidApp->activity->clazz);
			jmethodID getFilesDirMethod = (*env)->GetMethodID(env, activityClass, "getFilesDir", "()Ljava/io/File;");
			NotNull(getFilesDirMethod);
			
			jobject fileObj = (*env)->CallObjectMethod(env, androidApp->activity->clazz, getFilesDirMethod);
			jclass fileClass = (*env)->GetObjectClass(env, fileObj);
			jmethodID getAbsolutePathMethod = (*env)->GetMethodID(env, fileClass, "getAbsolutePath", "()Ljava/lang/String;");
			NotNull(getAbsolutePathMethod);
			
			jstring pathString = (*env)->CallObjectMethod(env, fileObj, getAbsolutePathMethod);
			const char* pathStringNt = (*env)->GetStringUTFChars(env, pathString, NULL);
			NotNull(pathStringNt);
			Str8 pathStr = AllocStr8Nt(scratch, pathStringNt);
			
			(*androidApp->activity->vm)->DetachCurrentThread(androidApp->activity->vm);
			
			PrintLine_I("Application Internal Storage Path: \"%.*s\"", StrPrint(pathStr));
		}
		#endif
		
		#if 0
		{
			PrintLine_I("sizeof(char) = %zu", sizeof(char)); //1
			PrintLine_I("sizeof(int) = %zu", sizeof(int)); //4
			PrintLine_I("sizeof(long) = %zu", sizeof(long)); //8
			PrintLine_I("sizeof(long long) = %zu", sizeof(long long)); //8
			PrintLine_I("sizeof(unsigned int) = %zu", sizeof(unsigned int)); //4
			PrintLine_I("sizeof(unsigned long) = %zu", sizeof(unsigned long)); //8
			PrintLine_I("sizeof(unsigned long long) = %zu", sizeof(unsigned long long)); //8
			PrintLine_I("sizeof(float) = %zu", sizeof(float)); //4
			PrintLine_I("sizeof(double) = %zu", sizeof(double)); //8
			PrintLine_I("sizeof(u8)  = %zu", sizeof(u8));  //1
			PrintLine_I("sizeof(u16) = %zu", sizeof(u16)); //2
			PrintLine_I("sizeof(u32) = %zu", sizeof(u32)); //4
			PrintLine_I("sizeof(u64) = %zu", sizeof(u64)); //8
			PrintLine_I("sizeof(i8)  = %zu", sizeof(i8));  //1
			PrintLine_I("sizeof(i16) = %zu", sizeof(i16)); //2
			PrintLine_I("sizeof(i32) = %zu", sizeof(i32)); //4
			PrintLine_I("sizeof(i64) = %zu", sizeof(i64)); //8
			PrintLine_I("sizeof(uxx) = %zu", sizeof(uxx)); //8
			PrintLine_I("sizeof(ixx) = %zu", sizeof(ixx)); //8
			PrintLine_I("sizeof(rxx) = %zu", sizeof(rxx)); //8
			PrintLine_I("sizeof(r32) = %zu", sizeof(r32)); //4
			PrintLine_I("sizeof(r64) = %zu", sizeof(r64)); //8
			PrintLine_I("sizeof(bool) = %zu", sizeof(bool)); //1
			PrintLine_I("sizeof(void* = %zu", sizeof(void*)); //8
			PrintLine_I("sizeof(size_t) = %zu", sizeof(size_t)); //8
			PrintLine_I("sizeof(ptrdiff_t) = %zu", sizeof(ptrdiff_t)); //8
			PrintLine_I("sizeof(char16_t) = %zu", sizeof(char16_t)); //2
		}
		#endif
		
		#if 0
		{
			// uxx pageSize = OsGetMemoryPageSize();
			// PrintLine_I("pageSize: %llu", pageSize); //4096
			const uxx reserveSize = Kilobytes(256);
			void* reservePntr = OsReserveMemory(reserveSize);
			PrintLine_I("Reserved %llu bytes: %p", reserveSize, reservePntr);
			for (uxx byteIndex = 0; byteIndex < reserveSize; byteIndex += 4096)
			{
				PrintLine_I("Committing +%llu...", byteIndex);
				OsCommitReservedMemory((u8*)reservePntr + byteIndex, 4096);
				*((u8*)reservePntr + byteIndex) = 0xCA;
			}
			for (uxx byteIndex = reserveSize; byteIndex > 0; byteIndex -= 4096)
			{
				PrintLine_I("Decommitting +%llu...", byteIndex-4096);
				OsDecommitReservedMemory((u8*)reservePntr + (byteIndex-4096), 4096);
			}
			OsFreeReservedMemory(reservePntr, reserveSize);
		}
		#endif
		
		#if 0
		{
			PrintLine_I("stdHeap %llu/%llu", stdHeap->used, stdHeap->size);
			u8* alloc1 = (u8*)AllocMem(stdHeap, 1024);
			PrintLine_I("alloc1: %p", alloc1);
			u8* alloc2 = (u8*)AllocMem(stdHeap, sizeof(r32));
			PrintLine_I("alloc2: %p", alloc2);
			u8* alloc3 = (u8*)AllocMem(stdHeap, Kilobytes(2));
			PrintLine_I("alloc3: %p", alloc3);
			PrintLine_I("stdHeap %llu/%llu", stdHeap->used, stdHeap->size);
			FreeMem(stdHeap, alloc2, sizeof(r32));
			PrintLine_I("stdHeap %llu/%llu", stdHeap->used, stdHeap->size);
			FreeMem(stdHeap, alloc3, Kilobytes(2));
			PrintLine_I("stdHeap %llu/%llu", stdHeap->used, stdHeap->size);
			FreeMem(stdHeap, alloc1, 1024);
			PrintLine_I("stdHeap %llu/%llu", stdHeap->used, stdHeap->size);
		}
		#endif
		
		#if 0
		{
			char contentsBuffer[4096];
			FILE* memFile = fopen("/proc/meminfo", "r");
			size_t readResult = fread(
				contentsBuffer, //buffer
				1, //size
				(size_t)ArrayCount(contentsBuffer), //count
				memFile //stream
			);
			Assert(readResult < ArrayCount(contentsBuffer));
			readResult += fread(&contentsBuffer[readResult], 1, (size_t)ArrayCount(contentsBuffer) - readResult, memFile);
			Assert(readResult < ArrayCount(contentsBuffer));
			contentsBuffer[readResult] = '\0';
			PrintLine_D("memFile[%zu]: %s", readResult, contentsBuffer);
			//File Contents:
			// MemTotal:        2021088 kB
			// MemFree:          106832 kB
			// MemAvailable:     859548 kB
			// Buffers:           13172 kB
			// Cached:           938160 kB
			// SwapCached:        11796 kB
			// Active:           800820 kB
			// Inactive:         635440 kB
			// Active(anon):     375064 kB
			// Inactive(anon):   129020 kB
			// Active(file):     425756 kB
			// Inactive(file):   506420 kB
			// Unevictable:        4104 kB
			// Mlocked:            4104 kB
			// SwapTotal:       1515812 kB
			// SwapFree:        1048432 kB
			// Dirty:               412 kB
			// Writeback:            16 kB
			// AnonPages:        486084 kB
			// Mapped:           658464 kB
			// Shmem:             16020 kB
			// KReclaimable:      57752 kB
			// Slab:             162980 kB
			// SReclaimable:      57752 kB
			// SUnreclaim:       105228 kB
			// KernelStack:       30464 kB
			// PageTables:        60140 kB
			// SecPageTables:         0 kB
			// NFS_Unstable:          0 kB
			// Bounce:                0 kB
			// WritebackTmp:          0 kB
			// CommitLimit:     2526356 kB
			// Committed_AS:   31021896 kB
			// VmallocTotal:   34359738367 kB
			// VmallocUsed:       51320 kB
			// VmallocChunk:          0
		}
		#endif
		
		#if 0
		{
			const char* filePathNt = "/data/user/0/" ANDROID_PACKAGE_PATH_STR "/files/test.txt";
			FilePath filePath = FilePathLit(filePathNt);
			FilePath fullPath = OsGetFullPath(scratch, filePath);
			PrintLine_I("fullPath: \"%.*s\"", StrPrint(fullPath));
			
			#if 0
			Str8 fileContents = StrLit("Hello Android file system!\nThis is a nice file!\n\nHello");
			bool writeResult = OsWriteFile(filePath, fileContents, true);
			if (!writeResult) { PrintLine_E("Failed to write \"%s\"", filePathNt); }
			#else
			Str8 fileContents = Str8_Empty;
			bool readResult = OsReadTextFile(filePath, scratch, &fileContents);
			if (readResult)
			{
				PrintLine_I("Read %llu bytes from file: \"%.*s\"", fileContents.length, StrPrint(fileContents));
			}
			else { PrintLine_E("Failed to read file at \"%.*s\"", StrPrint(filePath)); }
			#endif
		}
		#endif
		
		ScratchEnd(scratch2);
		ScratchEnd(scratch1);
		ScratchEnd(scratch);
	}
	
	//NOTE: androidApp->window is probably nullptr at this point. We need to wait until it becomes filled before initializing GLES
	initialized = false;
	
	// +==============================+
	// |        Main Game Loop        |
	// +==============================+
	bool shouldContinue = true;
	while (shouldContinue)
	{
		ScratchBegin(scratch);
		ScratchBegin1(scratch1, scratch);
		ScratchBegin2(scratch2, scratch, scratch1);
		
		// +==============================+
		// |        Event Polling         |
		// +==============================+
		int pollResult = 0;
		int events = 0;
		struct android_poll_source* source = nullptr;
		while ((pollResult = ALooper_pollOnce(0, nullptr, &events, (void**)&source)) >= 0)
		{
			if (source != nullptr) { source->process(androidApp, source); }
			if (androidApp->destroyRequested != 0) { break; }
		}
		if (pollResult == ALOOPER_POLL_ERROR) { WriteLine_E("ALooper returned a POLL_ERROR error!"); }
		if (androidApp->destroyRequested != 0) { WriteLine_E("destroyRequested"); break; }
		
		shouldContinue = draw_frame();
		if (!shouldContinue) { WriteLine_E("exiting"); }
			
		ScratchEnd(scratch2);
		ScratchEnd(scratch1);
		ScratchEnd(scratch);
	}
}

// JNINativeInterface
JNIEXPORT jstring JNICALL Java_com_piggybank_pigcore_tests_MainActivity_MyCFunction(JNIEnv* env, jobject this)
{
	WriteLine_I("Hello Android from MyCFunction() in C!!");
	return (*env)->NewStringUTF(env, "String from MyCFunction()");
}

#endif //TARGET_IS_ANDROID
