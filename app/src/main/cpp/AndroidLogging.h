#ifndef ANDROIDLOGGING_H
#define ANDROIDLOGGING_H

#include <android/log.h>

#define LOG_INFO(...) ((void)__android_log_print(ANDROID_LOG_INFO, "vulkan-template", __VA_ARGS__))

#endif
