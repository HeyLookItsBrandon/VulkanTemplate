#ifndef VULKANNATIVEAPP_H
#define VULKANNATIVEAPP_H

#include "BaseNativeApp.h"
#include "vulkan_wrapper/vulkan_wrapper.h"

#include <vector>
#include <tuple>
#include <memory>

struct DeviceInfo {
	VkPhysicalDevice device;
	int queueFamilyIndex = -1;
	int presentationFamilyIndex = -1;

	bool isComplete() {
		return queueFamilyIndex >= 0 && presentationFamilyIndex >= 0;
	}
};

class VulkanNativeApp : public BaseNativeApp {
	public:
		VulkanNativeApp();
	protected:
		void initializeDisplay();
		void deinitializeDisplay();

		void onWindowInitialized() override;
		void onWindowTerminated() override;

		virtual void onReportingEvent(const char *message);

	private:
		const bool debug;

		VkInstance vulkanInstance = {};
		VkDevice device = {};
		VkDebugReportCallbackEXT reportCallback = {};
		VkQueue graphicsQueue;
		VkQueue presentQueue;
		VkSurfaceKHR surface;

		VkApplicationInfo createApplicationInfo();
		VkInstanceCreateInfo createInstanceCreationInfo(VkApplicationInfo& applicationInfo);
		VkDebugReportCallbackCreateInfoEXT createReportCallbackInfo();
		std::vector<VkDeviceQueueCreateInfo> createQueueCreationInfos(DeviceInfo info);

		const DeviceInfo pickPhysicalDevice(std::vector<VkPhysicalDevice> physicalDevices, const VkSurfaceKHR& surface);

		VkDeviceCreateInfo createDeviceCreationInfo(std::vector<VkDeviceQueueCreateInfo>& queueCreationInfo);

		VkSurfaceKHR createSurface(VkInstance instance);

		static VKAPI_ATTR VkBool32 VKAPI_CALL delegateReportCallback( VkDebugReportFlagsEXT flags,
				VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
				int32_t code, const char* layerPrefix, const char* message, void* userData);
};

#endif
