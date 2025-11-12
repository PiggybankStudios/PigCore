/*
File:   base_macros.h
Author: Taylor Robbins
Date:   01\01\2025
Description:
	** Contains all the basic macros that we use while working in C on any project
*/

#ifndef _BASE_MACROS_H
#define _BASE_MACROS_H

#include "base/base_compiler_check.h"
#include "base/base_typedefs.h"

// +--------------------------------------------------------------+
// |                       Global Constants                       |
// +--------------------------------------------------------------+
//Actual Value of Pi:      3.1415926535897932384626433832795...
#define Pi64               3.14159265358979311599796346854      //accurate to 15 digits
#define Pi32               3.1415927410125732421875f            //accurate to 6 digits
#define QuarterPi64        (Pi64/4.0)
#define ThirdPi64          (Pi64/3.0)
#define HalfPi64           (Pi64/2.0)
#define ThreeHalfsPi64     (Pi64*(3.0/2.0))
#define TwoPi64            (2*Pi64)
#define QuarterPi32        (Pi32/4.0f)
#define ThirdPi32          (Pi32/3.0f)
#define HalfPi32           (Pi32/2.0f)
#define ThreeHalfsPi32     (Pi32*(3.0f/2.0f))
#define TwoPi32            (2*Pi32)
//Actual Value of e:       2.7182818284590452353602874713526...
#define e64                2.71828182845904509079559829843      //accurate to 15 digits
#define e32                2.71828174591064453125f              //accurate to 6 digits
//Actual Value of sqrt(2): 1.4142135623730950488016887242096...
#define Sqrt2_64           1.41421356237309514547462185874      //accurate to 15 digits
#define Sqrt2_32           1.41421353816986083984375f           //accurate to 7 digits
#define HugeR32            3.40282346638528859812e+38F          //aka FLT_MAX or __FLT_MAX__, the largest finite float that can be represented by a 32-bit float
#define NegativeHugeR32    1.17549435082228750797e-38F          //the smallest (as in most negative) float that can be represented by a 32-bit float

// Used mostly by enum to string conversion functions when passed a value that is not valid in the enumeration
#define UNKNOWN_STR  "Unknown"

// +--------------------------------------------------------------+
// |               Language Feature Related Macros                |
// +--------------------------------------------------------------+
// Indicates the return value of a function should not be discarded (not assigned to a variable)
// NOTE: This is part of C23 spec and seems to be supported by most recent MSVC compiler (as well as GCC and Clang of course)
#define NODISCARD [[nodiscard]]

// Used to specific an initial value for a structure that is entirely cleared to 0
// NOTE: The 0 is required when compiling in C mode in MSVC compiler since it doesn't support C23 empty {} syntax
#if LANGUAGE_IS_C
#define ZEROED {0}
#else
#define ZEROED {}
#endif

#if LANGUAGE_IS_C
#define NEW_STRUCT(type) (type)
#else
#define NEW_STRUCT(type) type
#endif

//TODO: Confirm that this works on Windows, OSX, and Linux with Clang, GCC, and MSVC compilers
#if !TARGET_HAS_THREADING
#define THREAD_LOCAL //nothing
#elif TARGET_IS_OSX
#define THREAD_LOCAL __thread
#else
#define THREAD_LOCAL thread_local
#endif

// +--------------------------------------------------------------+
// |                     Function Like Macros                     |
// +--------------------------------------------------------------+
// Version numbers may be held in a variety of structs with various sizes for their major/minor and optionally build numbers
// These macros simply make it easy to do < or > like comparisons when two numbers are paired together,
// one being the "primary" number and the other only being significant if the primary numbers are equal
#define IsVersionBelow(versionMajor, versionMinor, numberMajor, numberMinor) (((versionMajor) < (numberMajor)) || ((versionMajor) == (numberMajor) && (versionMinor) < (numberMinor)))
#define IsVersionAbove(versionMajor, versionMinor, numberMajor, numberMinor) (((versionMajor) > (numberMajor)) || ((versionMajor) == (numberMajor) && (versionMinor) > (numberMinor)))

