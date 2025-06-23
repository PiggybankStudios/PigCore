/*
File:   tools_build_basics.c
Author: Taylor Robbins
Date:   06\23\2025
Description: 
	** Contains the majority of the logic used to build binaries in PigCore itself
	** These "basics" serve as a platform for other projects' build scripts to pull
	** from since they cover a variety of build scenarios.
	** This file is a .c file, not a .h file, to imply it has direct access to the
	** BuildContext ctx; global variable, which must be defined before #including this file
*/

