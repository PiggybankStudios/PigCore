/*
File:   os_jni.h
Author: Taylor Robbins
Date:   09\20\2025
Description:
	** Holds helper functions for interacting with the Java Native Interface (JNI) on Android
	** This interface allows us to interact with the Java Virtual Machine by acquiring a
	** JNIEnv pntr through AttachCurrentThread\DetachCurrentThread and then using that "env"
	** to get references to objects\functions\strings\etc. and call or manipulate them.
*/

#ifndef _OS_JNI_H
#define _OS_JNI_H

#include "base/base_compiler_check.h"
#include "base/base_defines_check.h"
#include "base/base_macros.h"
#include "std/std_includes.h"
#include "struct/struct_string.h"
#include "mem/mem_arena.h"
#include "mem/mem_scratch.h"

#if TARGET_IS_ANDROID

typedef enum JvmType JvmType;
enum JvmType
{
	JvmType_None = 0,
	JvmType_Void,
	JvmType_Bool,
	JvmType_Byte,
	JvmType_Char,
	JvmType_Short,
	JvmType_Int,
	JvmType_Long,
	JvmType_Float,
	JvmType_Double,
	JvmType_Object,
	JvmType_Class,
	JvmType_String,
	JvmType_Throwable,
	JvmType_Array,
	JvmType_Count,
};
#if !PIG_CORE_IMPLEMENTATION
const char* GetJvmTypeStr(JvmType enumValue);
#else
PEXP const char* GetJvmTypeStr(JvmType enumValue)
{
	switch (enumValue)
	{
		case JvmType_None:      return "None";
		case JvmType_Void:      return "Void";
		case JvmType_Bool:      return "Bool";
		case JvmType_Byte:      return "Byte";
		case JvmType_Char:      return "Char";
		case JvmType_Short:     return "Short";
		case JvmType_Int:       return "Int";
		case JvmType_Long:      return "Long";
		case JvmType_Float:     return "Float";
		case JvmType_Double:    return "Double";
		case JvmType_Object:    return "Object";
		case JvmType_Class:     return "Class";
		case JvmType_String:    return "String";
		case JvmType_Throwable: return "Throwable";
		case JvmType_Array:     return "Array";
		default: return UNKNOWN_STR;
	}
}
#endif

