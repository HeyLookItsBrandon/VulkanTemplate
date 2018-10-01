#include <android_native_app_glue.h>

#include "VulkanNativeApp.h"

void android_main(android_app *app) {
	VulkanNativeApp vulkanApp(app);
	vulkanApp.run();
}
