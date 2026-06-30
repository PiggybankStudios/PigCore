/*
File:   clay_arrays.c
Author: Taylor Robbins
Date:   03\31\2025
Description: 
	** Holds implementation of typed arrays that clay uses
	** Was a piece of original clay.h (zlib LICENSE attached below)
*/

//Clay__WarningArray is implemented in clay.h as part of the public API
//Clay_RenderCommandArray is implemented in clay.h as part of the public API

IMPLEMENT_TYPED_ARRAY(Clay_ElementId, Clay__ElementIdArray)

IMPLEMENT_TYPED_ARRAY(Clay_LayoutConfig, Clay__LayoutConfigArray)

IMPLEMENT_TYPED_ARRAY(Clay_TextElementConfig, Clay__TextElementConfigArray)

IMPLEMENT_TYPED_ARRAY(Clay_ImageElementConfig, Clay__ImageElementConfigArray)

IMPLEMENT_TYPED_ARRAY(Clay_FloatingElementConfig, Clay__FloatingElementConfigArray)

IMPLEMENT_TYPED_ARRAY(Clay_CustomElementConfig, Clay__CustomElementConfigArray)

IMPLEMENT_TYPED_ARRAY(Clay_ScrollElementConfig, Clay__ScrollElementConfigArray)

IMPLEMENT_TYPED_ARRAY(Clay_BorderElementConfig, Clay__BorderElementConfigArray)

IMPLEMENT_TYPED_ARRAY(Clay_SharedElementConfig, Clay__SharedElementConfigArray)

IMPLEMENT_TYPED_ARRAY(Clay_ElementConfig, Clay__ElementConfigArray)

IMPLEMENT_TYPED_ARRAY(Clay__WrappedTextLine, Clay__WrappedTextLineArray)

IMPLEMENT_TYPED_ARRAY(Clay__TextElementData, Clay__TextElementDataArray)

IMPLEMENT_TYPED_ARRAY(Clay_LayoutElement, Clay_LayoutElementArray)

IMPLEMENT_TYPED_ARRAY(Clay__ScrollContainerDataInternal, Clay__ScrollContainerDataInternalArray)

IMPLEMENT_TYPED_ARRAY(Clay__DebugElementData, Clay__DebugElementDataArray)

IMPLEMENT_TYPED_ARRAY(Clay_LayoutElementHashMapItem, Clay__LayoutElementHashMapItemArray)

IMPLEMENT_TYPED_ARRAY(Clay__MeasuredWord, Clay__MeasuredWordArray)

IMPLEMENT_TYPED_ARRAY(Clay__MeasureTextCacheItem, Clay__MeasureTextCacheItemArray)

IMPLEMENT_TYPED_ARRAY(Clay__LayoutElementTreeNode, Clay__LayoutElementTreeNodeArray)

IMPLEMENT_TYPED_ARRAY(Clay__LayoutElementTreeRoot, Clay__LayoutElementTreeRootArray)

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
