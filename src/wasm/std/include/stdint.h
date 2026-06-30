/*
File:   stdint.h
Author: Taylor Robbins
Date:   01\10\2025
NOTE: This file cannot depend on anything but wasm_std_common.h
*/

#ifndef _STDINT_H
#define _STDINT_H

#include <internal/wasm_std_common.h>

// +--------------------------------------------------------------+
// |                           Typedefs                           |
// +--------------------------------------------------------------+
#define _Addr long
#define _Int64 long long
#define _Reg long
#define __BYTE_ORDER __LITTLE_ENDIAN
#define __LONG_MAX 0x7FFFFFFFFFFFFFFFL
#if LANGUAGE_C
#ifdef __WCHAR_TYPE__
typedef __WCHAR_TYPE__ wchar_t;
#else
typedef long wchar_t;
#endif
#endif

typedef float  float_t;
typedef double double_t;

typedef unsigned _Addr size_t;
typedef unsigned _Addr uintptr_t;
typedef _Addr          ptrdiff_t;
typedef _Addr          ssize_t;
typedef _Addr          intptr_t;
// typedef _Addr          regoff_t;
// typedef _Reg           register_t;
// typedef _Int64         time_t;
// typedef _Int64         suseconds_t;

typedef signed char     int8_t;
typedef signed short    int16_t;
typedef signed int      int32_t;
typedef signed _Int64   int64_t;
typedef signed _Int64   intmax_t;
typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned int    uint32_t;
typedef unsigned _Int64 uint64_t;
typedef unsigned _Int64 u_int64_t;
typedef unsigned _Int64 uintmax_t;

// typedef unsigned        mode_t;
// typedef unsigned _Reg   nlink_t;
// typedef _Int64          off_t;
// typedef unsigned _Int64 ino_t;
// typedef unsigned _Int64 dev_t;
// typedef long            blksize_t;
// typedef _Int64          blkcnt_t;
// typedef unsigned _Int64 fsblkcnt_t;
// typedef unsigned _Int64 fsfilcnt_t;

// typedef unsigned      wint_t;
// typedef unsigned long wctype_t;

// typedef void *  timer_t;
// typedef int     clockid_t;
// typedef long    clock_t;
// struct timeval  { time_t tv_sec; suseconds_t tv_usec; };
// struct timespec { time_t tv_sec; int :8*(sizeof(time_t)-sizeof(long))*(__BYTE_ORDER==__BIG_ENDIAN); long tv_nsec; int :8*(sizeof(time_t)-sizeof(long))*(__BYTE_ORDER!=__BIG_ENDIAN); };

// typedef int      pid_t;
// typedef unsigned id_t;
// typedef unsigned uid_t;
// typedef unsigned gid_t;
// typedef int      key_t;
// typedef unsigned useconds_t;

// +--------------------------------------------------------------+
// |                     MAX and MIN #defines                     |
// +--------------------------------------------------------------+
#define INT8_MIN   (-1-0x7f)
#define INT16_MIN  (-1-0x7fff)
#define INT32_MIN  (-1-0x7fffffff)
#define INT64_MIN  (-1-0x7fffffffffffffff)

#define INT8_MAX   (0x7f)
#define INT16_MAX  (0x7fff)
#define INT32_MAX  (0x7fffffff)
#define INT64_MAX  (0x7fffffffffffffff)

#define UINT8_MAX  (0xff)
#define UINT16_MAX (0xffff)
#define UINT32_MAX (0xffffffffu)
#define UINT64_MAX (0xffffffffffffffffu)

// #define INT_FAST8_MIN   INT8_MIN
// #define INT_FAST64_MIN  INT64_MIN

// #define INT_LEAST8_MIN   INT8_MIN
// #define INT_LEAST16_MIN  INT16_MIN
// #define INT_LEAST32_MIN  INT32_MIN
// #define INT_LEAST64_MIN  INT64_MIN

// #define INT_FAST8_MAX   INT8_MAX
// #define INT_FAST64_MAX  INT64_MAX

// #define INT_LEAST8_MAX   INT8_MAX
// #define INT_LEAST16_MAX  INT16_MAX
// #define INT_LEAST32_MAX  INT32_MAX
// #define INT_LEAST64_MAX  INT64_MAX

// #define UINT_FAST8_MAX  UINT8_MAX
// #define UINT_FAST64_MAX UINT64_MAX

// #define UINT_LEAST8_MAX  UINT8_MAX
// #define UINT_LEAST16_MAX UINT16_MAX
// #define UINT_LEAST32_MAX UINT32_MAX
// #define UINT_LEAST64_MAX UINT64_MAX

#define INTMAX_MIN  INT64_MIN
#define INTMAX_MAX  INT64_MAX
#define UINTMAX_MAX UINT64_MAX

// #define WINT_MIN 0U
// #define WINT_MAX UINT32_MAX

#define WCHAR_MAX (0x7FFFFFFF+L'\0')
#define WCHAR_MIN (-1-0x7FFFFFFF+L'\0')

#define SIG_ATOMIC_MIN  INT32_MIN
#define SIG_ATOMIC_MAX  INT32_MAX

#endif //  _STDINT_H
