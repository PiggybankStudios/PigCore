cmake_minimum_required(VERSION 4.1.1)

project(PigCore VERSION 0.1.0 DESCRIPTION "Pig Core is cool" LANGUAGES C)

# add_library(app-glue STATIC ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
add_library(pig-core SHARED tests_android.c)
include_directories(${ANDROID_NDK}/sources/android/native_app_glue)
# include_directories(tests/)
find_library(log-lib log)
# target_link_libraries(pig-core app-glue android ${log-lib})
target_link_libraries(pig-core android EGL GLESv2 ${log-lib})

# To satisfy 16kB Page Alignment Requirement - https://developer.android.com/guide/practices/page-sizes
target_link_libraries(pig-core LINKER:-z,max-page-size=16384)
