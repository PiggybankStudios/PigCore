/*
File:   wasm_std_misc.c
Author: Taylor Robbins
Date:   01\10\2025
Description: 
	** Contains a bunch of random functions that don't really need their own dedicated file
*/

int abs(int value)
{
	return ((value > 0) ? value : -value);
}

_Noreturn void exit(int exitCode)
{
	#if TARGET_IS_WEB
	jsStdAbort("exit", exitCode);
	#else
	#error The current TARGET doesn't have an abort implementation!
	#endif
}

_Noreturn void abort()
{
	#if TARGET_IS_WEB
	jsStdAbort("abort", 0);
	#else
	#error The current TARGET doesn't have an abort implementation!
	#endif
}
_Noreturn void abort_msg(const char* message)
{
	#if TARGET_IS_WEB
	jsStdAbort(message, 0);
	#else
	#error The current TARGET doesn't have an abort implementation!
	#endif
}
