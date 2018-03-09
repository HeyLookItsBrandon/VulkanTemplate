#ifndef VULKANNATIVEAPP_H
#define VULKANNATIVEAPP_H

#include "BaseNativeApp.h"
#include "vulkan_wrapper/vulkan_wrapper.h"

#include <vector>

struct VulkanSession {
};

class VulkanNativeApp : public BaseNativeApp<VulkanSession> {
	protected:
		VulkanSession* createUserData();
		void initializeDisplay(android_app *app);
		void deinitializeDisplay(android_app *app);

	private:
		VkApplicationInfo createApplicationInfo();
		VkInstanceCreateInfo createInstanceCreationInfo(VkApplicationInfo& applicationInfo);
};

#endif
