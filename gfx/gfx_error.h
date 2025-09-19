/*
File:   gfx_error.h
Author: Taylor Robbins
Date:   08\15\2025
*/

#ifndef _GFX_ERROR_H
#define _GFX_ERROR_H

#if BUILD_WITH_SOKOL_GFX

#include "base/base_defines_check.h"
#include "misc/misc_sokol_gfx_include.h"

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	#if defined(SOKOL_GLCORE) || defined(SOKOL_GLES3)
	const char* GetOpenGLErrorStr(unsigned int error);
	#endif
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

#if defined(SOKOL_GLCORE) || defined(SOKOL_GLES3)
PEXP const char* GetOpenGLErrorStr(unsigned int error)
{
	switch (error)
	{
		case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
		#ifdef SOKOL_GLCORE
		case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";
		case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW";
		#endif
		case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
		case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
		default: return UNKNOWN_STR;
	}
}
#endif //defined(SOKOL_GLCORE) || defined(SOKOL_GLES3)

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_SOKOL_GFX

#endif //  _GFX_ERROR_H