// Either v1 == c1 and v2 == c2 OR v2 = c1 and v1 == c2
#define IsEqualXor(variable1, variable2, condition1, condition2) (((variable1) == (condition1) && (variable2) == (condition2)) || ((variable1) == (condition2) && (variable2) == (condition1)))

#define ArrayCount(array) (sizeof(array) / sizeof((array)[0]))
// Similar to ArrayCount, but used on String Literals who's length includes the null-terminating character
#define StrLitLength(strLit) ((sizeof(strLit) / sizeof((strLit)[0])) - sizeof((strLit)[0]))

// Macros used to check or modify specific bit(s) in a field, the pattern of using individual bits as booleans is often referred to as "flags"
#define IsFlagSet(BitwiseField, Bit) ((Bit) != 0 && ((BitwiseField) & (Bit)) == (Bit))
#define FlagSet(BitwiseField, Bit)   (BitwiseField) |= (Bit)
#define FlagUnset(BitwiseField, Bit) (BitwiseField) &= ~(Bit)
#define FlagToggle(BitwiseField, Bit) ((BitwiseField) ^= (Bit))
#define FlagSetTo(BitwiseField, Bit, condition) if (condition) { FlagSet((BitwiseField), (Bit)); } else { FlagUnset((BitwiseField), (Bit)); }

// Similar to Flag macros above but satiates compiler warnings when using an enum type BitwiseField by doing C-style casts on the bit and the result
#define FlagEnumSet(BitwiseField, Bit, enumType, castType)   (BitwiseField) = (enumType)(((castType)(BitwiseField)) | (castType)(Bit))
#define FlagEnumUnset(BitwiseField, Bit, enumType, castType) (BitwiseField) = (enumType)(((castType)(BitwiseField)) & ~((castType)(Bit)))
#define FlagEnumToggle(BitwiseField, Bit, enumType, castType) (BitwiseField) = (enumType)(((castType)(BitwiseField)) ^ ((castType)(Bit)))
#define FlagEnumSetTo(BitwiseField, Bit, condition, enumType, castType) if (condition) { FlagEnumSet((BitwiseField), (Bit), (enumType), (castType)); } else { FlagEnumUnset((BitwiseField), (Bit), (enumType), (castType)); }

// Metric System (SI) prefixes
#define Tera(value)  ((value) * 1000000000000ULL)
#define Giga(value)  ((value) * 1000000000ULL)
#define Mega(value)  ((value) * 1000000ULL)
#define Kilo(value)  ((value) * 1000ULL)
#define Hecto(value) ((value) * 100ULL)
#define Deca(value)  ((value) * 10ULL)
// No Prefix = 1x
#define Deci(value)  ((value) * 0.1)
#define Centi(value) ((value) * 0.01)
#define Milli(value) ((value) * 0.001)
#define Micro(value) ((value) * 0.000001)
#define Nano(value)  ((value) * 0.000000001)
#define Pico(value)  ((value) * 0.000000000001)

// English names for orders of magnitude
#define Trillion(value)   Tera(value)
#define Billion(value)    Giga(value)
#define Million(value)    Mega(value)
#define Thousand(value)   Kilo(value)
#define Hundred(value)    Hecto(value)
#define Tenth(value)      Deci(value)
#define Hundredth(value)  Centi(value)
#define Thousandth(value) Milli(value)
#define Millionth(value)  Micro(value)
#define Billionth(value)  Nano(value)
#define Trillionth(value) Pico(value)

// Shorthand for writing things like (4 * 1024 * 1024) as Megabytes(4).
// Can be used for more than just memory sizes but these powers of 1024 are often
// used when partitioning memory because they relate to binary bit patterns
#define Kilobytes(value) ((value) * 1024ULL)
#define Megabytes(value) (Kilobytes(value) * 1024ULL)
#define Gigabytes(value) (Megabytes(value) * 1024ULL)
#define Terabytes(value) (Gigabytes(value) * 1024ULL)

// Convert between radians and degrees, either with Pi32 or Pi64
#define ToRadians32(degrees)		((degrees)/180.0f * Pi32)
#define ToRadians64(degrees)		((degrees)/180.0 * Pi64)
#define ToDegrees32(radians)		((radians)/Pi32 * 180.0f)
#define ToDegrees64(radians)		((radians)/Pi64 * 180.0)

