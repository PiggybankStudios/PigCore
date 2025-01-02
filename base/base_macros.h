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

// +--------------------------------------------------------------+
// |                     Function Like Macros                     |
// +--------------------------------------------------------------+
#define IsVersionBelow(versionMajor, versionMinor, numberMajor, numberMinor) (((versionMajor) < (numberMajor)) || ((versionMajor) == (numberMajor) && (versionMinor) < (numberMinor)))
#define IsVersionAbove(versionMajor, versionMinor, numberMajor, numberMinor) (((versionMajor) > (numberMajor)) || ((versionMajor) == (numberMajor) && (versionMinor) > (numberMinor)))

#define IsEqualXor(variable1, variable2, condition1, condition2) (((variable1) == (condition1) && (variable2) == (condition2)) || ((variable1) == (condition2) && (variable2) == (condition1)))

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define IsFlagSet(BitwiseField, Bit) ((Bit) != 0 && ((BitwiseField) & (Bit)) == (Bit))
#define FlagSet(BitwiseField, Bit)   (BitwiseField) |= (Bit)
#define FlagUnset(BitwiseField, Bit) (BitwiseField) &= ~(Bit)
#define FlagToggle(BitwiseField, Bit) ((BitwiseField) ^= (Bit))
#define FlagSetTo(BitwiseField, Bit, condition) if (condition) { FlagSet((BitwiseField), (Bit)); } else { FlagUnset((BitwiseField), (Bit)); }

#define FlagEnumSet(BitwiseField, Bit, enumType, castType)   (BitwiseField) = (enumType)(((castType)(BitwiseField)) | (castType)(Bit))
#define FlagEnumUnset(BitwiseField, Bit, enumType, castType) (BitwiseField) = (enumType)(((castType)(BitwiseField)) & ~((castType)(Bit)))
#define FlagEnumToggle(BitwiseField, Bit, enumType, castType) (BitwiseField) = (enumType)(((castType)(BitwiseField)) ^ ((castType)(Bit)))
#define FlagEnumSetTo(BitwiseField, Bit, condition, enumType, castType) if (condition) { FlagEnumSet((BitwiseField), (Bit), (enumType), (castType)); } else { FlagEnumUnset((BitwiseField), (Bit), (enumType), (castType)); }

#define Kilobytes(value) ((value) * 1024UL)
#define Megabytes(value) (Kilobytes((value)) * 1024UL)
#define Gigabytes(value) (Megabytes((value)) * 1024UL)
#define Terabytes(value) (Gigabytes((value)) * 1024UL)

#define ToRadians32(degrees)		((degrees)/180.0f * Pi32)
#define ToRadians64(degrees)		((degrees)/180.0 * Pi64)
#define ToDegrees32(radians)		((radians)/Pi32 * 180.0f)
#define ToDegrees64(radians)		((radians)/Pi64 * 180.0)

#define IsPntrWithin(regionStart, regionSize, pntr) (((u8*)(pntr)) >= ((u8*)(regionStart)) && ((u8*)(pntr)) <= (((u8*)(regionStart)) + (regionSize)))
#define IsSizedPntrWithin(regionStart, regionSize, pntr, size) (((u8*)(pntr)) >= ((u8*)(regionStart)) && (((u8*)(pntr)) + (size)) <= (((u8*)(regionStart)) + (regionSize)))

#define SetOptionalOutPntr(outputPntr, value) if ((outputPntr) != nullptr) { *(outputPntr) = (value); }

#define PluralEx(number, singularSuffix, multipleSuffix) (((number) == 1) ? (singularSuffix) : (multipleSuffix))
#define Plural(number, multipleSuffix) (((number) == 1) ? "" : (multipleSuffix))

#define TrackMax(isFirst, trackVariable, newValue) do { if ((isFirst) || (trackVariable) < (newValue)) { (trackVariable) = (newValue); } } while(0)
#define TrackMin(isFirst, trackVariable, newValue) do { if ((isFirst) || (trackVariable) > (newValue)) { (trackVariable) = (newValue); } } while(0)

