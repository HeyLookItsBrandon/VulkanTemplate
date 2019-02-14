#include "VulkanNativeApp.h"

#include <android_native_app_glue.h>

void android_main(android_app *app) {
	VulkanNativeApp vulkanApp(app);
	vulkanApp.run();
}
