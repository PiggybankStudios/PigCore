/*
File:   std_math_ex.h
Author: Taylor Robbins
Date:   01\03\2025
Description:
	** This file contains mathematical functions in the style of something that may be
	** provided by the standard library but is not
*/

#ifndef _STD_MATH_EX_H
#define _STD_MATH_EX_H

#include "base/base_compiler_check.h"
#include "base/base_typedefs.h"

i32 CeilDivI32(i32 dividend, i32 divisor)
{
	if ((dividend % divisor) == 0) { return dividend / divisor; }
	else { return (dividend / divisor) + 1; }
}
i64 CeilDivI64(i64 dividend, i64 divisor)
{
	if ((dividend % divisor) == 0) { return dividend / divisor; }
	else { return (dividend / divisor) + 1; }
}
ixx CeilDivIXX(ixx dividend, ixx divisor)
{
	#if TARGET_IS_32BIT
	return CeilDivI32(dividend, divisor);
	#else
	return CeilDivI64(dividend, divisor);
	#endif
}
u32 CeilDivU32(u32 dividend, u32 divisor)
{
	if ((dividend % divisor) == 0) { return dividend / divisor; }
	else { return (dividend / divisor) + 1; }
}
u64 CeilDivU64(u64 dividend, u64 divisor)
{
	if ((dividend % divisor) == 0) { return dividend / divisor; }
	else { return (dividend / divisor) + 1; }
}
uxx CeilDivUXX(uxx dividend, uxx divisor)
{
	#if TARGET_IS_32BIT
	return CeilDivU32(dividend, divisor);
	#else
	return CeilDivU64(dividend, divisor);
	#endif
}


#endif //  _STD_MATH_EX_H
