#include "VulkanNativeApp.h"
#include "AndroidLogging.h"
#include "CapabilityUtil.h"
#include <system_error>

const std::vector<const char *> INSTANCE_EXTENSION_NAMES = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME};

const std::vector<const char *> DEVICE_EXTENSION_NAMES = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME };

// Ideally, this would just use VK_LAYER_LUNARG_standard_validation meta layer but according to
// presentation slides from LunarG, it isn't available on Android. Instead, these are the five
// actual layers it's comprised of according to
// https://vulkan.lunarg.com/doc/sdk/1.1.70.1/linux/validation_layers.html
const std::vector<const char *> VALIDATION_LAYER_NAMES = {
//		"VK_LAYER_GOOGLE_threading",
		"VK_LAYER_LUNARG_parameter_validation",
//		"VK_LAYER_LUNARG_object_tracker",
		"VK_LAYER_LUNARG_core_validation",
//		"VK_LAYER_GOOGLE_unique_objects"
};

bool isDebugBuild() {
	bool debug = false;
    #ifndef NDEBUG
        debug = true;
    #endif

	return debug;
}

VulkanNativeApp::VulkanNativeApp() : debug(isDebugBuild()) {
	InitVulkan();
}

void VulkanNativeApp::onWindowInitialized() {
	initializeDisplay();
}

void VulkanNativeApp::onWindowTerminated() {
	deinitializeDisplay();
}

void VulkanNativeApp::initializeDisplay() {
	logSupportedInstanceExtensions();
	logSupportedValidationLayers();

	VkApplicationInfo appInfo = createApplicationInfo();
	VkInstanceCreateInfo instanceCreationInfo = createInstanceCreationInfo(appInfo);

	VkResult instanceCreationResult = vkCreateInstance(&instanceCreationInfo, nullptr, &vulkanInstance);
	LOG_INFO("Vulkan instance creation result: %d", instanceCreationResult);
	assertSuccess(instanceCreationResult, "Vulkan instance creation unsuccessful.");

	if(debug) {
		// Examples always seem to go straight to the lookup but InitVulkan() should have already looked
		// it up from libvulkan.so. That never actually seems to be the case but as a best practice,
		// don't bother looking the reference up if it's already available.
		if(vkCreateDebugReportCallbackEXT == nullptr) {
			vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
					vkGetInstanceProcAddr(vulkanInstance, "vkCreateDebugReportCallbackEXT"));
		}

		VkDebugReportCallbackCreateInfoEXT reportCallbackCreationInfo = createReportCallbackInfo();
		VkResult reportCallbackCreationResult = vkCreateDebugReportCallbackEXT(
				vulkanInstance, &reportCallbackCreationInfo, nullptr, &reportCallback);
		LOG_INFO("Vulkan report callback creation result: %d", reportCallbackCreationResult);
		assertSuccess(reportCallbackCreationResult, "Failed to create report callback!");
	}

	std::vector<VkPhysicalDevice> physicalDevices = getPhysicalDevices(vulkanInstance);
	LOG_DEBUG("Found %lu physical devices.", physicalDevices.size());
	if (physicalDevices.size() == 0) {
		throw std::runtime_error("No physical devices with Vulkan support were found.");
	}

	surface = createSurface(vulkanInstance);

	DeviceInfo deviceInfo = pickPhysicalDevice(physicalDevices, surface);

	// Create logical device
	VkDeviceQueueCreateInfo queueCreationInfo = createQueueCreationInfo(deviceInfo);
	VkDeviceCreateInfo deviceCreationInfo = createDeviceCreationInfo(queueCreationInfo);

	VkResult deviceCreationResult = vkCreateDevice(deviceInfo.device, &deviceCreationInfo, nullptr, &device);
	assertSuccess(deviceCreationResult, "Failed to create logical device.");

	vkGetDeviceQueue(device, queueCreationInfo.queueFamilyIndex, 0, &graphicsQueue);
}

void VulkanNativeApp::deinitializeDisplay() {
	vkDestroyDevice(device, nullptr);

	if(debug) {
		if(vkDestroyDebugReportCallbackEXT == nullptr) {
			vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
					vkGetInstanceProcAddr(vulkanInstance, "vkDestroyDebugReportCallbackEXT"));
		}
		vkDestroyDebugReportCallbackEXT(vulkanInstance, reportCallback, nullptr);
	}

	vkDestroySurfaceKHR(vulkanInstance, surface, nullptr);
	vkDestroyInstance(vulkanInstance, nullptr);
}

