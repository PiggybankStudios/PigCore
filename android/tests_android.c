/*
File:   tests_android.c
Author: Taylor Robbins
Date:   09\15\2025
Description: 
	** A simple C program to test compilation to Android
*/

#include "base/base_compiler_check.h"
#if !defined(TARGET_IS_ANDROID) || !TARGET_IS_ANDROID
#error We have misdiagnosed the TARGET as something other than ANDROID
#endif
#if !defined(COMPILER_IS_CLANG) || !COMPILER_IS_CLANG
#error We have misdiagnosed the COMPILER as something other than CLANG
#endif
#include "base/base_defines_check.h"
#include "std/std_includes.h"
#include "base/base_typedefs.h"
#include "base/base_assert.h"
#include "base/base_macros.h"
#include "base/base_math.h"
#include "std/std_memset.h"
#include "std/std_printf.h"
#include "std/std_malloc.h"
#include "struct/struct_string.h"
#include "os/os_virtual_mem.h"
#include "mem/mem_arena.h"
#include "misc/misc_printing.h"
#include "mem/mem_scratch.h"
#include "base/base_char.h"
#include "base/base_debug_output.h"
#include "base/base_debug_output_impl.h"
#include "misc/misc_profiling_tracy_include.h"
// #include "os/os_threading.h"

#include "android_native_app_glue.c"

bool initialized = false;
EGLDisplay display = {0};
EGLConfig config = {0};
EGLSurface surface = {0};
EGLContext context = {0};
int frameIndex = 0;
Arena* stdHeap = nullptr;

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
		Assert(numConfigs > 0);
		config = configs[0];
		EGLint eglFormat = 0;
	    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &eglFormat);
	    ANativeWindow_setBuffersGeometry(app->window, 0, 0, eglFormat);
		surface = eglCreateWindowSurface(display, config, app->window, nullptr);
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
		glClearColor(0.5f, (float)(frameIndex%600)/599.0f, 0.2f, 1.0f);
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
	WriteLine_I("=================== android_main() ===================");
	
	//Suppresses link-time dead code removal of stuff in android_native_app_glue.c
	(void)ANativeActivity_onCreate;
	
	Arena stdHeapStruct = ZEROED;
	WriteLine_I("Initializing stdHeap Arena...");
	InitArenaStdHeap(&stdHeapStruct);
	stdHeap = &stdHeapStruct;
	InitScratchArenasVirtual(Megabytes(64));
	
	{
		ScratchBegin(scratch);
		ScratchBegin1(scratch1, scratch);
		ScratchBegin2(scratch2, scratch, scratch1);
		// PrintLine_I("Scratch: %p %llu/%llu/%llu - Scratch1: %p %llu/%llu/%llu - Scratch2: %p %llu/%llu/%llu",
		// 	scratch, scratch->committed, scratch->used, scratch->size,
		// 	scratch1, scratch1->committed, scratch1->used, scratch1->size,
		// 	scratch2, scratch2->committed, scratch2->used, scratch2->size
		// );
		
		// struct mallinfo info = mallinfo();
		// size_t heap_size = info.uordblks + info.hblkhd;
		// PrintLine_I("heap_size: %zu", heap_size); //Got 17,754,416 from Pixel_3a_API_34 AVD
		
		// void* testAlloc1 = malloc(1024);
		// PrintLine_I("testAlloc1: %p", testAlloc1);
		// void* testAlloc2 = malloc(1024);
		// PrintLine_I("testAlloc2: %p", testAlloc2);
		
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
		
		ScratchEnd(scratch2);
		ScratchEnd(scratch1);
		ScratchEnd(scratch);
	}
	
	//NOTE: app->window is probably nullptr at this point. We need to wait until it becomes filled before initializing GLES
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
			if (source != nullptr) { source->process(app, source); }
			if (app->destroyRequested != 0) { break; }
		}
		if (pollResult == ALOOPER_POLL_ERROR) { WriteLine_E("ALooper returned a POLL_ERROR error!"); }
		if (app->destroyRequested != 0) { WriteLine_E("destroyRequested"); break; }
		
		shouldContinue = draw_frame(app);
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
