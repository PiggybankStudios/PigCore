/*
File:   wasm_std_debugbreak.c
Author: Taylor Robbins
Date:   01\10\2025
*/

void __debugbreak()
{
	#if TARGET_IS_WEB
	jsStdDebugBreak();
	#else
	#error The current TARGET doesn't have a DebugBreak implementation!
	#endif
}
