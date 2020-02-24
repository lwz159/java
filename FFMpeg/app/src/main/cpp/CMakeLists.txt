# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
# FFmpeg include 文件
include_directories(../cpp)

add_library( # Sets the name of the library.
        native-lib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        native-lib.cpp)


# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

add_library( avcodec
        SHARED
        IMPORTED )

set_target_properties( # Specifies the target library.
        avcodec

        # Specifies the parameter you want to define.
        PROPERTIES IMPORTED_LOCATION

        # Provides the path to the library you want to import.
        C:/Git/java/FFMpeg/app/src/main/jniLibs/${ANDROID_ABI}/libavcodec.so )

add_library( avdevice
        SHARED
        IMPORTED )

set_target_properties( # Specifies the target library.
        avdevice

        # Specifies the parameter you want to define.
        PROPERTIES IMPORTED_LOCATION

        # Provides the path to the library you want to import.
        C:/Git/java/FFMpeg/app/src/main/jniLibs/${ANDROID_ABI}/libavdevice.so )

add_library( avfilter
        SHARED
        IMPORTED )

set_target_properties( # Specifies the target library.
        avfilter

        # Specifies the parameter you want to define.
        PROPERTIES IMPORTED_LOCATION

        # Provides the path to the library you want to import.
        C:/Git/java/FFMpeg/app/src/main/jniLibs/${ANDROID_ABI}/libavfilter.so )

add_library( avformat
        SHARED
        IMPORTED )

set_target_properties( # Specifies the target library.
        avformat

        # Specifies the parameter you want to define.
        PROPERTIES IMPORTED_LOCATION

        # Provides the path to the library you want to import.
        C:/Git/java/FFMpeg/app/src/main/jniLibs/${ANDROID_ABI}/libavformat.so )

add_library( avutil
        SHARED
        IMPORTED )

set_target_properties( # Specifies the target library.
        avutil

        # Specifies the parameter you want to define.
        PROPERTIES IMPORTED_LOCATION

        # Provides the path to the library you want to import.
        C:/Git/java/FFMpeg/app/src/main/jniLibs/${ANDROID_ABI}/libavutil.so )

add_library( swscale
        SHARED
        IMPORTED )

set_target_properties( # Specifies the target library.
        swscale

        # Specifies the parameter you want to define.
        PROPERTIES IMPORTED_LOCATION

        # Provides the path to the library you want to import.
        C:/Git/java/FFMpeg/app/src/main/jniLibs/${ANDROID_ABI}/libswscale.so )

add_library( swresample
        SHARED
        IMPORTED )

set_target_properties( # Specifies the target library.
        swresample

        # Specifies the parameter you want to define.
        PROPERTIES IMPORTED_LOCATION

        # Provides the path to the library you want to import.
        C:/Git/java/FFMpeg/app/src/main/jniLibs/${ANDROID_ABI}/libswresample.so )

add_library( postproc
        SHARED
        IMPORTED )

set_target_properties( # Specifies the target library.
        postproc

        # Specifies the parameter you want to define.
        PROPERTIES IMPORTED_LOCATION

        # Provides the path to the library you want to import.
        C:/Git/java/FFMpeg/app/src/main/jniLibs/${ANDROID_ABI}/libpostproc.so )
# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        native-lib
        avcodec
        avdevice
        avfilter
        avformat
        avutil
        swscale
        swresample
        postproc
        android
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})