// Converts all 3 pointers to u8* and does pointer arithmetic to determine if pntr is >= regionStart and < (regionStart + regionSize)
#define IsPntrWithin(regionStart, regionSize, pntr) (((u8*)(pntr)) >= ((u8*)(regionStart)) && ((u8*)(pntr)) <= (((u8*)(regionStart)) + (regionSize)))
#define IsSizedPntrWithin(regionStart, regionSize, pntr, size) (((u8*)(pntr)) >= ((u8*)(regionStart)) && (((u8*)(pntr)) + (size)) <= (((u8*)(regionStart)) + (regionSize)))

// If outputPntr is not nullptr then assign it a given value.
// Mostly used when an optional output parameter is being supplied to a function
#define SetOptionalOutPntr(outputPntr, value) if ((outputPntr) != nullptr) { *(outputPntr) = (value); }

// Used when doing debug logs in english, a particular number might have sway on the plurality of a following word
// For example we say "1 bird" and "3 birds", and also "0 birds". So basically use a suffix (like "s") when the number != 1
#define PluralEx(number, singularSuffix, multipleSuffix) (((number) == 1) ? (singularSuffix) : (multipleSuffix))
#define Plural(number, multipleSuffix) (((number) == 1) ? "" : (multipleSuffix))

// Meant to be used in a loop where we are finding the max/min value but we want to
// accept the first value regardless of the current value in the trackVariable
#define TrackMax(isFirst, trackVariable, newValue) do { if ((isFirst) || (trackVariable) < (newValue)) { (trackVariable) = (newValue); } } while(0)
#define TrackMin(isFirst, trackVariable, newValue) do { if ((isFirst) || (trackVariable) > (newValue)) { (trackVariable) = (newValue); } } while(0)

#define IsAlignedTo(pntr, alignment) ((alignment) == 0 || (((size_t)(pntr)) % (alignment)) == 0)
#define AlignOffset(pntr, alignment) ((alignment) == 0 ? 0 : (((alignment) - (((size_t)(pntr)) % (alignment))) % alignment))

#ifndef STRINGIFY_DEFINE
#define STRINGIFY_DEFINE(define) STRINGIFY(define)
#endif
#ifndef STRINGIFY
#define STRINGIFY(text)          #text
#endif

//This is used to make sure a macro parameters is a string literal, not just a pointer, by checking to see if it implicitly concatenates with empty strings
#define CheckStrLit(stringLiteral) ("" stringLiteral "")

#define INDEX_FROM_COORD2D(coordX, coordY, arrayWidth, arrayHeight) ( \
	(coordY) * (arrayWidth) +                                         \
	(coordX)                                                          \
)
#define COORD2D_X_FROM_INDEX(index, arrayWidth, arrayHeight) ((index) % (arrayWidth))
#define COORD2D_Y_FROM_INDEX(index, arrayWidth, arrayHeight) ((index) / (arrayWidth))
//NOTE: See struct_vectors.h for COORD2D_FROM_INDEX which produces a v2i

// When dealing with 3D coordinate systems we often will put a collection of items that are logically
// part of a 3D grid into a 1-dimensional array. To convert between the 3D coordinates and the index
// in the array we use these macros which ensure the ordering x/y/z axis is consistent
#define INDEX_FROM_COORD3D(coordX, coordY, coordZ, arrayWidth, arrayHeight, arrayDepth) ( \
	(coordY) * ((arrayWidth) * (arrayDepth)) +                                            \
	(coordZ) * (arrayWidth) +                                                             \
	(coordX)                                                                              \
)
#define COORD3D_X_FROM_INDEX(voxelIndex, arrayWidth) ((voxelIndex) % (arrayWidth))
#define COORD3D_Y_FROM_INDEX(voxelIndex, arrayWidth, arrayDepth) ((voxelIndex) / ((arrayWidth) * (arrayDepth)))
#define COORD3D_Z_FROM_INDEX(voxelIndex, arrayWidth, arrayHeight, arrayDepth) (((voxelIndex) % ((arrayWidth) * (arrayDepth))) / (arrayWidth))
//NOTE: See struct_vectors.h for COORD3D_FROM_INDEX which produces a v3i