#ifndef STRINGIFY_DEFINE
#define STRINGIFY_DEFINE(define) STRINGIFY(define)
#endif
#ifndef STRINGIFY
#define STRINGIFY(text)          #text
#endif

#define INDEX_FROM_COORD3D(coordX, coordY, coordZ, arrayWidth, arrayHeight, arrayDepth) ( \
	(coordY) * ((arrayWidth) * (arrayDepth)) +                                            \
	(coordZ) * (arrayWidth) +                                                             \
	(coordX)                                                                              \
)
#define COORD3D_FROM_INDEX(voxelIndex, arrayWidth, arrayHeight, arrayDepth) NewVec3i( \
	(i32)((voxelIndex) % (arrayWidth)),                                               \
	(i32)((voxelIndex) / ((arrayWidth) * (arrayDepth))),                              \
	(i32)(((voxelIndex) % ((arrayWidth) * (arrayDepth))) / (arrayWidth))              \
)

// +--------------------------------------------------------------+
// |                  Platform Dependant Macros                   |
// +--------------------------------------------------------------+
#define UNUSED(varName)        (void)(varName)
#define UNREFERENCED(varName)  (void)(varName)
// #define UNUSED(varName)        (void)sizeof(varName)
// #define UNREFERENCED(varName)  (void)sizeof(varName)

// #if WINDOWS_COMPILATION
// 	#define PACKED(class_to_pack) __pragma( pack(push, 1) ) class_to_pack __pragma(pack(pop))
// 	#define START_PACK()  __pragma(pack(push, 1))
// 	#define END_PACK()    __pragma(pack(pop))
// 	#define ATTR_PACKED //nothing
// 	#define EXPORT(returnType, functionName, ...) returnType __declspec(dllexport) functionName(__VA_ARGS__)
// 	#define IMPORT(returnType, functionName, ...) returnType __declspec(dllimport) functionName(__VA_ARGS__)
// 	#define __func__ __FUNCTION__
// #elif PLAYDATE_COMPILATION
// 	#define PACKED(class_to_pack) //TODO: Nothing?
// 	#define START_PACK()          //TODO: Nothing?
// 	#define END_PACK()            //TODO: Nothing?
// 	#define ATTR_PACKED           //TODO: Nothing?
// 	#if PLAYDATE_SIMULATOR
// 	#define EXPORT(returnType, functionName, ...) returnType __declspec(dllexport) functionName(__VA_ARGS__)
// 	#define IMPORT(returnType, functionName, ...) returnType __declspec(dllimport) functionName(__VA_ARGS__)
// 	#else
// 	#define EXPORT(returnType, functionName, ...) returnType __attribute__((dllexport)) functionName(__VA_ARGS__)
// 	#define IMPORT(returnType, functionName, ...) // Not supported!
// 	#endif
// #else
// 	#define PACKED(class_to_pack) class_to_pack __attribute__((__packed__))
// 	#define START_PACK()  //nothing
// 	#define END_PACK()    //nothing
// 	#define ATTR_PACKED __attribute__((__packed__))
// 	#define EXPORT(returnType, functionName, ...) returnType __attribute__((export_name(#functionName))) functionName(__VA_ARGS__)
// 	//TODO: Figure out how to do IMPORT on other platforms
// #endif

#define EXTERN_C_START extern "C" {
#define EXTERN_C_END }

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
ATTR_PACKED
__func__
EXTERN_C_START
EXTERN_C_END
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
#define COORD3D_FROM_INDEX(voxelIndex, arrayWidth, arrayHeight, arrayDepth)
#define UNUSED(varName)
#define UNREFERENCED(varName)
#define PACKED(class_to_pack)
#define START_PACK()
#define END_PACK()
#define EXPORT(returnType, functionName, ...)
#define IMPORT(returnType, functionName, ...)
*/
