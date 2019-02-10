# Android Vulkan Template

This project is a work in progress. It aims to (eventually) provide a clean, concise template for
implementing a native [Vulkan](https://www.khronos.org/vulkan/) app on Android.

## Motivation

Even with previous OpenGL experience, I haven't found Google/LunarG's
[examples](https://github.com/googlesamples/vulkan-basic-samples) to be a very friendly
introduction to Vulkan on Android. It's an impressive example of code reuse between four dozen
similar apps in a single project. But as an introduction to Vulkan, it's very convoluted.

For me, learning Vulkan on Android is also a refresher in C/C++ development, not to mention a crash
course in NDK development,
[native activities](https://developer.android.com/ndk/guides/concepts.html#naa) and
[CMake](https://cmake.org/). So the more clutter I can strip away from the essential code, the
better.

## Why Vulkan?

Vulkan seems to be the future of cross-platform graphics as the (eventual) successor to
[OpenGL](https://www.khronos.org/opengl/), not to mention [OpenCL](https://www.khronos.org/opencl/).