// When unused variable warnings are enabled, these macros can help satiate the
// warning for a particular variable but have no actual affect on the variable.
// NOTE: This does not ensure the variable is not in use, it only satiates the warning
#define UNUSED(varName)        (void)(varName)
#if DEBUG_BUILD
#define DEBUG_USED(varName)    //nothing
#else
#define DEBUG_USED(varName)    (void)(varName)
#endif

// Often in a program we will have a running integer counter (say ms) that we will take copies of
// and then compare later to see how long it's been since we took that copy. It's a simple
// mathematically operation but making a macro makes it way more readable what we are doing
// NOTE: A version without the word "By" will be defined later when the current programTime source can be inferred
#define TimeSinceBy(programTime, programTimeSnapshot) ((programTimeSnapshot <= programTime) ? (programTime - programTimeSnapshot) : 0)

// These macros increment various integer types ONLY if they would not overflow when incremented (same with decrement and preventing underflowing for unsigned types)
#define Increment(variable, max)           if ((variable) < (max)) { (variable)++; } else { (variable) = (max); }
#define IncrementU8(variable)              if ((variable) < 0xFF) { (variable)++; } else { (variable) = 0xFF; }
#define IncrementU16(variable)             if ((variable) < 0xFFFF) { (variable)++; } else { (variable) = 0xFFFF; }
#define IncrementU32(variable)             if ((variable) < 0xFFFFFFFFUL) { (variable)++; } else { (variable) = 0xFFFFFFFFUL; }
#define IncrementU64(variable)             if ((variable) < 0xFFFFFFFFFFFFFFFFULL) { (variable)++; } else { (variable) = 0xFFFFFFFFFFFFFFFFULL; }
#if TARGET_IS_32BIT
#define IncrementUXX(variable)             IncrementU32(variable)
#else
#define IncrementUXX(variable)             IncrementU64(variable)
#endif
#define IncrementBy(variable, amount, max) if ((variable) + (amount) < (max) && (variable) + (amount) > (variable)) { (variable) += (amount); } else { (variable) = (max); }
#define IncrementU8By(variable, amount)    if ((variable) + (amount) < 0xFF && (variable) + (amount) > (variable)) { (variable) += (amount); } else { (variable) = 0xFF; }
#define IncrementU16By(variable, amount)   if ((variable) + (amount) < 0xFFFF && (variable) + (amount) > (variable)) { (variable) += (amount); } else { (variable) = 0xFFFF; }
#define IncrementU32By(variable, amount)   if ((variable) + (amount) < 0xFFFFFFFFUL && (variable) + (amount) > (variable)) { (variable) += (amount); } else { (variable) = 0xFFFFFFFFUL; }
#define IncrementU64By(variable, amount)   if ((variable) + (amount) < 0xFFFFFFFFFFFFFFFFULL && (variable) + (amount) > (variable)) { (variable) += (amount); } else { (variable) = 0xFFFFFFFFFFFFFFFFULL; }
#if TARGET_IS_32BIT
#define IncrementUXXBy(variable, amount)   IncrementU32By(variable, amount)
#else
#define IncrementUXXBy(variable, amount)   IncrementU64By(variable, amount)
#endif
#define Decrement(variable)                if ((variable) > 0) { (variable)--; } else { (variable) = 0; }
#define DecrementBy(variable, amount)      if ((variable) >= (amount)) { (variable) -= (amount); } else { (variable) = 0; }

// Swaps the order of the 4-bytes in a u32 type variable using pointer arithmetic and shifting+masking+or-ing
#define FlipEndianU32(variable) variable = (((*(((const u8*)&(variable)) + 0)) & 0xFF) << 24) | (((*(((const u8*)&(variable)) + 1)) & 0xFF) << 16) | (((*(((const u8*)&(variable)) + 2)) & 0xFF) << 8) | (((*(((const u8*)&(variable)) + 3)) & 0xFF) << 0);

// Finds the one's compliment value for a particular u32 value
#define OnesComplimentU32(variable) (variable ^ 0xFFFFFFFFL)

