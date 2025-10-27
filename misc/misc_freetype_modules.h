/*
File:   misc_freetype_modules.h
Author: Taylor Robbins
Date:   10\15\2025
Description:
	** This file registers the FreeType modules compiled into the library.
	** This file is #included in X-Macro style by FreeType source files through FT_CONFIG_MODULES_H which is defined in misc_freetype_include.h
*/

//NOTE: No include guard here since this file can be #included multiple times

#if !BUILD_WITH_FREETYPE
#error misc_freetype_modules.h should not be #included when we aren't building FreeType source!
#endif

// FT_USE_MODULE( FT_Module_Class, autofit_module_class )
FT_USE_MODULE( FT_Driver_ClassRec, tt_driver_class )
// FT_USE_MODULE( FT_Driver_ClassRec, t1_driver_class )
// FT_USE_MODULE( FT_Driver_ClassRec, cff_driver_class )
// FT_USE_MODULE( FT_Driver_ClassRec, t1cid_driver_class )
// FT_USE_MODULE( FT_Driver_ClassRec, pfr_driver_class )
// FT_USE_MODULE( FT_Driver_ClassRec, t42_driver_class )
// FT_USE_MODULE( FT_Driver_ClassRec, winfnt_driver_class )
// FT_USE_MODULE( FT_Driver_ClassRec, pcf_driver_class )
// FT_USE_MODULE( FT_Driver_ClassRec, bdf_driver_class )
// FT_USE_MODULE( FT_Module_Class, psaux_module_class )
FT_USE_MODULE( FT_Module_Class, psnames_module_class )
// FT_USE_MODULE( FT_Module_Class, pshinter_module_class )
FT_USE_MODULE( FT_Module_Class, sfnt_module_class )
FT_USE_MODULE( FT_Renderer_Class, ft_smooth_renderer_class )
// FT_USE_MODULE( FT_Renderer_Class, ft_raster1_renderer_class )
// FT_USE_MODULE( FT_Renderer_Class, ft_sdf_renderer_class )
// FT_USE_MODULE( FT_Renderer_Class, ft_bitmap_sdf_renderer_class )
FT_USE_MODULE( FT_Renderer_Class, ft_svg_renderer_class )
