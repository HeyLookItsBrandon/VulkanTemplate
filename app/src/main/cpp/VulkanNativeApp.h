#ifndef VULKANNATIVEAPP_H
#define VULKANNATIVEAPP_H

#include "BaseNativeApp.h"
#include "vulkan_wrapper/vulkan_wrapper.h"

#include <vector>

class VulkanNativeApp : public BaseNativeApp {
	public:
		VulkanNativeApp();
	protected:
		VkInstance* initializeDisplay();
		void deinitializeDisplay(VkInstance*);

		void onWindowInitialized() override;
		void onWindowTerminated() override;

	private:
		VkInstance* vulkanInstance;
		VkApplicationInfo createApplicationInfo();
		VkInstanceCreateInfo createInstanceCreationInfo(VkApplicationInfo& applicationInfo);
};

#endif
