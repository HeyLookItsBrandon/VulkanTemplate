#ifndef ANDROIDLOGGING_H
#define ANDROIDLOGGING_H

#include <android/log.h>

#define LOG_INFO(...) ((void)__android_log_print(ANDROID_LOG_INFO, "vulkan-template", __VA_ARGS__))
#define LOG_WARN(...) ((void)__android_log_print(ANDROID_LOG_WARN, "vulkan-template", __VA_ARGS__))
#define LOG_ERROR(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "vulkan-template", __VA_ARGS__))

#endif
