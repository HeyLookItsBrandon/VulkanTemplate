Vulkan Wrapper
==============

This directory contains source generated to load Vulkan from a device's local
Vulkan library – libvulkan.so – as described by Google's [Vulkan
documentation][1]. It does not seem strictly necessary but because it allows
developers to target pre-24 Android API levels, it seems like a best practice.
(Plus I wasn't able to solve unsatisfied linking errors I was running into when
I wasn't using the wrapper.)

It's unclear how (i.e., when and by what script) this source should be
generated. For the time being, it has been copied from one of Google's examples
but I'm hoping to find out those details by asking this [Stackoverflow
question][2]. I haven't had a chance to dig into it but I wonder if the files
aren't generated by something in the Android plaform's [Vulkan framework][3]
project.


[1]: https://developer.android.com/ndk/guides/graphics/getting-started.html#using
[2]: https://stackoverflow.com/questions/49099500
[3]: https://android.googlesource.com/platform/frameworks/native/+/master/vulkan/