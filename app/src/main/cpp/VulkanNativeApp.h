#ifndef VULKANNATIVEAPP_H
#define VULKANNATIVEAPP_H

#include "BaseNativeApp.h"
#include "vulkan_wrapper/vulkan_wrapper.h"

#include <vector>
#include <tuple>

class VulkanNativeApp : public BaseNativeApp {
	public:
		VulkanNativeApp();
	protected:
		VkInstance* initializeDisplay();
		void deinitializeDisplay(VkInstance*);

		void onWindowInitialized() override;
		void onWindowTerminated() override;

		virtual void onReportingEvent(const char *message);

	private:
		VkInstance* vulkanInstance;
		VkDevice device = {};

		// Debugging
		VkDebugReportCallbackEXT reportCallback; // Used for validation layer callbacks

		VkApplicationInfo createApplicationInfo();
		VkInstanceCreateInfo createInstanceCreationInfo(VkApplicationInfo& applicationInfo);
		VkDebugReportCallbackCreateInfoEXT createReportCallbackInfo();
		VkDeviceQueueCreateInfo createQueueCreationInfo(VkPhysicalDevice device);

		const VkPhysicalDevice& pickPhysicalDevice(std::vector<VkPhysicalDevice> physicalDevices);
		bool isDeviceSuitable(VkPhysicalDevice device);

		uint32_t findQueueFamily(VkPhysicalDevice device);

		VkDeviceCreateInfo createDeviceCreationInfo(VkDeviceQueueCreateInfo& queueCreationInfo);

		static VKAPI_ATTR VkBool32 VKAPI_CALL delegateReportCallback( VkDebugReportFlagsEXT flags,
				VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
				int32_t code, const char* layerPrefix, const char* message, void* userData);
};

#endif
