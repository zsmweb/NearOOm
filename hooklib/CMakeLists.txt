# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

set(TARGET arthook)
project(${TARGET})

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
add_compile_options(-Wall -funwind-tables -fPIC -O2)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
        ${TARGET}

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        src/main/cpp/queue.h
        src/main/cpp/tree.h
        src/main/cpp/xh_core.c
        src/main/cpp/xh_core.h
        src/main/cpp/xh_elf.h
        src/main/cpp/xh_elf.c
        src/main/cpp/xh_errno.h
        src/main/cpp/xh_log.c
        src/main/cpp/xh_log.h
        src/main/cpp/xh_util.c
        src/main/cpp/xh_util.h
        src/main/cpp/xh_version.c
        src/main/cpp/xh_version.h
        src/main/cpp/xhook.c
        src/main/cpp/xhook.h

        src/main/cpp/kwai_macros.h
        src/main/cpp/kwai_log.h
        src/main/cpp/kwai_dlfcn.h
        src/main/cpp/kwai_dlfcn.cpp

        src/main/cpp/hookart.cpp
        )

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

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        ${TARGET}

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})