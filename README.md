# Android Vulkan Template

This project is a work-in-progress that aims to eventually provide a clean,
concise template for implementing a native
[Vulkan](https://www.khronos.org/vulkan/) app on Android.

## Motivation

Even with previous OpenGL experience, I haven't found Google/LunarG's
[examples](https://github.com/googlesamples/vulkan-basic-samples) to be a very
friendly introduction to using Vulkan on Android. I say that because it is a
single project with nearly four dozen build targets that share different bits
of code, resources and configuration. It might be a great example of how to fit
multiple cross-platform apps into a single codebase with minimal duplication.
But as an introduction to Vulkan, it's very convoluted.

For me, learning Vulkan on Android is also a refresher in C/C++ development,
not to mention a crash course in NDK development,
[native activities](https://developer.android.com/ndk/guides/concepts.html#naa)
and [CMake](https://cmake.org/). So the more clutter I can strip away from the
essential code, the better.

## Why Vulkan?

Vulkan seems to be the future of cross-platform graphics as the (eventual)
successor to [OpenGL](https://www.khronos.org/opengl/), not to mention
[OpenCL](https://www.khronos.org/opencl/).

## Why Android?

When I started considering Vulkan development in late 2017, it wasn't very
widely-supported. As a Mac user, my access to it was even more limited. As a
Mac user **with a laptop (and GPU) from 2010,** Android seemed like the best
option of these three:

- [MoltenVK](https://github.com/KhronosGroup/MoltenVK) is a Vulkan SDK for Mac
and iOS that appears to have been acquired by The Khronos Group and
open-sourced. For me, it had these drawbacks:
  1. It's a wrapper around Apple's [Metal](https://developer.apple.com/metal/)
  API. My laptop doesn't support Metal but even if it did, it's unclear how
  MoltenVK would compare to developing for a "true" Vulkan implementation.
  2. At the time, it was still in a pre-release state.
  3. Prior to the library being open-sourced, the developer required a $150
  licensing fee if you released software using it.
- Install Linux on my laptop and develop on Linux, for Linux, using [Lunar G's
SDK](https://www.lunarg.com/vulkan-sdk/). This would have been ideal but like
Metal, my laptop's old GeForce GT 330M graphics chip doesn't support Vulkan.
- LunarG's SDK is also included with the Android NDK. But the emulators don't
support Vulkan yet and hardware support is/was pretty limited. The two main
Vulkan-compatible devices I found were the $300 nVidia Shield tablet and
the $180 [Shield TV](https://www.nvidia.com/en-us/shield/shield-tv/).
 
I went with the Shield TV since it was the least expensive way to experiment
with a new technology.