#define STRUCT_VAR_SIZE(structureName, variableName) sizeof(((const structureName*)1)->variableName)
#define STRUCT_VAR_OFFSET(structureName, variableName) (u32)((const u8*)&((const structureName*)1)->variableName - (const u8*)((const structureName*)1))
#define STRUCT_VAR_END_OFFSET(structureName, variableName) (u32)(((const u8*)&((const structureName*)1)->variableName + sizeof(((const structureName*)1)->variableName)) - (const u8*)((const structureName*)1))
#define IS_VAR_IN_X_BYTES_OF_STRUCT(structureName, numBytes, variableName) ((numBytes) >= STRUCT_VAR_END_OFFSET(structureName, variableName))

#define SwapVariables(varType, var1, var2) do { varType tempVarWithLongNameThatWontConflict = (var2); (var2) = (var1); (var1) = tempVarWithLongNameThatWontConflict; } while(0)

//Use a for loop to execute code at the end of a block (warning: if a break is hit inside the block then the endCode will NOT run!)
#define DeferBlockEx(uniqueName, endCode)                                 for (int uniqueName = 0; uniqueName == 0; (uniqueName = 1, (endCode)))
#define DeferBlock(endCode)                                               DeferBlockEx(DeferBlockIter, (endCode))
//startCode runs at beginning of block
#define DeferBlockWithStartEx(uniqueName, startCode, endCode)             for (int uniqueName = ((startCode), 0); uniqueName == 0; (uniqueName = 1, (endCode)))
#define DeferBlockWithStart(startCode, endCode)                           DeferBlockEx(DeferBlockIter, (startCode), (endCode))
//startCode returns bool to determine if block should run, endCode always runs
#define DeferIfBlockEx(uniqueName, startCodeAndCondition, endCode)        for (int uniqueName = 2 * !(startCodeAndCondition); (uniqueName == 2) ? ((endCode), false) : (uniqueName == 0); (uniqueName = 1, (endCode)))
#define DeferIfBlock(startCodeAndCondition, endCode)                      DeferIfBlockEx(DeferBlockIter, (startCodeAndCondition), (endCode))
//startCode returns bool to determine block should run, endCode only runs if startCode returns true
#define DeferIfBlockCondEndEx(uniqueName, startCodeAndCondition, endCode) for (int uniqueName = 1 * !(startCodeAndCondition); uniqueName == 0; (uniqueName = 1, (endCode)))
#define DeferIfBlockCondEnd(startCodeAndCondition, endCode)               DeferIfBlockCondEndEx(DeferBlockIter, (startCodeAndCondition), (endCode))

// +--------------------------------------------------------------+
// |                  Platform Dependant Macros                   |
// +--------------------------------------------------------------+
//NOTE: Because MSVC is annoying and doesn't have an attribute, you must do
// both START_PACK()+END_PACK() around your plex(s) and add ATTR_PACKED to each plex
#if COMPILER_IS_MSVC
	#define PACKED(structCode)        __pragma( pack(push, 1) ) structCode __pragma(pack(pop))
	#define START_PACK()              __pragma(pack(push, 1))
	#define END_PACK()                __pragma(pack(pop))
	#define ATTR_PACKED               //nothing
	#define EXPORT_FUNC               __declspec(dllexport)
	// #define EXPORT(returnType, functionName, ...) returnType __declspec(dllexport) functionName(__VA_ARGS__)
	// #define IMPORT(returnType, functionName, ...) returnType __declspec(dllimport) functionName(__VA_ARGS__)
	// #define __func__ __FUNCTION__
#else
	#define PACKED(structCode)        structCode __attribute__((__packed__))
	#define START_PACK()              //nothing
	#define END_PACK()                //nothing
	#define ATTR_PACKED               __attribute__((__packed__))
	#define EXPORT_FUNC               __attribute__((visibility("default")))
	// #define EXPORT(returnType, functionName, ...) returnType __attribute__((export_name(#functionName))) functionName(__VA_ARGS__)
	// #define IMPORT(returnType, functionName, ...) returnType __declspec(dllimport) functionName(__VA_ARGS__)
	//TODO: Figure out how to do IMPORT on other platforms
#endif