VkApplicationInfo VulkanNativeApp::createApplicationInfo() {
	VkApplicationInfo info = {};

	info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	info.pApplicationName = "Vulkan Template";
	info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	info.pEngineName = "No Engine";
	info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	info.apiVersion = VK_API_VERSION_1_0;

	return info;
}

VkInstanceCreateInfo VulkanNativeApp::createInstanceCreationInfo(VkApplicationInfo& applicationInfo) {
	VkInstanceCreateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &applicationInfo;

	createInfo.enabledExtensionCount = (uint32_t) INSTANCE_EXTENSION_NAMES.size();
	createInfo.ppEnabledExtensionNames = INSTANCE_EXTENSION_NAMES.data();

	if(debug) {
		std::vector<const char *> availableLayers =
				filterUnavailableValidationLayers(VALIDATION_LAYER_NAMES);
		createInfo.enabledLayerCount = (uint32_t) availableLayers.size();
		createInfo.ppEnabledLayerNames = availableLayers.data();
	} else {
		createInfo.enabledLayerCount = 0;
	}

	return createInfo;
}

void VulkanNativeApp::onReportingEvent(const char *message) {
	LOG_WARN("Validation message: %s", message);
}

VkDebugReportCallbackCreateInfoEXT VulkanNativeApp::createReportCallbackInfo() {
	VkDebugReportCallbackCreateInfoEXT createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
			VK_DEBUG_REPORT_WARNING_BIT_EXT |
			VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	createInfo.pUserData = reinterpret_cast<void *>(this);
	createInfo.pfnCallback = delegateReportCallback;

	return createInfo;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanNativeApp::delegateReportCallback( VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t code,
		const char* layerPrefix, const char* message, void* userData) {
	VulkanNativeApp *app = reinterpret_cast<VulkanNativeApp *>(userData);
	app->onReportingEvent(message);

	return (VkBool32) false;
}

const DeviceInfo VulkanNativeApp::pickPhysicalDevice(std::vector<VkPhysicalDevice> physicalDevices, const VkSurfaceKHR& surface) {
	DeviceInfo info = {};
	for(const VkPhysicalDevice& physicalDevice : physicalDevices) {
		VkPhysicalDeviceFeatures physicalDeviceFeatures = getPhysicalDeviceFeatures(physicalDevice);
		if(!physicalDeviceFeatures.geometryShader) {
			continue;
		}

		std::vector<VkQueueFamilyProperties> queueFamilyProperties = getQueueFamilyProperties(physicalDevice);
		for(uint32_t i = 0; i < queueFamilyProperties.size(); i++) {
			VkQueueFamilyProperties familyProperties = queueFamilyProperties[i];
			if(familyProperties.queueCount > 0) {
				if(familyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
					info.queueFamilyIndex = i;
				}

				if(isPresentationSupported(physicalDevice, i, surface)) {
					info.presentationFamilyIndex = i;
				}
			}

			if(info.isComplete()) {
				info.device = physicalDevice;
				return info;
			}
		}

		throw std::runtime_error("No suitable physical devices found.");
	}

}

VkDeviceQueueCreateInfo VulkanNativeApp::createQueueCreationInfo(DeviceInfo info) {
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = info.queueFamilyIndex;
	queueCreateInfo.queueCount = 1;

	float priorities[] = { 1.0f };
	queueCreateInfo.pQueuePriorities = &priorities[0];

	return queueCreateInfo;
}

VkDeviceCreateInfo VulkanNativeApp::createDeviceCreationInfo(VkDeviceQueueCreateInfo& queueCreationInfo) {
	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.pQueueCreateInfos = &queueCreationInfo;
	createInfo.queueCreateInfoCount = 1;

	if(debug) {
		std::vector<const char *> availableLayers =
				filterUnavailableValidationLayers(VALIDATION_LAYER_NAMES);
		createInfo.enabledLayerCount = (uint32_t) availableLayers.size();
		createInfo.ppEnabledLayerNames = availableLayers.data();
	} else {
		createInfo.enabledLayerCount = 0;
	}

	return createInfo;
}

VkSurfaceKHR VulkanNativeApp::createSurface(VkInstance instance) {
	VkAndroidSurfaceCreateInfoKHR surfaceInfo = {};
	surfaceInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
	surfaceInfo.window = getApplication()->window;

	VkSurfaceKHR surface;
	VkResult result = vkCreateAndroidSurfaceKHR(instance, &surfaceInfo, nullptr, &surface);
	assertSuccess(result, "Failed to create window");

	return surface;
}
