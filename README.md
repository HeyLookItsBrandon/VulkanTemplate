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
[native Activities](https://developer.android.com/ndk/guides/concepts.html#naa)
and [CMake](https://cmake.org/). So the more clutter I can strip away from the
essential code, the better.

## Why Vulkan?

Vulkan seems to be the future of cross-platform graphics as the (eventual)
successor to [OpenGL](https://www.khronos.org/opengl/), not to mention
[OpenCL](https://www.khronos.org/opencl/).

## Why Android?

When I started considering Vulkan development in late 2017, it wasn't very
widely-supported compared to more established APIs. As a Mac user, my options
were even more limited. As a Mac user with a computer (and GPU) **from 2010,**
only one of these options seemed possible:

- Molten offers a Vulkan [SDK](https://moltengl.com/moltenvk/) for both Mac and
iOS. But it has some drawbacks:
  1. It's a wrapper around Apple's [Metal](https://developer.apple.com/metal/)
  API. My laptop doesn't support Metal but even if it did, it's unclear how
  Molten would compare to developing for a "true" Vulkan implementation.
  2. It's still in pre-release development (v0.19 as of February 2018.)
  3. It's free to try but a license is $150 if you want to release anything
  that uses it.
- Develop on Linux, for Linux, using Lunar G's
[SDK](https://www.lunarg.com/vulkan-sdk/). This would be ideal but like Metal,
my old GeForce GT 330M graphics chip is too outdated to run Vulkan.
- LunarG's SDK is also included with the Android NDK. But the emulators don't
support Vulkan yet and hardware support is still pretty limited too. The two
main Vulkan-compatible devices I found were the $300 nVidia Shield tablet and
the $180 [Shield TV](https://www.nvidia.com/en-us/shield/shield-tv/).
 
I went with the Shield TV since it was the least expensive way to experiment with a
new technology.
