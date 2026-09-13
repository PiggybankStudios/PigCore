/*
File:   build_script.c
Author: Taylor Robbins
Date:   07\04\2026
Description: 
	** A PigBuild-based build script for Box3D (https://github.com/erincatto/box3d)
*/

#ifdef _PIG_BUILD_H
#define RUNNING_INDEPENDENTLY 0
#else
#define RUNNING_INDEPENDENTLY 1
#endif

#include "pig_build.h"

#define BOX3D_LIB_FILENAME  "box3d" LIB_EXT
#define BOX3D_DLL_FILENAME  "box3d" DLL_EXT

#define BOX3D_SOURCE_FILES {          \
	"[ROOT]/shared/benchmarks.c",     \
	"[ROOT]/shared/benchmarks.h",     \
	"[ROOT]/shared/determinism.c",    \
	"[ROOT]/shared/determinism.h",    \
	"[ROOT]/shared/human.c",          \
	"[ROOT]/shared/human.h",          \
	"[ROOT]/shared/overflow_color.c", \
	"[ROOT]/shared/overflow_color.h", \
	"[ROOT]/shared/stability.c",      \
	"[ROOT]/shared/stability.h",      \
	"[ROOT]/shared/utils.c",          \
	"[ROOT]/shared/utils.h",          \
	"[ROOT]/src/aabb.c",              \
	"[ROOT]/src/aabb.h",              \
	"[ROOT]/src/algorithm.h",         \
	"[ROOT]/src/arena_allocator.c",   \
	"[ROOT]/src/arena_allocator.h",   \
	"[ROOT]/src/bitset.c",            \
	"[ROOT]/src/bitset.h",            \
	"[ROOT]/src/block_allocator.c",   \
	"[ROOT]/src/block_allocator.h",   \
	"[ROOT]/src/body.c",              \
	"[ROOT]/src/body.h",              \
	"[ROOT]/src/broad_phase.c",       \
	"[ROOT]/src/broad_phase.h",       \
	"[ROOT]/src/capsule.c",           \
	"[ROOT]/src/compound.c",          \
	"[ROOT]/src/compound.h",          \
	"[ROOT]/src/constraint_graph.c",  \
	"[ROOT]/src/constraint_graph.h",  \
	"[ROOT]/src/contact.c",           \
	"[ROOT]/src/contact.h",           \
	"[ROOT]/src/contact_solver.c",    \
	"[ROOT]/src/contact_solver.h",    \
	"[ROOT]/src/container.h",         \
	"[ROOT]/src/convex_manifold.c",   \
	"[ROOT]/src/core.c",              \
	"[ROOT]/src/core.h",              \
	"[ROOT]/src/ctz.h",               \
	"[ROOT]/src/distance.c",          \
	"[ROOT]/src/distance_joint.c",    \
	"[ROOT]/src/dynamic_tree.c",      \
	"[ROOT]/src/height_field.c",      \
	"[ROOT]/src/hull.c",              \
	"[ROOT]/src/id_pool.c",           \
	"[ROOT]/src/id_pool.h",           \
	"[ROOT]/src/island.c",            \
	"[ROOT]/src/island.h",            \
	"[ROOT]/src/joint.c",             \
	"[ROOT]/src/joint.h",             \
	"[ROOT]/src/manifold.c",          \
	"[ROOT]/src/manifold.h",          \
	"[ROOT]/src/math_functions.c",    \
	"[ROOT]/src/math_internal.h",     \
	"[ROOT]/src/mesh.c",              \
	"[ROOT]/src/mesh_contact.c",      \
	"[ROOT]/src/motor_joint.c",       \
	"[ROOT]/src/mover.c",             \
	"[ROOT]/src/parallel_for.c",      \
	"[ROOT]/src/parallel_for.h",      \
	"[ROOT]/src/parallel_joint.c",    \
	"[ROOT]/src/physics_world.c",     \
	"[ROOT]/src/physics_world.h",     \
	"[ROOT]/src/platform.h",          \
	"[ROOT]/src/prismatic_joint.c",   \
	"[ROOT]/src/qsort.h",             \
	"[ROOT]/src/recording.c",         \
	"[ROOT]/src/recording.h",         \
	"[ROOT]/src/recording_ops.inl",   \
	"[ROOT]/src/recording_replay.c",  \
	"[ROOT]/src/recording_replay.h",  \
	"[ROOT]/src/world_snapshot.c",    \
	"[ROOT]/src/world_snapshot.h",    \
	"[ROOT]/src/revolute_joint.c",    \
	"[ROOT]/src/scheduler.c",         \
	"[ROOT]/src/scheduler.h",         \
	"[ROOT]/src/sensor.c",            \
	"[ROOT]/src/sensor.h",            \
	"[ROOT]/src/shape.c",             \
	"[ROOT]/src/shape.h",             \
	"[ROOT]/src/simd.c",              \
	"[ROOT]/src/simd.h",              \
	"[ROOT]/src/solver.c",            \
	"[ROOT]/src/solver.h",            \
	"[ROOT]/src/solver_set.c",        \
	"[ROOT]/src/solver_set.h",        \
	"[ROOT]/src/sphere.c",            \
	"[ROOT]/src/spherical_joint.c",   \
	"[ROOT]/src/table.c",             \
	"[ROOT]/src/table.h",             \
	"[ROOT]/src/timer.c",             \
	"[ROOT]/src/triangle_manifold.c", \
	"[ROOT]/src/types.c",             \
	"[ROOT]/src/verstable.h",         \
	"[ROOT]/src/weld_joint.c",        \
	"[ROOT]/src/wheel_joint.c",       \
}

