/*
File:   tests_auto_profile.c
Author: Taylor Robbins
Date:   01\19\2025
Description: 
	** Holds the function entry\exit callbacks that are called when
	** -finstrument-functions is passed to clang
*/

#if (ENABLE_AUTO_PROFILE && COMPILER_IS_CLANG && !TARGET_IS_WASM)

void __cyg_profile_func_enter(void *fn, void *caller)
{
	MyPrint("Entering %p %p...", fn, caller);
}
void __cyg_profile_func_exit(void *fn, void *caller)
{
	MyPrint("Exiting %p %p", fn, caller);
}

#endif //(ENABLE_AUTO_PROFILE && COMPILER_IS_CLANG && !TARGET_IS_WASM)