#include "VulkanNativeApp.h"
#include "AndroidLogging.h"
#include "CapabilityUtils.h"
#include <system_error>
#include <set>

const std::vector<const char*> INSTANCE_EXTENSION_NAMES = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME};

const std::vector<const char*> REQUIRED_DEVICE_EXTENSION_NAMES = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME };

// At the time of writing, these five layers make up the VK_LAYER_LUNARG_standard_validation meta
// layer, which according to presentation slides from LunarG isn't available on Android.
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
	if(debug) {
		logSupportedInstanceExtensions();
		logSupportedValidationLayers();
	}

	createInstance(instance);
	if(debug) {
		registerDebugReportCallback(instance, reportCallback);
	}

	surface = createSurface(instance);

	DeviceInfo deviceInfo = pickPhysicalDevice(surface);

	createLogicalDevice(deviceInfo, logicalDevice);
	vkGetDeviceQueue(logicalDevice, deviceInfo.queueFamilyIndex, 0, &graphicsQueue);
	vkGetDeviceQueue(logicalDevice, deviceInfo.presentationFamilyIndex, 0, &presentQueue);
}

void VulkanNativeApp::deinitializeDisplay() {
	vkDestroyDevice(logicalDevice, nullptr);

	if(debug) {
		destroyDebugReportCallback(instance, reportCallback, nullptr);
	}

	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
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

const DeviceInfo VulkanNativeApp::pickPhysicalDevice(const VkSurfaceKHR& surface) {
	std::vector<VkPhysicalDevice> physicalDevices = getPhysicalDevices(instance);
	LOG_DEBUG("Found %lu physical devices.", physicalDevices.size());
	if (physicalDevices.size() == 0) {
		throw std::runtime_error("No physical devices found.");
	}

	DeviceInfo info = {};
	for(const VkPhysicalDevice& physicalDevice : physicalDevices) {
		if(!getPhysicalDeviceFeatures(physicalDevice).geometryShader ||
				!arePhysicalDeviceExtensionSupported(physicalDevice, REQUIRED_DEVICE_EXTENSION_NAMES)) {
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
				info.physicalDevice = physicalDevice;
				return info;
			}
		}
	}

	throw std::runtime_error("No suitable physical devices found.");
}

std::vector<VkDeviceQueueCreateInfo> VulkanNativeApp::createQueueCreationInfos(DeviceInfo info) {
	std::set<unsigned int> uniqueQueueFamilies = {info.queueFamilyIndex, info.presentationFamilyIndex};
	std::vector<VkDeviceQueueCreateInfo> infos;

	for(unsigned int familyIndex : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

		queueCreateInfo.queueFamilyIndex = familyIndex;
		queueCreateInfo.queueCount = 1;

		float priorities[] = { 1.0f };
		queueCreateInfo.pQueuePriorities = &priorities[0];

		infos.push_back(queueCreateInfo);
	}

	return infos;
}

void VulkanNativeApp::createLogicalDevice(const DeviceInfo &deviceInfo, VkDevice& logicalDevice) {
	std::vector<VkDeviceQueueCreateInfo> queueCreationInfos = createQueueCreationInfos(deviceInfo);

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreationInfos.data();
	createInfo.queueCreateInfoCount = queueCreationInfos.size();
	if(debug) {
		std::vector<const char *> availableLayers = filterUnavailableValidationLayers(VALIDATION_LAYER_NAMES);
		createInfo.enabledLayerCount = (uint32_t) availableLayers.size();
		createInfo.ppEnabledLayerNames = availableLayers.data();
	} else {
		createInfo.enabledLayerCount = 0;
	}

	VkResult deviceCreationResult = vkCreateDevice(deviceInfo.physicalDevice, &createInfo, nullptr, &logicalDevice);
	assertSuccess(deviceCreationResult, "Failed to create logical device.");
}

VkSurfaceKHR VulkanNativeApp::createSurface(VkInstance& instance) {
	VkAndroidSurfaceCreateInfoKHR surfaceInfo = {};
	surfaceInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
	surfaceInfo.window = getApplication()->window;

	VkSurfaceKHR surface;
	VkResult result = vkCreateAndroidSurfaceKHR(instance, &surfaceInfo, nullptr, &surface);
	assertSuccess(result, "Failed to create window");

	return surface;
}

void VulkanNativeApp::createInstance(VkInstance& instance) {
	VkApplicationInfo appInfo = createApplicationInfo();
	VkInstanceCreateInfo instanceCreationInfo = createInstanceCreationInfo(appInfo);
	VkResult instanceCreationResult = vkCreateInstance(&instanceCreationInfo, nullptr, &instance);
	LOG_DEBUG("Vulkan instance creation result: %d", instanceCreationResult);
	assertSuccess(instanceCreationResult, "Vulkan instance creation unsuccessful.");
}

void VulkanNativeApp::registerDebugReportCallback(VkInstance &instance,
		VkDebugReportCallbackEXT &reportCallback) {
	VkDebugReportCallbackCreateInfoEXT reportCallbackCreationInfo = createReportCallbackInfo();
	VkResult reportCallbackCreationResult = createDebugReportCallback(
			instance, &reportCallbackCreationInfo, nullptr, &reportCallback);
	LOG_DEBUG("Vulkan report callback creation result: %d", reportCallbackCreationResult);
	assertSuccess(reportCallbackCreationResult, "Failed to debug create report callback.");
}
