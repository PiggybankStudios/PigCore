/*
File:   tools_gcc_flags.h
Author: Taylor Robbins
Date:   06\22\2025
Description:
	** Contains defines for all the GCC compiler flags that we may use in our build scripts
*/

#ifndef _TOOLS_GCC_FLAGS_H
#define _TOOLS_GCC_FLAGS_H

#define GCC_COMPILE                  "-c"
#define GCC_PRECOMPILE_ONLY          "-E"
#define GCC_OUTPUT_FILE              "-o \"[VAL]\""
#define GCC_DEFINE                   "-D \"[VAL]\""
#define GCC_INCLUDE_DIR              "-I \"[VAL]\""
#define GCC_DEBUG_INFO_EX            "-g[VAL]" //Produce debug information in the OS's native format
#define GCC_STD_LIB_STATIC           "-MT" //MSVC-style option
#define GCC_STD_LIB_DYNAMIC          "-MD" //MSVC-style option
#define GCC_STD_LIB_STATIC_DBG       "-MTd" //MSVC-style option
#define GCC_STD_LIB_DYNAMIC_DBG      "-MDd" //MSVC-style option
#define GCC_WARNING_LEVEL            "-W[VAL]"
#define GCC_ENABLE_WARNING           "-W[VAL]"
#define GCC_DISABLE_WARNING          "-Wno-[VAL]"
#define GCC_TARGET_THUMB             "-mthumb" //Requests that the compiler targets the T32 (Thumb) instruction set instead of A32 (Arm)
#define GCC_TARGET_CPU               "-mcpu=[VAL]" //Specify a specific CPU to target during code generation
#define GCC_FLOAT_ABI_MODE           "-mfloat-abi=[VAL]" //Whether to use hardware instructions for floating-point operations
#define GCC_TARGET_FPU               "-mfpu=[VAL]" //Specifies the target FPU architecture, that is the floating-point hardware available on the target
#define GCC_ONLY_RELOC_WORD_SIZE     "-mword-relocations" //Only generate absolute relocations on word-sized values (ARM specific option?)
#define GCC_USE_SPEC_FILE            "-specs=[VAL]" //Specify a file to process after the compiler reads in the standard "specs" file. Determines what switches to pass to cc1, cc1plus, as, ld, etc. These are likely located somewhere like [gcc_install_dir]/lib/[file_name].specs
#define GCC_DEPENDENCY_FILE          "-MF \"[VAL]\""
#define GCC_VERBOSE_ASSEMBLY         "-fverbose-asm" //Put extra commentary information in the generated assembly code to make it more readable
#define GCC_GLOBAL_VAR_NO_COMMON     "-fno-common" //Specifies that the compiler places uninitialized global variables in the BSS section of the object files
#define GCC_ALIGN_FUNCS_TO           "-falign-functions=[VAL]" //Align the start of functions to a power of 2 >= the given value
#define GCC_SEP_DATA_SECTIONS        "-fdata-sections" //Place each data into its own section in the output file if the target supports arbitrary sections
#define GCC_SEP_FUNC_SECTIONS        "-ffunction-sections" //Place each function into its own section in the output file if the target supports arbitrary sections
#define GCC_DISABLE_EXCEPTIONS       "-fno-exceptions" //Disable exception handling, which means we don't need to generate code to propagate exceptions up the call stack
#define GCC_OMIT_FRAME_PNTR          "-fomit-frame-pointer" //Omit the frame pointer in functions that don’t need one. This avoids the instructions to save, set up and restore the frame pointer; on many targets it also makes an extra register available
#define GCC_LINKER_SCRIPT            "-T\"[VAL]\"" //Specify the path to a linker script to use (.ld file)
#define GCC_NO_STD_STARTUP           "-nostartfiles" //Do not use the standard system startup files when linking
#define GCC_ENTRYPOINT_NAME          "--entry=[VAL]" //Specify a function name to use as the entrypoint for the application
//NOTE: Arguments prefixed with -Wl, are passed along to the linker
#define GCC_DISABLE_RWX_WARNING      "-Wl,--no-warn-rwx-segments" //Disables an annoying warning about executable+writable sections which comes from choices in the linker script which we don't control
#define GCC_CREF                     "-Wl,--cref" //TODO: Find the documentation for this option and give this define a better name!
#define GCC_GC_SECTIONS              "-Wl,--gc-sections" //TODO: Find the documentation for this option and give this define a better name!
#define GCC_DISABLE_MISMATCH_WARNING "-Wl,--no-warn-mismatch" //Do not warn about mismatching input files? TODO: Find the documentation for this option and give this define a better name!
#define GCC_EMIT_RELOCATIONS         "-Wl,--emit-relocs" //TODO: Find the documentation for this option and give this define a better name!
#define GCC_MAP_FILE                 "-Wl,-Map=\"[VAL]\""

#endif //  _TOOLS_GCC_FLAGS_H