#if BUILDING_ON_WINDOWS
#define IF_WINDOWS(...)     __VA_ARGS__
#define IF_NOT_WINDOWS(...) //nothing
#else
#define IF_WINDOWS(...)     //nothing
#define IF_NOT_WINDOWS(...) __VA_ARGS__
#endif
#if BUILDING_ON_LINUX
#define IF_LINUX(...)     __VA_ARGS__
#define IF_NOT_LINUX(...) //nothing
#else
#define IF_LINUX(...)     //nothing
#define IF_NOT_LINUX(...) __VA_ARGS__
#endif
#if BUILDING_ON_OSX
#define IF_OSX(...)     __VA_ARGS__
#define IF_NOT_OSX(...) //nothing
#else
#define IF_OSX(...)     //nothing
#define IF_NOT_OSX(...) __VA_ARGS__
#endif

bool IsCompilableExt(Str pathStr)
{
	Str extension = GetFileExtPart(pathStr, false);
	return (
		StrAnyCaseEquals(extension, StrLit(".c")) ||
		StrAnyCaseEquals(extension, StrLit(".cpp")) ||
		StrAnyCaseEquals(extension, StrLit(".m")) ||
		StrAnyCaseEquals(extension, StrLit(".mm"))
	);
}
bool IsCppExt(Str pathStr)
{
	Str extension = GetFileExtPart(pathStr, false);
	return (
		StrAnyCaseEquals(extension, StrLit(".cpp")) ||
		StrAnyCaseEquals(extension, StrLit(".mm"))
	);
}
bool IsObjectiveCExt(Str pathStr)
{
	Str extension = GetFileExtPart(pathStr, false);
	return (
		StrAnyCaseEquals(extension, StrLit(".m")) ||
		StrAnyCaseEquals(extension, StrLit(".mm"))
	);
}

void BuildObject(Str box3dRootFolder, Str sourcePath, Str objectPath, CliArgs* compilerFlags, StrArray* commonTags)
{
	Str sourceFileName = GetFileNamePart(sourcePath, true);
	PrintLine("[Compiling... %.*s -> %.*s]", StrPrint(sourceFileName), StrPrint(objectPath));
	
	bool isCpp = IsCppExt(sourceFileName);
	bool isObjectiveC = IsObjectiveCExt(sourceFileName);
	
	CliArgs args = EMPTY;
	args.rootDirPath = ResolveRootTo(box3dRootFolder, StrLit(".."));
	AddTaggedArg(&args, T_MSVC_CL, CL_COMPILE);
	AddTaggedArg(&args, T_CLANG,   CLANG_COMPILE);
	AddTaggedArgStr(&args, T_MSVC_CL, CL_OBJ_FILE, objectPath);
	AddTaggedArgStr(&args, T_CLANG,   CLANG_OUTPUT_FILE, objectPath);
	AddArgList(&args, compilerFlags);
	AddArgStr(&args, CLI_QUOTED_ARG, sourcePath);
	
	StrArray tags = EMPTY;
	AddTag(&tags, T_OBJECT);
	AddTag(&tags, isObjectiveC ? (isCpp ? T_LANG_OBJECTIVECPP : T_LANG_OBJECTIVEC) : (isCpp ? T_LANG_CPP : T_LANG_C));
	AddStrArray(&tags, commonTags);
	IF_WINDOWS(AddTag(&tags, T_MSVC_CL));
	IF_WINDOWS(AddTag(&tags, T_MSVC_CL_OR_LINK));
	IF_NOT_WINDOWS(AddTag(&tags, T_CLANG));
	
	Str compilerExe = BUILDING_ON_WINDOWS ? StrLit(EXE_MSVC_CL) : StrLit(EXE_CLANG);
	RunCliProgramAndExitOnFailureTags(compilerExe, tags, &args, FormatStr("Failed to compile \"%.*s\"", StrPrint(sourceFileName)));
	AssertFileExist(objectPath, true);
}

