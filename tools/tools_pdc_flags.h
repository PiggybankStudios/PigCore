/*
File:   tools_pdc_flags.h
Author: Taylor Robbins
Date:   06\22\2025
Description:
	** Contains defines for all the pdc.exe (Playdate packager util) flags that we may use in our build scripts
*/

#ifndef _TOOLS_PDC_FLAGS_H
#define _TOOLS_PDC_FLAGS_H

#define PDC_SDK_PATH      "-sdkpath \"[VAL]\"" //use the SDK at the given path instead of the default
#define PDC_LIBPATH       "--libpath \"[VAL]\"" //add the given path to the list of folders to search when resolving imports
#define PDC_STRIP         "--strip" //strip debug symbols
#define PDC_NO_COMPRESS   "--no-compress" //don't compress output files
#define PDC_MAIN          "--main" //compile lua script at <input> as if it were main.lua
#define PDC_VERBOSE       "--verbose" //verbose mode, gives info about what the compiler is doing
#define PDC_QUIET         "--quiet" //quiet mode, suppresses non-error output
#define PDC_SKIP_UNKNOWN  "--skip-unknown" //skip unrecognized files instead of copying them to the pdx folder
#define PDC_CHECK_FONTS   "--check-fonts" //perform additional validation on font data (may produce false warnings)

#endif //  _TOOLS_PDC_FLAGS_H
