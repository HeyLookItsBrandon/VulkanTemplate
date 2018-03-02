Android Vulkan Template
=======================

This project aims to eventually serve as a clean, readable template for writing
native [Vulkan](https://www.khronos.org/vulkan/) apps on Android.

Vulkan seems to be the future of cross-platform graphics as the (eventual)
successor to [OpenGL](https://www.khronos.org/opengl/), not to mention
[OpenCL](https://www.khronos.org/opencl/). When I started looking into Vulkan
development in late 2017, these seemed to be the options:

- A company called Molten offers a Vulkan [SDK](https://moltengl.com/moltenvk/)
for both Mac and iOS but it has some drawbacks:
  1. It's a wrapper around Apple's [Metal](https://developer.apple.com/metal/)
  API, so it's unclear how "true" it would be to developing for a native
  implementation.
  2. It's still in pre-release development (v0.19 as of February 2018.)
  3. It's free to try but a license is $150 if you want to release something
  that uses it.
- LunarG offers [SDKs](https://www.lunarg.com/vulkan-sdk/) for Windows and
Linux but I don't believe the GPU in my laptop from 2010 would support it.
- LunarG's SDK is also included as part of the Android SDK but when I first
started looking into Vulkan development in September 2017, few devices
supported it. The two main devices that _did_ were the (obnoxiously expensive)
nVidia Shield tablet and the (relatively inexpensive)
[Shield TV](https://www.nvidia.com/en-us/shield/shield-tv/).
 
Since the Shield TV seemed to be the most cost-effective way of getting into
Vulkan, I picked one up. However, even with previous OpenGL experience, I
haven't found Google's
[samples](https://github.com/googlesamples/vulkan-basic-samples) to be a very
friendly introduction to using Vulkan on Android. I say that because it is a
single project with dozens and dozens of build targets that share code,
resources and configuration. It's convoluted.

For me, learning Vulkan is also a bit of a crash course in modern C/C++
development, NDK development,
[native Activities](https://developer.android.com/ndk/guides/concepts.html#naa)
and [CMake](https://cmake.org/). So the more I can strip away clutter and start
learning about Vulkan itself, the better.
