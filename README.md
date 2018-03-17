Android Vulkan Template
=======================

This project is a work-in-progress but aims to eventually provide a clean,
concise template for implementing native
[Vulkan](https://www.khronos.org/vulkan/) apps on Android.

Vulkan seems to be the future of cross-platform graphics as the (eventual)
successor to [OpenGL](https://www.khronos.org/opengl/), not to mention
[OpenCL](https://www.khronos.org/opencl/). When I started considering Vulkan
development in late 2017, it wasn't very widely-supported compared to more
established APIs. As a Mac user with a laptop from 2010, these seemed to be my
options:

- A company called Molten offers a Vulkan [SDK](https://moltengl.com/moltenvk/)
for both Mac and iOS. But it has some drawbacks:
  1. It's a wrapper around Apple's [Metal](https://developer.apple.com/metal/)
  API, so it's unclear how that would compare to developing for a "true" Vulkan
  implementation.
  2. It's still in pre-release development (v0.19 as of February 2018.)
  3. It's free to try but a license is $150 if you want to release something
  that uses it.
- Develop on Linux, for Linux, using Lunar G's
[SDK](https://www.lunarg.com/vulkan-sdk/). This seems ideal but I was skeptical
that my ancient GPU would be able to run what I wrote.
- LunarG's SDK is also included as part of the Android SDK. Although, few
devices supported it. The two main devices I found were the (obnoxiously
expensive) nVidia Shield tablet and the (relatively inexpensive)
[Shield TV](https://www.nvidia.com/en-us/shield/shield-tv/).
 
Since the Shield TV seemed to be the most cost-effective way of getting into
Vulkan, I picked one up. However, even with previous OpenGL experience, I
haven't found Google's
[samples](https://github.com/googlesamples/vulkan-basic-samples) to be a very
friendly introduction to using Vulkan on Android. I say that because it is a
single project with nearly four dozen build targets that share different bits
of code, resources and configuration. It might be a great example of how to
write a lot of similar apps without repeating the boilerplate setup work but if
you are just getting started, it's unclear what is essential Vulkan code.
Simply put, it's convoluted.

For me, learning Vulkan is also a bit of a crash course in modern C/C++
development, NDK development,
[native Activities](https://developer.android.com/ndk/guides/concepts.html#naa)
and [CMake](https://cmake.org/). So the more I can strip away clutter and start
learning about Vulkan itself, the better.
