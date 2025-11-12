/*
File:   tools_android_build_helpers.h
Author: Taylor Robbins
Date:   09\16\2025
*/

#ifndef _TOOLS_ANDROID_BUILD_HELPERS_H
#define _TOOLS_ANDROID_BUILD_HELPERS_H

typedef enum AndroidTargetArchitechture AndroidTargetArchitechture;
enum AndroidTargetArchitechture
{
	AndroidTargetArchitechture_None = 0,
	AndroidTargetArchitechture_Arm8,
	AndroidTargetArchitechture_Arm7,
	AndroidTargetArchitechture_x86,
	AndroidTargetArchitechture_Count,
};
const char* GetAndroidTargetArchitechtureStr(AndroidTargetArchitechture enumValue)
{
	switch (enumValue)
	{
		case AndroidTargetArchitechture_None:  return "None";
		case AndroidTargetArchitechture_Arm8:  return "Arm8";
		case AndroidTargetArchitechture_Arm7:  return "Arm7";
		case AndroidTargetArchitechture_x86:   return "x86";
		default: return "Unknown";
	}
}
const char* GetAndroidTargetArchitechtureFolderName(AndroidTargetArchitechture enumValue)
{
	switch (enumValue)
	{
		case AndroidTargetArchitechture_Arm8:  return "arm64-v8a";
		case AndroidTargetArchitechture_Arm7:  return "armeabi-v7a";
		case AndroidTargetArchitechture_x86:   return "x86_64";
		default: return "unknown";
	}
}
const char* GetAndroidTargetArchitechtureTargetStr(AndroidTargetArchitechture enumValue)
{
	switch (enumValue)
	{
		case AndroidTargetArchitechture_Arm8:  return "aarch64-none-linux-android35";
		case AndroidTargetArchitechture_Arm7:  return "armv7a-none-linux-androideabi35";
		case AndroidTargetArchitechture_x86:   return "x86_64-none-linux-android35";
		default: return "unknown";
	}
}
const char* GetAndroidTargetArchitechtureToolchainFolderStr(AndroidTargetArchitechture enumValue)
{
	switch (enumValue)
	{
		case AndroidTargetArchitechture_Arm8:  return "aarch64-linux-android";
		case AndroidTargetArchitechture_Arm7:  return "arm-linux-androideabi";
		case AndroidTargetArchitechture_x86:   return "x86_64-linux-android";
		default: return "unknown";
	}
}

static inline Str8 GetAndroidSdkPath()
{
	const char* sdkEnvVariable = getenv("ANDROID_SDK");
	if (sdkEnvVariable == nullptr)
	{
		WriteLine_E("Please set the ANDROID_SDK environment variable before trying to build for Android");
		exit(7);
	}
	Str8 result = MakeStr8Nt(sdkEnvVariable);
	if (IS_SLASH(result.chars[result.length-1])) { result.length--; }
	result = CopyStr8(result, true);
	FixPathSlashes(result, PATH_SEP_CHAR);
	return result;
}

#endif //  _TOOLS_ANDROID_BUILD_HELPERS_H
