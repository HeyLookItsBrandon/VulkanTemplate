#ifndef VULKANNATIVEAPP_H
#define VULKANNATIVEAPP_H

#include "BaseNativeApp.h"
#include "vulkan_wrapper/vulkan_wrapper.h"

#include <vector>

class VulkanNativeApp : public BaseNativeApp {
	protected:
		void initializeDisplay(android_app *app);
		void deinitializeDisplay(android_app *app);

		void onWindowInitialized() override;
		void onWindowTerminated() override;

	private:
		VkApplicationInfo createApplicationInfo();
		VkInstanceCreateInfo createInstanceCreationInfo(VkApplicationInfo& applicationInfo);
};

#endif
