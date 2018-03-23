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

// Ideally, this would just use VK_LAYER_LUNARG_standard_validation meta layer but since it doesn't
// seem to be available, these are the five that make up the meta layer according to
// https://vulkan.lunarg.com/doc/sdk/1.1.70.1/linux/validation_layers.html
const std::vector<const char *> VALIDATION_LAYER_NAMES = {
		"VK_LAYER_GOOGLE_threading",
		"VK_LAYER_LUNARG_parameter_validation",
		"VK_LAYER_LUNARG_object_tracker",
		"VK_LAYER_LUNARG_core_validation",
		"VK_LAYER_GOOGLE_unique_objects"};

VulkanNativeApp::VulkanNativeApp() {
	InitVulkan();
}

void VulkanNativeApp::onWindowInitialized() {
	vulkanInstance = initializeDisplay();
}

void VulkanNativeApp::onWindowTerminated() {
	deinitializeDisplay(vulkanInstance);
}

VkInstance* VulkanNativeApp::initializeDisplay() {
	logSupportedInstanceExtensions();
	logSupportedValidationLayers();

	VkApplicationInfo appInfo = createApplicationInfo();
	VkInstanceCreateInfo instanceCreationInfo = createInstanceCreationInfo(appInfo);

	VkInstance* instance = new VkInstance;
	VkResult instanceCreationResult = vkCreateInstance(&instanceCreationInfo, nullptr, instance);
	LOG_INFO("Vulkan instance creation result: %d", instanceCreationResult);
	if (instanceCreationResult != VK_SUCCESS) {
		throw std::runtime_error("Vulkan instance creation unsuccessful.");
	}

	// Examples always seem to go straight to the lookup but InitVulkan() should have already looked
	// it up from libvulkan.so. That never actually seems to be the case but as a best practice,
	// don't bother looking the reference up if it's already available.
	if(vkCreateDebugReportCallbackEXT == nullptr) {
		vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
				vkGetInstanceProcAddr(*instance, "vkCreateDebugReportCallbackEXT"));
	}

	VkDebugReportCallbackCreateInfoEXT reportCallbackCreationInfo = createReportCallbackInfo();
	VkResult reportCallbackCreationResult = vkCreateDebugReportCallbackEXT(
			*instance, &reportCallbackCreationInfo, nullptr, &reportCallback);
	LOG_INFO("Vulkan report callback creation result: %d", reportCallbackCreationResult);
	if (reportCallbackCreationResult != VK_SUCCESS) {
		throw std::runtime_error("Failed to create report callback!");
	}

	return instance;
}

void VulkanNativeApp::deinitializeDisplay(VkInstance* instance) {
	vkDestroyDebugReportCallbackEXT(*instance, reportCallback, nullptr);
	vkDestroyInstance(*instance, nullptr);
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

	std::vector<const char *> availableLayers =
			filterUnavailableValidationLayers(VALIDATION_LAYER_NAMES);
	createInfo.enabledLayerCount = (uint32_t) availableLayers.size();
	createInfo.ppEnabledLayerNames = availableLayers.data();

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
