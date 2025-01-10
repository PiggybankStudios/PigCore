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
	jsStdAbort("exit", exitCode);
}

_Noreturn void abort()
{
	jsStdAbort("abort", 0);
}
_Noreturn void abort_msg(const char* message)
{
	jsStdAbort(message, 0);
}
