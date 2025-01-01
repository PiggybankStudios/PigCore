/*
File:   base_compiler_check.h
Author: Taylor Robbins
Date:   01\01\2025
Description:
	** Checks for various #defines to determine which compiler we are being compiled by.
	** Converts that information into a set of our own defines that are easier to remember\use
*/

#ifndef _BASE_COMPILER_CHECK_H
#define _BASE_COMPILER_CHECK_H

#ifdef _MSC_VER
#define COMPILER_MSVC 1
#else
#define COMPILER_MSVC 0
#endif

#endif //  _BASE_COMPILER_CHECK_H
