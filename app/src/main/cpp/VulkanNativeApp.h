#ifndef VULKANNATIVEAPP_H
#define VULKANNATIVEAPP_H

#include "BaseNativeApp.h"
#include "vulkan_wrapper/vulkan_wrapper.h"

#include <vector>
#include <tuple>
#include <memory>

struct DeviceInfo {
	const static unsigned int NONE = static_cast<const unsigned int>(-1);

	VkPhysicalDevice physicalDevice;
	unsigned int queueFamilyIndex = NONE;
	unsigned int presentationFamilyIndex = NONE;

	bool isComplete() {
		return queueFamilyIndex != NONE && presentationFamilyIndex != NONE;
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
		VkInstance instance = {};
		VkDebugReportCallbackEXT reportCallback = {};
		VkSurfaceKHR surface;
		VkDevice logicalDevice = {};
		VkQueue graphicsQueue;
		VkQueue presentQueue;

		void createInstance(VkInstance& instance);
		void registerDebugReportCallback(VkInstance &instance,
				VkDebugReportCallbackEXT &reportCallback);

		VkApplicationInfo createApplicationInfo();
		VkInstanceCreateInfo createInstanceCreationInfo(VkApplicationInfo& applicationInfo);
		VkDebugReportCallbackCreateInfoEXT createReportCallbackInfo();
		std::vector<VkDeviceQueueCreateInfo> createQueueCreationInfos(DeviceInfo info);

		const DeviceInfo pickPhysicalDevice(const VkSurfaceKHR& surface);

		void createLogicalDevice(const DeviceInfo &deviceInfo, VkDevice& logicalDevice);

		VkSurfaceKHR createSurface(VkInstance& instance);

		static VKAPI_ATTR VkBool32 VKAPI_CALL delegateReportCallback( VkDebugReportFlagsEXT flags,
				VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
				int32_t code, const char* layerPrefix, const char* message, void* userData);
};

#endif
