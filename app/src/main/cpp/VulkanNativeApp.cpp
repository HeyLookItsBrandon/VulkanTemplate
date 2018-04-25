#include "VulkanNativeApp.h"
#include "AndroidLogging.h"
#include "CapabilityUtils.h"
#include "MathUtils.h"
#include <system_error>
#include <set>
#include <limits>

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

	SwapChainSupportDetails swapChainDetails = {};
	swapChainDetails.format = pickFormat(deviceInfo.surfaceFormats);
	swapChainDetails.presentMode = pickPresentMode(deviceInfo.presentModes);
	VkSurfaceCapabilitiesKHR capabilities = getPhysicalDeviceSurfaceCapabilities(
			deviceInfo.physicalDevice, deviceInfo.surface);
	swapChainDetails.swapExtent = pickExtent(capabilities);
	swapChainDetails.imageCount = pickImageCount(capabilities);

	createLogicalDevice(deviceInfo, logicalDevice);
	vkGetDeviceQueue(logicalDevice, deviceInfo.queueFamilyIndex, 0, &graphicsQueue);
	vkGetDeviceQueue(logicalDevice, deviceInfo.presentationFamilyIndex, 0, &presentQueue);

	createSwapchain(
			swapchain,
			logicalDevice,
			swapChainDetails,
			deviceInfo,
			capabilities);
}

void VulkanNativeApp::deinitializeDisplay() {
	vkDestroySwapchainKHR(logicalDevice, swapchain, nullptr);

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

	for(const VkPhysicalDevice& physicalDevice : physicalDevices) {
		if(debug) {
			logPhysicalDeviceExtensionProperties(physicalDevice);
		}

		DeviceInfo info = {};
		info.surface = surface;
		info.surfaceFormats = getPhysicalDeviceSurfaceFormats(physicalDevice, surface);
		info.presentModes = getPhysicalDeviceSurfacePresentModes(physicalDevice, surface);

		if(!getPhysicalDeviceFeatures(physicalDevice).geometryShader ||
				!arePhysicalDeviceExtensionSupported(physicalDevice, REQUIRED_DEVICE_EXTENSION_NAMES) ||
				info.surfaceFormats.empty() ||
				info.presentModes.empty()) {
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

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreationInfos.size());
	createInfo.pQueueCreateInfos = queueCreationInfos.data();

	createInfo.enabledExtensionCount = static_cast<uint32_t>(REQUIRED_DEVICE_EXTENSION_NAMES.size());
	createInfo.ppEnabledExtensionNames = REQUIRED_DEVICE_EXTENSION_NAMES.data();

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

VkSurfaceFormatKHR VulkanNativeApp::pickFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
	if(formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
		return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
	}

	for(const VkSurfaceFormatKHR & format : formats) {
		if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
				format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return format;
		}
	}

	return formats[0]; // Okay, I give up. Let's give this a try.
}

VkPresentModeKHR VulkanNativeApp::pickPresentMode(const std::vector<VkPresentModeKHR>& presentModes) {
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	for (const VkPresentModeKHR& presentMode : presentModes) {
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return presentMode;
		} else if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			bestMode = presentMode;
		}
	}

	return bestMode;
}

VkExtent2D VulkanNativeApp::pickExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
	if (capabilities.currentExtent.width == std::numeric_limits<uint32_t>::max()) {
		return {
				clamp(extentWidth, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
				clamp(extentHeight, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)};
	} else {
		return capabilities.currentExtent;
	}
}

int VulkanNativeApp::pickImageCount(VkSurfaceCapabilitiesKHR capabilities) {
	uint32_t desiredCount = capabilities.minImageCount + 1;
	return capabilities.maxImageCount == 0 ? // 0 means no limit
			desiredCount :
			std::min(desiredCount, capabilities.maxImageCount);
}

void VulkanNativeApp::createSwapchain(
		VkSwapchainKHR& swapchain,
		const VkDevice& device,
		const SwapChainSupportDetails &swapChainSupportDetails,
		const DeviceInfo &deviceInfo,
		const VkSurfaceCapabilitiesKHR &capabilities) {
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

	createInfo.surface = surface;

	createInfo.minImageCount = swapChainSupportDetails.imageCount;
	createInfo.imageFormat = swapChainSupportDetails.format.format;
	createInfo.imageColorSpace = swapChainSupportDetails.format.colorSpace;
	createInfo.imageExtent = swapChainSupportDetails.swapExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	if (deviceInfo.queueFamilyIndex == deviceInfo.presentationFamilyIndex) {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;

		uint32_t indices[] = {
				(uint32_t) deviceInfo.queueFamilyIndex,
				(uint32_t) deviceInfo.presentationFamilyIndex};
		createInfo.pQueueFamilyIndices = indices;
	}

	createInfo.preTransform = capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = swapChainSupportDetails.presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VkResult result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain);
	assertSuccess(result, "Failed to create swap chain.");

	getSwapchainImages(device, swapchain, swapchainImages);

	// TODO Save image format and extent for later
}
