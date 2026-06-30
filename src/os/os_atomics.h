/*
File:   os_atomics.h
Author: Taylor Robbins
Date:   01\21\2026
Description:
	** Atomics are basic types that allow reading\writing in a manner that ensures
	** consistency even when read\written from another running thread around the same time.
	** The word "atomic" comes from the idea that the operations on the item cannot be split
	** i.e. the operation cannot be halfway done when another operation on another thread starts.
	** Most of these types\functions here are available since C11. See https://en.cppreference.com/w/c/atomic.html
*/

#ifndef _OS_ATOMICS_H
#define _OS_ATOMICS_H

#include "base/base_compiler_check.h"
#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "std/std_includes.h"

#if TARGET_HAS_ATOMICS
typedef atomic_bool           abool;
typedef atomic_char           achar;
typedef atomic_int            aint;
typedef atomic_uint           auint;
typedef atomic_int_least8_t   ai8;
typedef atomic_uint_least8_t  au8;
typedef atomic_int_least16_t  ai16;
typedef atomic_uint_least16_t au16;
typedef atomic_int_least32_t  ai32;
typedef atomic_uint_least32_t au32;
typedef atomic_int_least64_t  ai64;
typedef atomic_uint_least64_t au64;
#else //!TARGET_HAS_ATOMICS
typedef bool         abool;
typedef char         achar;
typedef int          aint;
typedef unsigned int auint;
typedef i8           ai8;
typedef u8           au8;
typedef i16          ai16;
typedef u16          au16;
typedef i32          ai32;
typedef u32          au32;
typedef i64          ai64;
typedef u64          au64;
#endif //TARGET_HAS_ATOMICS

//TODO: Use these on Windows?: InterlockedIncrement(64), InterlockedDecrement(64), InterlockedAdd(64), InterlockedExchange(8/64), InterlockedCompareExchange(64/128)
#if TARGET_HAS_ATOMICS
	#define AtomicRead(atomicPntr)                                         atomic_load(atomicPntr) //returns atomic value
	#define AtomicWrite(atomicPntr, value)                                 atomic_store((atomicPntr), (value)) //returns nothing
	#define AtomicAdd(atomicPntr, value)                                   atomic_fetch_add((atomicPntr), (value)) //returns atomic value before addition
	#define AtomicSub(atomicPntr, value)                                   atomic_fetch_sub((atomicPntr), (value)) //returns atomic value before subtraction
	#define AtomicIncrement(atomicPntr)                                    AtomicAdd((atomicPntr), 1) //returns atomic value before increment
	#define AtomicDecrement(atomicPntr)                                    AtomicSub((atomicPntr), 1) //returns atomic value before decrement
	#define AtomicExchange(atomicPntr, value)                              atomic_exchange((atomicPntr), (value)) //returns previous atomic value
	#define AtomicCompareExchange(atomicPntr, expectedValuePntr, newValue) atomic_compare_exchange_strong((atomicPntr), (expectedValuePntr), (newValue)) //returns true if successful. expectedValuePntr gets the atomic value before the exchange
#else //!TARGET_HAS_ATOMICS
	#define AtomicRead(atomicPntr)         (*(atomicPntr))
	#define AtomicWrite(atomicPntr, value) *(atomicPntr) = (value)
	#define AtomicAdd(atomicPntr, value)   ((*(atomicPntr) += (value)) - (value))
	#define AtomicSub(atomicPntr, value)   ((*(atomicPntr) -= (value)) + (value))
	#define AtomicIncrement(atomicPntr)    ((*(atomicPntr) += 1) - 1)
	#define AtomicDecrement(atomicPntr)    ((*(atomicPntr) -= 1) + 1)
	#define AtomicExchange(atomicPntr, value) *(atomicPntr); *(atomicPntr) = (value) //WARNING: Multi-piece macro without do { } while(0) wrapper!
	#define AtomicCompareExchange(atomicPntr, expectedValuePntr, newValue) (*(atomicPntr) == *(expectedValuePntr)); do { if (*(atomicPntr) == *(expectedValuePntr)) { *(atomicPntr) = (newValue); } } while(0) //WARNING: Multi-piece macro without do { } while(0) wrapper!
#endif //TARGET_HAS_ATOMICS

#endif //  _OS_ATOMICS_H
