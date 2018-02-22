#include <android_native_app_glue.h>
#include <android/log.h>

#define LOG_INFO(...) ((void)__android_log_print(ANDROID_LOG_INFO, "vulkan-template", __VA_ARGS__))

void android_main(android_app *app) {
	LOG_INFO("Success!");
}
