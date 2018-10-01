# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library(native-lib SHARED
            src/main/cpp/native-lib.cpp
            src/main/cpp/BaseNativeApp.cpp
            src/main/cpp/vulkan_wrapper/vulkan_wrapper.cpp
            src/main/cpp/VulkanNativeApp.cpp
            src/main/cpp/AssetUtils.cpp)

add_library(native_app_glue STATIC
		${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

set(CMAKE_SHARED_LINKER_FLAGS
    "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that you want CMake to locate.
              log )

# As seen at https://github.com/googlesamples/android-ndk/blob/master/endless-tunnel/app/src/main/cpp/CMakeLists.txt
# An alternative is get_filename_component(glm_dir "glm" ABSOLUTE) and adding glm_dir to
# target_include_directories. Using include_directories(…) also worked. Not sure which is correct
# but none of them allowed glm/glm.h to be accessed as a system include, like in examples.
add_subdirectory(src/main/cpp/glm)

target_include_directories(native-lib
        PRIVATE ${ANDROID_NDK}/sources/android/native_app_glue)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       native-lib
                       android
                       native_app_glue

                       # Links the target library to the log library included in the NDK.
                       ${log-lib} )