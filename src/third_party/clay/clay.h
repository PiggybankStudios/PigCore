// VERSION: 0.12
// WARNING: Modifications have been made to this version that resides in PigCore.
//  Please refer to the official Clay repostiory for an unmodified version of this file!
//  https://github.com/nicbarker/clay

/*
	NOTE: In order to use this library you must define
	the following macro in exactly one file, _before_ including clay.h:

	#define CLAY_IMPLEMENTATION
	#include "clay.h"

	See the examples folder for details.
*/

// SIMD includes on supported platforms
#if !defined(CLAY_DISABLE_SIMD) && (defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64))
#include <emmintrin.h>
#elif !defined(CLAY_DISABLE_SIMD) && defined(__aarch64__)
#include <arm_neon.h>
#endif

#if !( \
	(defined(__cplusplus) && __cplusplus >= 202002L) || \
	(defined(__STDC__) && __STDC__ == 1 && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) || \
	defined(_MSC_VER) \
)
#error "Clay requires C99, C++20, or MSVC"
#endif

#include "clay_options.h"
#include "clay_macros.h"
#include "clay_arrays.h"
#include "clay_types.h"
#include "clay_globals.h"
IMPLEMENT_TYPED_ARRAY(Clay__Warning, Clay__WarningArray)
IMPLEMENT_TYPED_ARRAY(Clay_RenderCommand, Clay_RenderCommandArray)
#include "clay_api.h"

#ifdef CLAY_IMPLEMENTATION
#undef CLAY_IMPLEMENTATION
#include "clay_arrays.c"
#include "clay_private_api.c"
#include "clay_debug.c"
#include "clay_public_api.c"
#endif // CLAY_IMPLEMENTATION

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
