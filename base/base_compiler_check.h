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

//TODO: Figure out what macro we can use to detect GCC
#define COMPILER_GCC 0

#ifdef __clang__
#define COMPILER_CLANG 1
#else
#define COMPILER_CLANG 0
#endif

//format=VVRR VV=version, RR=Revision, ex. v8.23=0823, LG_Laptop=v19.41.34120
#if defined(_MSC_VER) && !COMPILER_CLANG
#define COMPILER_MSVC 1
#else
#define COMPILER_MSVC 0
#endif

#if COMPILER_MSVC && COMPILER_CLANG
#error Somehow we think are are both in MSVC and CLANG at the same time!
#endif

#endif //  _BASE_COMPILER_CHECK_H
