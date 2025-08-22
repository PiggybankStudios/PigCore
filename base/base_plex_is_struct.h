/*
File:   base_plex_is_struct.h
Author: Taylor Robbins
Date:   07\19\2025
*/

#ifndef _BASE_PLEX_IS_STRUCT_H
#define _BASE_PLEX_IS_STRUCT_H

//NOTE: See https://www.youtube.com/watch?v=wo84LFzx5nI
#define plex struct

//NOTE: There is no suggestion for a replacement for union in the video above,
//      Casey recommended "hoare" as a possible rename to call back to C.A.R. Hoare,
//      I chose "car" since it sounds like a simpler/friendlier english word
#define car union

#endif //  _BASE_PLEX_IS_STRUCT_H
