/*
File:   cross_sokol_gfx_and_matrices.h
Author: Taylor Robbins
Date:   03\13\2025
*/

#ifndef _CROSS_SOKOL_GFX_AND_MATRICES_H
#define _CROSS_SOKOL_GFX_AND_MATRICES_H

//NOTE: Intentionally no includes here

#if BUILD_WITH_SOKOL_GFX

#if defined(SOKOL_D3D11)
#define MakeOrthographicMat4(left, right, top, bottom, zNear, zFar) MakeOrthographicMat4Dx(left, right, top, bottom, zNear, zFar)
#define MakePerspectiveMat4(fov, aspectRatio, zNear, zFar)          MakePerspectiveMat4Dx(fov, aspectRatio, zNear, zFar)
#else //!defined(SOKOL_D3D11)
#define MakeOrthographicMat4(left, right, top, bottom, zNear, zFar) MakeOrthographicMat4Gl(left, right, top, bottom, zNear, zFar)
#define MakePerspectiveMat4(fov, aspectRatio, zNear, zFar)          MakePerspectiveMat4Gl(fov, aspectRatio, zNear, zFar)
#endif //defined(SOKOL_D3D11)

#endif //BUILD_WITH_SOKOL_GFX

#endif //  _CROSS_SOKOL_GFX_AND_MATRICES_H