void BuildBox3D(bool debugBuild, Str box3dRootFolder)
{
	#if BUILDING_ON_WINDOWS
	bool isMsvcInitialized = WasMsvcDevBatchRun();
	#endif
	const char* sourcePathsNt[] = BOX3D_SOURCE_FILES;
	// const char* includePathsNt[] = BOX3D_INCLUDE_FILES;
	Str box3dDllPath = StrLit(BOX3D_DLL_FILENAME);
	Str box3dLibPath = StrLit(BOX3D_LIB_FILENAME);
	IF_WINDOWS(Str box3dPdbPath = StrLit("box3d.pdb"));
	
	CliArgs compilerFlags = EMPTY;
	CliArgs linkerFlags = EMPTY;
	AddTaggedArg(&compilerFlags,    T_MSVC_CL, CL_FULL_FILE_PATHS);
	AddTaggedArg(&compilerFlags,    T_MSVC_CL_OR_LINK, CL_NO_LOGO);
	AddTaggedArg(&linkerFlags,      T_MSVC_CL, LINK_DISABLE_INCREMENTAL);
	AddTaggedArgNt(&compilerFlags,  T_MSVC_CL, CL_CONFIGURE_EXCEPTION_HANDLING, "s"); //enable stack-unwinding
	AddTaggedArgNt(&compilerFlags,  T_MSVC_CL, CL_CONFIGURE_EXCEPTION_HANDLING, "c"); //extern "C" functions don't through exceptions
	AddTaggedArg(&compilerFlags,    T_CLANG,   CLANG_FULL_FILE_PATHS);
	
	AddTaggedArgNt(&compilerFlags,  T_CLANG,   CLANG_INCLUDE_DIR, "[ROOT]/include");
	AddTaggedArgNt(&compilerFlags,  T_MSVC_CL, CL_INCLUDE_DIR,    "[ROOT]/include");
	AddTaggedArgNt(&compilerFlags,  T_CLANG,   CLANG_INCLUDE_DIR, "[ROOT]/shared");
	AddTaggedArgNt(&compilerFlags,  T_MSVC_CL, CL_INCLUDE_DIR,    "[ROOT]/shared");
	AddTaggedArgNt(&compilerFlags,  T_CLANG,   CLANG_INCLUDE_DIR, "[ROOT]/src");
	AddTaggedArgNt(&compilerFlags,  T_MSVC_CL, CL_INCLUDE_DIR,    "[ROOT]/src");
	
	AddTaggedArgNt(&compilerFlags,  T_MSVC_CL, CL_DEFINE,    "box3d_EXPORTS");
	AddTaggedArgNt(&compilerFlags,  T_CLANG,   CLANG_DEFINE, "box3d_EXPORTS");
	
	AddTaggedArgNt(&compilerFlags,  T_MSVC_CL T_LANG_C,            CL_LANG_VERSION, "c17");
	AddTaggedArgNt(&compilerFlags,  T_CLANG   T_LANG_C,            CLANG_LANG_VERSION, "gnu17");
	AddTaggedArgNt(&compilerFlags,  T_CLANG   T_LANG_OBJECTIVEC,   CLANG_LANG_VERSION, "gnu17");
	AddTaggedArgNt(&compilerFlags,  T_MSVC_CL T_LANG_CPP,          CL_LANG_VERSION, "c++20");
	AddTaggedArgNt(&compilerFlags,  T_CLANG   T_LANG_CPP,          CLANG_LANG_VERSION, "c++20");
	AddTaggedArgNt(&compilerFlags,  T_CLANG   T_LANG_CPP,          CLANG_SYSTEM_LIBRARY, "stdc++");
	AddTaggedArgNt(&compilerFlags,  T_CLANG   T_LANG_OBJECTIVECPP, CLANG_SYSTEM_LIBRARY, "stdc++");
	AddTaggedArgNt(&compilerFlags,  T_CLANG   T_LANG_OBJECTIVEC,   CLANG_LANGUAGE, "objective-c");
	AddTaggedArgNt(&compilerFlags,  T_CLANG   T_LANG_OBJECTIVECPP, CLANG_LANGUAGE, "objective-c++");
	AddTaggedArg(&compilerFlags,    T_CLANG   T_LANG_OBJECTIVEC,   CLANG_ENABLE_OBJC_ARC);
	AddTaggedArg(&compilerFlags,    T_CLANG   T_LANG_OBJECTIVECPP, CLANG_ENABLE_OBJC_ARC);
	
	AddTaggedArg(&compilerFlags,    T_MSVC_CL T_DEBUG_BUILD,        CL_DEBUG_INFO);
	AddTaggedArg(&compilerFlags,    T_MSVC_CL T_DEBUG_BUILD,        CL_STD_LIB_DYNAMIC_DBG);
	AddTaggedArg(&compilerFlags,    T_MSVC_CL T_RELEASE_BUILD,      CL_STD_LIB_DYNAMIC);
	AddTaggedArgNt(&compilerFlags,  T_MSVC_CL T_DEBUG_BUILD,        CL_OPTIMIZATION_LEVEL, "d");
	AddTaggedArgNt(&compilerFlags,  T_MSVC_CL T_RELEASE_BUILD,      CL_OPTIMIZATION_LEVEL, "2");
	AddTaggedArgNt(&compilerFlags,  T_MSVC_CL T_RELEASE_BUILD,      CL_OPTIMIZATION_LEVEL, "y");
	AddTaggedArgNt(&compilerFlags,  T_MSVC_CL T_RELEASE_BUILD,      CL_OPTIMIZATION_LEVEL, "t");
	AddTaggedArgNt(&compilerFlags,  T_CLANG T_UNIX T_DEBUG_BUILD,   CLANG_OPTIMIZATION_LEVEL, "0");
	AddTaggedArgNt(&compilerFlags,  T_CLANG T_UNIX T_RELEASE_BUILD, CLANG_OPTIMIZATION_LEVEL, "2");
	AddTaggedArgNt(&compilerFlags,  T_CLANG T_UNIX T_DEBUG_BUILD,   CLANG_DEBUG_INFO, "dwarf-4");
	
	// AddTaggedArgNt(&compilerFlags,  T_MSVC_CL, CL_WARNING_LEVEL, "X");
	// AddTaggedArgNt(&compilerFlags,  T_MSVC_CL, CL_WARNING_LEVEL, "4");
	// AddTaggedArgNt(&compilerFlags,  T_CLANG,   CLANG_WARNING_LEVEL, "all");
	// AddTaggedArgNt(&compilerFlags,  T_CLANG,   CLANG_WARNING_LEVEL, "extra");
	AddTaggedArgNt(&compilerFlags,  T_CLANG,   CLANG_DISABLE_WARNING, "unused-command-line-argument"); //warning: -Z-reserved-lib-stdc++: 'linker' input unused
	AddTaggedArgNt(&compilerFlags,  T_CLANG,   CLANG_DISABLE_WARNING, "deprecated-enum-enum-conversion"); //warning: bitwise operation between different enumeration types ('ImGuiButtonFlags_' and 'ImGuiButtonFlagsPrivate_') is deprecated
	
	// // AddTaggedArgNt(&linkerFlags, T_CLANG T_OSX T_SAMPLES, CLANG_FRAMEWORK, "CoreText"); //For functions like CTFontCollectionCreateMatchingFontDescriptors in os_font.h OsReadPlatformFont
	// AddTaggedArgNt(&linkerFlags, T_CLANG T_OSX T_SAMPLES, CLANG_FRAMEWORK, "Foundation");
	// AddTaggedArgNt(&linkerFlags, T_CLANG T_OSX T_SAMPLES, CLANG_FRAMEWORK, "Cocoa");
	// AddTaggedArgNt(&linkerFlags, T_CLANG T_OSX T_SAMPLES, CLANG_FRAMEWORK, "QuartzCore");
	// // AddTaggedArgNt(&linkerFlags, T_CLANG T_OSX T_SAMPLES, CLANG_FRAMEWORK, "CoreFoundation");
	// AddTaggedArgNt(&linkerFlags, T_CLANG T_OSX T_SAMPLES, CLANG_FRAMEWORK, "Metal");
	// AddTaggedArgNt(&linkerFlags, T_CLANG T_OSX T_SAMPLES, CLANG_FRAMEWORK, "MetalKit");
	// AddTaggedArgNt(&linkerFlags, T_CLANG T_OSX T_SAMPLES, CLANG_FRAMEWORK, "AppKit"); //required for nfd
	// AddTaggedArgNt(&linkerFlags, T_CLANG T_OSX T_SAMPLES, CLANG_FRAMEWORK, "UniformTypeIdentifiers");
	
	StrArray commonTags = EMPTY;
	if (debugBuild) { AddTag(&commonTags, "DEBUG_BUILD"); } 
	IF_WINDOWS(AddTag(&commonTags, T_WINDOWS));
	IF_LINUX(AddTag(&commonTags, T_LINUX));
	IF_OSX(AddTag(&commonTags, T_OSX));
	IF_NOT_WINDOWS(AddTag(&commonTags, T_UNIX));
	
	// +==============================+
	// |       Compile Objects        |
	// +==============================+
	StrArray objectPaths = EMPTY;
	for (u64 fIndex = 0; fIndex < ArrayCount(sourcePathsNt); fIndex++)
	{
		Str sourcePath = MakeStrNt(sourcePathsNt[fIndex]);
		if (IsCompilableExt(sourcePath))
		{
			Str objectPath = ChangePathExtension(GetFileNamePart(sourcePath, true), StrLit(OBJ_EXT), true);
			BuildObject(box3dRootFolder, sourcePath, objectPath, &compilerFlags, &commonTags);
			AddStr(&objectPaths, objectPath);
		}
	}
	
	// +==============================+
	// |        Link box3d.dll        |
	// +==============================+
	{
		PrintLine("[Linking %.*s...]", StrPrint(box3dDllPath));
		
		CliArgs args = EMPTY;
		AddTaggedArg(&args,    T_MSVC_LINK, LINK_BUILD_DLL);
		AddTaggedArg(&args,    T_CLANG,     CLANG_BUILD_SHARED_LIB);
		AddTaggedArg(&args,    T_CLANG,     CLANG_fPIC);
		AddTaggedArgStr(&args, T_MSVC_LINK, LINK_OUTPUT_FILE, box3dDllPath);
		AddTaggedArgStr(&args, T_CLANG,     CLANG_OUTPUT_FILE, box3dDllPath);
		AddArgList(&args, &compilerFlags);
		AddArgList(&args, &linkerFlags);
		for (u64 oIndex = 0; oIndex < objectPaths.length; oIndex++) { AddArgStr(&args, CLI_QUOTED_ARG, objectPaths.strings[oIndex]); }
		
		StrArray tags = EMPTY;
		AddTag(&tags, T_LIBRARY);
		AddStrArray(&tags, &commonTags);
		IF_WINDOWS(AddTag(&tags, T_MSVC_LINK));
		IF_WINDOWS(AddTag(&tags, T_MSVC_CL_OR_LINK));
		IF_NOT_WINDOWS(AddTag(&tags, T_CLANG));
		
		Str linkerExe = BUILDING_ON_WINDOWS ? StrLit(EXE_MSVC_LINK) : StrLit(EXE_CLANG);
		RunCliProgramAndExitOnFailureTags(linkerExe, tags, &args, FormatStr("Failed to link \"%.*s\"", StrPrint(box3dDllPath)));
		AssertFileExist(box3dDllPath, true);
		PrintLine("[Done Linking %.*s!]", StrPrint(box3dDllPath));
	}
}

#if RUNNING_INDEPENDENTLY
int main(int argc, char* argv[])
{
	RecompileIfNeeded(StrArray_Empty);
	
	mkdir("debug", FOLDER_PERMISSIONS);
	chdir("debug");
	BuildBox3D(true,  StrLit("../.."));
	chdir("..");
	
	mkdir("release", FOLDER_PERMISSIONS);
	chdir("release");
	BuildBox3D(false, StrLit("../.."));
	chdir("..");
}
#endif //RUNNING_INDEPENDENTLY
