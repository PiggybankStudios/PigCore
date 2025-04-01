/*
File:   clay_options.h
Author: Taylor Robbins
Date:   03\31\2025
Description:
	** Checks all the #define driven options for controlling how clay code compiles
	** Was a piece of original clay.h (zlib LICENSE attached below)
*/

#ifndef _CLAY_OPTIONS_H
#define _CLAY_OPTIONS_H

//NOTE: Define CLAY_IMPLEMENTATION in one compilation unit before including clay.h

#ifndef CLAY_DECOR
#define CLAY_DECOR //nothing
#endif

//NOTE: These types MUST be able to be assigned to ZEROED, and a Clay__MemCmp with a zeroed version should be equivalent with checking if it's "empty"
#ifndef CLAY_ELEMENT_USERDATA_TYPE
#define CLAY_ELEMENT_USERDATA_TYPE void*
#endif
#ifndef CLAY_TEXT_USERDATA_TYPE
#define CLAY_TEXT_USERDATA_TYPE void*
#endif
#ifndef CLAY_IMAGEDATA_TYPE
#define CLAY_IMAGEDATA_TYPE void*
#endif

#ifndef CLAY_ERROR_USERDATA_TYPE
#define CLAY_ERROR_USERDATA_TYPE void*
#endif
#ifndef CLAY_ONHOVER_USERDATA_TYPE
#define CLAY_ONHOVER_USERDATA_TYPE void*
#endif
#ifndef CLAY_QUERYSCROLL_USERDATA_TYPE
#define CLAY_QUERYSCROLL_USERDATA_TYPE void*
#endif
#ifndef CLAY_MEASURE_USERDATA_TYPE
#define CLAY_MEASURE_USERDATA_TYPE void*
#endif

#if LANGUAGE_IS_CPP
	#define CLAY_PACKED_ENUM enum : u8
#else //!LANGUAGE_IS_CPP
	#if COMPILER_IS_MSVC
	#define CLAY_PACKED_ENUM __pragma(pack(push, 1)) enum __pragma(pack(pop))
	#else
	#define CLAY_PACKED_ENUM enum __attribute__((__packed__))
	#endif
#endif // LANGUAGE_IS_CPP

#endif //  _CLAY_OPTIONS_H

/*
LICENSE
zlib/libpng license

Copyright (c) 2024 Nic Barker

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the
use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

	1. The origin of this software must not be misrepresented; you must not
	claim that you wrote the original software. If you use this software in a
	product, an acknowledgment in the product documentation would be
	appreciated but is not required.

	2. Altered source versions must be plainly marked as such, and must not
	be misrepresented as being the original software.

	3. This notice may not be removed or altered from any source
	distribution.
*/