#if TARGET_IS_WASM
	#define WASM_EXPORT(exportName) __attribute__((export_name(exportName)))
#else
	#define WASM_EXPORT(exportName) //nothing
#endif

#endif //  _BASE_MACROS_H

// +--------------------------------------------------------------+
// |                   Autocomplete Dictionary                    |
// +--------------------------------------------------------------+
/*
@Defines
Pi64
Pi32
QuarterPi64
ThirdPi64
HalfPi64
ThreeHalfsPi64
TwoPi64
QuarterPi32
ThirdPi32
HalfPi32
ThreeHalfsPi32
TwoPi32
e64
e32
Sqrt2_64
Sqrt2_32
UNKNOWN_STR
ZEROED
ATTR_PACKED
__func__
START_EXTERN_C
END_EXTERN_C
@Types
@Functions
#define IsVersionBelow(versionMajor, versionMinor, numberMajor, numberMinor)
#define IsVersionAbove(versionMajor, versionMinor, numberMajor, numberMinor)
#define IsEqualXor(variable1, variable2, condition1, condition2)
#define ArrayCount(Array)
#define IsFlagSet(BitwiseField, Bit)
#define FlagSet(BitwiseField, Bit)
#define FlagUnset(BitwiseField, Bit)
#define FlagToggle(BitwiseField, Bit)
#define FlagSetTo(BitwiseField, Bit, condition)
#define FlagEnumSet(BitwiseField, Bit, enumType, castType)
#define FlagEnumUnset(BitwiseField, Bit, enumType, castType)
#define FlagEnumToggle(BitwiseField, Bit, enumType, castType)
#define FlagEnumSetTo(BitwiseField, Bit, condition, enumType, castType)
#define Kilobytes(value)
#define Megabytes(value)
#define Gigabytes(value)
#define Terabytes(value)
#define ToRadians32(degrees)
#define ToRadians64(degrees)
#define ToDegrees32(radians)
#define ToDegrees64(radians)
#define IsPntrWithin(regionStart, regionSize, pntr)
#define IsSizedPntrWithin(regionStart, regionSize, pntr, size)
#define SetOptionalOutPntr(outputPntr, value)
#define PluralEx(number, singularSuffix, multipleSuffix)
#define Plural(number, multipleSuffix)
#define TrackMax(isFirst, trackVariable, newValue)
#define TrackMin(isFirst, trackVariable, newValue)
#define STRINGIFY_DEFINE(define)
#define STRINGIFY(text)
#define INDEX_FROM_COORD3D(coordX, coordY, coordZ, arrayWidth, arrayHeight, arrayDepth)
#define COORD3D_X_FROM_INDEX(voxelIndex, arrayWidth)
#define COORD3D_Y_FROM_INDEX(voxelIndex, arrayWidth, arrayDepth)
#define COORD3D_Z_FROM_INDEX(voxelIndex, arrayWidth, arrayHeight, arrayDepth)
#define COORD3D_FROM_INDEX(voxelIndex, arrayWidth, arrayHeight, arrayDepth)
#define UNUSED(varName)
#define Increment(variable, max)
#define IncrementU8(variable)
#define IncrementU16(variable)
#define IncrementU32(variable)
#define IncrementU64(variable)
#define IncrementUXX(variable)
#define IncrementBy(variable, amount, max)
#define IncrementU8By(variable, amount)
#define IncrementU16By(variable, amount)
#define IncrementU32By(variable, amount)
#define IncrementU64By(variable, amount)
#define IncrementUXXBy(variable, amount)
#define Decrement(variable)
#define DecrementBy(variable, amount)
#define FlipEndianU32(variable)
#define OnesComplimentU32(variable)
#define STRUCT_VAR_SIZE(structureName, variableName)
#define STRUCT_VAR_OFFSET(structureName, variableName)
#define STRUCT_VAR_END_OFFSET(structureName, variableName)
#define IS_VAR_IN_X_BYTES_OF_STRUCT(structureName, numBytes, variableName)
#define SwapVariables(varType, var1, var2)
#define PACKED(class_to_pack)
#define START_PACK()
#define END_PACK()
#define EXPORT(returnType, functionName, ...)
#define IMPORT(returnType, functionName, ...)
*/