typedef plex JvmReturn JvmReturn;
plex JvmReturn
{
	JvmType type;
	car
	{
		jboolean boolValue; //u8
		jbyte byteValue; //i8
		jchar charValue; //u16
		jshort shortValue; //i16
		jint intValue; //i32
		jlong longValue; //i64
		jfloat floatValue; //r32
		jdouble doubleValue; //r64
		jobject objectValue;
		jclass classValue;
		jstring stringValue;
		jthrowable throwableValue;
		jarray arrayValue;
		jbooleanArray boolArrayValue; //bool[]
		jbyteArray byteArrayValue; //i8[]
		jcharArray charArrayValue; //u16[]
		jshortArray shortArrayValue; //i16[]
		jintArray intArrayValue; //i32[]
		jlongArray longArrayValue; //i64[]
		jfloatArray floatArrayValue; //r32[]
		jdoubleArray doubleArrayValue; //r64[]
	};
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void FreeJStr(JNIEnv* env, jstring jstr);
	PIG_CORE_INLINE jstring NewJStr(JNIEnv* env, Str8 str);
	PIG_CORE_INLINE jstring NewJStrNt(JNIEnv* env, const char* nullTermStr);
	PIG_CORE_INLINE Str8 ToStr8FromJStr(JNIEnv* env, Arena* arena, jstring jstr, bool addNullTerm);
	JvmReturn jObjCall(JNIEnv* env, jobject jobj, bool isStaticMethod, const char* funcNameNt, const char* funcTypeSignatureNt, JvmType returnType, bool assertOnNullReturn, ...);
	JvmReturn jClassCall(JNIEnv* env, const char* classNameNt, const char* funcNameNt, const char* funcTypeSignatureNt, JvmType returnType, bool assertOnNullReturn, ...);
	JvmReturn jObjGetField(JNIEnv* env, jobject jobj, bool isStaticField, const char* fieldNameNt, const char* typeSignatureNt, JvmType fieldType, bool assertOnNull);
	JvmReturn jClassGetField(JNIEnv* env, const char* classNameNt, const char* fieldNameNt, const char* typeSignatureNt, JvmType fieldType, bool assertOnNull);
#endif

// +--------------------------------------------------------------+
// |                           Globals                            |
// +--------------------------------------------------------------+
//NOTE: The application must set AndroidJavaVM before calling any functions that rely on the JNI
//      Some functions also need the AndroidNativeActivity set (often to reference it's clazz member)
//      The JavaVM* can be found inside the ANativeActivity* as ->vm member
#if !PIG_CORE_IMPLEMENTATION
extern ANativeActivity* AndroidNativeActivity;
extern JavaVM* AndroidJavaVM;
#else
ANativeActivity* AndroidNativeActivity = nullptr;
JavaVM* AndroidJavaVM = nullptr;
#endif

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define JavaVMAttachBlock(envVarName) JNIEnv* envVarName = nullptr;              \
	DeferBlockWithStartEx(envVarName##_deferIter,                                \
		(*AndroidJavaVM)->AttachCurrentThread(AndroidJavaVM, &envVarName, NULL), \
		(*AndroidJavaVM)->DetachCurrentThread(AndroidJavaVM))

// +==============================+
// |       Field Signatures       |
// +==============================+
#define jGetField_Build_VERSION_CODES(env, levelFieldNameNt) jClassGetField(env, "android/os/Build$VERSION_CODES", (levelFieldNameNt), "I", JvmType_Int, true).intValue

// +==============================+
// |     Function Signatures      |
// +==============================+
// NOTE: JNI type signature strings include the following types (For example when calling GetMethodID on a jclass):
//		'Z': boolean
//		'B': byte
//		'C': char
//		'S': short
//		'I': int
//		'J': long
//		'F': float
//		'D': double
//		"L fully_qualified_class;": fully_qualified_class
//		"[ type": type[]
//		"( arg_types ) return_type": Method with "arg_types" and "return_type"

// android.app.Activity Functions (https://developer.android.com/reference/android/app/Activity)
#define jCall_getWindow(env, activity) jObjCall((env), (activity)->clazz, false, "getWindow", "()Landroid/view/Window;", JvmType_Object, true).objectValue

// android.app.NativeActivity Functions (https://developer.android.com/reference/android/app/NativeActivity)
#define jCall_getSystemService(env, nativeActivity, serviceJStr) jObjCall((env), (nativeActivity)->clazz, false, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;", JvmType_Object, true, (serviceJStr)).objectValue
#define jCall_getFilesDir(env, nativeActivity) jObjCall((env), (nativeActivity)->clazz, false, "getFilesDir", "()Ljava/io/File;", JvmType_Object, true).objectValue

// android.view.Window Functions (https://developer.android.com/reference/android/view/Window)
#define jCall_getDecorView(env, window) jObjCall((env), (window), false, "getDecorView", "()Landroid/view/View;", JvmType_Object, true).objectValue

// android.view.WindowManager Functions (https://developer.android.com/reference/android/view/WindowManager)
#define jCall_getDefaultDisplay(env, windowManager) jObjCall((env), (windowManager), false, "getDefaultDisplay", "()Landroid/view/Display;", JvmType_Object, true).objectValue

// android.view.View Functions (https://developer.android.com/reference/android/view/View)
#define jCall_getRootWindowInsets(env, view) jObjCall((env), (view), false, "getRootWindowInsets", "()Landroid/view/WindowInsets;", JvmType_Object, true).objectValue

// android.view.Display Functions (https://developer.android.com/reference/android/view/Display)
#define jCall_getRotation(env, display) jObjCall((env), (display), false, "getRotation", "()I", JvmType_Int, true).intValue

// android.view.WindowInsets (https://developer.android.com/reference/android/view/WindowInsets)
#define jCall_getDisplayCutout(env, windowInsets) jObjCall((env), (windowInsets), false, "getDisplayCutout", "()Landroid/view/DisplayCutout;", JvmType_Object, false).objectValue
#define jCall_getSystemWindowInsetBottom(env, windowInsets) jObjCall((env), (windowInsets), false, "getSystemWindowInsetBottom", "()I", JvmType_Int, true).intValue
#define jCall_getSystemWindowInsetLeft(env, windowInsets)   jObjCall((env), (windowInsets), false, "getSystemWindowInsetLeft",   "()I", JvmType_Int, true).intValue
#define jCall_getSystemWindowInsetRight(env, windowInsets)  jObjCall((env), (windowInsets), false, "getSystemWindowInsetRight",  "()I", JvmType_Int, true).intValue
#define jCall_getSystemWindowInsetTop(env, windowInsets)    jObjCall((env), (windowInsets), false, "getSystemWindowInsetTop",    "()I", JvmType_Int, true).intValue

// android.view.DisplayCutout (https://developer.android.com/reference/android/view/DisplayCutout)
#define jCall_getSafeInsetBottom(env, displayCutout) jObjCall((env), (displayCutout), false, "getSafeInsetBottom", "()I", JvmType_Int, true).intValue
#define jCall_getSafeInsetLeft(env, displayCutout)   jObjCall((env), (displayCutout), false, "getSafeInsetLeft",   "()I", JvmType_Int, true).intValue
#define jCall_getSafeInsetRight(env, displayCutout)  jObjCall((env), (displayCutout), false, "getSafeInsetRight",  "()I", JvmType_Int, true).intValue
#define jCall_getSafeInsetTop(env, displayCutout)    jObjCall((env), (displayCutout), false, "getSafeInsetTop",    "()I", JvmType_Int, true).intValue

// java.io.File Functions (https://developer.android.com/reference/java/io/File)
#define jCall_getAbsolutePath(env, fileJObj) jObjCall((env), (fileJObj), false, "getAbsolutePath", "()Ljava/lang/String;", JvmType_String, true).stringValue

// android.text.ClipboardManager Functions (https://developer.android.com/reference/android/content/ClipboardManager)
#define jCall_setPrimaryClip(env, clipboardManager, clipData) jObjCall((env), (clipboardManager), false, "setPrimaryClip", "(Landroid/content/ClipData;)V", JvmType_Void, false, (clipData))
#define jCall_hasPrimaryClip(env, clipboardManager) jObjCall((env), (clipboardManager), false, "hasPrimaryClip", "()Z", JvmType_Bool, true).boolValue
#define jCall_getPrimaryClip(env, clipboardManager) jObjCall((env), (clipboardManager), false, "getPrimaryClip", "()Landroid/content/ClipData;", JvmType_Object, true).objectValue

// android.content.ClipData Functions (https://developer.android.com/reference/android/content/ClipData)
#define jCall_ClipData_newPlainText(env, labelJStr, textJStr) jClassCall((env), "android/content/ClipData", "newPlainText", "(Ljava/lang/CharSequence;Ljava/lang/CharSequence;)Landroid/content/ClipData;", JvmType_Object, true, (labelJStr), (textJStr)).objectValue
#define jCall_getItemAt(env, clipData, indexJInt) jObjCall((env), (clipData), false, "getItemAt", "(I)Landroid/content/ClipData$Item;", JvmType_Object, true, (indexJInt)).objectValue
#define jCall_getText(env, clipDataItem) jObjCall((env), (clipDataItem), false, "getText", "()Ljava/lang/CharSequence;", JvmType_Object, true).objectValue
#define jCall_toString(env, charSequence) jObjCall((env), (charSequence), false, "toString", "()Ljava/lang/String;", JvmType_String, true).stringValue

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI void FreeJStr(JNIEnv* env, jstring jstr)
{
	(*env)->DeleteLocalRef(env, jstr);
}

PEXPI jstring NewJStr(JNIEnv* env, Str8 str)
{
	ScratchBegin(scratch);
	Str8 strNt = AllocStrAndCopy(scratch, str.length, str.chars, true);
	jstring result = (*env)->NewStringUTF(env, strNt.chars);
	ScratchEnd(scratch);
	return result;
}
PEXPI jstring NewJStrNt(JNIEnv* env, const char* nullTermStr)
{
	return (*env)->NewStringUTF(env, nullTermStr);
}

PEXPI Str8 ToStr8FromJStr(JNIEnv* env, Arena* arena, jstring jstr, bool addNullTerm)
{
	const char* utfStrNt = (*env)->GetStringUTFChars(env, jstr, nullptr);
	NotNull(utfStrNt);
	Str8 result = AllocStrAndCopyNt(arena, utfStrNt, addNullTerm);
	NotNull(result.chars);
	(*env)->ReleaseStringUTFChars(env, jstr, utfStrNt);
	return result;
}

PEXP JvmReturn jObjCall(JNIEnv* env, jobject jobj, bool isStaticMethod, const char* funcNameNt, const char* funcTypeSignatureNt, JvmType returnType, bool assertOnNullReturn, ...)
{
	jclass classRef = (*env)->GetObjectClass(env, jobj);
	AssertMsg(classRef != nullptr, "Couldn't get Java object's class reference!");
	jmethodID methodId = isStaticMethod
		? (*env)->GetStaticMethodID(env, classRef, funcNameNt, funcTypeSignatureNt)
		: (*env)->GetMethodID(env, classRef, funcNameNt, funcTypeSignatureNt);
	if (methodId == nullptr) { PrintLine_E("Couldn't find method \"%s\" of type \"%s\" on Java object", funcNameNt, funcTypeSignatureNt); }
	AssertMsg(methodId != nullptr, "Couldn't find Java method by name/signature on object's class!");
	va_list args;
	va_start(args, methodId);
	JvmReturn result = { .type = returnType };
	switch (returnType)
	{
		case JvmType_Void:                           if (isStaticMethod) { (*env)->CallStaticVoidMethodV(env,    classRef, methodId, args); } else { (*env)->CallVoidMethodV(env,    jobj, methodId, args); } break;
		case JvmType_Bool:      result.boolValue       = (isStaticMethod ? (*env)->CallStaticBooleanMethodV(env, classRef, methodId, args)     :     (*env)->CallBooleanMethodV(env, jobj, methodId, args)); break;
		case JvmType_Byte:      result.byteValue       = (isStaticMethod ? (*env)->CallStaticByteMethodV(env,    classRef, methodId, args)     :     (*env)->CallByteMethodV(env,    jobj, methodId, args)); break;
		case JvmType_Char:      result.charValue       = (isStaticMethod ? (*env)->CallStaticCharMethodV(env,    classRef, methodId, args)     :     (*env)->CallCharMethodV(env,    jobj, methodId, args)); break;
		case JvmType_Short:     result.shortValue      = (isStaticMethod ? (*env)->CallStaticShortMethodV(env,   classRef, methodId, args)     :     (*env)->CallShortMethodV(env,   jobj, methodId, args)); break;
		case JvmType_Int:       result.intValue        = (isStaticMethod ? (*env)->CallStaticIntMethodV(env,     classRef, methodId, args)     :     (*env)->CallIntMethodV(env,     jobj, methodId, args)); break;
		case JvmType_Long:      result.longValue       = (isStaticMethod ? (*env)->CallStaticLongMethodV(env,    classRef, methodId, args)     :     (*env)->CallLongMethodV(env,    jobj, methodId, args)); break;
		case JvmType_Float:     result.floatValue      = (isStaticMethod ? (*env)->CallStaticFloatMethodV(env,   classRef, methodId, args)     :     (*env)->CallFloatMethodV(env,   jobj, methodId, args)); break;
		case JvmType_Double:    result.doubleValue     = (isStaticMethod ? (*env)->CallStaticDoubleMethodV(env,  classRef, methodId, args)     :     (*env)->CallDoubleMethodV(env,  jobj, methodId, args)); break;
		case JvmType_Object:    result.objectValue     = (isStaticMethod ? (*env)->CallStaticObjectMethodV(env,  classRef, methodId, args)     :     (*env)->CallObjectMethodV(env,  jobj, methodId, args)); break;
		case JvmType_Class:     result.classValue      = (isStaticMethod ? (*env)->CallStaticObjectMethodV(env,  classRef, methodId, args)     :     (*env)->CallObjectMethodV(env,  jobj, methodId, args)); break;
		case JvmType_String:    result.stringValue     = (isStaticMethod ? (*env)->CallStaticObjectMethodV(env,  classRef, methodId, args)     :     (*env)->CallObjectMethodV(env,  jobj, methodId, args)); break;
		case JvmType_Throwable: result.throwableValue  = (isStaticMethod ? (*env)->CallStaticObjectMethodV(env,  classRef, methodId, args)     :     (*env)->CallObjectMethodV(env,  jobj, methodId, args)); break;
		case JvmType_Array:     result.arrayValue      = (isStaticMethod ? (*env)->CallStaticObjectMethodV(env,  classRef, methodId, args)     :     (*env)->CallObjectMethodV(env,  jobj, methodId, args)); break;
		default: AssertMsg(false, "Unsupported returnType in jObjCall!"); break;
	}
	va_end(args);
	if (assertOnNullReturn && (returnType == JvmType_Object || returnType == JvmType_String))
	{
		if (result.objectValue == nullptr) { PrintLine_E("Got null from method \"%s\" with type \"%s\" on Java object", funcNameNt, funcTypeSignatureNt); }
		AssertMsg(result.objectValue != nullptr, "Java method returned null jobject!");
	}
	return result;
}

PEXP JvmReturn jClassCall(JNIEnv* env, const char* classNameNt, const char* funcNameNt, const char* funcTypeSignatureNt, JvmType returnType, bool assertOnNullReturn, ...)
{
	jclass classRef = (*env)->FindClass(env, classNameNt);
	if (classRef == nullptr) { PrintLine_E("Couldn't find Java class named \"%s\"", classNameNt); }
	AssertMsg(classRef != nullptr, "Couldn't find Java class by name!");
	jmethodID methodId = (*env)->GetStaticMethodID(env, classRef, funcNameNt, funcTypeSignatureNt);
	if (methodId == nullptr) { PrintLine_E("Couldn't find method \"%s\" of type \"%s\" on Java class \"%s\"", funcNameNt, funcTypeSignatureNt, classNameNt); }
	AssertMsg(methodId != nullptr, "Couldn't find Java method by name/signature on found class!");
	va_list args;
	va_start(args, methodId);
	JvmReturn result = { .type = returnType };
	switch (returnType)
	{
		case JvmType_Void:                              (*env)->CallStaticVoidMethodV(env,    classRef, methodId, args); break;
		case JvmType_Bool:      result.boolValue      = (*env)->CallStaticBooleanMethodV(env, classRef, methodId, args); break;
		case JvmType_Byte:      result.byteValue      = (*env)->CallStaticByteMethodV(env,    classRef, methodId, args); break;
		case JvmType_Char:      result.charValue      = (*env)->CallStaticCharMethodV(env,    classRef, methodId, args); break;
		case JvmType_Short:     result.shortValue     = (*env)->CallStaticShortMethodV(env,   classRef, methodId, args); break;
		case JvmType_Int:       result.intValue       = (*env)->CallStaticIntMethodV(env,     classRef, methodId, args); break;
		case JvmType_Long:      result.longValue      = (*env)->CallStaticLongMethodV(env,    classRef, methodId, args); break;
		case JvmType_Float:     result.floatValue     = (*env)->CallStaticFloatMethodV(env,   classRef, methodId, args); break;
		case JvmType_Double:    result.doubleValue    = (*env)->CallStaticDoubleMethodV(env,  classRef, methodId, args); break;
		case JvmType_Object:    result.objectValue    = (*env)->CallStaticObjectMethodV(env,  classRef, methodId, args); break;
		case JvmType_Class:     result.classValue     = (*env)->CallStaticObjectMethodV(env,  classRef, methodId, args); break;
		case JvmType_String:    result.stringValue    = (*env)->CallStaticObjectMethodV(env,  classRef, methodId, args); break;
		case JvmType_Throwable: result.throwableValue = (*env)->CallStaticObjectMethodV(env,  classRef, methodId, args); break;
		case JvmType_Array:     result.arrayValue     = (*env)->CallStaticObjectMethodV(env,  classRef, methodId, args); break;
		default: AssertMsg(false, "Unsupported returnType in jClassCall!"); break;
	}
	va_end(args);
	if (assertOnNullReturn && (returnType == JvmType_Object || returnType == JvmType_String))
	{
		if (result.objectValue == nullptr) { PrintLine_E("Got null from method \"%s\" with type \"%s\" on Java class \"%s\"", funcNameNt, funcTypeSignatureNt, classNameNt); }
		AssertMsg(result.objectValue != nullptr, "Java method returned null jobject!");
	}
	return result;
}

PEXP JvmReturn jObjGetField(JNIEnv* env, jobject jobj, bool isStaticField, const char* fieldNameNt, const char* typeSignatureNt, JvmType fieldType, bool assertOnNull)
{
	jclass classRef = (*env)->GetObjectClass(env, jobj);
	AssertMsg(classRef != nullptr, "Couldn't get Java object's class reference!");
	jfieldID fieldId = isStaticField
		? (*env)->GetStaticFieldID(env, classRef, fieldNameNt, typeSignatureNt)
		: (*env)->GetFieldID(env, classRef, fieldNameNt, typeSignatureNt);
	
	if (fieldId == nullptr) { PrintLine_E("Couldn't find field \"%s\" of type \"%s\" on Java object", fieldNameNt, typeSignatureNt); }
	AssertMsg(fieldId != nullptr, "Couldn't find Java field by name/signature on object's class!");
	JvmReturn result = { .type = fieldType };
	switch (fieldType)
	{
		case JvmType_Bool:      result.boolValue      = (isStaticField ? (*env)->GetStaticBooleanField(env, classRef, fieldId) : (*env)->GetBooleanField(env, jobj, fieldId)); break;
		case JvmType_Byte:      result.byteValue      = (isStaticField ? (*env)->GetStaticByteField(env,    classRef, fieldId) : (*env)->GetByteField(env,    jobj, fieldId)); break;
		case JvmType_Char:      result.charValue      = (isStaticField ? (*env)->GetStaticCharField(env,    classRef, fieldId) : (*env)->GetCharField(env,    jobj, fieldId)); break;
		case JvmType_Short:     result.shortValue     = (isStaticField ? (*env)->GetStaticShortField(env,   classRef, fieldId) : (*env)->GetShortField(env,   jobj, fieldId)); break;
		case JvmType_Int:       result.intValue       = (isStaticField ? (*env)->GetStaticIntField(env,     classRef, fieldId) : (*env)->GetIntField(env,     jobj, fieldId)); break;
		case JvmType_Long:      result.longValue      = (isStaticField ? (*env)->GetStaticLongField(env,    classRef, fieldId) : (*env)->GetLongField(env,    jobj, fieldId)); break;
		case JvmType_Float:     result.floatValue     = (isStaticField ? (*env)->GetStaticFloatField(env,   classRef, fieldId) : (*env)->GetFloatField(env,   jobj, fieldId)); break;
		case JvmType_Double:    result.doubleValue    = (isStaticField ? (*env)->GetStaticDoubleField(env,  classRef, fieldId) : (*env)->GetDoubleField(env,  jobj, fieldId)); break;
		case JvmType_Object:    result.objectValue    = (isStaticField ? (*env)->GetStaticObjectField(env,  classRef, fieldId) : (*env)->GetObjectField(env,  jobj, fieldId)); break;
		case JvmType_Class:     result.classValue     = (isStaticField ? (*env)->GetStaticObjectField(env,  classRef, fieldId) : (*env)->GetObjectField(env,  jobj, fieldId)); break;
		case JvmType_String:    result.stringValue    = (isStaticField ? (*env)->GetStaticObjectField(env,  classRef, fieldId) : (*env)->GetObjectField(env,  jobj, fieldId)); break;
		case JvmType_Throwable: result.throwableValue = (isStaticField ? (*env)->GetStaticObjectField(env,  classRef, fieldId) : (*env)->GetObjectField(env,  jobj, fieldId)); break;
		case JvmType_Array:     result.arrayValue     = (isStaticField ? (*env)->GetStaticObjectField(env,  classRef, fieldId) : (*env)->GetObjectField(env,  jobj, fieldId)); break;
		default: AssertMsg(false, "Unsupported fieldType in jObjGetField!"); break;
	}
	if (assertOnNull && (fieldType == JvmType_Object || fieldType == JvmType_String))
	{
		if (result.objectValue == nullptr) { PrintLine_E("Field \"%s\" of type \"%s\" is null on Java object", fieldNameNt, typeSignatureNt); }
		AssertMsg(result.objectValue != nullptr, "Java field was null!");
	}
	return result;
}

PEXP JvmReturn jClassGetField(JNIEnv* env, const char* classNameNt, const char* fieldNameNt, const char* typeSignatureNt, JvmType fieldType, bool assertOnNull)
{
	jclass classRef = (*env)->FindClass(env, classNameNt);
	if (classRef == nullptr) { PrintLine_E("Couldn't find Java class named \"%s\"", classNameNt); }
	AssertMsg(classRef != nullptr, "Couldn't find Java class by name!");
	jfieldID fieldId = (*env)->GetStaticFieldID(env, classRef, fieldNameNt, typeSignatureNt);
	if (fieldId == nullptr) { PrintLine_E("Couldn't find field \"%s\" of type \"%s\" on Java class \"%s\"", fieldNameNt, typeSignatureNt, classNameNt); }
	AssertMsg(fieldId != nullptr, "Couldn't find Java field by name/signature on found class!");
	JvmReturn result = { .type = fieldType };
	switch (fieldType)
	{
		case JvmType_Bool:      result.boolValue      = (*env)->GetStaticBooleanField(env, classRef, fieldId); break;
		case JvmType_Byte:      result.byteValue      = (*env)->GetStaticByteField(env,    classRef, fieldId); break;
		case JvmType_Char:      result.charValue      = (*env)->GetStaticCharField(env,    classRef, fieldId); break;
		case JvmType_Short:     result.shortValue     = (*env)->GetStaticShortField(env,   classRef, fieldId); break;
		case JvmType_Int:       result.intValue       = (*env)->GetStaticIntField(env,     classRef, fieldId); break;
		case JvmType_Long:      result.longValue      = (*env)->GetStaticLongField(env,    classRef, fieldId); break;
		case JvmType_Float:     result.floatValue     = (*env)->GetStaticFloatField(env,   classRef, fieldId); break;
		case JvmType_Double:    result.doubleValue    = (*env)->GetStaticDoubleField(env,  classRef, fieldId); break;
		case JvmType_Object:    result.objectValue    = (*env)->GetStaticObjectField(env,  classRef, fieldId); break;
		case JvmType_Class:     result.classValue     = (*env)->GetStaticObjectField(env,  classRef, fieldId); break;
		case JvmType_String:    result.stringValue    = (*env)->GetStaticObjectField(env,  classRef, fieldId); break;
		case JvmType_Throwable: result.throwableValue = (*env)->GetStaticObjectField(env,  classRef, fieldId); break;
		case JvmType_Array:     result.arrayValue     = (*env)->GetStaticObjectField(env,  classRef, fieldId); break;
		default: AssertMsg(false, "Unsupported fieldType in jClassGetField!"); break;
	}
	if (assertOnNull && (fieldType == JvmType_Object || fieldType == JvmType_String))
	{
		if (result.objectValue == nullptr) { PrintLine_E("Field \"%s\" of type \"%s\" is null on Java class \"%s\"", fieldNameNt, typeSignatureNt, classNameNt); }
		AssertMsg(result.objectValue != nullptr, "Java field was null!");
	}
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //TARGET_IS_ANDROID

#endif //  _OS_JNI_H
